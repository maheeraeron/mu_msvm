/*++

Copyright (c) Microsoft Corporation

Module Name:

    EfiHv.c

Abstract:

    Provides an implementation of the EFI_HV_PROTOCOL protocol, which provides
    UEFI access to the Hyper-V hypervisor.

Author:

    John Starks (jostarks) - 2-Jul-2012

--*/

#include <PiDxe.h>
#include <BiosInterface.h>

#include <Protocol/Cpu.h>
#include <Protocol/EfiHv.h>

#include <Guid/EventGroup.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HvHypercallLib.h>
#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)
#include <Library/LocalApicLib.h>
#endif
#if defined(MDE_CPU_AARCH64)
#include <Protocol/HardwareInterrupt.h>
#endif

#define WINHVP_MAX_REPS_PER_HYPERCALL   0xFFF

// Turn off DEBUG output by default as it can be really noisy
#undef DEBUG
#define DEBUG(arg)

#if defined (MDE_CPU_X64)
// Intrinsic defines for X64
// TODO-cho-19259739: This should be removed when we have real fail-fast support.
void
__ud2(
    void
    );

#pragma intrinsic(__ud2)

#endif

typedef struct _EFI_HV_SINT_CONFIGURATION
{
    EFI_HV_INTERRUPT_HANDLER InterruptHandler;
    VOID *Context;
    UINT8 Vector;
} EFI_HV_SINT_CONFIGURATION, *PEFI_HV_SINT_CONFIGURATION;

typedef struct _EFI_HV_PAGES
{
#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)
    UCHAR HypercallPage[EFI_PAGE_SIZE];
#endif
    UCHAR HypercallInputPage[EFI_PAGE_SIZE];
    UCHAR HypercallOutputPage[EFI_PAGE_SIZE];
    HV_SYNIC_EVENT_FLAGS_PAGE EventFlagsPage;
    HV_MESSAGE_PAGE MessagePage;
} EFI_HV_PAGES, *PEFI_HV_PAGES;

struct _EFI_HV_PROTECTION_OBJECT
{
    LIST_ENTRY ListEntry;
    UINT64 GpaPageBase;
    UINT32 NumberOfPages;
};

typedef struct _EFI_HV_PROTECTION_OBJECT EFI_HV_PROTECTION_OBJECT;

HV_HYPERCALL_CONTEXT mHvContext;
HV_HYPERCALL_CONTEXT mHvBypassContext;
BOOLEAN mUseBypassContext;
PEFI_HV_PAGES mHvPages;
PVOID mOutputPageBypass;
PHV_SYNIC_EVENT_FLAGS_PAGE mEventFlagsPage;
PHV_MESSAGE_PAGE mMessagePage;
EFI_HANDLE mHvHandle;
BOOLEAN mSynicConnected;
EFI_EVENT mExitBootServicesEvent;
BOOLEAN mAutoEoi;
BOOLEAN mDirectTimerSupported;
LIST_ENTRY mHostVisiblePageList;
UINT64 mSharedGpaBoundary;

EFI_HV_SINT_CONFIGURATION mSintConfiguration[HV_SYNIC_SINT_COUNT];
UINT8 mVectorSint[256];

EFI_HV_INTERRUPT_HANDLER mDirectTimerInterruptHandlers[256];
HV_X64_MSR_STIMER_CONFIG_CONTENTS mTimerConfiguration[HV_SYNIC_STIMER_COUNT];

#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)

EFI_CPU_ARCH_PROTOCOL *mCpu;

#elif defined(MDE_CPU_AARCH64)

EFI_HARDWARE_INTERRUPT_PROTOCOL *mHwInt;

#endif

extern EFI_HV_PROTOCOL mHv;
extern EFI_HV_IVM_PROTOCOL mHvIvm;


VOID
EFIAPI
EfiHvInterruptHandler (
#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)
    __in EFI_EXCEPTION_TYPE InterruptType,
#elif defined(MDE_CPU_AARCH64)
    __in HARDWARE_INTERRUPT_SOURCE InterruptType,
#endif
    __in EFI_SYSTEM_CONTEXT SystemContext
    )
/*++

Routine Description:

    The interrupt handler for SINT interrupts. Raises to high level and
    calls out to the connected handler.

Arguments:

    InterruptType - The interrupt vector of the arriving interrupt.

    SystemContext - A pointer to a structure containing the processor context
        when the processor was interrupted.

Return Value:

    None.

--*/
{
    EFI_TPL tpl;
    PEFI_HV_SINT_CONFIGURATION sintConfiguration;

    DEBUG((DEBUG_VERBOSE, ">>> %a\n", __FUNCTION__));

    tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);
    if (!mAutoEoi)
    {
#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)

        SendApicEoi();

#elif defined(MDE_CPU_AARCH64)

        mHwInt->EndOfInterrupt(mHwInt, InterruptType);

#endif
    }

    sintConfiguration = &mSintConfiguration[mVectorSint[InterruptType]];
    if (sintConfiguration->InterruptHandler != NULL)
    {
        DEBUG((DEBUG_VERBOSE, "--- %a: calling 0x%p\n", __FUNCTION__,
            sintConfiguration->InterruptHandler));
        sintConfiguration->InterruptHandler(sintConfiguration->Context);
    }

    gBS->RestoreTPL(tpl);

    DEBUG((DEBUG_VERBOSE, "<<< %a\n", __FUNCTION__));
}


EFI_STATUS
EFIAPI
EfiHvConnectSint (
    __in EFI_HV_PROTOCOL *This,
    __in_range(<, HV_SYNIC_SINT_COUNT) HV_SYNIC_SINT_INDEX SintIndex,
    __in UINT8 Vector,
    __in EFI_HV_INTERRUPT_HANDLER InterruptHandler,
    __in VOID *Context
    )
