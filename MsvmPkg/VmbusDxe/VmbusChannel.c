/*++

Copyright (c) Microsoft Corporation

Module Name:

    VmbusChannel.c

Abstract:

    Provides the implementation of the VMBus EFI protocol.

Author:

    Arseney Romanenko (arseneyr) - 7-Jul-2012

--*/

#include <VmbusP.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/DevicePathLib.h>

VMBUS_DEVICE_PATH gVmbusChannelNode =
{
    {
        {
            HARDWARE_DEVICE_PATH,
            HW_VENDOR_DP,
            {
                (UINT8) (sizeof (VMBUS_DEVICE_PATH)),
                (UINT8) ((sizeof (VMBUS_DEVICE_PATH)) >> 8)
            }
        },
        EFI_VMBUS_CHANNEL_DEVICE_PATH_GUID
    },
    0,
    0
};


EFI_STATUS
EFIAPI
VmbusChannelCreateGpadl (
    __in EFI_VMBUS_PROTOCOL *This,
    __in_bcount(BufferLength) VOID *Buffer,
    __in UINT32 BufferLength,
    __out UINT32 *GpadlHandle
    )
/*++

Routine Description:

    This routine implements GPADL creation for the EFI VMBus protocol.

    This routine must be called at TPL < TPL_NOTIFY.

Arguments:

    This - Pointer to the VMBus protocol.

    Buffer - Buffer describing the GPADL to create.

    BufferLength - Length of the buffer.

    GpadlHandle - Returns a handle to the GPADL.

Return Value:

    EFI_STATUS.

--*/
{
    VMBUS_CHANNEL_CONTEXT *channelContext;
    VMBUS_MESSAGE sendMessage;
    VMBUS_MESSAGE *receiveMessage;
    UINT32 gpadlHandle;
    UINT32 numPages;
    UINT32 numPfnInHeader;
    UINT32 numPfnInBody;
    UINT32 pfnIndex;
    UINT32 pfnSent;
    EFI_STATUS status;

    gpadlHandle = 0;

    if (BufferLength == 0)
    {
        return EFI_INVALID_PARAMETER;
    }

    channelContext = CR(This,
                        VMBUS_CHANNEL_CONTEXT,
                        VmbusProtocol,
                        VMBUS_CHANNEL_CONTEXT_SIGNATURE);

    status = VmbusRootGetFreeGpadl(channelContext->RootContext,
                                   &gpadlHandle);

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    //
    // Calculate how many pages the buffer spans and how many PFNs can fit in a
    // header and body packet.
    //

    numPages = ((UINT32)((UINTN)Buffer & EFI_PAGE_MASK) + BufferLength +
        EFI_PAGE_SIZE - 1) >> EFI_PAGE_SHIFT;

    numPfnInHeader = (MAXIMUM_SYNIC_MESSAGE_BYTES -
                      OFFSET_OF(VMBUS_CHANNEL_GPADL_HEADER, Range) -
                      OFFSET_OF(GPA_RANGE, PfnArray)) / sizeof(UINT64);

    numPfnInBody = (MAXIMUM_SYNIC_MESSAGE_BYTES -
                    OFFSET_OF(VMBUS_CHANNEL_GPADL_BODY, Pfn)) / sizeof(UINT64);

    //
    // Create the GPADL header.
    //

    VmbusRootInitializeMessage(&sendMessage,
                               ChannelMessageGpadlHeader,
                               MAXIMUM_SYNIC_MESSAGE_BYTES);

    sendMessage.GpadlHeader.ChildRelId = channelContext->ChannelId;
    sendMessage.GpadlHeader.Gpadl = gpadlHandle;
    sendMessage.GpadlHeader.RangeCount = 1;
    sendMessage.GpadlHeader.RangeBufLen = (UINT16)(sizeof(GPA_RANGE) +
        (numPages - 1) * sizeof(UINT64));

    sendMessage.GpadlHeader.Range[0].ByteCount = BufferLength;
    sendMessage.GpadlHeader.Range[0].ByteOffset =
        (UINT32)((UINTN)Buffer & EFI_PAGE_MASK);

    for (pfnIndex = 0; pfnIndex < MIN(numPages, numPfnInHeader); ++pfnIndex)
    {
        sendMessage.GpadlHeader.Range[0].PfnArray[pfnIndex] =
            ((UINTN)Buffer >> EFI_PAGE_SHIFT) + pfnIndex;
    }

    // Call the hypervisor to make these pages host visible
    if (PcdGetBool(PcdSystemIsolated))
    {
        EFI_STATUS modifyStatus;
        UINT32 pageCountProcessed = 0;
        HV_MAP_GPA_FLAGS mapFlags = HV_MAP_GPA_READABLE | HV_MAP_GPA_WRITABLE;

        modifyStatus = mHvIvm->ModifySparseGpaPageHostVisibility(mHvIvm,
                                                                mapFlags,
                                                                numPages,
                                                                ((UINTN)Buffer >> EFI_PAGE_SHIFT),
                                                                &pageCountProcessed);
        if (EFI_ERROR(status))
        {
            DEBUG((EFI_D_ERROR,
                "%a(%d) ModifySparseGpaPageHostVisibility returned status 0x%x numPages=%d pageCountProcessed=%d\n",
                __FUNCTION__,
                __LINE__,
                modifyStatus,
                numPages,
                pageCountProcessed));

            // TODO-19259739: Have a better way of reporting UEFI errors.
            ASSERT(FALSE);
            CpuDeadLoop();
        }
        else
        {
            // Record the pages made host-visible here so they can be revoked when UEFI exits
            VmbusRootSetGpadlPageRange(channelContext->RootContext,
                                       gpadlHandle,
                                       (UINTN)Buffer >> EFI_PAGE_SHIFT,
                                       numPages);
        }
    }
    else
    {
        DEBUG((EFI_D_INFO,
            "%a(%d) not isolated numPages=%d base=%p gpadlHandle=0x%x\n",
            __FUNCTION__,
            __LINE__,
            numPages,
            (UINTN)Buffer >> EFI_PAGE_SHIFT,
            gpadlHandle));

        VmbusRootSetGpadlPageRange(channelContext->RootContext,
                                   gpadlHandle,
                                   0,
                                   0);
    }

    pfnSent = pfnIndex;
    VmbusRootSendMessage(&sendMessage);

    //
    // Keep sending GPADL body packets until we run out of PFNs to send.
    //

    while (numPages - pfnSent > 0)
    {
        VmbusRootInitializeMessage(&sendMessage,
                                   ChannelMessageGpadlBody,
                                   MAXIMUM_SYNIC_MESSAGE_BYTES);

        sendMessage.GpadlBody.Gpadl = gpadlHandle;
        for (pfnIndex = 0; pfnIndex < MIN(numPages - pfnSent, numPfnInBody); ++pfnIndex)
        {
            sendMessage.GpadlBody.Pfn[pfnIndex] =
                ((UINTN)Buffer >> EFI_PAGE_SHIFT) + pfnSent + pfnIndex;
        }

        pfnSent += pfnIndex;
        VmbusRootSendMessage(&sendMessage);
    }

    receiveMessage = NULL;
    status = VmbusRootWaitForGpadlResponse(channelContext->RootContext,
                                           gpadlHandle,
                                           &receiveMessage);

    ASSERT_EFI_ERROR(status);

    ASSERT(receiveMessage->Header.MessageType == ChannelMessageGpadlCreated);
    ASSERT(receiveMessage->GpadlCreated.ChildRelId == channelContext->ChannelId);
    ASSERT(receiveMessage->GpadlCreated.Gpadl == gpadlHandle);

    if (receiveMessage->GpadlCreated.CreationStatus != 0)
    {
        status = EFI_OUT_OF_RESOURCES;
        goto Cleanup;
    }

    *GpadlHandle = gpadlHandle;
    status = EFI_SUCCESS;

Cleanup:
    if (EFI_ERROR(status))
    {
        if (gpadlHandle != 0)
        {
            VmbusRootReclaimGpadl(channelContext->RootContext, gpadlHandle);
        }
    }

    return status;
}


