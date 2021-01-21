/*++

Copyright (c) Microsoft Corporation

Module Name:

    VmbusRoot.c

Abstract:

    Provides the root controller and bus implementation for the VMBus driver.

Author:

    Arseney Romanenko (arseneyr) - 7-Jul-2012

--*/


#include <PiDxe.h>
#include <VmbusP.h>
#include <IsolationTypes.h>

#include <Library/DevicePathLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SynchronizationLib.h>

typedef struct _VMBUS_HOT_MESSAGE
{
    LIST_ENTRY Link;
    VMBUS_MESSAGE Message;

} VMBUS_HOT_MESSAGE;


struct _VMBUS_ROOT_CONTEXT
{
    UINT32 Signature;

    EFI_EVENT WaitForMessage;
    EFI_EVENT ExitBootEvent;

    EFI_EVENT HotAllocationEvent;
    EFI_EVENT HotEvent;
    LIST_ENTRY HotMessageList;

    BOOLEAN SintConnected;
    BOOLEAN ContactInitiated;
    BOOLEAN OffersDelivered;
    VMBUS_MESSAGE_RESPONSE GpadlTable[VMBUS_MAX_GPADLS];

    VMBUS_CHANNEL_CONTEXT *Channels[VMBUS_MAX_CHANNELS];
    UINT32 MaxInterruptUsed;
};

#if defined(MDE_CPU_IA32)


BOOLEAN
_BitScanForward64(
    __deref_out_range(<, (sizeof(UINT64) * 8)) PULONG BitNumber,
    __in UINT64 Mask
    )
/*++

Routine Description:

    This routine returns the first low order bit set in a 64bit mask, scanning
    from the least signifcant bit to the most significant bit.

Arguments:

    BitNumber - Supplies a pointer to the variable that will receive the bit
        number of the first low bit set (from 0 to 63), or zero if no bits are
        set.

    Mask - Supplies a bit mask that will be scanned for set bits.

Return Value:

    TRUE if a bit was set in Mask, FALSE if no bits were set.

--*/
{
    unsigned long partialBitNumber;

    if (_BitScanForward(&partialBitNumber, (unsigned long) Mask))
    {
        *BitNumber = partialBitNumber;
        return TRUE;
    }
    else if (_BitScanForward(&partialBitNumber, (unsigned long) (Mask >> 32)))
    {
        *BitNumber = partialBitNumber + 32;
        return TRUE;
    }
    else
    {
        *BitNumber = 0;
        return FALSE;
    }
}

UINT64 _InterlockedExchange64(UINT64* Target, UINT64 Value)
{
    UINT64 Old;
    do Old = *Target;
    while (InterlockedCompareExchange64(Target, Value, Old) != Old);
    return Old;
}


#endif // defined(MDE_CPU_IA32)

VOID
EFIAPI
VmbusRootSintNotify(
    __in VOID *Context
    );

VOID
VmbusRootScanEventFlags(
    __in VMBUS_ROOT_CONTEXT *RootContext,
    __in volatile HV_SYNIC_EVENT_FLAGS *Flags
    );

BOOLEAN
VmbusRootDispatchMessage(
    __in VMBUS_ROOT_CONTEXT *RootContext,
    __in HV_MESSAGE *HvMessage
    );

VOID
EFIAPI
VmbusRootExitBootServices(
    __in EFI_EVENT Event,
    __in VOID *Context
    );

EFI_STATUS
VmbusRootInitializeContext(
    __in VMBUS_ROOT_CONTEXT *RootContext
    );

EFI_STATUS
VmbusRootDestroyContext(
    __in VMBUS_ROOT_CONTEXT *RootContext
    );

EFI_STATUS
VmbusRootDestroyChannel(
    __in VMBUS_CHANNEL_CONTEXT *ChannelContext
    );

VOID
VmbusRootWaitForMessage(
    __in VMBUS_ROOT_CONTEXT *RootContext,
    __in BOOLEAN PollForMessage,
    __out VMBUS_MESSAGE *Message
    );

EFI_STATUS
VmbusRootInitiateContact(
    __in VMBUS_ROOT_CONTEXT *RootContext
    );

VOID
VmbusRootSendUnload(
    __in VMBUS_ROOT_CONTEXT *RootContext
    );

EFI_STATUS
VmbusRootCreateChannel(
    __in VMBUS_ROOT_CONTEXT *RootContext,
    __in VMBUS_CHANNEL_OFFER_CHANNEL *OfferMessage,
    __out_opt VMBUS_CHANNEL_CONTEXT **ChannelContext
    );

BOOLEAN
VmbusRootIsChannelAllowed(
    __in VMBUS_CHANNEL_OFFER_CHANNEL *OfferMessage
);

EFI_STATUS
VmbusRootEnumerateChildren(
    __in VMBUS_ROOT_CONTEXT *RootContext
    );

VOID
EFIAPI
VmbusRootHotAddAllocation(
    __in EFI_EVENT Event,
    __in VOID *Context
    );

VOID
EFIAPI
VmbusRootHotAdd(
    __in EFI_EVENT Event,
    __in VOID *Context
    );

EFI_STATUS
EFIAPI
VmbusComponentNameGetDriverName (
    __in EFI_COMPONENT_NAME_PROTOCOL *This,
    __in CHAR8 *Language,
    __out CHAR16 **DriverName
    );

EFI_STATUS
EFIAPI
VmbusComponentNameGetControllerName(
    __in EFI_COMPONENT_NAME_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in_opt EFI_HANDLE ChildHandle,
    __in CHAR8 *Language,
    __out CHAR16 **ControllerName
    );

VMBUS_ROOT_CONTEXT mRootContext;

EFI_HANDLE mRootDevice;
EFI_HANDLE mVmbusImageHandle;
HV_CONNECTION_ID gVmbusConnectionId = {VMBUS_MESSAGE_CONNECTION_ID};
EFI_GUID *mVmbusLegacyProtocolGuid;

VMBUS_ROOT_DEVICE_PATH gVmbusRootDevicePath;
VMBUS_ROOT_NODE gVmbusRootNode =
{
    {
        {
            ACPI_DEVICE_PATH,
            ACPI_EXTENDED_DP,
            {
                (UINT8) (sizeof (VMBUS_ROOT_NODE)),
                (UINT8) ((sizeof (VMBUS_ROOT_NODE)) >> 8)
            }
        },
        0,
        0,
        0
    },
    VMBUS_ROOT_NODE_HID_STR,
    '\0',
    '\0'
};