/*++

Routine Description:

    Enables a SINT and provides an interrupt routine to be called at
    TPL_HIGH_LEVEL when the interrupt arrives.

Arguments:

    This - A pointer to the EFI_HV_PROTOCOL instance.

    SintIndex - The SINT to connect.

    Vector - The vector to use for the SINT interrupt.

    InterruptHandler - A pointer to the interrupt handler for the SINT.

    Context - An opaque context to pass to the interrupt handler.

Return Value:

    EFI status.

--*/
{
    HV_SYNIC_SINT sint;
    PEFI_HV_SINT_CONFIGURATION sintConfiguration;
    EFI_STATUS status;
    EFI_TPL tpl;

    DEBUG((DEBUG_VERBOSE, ">>> %a: Index %lx Vector 0x%x Handler 0x%p Context 0x%p\n",
        __FUNCTION__, SintIndex, Vector, InterruptHandler, Context));

    // Disable interrupts while manipulating interrupts.

    tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

    // Ensure the SINT is not already registered.

    sintConfiguration = &mSintConfiguration[SintIndex];
    if (sintConfiguration->Vector != 0)
    {
        status = EFI_ALREADY_STARTED;
        goto Cleanup;
    }

    // Register the interrupt handler.

#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)

    status = mCpu->RegisterInterruptHandler(mCpu, Vector, EfiHvInterruptHandler);

#elif defined(MDE_CPU_AARCH64)

    status = mHwInt->RegisterInterruptSource(mHwInt, (UINTN)Vector, EfiHvInterruptHandler);

#endif
    DEBUG((DEBUG_VERBOSE, "--- %a: RegisterInterruptSource: %r\n", __FUNCTION__, status));

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    // Register the SINT with the hypervisor.

    sint.AsUINT64 = 0;
    sint.Vector = Vector;
    sint.Masked = FALSE;
    sint.AutoEoi = mAutoEoi;

    if (mUseBypassContext)
    {
        // Register the SINT with the host hypervisor before registering it with the paravisor as a proxy interrupt.

        HvHypercallSetVpRegister64Self(&mHvBypassContext, HvRegisterSint0 + SintIndex, sint.AsUINT64);

        sint.Proxy = 1;
    }

    HvHypercallSetVpRegister64Self(&mHvContext, HvRegisterSint0 + SintIndex, sint.AsUINT64);

    // Store the state used by the interrupt handler.

    sintConfiguration->InterruptHandler = InterruptHandler;
    sintConfiguration->Context = Context;
    sintConfiguration->Vector = Vector;
    mVectorSint[Vector] = (UINT8)SintIndex;
    status = EFI_SUCCESS;

Cleanup:
    gBS->RestoreTPL(tpl);
    DEBUG((DEBUG_VERBOSE, "<<< %a: %r\n", __FUNCTION__, status));
    return status;
}


VOID
EFIAPI
EfiHvEventInterruptHandler (
    VOID *Context
    )
/*++

Routine Description:

    An interrupt handler for a SINT interrupt that just signals an event.

Arguments:

    Context - A pointer to the interrupt handler context.

Return Value:

    None.

--*/
{
    EFI_EVENT *event;

    DEBUG((DEBUG_VERBOSE, ">>> %a\n", __FUNCTION__));
    event = Context;
    gBS->SignalEvent(event);
    DEBUG((DEBUG_VERBOSE, "<<< %a\n", __FUNCTION__));
}


EFI_STATUS
EFIAPI
EfiHvConnectSintToEvent (
    __in EFI_HV_PROTOCOL *This,
    __in_range(<, HV_SYNIC_SINT_COUNT) HV_SYNIC_SINT_INDEX SintIndex,
    __in UINT8 Vector,
    __in EFI_EVENT Event
    )
/*++

Routine Description:

    Enables a SINT and provides an event to be signaled when the interrupt
    arrives.

Arguments:

    This - A pointer to the EFI_HV_PROTOCOL instance.

    SintIndex - The SINT to connect.

    Vector - The vector to use for the SINT interrupt.

    Event - An EFI event to signal when the interrupt arrives.

Return Value:

    EFI status.

--*/
{
    EFI_STATUS status;
    DEBUG((DEBUG_VERBOSE, ">>> %a\n", __FUNCTION__));
    status = EfiHvConnectSint(This,
                              SintIndex,
                              Vector,
                              EfiHvEventInterruptHandler,
                              Event);
    DEBUG((DEBUG_VERBOSE, "<<< %a: %r\n", __FUNCTION__, status));
    return status;
}


VOID
EFIAPI
EfiHvDisconnectSint (
    __in EFI_HV_PROTOCOL *This,
    __in_range(<, HV_SYNIC_SINT_COUNT) HV_SYNIC_SINT_INDEX SintIndex
    )
/*++

Routine Description:

    Disables a SINT that was previously enabled with EfiHvConnectSint
    or EfiHvConnectSintToEvent.

Arguments:

    This - A pointer to the EFI_HV_PROTOCOL instance.

    SintIndex - The SINT to disconnect.

Return Value:

    None.

--*/
{
    HV_SYNIC_SINT sint;
    PEFI_HV_SINT_CONFIGURATION sintConfiguration;
    EFI_TPL tpl;

    DEBUG((DEBUG_VERBOSE, ">>> %a: Index 0x%x\n", __FUNCTION__, SintIndex));

    tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

    // Unregister the SINT with the hypervisor.

    sint.AsUINT64 = 0;
    sint.Masked = 1;

    if (mUseBypassContext)
    {
        HvHypercallSetVpRegister64Self(&mHvBypassContext, HvRegisterSint0 + SintIndex, sint.AsUINT64);
    }
    HvHypercallSetVpRegister64Self(&mHvContext, HvRegisterSint0 + SintIndex, sint.AsUINT64);

    // Unregister the interrupt handler.

    sintConfiguration = &mSintConfiguration[SintIndex];
    if (sintConfiguration->Vector != 0)
    {
#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)

        mCpu->RegisterInterruptHandler(mCpu, sintConfiguration->Vector, NULL);

#elif defined(MDE_CPU_AARCH64)

        mHwInt->RegisterInterruptSource(mHwInt, sintConfiguration->Vector, NULL);

#endif
        mVectorSint[sintConfiguration->Vector] = 0;
    }
    sintConfiguration->Vector = 0;
    sintConfiguration->InterruptHandler = NULL;
    sintConfiguration->Context = NULL;

    gBS->RestoreTPL(tpl);
    DEBUG((DEBUG_VERBOSE, "<<< %a\n", __FUNCTION__));
}


HV_MESSAGE *
EFIAPI
EfiHvGetSintMessage (
    __in EFI_HV_PROTOCOL *This,
    __in_range(<, HV_SYNIC_SINT_COUNT) HV_SYNIC_SINT_INDEX SintIndex
    )
/*++

Routine Description:

    Retrieves the next message from the SINT message queue.

Arguments:

    This - A pointer to the EFI_HV_PROTOCOL instance.

    SintIndex - The index of the SINT.

Return Value:

    A pointer to the next message, or NULL if there is currently no message.

--*/
{
    volatile HV_MESSAGE *message;

    DEBUG((DEBUG_VERBOSE, ">>> %a: Index 0x%x\n", __FUNCTION__, SintIndex));

    message = &mMessagePage->SintMessage[SintIndex];
    if (message->Header.MessageType == HvMessageTypeNone)
    {
        DEBUG((DEBUG_VERBOSE, "<<< %a: message is NULL\n", __FUNCTION__));
        return NULL;
    }

    DEBUG((DEBUG_VERBOSE, "<<< %a: message @ 0x%p\n", __FUNCTION__, message));
    return (HV_MESSAGE *)message;
}


