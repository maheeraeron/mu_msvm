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
#include <IsolationTypes.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>

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
VmbusChannelPrepareGpadl (
    __in EFI_VMBUS_PROTOCOL *This,
    __in_bcount(BufferLength) VOID *Buffer,
    __in UINT32 BufferLength,
    __in BOOLEAN ZeroPages,
    __in HV_MAP_GPA_FLAGS MapFlags,
    __out EFI_VMBUS_GPADL **Gpadl
    )
/*++

Routine Description:

    This routine prepares a GPADL for use with the EFI VMBus protocol.

    This routine must be called at TPL < TPL_NOTIFY.

    For isolated guests, buffer address of the buffer may change after the
    pages are made visible to the host, so the caller is expected to call
    GetGpadlBuffer to determine the usable buffer address once this routine
    completes.

Arguments:

    This - Pointer to the VMBus protocol.

    Buffer - Buffer describing the GPADL to create.

    BufferLength - Length of the buffer.

    ZeroPages - TRUE if the pages must be zero-filled upon return to the
                caller.

    MapFlags - Mapping flags to control the host visibility. Only HV_MAP_GPA_READABLE and
        HV_MAP_GPA_WRITABLE are valid in the access mask when isolation is used.

    Gpadl - Returns a pointer to the GPADL.

Return Value:

    EFI_STATUS.

--*/
{
    VMBUS_CHANNEL_CONTEXT *channelContext;
    EFI_VMBUS_GPADL *gpadl;
    EFI_STATUS status;

    if (BufferLength == 0)
    {
        return EFI_INVALID_PARAMETER;
    }

    //
    // GPADLs that could be subject to isolation restrictions must be aligned
    // to page boundaries.
    //

    if ((((UINTN)Buffer & (EFI_PAGE_SIZE - 1)) != 0) ||
        ((BufferLength & (EFI_PAGE_SIZE - 1)) != 0))
    {
        return EFI_INVALID_PARAMETER;
    }

    channelContext = CR(This,
                        VMBUS_CHANNEL_CONTEXT,
                        VmbusProtocol,
                        VMBUS_CHANNEL_CONTEXT_SIGNATURE);

    //
    // Allocate a structure to track the state of the GPADL.
    //

    gpadl = AllocatePool(sizeof(*gpadl));
    if (gpadl == NULL)
    {
        status = EFI_OUT_OF_RESOURCES;
        goto Cleanup;
    }

    gpadl->AllocatedBuffer = Buffer;
    gpadl->BufferLength = BufferLength;
    gpadl->NumberOfPages = (UINT32)((UINTN)BufferLength >> EFI_PAGE_SHIFT);
    gpadl->GpadlHandle = 0;
    gpadl->Legacy = FALSE;

    //
    // Make the entire buffer visible to the host if required.
    //

    if (PcdGet32(PcdIsolationArchitecture) != UefiIsolationTypeNone)
    {
        status = mHvIvm->MakeAddressRangeHostVisible(mHvIvm,
                                                     MapFlags,
                                                     Buffer,
                                                     BufferLength,
                                                     ZeroPages,
                                                     &gpadl->ProtectionHandle);

        if (EFI_ERROR(status))
        {
            goto Cleanup;
        }

        gpadl->VisibleBuffer = (PVOID)((UINTN)Buffer + mSharedGpaBoundary);
    }
    else
    {
        gpadl->VisibleBuffer = Buffer;
        if (ZeroPages)
        {
            ZeroMem(Buffer, BufferLength);
        }
    }

    *Gpadl = gpadl;
    status = EFI_SUCCESS;

Cleanup:

    if (EFI_ERROR(status))
    {
        if (gpadl != NULL)
        {
            FreePool(gpadl);
            gpadl = NULL;
        }
    }

    return status;
}


EFI_STATUS
EFIAPI
VmbusChannelCreateGpadl (
    __in EFI_VMBUS_PROTOCOL *This,
    __in EFI_VMBUS_GPADL *Gpadl
    )