EFI_DEVICE_PATH_PROTOCOL gEfiEndNode =
{
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
        (UINT8) (END_DEVICE_PATH_LENGTH),
        (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
};


EFI_STATUS
VmbusRootInitializeContext(
    __in VMBUS_ROOT_CONTEXT *RootContext
    )
/*++

Routine Description:

    This routine initializes a root context.

Arguments:

    RootContext - Pointer to the root context to initialize.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;

    ZeroMem(RootContext, sizeof(*RootContext));
    RootContext->Signature = VMBUS_ROOT_CONTEXT_SIGNATURE;
    InitializeListHead(&RootContext->HotMessageList);
    RootContext->SintConnected = FALSE;
    RootContext->ContactInitiated = FALSE;
    RootContext->OffersDelivered = FALSE;

    status = gBS->CreateEvent(0,
                              0,
                              NULL,
                              NULL,
                              &RootContext->WaitForMessage);

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    //
    // Set the hot event to the lowest TPL possible so any driver unbindings
    // triggered by hot-remove can safely stop the EMCL channel.
    //

    status = gBS->CreateEvent(EVT_NOTIFY_SIGNAL,
                              TPL_APPLICATION + 1,
                              VmbusRootHotAdd,
                              (VOID*) RootContext,
                              &RootContext->HotEvent);

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    //
    // Set the hot allocation event to the highest TPL that allows us to
    // allocate memory for the hot message.
    //

    status = gBS->CreateEvent(EVT_NOTIFY_SIGNAL,
                              TPL_NOTIFY,
                              VmbusRootHotAddAllocation,
                              (VOID*) RootContext,
                              &RootContext->HotAllocationEvent);

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    status = EFI_SUCCESS;

Cleanup:
    if (EFI_ERROR(status))
    {
        VmbusRootDestroyContext(RootContext);
    }

    return status;
}


EFI_STATUS
VmbusRootDestroyChannel(
    __in VMBUS_CHANNEL_CONTEXT *ChannelContext
    )
/*++

Routine Description:

    This routine destroys a channel handle by uninstalling the VMBus and Device
    Path protocols and then destroying the channel context.

Arguments:

    ChannelContext - Pointer to the channel context to destroy.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;

    DEBUG((EFI_D_INFO, "%a(%d) channelContext = %p ChannelId 0x%x\n",
        __FUNCTION__,
        __LINE__,
        ChannelContext,
        ChannelContext->ChannelId));

    status = gBS->UninstallMultipleProtocolInterfaces(
        ChannelContext->Handle,
        &gEfiVmbusProtocolGuid,
        &ChannelContext->VmbusProtocol,
        &gEfiDevicePathProtocolGuid,
        &ChannelContext->DevicePath,
        NULL);

    if (EFI_ERROR(status))
    {
        DEBUG((EFI_D_ERROR, "Could not uninstall VMBus protocol\n"));
        return status;
    }

    status = gBS->UninstallMultipleProtocolInterfaces(
        ChannelContext->Handle,
        mVmbusLegacyProtocolGuid,
        &ChannelContext->LegacyVmbusProtocol,
        NULL);

    if (EFI_ERROR(status))
    {
        DEBUG((EFI_D_ERROR, "Could not uninstall legacy VMBus protocol\n"));
        return status;
    }

    gBS->CloseProtocol(mRootDevice,
                       &gEfiVmbusRootProtocolGuid,
                       mVmbusImageHandle,
                       ChannelContext->Handle);

    ASSERT(ChannelContext->RootContext->Channels[ChannelContext->ChannelId] != NULL);

    ChannelContext->RootContext->Channels[ChannelContext->ChannelId] = NULL;
    VmbusChannelDestroyContext(ChannelContext);
    FreePool(ChannelContext);
    return EFI_SUCCESS;
}


EFI_STATUS
VmbusRootDestroyContext (
    __in VMBUS_ROOT_CONTEXT *RootContext
    )
/*++

Routine Description:

    This routine destroyes a root context.

Arguments:

    RootContext - Pointer to the root context to destroy.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;
    UINT32 index;
    VMBUS_HOT_MESSAGE *hotMessage;

    DEBUG((EFI_D_INFO, "%a(%d) RootContext = %p\n",
        __FUNCTION__,
        __LINE__,
        RootContext));

    if (RootContext->ContactInitiated)
    {
        VmbusRootSendUnload(RootContext);
        RootContext->ContactInitiated = FALSE;
        RootContext->OffersDelivered = FALSE;
    }

    if (RootContext->SintConnected)
    {
        mHv->DisconnectSint(mHv, FixedPcdGet8(PcdVmbusSintIndex));
        RootContext->SintConnected = FALSE;
    }

    for (index = 0; index < VMBUS_MAX_CHANNELS; ++index)
    {
        if (RootContext->Channels[index] != NULL)
        {
            status = VmbusRootDestroyChannel(RootContext->Channels[index]);
            if (EFI_ERROR(status))
            {
                return status;
            }
            ASSERT(RootContext->Channels[index] == NULL);
        }
    }

    if (RootContext->WaitForMessage != NULL)
    {
        gBS->CloseEvent(RootContext->WaitForMessage);
        RootContext->WaitForMessage = NULL;
    }

    if (RootContext->ExitBootEvent != NULL)
    {
        gBS->CloseEvent(RootContext->ExitBootEvent);
        RootContext->ExitBootEvent = NULL;
    }

    if (RootContext->HotEvent != NULL)
    {
        gBS->CloseEvent(RootContext->HotEvent);
        RootContext->HotEvent = NULL;
    }

    if (RootContext->HotAllocationEvent != NULL)
    {
        gBS->CloseEvent(RootContext->HotAllocationEvent);
        RootContext->HotAllocationEvent = NULL;
    }

    while (!IsListEmpty(&RootContext->HotMessageList))
    {
        hotMessage = BASE_CR(RemoveEntryList(GetFirstNode(&RootContext->HotMessageList)),
                             VMBUS_HOT_MESSAGE,
                             Link);

        FreePool(hotMessage);
    }

    for (index = 0; index < VMBUS_MAX_GPADLS; ++index)
    {
        //
        // All drivers above should have released all GPADLs by now.
        //

        ASSERT(!VmbusRootValidateGpadl(RootContext, index));

        VmbusRootReclaimGpadl(RootContext, index);
    }

    return EFI_SUCCESS;
}


VOID
VmbusRootWaitForMessage(
    __in VMBUS_ROOT_CONTEXT *RootContext,
    __in BOOLEAN PollForMessage,
    __out VMBUS_MESSAGE *Message
    )
/*++

Routine Description:

    This routine waits for a message targeted at the root device.

    This routine must be called at TPL < TPL_NOTIFY.

Arguments:

    RootContext - Pointer to the root context.

    PollForMessage - poll for a message instead of waiting for event

    Message - Returns the message received.

Return Value:

    None.

--*/
{   UINTN index;
    HV_MESSAGE *hvMessage;

    //
    // TPL must be less than TPL_NOTIFY, since hot add/remove messages are
    // processed in events at that TPL and will block all other messages.
    //

    ASSERT(EfiGetCurrentTpl() < TPL_NOTIFY);
    ASSERT(RootContext->SintConnected);

    if (!PollForMessage)
    {
        gBS->WaitForEvent(1, &RootContext->WaitForMessage, &index);
    }

    hvMessage = NULL;
    while (hvMessage == NULL)
    {
        hvMessage = mHv->GetSintMessage(mHv, FixedPcdGet8(PcdVmbusSintIndex));
    }

    // Read the message size and store it before validation to avoid
    // double fetch.
    Message->Size = hvMessage->Header.PayloadSize;

    VMBUS_FAIL_FAST_IF_FALSE(Message->Size <= MAXIMUM_SYNIC_MESSAGE_BYTES);

    CopyMem(Message->Data, hvMessage->Payload, Message->Size);
    mHv->CompleteSintMessage(mHv, FixedPcdGet8(PcdVmbusSintIndex));
}


VMBUS_MESSAGE*
VmbusRootWaitForChannelResponse(
    __in VMBUS_CHANNEL_CONTEXT *ChannelContext
    )
/*++

Routine Description:

    This routine waits for a message targeted at a specific channel.

    This routine must be called at TPL < TPL_NOTIFY.

Arguments:

    ChannelContext - The channel to which the message is targeted to.

Return Value:

    The message received.

--*/
{
    EFI_STATUS status;
    UINTN index;

    //
    // TPL must be less than TPL_NOTIFY, since hot add/remove messages are
    // processed in events at that TPL and will block all other messages.
    //

    ASSERT(EfiGetCurrentTpl() < TPL_NOTIFY);

    status = gBS->WaitForEvent(1, &ChannelContext->Response.Event, &index);

    ASSERT_EFI_ERROR(status);

    return &ChannelContext->Response.Message;
}


EFI_STATUS
VmbusRootWaitForGpadlResponse(
    __in VMBUS_ROOT_CONTEXT *RootContext,
    __in UINT32 GpadlHandle,
    __out VMBUS_MESSAGE **Message
    )
/*++

Routine Description:

    This routine waits for a message targeted at a specific GPADL.

    This routine must be called at TPL < TPL_NOTIFY.

Arguments:

    RootContext - Pointer to the root context.

    GpadlHandle - GPADL handle to wait on.

    Message - Returns the message.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;
    UINTN index;

    //
    // TPL must be less than TPL_NOTIFY, since hot add/remove messages are
    // processed in events at that TPL and will block all other messages.
    //

    ASSERT(EfiGetCurrentTpl() < TPL_NOTIFY);

    if (RootContext->GpadlTable[GpadlHandle].Event == NULL)
    {
        return EFI_INVALID_PARAMETER;
    }

    status = gBS->WaitForEvent(1,
                               &RootContext->GpadlTable[GpadlHandle].Event,
                               &index);

    ASSERT_EFI_ERROR(status);

    *Message = &RootContext->GpadlTable[GpadlHandle].Message;
    return EFI_SUCCESS;
}


VOID
VmbusRootInitializeMessage(
    __inout VMBUS_MESSAGE *Message,
    __in VMBUS_CHANNEL_MESSAGE_TYPE Type,
    __in UINT32 Size
    )
/*++

Routine Description:

    This routine initializes a VMBus message.

Arguments:

    Message - Pointer to the VMBus message to initialize.

    Type - Type of the message.

    Size - Size of the message.

Return Value:

    None.

--*/
{
    ZeroMem(Message, sizeof(*Message));
    Message->Size = Size;
    Message->Header.MessageType = Type;
}


VOID
VmbusRootSendMessage(
    __in VMBUS_MESSAGE *Message
    )
/*++

Routine Description:

    This routine synchronously sends a VMBus message to the opposite endpoint.

Arguments:

    Message - VMBus message to send.

Return Value:

    None.

--*/
{
    EFI_STATUS status;

    do
    {
        status = mHv->PostMessage(mHv,
                                  gVmbusConnectionId,
                                  VMBUS_MESSAGE_TYPE,
                                  Message->Data,
                                  Message->Size);

    } while (status == EFI_NOT_READY);

    if (EFI_ERROR(status))
    {
        DEBUG((EFI_D_ERROR, "Vmbus failed to send message\n"));
    }
}


VOID
EFIAPI
VmbusRootSintNotify (
    __in VOID *Context
    )
/*++

Routine Description:

    This interrupt callback scans event flags and dispatches VMBus messages when
    a VMBus SINT is received.

Arguments:

    Context - Pointer to the interrupt context, which is a pointer to the root
    context.

Return Value:

    None.

--*/
{
    VMBUS_ROOT_CONTEXT *rootContext;
    HV_MESSAGE *hvMessage;

    rootContext = (VMBUS_ROOT_CONTEXT*)Context;

    VmbusRootScanEventFlags(rootContext,
                            mHv->GetSintEventFlags(mHv, FixedPcdGet8(PcdVmbusSintIndex)));

    hvMessage = mHv->GetSintMessage(mHv, FixedPcdGet8(PcdVmbusSintIndex));

    if (hvMessage != NULL)
    {
        if (VmbusRootDispatchMessage(rootContext, hvMessage))
        {
            mHv->CompleteSintMessage(mHv, FixedPcdGet8(PcdVmbusSintIndex));
        }
    }
}


VOID
VmbusRootScanEventFlags(
    __in VMBUS_ROOT_CONTEXT *RootContext,
    __in volatile HV_SYNIC_EVENT_FLAGS *Flags
    )
/*++

Routine Description:

    This routine scans the hypervisor event flags and signals interrupt events
    that channels have registered.

    This routine must be called at TPL == TPL_HIGH_LEVEL.

Arguments:

    RootContext - Pointer to the root context.

    Flags - Hypervisor flags to scan.

Return Value:

    None.

--*/
{
    UINT64 *flags;
    UINT32 wordIndex;
    UINT32 bitIndex;
    UINT64 currentWord;
    UINT32 wordCount;

    flags = (UINT64*)Flags->Flags32;

    //
    // Scan through all the words up to and including largest interrupt flag
    // used.
    //

    wordCount = RootContext->MaxInterruptUsed / 64 + 1;
    for (wordIndex = 0; wordIndex < wordCount; ++wordIndex)
    {
        currentWord = _InterlockedExchange64(&flags[wordIndex], 0);
        while(_BitScanForward64(&bitIndex, currentWord) != 0)
        {
            currentWord &= ~((UINT64)1 << bitIndex);
            gBS->SignalEvent(
                    RootContext->Channels[wordIndex * 64 + bitIndex]->Interrupt);
        }
    }
}


BOOLEAN
VmbusRootDispatchMessage(
    __in VMBUS_ROOT_CONTEXT *RootContext,
    __in HV_MESSAGE *HvMessage
    )
/*++

Routine Description:

    This routine dispatches a hypervisor message based on its type, notifying
    either the root device, a channel device, or a GPADL handle.

    This routine must be called at TPL == TPL_HIGH_LEVEL.

    This routine receives a message from the host and therefore
    must validate this message before using it.

Arguments:

    RootContext - Pointer to the root context.

    HvMessage - Hypervisor message to dispatch.

Return Value:

    TRUE if hypervisor message should be completed, FALSE otherwise.

--*/
{
    VMBUS_MESSAGE *message;
    VMBUS_MESSAGE_RESPONSE *response;
    BOOLEAN completeMessage;
    UINT32 childId;
    UINT32 gpadl;

    completeMessage = TRUE;
    response = NULL;

    childId = 0;
    gpadl = 0;

    VMBUS_FAIL_FAST_IF_FALSE(HvMessage->Header.MessageType == VMBUS_MESSAGE_TYPE);

    message = BASE_CR(HvMessage->Payload, VMBUS_MESSAGE, Header);

    switch (message->Header.MessageType)
    {
    case ChannelMessageOfferChannel:
        //
        // Hot add events need to drop TPL to allocate memory and should queue
        // up messages behind them, so don't complete this message.
        //
        if (RootContext->OffersDelivered)
        {
            gBS->SignalEvent(RootContext->HotAllocationEvent);
            completeMessage = FALSE;
            break;
        }

        __fallthrough;

    case ChannelMessageVersionResponse:
        __fallthrough;

    case ChannelMessageAllOffersDelivered:
        __fallthrough;

    case ChannelMessageUnloadComplete:

        //
        // These messages are dealt with differently, since they arrive
        // synchronously during initialization and are not channel or GPADL-
        // specific.
        //

        gBS->SignalEvent(RootContext->WaitForMessage);
        completeMessage = FALSE;
        break;

    case ChannelMessageOpenChannelResult:

        // Store the channel ID before validating to avoid a double fetch.
        childId = message->OpenResult.ChildRelId;
        VMBUS_FAIL_FAST_IF_FALSE(childId < VMBUS_MAX_CHANNELS);
        response = &RootContext->Channels[childId]->Response;

        break;

    case ChannelMessageGpadlTorndown:

        // Store the GPADL before validating to avoid a double fetch.
        gpadl = message->GpadlTorndown.Gpadl;
        VMBUS_FAIL_FAST_IF_FALSE(gpadl < VMBUS_MAX_GPADLS);
        VMBUS_FAIL_FAST_IF_FALSE(VmbusRootValidateGpadl(RootContext, gpadl));
        response = &RootContext->GpadlTable[gpadl];

        break;

    case ChannelMessageGpadlCreated:

        // Store the GPADL before validating to avoid a double fetch.
        gpadl = message->GpadlCreated.Gpadl;
        VMBUS_FAIL_FAST_IF_FALSE(gpadl < VMBUS_MAX_GPADLS);
        VMBUS_FAIL_FAST_IF_FALSE(VmbusRootValidateGpadl(RootContext, gpadl));
        response = &RootContext->GpadlTable[gpadl];

        break;

    case ChannelMessageRescindChannelOffer:

        //
        // Hot remove is not supported because UEFI makes it difficult to
        // guarantee a channel will not be used once it is gone. Silently accept
        // rescind messages but never send a RelIdReleased in response.
        //

        break;

    default:
        ASSERT(!"Vmbus received unexpected message");

        break;
    }

    if (response != NULL)
    {
        // Validate the payload size coming in from the host.
        // Validate a locally stored value to avoid a double fetch.
        response->Message.Size = HvMessage->Header.PayloadSize;
        VMBUS_FAIL_FAST_IF_FALSE(response->Message.Size <= MAXIMUM_SYNIC_MESSAGE_BYTES);

        CopyMem(response->Message.Data,
                HvMessage->Payload,
                response->Message.Size);

        gBS->SignalEvent(response->Event);
    }

    return completeMessage;
}


VOID
EFIAPI
VmbusRootHotAddAllocation(
    _In_ EFI_EVENT Event,
    _In_ VOID * Context
    )
/*++

Routine Description:

    This routine allocates space for hot add messages and copies the message
    from the SINT queue, to be processed by VmbusRootHotAdd.

    This routine receives a message from the host and therefore
    must validate this message before using it.

Arguments:

    Event - The event that was signalled.

    Context - A pointer to the event context, which is the root context.

Return Value:

    None.

--*/
{
    VMBUS_ROOT_CONTEXT *context;
    HV_MESSAGE *hvMessage;
    VMBUS_HOT_MESSAGE *hotMessage;

    ASSERT(EfiGetCurrentTpl() == TPL_NOTIFY);

    context = (VMBUS_ROOT_CONTEXT*)Context;
    hvMessage = mHv->GetSintMessage(mHv, FixedPcdGet8(PcdVmbusSintIndex));

    hotMessage = AllocatePool(sizeof(*hotMessage));
    if (hotMessage == NULL)
    {
        goto Cleanup;
    }

    ZeroMem(hotMessage, sizeof(*hotMessage));

    hotMessage->Message.Size = hvMessage->Header.PayloadSize;

    VMBUS_FAIL_FAST_IF_FALSE(hotMessage->Message.Size == sizeof(hotMessage->Message.OfferChannel));

    CopyMem(hotMessage->Message.Data,
            hvMessage->Payload,
            hotMessage->Message.Size);
    
    VMBUS_FAIL_FAST_IF_FALSE(hotMessage->Message.Header.MessageType == ChannelMessageOfferChannel);

    VMBUS_FAIL_FAST_IF_FALSE(hotMessage->Message.OfferChannel.ChildRelId < VMBUS_MAX_CHANNELS);
    VMBUS_FAIL_FAST_IF_FALSE(context->Channels[hotMessage->Message.OfferChannel.ChildRelId] == NULL);

    // Do not proceed if this channel is not allowed during UEFI boot.
    if (!VmbusRootIsChannelAllowed(&hotMessage->Message.OfferChannel))
    {
        // Do nothing for this channel creation.
        FreePool(hotMessage);
        goto Cleanup;
    }

    InsertTailList(&context->HotMessageList, &hotMessage->Link);
    gBS->SignalEvent(context->HotEvent);

Cleanup:
    mHv->CompleteSintMessage(mHv, FixedPcdGet8(PcdVmbusSintIndex));
}


VOID
EFIAPI
VmbusRootHotAdd(
    __in EFI_EVENT Event,
    __in VOID *Context
    )
/*++

Routine Description:

    This routine processes hot-add messages. Hot-remove is tricky under UEFI,
    as we cannot guarantee that a channel isn't being used (or block on it) when
    it's being removed.

Arguments:

    Event - The event that was signalled.

    Context - A pointer to the event context, which is the root context.

Return Value:

    None.

--*/
{
    EFI_STATUS status;
    EFI_TPL tpl;
    LIST_ENTRY list;
    VMBUS_ROOT_CONTEXT *context;
    VMBUS_CHANNEL_CONTEXT *channelContext;
    VMBUS_HOT_MESSAGE *hotMessage;

    context = (VMBUS_ROOT_CONTEXT*)Context;
    channelContext = NULL;
    InitializeListHead(&list);

    tpl = gBS->RaiseTPL(TPL_NOTIFY);

    //
    // While TPL is raised, copy list of messages locally.
    //

    if (!IsListEmpty(&context->HotMessageList))
    {
        list = context->HotMessageList;
        list.ForwardLink->BackLink = &list;
        list.BackLink->ForwardLink = &list;
        InitializeListHead(&context->HotMessageList);
    }

    gBS->RestoreTPL(tpl);
    while (!IsListEmpty(&list))
    {
        hotMessage = BASE_CR(GetFirstNode(&list), VMBUS_HOT_MESSAGE, Link);

        // The offer message is validated before adding it to the list. 
        ASSERT(hotMessage->Message.Header.MessageType == ChannelMessageOfferChannel);
        ASSERT(hotMessage->Message.Size == sizeof(hotMessage->Message.OfferChannel));

        status = VmbusRootCreateChannel(context,
                                        &hotMessage->Message.OfferChannel,
                                        &channelContext);

        if (EFI_ERROR(status))
        {
            DEBUG((EFI_D_ERROR, "Hot add returned %r\n", status));
        }
        else
        {
            //
            // ConnectController must be manually called to hook this channel up to
            // any drivers that can manage it.
            //

            gBS->ConnectController(channelContext->Handle,
                                   NULL,
                                   NULL,
                                   TRUE);
        }

        RemoveEntryList(&hotMessage->Link);
        FreePool(hotMessage);
    }
}


EFI_STATUS
VmbusRootGetFreeGpadl(
    __in VMBUS_ROOT_CONTEXT *RootContext,
    __out UINT32 *GpadlHandle
    )
/*++

Routine Description:

    This routine allocates a new GPADL and returns its handle.

    This routine must be called at TPL <= TPL_VMBUS.

Arguments:

    RootContext - Pointer to the root context.

    GpadlHandle - Handle of the GPADL that was created.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;
    EFI_TPL tpl;
    EFI_EVENT event;
    UINT32 index;

    event = NULL;

    status = gBS->CreateEvent(0,
                              0,
                              NULL,
                              NULL,
                              &event);

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    tpl = gBS->RaiseTPL(TPL_VMBUS);

    //
    // The whole GPADL array is scanned for a free entry.
    // FUTURE-arseneyr-20120731: Make this more efficient.
    //

    for (index = 1; index < VMBUS_MAX_GPADLS; ++index)
    {
        if (RootContext->GpadlTable[index].Event == NULL)
        {
            *GpadlHandle = index;

            //
            // Create a new event to mark it as taken.
            //

            RootContext->GpadlTable[index].Event = event;
            break;
        }
    }

    gBS->RestoreTPL(tpl);

    if (index == VMBUS_MAX_GPADLS)
    {
        status = EFI_OUT_OF_RESOURCES;
        goto Cleanup;
    }

    status = EFI_SUCCESS;

Cleanup:
    if (EFI_ERROR(status))
    {
        if (event != NULL)
        {
            gBS->CloseEvent(event);
        }
    }

    return status;
}


VOID
VmbusRootReclaimGpadl(
    __in VMBUS_ROOT_CONTEXT *RootContext,
    __in UINT32 GpadlHandle
    )
/*++

Routine Description:

    This routine releases a GPADL to be reused. The routine revokes host
    visibility.

Arguments:

    RootContext - Pointer to the root context.

    GpadlHandle - GPADL handle to release.

Return Value:

    None.

--*/
{
    VMBUS_MESSAGE_RESPONSE *gpadlEntry;

    gpadlEntry = &RootContext->GpadlTable[GpadlHandle];
    if (gpadlEntry->Event != NULL)
    {
        gBS->CloseEvent(gpadlEntry->Event);
        gpadlEntry->Event = NULL;
    }
}

BOOLEAN
VmbusRootValidateGpadl(
    __in VMBUS_ROOT_CONTEXT *RootContext,
    __in UINT32 GpadlHandle
    )
/*++

Routine Description:

    This routine verifies if the provided GPADL handle is valid.

Arguments:

    RootContext - Pointer to the root context.

    GpadlHandle - GPADL handle to verify.

Return Value:

    TRUE if the GPADL has been created previously, FALSE otherwise.

--*/
{
    return RootContext->GpadlTable[GpadlHandle].Event != NULL;
}


VOID
VmbusRootSetInterruptEntry(
    __in VMBUS_ROOT_CONTEXT *RootContext,
    __in UINT32 ChannelId,
    __in EFI_EVENT Event
    )

/*++

Routine Description:

    This routine registers an interrupt event for a channel.

Arguments:

    RootContext - Pointer to the root context.

    ChannelId - Index of the interrupt to set.

    Event - The event to signal when the interrupt occurs.

Return Value:

    None.

--*/

{
    EFI_TPL tpl;

    ASSERT(ChannelId < VMBUS_MAX_CHANNELS);

    tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);
    if (ChannelId > RootContext->MaxInterruptUsed)
    {
        RootContext->MaxInterruptUsed = ChannelId;
    }

    RootContext->Channels[ChannelId]->Interrupt = Event;
    gBS->RestoreTPL(tpl);
}


