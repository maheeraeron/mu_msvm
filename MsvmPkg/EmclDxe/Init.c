/*----------------------------------------------------------------------------
 $Microsoft Confidential$
 $Copyright (C) 2004 Microsoft Corporation.  All Rights Reserved.$

 $File: Init.c $

 Abstract:

     This file implements setup and teardown parts of the VMBus transport
     library

 Environment:

     Kernel mode or user mode

----------------------------------------------------------------------------*/

#include "transportp.h"

#define RING_BUFFER_POOL_TAG 'gnrV'

#ifndef VMBUS_RING_BUFFER_SINGLE_MAPPED

#include <dvrtl.h>

NTSTATUS
PkpDoubleMapBuffer(
    __in_bcount(BufferSize) PVOID  Buffer,
    __in                    UINT32 BufferSize,
    __out                   PMDL*  DoubleMdl,
    __out                   PVOID* DoubleBuffer
    );

VOID
PkpFreeDoubleMappedBuffer(
    __in PMDL  DoubleMdl
    );


#pragma alloc_text(PAGE, PkInitializeDoubleMappedRingBuffer)
#pragma alloc_text(PAGE, PkInitializeRingBuffer)
#pragma alloc_text(PAGE, PkUninitializeRingBuffer)
#pragma alloc_text(PAGE, PkInit)
#pragma alloc_text(PAGE, PkCleanup)
#pragma alloc_text(PAGE, PkpDoubleMapBuffer)
#pragma alloc_text(PAGE, PkpFreeDoubleMappedBuffer)

NTSTATUS
PkInitializeDoubleMappedRingBuffer(
    __out PPACKET_LIB_CONTEXT Context,
    __in_bcount(PAGE_SIZE) PVOID IncomingControl,
    __in_bcount(PAGE_SIZE * IncomingDataPageCount * 2) PVOID IncomingDataPages,
    __in_range(>, 0) UINT32 IncomingDataPageCount,
    __in_bcount(PAGE_SIZE) PVOID OutgoingControl,
    __in_bcount(PAGE_SIZE * OutgoingDataPageCount * 2) PVOID OutgoingDataPages,
    __in_range(>, 0) UINT32 OutgoingDataPageCount
    )
/*++

Routine Description:

    Initializes a ring buffer structure.

Arguments:

    Context - A pointer to the packet library context structure to initialize.

    IncomingControl - A pointer to the incoming control region.

    IncomingDataPages - A pointer to the incoming data pages, mapped twice
        contiguously in virtual address space.

    IncomingDataPageCount - The size of the incoming data buffer, measured in
        pages.

    OutgoingControl - A pointer to the outgoing control region.

    OutgoingDataPages - A pointer to the outgoing data pages, mapped twice
        contiguously in virtual address space.

    OutgoingDataPageCount - The size of the outgoing buffer, measured in
        pages.

Return Value:

    NTSTATUS.

--*/
{
    PAGED_CODE();

    RtlZeroMemory(Context, sizeof(*Context));
    Context->Incoming.Control = (PVMRCB)IncomingControl;
    Context->Incoming.Data = IncomingDataPages;
    Context->Incoming.DataBytesInRing = IncomingDataPageCount * PAGE_SIZE;
    Context->Outgoing.Control = (PVMRCB)OutgoingControl;
    Context->Outgoing.Data = OutgoingDataPages;
    Context->Outgoing.DataBytesInRing = OutgoingDataPageCount * PAGE_SIZE;
    Context->InterruptMaskSkips = &Context->StaticInterruptMaskSkips;
    return PkpInitRingBufferControl(Context);
}


NTSTATUS
PkpDoubleMapBuffer(
    __in_bcount(BufferSize) PVOID  Buffer,
    __in                    UINT32 BufferSize,
    __out                   PMDL*  DoubleMdl,
    __out                   PVOID* DoubleBuffer
    )