VOID
EFIAPI
EfiHvCompleteSintMessage (
    __in EFI_HV_PROTOCOL *This,
    __in_range(<, HV_SYNIC_SINT_COUNT) HV_SYNIC_SINT_INDEX SintIndex
    )
/*++

Routine Description:

    Marks the current message in the SINT message queue as complete so
    that the next message can be processed.

Arguments:

    This - A pointer to the EFI_HV_PROTOCOL instance.

    SintIndex - The index of the SINT.

Return Value:

    None.

--*/
{
    volatile HV_MESSAGE *message;

    DEBUG((DEBUG_VERBOSE, ">>> %a: Index 0x%x\n", __FUNCTION__, SintIndex));

    message = &mMessagePage->SintMessage[SintIndex];
    message->Header.MessageType = HvMessageTypeNone;
    MemoryBarrier();
    if (message->Header.MessageFlags.MessagePending)
    {
        HvHypercallSetVpRegister64Self(mUseBypassContext ? &mHvBypassContext : &mHvContext, HvRegisterEom, 0);
    }
    DEBUG((DEBUG_VERBOSE, "<<< %a\n", __FUNCTION__));
}


volatile HV_SYNIC_EVENT_FLAGS *
EFIAPI
EfiHvGetSintEventFlags (
    __in EFI_HV_PROTOCOL *This,
    __in_range(<, HV_SYNIC_SINT_COUNT) HV_SYNIC_SINT_INDEX SintIndex
    )
/*++

Routine Description:

    Retrieves a pointer to the event flags for a SINT.

Arguments:

    This - A pointer to the EFI_HV_PROTOCOL instance.

    SintIndex - The index of the SINT.

Return Value:

    A pointer to the event flags.

--*/
{
    volatile HV_SYNIC_EVENT_FLAGS *pFlags;
    DEBUG((DEBUG_VERBOSE, ">>> %a: Index 0x%x\n", __FUNCTION__, SintIndex));

    pFlags = &mEventFlagsPage->SintEventFlags[SintIndex];

    DEBUG((DEBUG_VERBOSE, "<<< %a: flags @ 0x%p\n", __FUNCTION__, pFlags));
    return pFlags;
}


UINT64
EFIAPI
EfiHvGetReferenceTime (
    __in EFI_HV_PROTOCOL *This
    )
/*++

Routine Description:

    Retrieves the current hypervisor reference time, in 100ns units.

Arguments:

    This - A pointer to the EFI_HV_PROTOCOL instance.

Return Value:

    The time, in 100ns units.

--*/
{
    UINT64 refTime;
    DEBUG((DEBUG_VERBOSE, ">>> %a\n", __FUNCTION__));
    refTime = HvHypercallGetVpRegister64Self(&mHvContext, HvRegisterTimeRefCount);
    DEBUG((DEBUG_VERBOSE, "<<< %a: reftime 0x%p\n", __FUNCTION__, refTime));
    return refTime;
}


UINT32
EFIAPI
EfiHvGetCurrentVpIndex (
    __in EFI_HV_PROTOCOL *This
    )
/*++

Routine Description:

    Retrieves the current virtual processor index.

Arguments:

    This - A pointer to the EFI_HV_PROTOCOL instance.

Return Value:

    The VP index.

--*/
{
    UINT32 vpIndex;
    DEBUG((DEBUG_VERBOSE, ">>> %a\n", __FUNCTION__));
    vpIndex = (UINT32)HvHypercallGetVpRegister64Self(&mHvContext, HvRegisterVpIndex);
    DEBUG((DEBUG_VERBOSE, "<<< %a: index 0x%x\n", __FUNCTION__, vpIndex));
    return vpIndex;
}


VOID
EFIAPI
EfiHvSetTimer (
    __in EFI_HV_PROTOCOL *This,
    __in UINT32 TimerIndex,
    __in UINT64 Expiration
    )
/*++

Routine Description:

    Sets a hypervisor timer to expire.

Arguments:

    This - A pointer to the EFI_HV_PROTOCOL instance.

    TimerIndex - The index of the timer.

    Expiration - The time to expire. If the timer is periodic, then this
        is the period. Otherwise, this is an absolute time, based on the
        reference time base.

        If 0, then the timer is cancelled.

Return Value:

    None.

--*/
{
    DEBUG((DEBUG_VERBOSE, ">>> %a: Index 0x%x Expiration 0x%x\n", __FUNCTION__,
        TimerIndex, Expiration));
    HvHypercallSetVpRegister64Self(&mHvContext, HvRegisterStimer0Count + (2 * TimerIndex), Expiration);
    DEBUG((DEBUG_VERBOSE, "<<< %a\n", __FUNCTION__));
}


BOOLEAN
EFIAPI
EfiHvDirectTimerSupported (
    VOID
    )
/*++

Routine Description:

    Indicates whether the hypervisor supports direct-mode timers.

Arguments:

    None.

Return Value:

    TRUE if direct mode timers are supported.

--*/
{
    return mDirectTimerSupported;
}


VOID
EFIAPI
EfiHvDirectTimerInterruptHandler (
#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)
    __in EFI_EXCEPTION_TYPE InterruptType,
#elif defined(MDE_CPU_AARCH64)
    __in HARDWARE_INTERRUPT_SOURCE InterruptType,
#endif
    __in EFI_SYSTEM_CONTEXT SystemContext
    )
/*++

Routine Description:

    The interrupt handler for direct-mode timers. Raises to high level and
    calls out to the connected handler.

Arguments:

    InterruptType - The interrupt vector of the arriving interrupt.

    SystemContext - A pointer to a structure containing the processor context
        when the processor was interrupted.

Return Value:

    None.

--*/
{
    EFI_TPL tpl;

    DEBUG((DEBUG_VERBOSE, ">>> %a\n", __FUNCTION__));

    tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)

    SendApicEoi();

#elif defined(MDE_CPU_AARCH64)

    mHwInt->EndOfInterrupt(mHwInt, InterruptType);

#endif

    if (mDirectTimerInterruptHandlers[InterruptType] != NULL)
    {
        DEBUG((DEBUG_VERBOSE, "--- %a: calling 0x%p\n", __FUNCTION__,
            mDirectTimerInterruptHandlers[InterruptType]));
        mDirectTimerInterruptHandlers[InterruptType](NULL);
    }

    gBS->RestoreTPL(tpl);

    DEBUG((DEBUG_VERBOSE, "<<< %a\n", __FUNCTION__));
}


EFI_STATUS
EFIAPI
EfiHvConfigureTimer (
    __in EFI_HV_PROTOCOL *This,
    __in UINT32 TimerIndex,
    __in HV_SYNIC_SINT_INDEX SintIndex,
    __in BOOLEAN Periodic,
    __in BOOLEAN DirectMode,
    __in UINT8 Vector,
    __in_opt EFI_HV_INTERRUPT_HANDLER InterruptHandler
    )