/*++

Routine Description:

    This routine implements GPADL creation for the EFI VMBus protocol.

    This routine must be called at TPL < TPL_NOTIFY.

    This routine receives a message from the host and therefore
    must validate this message before using it.

Arguments:

    This - Pointer to the VMBus protocol.

    Gpadl - A pointer to the GPADL being created.

Return Value:

    EFI_STATUS.

--*/
{
    VMBUS_CHANNEL_CONTEXT *channelContext;
    VMBUS_MESSAGE sendMessage;
    VMBUS_MESSAGE *receiveMessage;
    UINT32 numPfnInHeader;
    UINT32 numPfnInBody;
    UINT32 pfnIndex;
    UINT32 pfnSent;
    EFI_STATUS status;

    if (Gpadl->GpadlHandle != 0)
    {
        return EFI_INVALID_PARAMETER;
    }

    channelContext = CR(This,
                        VMBUS_CHANNEL_CONTEXT,
                        VmbusProtocol,
                        VMBUS_CHANNEL_CONTEXT_SIGNATURE);

    status = VmbusRootGetFreeGpadl(channelContext->RootContext,
                                   &Gpadl->GpadlHandle);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    //
    // Calculate how many pages the buffer spans and how many PFNs can fit in a
    // header and body packet.
    //

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
    sendMessage.GpadlHeader.Gpadl = Gpadl->GpadlHandle;
    sendMessage.GpadlHeader.RangeCount = 1;
    sendMessage.GpadlHeader.RangeBufLen = (UINT16)(sizeof(GPA_RANGE) +
        (Gpadl->NumberOfPages - 1) * sizeof(UINT64));

    sendMessage.GpadlHeader.Range[0].ByteCount = Gpadl->BufferLength;
    sendMessage.GpadlHeader.Range[0].ByteOffset =
        (UINT32)((UINTN)Gpadl->VisibleBuffer & EFI_PAGE_MASK);

    for (pfnIndex = 0; pfnIndex < MIN(Gpadl->NumberOfPages, numPfnInHeader); ++pfnIndex)
    {
        sendMessage.GpadlHeader.Range[0].PfnArray[pfnIndex] =
            ((UINTN)Gpadl->VisibleBuffer >> EFI_PAGE_SHIFT) + pfnIndex;
    }

    DEBUG((EFI_D_INFO,
        "%a(%dnumPages=%d base=%p gpadlHandle=0x%x\n",
        __FUNCTION__,
        __LINE__,
        Gpadl->NumberOfPages,
        (UINTN)Gpadl->VisibleBuffer >> EFI_PAGE_SHIFT,
        Gpadl->GpadlHandle));

    pfnSent = pfnIndex;
    VmbusRootSendMessage(&sendMessage);

    //
    // Keep sending GPADL body packets until we run out of PFNs to send.
    //

    while (Gpadl->NumberOfPages - pfnSent > 0)
    {
        VmbusRootInitializeMessage(&sendMessage,
                                   ChannelMessageGpadlBody,
                                   MAXIMUM_SYNIC_MESSAGE_BYTES);

        sendMessage.GpadlBody.Gpadl = Gpadl->GpadlHandle;
        for (pfnIndex = 0; pfnIndex < MIN(Gpadl->NumberOfPages - pfnSent, numPfnInBody); ++pfnIndex)
        {
            sendMessage.GpadlBody.Pfn[pfnIndex] =
                ((UINTN)Gpadl->VisibleBuffer >> EFI_PAGE_SHIFT) + pfnSent + pfnIndex;
        }

        pfnSent += pfnIndex;
        VmbusRootSendMessage(&sendMessage);
    }

    receiveMessage = NULL;
    status = VmbusRootWaitForGpadlResponse(channelContext->RootContext,
                                           Gpadl->GpadlHandle,
                                           &receiveMessage);

    ASSERT_EFI_ERROR(status);

    VMBUS_FAIL_FAST_IF_FALSE(receiveMessage->Header.MessageType == ChannelMessageGpadlCreated);
    VMBUS_FAIL_FAST_IF_FALSE(receiveMessage->Size == sizeof(receiveMessage->GpadlCreated));
    VMBUS_FAIL_FAST_IF_FALSE(receiveMessage->GpadlCreated.Gpadl == Gpadl->GpadlHandle);
    VMBUS_FAIL_FAST_IF_FALSE(receiveMessage->GpadlCreated.ChildRelId == channelContext->ChannelId);

    if (receiveMessage->GpadlCreated.CreationStatus != 0)
    {
        status = EFI_OUT_OF_RESOURCES;
        goto Cleanup;
    }

    status = EFI_SUCCESS;

Cleanup:
    if (EFI_ERROR(status))
    {
        if (Gpadl->GpadlHandle != 0)
        {
            VmbusRootReclaimGpadl(channelContext->RootContext, Gpadl->GpadlHandle);
            Gpadl->GpadlHandle = 0;
        }
    }

    return status;
}