VOID
VmbusRootClearInterruptEntry(
    __in VMBUS_ROOT_CONTEXT *RootContext,
    __in UINT32 ChannelId
    )
/*++

Routine Description:

    This routine unregisters an interrupt for a channel.

Arguments:

    RootContext - Pointer to the root context.

    ChannelId - Index of the interrupt to clear.

Return Value:

    None.

--*/
{
    EFI_TPL tpl;
    UINT32 index;

    ASSERT(ChannelId < VMBUS_MAX_CHANNELS);

    tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);
    RootContext->Channels[ChannelId]->Interrupt = NULL;
    if (ChannelId == RootContext->MaxInterruptUsed)
    {
        //
        // Scan backwards for the first set interrupt.
        //

        for (index = RootContext->MaxInterruptUsed; index > 0; --index)
        {
            if (RootContext->Channels[index]->Interrupt != NULL)
            {
                break;
            }
        }

        RootContext->MaxInterruptUsed = index;
    }

    gBS->RestoreTPL(tpl);
}


VOID
EFIAPI
VmbusRootExitBootServices(
    __in EFI_EVENT Event,
    __in VOID *Context
    )
/*++

Routine Description:

    This event notification sends an unload message when ExitBootServices is
    called.

Arguments:

    Event - The event that was signalled.

    Context - A pointer to the event context, which is the root context.

Return Value:

    None.

--*/
{
    VMBUS_ROOT_CONTEXT *rootContext;
    int i;
    int orphanedGpadlCount = 0;

    rootContext = (VMBUS_ROOT_CONTEXT*)Context;

    for (i = 0; i < VMBUS_MAX_GPADLS; i++);
    {
        if (rootContext->GpadlTable[i].Event)
        {
            DEBUG((EFI_D_WARN,
                "%a (%d) GPADL 0x%x not cleaned up.\n",
                __FUNCTION__,
                __LINE__,
                i));
            orphanedGpadlCount++;
        }
    }

    DEBUG((EFI_D_WARN, "%a (%d) orphaned %d GPADLs (IsolationArchitecture=%d)\n",
        __FUNCTION__,
        __LINE__,
        orphanedGpadlCount,
        PcdGet32(PcdIsolationArchitecture)));

    VmbusRootSendUnload(rootContext);
}