/*++

Routine Description:

    Configures a timer for use. Start it with EfiHvSetTimer.

Arguments:

    This - A pointer to the EFI_HV_PROTOCOL instance.

    TimerIndex - The index of the timer.

    SintIndex - The SINT to deliver a message to when the timer expires.

    Periodic - TRUE if this is a periodic timer.

    DirectMode - TRUE if direct mode.

    Vector - Interrupt vector/number.

    InterruptHandler - A pointer to the interrupt handler for the timer.

Return Value:

    EFI status.

--*/
{
    HV_X64_MSR_STIMER_CONFIG_CONTENTS config;
    EFI_STATUS status;
    DEBUG((DEBUG_VERBOSE, ">>> %a: tindex 0x%x sindex 0x%x periodic %s direct %s vector 0x%x\n",
        __FUNCTION__, TimerIndex, SintIndex, Periodic ? L"TRUE" : L"FALSE",
        DirectMode ? L"TRUE" : L"FALSE", Vector));

    if (TimerIndex >= HV_SYNIC_STIMER_COUNT)
    {
        return EFI_INVALID_PARAMETER;
    }

    // Verify that an existing timer is not being reconfigured with an incompatible configuration.

    if (DirectMode)
    {
        if (mTimerConfiguration[TimerIndex].Enable)
        {
            if (!mTimerConfiguration[TimerIndex].DirectMode ||
                (mTimerConfiguration[TimerIndex].ApicVector != Vector) ||
                (mDirectTimerInterruptHandlers[Vector] != InterruptHandler))
            {
                return EFI_INVALID_PARAMETER;
            }
        }
        else
        {
            // Configure the interrupt handler for this timer.

#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)

            status = mCpu->RegisterInterruptHandler(mCpu, Vector, EfiHvDirectTimerInterruptHandler);

#elif defined(MDE_CPU_AARCH64)

            status = mHwInt->RegisterInterruptSource(mHwInt, (UINTN)Vector, EfiHvDirectTimerInterruptHandler);

#endif

            if (EFI_ERROR(status))
            {
                return status;
            }

            mDirectTimerInterruptHandlers[Vector] = InterruptHandler;
        }
    }
    else
    {
        if (mTimerConfiguration[TimerIndex].DirectMode)
        {
            return EFI_INVALID_PARAMETER;
        }
    }

    // Stop the timer if it's already running.

    EfiHvSetTimer(&mHv, TimerIndex, 0);

    // Configure the timer. Always use lazy mode if the timer is periodic.

    config.AsUINT64 = 0;
    config.Periodic = (Periodic != FALSE);
    config.Lazy = (Periodic != FALSE);
    config.AutoEnable = TRUE;
    if (DirectMode)
    {
        config.DirectMode = 1;
        config.ApicVector = Vector;
    }
    else
    {
        config.SINTx = SintIndex;
    }
    mTimerConfiguration[TimerIndex] = config;
    mTimerConfiguration[TimerIndex].Enable = 1;
    HvHypercallSetVpRegister64Self(&mHvContext, HvRegisterStimer0Config + (2 * TimerIndex), config.AsUINT64);

    DEBUG((DEBUG_VERBOSE, "<<< %a\n", __FUNCTION__));
    return EFI_SUCCESS;
}


HV_STATUS
EfiHvIssueHypercall (
    _In_ HV_CALL_CODE CallCode,
    _In_ BOOLEAN Fast,
    _In_ UINT64 FirstRegister,
    _In_ UINT64 SecondRegister
    )
/*++

Routine Description:

    Issues a hypercall.

Arguments:

    CallCode - The hypercall code.

    Fast - If TRUE, this is a fast hypercall.

    FirstRegister - The first register value for the hypercall.

    SecondRegister - The second register value for the hypercall.

Return Value:

    The hypercall status.

--*/
{
    return HvHypercallIssue(mUseBypassContext ? &mHvBypassContext : &mHvContext,
                            CallCode,
                            Fast,
                            0,
                            FirstRegister,
                            SecondRegister,
                            NULL);
}


EFI_STATUS
EfiHvConvertStatus (
    __in HV_STATUS Status
    )
/*++

Routine Description:

    Converts a hypervisor status code into an EFI status code.

Arguments:

    Status - The hypervisor status code.

Return Value:

    EFI status.

--*/
{
    switch (Status)
    {
    case HV_STATUS_SUCCESS:
        return EFI_SUCCESS;

    case HV_STATUS_INVALID_PARAMETER:
        return EFI_INVALID_PARAMETER;

    default:
        return EFI_DEVICE_ERROR;
    }
}


EFI_STATUS
EFIAPI
EfiHvPostMessage (
    __in EFI_HV_PROTOCOL *This,
    __in HV_CONNECTION_ID ConnectionId,
    __in HV_MESSAGE_TYPE MessageType,
    __in_bcount(PayloadSize) VOID *Payload,
    __in_range(0, HV_MESSAGE_PAYLOAD_BYTE_COUNT) UINT32 PayloadSize
    )
/*++

Routine Description:

    Posts a message to a hypervisor message port.

Arguments:

    This - A pointer to the EFI_HV_PROTOCOL instance.

    ConnectionId - The connection ID of the message port.

    MessageType - The type of the message.

    Payload - A pointer to the payload buffer.

    PayloadSize - The length of the payload buffer, in bytes.

Return Value:

    EFI status.

--*/
{
    PHV_INPUT_POST_MESSAGE input;
    HV_STATUS hvStatus;
    EFI_STATUS status;
    EFI_TPL oldTpl;

    DEBUG((DEBUG_VERBOSE, ">>> %a: ConnId 0x%x MessageType 0x%x Payload 0x%p Size 0x%x\n",
        __FUNCTION__, ConnectionId, MessageType, Payload, PayloadSize));

    oldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);
    input = (PHV_INPUT_POST_MESSAGE)mHvPages->HypercallInputPage;
    input->ConnectionId = ConnectionId;
    input->Reserved = 0;
    input->MessageType = MessageType;
    input->PayloadSize = PayloadSize;
    CopyMem(input->Payload, Payload, PayloadSize);
    ZeroMem((UINT8 *)input->Payload + PayloadSize,
            sizeof(input->Payload) - PayloadSize);

    hvStatus = EfiHvIssueHypercall(HvCallPostMessage,
                                   FALSE,
                                   (UINTN)input,
                                   0);

    gBS->RestoreTPL(oldTpl);
    switch (hvStatus)
    {
    default:
        status = EfiHvConvertStatus(hvStatus);
        break;

    //
    // The following status values will be returned if the message queue is full
    // or if the VM has been throttled. Convert this to EFI_NOT_READY so
    // that the caller can retry later.
    //

    case HV_STATUS_INVALID_CONNECTION_ID:
    case HV_STATUS_INSUFFICIENT_BUFFERS:
        status = EFI_NOT_READY;
    }

    DEBUG((DEBUG_VERBOSE, "<<< %a: %r\n", __FUNCTION__, status));
    return status;
}