EFI_STATUS
EFIAPI
VmbusChannelCreateGpadlLegacy (
    __in EFI_VMBUS_LEGACY_PROTOCOL *This,
    __in_bcount(BufferLength) VOID *Buffer,
    __in UINT32 BufferLength,
    __out UINT32 *GpadlHandle
    )
/*++

Routine Description:

    This routine implements GPADL creation for the legacy EFI VMBus protocol.

    This routine must be called at TPL < TPL_NOTIFY.

Arguments:

    This - Pointer to the legacy VMBus protocol.

    Buffer - Buffer describing the GPADL to create.

    BufferLength - Length of the buffer.

    GpadlHandle - Returns a handle to the GPADL.

Return Value:

    EFI_STATUS.

--*/
{
    VMBUS_CHANNEL_CONTEXT *channelContext;
    EFI_VMBUS_GPADL gpadl;
    EFI_STATUS status;

    ZeroMem(&gpadl, sizeof(gpadl));
    if (BufferLength == 0)
    {
        return EFI_INVALID_PARAMETER;
    }

    channelContext = CR(This,
                        VMBUS_CHANNEL_CONTEXT,
                        LegacyVmbusProtocol,
                        VMBUS_CHANNEL_CONTEXT_SIGNATURE);

    gpadl.AllocatedBuffer = Buffer;
    gpadl.VisibleBuffer = Buffer;
    gpadl.BufferLength = BufferLength;
    gpadl.NumberOfPages = ((UINT32)((UINTN)Buffer & EFI_PAGE_MASK) + BufferLength +
        EFI_PAGE_SIZE - 1) >> EFI_PAGE_SHIFT;
    gpadl.Legacy = TRUE;

    status = VmbusChannelCreateGpadl(&channelContext->VmbusProtocol, &gpadl);
    if (!EFI_ERROR(status))
    {
        *GpadlHandle = gpadl.GpadlHandle;
    }

    return status;
}


EFI_STATUS
EFIAPI
VmbusChannelDestroyGpadl (
    __in EFI_VMBUS_PROTOCOL *This,
    __in EFI_VMBUS_GPADL *Gpadl
    )