EFI_STATUS
VmbusRootInitiateContact(
    __in VMBUS_ROOT_CONTEXT *RootContext
    )
/*++

Routine Description:

    This routine initiates contact with the host endpoint and negotiates the
    VMBus version.

    This function must be called at TPL < TPL_HIGH_LEVEL.

    This routine receives a message from the host and therefore
    must validate this message before using it.

Arguments:

    RootContext - Pointer to the root context.

Return Value:

    EFI_STATUS.

--*/
{
    VMBUS_MESSAGE message;
    EFI_STATUS status;

    ASSERT(RootContext->SintConnected);

    VmbusRootInitializeMessage(&message,
                           ChannelMessageInitiateContact,
                           sizeof(message.InitiateContact));

    message.InitiateContact.VMBusVersionRequested = VMBUS_VERSION_LATEST;
    message.InitiateContact.TargetMessageVp = mHv->GetCurrentVpIndex(mHv);
    VmbusRootSendMessage(&message);

    //
    // We may have leftover messages if this driver was stopped previously.
    //
    do
    {
        VmbusRootWaitForMessage(RootContext, FALSE, &message);
    } while (message.Header.MessageType != ChannelMessageVersionResponse);

    VMBUS_FAIL_FAST_IF_FALSE(message.Size == sizeof(message.VersionResponse));

    if (!message.VersionResponse.VersionSupported ||
        message.VersionResponse.ConnectionState
            != VmbusChannelConnectionSuccessful)
    {
        status = EFI_PROTOCOL_ERROR;

    }
    else
    {
        RootContext->ContactInitiated = TRUE;
        status = EFI_SUCCESS;
    }
    return status;
}