EFI_STATUS
EFIAPI
EfiHvSignalEvent (
    __in EFI_HV_PROTOCOL *This,
    __in HV_CONNECTION_ID ConnectionId,
    __in UINT16 FlagNumber
    )
/*++

Routine Description:

    Signals a hypervisor event port.

Arguments:

    This - A pointer to the EFI_HV_PROTOCOL instance.

    ConnectionId - The connection ID of the port.

    FlagNumber - The flag number offset.

Return Value:

    EFI status.

--*/
{
    HV_STATUS hvStatus;
    PHV_INPUT_SIGNAL_EVENT input;
    UINT64 registers[2];

    ZeroMem(registers, sizeof(registers));

    DEBUG((DEBUG_VERBOSE, ">>> %a: ConnectionId 0x%x FlagNumber 0x%x\n", __FUNCTION__,
        ConnectionId, FlagNumber));

    input = (PHV_INPUT_SIGNAL_EVENT)registers;
    input->ConnectionId = ConnectionId;
    input->FlagNumber = FlagNumber;
    input->RsvdZ = 0;
    hvStatus = EfiHvIssueHypercall(HvCallSignalEvent,
                                   TRUE,
                                   registers[0],
                                   registers[1]);

    DEBUG((DEBUG_VERBOSE, "<<< %a: status %r\n", __FUNCTION__, EfiHvConvertStatus(hvStatus)));
    return EfiHvConvertStatus(hvStatus);
}


EFI_STATUS
EFIAPI
EfiHvpModifySparseGpaPageHostVisibility(
    _In_ HV_MAP_GPA_FLAGS MapFlags,
    _In_ UINT32 PageCount,
    _In_ HV_GPA_PAGE_NUMBER GpaPageBase,
    _Out_ UINT32* PageCountProcessed
    )
{
    // For this rep call, it's easier to treat the input page as a pointer
    // to this structure.
    PHV_INPUT_MODIFY_SPARSE_GPA_PAGE_HOST_VISIBILITY pInputBuffer;
    HV_STATUS hvStatus;
    EFI_STATUS status;
    EFI_TPL oldTpl;
    UINT32 possibleRepsPerCall;
    UINT32 repsInCurrentCall;
    UINT32 repsProcessedThisCall;
    UINT32 gpaPageBaseIndex = 0;
    UINT32 i;

    DEBUG((DEBUG_VERBOSE,
        ">>> %a: GpaBase 0x%p PageCount 0x%x MapFlags 0x%x \n",
        __FUNCTION__,
        GpaPageBase,
        PageCount,
        MapFlags));

    *PageCountProcessed = 0;

    oldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

    //
    // Simplified version of WinHvpSpecialListRepHypercall with no output parameters
    //

    possibleRepsPerCall = (HV_PAGE_SIZE - sizeof(*pInputBuffer)) / sizeof(HV_GPA_PAGE_NUMBER);

    ASSERT(possibleRepsPerCall <= WINHVP_MAX_REPS_PER_HYPERCALL);

    pInputBuffer = (PHV_INPUT_MODIFY_SPARSE_GPA_PAGE_HOST_VISIBILITY)mHvPages->HypercallInputPage;

    for (;;)
    {
        ASSERT(PageCount > 0);

        repsProcessedThisCall = 0;

        ZeroMem(pInputBuffer, HV_PAGE_SIZE);

        //
        // Build the input.
        //
        repsInCurrentCall = MIN(possibleRepsPerCall, PageCount);

        ASSERT(repsInCurrentCall <= WINHVP_MAX_REPS_PER_HYPERCALL);

        // Fill header
        pInputBuffer->TargetPartitionId = HV_PARTITION_ID_SELF;
        pInputBuffer->HostVisibility = MapFlags;

        //
        // Fill page numbers
        // N.B. instead of copying from an existing list of page numbers, we
        // generate a list of consecutive numbers from GpaPageBase.
        //

        for (i = 0; i < repsInCurrentCall; i++, gpaPageBaseIndex++)
        {
            pInputBuffer->GpaPageList[i] = GpaPageBase + gpaPageBaseIndex;
        }

        //
        // Call the hypervisor.
        //

        hvStatus = HvHypercallIssue(&mHvContext,
                                    HvCallModifySparseGpaPageHostVisibility,
                                    FALSE, // not fast
                                    repsInCurrentCall,
                                    (UINTN)pInputBuffer,
                                    0, // no output
                                    &repsProcessedThisCall);
        status = EfiHvConvertStatus(hvStatus);

        ASSERT(repsProcessedThisCall <= repsInCurrentCall);
        ASSERT(repsProcessedThisCall <= PageCount);

        ASSERT(((repsProcessedThisCall == repsInCurrentCall) &&
                (status == EFI_SUCCESS)) ||
               (status != EFI_SUCCESS));

        //
        // Update the count of reps processed.
        //
        *PageCountProcessed += repsProcessedThisCall;

        //
        // Check that we haven't overflowed.
        //

        if (repsProcessedThisCall > PageCount)
        {
            status = EFI_BAD_BUFFER_SIZE;
        }

        PageCount -= repsProcessedThisCall;

        if ((status != EFI_SUCCESS) || (PageCount == 0))
        {
            break;
        }
    }

    gBS->RestoreTPL(oldTpl);

    DEBUG((DEBUG_VERBOSE, "<<< %a: %r\n", __FUNCTION__, status));

    return status;
}