/*++

Routine Description:

    This routine implements GPADL destruction for the EFI VMBus protocol.

    This routine must be called at TPL < TPL_NOTIFY.

    The buffer address is guaranteed to be restored to whatever value it had
    prior to the call to PrepareGpadl, so it can be freed correctly by the
    caller based on its original address.

    This routine receives a message from the host and therefore
    must validate this message before using it.

Arguments:

    This - Pointer to the VMBus protocol.

    Gpadl - Pointer to the GPADL to destroy.

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

    if (Gpadl->GpadlHandle != 0)
    {
        if (!VmbusRootValidateGpadl(channelContext->RootContext, Gpadl->GpadlHandle))
        {
            return EFI_INVALID_PARAMETER;
        }

        VmbusRootInitializeMessage(&sendMessage,
                                   ChannelMessageGpadlTeardown,
                                   sizeof(sendMessage.GpadlTeardown));

        sendMessage.GpadlTeardown.ChildRelId = channelContext->ChannelId;
        sendMessage.GpadlTeardown.Gpadl = Gpadl->GpadlHandle;
        VmbusRootSendMessage(&sendMessage);
        status = VmbusRootWaitForGpadlResponse(channelContext->RootContext,
                                               Gpadl->GpadlHandle,
                                               &receiveMessage);

        ASSERT_EFI_ERROR(status);

        VMBUS_FAIL_FAST_IF_FALSE(receiveMessage->Size == sizeof(receiveMessage->GpadlTorndown));
        VMBUS_FAIL_FAST_IF_FALSE(receiveMessage->Header.MessageType == ChannelMessageGpadlTorndown);
        VMBUS_FAIL_FAST_IF_FALSE(receiveMessage->GpadlTorndown.Gpadl == Gpadl->GpadlHandle);

        VmbusRootReclaimGpadl(channelContext->RootContext, Gpadl->GpadlHandle);
        Gpadl->GpadlHandle = 0;
    }

    //
    // Revoke host visibility on these pages as they may be reused once the
    // GPADL has been deleted.
    //

    if (PcdGet32(PcdIsolationArchitecture) != UefiIsolationTypeNone)
    {
        ASSERT(!Gpadl->Legacy);
        mHvIvm->MakeAddressRangeNotHostVisible(mHvIvm, Gpadl->ProtectionHandle);
    }

    //
    // Legacy GPADL objects are not allocated; they are stack locals in the
    // legacy entry points, and therefore should not be freed.  All other
    // GPADL objects were allocated by PrepareGpadl and therefore should be
    // freed here.
    //

    if (!Gpadl->Legacy)
    {
        FreePool(Gpadl);
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
VmbusChannelDestroyGpadlLegacy (
    __in EFI_VMBUS_LEGACY_PROTOCOL *This,
    __in UINT32 GpadlHandle
    )
/*++

Routine Description:

    This routine implements GPADL destruction for the legacy EFI VMBus protocol.

    This routine must be called at TPL < TPL_NOTIFY.

Arguments:

    This - Pointer to the legacy VMBus protocol.

    GpadlHandle - Handle of the GPADL to destroy.

Return Value:

    EFI_STATUS.

--*/
{
    VMBUS_CHANNEL_CONTEXT *channelContext;
    EFI_VMBUS_GPADL gpadl;

    channelContext = CR(This,
                        VMBUS_CHANNEL_CONTEXT,
                        LegacyVmbusProtocol,
                        VMBUS_CHANNEL_CONTEXT_SIGNATURE);

    gpadl.AllocatedBuffer = NULL;
    gpadl.VisibleBuffer = NULL;
    gpadl.BufferLength = 0;
    gpadl.NumberOfPages = 0;
    gpadl.GpadlHandle = GpadlHandle;
    gpadl.Legacy = TRUE;

    return VmbusChannelDestroyGpadl(&channelContext->VmbusProtocol, &gpadl);

}

UINT32
EFIAPI
VmbusChannelGetGpadlHandle(
    __in EFI_VMBUS_PROTOCOL *This,
    __in EFI_VMBUS_GPADL *Gpadl
    )
/*++

Routine Description:

    This routine retrieves the GPADL handle associated with a GPADL.

Arguments:

    This - Pointer to the VMBus protocol.

    Gpadl - Pointer to the GPADL.

Return Value:

    GPADL handle.

--*/
{
    return Gpadl->GpadlHandle;
}


PVOID
EFIAPI
VmbusChannelGetGpadlBuffer(
    __in EFI_VMBUS_PROTOCOL *This,
    __in EFI_VMBUS_GPADL *Gpadl
    )