/*++

Routine Description:

    This routine creates a new buffer which maps the same physical
    pages as the original buffer, but does so twice in a row.
    This allows us to simplify operations where copying into the
    buffer would "loop" to the beginning of the buffer for a
    portion of the copy.

 Arguments:

    Buffer - Pre-allocated input buffer which will be double mapped by the
        output buffer.

    BufferSize - Size of the original Buffer in bytes.

    DoubleMdl - Pointer to a PMDL used to describe the new double mapped buffer.
        This is used to later unmap the new buffer.

    DoubleBuffer - Virtual address of the new, double-mapped buffer backed
        by the original buffer's physical pages.

Return Value:

    NTSTATUS

--*/
{
    PPFN_NUMBER pfnArray;
    PMDL mdl;
    NTSTATUS status;
    UINT32 pageCount;
    BOOLEAN locked;
    ULONG priority;

    PAGED_CODE();

    mdl = NULL;
    locked = FALSE;

    //
    // Make sure the buffer is page aligned and its size is a
    // multiple of PAGE_SIZE.
    //

    ASSERT(Buffer == PAGE_ALIGN(Buffer));

    ASSERT((BufferSize % PAGE_SIZE) == 0);

    ASSERT(BufferSize >= PAGE_SIZE);

    //
    // Allocate a MDL with enough space for the double mapping.
    //

    mdl = IoAllocateMdl(Buffer, BufferSize * 2, FALSE, FALSE, NULL);
    if (mdl == NULL)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    //
    // Temporarily restrict the byte count and probe and lock the pages.
    //

    mdl->ByteCount = BufferSize;

    __try
    {
        MmProbeAndLockPages(mdl, KernelMode, IoModifyAccess);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = GetExceptionCode();
        goto Cleanup;
    }

    locked = TRUE;

    //
    // Duplicate the array to fill in the second half. This is safe now that
    // the pages are locked down, but care must be taken to un-double the
    // MDL before unlocking the pages.
    //

    pfnArray = MmGetMdlPfnArray(mdl);
    pageCount = BufferSize / PAGE_SIZE;
    RtlCopyMemory(&pfnArray[pageCount], &pfnArray[0], sizeof(PFN_NUMBER) * pageCount);
    mdl->ByteCount = BufferSize * 2;

    //
    // Double map the pages.
    //

    priority = NormalPagePriority;
    if (DvIsNtDdiVersionAvailable(NTDDI_WIN8))
    {
        priority |= MdlMappingNoExecute;
    }

    if (MmMapLockedPagesSpecifyCache(
        mdl,
        KernelMode,
        MmCached,
        NULL,
        0,
        priority) == NULL)
    {
        //
        // Un-double the MDL before failing to ensure that the pages are
        // not double unlocked.
        //

        mdl->ByteCount /= 2;
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    *DoubleMdl = mdl;
    *DoubleBuffer = mdl->MappedSystemVa;
    mdl = NULL;
    status = STATUS_SUCCESS;

Cleanup:

    if (mdl != NULL)
    {
        if (locked)
        {
            MmUnlockPages(mdl);
        }

        IoFreeMdl(mdl);
    }

    return status;
}


VOID
PkpFreeDoubleMappedBuffer(
    __in PMDL  DoubleMdl
    )
/*++

Routine Description:

    This routine frees a double mapped buffer created by a call to
    PkpDoubleMapBuffer.

Arguments:

    DoubleMdl - MDL used to describe the double mapped buffer.

Return Value:

    None.

--*/
{
    PAGED_CODE();

    MmUnmapLockedPages(DoubleMdl->MappedSystemVa, DoubleMdl);

    //
    // Undouble the pages before unlocking them, since each page was only
    // locked once despite being used twice.
    //

    DoubleMdl->ByteCount /= 2;
    MmUnlockPages(DoubleMdl);
    IoFreeMdl(DoubleMdl);
}


NTSTATUS
PkInitializeRingBuffer(
    __out PPACKET_LIB_CONTEXT Context,
    __in_bcount(PAGE_SIZE) PVOID IncomingControl,
    __in_bcount(PAGE_SIZE * IncomingDataPageCount) PVOID IncomingDataPages,
    __in_range(>, 0) UINT32 IncomingDataPageCount,
    __in_bcount(PAGE_SIZE) PVOID OutgoingControl,
    __in_bcount(PAGE_SIZE * OutgoingDataPageCount) PVOID OutgoingDataPages,
    __in_range(>, 0) UINT32 OutgoingDataPageCount
    )
/*++

Routine Description:

    Initializes a ring buffer structure.

Arguments:

    Context - A pointer to the packet library context structure to initialize.

    IncomingControl - A pointer to the incoming control region.

    IncomingDataPages - A pointer to the incoming data pages. Must reside in
        non-paged pool.

    IncomingDataPageCount - The size of the incoming data buffer, measured in
        pages.

    OutgoingControl - A pointer to the outgoing control region.

    OutgoingDataPages - A pointer to the outgoing data pages. Must reside in
        non-paged pool.

    OutgoingDataPageCount - The size of the outgoing buffer, measured in
        pages.

Return Value:

    NTSTATUS.

--*/
{
    PVOID incomingDoubleMapped;
    PMDL incomingMdl;
    PVOID outgoingDoubleMapped;
    PMDL outgoingMdl;
    NTSTATUS status;

    PAGED_CODE();

    incomingMdl = NULL;
    outgoingMdl = NULL;

    //
    // Double map the data portion of the ring buffers so that all packets
    // can be contiguous in memory.
    //

    status = PkpDoubleMapBuffer(IncomingDataPages,
                                IncomingDataPageCount * PAGE_SIZE,
                                &incomingMdl,
                                &incomingDoubleMapped);
    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

    status = PkpDoubleMapBuffer(OutgoingDataPages,
                               OutgoingDataPageCount * PAGE_SIZE,
                               &outgoingMdl,
                               &outgoingDoubleMapped);

    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

    status = PkInitializeDoubleMappedRingBuffer(
        Context,
        IncomingControl,
        incomingDoubleMapped,
        IncomingDataPageCount,
        OutgoingControl,
        outgoingDoubleMapped,
        OutgoingDataPageCount);

    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

    Context->IncomingDoubleMappedMdl = incomingMdl;
    Context->OutgoingDoubleMappedMdl = outgoingMdl;
    status = STATUS_SUCCESS;

Cleanup:

    if (!NT_SUCCESS(status))
    {
        if (incomingMdl != NULL)
        {
            PkpFreeDoubleMappedBuffer(incomingMdl);
        }

        if (outgoingMdl != NULL)
        {
            PkpFreeDoubleMappedBuffer(outgoingMdl);
        }
    }

    return status;
}


VOID
PkUninitializeRingBuffer(
    __in PPACKET_LIB_CONTEXT PkLibContext
    )
/*++

Routine Description:

    Uninitializes a ring buffer context structure, freeing the double mapped
    ring MDLs.

Arguments:

    PkLibContext - A pointer to the context to uninitialize.

Return Value:

    None.

--*/
{
    PAGED_CODE();

    if (PkLibContext->IncomingDoubleMappedMdl != NULL)
    {
        PkpFreeDoubleMappedBuffer(PkLibContext->IncomingDoubleMappedMdl);
        PkLibContext->IncomingDoubleMappedMdl = NULL;
    }

    if (PkLibContext->OutgoingDoubleMappedMdl != NULL)
    {
        PkpFreeDoubleMappedBuffer(PkLibContext->OutgoingDoubleMappedMdl);
        PkLibContext->OutgoingDoubleMappedMdl = NULL;
    }
}


NTSTATUS
PkInit(
    __in_bcount(PAGE_SIZE * RingBufferPageCount)
                PVOID                  RingBufferPages,
    __in        UINT32                 RingBufferPageCount,
    __in        UINT32                 ClientToServerPages,
    __in        ENDPOINT_TYPE          EndpointType,
    __in_opt    UINT32                 IncomingTransactionQuota,
    __out __deref __drv_allocatesMem(Mem)
                PPACKET_LIB_CONTEXT   *PkLibContext
    )
/*++

Routine Description:

    This routine initializes the packet management library.  If this is a
    client endpoint, then it also sets up the ring buffer itself.  If this
    is a server endpoint, then the ring must be set up by the client endpoint
    before this call is made.

Arguments:

    RingBufferPages - Pointer to the allocated pages used as ring buffers.

    RingBufferPageCount - Number of allocated pages pointed to by RingBufferPages

    ClientToServerPages - Number of pages used for the client-to-server
        ring buffer.

    EndpointType - Server or client.

    IncomingTransactionQuota - If specified, this parameter limits the number
        of incoming transactions that can be in-flight.  This is measured by
        the number of packets that have come in with the
        VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED flag set minus the number
        of times that PkSendCompletion has been called.  If this threshold is
        met, then any new calls to PkReceivePacket will fail with
        STATUS_QUOTA_EXCEEDED.

    PkLibContext - Pointer to be filled in with the lib's context, which will
        be passed back into the lib on subsequent calls.

Return Value:

    NTSTATUS

--*/
{
    PPACKET_LIB_CONTEXT context;
    PVOID clientControl;
    PVOID serverControl;
    UINT32 serverToClientPages;
    NTSTATUS status;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(IncomingTransactionQuota);

    context = NULL;
    if (ClientToServerPages >= RingBufferPageCount)
    {
        status = STATUS_BUFFER_OVERFLOW;
        goto Cleanup;
    }

    serverToClientPages = RingBufferPageCount - ClientToServerPages;
    if ((ClientToServerPages < 2) ||
        (serverToClientPages < 2))
    {
        status = STATUS_BUFFER_OVERFLOW;
        goto Cleanup;
    }

    clientControl = RingBufferPages;
    serverControl = (PUCHAR)RingBufferPages + ClientToServerPages * PAGE_SIZE;
    context = ExAllocatePoolWithTag(NonPagedPool,
                                    sizeof(*context),
                                    RING_BUFFER_POOL_TAG);

    if (context == NULL)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    if (EndpointType == VmbusClientEndpoint)
    {
        RtlZeroMemory(RingBufferPages, RingBufferPageCount * PAGE_SIZE);
        status = PkInitializeRingBuffer(context,
                                        serverControl,
                                        (PUCHAR)serverControl + PAGE_SIZE,
                                        serverToClientPages - 1,
                                        clientControl,
                                        (PUCHAR)clientControl + PAGE_SIZE,
                                        ClientToServerPages - 1);
    }
    else
    {
        status = PkInitializeRingBuffer(context,
                                        clientControl,
                                        (PUCHAR)clientControl + PAGE_SIZE,
                                        ClientToServerPages - 1,
                                        serverControl,
                                        (PUCHAR)serverControl + PAGE_SIZE,
                                        serverToClientPages - 1);
    }

    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

    NT_ASSERT(IncomingTransactionQuota == 0);

    *PkLibContext = context;
    context = NULL;
    status = STATUS_SUCCESS;

Cleanup:
    if (context != NULL)
    {
        ExFreePoolWithTag(context, RING_BUFFER_POOL_TAG);
    }

    return status;
}


VOID
PkCleanup(
    __in __drv_freesMem(Mem) PPACKET_LIB_CONTEXT PkLibContext
    )
/*++

Routine Description:

    This routine frees any resources associated with the ring buffer library.

Arguments:

    PkLibContext - Struct containing all of the lib's context.

Return Value:

    none

--*/
{
    PAGED_CODE();

    PkUninitializeRingBuffer(PkLibContext);
    ExFreePoolWithTag(PkLibContext, RING_BUFFER_POOL_TAG);
}

#else

NTSTATUS
PkInitializeSingleMappedRingBuffer(
    __out PPACKET_LIB_CONTEXT Context,
    __in_bcount(PAGE_SIZE) PVOID IncomingControl,
    __in_bcount(PAGE_SIZE * IncomingDataPageCount) PVOID IncomingDataPages,
    __in_range(>, 0) UINT32 IncomingDataPageCount,
    __in_bcount(PAGE_SIZE) PVOID OutgoingControl,
    __in_bcount(PAGE_SIZE * OutgoingDataPageCount) PVOID OutgoingDataPages,
    __in_range(>, 0) UINT32 OutgoingDataPageCount
    )
/*++

Routine Description:

    Initializes a single-mapped ring buffer structure. This functions differs from 
    PkInitializeDoubleMappedRingBuffer only in the SAL annotation.

Arguments:

    Context - A pointer to the packet library context structure to initialize.

    IncomingControl - A pointer to the incoming control region.

    IncomingDataPages - A pointer to the incoming data pages.

    IncomingDataPageCount - The size of the incoming data buffer, measured in
        pages.

    OutgoingControl - A pointer to the outgoing control region.

    OutgoingDataPages - A pointer to the outgoing data pages.

    OutgoingDataPageCount - The size of the outgoing buffer, measured in
        pages.

Return Value:

    NTSTATUS.

--*/
{
    PAGED_CODE();

    RtlZeroMemory(Context, sizeof(*Context));
    Context->Incoming.Control = (PVMRCB)IncomingControl;
    Context->Incoming.Data = IncomingDataPages;
    Context->Incoming.DataBytesInRing = IncomingDataPageCount * PAGE_SIZE;
    Context->Outgoing.Control = (PVMRCB)OutgoingControl;
    Context->Outgoing.Data = OutgoingDataPages;
    Context->Outgoing.DataBytesInRing = OutgoingDataPageCount * PAGE_SIZE;
    Context->InterruptMaskSkips = &Context->StaticInterruptMaskSkips;
    return PkpInitRingBufferControl(Context);
}

#endif

VOID
PkSetInterruptMaskSkipCount(
    __in PPACKET_LIB_CONTEXT PkLibContext,
    __in PUINT64 MaskSkips
    )
/*++

Routine Description:

    This routine sets a memory location to be updated when a signal is
    skipped due to interrupt masking.

Arguments:

    PkLibContext - Struct containing all of the lib's context.

    MaskSkips - A pointer to a value to increment.

Return Value:

    None.

--*/
{
    PkLibContext->InterruptMaskSkips = MaskSkips;
}