EFI_STATUS
EFIAPI
EfiHvMakeAddressRangeHostVisible(
    _In_ EFI_HV_IVM_PROTOCOL *This,
    _In_ HV_MAP_GPA_FLAGS MapFlags,
    _In_ VOID *BaseAddress,
    _In_ UINT32 ByteCount,
    _In_ BOOLEAN ZeroPages,
    _Out_ EFI_HV_PROTECTION_HANDLE *ProtectionHandle
    )
{
    UINT32 isolationType;
    UINT32 pageCountProcessed;
    EFI_HV_PROTECTION_OBJECT *protectionObject;
    EFI_STATUS revertStatus;
    EFI_STATUS status;

    //
    // Visibility changes are only permitted on isolated systems.
    //

    isolationType = PcdGet32(PcdIsolationArchitecture);
    if (isolationType == UefiIsolationTypeNone)
    {
        return EFI_INVALID_PARAMETER;
    }

    //
    // All arguments must be page aligned, and the access must imply host
    // visibility.
    //

    if ((((UINTN)BaseAddress & (EFI_PAGE_SIZE - 1)) != 0) ||
        ((ByteCount & (EFI_PAGE_SIZE - 1)) != 0) ||
        ((MapFlags & HV_MAP_GPA_READABLE) == 0) ||
        ((MapFlags & ~(HV_MAP_GPA_READABLE | HV_MAP_GPA_WRITABLE)) != 0))
    {
        return EFI_INVALID_PARAMETER;
    }

    //
    // Allocate memory to use as a tracking object.
    //

    protectionObject = AllocatePool(sizeof(*protectionObject));
    if (protectionObject == NULL)
    {
        return EFI_OUT_OF_RESOURCES;
    }

    protectionObject->GpaPageBase = (UINTN)BaseAddress / EFI_PAGE_SIZE;
    protectionObject->NumberOfPages = ByteCount / EFI_PAGE_SIZE;

    //
    // If this is a software-isolated VM, then memory must be zeroed before it
    // is made visible to the host, since page contents will remain intact
    // following the visibility change.  For a hardware-isolated VM, memory
    // encryption differences will obscure the original contents following the
    // visibility change.
    //

    if (ZeroPages || (isolationType == UefiIsolationTypeVbs))
    {
        ZeroMem(BaseAddress, ByteCount);
        ZeroPages = FALSE;
    }

    //
    // Update the visibility as requested.
    //

    status = EfiHvpModifySparseGpaPageHostVisibility(MapFlags,
                                                     protectionObject->NumberOfPages,
                                                     protectionObject->GpaPageBase,
                                                     &pageCountProcessed);

    if (EFI_ERROR(status))
    {
        //
        // If the protection change was partially made, then undo whatever
        // was done.
        //

        if (pageCountProcessed != 0)
        {
            revertStatus = EfiHvpModifySparseGpaPageHostVisibility(HV_MAP_GPA_PERMISSIONS_NONE,
                                                                   pageCountProcessed,
                                                                   protectionObject->GpaPageBase,
                                                                   &pageCountProcessed);
            if (EFI_ERROR(revertStatus))
            {
                // this is not allowed to fail - need to crash here.
                ASSERT(FALSE);
                CpuDeadLoop();
            }
        }

        FreePool(protectionObject);
    }
    else
    {
        InsertTailList(&mHostVisiblePageList, &protectionObject->ListEntry);

        //
        // If zeroing was requested and has not already been performed, then
        // zero the buffer now.
        //

        if (ZeroPages)
        {
            ZeroMem((PVOID)((UINTN)BaseAddress + mSharedGpaBoundary), ByteCount);
        }

        *ProtectionHandle = protectionObject;
    }

    return status;
}


VOID
EFIAPI
EfiHvMakeAddressRangeNotHostVisible(
    _In_ EFI_HV_IVM_PROTOCOL *This,
    _In_ EFI_HV_PROTECTION_HANDLE ProtectionHandle
    )
{
    UINT32 pageCountProcessed;
    EFI_STATUS status;

    RemoveEntryList(&ProtectionHandle->ListEntry);

    status = EfiHvpModifySparseGpaPageHostVisibility(HV_MAP_GPA_PERMISSIONS_NONE,
                                                     ProtectionHandle->NumberOfPages,
                                                     ProtectionHandle->GpaPageBase,
                                                     &pageCountProcessed);
    if (EFI_ERROR(status))
    {
        // this is not allowed to fail - need to crash here.
        ASSERT(FALSE);
    }
}


EFI_STATUS
EfiHvConnectToHypervisor (
    VOID
    )
/*++

Routine Description:

    Initializes a connection to the hypervisor.

Arguments:

    None.

Return Value:

    EFI status.

--*/
{
    EFI_STATUS status;

    DEBUG((DEBUG_VERBOSE, ">>> %a\n", __FUNCTION__));

#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)

    HV_CPUID_RESULT cpuidResult;

    // Validate that the hypervisor is present, is a Microsoft hypervisor,
    // and has all the required features.

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionVersionAndFeatures);
    if (!cpuidResult.VersionAndFeatures.HypervisorPresent)
    {
        DEBUG((DEBUG_VERBOSE, "--- %a: No Hypervisor Present!\n", __FUNCTION__));
        status = EFI_UNSUPPORTED;
        goto Exit;
    }

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionHvInterface);
    if (cpuidResult.HvInterface.Interface != HvMicrosoftHypervisorInterface)
    {
        DEBUG((DEBUG_VERBOSE, "--- %a: Not Microsoft Hypervisor!\n", __FUNCTION__));
        status = EFI_UNSUPPORTED;
        goto Exit;
    }

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionMsHvFeatures);
    if (!(cpuidResult.MsHvFeatures.PartitionPrivileges.AccessPartitionReferenceCounter &&
          cpuidResult.MsHvFeatures.PartitionPrivileges.AccessSynicRegs &&
          cpuidResult.MsHvFeatures.PartitionPrivileges.AccessSyntheticTimerRegs &&
          cpuidResult.MsHvFeatures.PartitionPrivileges.AccessHypercallMsrs))
    {
        DEBUG((DEBUG_VERBOSE, "--- %a: Missing Hypervisor features!\n", __FUNCTION__));
        status = EFI_UNSUPPORTED;
        goto Exit;
    }

    if (cpuidResult.MsHvFeatures.DirectSyntheticTimers)
    {
        mDirectTimerSupported = TRUE;
    }

    if (cpuidResult.MsHvFeatures.PartitionPrivileges.Isolation)
    {
        DEBUG((EFI_D_INFO, "--- %a: Partition is Isolated\n", __FUNCTION__));
    }

    // Allocate hypervisor communication pages.

    mHvPages = AllocatePages(sizeof(*mHvPages) / EFI_PAGE_SIZE);
    if (mHvPages == NULL)
    {
        DEBUG((DEBUG_VERBOSE, "--- %a: Failed to allocate hypercall pages!\n", __FUNCTION__));
        status =  EFI_OUT_OF_RESOURCES;
        goto Exit;
    }
    DEBUG((DEBUG_VERBOSE, "--- %a: pages @ 0x%p\n", __FUNCTION__, (UINTN)mHvPages));

    // Zero the hypercall page
    ZeroMem(mHvPages, sizeof(*mHvPages));

    HvHypercallConnect(mHvPages->HypercallPage, NULL, &mHvContext);

    // Check to see if the hypercall page was mapped. If it wasn't, abort here.
    if (mHvPages->HypercallPage[0] == 0 &&
        mHvPages->HypercallPage[1] == 0 &&
        mHvPages->HypercallPage[2] == 0)
    {
        // TODO-cho-19259739: Figure out a better way to abort.
        __ud2();
    }

    // Cache some enlightenment information.

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionMsHvEnlightenmentInformation);
    mAutoEoi = !cpuidResult.MsHvEnlightenmentInformation.DeprecateAutoEoi;
    DEBUG((DEBUG_VERBOSE, "--- %a: mAutoEoi 0x%x\n", __FUNCTION__, mAutoEoi));


