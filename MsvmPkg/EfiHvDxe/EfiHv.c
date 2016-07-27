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

#include <Protocol/Cpu.h>
#include <Protocol/EfiHv.h>

#include <Guid/EventGroup.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/LocalApicLib.h>

typedef struct _EFI_HV_SINT_CONFIGURATION
{
    EFI_HV_INTERRUPT_HANDLER InterruptHandler;
    VOID *Context;
    UINT8 Vector;
} EFI_HV_SINT_CONFIGURATION, *PEFI_HV_SINT_CONFIGURATION;

typedef struct _EFI_HV_PAGES
{
    UCHAR HypercallPage[EFI_PAGE_SIZE];
    UCHAR HypercallInputPage[EFI_PAGE_SIZE];
    UCHAR HypercallOutputPage[EFI_PAGE_SIZE];
    HV_SYNIC_EVENT_FLAGS_PAGE EventFlagsPage;
    HV_MESSAGE_PAGE MessagePage;
} EFI_HV_PAGES, *PEFI_HV_PAGES;

PEFI_HV_PAGES mHvPages;
EFI_HANDLE mHvHandle;
BOOLEAN mHypervisorConnected;
BOOLEAN mSynicConnected;
EFI_EVENT mExitBootServicesEvent;
BOOLEAN mAutoEoi;

EFI_HV_SINT_CONFIGURATION mSintConfiguration[HV_SYNIC_SINT_COUNT];
UINT8 mVectorSint[256];

EFI_CPU_ARCH_PROTOCOL *mCpu;
extern EFI_HV_PROTOCOL mHv;

VOID
EFIAPI
EfiHvInterruptHandler (
    __in EFI_EXCEPTION_TYPE InterruptType,
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

    tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);
    if (!mAutoEoi)
    {
        SendApicEoi();
    }

    sintConfiguration = &mSintConfiguration[mVectorSint[InterruptType]];
    if (sintConfiguration->InterruptHandler != NULL)
    {
        sintConfiguration->InterruptHandler(sintConfiguration->Context);
    }

    gBS->RestoreTPL(tpl);
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

    //
    // Disable interrupts while manipulating IDT state.
    //

    tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

    //
    // Ensure the SINT is not already registered.
    //

    sintConfiguration = &mSintConfiguration[SintIndex];
    if (sintConfiguration->Vector != 0)
    {
        return EFI_ALREADY_STARTED;
    }

    //
    // Register the interrupt handler.
    //

    status = mCpu->RegisterInterruptHandler(mCpu,
                                            Vector,
                                            EfiHvInterruptHandler);

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    //
    // Register the SINT with the hypervisor.
    //

    sint.AsUINT64 = 0;
    sint.Vector = Vector;
    sint.Masked = FALSE;
    sint.AutoEoi = mAutoEoi;
    AsmWriteMsr64(HV_X64_MSR_SINT0 + SintIndex, sint.AsUINT64);

    //
    // Store the state used by the interrupt handler.
    //

    sintConfiguration->InterruptHandler = InterruptHandler;
    sintConfiguration->Context = Context;
    sintConfiguration->Vector = Vector;
    mVectorSint[Vector] = (UINT8)SintIndex;
    status = EFI_SUCCESS;