VOID
VmbusRootSendUnload(
    __in VMBUS_ROOT_CONTEXT *RootContext
    )
/*++

Routine Description:

    This routine sends an unload message and synchronously waits for a response
    from the root.

    This function must be called at TPL < TPL_HIGH_LEVEL.

    This routine receives a message from the host and therefore
    must validate this message before using it.

Arguments:

    RootContext - Pointer to the root context.

Return Value:

    None.

--*/
{
    VMBUS_MESSAGE message;

    VmbusRootInitializeMessage(&message,
                               ChannelMessageUnload,
                               sizeof(message.Header));

    VmbusRootSendMessage(&message);

    //
    // Ignore all messages until the unload response comes back.
    //

    do
    {
        VmbusRootWaitForMessage(RootContext, TRUE, &message);
    } while (message.Header.MessageType != ChannelMessageUnloadComplete);

    VMBUS_FAIL_FAST_IF_FALSE(message.Size == sizeof(message.Header));
}


EFI_STATUS
VmbusRootCreateChannel(
    __in VMBUS_ROOT_CONTEXT *RootContext,
    __in VMBUS_CHANNEL_OFFER_CHANNEL *OfferMessage,
    __out_opt VMBUS_CHANNEL_CONTEXT **ChannelContext
    )
/*++

Routine Description:

    This routine constructs a channel from an offer message.

Arguments:

    RootContext - Pointer to the root context.

    OfferMessage - The offer message received.

    ChannelContext - Optionally returns a pointer to the newly constructed
        channel context.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;
    VMBUS_CHANNEL_CONTEXT *channelContext;
    VOID *protocol;
    EFI_TPL tpl;

    channelContext = AllocatePool(sizeof(VMBUS_CHANNEL_CONTEXT));
    if (channelContext == NULL)
    {
        status = EFI_OUT_OF_RESOURCES;
        goto Cleanup;
    }

    VmbusChannelInitializeContext(channelContext,
                                  OfferMessage,
                                  RootContext);

    //
    // The following validations should have been done when the channel offer
    // was received. However, it is possible that the host can send multiple
    // channel offers with the same channel ID which would not be caught
    // unless an entry for this ID was made into the Channels list.
    //
    ASSERT(OfferMessage->ChildRelId < VMBUS_MAX_CHANNELS);

    tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);
    VMBUS_FAIL_FAST_IF_FALSE(RootContext->Channels[channelContext->ChannelId] == NULL);
    RootContext->Channels[channelContext->ChannelId] = channelContext;
    gBS->RestoreTPL(tpl);

    //
    // Install the Device Path and VMBus protocols onto a new child handle.
    //

    status = gBS->InstallMultipleProtocolInterfaces(&channelContext->Handle,
                                                    &gEfiDevicePathProtocolGuid,
                                                    &channelContext->DevicePath,
                                                    &gEfiVmbusProtocolGuid,
                                                    &channelContext->VmbusProtocol,
                                                    NULL);

    ASSERT_EFI_ERROR(status);

    status = gBS->InstallMultipleProtocolInterfaces(&channelContext->Handle,
                                                    mVmbusLegacyProtocolGuid,
                                                    &channelContext->LegacyVmbusProtocol,
                                                    NULL);

    ASSERT_EFI_ERROR(status);

    //
    // Open the root VMBus tag protocol BY_CHILD_CONTROLLER so EFI can track
    // this relation.
    //

    status = gBS->OpenProtocol(mRootDevice,
                               &gEfiVmbusRootProtocolGuid,
                               &protocol,
                               mVmbusImageHandle,
                               channelContext->Handle,
                               EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER);

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    if (ChannelContext != NULL)
    {
        *ChannelContext = channelContext;
    }

    status = EFI_SUCCESS;

Cleanup:
    if (EFI_ERROR(status))
    {
        if (channelContext != NULL)
        {
            VmbusRootDestroyChannel(channelContext);
        }
    }

    return status;
}


BOOLEAN
VmbusRootIsChannelAllowed(
    __in VMBUS_CHANNEL_OFFER_CHANNEL *OfferMessage
)
/*++

Routine Description:

    This routine determines if a VmBus channel is allowed or not.

Arguments:

    OfferMessage - The offer message received that contains the channel details.

Return Value:

    TRUE if the channel is allowed, FALSE otherwise.

--*/
{
    int index = 0;
    int allowedGuidCount = 0;
    UINT32 isolationType = 0;

    allowedGuidCount = sizeof(gAllowedGuids) / sizeof(gAllowedGuids[0]);
    isolationType = PcdGet32(PcdIsolationArchitecture);

    for (index = 0; index < allowedGuidCount; index++)
    {
        if (isolationType != UefiIsolationTypeNone)
        {
            if (gAllowedGuids[index].IsAllowedWhenIsolated == FALSE)
            {
                continue;
            }
        }

        if (CompareMem(&OfferMessage->InterfaceType, &gAllowedGuids[index].AllowedGuid, sizeof(EFI_GUID)) == 0)
        {
            DEBUG((DEBUG_INFO, "%a: Channel allowed during boot (%g).\n", __FUNCTION__, &OfferMessage->InterfaceType));
            return TRUE;
        }
    }

    DEBUG((DEBUG_WARN, "%a: Channel not allowed during boot (%g).\n", __FUNCTION__, &OfferMessage->InterfaceType));
    return FALSE;

}