#elif defined(MDE_CPU_AARCH64)

    // Direct timers are always supported on ARM64.

    mDirectTimerSupported = TRUE;

    //
    // Allocate hypervisor communication pages.
    //
    mHvPages = AllocatePages(sizeof(*mHvPages) / EFI_PAGE_SIZE);
    if (mHvPages == NULL)
    {
        status =  EFI_OUT_OF_RESOURCES;
        goto Exit;
    }

    HvHypercallConnect(&mHvContext);

    // AutoEoi is not possible on ARM.

    mAutoEoi = FALSE;

#else
#error Unsupported architecture
#endif

    mSharedGpaBoundary = PcdGet64(PcdIsolationSharedGpaBoundary);

#if defined(MDE_CPU_X64)

    // Determine whether this system uses a hardware isolation architecture
    // that will require a direct connection to the hypervisor that bypasses
    // the paravisor.

    if (PcdGet32(PcdIsolationArchitecture) == UefiIsolationTypeSnp)
    {
        ASSERT(PcdGetBool(PcdIsolationParavisorPresent) != FALSE);
        ASSERT(mSharedGpaBoundary != 0);

        //
        // Allocate a page to use as the hypercall output page.
        //

        mOutputPageBypass = AllocatePages(1);
        if (mOutputPageBypass == NULL)
        {
            status = EFI_OUT_OF_RESOURCES;
            goto Exit;
        }

        //
        // Make this page visible to the hypervisor.  Zero its contents first
        // to prevent any data leakage.
        //

        ZeroMem(mOutputPageBypass, EFI_PAGE_SIZE);
        status = EfiHvpModifySparseGpaPageHostVisibility(
            HV_MAP_GPA_READABLE | HV_MAP_GPA_WRITABLE,
            1,
            (UINTN)mOutputPageBypass / EFI_PAGE_SIZE,
            NULL);

        if (EFI_ERROR(status))
        {
            goto Exit;
        }

        HvHypercallConnect(NULL,
                           (PVOID)((UINTN)mOutputPageBypass + mSharedGpaBoundary),
                           &mHvBypassContext);

        mUseBypassContext = TRUE;
    }

#endif

    status = EFI_SUCCESS;

Exit:

    DEBUG((DEBUG_VERBOSE, "<<< %a: %r\n", __FUNCTION__, status));

    return status;
}


VOID
EfiHvDisconnectFromHypervisor (
    VOID
    )
/*++

Routine Description:

    Tears down a connection to the hypervisor.

Arguments:

    None.

Return Value:

    None.

--*/
{
    LIST_ENTRY *entry;
    EFI_HV_PROTECTION_OBJECT *protectionObject;
    EFI_STATUS status;

    if (mUseBypassContext)
    {
        HvHypercallDisconnect(&mHvBypassContext);
    }

    // Revoke host visibility for any pages that were made visible.

    while (!IsListEmpty(&mHostVisiblePageList))
    {
        entry = GetFirstNode(&mHostVisiblePageList);
        protectionObject = BASE_CR(entry, EFI_HV_PROTECTION_OBJECT, ListEntry);
        EfiHvMakeAddressRangeNotHostVisible(NULL, protectionObject);
    }

    // Free the bypass output page if required.

    if (mOutputPageBypass != NULL)
    {
        status = EfiHvpModifySparseGpaPageHostVisibility(
            HV_MAP_GPA_PERMISSIONS_NONE,
            1,
            (UINTN)mOutputPageBypass / EFI_PAGE_SIZE,
            NULL);

        if (EFI_ERROR(status))
        {
            // Failure is not allowed here - need to crash
            ASSERT(FALSE);
            CpuDeadLoop();
        }

        FreePages(mOutputPageBypass, 1);
    }

    HvHypercallDisconnect(&mHvContext);

    // Free the hypercall communication pages.

    if (mHvPages != NULL)
    {
        FreePages(mHvPages, sizeof(*mHvPages) / EFI_PAGE_SIZE);
        mHvPages = NULL;
    }
}


EFI_STATUS
EfiHvConnectToSynic (
    VOID
    )
/*++

Routine Description:

    Initializes a connection to the synthetic interrupt controller.

Arguments:

    None.

Return Value:

    EFI status.

--*/
{
    HV_HYPERCALL_CONTEXT *context;
    HV_SYNIC_SIEFP siefp;
    HV_SYNIC_SIMP simp;

    DEBUG((DEBUG_VERBOSE, ">>> %a\n", __FUNCTION__));

    context = mUseBypassContext ? &mHvBypassContext : &mHvContext;

    // Enable the message page.

    simp.AsUINT64 = HvHypercallGetVpRegister64Self(context, HvRegisterSipp);
    if (simp.SimpEnabled != 0)
    {
        mMessagePage = (PHV_MESSAGE_PAGE)(simp.BaseSimpGpa * EFI_PAGE_SIZE);
        if ((UINTN)mMessagePage < mSharedGpaBoundary)
        {
            // Failure is not allowed here - need to crash
            ASSERT(FALSE);
            CpuDeadLoop();
        }
    }
    else
    {
        ASSERT(mUseBypassContext == FALSE);
        mMessagePage = &mHvPages->MessagePage;
        simp.SimpEnabled = 1;
        simp.BaseSimpGpa = (UINTN)mMessagePage / EFI_PAGE_SIZE;
        HvHypercallSetVpRegister64Self(context, HvRegisterSipp, simp.AsUINT64);
    }

    // Enable the event page.

    siefp.AsUINT64 = HvHypercallGetVpRegister64Self(context, HvRegisterSifp);
    if (siefp.SiefpEnabled != 0)
    {
        mEventFlagsPage = (PHV_SYNIC_EVENT_FLAGS_PAGE)(siefp.BaseSiefpGpa * EFI_PAGE_SIZE);
        if ((UINTN)mEventFlagsPage < mSharedGpaBoundary)
        {
            // Failure is not allowed here - need to crash
            ASSERT(FALSE);
            CpuDeadLoop();
        }
    }
    else
    {
        ASSERT(mUseBypassContext == FALSE);
        mEventFlagsPage = &mHvPages->EventFlagsPage;
        siefp.SiefpEnabled = 1;
        siefp.BaseSiefpGpa = (UINTN)mEventFlagsPage / EFI_PAGE_SIZE;
        HvHypercallSetVpRegister64Self(context, HvRegisterSifp, siefp.AsUINT64);
    }

    mSynicConnected = TRUE;

    DEBUG((DEBUG_VERBOSE, "<<< %a: %r\n", __FUNCTION__, EFI_SUCCESS));
    return EFI_SUCCESS;
}