Cleanup:
    gBS->RestoreTPL(tpl);
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

    event = Context;
    gBS->SignalEvent(event);
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
    return EfiHvConnectSint(This,
                            SintIndex,
                            Vector,
                            EfiHvEventInterruptHandler,
                            Event);
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

    tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);
    sintConfiguration = &mSintConfiguration[SintIndex];

    //
    // Unregister the SINT with the hypervisor.
    //

    sint.AsUINT64 = 0;
    sint.Masked = 1;
    AsmWriteMsr64(HV_X64_MSR_SINT0 + SintIndex, sint.AsUINT64);

    //
    // Unregister the interrupt handler.
    //

    if (sintConfiguration->Vector != 0)
    {
        mCpu->RegisterInterruptHandler(mCpu, sintConfiguration->Vector, NULL);
        mVectorSint[sintConfiguration->Vector] = 0;
    }

    sintConfiguration->Vector = 0;
    sintConfiguration->InterruptHandler = NULL;
    sintConfiguration->Context = NULL;
    gBS->RestoreTPL(tpl);
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

    message = &mHvPages->MessagePage.SintMessage[SintIndex];
    if (message->Header.MessageType == HvMessageTypeNone)
    {
        return NULL;
    }

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

    message = &mHvPages->MessagePage.SintMessage[SintIndex];
    message->Header.MessageType = HvMessageTypeNone;
    MemoryBarrier();
    if (message->Header.MessageFlags.MessagePending)
    {
        AsmWriteMsr64(HV_X64_MSR_EOM, 0);
    }
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
    return &mHvPages->EventFlagsPage.SintEventFlags[SintIndex];
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
    return AsmReadMsr64(HV_X64_MSR_TIME_REF_COUNT);
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
    return (UINT32)AsmReadMsr64(HV_X64_MSR_VP_INDEX);
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
    AsmWriteMsr64(HV_X64_MSR_STIMER0_COUNT + 2 * TimerIndex, Expiration);
}


EFI_STATUS
EFIAPI
EfiHvConfigureTimer (
    __in EFI_HV_PROTOCOL *This,
    __in UINT32 TimerIndex,
    __in HV_SYNIC_SINT_INDEX SintIndex,
    __in BOOLEAN Periodic
    )
/*++

Routine Description:

    Configures a timer for use. Start it with EfiHvSetTimer.

Arguments:

    This - A pointer to the EFI_HV_PROTOCOL instance.

    TimerIndex - The index of the timer.

    SintIndex - The SINT to deliver a message to when the timer expires.

    Periodic - TRUE if this is a periodic timer.

Return Value:

    EFI status.

--*/
{
    HV_X64_MSR_STIMER_CONFIG_CONTENTS config;

    //
    // Stop the timer if it's already running.
    //

    EfiHvSetTimer(&mHv, TimerIndex, 0);

    //
    // Configure the timer. Always use lazy mode if the timer is periodic.
    //

    config.AsUINT64 = 0;
    config.Periodic = (Periodic != FALSE);
    config.Lazy = (Periodic != FALSE);
    config.AutoEnable = TRUE;
    config.SINTx = SintIndex;
    AsmWriteMsr64(HV_X64_MSR_STIMER0_CONFIG + 2 * TimerIndex, config.AsUINT64);
    return EFI_SUCCESS;
}