EFI_STATUS
EFIAPI
VmbusChannelDestroyGpadl (
    __in EFI_VMBUS_PROTOCOL *This,
    __in UINT32 GpadlHandle
    )
/*++

Routine Description:

    This routine implements GPADL destruction for the EFI VMBus protocol.

    This routine must be called at TPL < TPL_NOTIFY.

Arguments:

    This - Pointer to the VMBus protocol.

    GpadlHandle - Handle of the GPADL to destroy.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;
    VMBUS_CHANNEL_CONTEXT *channelContext;
    VMBUS_MESSAGE sendMessage;
    VMBUS_MESSAGE *receiveMessage;

    channelContext = CR(This,
                        VMBUS_CHANNEL_CONTEXT,
                        VmbusProtocol,
                        VMBUS_CHANNEL_CONTEXT_SIGNATURE);

    if (!VmbusRootValidateGpadl(channelContext->RootContext, GpadlHandle))
    {
        return EFI_INVALID_PARAMETER;
    }

    VmbusRootInitializeMessage(&sendMessage,
                               ChannelMessageGpadlTeardown,
                               sizeof(sendMessage.GpadlTeardown));

    sendMessage.GpadlTeardown.ChildRelId = channelContext->ChannelId;
    sendMessage.GpadlTeardown.Gpadl = GpadlHandle;
    VmbusRootSendMessage(&sendMessage);
    status = VmbusRootWaitForGpadlResponse(channelContext->RootContext,
                                           GpadlHandle,
                                           &receiveMessage);

    ASSERT_EFI_ERROR(status);

    ASSERT(receiveMessage->Header.MessageType == ChannelMessageGpadlTorndown);
    ASSERT(receiveMessage->GpadlTorndown.Gpadl == GpadlHandle);

    VmbusRootReclaimGpadl(channelContext->RootContext, GpadlHandle);
    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
VmbusChannelOpenChannel (
    __in EFI_VMBUS_PROTOCOL *This,
    __in UINT32 RingBufferGpadlHandle,
    __in UINT32 RingBufferPageOffset
    )
/*++

Routine Description:

    This routine implements channel opening for the EFI VMBus protocol.

    This routine must be called at TPL < TPL_NOTIFY.

Arguments:

    This - Pointer to the VMBus protocol.

    RingBufferGpadlHandle - Handle of the GPADL describing the channel ring
        buffers.

    RingBufferPageOffset - Page offset of the outgoing ring buffer.

Return Value:

    EFI_STATUS.

--*/
{
    VMBUS_CHANNEL_CONTEXT *channelContext;
    VMBUS_MESSAGE sendMessage;
    VMBUS_MESSAGE *receiveMessage;

    channelContext = CR(This,
                        VMBUS_CHANNEL_CONTEXT,
                        VmbusProtocol,
                        VMBUS_CHANNEL_CONTEXT_SIGNATURE);

    VmbusRootInitializeMessage(&sendMessage,
                               ChannelMessageOpenChannel,
                               sizeof(sendMessage.OpenChannel));

    sendMessage.OpenChannel.ChildRelId = channelContext->ChannelId;
    sendMessage.OpenChannel.RingBufferGpadlHandle = RingBufferGpadlHandle;
    sendMessage.OpenChannel.DownstreamRingBufferPageOffset = RingBufferPageOffset;
    sendMessage.OpenChannel.TargetVp = mHv->GetCurrentVpIndex(mHv);
    VmbusRootSendMessage(&sendMessage);
    receiveMessage = VmbusRootWaitForChannelResponse(channelContext);

    ASSERT(receiveMessage->Header.MessageType == ChannelMessageOpenChannelResult);
    ASSERT(receiveMessage->OpenResult.ChildRelId == channelContext->ChannelId);

    if (receiveMessage->OpenResult.Status != 0)
    {
        return EFI_OUT_OF_RESOURCES;
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
VmbusChannelCloseChannel (
    __in EFI_VMBUS_PROTOCOL *This
    )
/*++

Routine Description:

    This routine implements channel closing for the EFI VMBus protocol.

Arguments:

    This - Pointer to the VMBus protocol.


Return Value:

    EFI_STATUS.

--*/
{
    VMBUS_CHANNEL_CONTEXT *channelContext;
    VMBUS_MESSAGE sendMessage;

    channelContext = CR(This,
                        VMBUS_CHANNEL_CONTEXT,
                        VmbusProtocol,
                        VMBUS_CHANNEL_CONTEXT_SIGNATURE);

    VmbusRootInitializeMessage(&sendMessage,
                               ChannelMessageCloseChannel,
                               sizeof(sendMessage.CloseChannel));

    sendMessage.CloseChannel.ChildRelId = channelContext->ChannelId;
    VmbusRootSendMessage(&sendMessage);
    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
VmbusChannelRegisterIsr(
    __in EFI_VMBUS_PROTOCOL *This,
    __in_opt EFI_EVENT Event
    )
/*++

Routine Description:

    This routine implements connection of interrupts for the EFI VMBus protocol.

Arguments:

    This - Pointer to the VMBus protocol.

    Event - Event to be signalled upon interrupt.

Return Value:

    EFI_STATUS.

--*/
{
    VMBUS_CHANNEL_CONTEXT *channelContext;

    channelContext = CR(This,
                        VMBUS_CHANNEL_CONTEXT,
                        VmbusProtocol,
                        VMBUS_CHANNEL_CONTEXT_SIGNATURE);

    if (Event != NULL)
    {
        VmbusRootSetInterruptEntry(channelContext->RootContext,
                                   channelContext->ChannelId,
                                   Event);
    }
    else
    {
        VmbusRootClearInterruptEntry(channelContext->RootContext,
                                     channelContext->ChannelId);
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
VmbusChannelSendInterrupt (
    __in EFI_VMBUS_PROTOCOL *This
    )
/*++

Routine Description:

    This routine implements sending an interrupt to the opposite endpoint for
    the EFI VMBus protocol.

Arguments:

    This - Pointer to the VMBus protocol.

Return Value:

    EFI_STATUS.

--*/
{
    VMBUS_CHANNEL_CONTEXT *channelContext;

    channelContext = CR(This,
                        VMBUS_CHANNEL_CONTEXT,
                        VmbusProtocol,
                        VMBUS_CHANNEL_CONTEXT_SIGNATURE);

    return mHv->SignalEvent(mHv,
                            channelContext->ConnectionId,
                            0);
}


VOID
VmbusChannelInitializeContext(
    __inout VMBUS_CHANNEL_CONTEXT *ChannelContext,
    __in VMBUS_CHANNEL_OFFER_CHANNEL *Offer,
    __in VMBUS_ROOT_CONTEXT *RootContext
    )
/*++

Routine Description:

    This routine initializes a channel context based on a VMBus offer message.

Arguments:

    ChannelContext - Pointer to the channel context to initialize.

    Offer - Pointer to the offer message received.

    RootContext - Pointer to the root context.

Return Value:

    None.

--*/
{
    ZeroMem(ChannelContext, sizeof(*ChannelContext));
    ChannelContext->Signature = VMBUS_CHANNEL_CONTEXT_SIGNATURE;
    InitializeListHead(&ChannelContext->Link);
    ChannelContext->DevicePath.VmbusRootNode = gVmbusRootNode;
    CopyMem(&ChannelContext->DevicePath.VmbusChannelNode,
            &gVmbusChannelNode,
            sizeof(VMBUS_DEVICE_PATH));

    //
    // Win8 and above forces dedicated interrupts.
    //

    ASSERT(Offer->IsDedicatedInterrupt);

    ChannelContext->DevicePath.VmbusChannelNode.InterfaceType = Offer->InterfaceType;
    ChannelContext->DevicePath.VmbusChannelNode.InterfaceInstance = Offer->InterfaceInstance;
    ChannelContext->DevicePath.End = gEfiEndNode;
    ChannelContext->ChannelId = Offer->ChildRelId;
    ChannelContext->ConnectionId.AsUINT32 = Offer->ConnectionId;
    ChannelContext->RootContext = RootContext;
    gBS->CreateEvent(0,
                     0,
                     NULL,
                     NULL,
                     &ChannelContext->Response.Event);

    ChannelContext->VmbusProtocol.CreateGpadl = VmbusChannelCreateGpadl;
    ChannelContext->VmbusProtocol.DestroyGpadl = VmbusChannelDestroyGpadl;
    ChannelContext->VmbusProtocol.OpenChannel = VmbusChannelOpenChannel;
    ChannelContext->VmbusProtocol.CloseChannel = VmbusChannelCloseChannel;
    ChannelContext->VmbusProtocol.RegisterIsr = VmbusChannelRegisterIsr;
    ChannelContext->VmbusProtocol.SendInterrupt = VmbusChannelSendInterrupt;
    if (Offer->Flags & VMBUS_OFFER_FLAG_NAMED_PIPE_MODE)
    {
        ChannelContext->VmbusProtocol.Flags |= EFI_VMBUS_PROTOCOL_FLAGS_PIPE_MODE;
    }
}


VOID
VmbusChannelDestroyContext(
    __in VMBUS_CHANNEL_CONTEXT *ChannelContext
    )
/*++

Routine Description:

    This routine destroys a channel context.

Arguments:

    ChannelContext - Pointer to the channel context to destroy.

Return Value:

    None.

--*/
{
    if (ChannelContext->Response.Event != NULL)
    {
        gBS->CloseEvent(ChannelContext->Response.Event);
        ChannelContext->Response.Event = NULL;
    }
}