EFI_STATUS
VmbusRootEnumerateChildren(
    __in VMBUS_ROOT_CONTEXT *RootContext
    )
/*++

Routine Description:

    This routine receives all VMBus offers from the root, creates a child
    handle for each one, and installs the VMBus and Device Path protocols onto
    the it.

    This function must be called at TPL < TPL_HIGH_LEVEL.

    This routine receives a message from the host and therefore
    must validate this message before using it.

Arguments:

    RootContext - Pointer to the root context.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;
    VMBUS_MESSAGE message;

    VmbusRootInitializeMessage(&message,
                               ChannelMessageRequestOffers,
                               sizeof(message.Header));

    VmbusRootSendMessage(&message);
    for (;;)
    {
        VmbusRootWaitForMessage(RootContext, FALSE, &message);
        if (message.Size == sizeof(message.Header) &&
            message.Header.MessageType == ChannelMessageAllOffersDelivered)
        {
            RootContext->OffersDelivered = TRUE;
            break;
        }

        if (message.Size != sizeof(message.OfferChannel) ||
            message.Header.MessageType != ChannelMessageOfferChannel)
        {
            ASSERT(!"Unexpected VMBus message received from root");
            return EFI_PROTOCOL_ERROR;
        }

        VMBUS_FAIL_FAST_IF_FALSE(message.OfferChannel.ChildRelId < VMBUS_MAX_CHANNELS);
        VMBUS_FAIL_FAST_IF_FALSE(RootContext->Channels[message.OfferChannel.ChildRelId] == NULL);

        // Do not proceed if this channel is not allowed during UEFI boot.
        if (!VmbusRootIsChannelAllowed(&message.OfferChannel))
        {
            // Do nothing for this channel creation.
            continue;
        }

        status = VmbusRootCreateChannel(RootContext,
                                        &message.OfferChannel,
                                        NULL);

        if (EFI_ERROR(status))
        {
            return status;
        }
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
VmbusRootDriverSupported (
    __in EFI_DRIVER_BINDING_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in_opt EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
    )
/*++

Routine Description:

    Supported routine for VMBus driver binding protocol.

Arguments:

    This - Pointer to the driver binding protocol.

    ControllerHandle - Device handle to check if supported.

    RemainingDevicePath - Device path of child to start.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;
    VOID *protocol;

    //
    // Check for the root controller tag GUID and make sure this driver is not
    // already managing this device.
    //

    status = gBS->OpenProtocol(ControllerHandle,
                               &gEfiVmbusRootProtocolGuid,
                               &protocol,
                               This->DriverBindingHandle,
                               ControllerHandle,
                               EFI_OPEN_PROTOCOL_BY_DRIVER);

    if (EFI_ERROR(status))
    {
        return status;
    }

    gBS->CloseProtocol(ControllerHandle,
                       &gEfiVmbusRootProtocolGuid,
                       This->DriverBindingHandle,
                       ControllerHandle);

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
VmbusRootDriverStart (
    __in EFI_DRIVER_BINDING_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in_opt EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
    )
/*++

Routine Description:

    Start routine for VMBus driver binding protocol.

Arguments:

    This - Pointer to the driver binding protocol.

    ControllerHandle - Device handle on which to start.

    RemainingDevicePath - Device path of device being started.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;
    VOID *protocol;
    DEBUG((DEBUG_VERBOSE, ">>> %a\n", __FUNCTION__));

    ASSERT(ControllerHandle == mRootDevice);

    status = gBS->LocateProtocol(&gEfiHvProtocolGuid, NULL, (VOID **)&mHv);

    if (EFI_ERROR(status))
    {
        return status;
    }

    status = gBS->LocateProtocol(&gEfiHvIvmProtocolGuid, NULL, (VOID **)&mHvIvm);

    if (EFI_ERROR(status))
    {
        return status;
    }

    mSharedGpaBoundary = (UINTN)PcdGet64(PcdIsolationSharedGpaBoundary);

    status = VmbusRootInitializeContext(&mRootContext);
    if (EFI_ERROR(status))
    {
        return status;
    }
    DEBUG((DEBUG_VERBOSE, "--- %a after VmbusRootInitializeContext\n", __FUNCTION__));

    status = mHv->ConnectSint(mHv,
                              FixedPcdGet8(PcdVmbusSintIndex),
                              FixedPcdGet8(PcdVmbusSintVector),
                              VmbusRootSintNotify,
                              &mRootContext);
    DEBUG((DEBUG_VERBOSE, "--- %a after ConnectSint status %r\n", __FUNCTION__, status));
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    mRootContext.SintConnected = TRUE;

    status = VmbusRootInitiateContact(&mRootContext);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }
    DEBUG((DEBUG_VERBOSE, "--- %a after VmbusRootInitiateContact status %r\n", __FUNCTION__, status));

    status = gBS->CreateEventEx(EVT_NOTIFY_SIGNAL,
                                TPL_CALLBACK,
                                VmbusRootExitBootServices,
                                &mRootContext,
                                &gEfiEventExitBootServicesGuid,
                                &mRootContext.ExitBootEvent);

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    status = VmbusRootEnumerateChildren(&mRootContext);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }
    DEBUG((DEBUG_VERBOSE, "--- %a after VmbusRootEnumerateChildren status %r\n", __FUNCTION__, status));
    status = gBS->OpenProtocol(ControllerHandle,
                               &gEfiVmbusRootProtocolGuid,
                               &protocol,
                               This->DriverBindingHandle,
                               ControllerHandle,
                               EFI_OPEN_PROTOCOL_BY_DRIVER);

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    status = EFI_SUCCESS;

Cleanup:
    if (EFI_ERROR(status))
    {
        VmbusRootDestroyContext(&mRootContext);
    }

    DEBUG((DEBUG_VERBOSE, "<<< %a status %r\n", __FUNCTION__, status));
    return status;
}


EFI_STATUS
EFIAPI
VmbusRootDriverStop (
    __in EFI_DRIVER_BINDING_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in UINTN NumberOfChildren,
    __in_ecount(NumberOfChildren) EFI_HANDLE *ChildHandleBuffer
    )
/*++

Routine Description:

    Stop routine for VMBus driver binding protocol.

Arguments:

    This - Pointer to the driver binding protocol.

    ControllerHandle - Pointer to the device handle which needs to be stopped.

    NumberOfChildren - If 0, stop the root controller. Otherwise, the number of
        children in ChildHandleBuffer to be stopped.

    ChildHandleBuffer - An array of child handles to stop.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;
    UINTN childIndex;
    UINTN channelIndex;
    VMBUS_CHANNEL_CONTEXT *channelContext;

    if (NumberOfChildren == 0)
    {
        ASSERT(ControllerHandle == mRootDevice);

        gBS->CloseProtocol(ControllerHandle,
                           &gEfiVmbusRootProtocolGuid,
                           This->DriverBindingHandle,
                           ControllerHandle);

        status = VmbusRootDestroyContext(&mRootContext);
        if (EFI_ERROR(status))
        {
            return status;
        }
    }
    else
    {
        for (childIndex = 0; childIndex < NumberOfChildren; ++childIndex)
        {
            for (channelIndex = 0;
                 channelIndex < VMBUS_MAX_CHANNELS;
                 ++channelIndex)
            {
                channelContext = mRootContext.Channels[channelIndex];
                if (channelContext != NULL &&
                    channelContext->Handle == ChildHandleBuffer[childIndex])
                {
                    status = VmbusRootDestroyChannel(channelContext);
                    if (EFI_ERROR(status))
                    {
                        return status;
                    }

                    ASSERT(mRootContext.Channels[channelIndex] == NULL);
                    break;
                }
            }

            if (channelIndex == VMBUS_MAX_CHANNELS)
            {
                ASSERT(!"VMBus stop call received invalid child");
            }
        }
    }

    return EFI_SUCCESS;
}


//
// Driver name table
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_UNICODE_STRING_TABLE gVmbusDriverNameTable[] =
{
    { "eng;en", (CHAR16 *)L"Hyper-V VMBus Driver"},
    { NULL, NULL }
};


//
// Controller name table
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_UNICODE_STRING_TABLE gVmbusControllerNameTable[] =
{
    { "eng;en", (CHAR16 *)L"Hyper-V VMBus Controller"},
    { NULL, NULL }
};


//
// EFI Component Name Protocol
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_COMPONENT_NAME_PROTOCOL gVmbusComponentName =
{
    VmbusComponentNameGetDriverName,
    VmbusComponentNameGetControllerName,
    "eng"
};

//
// EFI Component Name 2 Protocol
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_COMPONENT_NAME2_PROTOCOL gVmbusComponentName2 =
{
    (EFI_COMPONENT_NAME2_GET_DRIVER_NAME) VmbusComponentNameGetDriverName,
    (EFI_COMPONENT_NAME2_GET_CONTROLLER_NAME) VmbusComponentNameGetControllerName,
    "en"
};


EFI_DRIVER_BINDING_PROTOCOL gVmbusDriverBindingProtocol =
{
    VmbusRootDriverSupported,
    VmbusRootDriverStart,
    VmbusRootDriverStop,
    VMBUS_DRIVER_VERSION,
    NULL,
    NULL
};


EFI_STATUS
EFIAPI
VmbusComponentNameGetDriverName (
    __in EFI_COMPONENT_NAME_PROTOCOL *This,
    __in CHAR8 *Language,
    __out CHAR16 **DriverName
    )
/*++

Routine Description:

    Retrieves a Unicode string that is the user readable name of the EFI Driver.

    This function retrieves the user readable name of a driver in the form of a
    Unicode string. If the driver specified by This has a user readable name in
    the language specified by Language, then a pointer to the driver name is
    returned in DriverName, and EFI_SUCCESS is returned. If the driver specified
    by This does not support the language specified by Language,
    then EFI_UNSUPPORTED is returned.

Arguments:

    This - A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or EFI_COMPONENT_NAME_PROTOCOL instance.

    Language - A pointer to a Null-terminated ASCII string array indicating the language.

    DriverName - A pointer to the string to return.

Return Value:

    EFI_STATUS.

--*/
{
    return LookupUnicodeString2(
        Language,
        This->SupportedLanguages,
        gVmbusDriverNameTable,
        DriverName,
        (BOOLEAN)(This == &gVmbusComponentName));
}