HV_STATUS
EfiHvIssueHypercall (
    __in HV_CALL_CODE CallCode,
    __in BOOLEAN Fast,
    __in UINT64 FirstRegister,
    __in UINT64 SecondRegister
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
    HV_X64_HYPERCALL_INPUT callInput;
    HV_X64_HYPERCALL_OUTPUT callOutput;

    callInput.AsUINT64 = 0;
    callInput.CallCode = CallCode;
    callInput.IsFast = (Fast != FALSE);

#if defined(MDE_CPU_X64)

    {
        typedef HV_X64_HYPERCALL_OUTPUT HYPERCALL_ROUTINE(
            __in HV_X64_HYPERCALL_INPUT Control,
            __in UINT64                 InputPhysicalAddress,
            __in UINT64                 OutputPhysicalAddress
            );

#pragma warning(disable: 4055)

        HYPERCALL_ROUTINE* hypercallRoutine =
            (HYPERCALL_ROUTINE *)mHvPages->HypercallPage;

        callOutput = hypercallRoutine(callInput,
                                      FirstRegister,
                                      SecondRegister);
    }

#elif defined(MDE_CPU_IA32)

    {
        ULARGE_INTEGER packed;
        ULARGE_INTEGER first;
        ULARGE_INTEGER second;

        packed.QuadPart = callInput.AsUINT64;
        first.QuadPart = FirstRegister;
        second.QuadPart = SecondRegister;

        __asm
        {
            mov edi, second.HighPart;
            mov esi, second.LowPart;
            mov ebx, first.HighPart;
            mov ecx, first.LowPart;
            mov edx, packed.HighPart;
            mov eax, packed.LowPart;

            call mHvPages->HypercallPage;

            mov packed.LowPart, eax;
            mov packed.HighPart, edx;
        }

        callOutput.AsUINT64 = packed.QuadPart;
    }

#else
#error Unsupported architecture
#endif

    return callOutput.CallStatus;
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
    EFI_TPL oldTpl;

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
        return EfiHvConvertStatus(hvStatus);

    //
    // The following statuses will be returned if the message queue is full
    // or if the VM has been throttled. Convert this to EFI_NOT_READY so
    // that the caller can retry later.
    //

    case HV_STATUS_INVALID_CONNECTION_ID:
    case HV_STATUS_INSUFFICIENT_BUFFERS:
        return EFI_NOT_READY;
    }
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
    UINT64 registers[2] = {0};

    input = (PHV_INPUT_SIGNAL_EVENT)registers;
    input->ConnectionId = ConnectionId;
    input->FlagNumber = FlagNumber;
    input->RsvdZ = 0;
    hvStatus = EfiHvIssueHypercall(HvCallSignalEvent,
                                   TRUE,
                                   registers[0],
                                   registers[1]);

    return EfiHvConvertStatus(hvStatus);
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
    HV_CPUID_RESULT cpuidResult;
    HV_X64_MSR_GUEST_OS_ID_CONTENTS guestOsIdMsr;
    HV_X64_MSR_HYPERCALL_CONTENTS hypercallMsr;

    //
    // Validate that the hypervisor is present, is a Microsoft hypervisor,
    // and has all the required features.
    //

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionVersionAndFeatures);
    if (!cpuidResult.VersionAndFeatures.HypervisorPresent)
    {
        return EFI_UNSUPPORTED;
    }

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionHvInterface);
    if (cpuidResult.HvInterface.Interface != HvMicrosoftHypervisorInterface)
    {
        return EFI_UNSUPPORTED;
    }

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionMsHvFeatures);
    if (!(cpuidResult.MsHvFeatures.PartitionPrivileges.AccessPartitionReferenceCounter &&
          cpuidResult.MsHvFeatures.PartitionPrivileges.AccessSynicMsrs &&
          cpuidResult.MsHvFeatures.PartitionPrivileges.AccessSyntheticTimerMsrs &&
          cpuidResult.MsHvFeatures.PartitionPrivileges.AccessHypercallMsrs))
    {
        return EFI_UNSUPPORTED;
    }

    //
    // Allocate pages to communicate with the hypervisor.
    //

    mHvPages = AllocatePages(sizeof(*mHvPages) / EFI_PAGE_SIZE);
    if (mHvPages == NULL)
    {
        return EFI_OUT_OF_RESOURCES;
    }

    //
    // Set the guest ID.
    //

    guestOsIdMsr.AsUINT64 = 0;
    guestOsIdMsr.OsId = 1;
    AsmWriteMsr64(HV_X64_MSR_GUEST_OS_ID, guestOsIdMsr.AsUINT64);

    //
    // Initialize the hypercall page.
    //

    hypercallMsr.AsUINT64 = AsmReadMsr64(HV_X64_MSR_HYPERCALL);

    ASSERT(hypercallMsr.Enable == 0);

    hypercallMsr.Enable = 1;
    hypercallMsr.GpaPageNumber = (UINTN)mHvPages->HypercallPage / EFI_PAGE_SIZE;
    AsmWriteMsr64(HV_X64_MSR_HYPERCALL, hypercallMsr.AsUINT64);

    //
    // Cache some enlightenment information.
    //

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionMsHvEnlightenmentInformation);
    mAutoEoi = !cpuidResult.MsHvEnlightenmentInformation.DeprecateAutoEoi;

    mHypervisorConnected = TRUE;
    return EFI_SUCCESS;
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
    HV_X64_MSR_HYPERCALL_CONTENTS hypercallMsr;

    //
    // Disable the hypercall page.
    //

    if (mHypervisorConnected)
    {
        hypercallMsr.AsUINT64 = AsmReadMsr64(HV_X64_MSR_HYPERCALL);
        hypercallMsr.Enable = 0;
        hypercallMsr.GpaPageNumber = 0;
        AsmWriteMsr64(HV_X64_MSR_HYPERCALL, hypercallMsr.AsUINT64);

        AsmWriteMsr64(HV_X64_MSR_GUEST_OS_ID, 0);
        mHypervisorConnected = FALSE;
    }

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
    HV_SYNIC_SIEFP siefp;
    HV_SYNIC_SIMP simp;

    //
    // Enable the message page.
    //

    simp.AsUINT64 = AsmReadMsr64(HV_X64_MSR_SIMP);

    ASSERT(simp.SimpEnabled == 0);

    simp.SimpEnabled = 1;
    simp.BaseSimpGpa = (UINTN)&mHvPages->MessagePage / EFI_PAGE_SIZE;
    AsmWriteMsr64(HV_X64_MSR_SIMP, simp.AsUINT64);

    //
    // Enable the event page.
    //

    siefp.AsUINT64 = AsmReadMsr64(HV_X64_MSR_SIEFP);

    ASSERT(siefp.SiefpEnabled == 0);

    siefp.SiefpEnabled = 1;
    siefp.BaseSiefpGpa = (UINTN)&mHvPages->EventFlagsPage / EFI_PAGE_SIZE;
    AsmWriteMsr64(HV_X64_MSR_SIEFP, siefp.AsUINT64);
    mSynicConnected = TRUE;
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

    if (!mSynicConnected)
    {
        return;
    }

    //
    // Clear all the timers.
    //

    for (timerIndex = 0; timerIndex < HV_SYNIC_STIMER_COUNT; timerIndex += 1)
    {
        AsmWriteMsr64(HV_X64_MSR_STIMER0_COUNT + 2 * timerIndex, 0);
        AsmWriteMsr64(HV_X64_MSR_STIMER0_CONFIG + 2 * timerIndex, 0);
    }

    //
    // Disconnect the SINTs and drain all the message queues.
    //

    for (sintIndex = 0; sintIndex < HV_SYNIC_SINT_COUNT; sintIndex += 1)
    {
        EfiHvDisconnectSint(&mHv, sintIndex);
        while (EfiHvGetSintMessage(&mHv, sintIndex) != NULL)
        {
            EfiHvCompleteSintMessage(&mHv, sintIndex);
        }
    }

    //
    // Disable the message page.
    //

    simp.AsUINT64 = AsmReadMsr64(HV_X64_MSR_SIMP);
    simp.SimpEnabled = 0;
    simp.BaseSimpGpa = 0;
    AsmWriteMsr64(HV_X64_MSR_SIMP, simp.AsUINT64);

    //
    // Disable the event page.
    //

    siefp.AsUINT64 = AsmReadMsr64(HV_X64_MSR_SIEFP);
    siefp.SiefpEnabled = 0;
    siefp.BaseSiefpGpa = 0;
    AsmWriteMsr64(HV_X64_MSR_SIEFP, siefp.AsUINT64);
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
    EfiHvConfigureTimer,
    EfiHvSetTimer,
    EfiHvPostMessage,
    EfiHvSignalEvent,
};