/*++

Routine Description:

    This routine retrieves the usable GPADL buffer pointer associated with
    a GPADL.

Arguments:

    This - Pointer to the VMBus protocol.

    Gpadl - Pointer to the GPADL.

Return Value:

    GPADL buffer.

--*/
{
    return Gpadl->VisibleBuffer;
}


EFI_STATUS
EFIAPI
VmbusChannelOpenChannel (
    __in EFI_VMBUS_PROTOCOL *This,
    __in EFI_VMBUS_GPADL *RingBufferGpadl,
    __in UINT32 RingBufferPageOffset
    )
/*++

Routine Description:

    This routine implements channel opening for the EFI VMBus protocol.

    This routine must be called at TPL < TPL_NOTIFY.

    This routine receives a message from the host and therefore
    must validate this message before using it.

Arguments:

    This - Pointer to the VMBus protocol.

    RingBufferGpadl - Pointer to the GPADL describing the channel ring buffers.

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
    sendMessage.OpenChannel.RingBufferGpadlHandle = RingBufferGpadl->GpadlHandle;
    sendMessage.OpenChannel.DownstreamRingBufferPageOffset = RingBufferPageOffset;
    sendMessage.OpenChannel.TargetVp = mHv->GetCurrentVpIndex(mHv);
    VmbusRootSendMessage(&sendMessage);
    receiveMessage = VmbusRootWaitForChannelResponse(channelContext);

    VMBUS_FAIL_FAST_IF_FALSE(receiveMessage->Size == sizeof(receiveMessage->OpenResult));
    VMBUS_FAIL_FAST_IF_FALSE(receiveMessage->Header.MessageType == ChannelMessageOpenChannelResult);
    VMBUS_FAIL_FAST_IF_FALSE(receiveMessage->OpenResult.ChildRelId == channelContext->ChannelId);

    if (receiveMessage->OpenResult.Status != 0)
    {
        return EFI_OUT_OF_RESOURCES;
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
VmbusChannelOpenChannelLegacy (
    __in EFI_VMBUS_LEGACY_PROTOCOL *This,
    __in UINT32 RingBufferGpadlHandle,
    __in UINT32 RingBufferPageOffset
    )
/*++

Routine Description:

    This routine implements channel opening for the legacy EFI VMBus protocol.

    This routine must be called at TPL < TPL_NOTIFY.

Arguments:

    This - Pointer to the legacy VMBus protocol.

    RingBufferGpadlHandle - Handle of the GPADL describing the channel ring
        buffers.

    RingBufferPageOffset - Page offset of the outgoing ring buffer.

Return Value:

    EFI_STATUS.

--*/
{
    VMBUS_CHANNEL_CONTEXT *channelContext;
    EFI_VMBUS_GPADL gpadl;

    channelContext = CR(This,
                        VMBUS_CHANNEL_CONTEXT,
                        LegacyVmbusProtocol,
                        VMBUS_CHANNEL_CONTEXT_SIGNATURE);

    gpadl.AllocatedBuffer = NULL;
    gpadl.VisibleBuffer = NULL;
    gpadl.BufferLength = 0;
    gpadl.NumberOfPages = 0;
    gpadl.GpadlHandle = RingBufferGpadlHandle;
    gpadl.Legacy = TRUE;

    return VmbusChannelOpenChannel(&channelContext->VmbusProtocol,
                                   &gpadl,
                                   RingBufferPageOffset);
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
VmbusChannelCloseChannelLegacy (
    __in EFI_VMBUS_LEGACY_PROTOCOL *This
    )
/*++

Routine Description:

    This routine implements channel closing for the legacy EFI VMBus protocol.

Arguments:

    This - Pointer to the legacy VMBus protocol.


Return Value:

    EFI_STATUS.

--*/
{
    VMBUS_CHANNEL_CONTEXT *channelContext;

    channelContext = CR(This,
                        VMBUS_CHANNEL_CONTEXT,
                        LegacyVmbusProtocol,
                        VMBUS_CHANNEL_CONTEXT_SIGNATURE);

    return VmbusChannelCloseChannel(&channelContext->VmbusProtocol);
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
VmbusChannelRegisterIsrLegacy(
    __in EFI_VMBUS_LEGACY_PROTOCOL *This,
    __in_opt EFI_EVENT Event
    )
/*++

Routine Description:

    This routine implements connection of interrupts for the legacy EFI VMBus protocol.

Arguments:

    This - Pointer to the legacy VMBus protocol.

    Event - Event to be signalled upon interrupt.

Return Value:

    EFI_STATUS.

--*/
{
    VMBUS_CHANNEL_CONTEXT *channelContext;

    channelContext = CR(This,
                        VMBUS_CHANNEL_CONTEXT,
                        LegacyVmbusProtocol,
                        VMBUS_CHANNEL_CONTEXT_SIGNATURE);

    return VmbusChannelRegisterIsr(&channelContext->VmbusProtocol, Event);
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


EFI_STATUS
EFIAPI
VmbusChannelSendInterruptLegacy (
    __in EFI_VMBUS_LEGACY_PROTOCOL *This
    )
/*++

Routine Description:

    This routine implements sending an interrupt to the opposite endpoint for
    the legacy EFI VMBus protocol.

Arguments:

    This - Pointer to the legacy VMBus protocol.

Return Value:

    EFI_STATUS.

--*/
{
    VMBUS_CHANNEL_CONTEXT *channelContext;

    channelContext = CR(This,
                        VMBUS_CHANNEL_CONTEXT,
                        LegacyVmbusProtocol,
                        VMBUS_CHANNEL_CONTEXT_SIGNATURE);

    return VmbusChannelSendInterrupt(&channelContext->VmbusProtocol);
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

    ChannelContext->VmbusProtocol.PrepareGpadl = VmbusChannelPrepareGpadl;
    ChannelContext->VmbusProtocol.CreateGpadl = VmbusChannelCreateGpadl;
    ChannelContext->VmbusProtocol.DestroyGpadl = VmbusChannelDestroyGpadl;
    ChannelContext->VmbusProtocol.GetGpadlHandle = VmbusChannelGetGpadlHandle;
    ChannelContext->VmbusProtocol.GetGpadlBuffer = VmbusChannelGetGpadlBuffer;
    ChannelContext->VmbusProtocol.OpenChannel = VmbusChannelOpenChannel;
    ChannelContext->VmbusProtocol.CloseChannel = VmbusChannelCloseChannel;
    ChannelContext->VmbusProtocol.RegisterIsr = VmbusChannelRegisterIsr;
    ChannelContext->VmbusProtocol.SendInterrupt = VmbusChannelSendInterrupt;

    ChannelContext->LegacyVmbusProtocol.CreateGpadl = VmbusChannelCreateGpadlLegacy;
    ChannelContext->LegacyVmbusProtocol.DestroyGpadl = VmbusChannelDestroyGpadlLegacy;
    ChannelContext->LegacyVmbusProtocol.OpenChannel = VmbusChannelOpenChannelLegacy;
    ChannelContext->LegacyVmbusProtocol.CloseChannel = VmbusChannelCloseChannelLegacy;
    ChannelContext->LegacyVmbusProtocol.RegisterIsr = VmbusChannelRegisterIsrLegacy;
    ChannelContext->LegacyVmbusProtocol.SendInterrupt = VmbusChannelSendInterruptLegacy;

    if (Offer->Flags & VMBUS_OFFER_FLAG_NAMED_PIPE_MODE)
    {
        ChannelContext->VmbusProtocol.Flags |= EFI_VMBUS_PROTOCOL_FLAGS_PIPE_MODE;
        ChannelContext->LegacyVmbusProtocol.Flags |= EFI_VMBUS_PROTOCOL_FLAGS_PIPE_MODE;
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