EFI_STATUS
EFIAPI
VmbusComponentNameGetControllerName(
    __in EFI_COMPONENT_NAME_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in_opt EFI_HANDLE ChildHandle,
    __in CHAR8 *Language,
    __out CHAR16 **ControllerName
    )
/*++

Routine Description:

    Retrieves a Unicode string that is the user readable name of the controller
    that is being managed by a Driver.


    This function retrieves the user readable name of the controller specified by
    ControllerHandle and ChildHandle in the form of a Unicode string. If the
    driver specified by This has a user readable name in the language specified by
    Language, then a pointer to the controller name is returned in ControllerName,
    and EFI_SUCCESS is returned.  If the driver specified by This is not currently
    managing the controller specified by ControllerHandle and ChildHandle,
    then EFI_UNSUPPORTED is returned.  If the driver specified by This does not
    support the language specified by Language, then EFI_UNSUPPORTED is returned.

Arguments:

    This - A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or EFI_COMPONENT_NAME_PROTOCOL instance.

    ControllerHandle - The handle of a controller that the driver specified by This
           is managing.  This handle specifies the controller whose name is to be returned.

    ChildHandle - The handle of the child controller to retrieve the name of. This is an
        optional parameter that may be NULL.  It will be NULL for device drivers.  It will
        also be NULL for a bus drivers that wish to retrieve the name of the bus controller.
        It will not be NULL for a bus  driver that wishes to retrieve the name of a
        child controller.

    Language - A pointer to a Null-terminated ASCII string array indicating the language.
        This is the language of the driver name that the caller is requesting, and it
        must match one of the languages specified in SupportedLanguages. The number of
        languages supported by a driver is up to the driver writer. Language is specified in
        RFC 4646 or ISO 639-2 language code format.

    ControllerName - A pointer to the Unicode string to return. This Unicode string is the
        name of the controller specified by ControllerHandle and ChildHandle in the language
        specified by Language from the point of view of the driver specified by This.

Return Value:

    EFI_STATUS.


--*/
{
    EFI_STATUS status;

    //
    // Make sure this driver is currently managing a ControllerHandle
    //
    status = EfiTestManagedDevice(
        ControllerHandle,
        gVmbusDriverBindingProtocol.DriverBindingHandle,
        &gEfiVmbusRootProtocolGuid
        );
    if (EFI_ERROR(status))
    {
        return status;
    }

    //
    // ChildHandle must be NULL for a Device Driver
    //
    if (ChildHandle != NULL)
    {
        return EFI_UNSUPPORTED;
    }

    return LookupUnicodeString2(
        Language,
        This->SupportedLanguages,
        gVmbusControllerNameTable,
        ControllerName,
        (BOOLEAN)(This == &gVmbusComponentName));
}