EFI_STATUS
EFIAPI
EfiHvInitialize (
    __in EFI_HANDLE ImageHandle,
    __in EFI_SYSTEM_TABLE *SystemTable
    )
/*++

Routine Description:

    Initializes the EfiHv driver.

Arguments:

    ImageHandle - The handle of the loaded image.

    SystemTable - A pointer to the system table.

Return Value:

    EFI status.

--*/
{
    EFI_STATUS status;

    //
    // Find the CPU protocol.
    //

    status = gBS->LocateProtocol(&gEfiCpuArchProtocolGuid, NULL, (VOID **)&mCpu);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    //
    // Register notify function for EVT_SIGNAL_EXIT_BOOT_SERVICES.
    //

    status = gBS->CreateEventEx(EVT_NOTIFY_SIGNAL,
                                TPL_CALLBACK,
                                EfiHvExitBootServices,
                                NULL,
                                &gEfiEventExitBootServicesGuid,
                                &mExitBootServicesEvent);

    //
    // Connect to the hypervisor and SINT.
    //

    status = EfiHvConnectToHypervisor();
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    status = EfiHvConnectToSynic();
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    //
    // Register the HV protocol.
    //

    status = gBS->InstallMultipleProtocolInterfaces(
                    &mHvHandle,
                    &gEfiHvProtocolGuid, &mHv,
                    NULL);

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

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

    return EFI_SUCCESS;
}