VOID
EfiHvDisconnectFromSynic (
    VOID
    )
/*++

Routine Description:

    Tears down the connection to the synthetic interrupt controller.

Arguments:

    None.

Return Value:

    None.

--*/
{
    HV_SYNIC_SIEFP siefp;
    HV_SYNIC_SIMP simp;
    HV_SYNIC_SINT_INDEX sintIndex;
    UINT32 timerIndex;
    UINT32 flagsIndex = 0;

    if (!mSynicConnected)
    {
        return;
    }

    // Clear all the timers.

    for (timerIndex = 0; timerIndex < HV_SYNIC_STIMER_COUNT; timerIndex += 1)
    {
        HvHypercallSetVpRegister64Self(&mHvContext, HvRegisterStimer0Count + (2 * timerIndex), 0);
        HvHypercallSetVpRegister64Self(&mHvContext, HvRegisterStimer0Config + (2 * timerIndex), 0);
    }

    // Disconnect the SINTs and drain all the message queues.

    for (sintIndex = 0; sintIndex < HV_SYNIC_SINT_COUNT; sintIndex += 1)
    {
        EfiHvDisconnectSint(&mHv, sintIndex);
        while (EfiHvGetSintMessage(&mHv, sintIndex) != NULL)
        {
            EfiHvCompleteSintMessage(&mHv, sintIndex);
        }

        // Zero the event flags for this SINT.

        volatile HV_SYNIC_EVENT_FLAGS* flags = EfiHvGetSintEventFlags(&mHv, sintIndex);

        for (flagsIndex = 0; flagsIndex < HV_EVENT_FLAGS_DWORD_COUNT; flagsIndex++)
        {
            flags->Flags32[flagsIndex] = 0;
        }
    }

    if (mUseBypassContext == FALSE)
    {
        // Disable the message page.

        simp.AsUINT64 = HvHypercallGetVpRegister64Self(&mHvContext, HvRegisterSipp);
        simp.SimpEnabled = 0;
        simp.BaseSimpGpa = 0;
        HvHypercallSetVpRegister64Self(&mHvContext, HvRegisterSipp, simp.AsUINT64);

        // Disable the event page.

        siefp.AsUINT64 = HvHypercallGetVpRegister64Self(&mHvContext, HvRegisterSifp);
        siefp.SiefpEnabled = 0;
        siefp.BaseSiefpGpa = 0;
        HvHypercallSetVpRegister64Self(&mHvContext, HvRegisterSifp, siefp.AsUINT64);
    }

    mSynicConnected = FALSE;
}


VOID
EFIAPI
EfiHvExitBootServices (
    __in EFI_EVENT Event,
    __in VOID *Context
    )
/*++

Routine Description:

    Called when ExitBootServices() is called. Tears down the hypervisor
    connection so that the new OS sees a clean state.

Arguments:

    Event - An EFI event.

    Context - A pointer to the context.

Return Value:

    None.

--*/
{
    EfiHvDisconnectFromSynic();
    EfiHvDisconnectFromHypervisor();
}


EFI_HV_PROTOCOL mHv =
{
    EfiHvConnectSint,
    EfiHvConnectSintToEvent,
    EfiHvDisconnectSint,
    EfiHvGetSintMessage,
    EfiHvCompleteSintMessage,
    EfiHvGetSintEventFlags,
    EfiHvGetReferenceTime,
    EfiHvGetCurrentVpIndex,
    EfiHvDirectTimerSupported,
    EfiHvConfigureTimer,
    EfiHvSetTimer,
    EfiHvPostMessage,
    EfiHvSignalEvent
};

EFI_HV_IVM_PROTOCOL mHvIvm =
{
    EfiHvMakeAddressRangeHostVisible,
    EfiHvMakeAddressRangeNotHostVisible
};


EFI_STATUS
EFIAPI
EfiHvInitialize (
    __in EFI_HANDLE ImageHandle,
    __in EFI_SYSTEM_TABLE *SystemTable
    )
/*++

Routine Description:

    Entrypoint. Initializes the EfiHv driver.

Arguments:

    ImageHandle - The handle of the loaded image.

    SystemTable - A pointer to the system table.

Return Value:

    EFI status.

--*/
{
    EFI_STATUS status;
    DEBUG((DEBUG_VERBOSE, ">>> %a\n", __FUNCTION__));

    InitializeListHead(&mHostVisiblePageList);

#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)

    // For Intel find the CPU protocol.

    status = gBS->LocateProtocol(&gEfiCpuArchProtocolGuid, NULL, (VOID **)&mCpu);


#elif defined(MDE_CPU_AARCH64)

    // For ARM find the hardware interrupt protocol.

    status = gBS->LocateProtocol(&gHardwareInterruptProtocolGuid, NULL, (VOID **)&mHwInt);

#endif

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }
    DEBUG((DEBUG_VERBOSE, "--- %a: after LocateProtocol\n", __FUNCTION__));

    // Register notify function for EVT_SIGNAL_EXIT_BOOT_SERVICES.

    status = gBS->CreateEventEx(EVT_NOTIFY_SIGNAL,
                                TPL_CALLBACK,
                                EfiHvExitBootServices,
                                NULL,
                                &gEfiEventExitBootServicesGuid,
                                &mExitBootServicesEvent);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }
    DEBUG((DEBUG_VERBOSE, "--- %a: after CreateEventEx\n", __FUNCTION__));

    // Connect to the hypervisor and synic.

    status = EfiHvConnectToHypervisor();
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }
    DEBUG((DEBUG_VERBOSE, "--- %a: after EfiHvConnectToHypervisor\n", __FUNCTION__));

    status = EfiHvConnectToSynic();
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }
    DEBUG((DEBUG_VERBOSE, "--- %a: after EfiHvConnectToSynic\n", __FUNCTION__));

    // Register the HV protocols.

    status = gBS->InstallMultipleProtocolInterfaces(
                    &mHvHandle,
                    &gEfiHvProtocolGuid, &mHv,
                    &gEfiHvIvmProtocolGuid, &mHvIvm,
                    NULL);

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }
    DEBUG((DEBUG_VERBOSE, "--- %a: after InstallMultipleProtocolInterfaces\n", __FUNCTION__));

Cleanup:
    if (EFI_ERROR(status))
    {
        if (mExitBootServicesEvent != NULL)
        {
            gBS->CloseEvent(mExitBootServicesEvent);
            mExitBootServicesEvent = NULL;
        }

        EfiHvDisconnectFromSynic();
        EfiHvDisconnectFromHypervisor();
    }
    DEBUG((DEBUG_VERBOSE, "<<< %a: %r\n", __FUNCTION__, EFI_SUCCESS));
    return EFI_SUCCESS;
}