EFI_STATUS
EFIAPI
VmbusDriverInitialize (
    __in EFI_HANDLE ImageHandle,
    __in EFI_SYSTEM_TABLE *SystemTable
    )
/*++

Routine Description:

    Entry point into VMBus driver.

Arguments:

    ImageHandle - Handle of the driver image.

    SystemTable - EFI system table.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;

    DEBUG((DEBUG_VERBOSE, ">>> %a\n", __FUNCTION__));

    mVmbusImageHandle = ImageHandle;

    //
    // Determine which GUID will be used for the legacy interface.  The legacy
    // protocol is available in all VMs, but the GUID used to expose it
    // differs between isolated and non-isolated VMs.  This is required to
    // ensure that isolated VMs are correclty opting into the required
    // isolation behavior of the legacy protocol.
    //

    if (PcdGet32(PcdIsolationArchitecture) == UefiIsolationTypeNone)
    {
        mVmbusLegacyProtocolGuid = &gEfiVmbusLegacyProtocolGuid;
    }
    else
    {
        mVmbusLegacyProtocolGuid = &gEfiVmbusLegacyProtocolIvmGuid;
    }

    //
    // Install the VMBus root controller tag and device path protocols onto a
    // new root device handle.
    //
    gVmbusRootDevicePath.VmbusRootNode = gVmbusRootNode;
    gVmbusRootDevicePath.End = gEfiEndNode;

    status = gBS->InstallMultipleProtocolInterfaces(&mRootDevice,
                                                    &gEfiVmbusRootProtocolGuid,
                                                    NULL,
                                                    &gEfiDevicePathProtocolGuid,
                                                    &gVmbusRootDevicePath,
                                                    NULL);
    if (EFI_ERROR(status))
    {
        return status;
    }

    //
    // Install the DriverBinding and Component Name protocols onto the driver image handle.
    //
    status = EfiLibInstallDriverBindingComponentName2(ImageHandle,
                                                      SystemTable,
                                                      &gVmbusDriverBindingProtocol,
                                                      ImageHandle,
                                                      &gVmbusComponentName,
                                                      &gVmbusComponentName2
                                                      );
    if (EFI_ERROR(status))
    {
        return status;
    }

    DEBUG((DEBUG_VERBOSE, "<<< %a\n", __FUNCTION__));
    return EFI_SUCCESS;
}

