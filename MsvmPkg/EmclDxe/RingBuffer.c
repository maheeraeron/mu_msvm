/*----------------------------------------------------------------------------
 $Microsoft Confidential$
 $Copyright (C) 2004 Microsoft Corporation.  All Rights Reserved.$

 $File: RingBuffer.c $


 Abstract:

     This file implements VMBus's ring buffers.

 Environment:

     Kernel mode or user mode

  9/7/2004 (jakeo) - Initial VMBus creation

----------------------------------------------------------------------------*/

#include "transportp.h"

#define MAXIMUM_EXPECTED_INTERRUPT_COUNT 64
#define UINT32_MAX 0xffffffff


VOID
PkpExpectInterrupt(
    _In_ PPACKET_LIB_CONTEXT PkLibContext,
    _In_ BOOLEAN IsIncoming
    );

static
BOOLEAN
PkpValidatePointer(
    _In_ UINT32 DataBytesInRing,
    _In_ UINT32 Pointer
    );

/// Initializes and validates the ring buffer pointer caches in the packet
/// context from the public data in the ring control structure.
///
/// \param Context Context to finish initializing.
NTSTATUS
PkpInitRingBufferControl(
    _Inout_ PPACKET_LIB_CONTEXT Context
    )
{
    UINT32 incomingIn;
    UINT32 incomingOut;
    UINT32 outgoingIn;
    UINT32 outgoingOut;

    //
    // Fetch and validate the in/out pointers.
    //

    incomingIn = ReadNoFence((LONG*)&Context->Incoming.Control->In);
    incomingOut = ReadNoFence((LONG*)&Context->Incoming.Control->Out);
    outgoingIn = ReadNoFence((LONG*)&Context->Outgoing.Control->In);
    outgoingOut = ReadNoFence((LONG*)&Context->Outgoing.Control->Out);
    if (!PkpValidatePointer(Context->Incoming.DataBytesInRing, incomingIn) ||
        !PkpValidatePointer(Context->Incoming.DataBytesInRing, incomingOut) ||
        !PkpValidatePointer(Context->Outgoing.DataBytesInRing, outgoingIn) ||
        !PkpValidatePointer(Context->Outgoing.DataBytesInRing, outgoingOut))
    {
        return STATUS_FILE_CORRUPT_ERROR;
    }

    //
    // Store the validated information.
    //

    Context->IncomingInCache = incomingIn;
    Context->IncomingOut = incomingOut;
    Context->OutgoingIn = outgoingIn;
    Context->OutgoingOutCache = outgoingOut;

    //
    // Disable ring-full interrupts and enable ring-empty interrupts.
    //

    Context->Outgoing.Control->PendingSendSize = 0;
    Context->Incoming.Control->InterruptMask = FALSE;

    //
    // Set feature bits.
    //

    Context->Outgoing.Control->FeatureBits.Value = 0;
    Context->Outgoing.Control->FeatureBits.SupportsPendingSendSize = TRUE;

    //
    // The opposite endpoint is in an unknown state and may send an
    // interrupt for each direction.
    //

    PkpExpectInterrupt(Context, TRUE);
    PkpExpectInterrupt(Context, FALSE);
    return STATUS_SUCCESS;
}

/// This function reduces a specified value using a specfied modulus. This
/// function replaces very expensive modulo division operations when it is known
/// that the number of iterations is at most one and usually zero.
///
/// \param Value Supplies the value to be reduced.
/// \param Modulus Supplies the modulus value.
///
/// \returns The reduced value is returned as the function value.
static
UINT32
PkModuloReduce(
    _In_range_(0, Modulus * 2 - 1)  UINT32 Value,
    _In_                            UINT32 Modulus
    )
{
    if (Value >= Modulus)
    {
        Value -= Modulus;
        NT_ASSERT(Value < Modulus);
    }

    return Value;
}


/// Validates a ring pointer. It must be less than the number of bytes in the
/// ring and be aligned to a 64-bit value.
///
/// \param DataBytesInRing The number of bytes in the ring.
/// \param Pointer The ring pointer to validate.
///
/// \returns TRUE if the pointer is valid.
static
BOOLEAN
PkpValidatePointer(
    _In_ UINT32 DataBytesInRing,
    _In_ UINT32 Pointer
    )
{
    return Pointer < DataBytesInRing && (Pointer % sizeof(UINT64)) == 0;
}


/// Determine the number of bytes of data available in the ring.  That is how
/// much data has been placed in the ring by the other end and is available for
/// this end to process.
///
/// We don't want to "capture" the In and Out values in this function because
/// the caller will make decisions about what to do based on the results of this
/// function.  So we need the caller to be using the same values of In and Out
/// that we're using here.  Remember that the opposite endpoint can change the
/// In and Out values at any time.  Some of those changes are benign, expected
/// and normal. Some might be malicious.
///
/// \param DataBytesInRing The total number of bytes in the ring.
/// \param PreviouslyCapturedIn Value for In.
/// \param PreviouslyCapturedOut Value for Out.
///
/// \returns Returns the number of bytes of data available.
static
UINT32
PkpDataAvailable(
    _In_                                UINT32 DataBytesInRing,
    _In_range_(0, DataBytesInRing - 1)  UINT32 PreviouslyCapturedIn,
    _In_range_(0, DataBytesInRing - 1)  UINT32 PreviouslyCapturedOut
    )
{
    UINT32 bytesAvailable;

    NT_ASSERT(PreviouslyCapturedIn < DataBytesInRing);
    NT_ASSERT(PreviouslyCapturedOut < DataBytesInRing);

    bytesAvailable = PreviouslyCapturedIn - PreviouslyCapturedOut;
    if (bytesAvailable < DataBytesInRing)
    {
        //            1         2         3
        //  0123456789012345678901234567890123456789
        // +----------------------------------------+
        // |            aaaabbbb                    |
        // +----------------------------------------+
        //  ^           ^       ^                   ^
        //  Buffer      Out     In                  RingLength
        //
        // In this example, In >= Out, (remember that == means the
        // buffer is empty) there are 8 bytes of available data
        // represented by aaaabbbb.
        //
        return bytesAvailable;
    }
    else
    {
        //            1         2         3
        //  0123456789012345678901234567890123456789
        // +----------------------------------------+
        // |ffffgggghhhh        aaaabbbbccccddddeeee|
        // +----------------------------------------+
        //  ^           ^       ^                   ^
        //  Buffer      In      Out                 RingLength
        //
        // In this example, In < Out, there are 32 bytes of available
        // data, 20 at the end of the buffer and another 12 at the
        // beginning.
        //
        return DataBytesInRing + bytesAvailable;
    }
}


/// Determine the amount of free space in the ring.  If In equals Out, then the
/// buffer is empty.  One data byte can't be used, or a full buffer looks like
/// an empty buffer.
///
/// \param DataBytesInRing The total number of bytes in the ring.
/// \param PreviouslyCapturedIn Value of In when this transaction began.
///     (Remember that reading it again would open us up to atacks where the
///     opposite end- point changed it maliciously after the first reading.)
/// \param PreviouslyCapturedOut Value of In when this transaction began.
///
/// \returns Returns the number of bytes free.
static
UINT32
PkpFreeBytes(
    _In_                                UINT32 DataBytesInRing,
    _In_range_(0, DataBytesInRing - 1)  UINT32 PreviouslyCapturedIn,
    _In_range_(0, DataBytesInRing - 1)  UINT32 PreviouslyCapturedOut
    )
{
    UINT32 bytesFree;

    NT_ASSERT(PreviouslyCapturedIn < DataBytesInRing);
    NT_ASSERT(PreviouslyCapturedOut < DataBytesInRing);

    bytesFree = PreviouslyCapturedOut - PreviouslyCapturedIn - 1;
    if (bytesFree < DataBytesInRing)
    {
        //            1         2         3
        //  0123456789012345678901234567890123456789
        // +----------------------------------------+
        // |ffffgggghhhh        aaaabbbbccccddddeeee|
        // +----------------------------------------+
        //  ^           ^       ^                   ^
        //  Buffer      In      Out                 RingLength
        //
        // In this example, In < Out, there are 32 bytes of available
        // data, 20 at the end of the buffer and another 12 at the
        // beginning.
        //

        return bytesFree;
    }
    else
    {
        //            1         2         3
        //  0123456789012345678901234567890123456789
        // +----------------------------------------+
        // |            aaaabbbb                    |
        // +----------------------------------------+
        //  ^           ^       ^                   ^
        //  Buffer      Out     In                  RingLength
        //
        // In this example, In >= Out, (remember that == means the
        // buffer is empty) there are 8 bytes of available data
        // represented by aaaabbbb.
        //

        return DataBytesInRing + bytesFree;
    }
}

/// Checks to see if the ougoing ring buffer has enough space for the outgoing
/// packet. If not, updates the PendingSendSize in the ring control.
///
/// \param PkLibContext Pointer to the packet library context structure.
/// \param TotalPacketSize Size of the packet to be be placed in the ring
///     buffer.
/// \param In Current cached value of the In pointer.
/// \param Out Current cached value of the Out pointer.
/// \param DataBytesInRing How many data bytes are in the ring buffer.
///
/// \retval STATUS_SUCCESS The ring buffer has enough space for the packet.
/// \retval STATUS_FILE_CORRUPT_ERROR The ring buffer itself has become corrupt.
///     No further processing on it is likely to succeed.
/// \retval STATUS_INVALID_PARAMETER The packet is larger than the size of the
///     ring buffer.
/// \retval STATUS_BUFFER_OVERFLOW The ring buffer is currently full.
NTSTATUS
PkpCheckSendBufferFreeBytes(
    _In_    PPACKET_LIB_CONTEXT PkLibContext,
    _In_    UINT32              TotalPacketSize,
    _In_    UINT32              In,
    _In_    UINT32              Out,
    _In_    UINT32              DataBytesInRing
    )
{
    PVMRCB control;
    NTSTATUS status = STATUS_SUCCESS;
    UINT32 pendingSendSize;

    if (PkpFreeBytes(DataBytesInRing, In, Out) < TotalPacketSize)
    {
        control = PkLibContext->Outgoing.Control;

        //
        // The cached value of the Out pointer did not yield enough space. Fetch
        // the public version and check again.
        //

        Out = ReadNoFence((LONG*)&control->Out);
        if (!PkpValidatePointer(DataBytesInRing, Out))
        {
            status = STATUS_FILE_CORRUPT_ERROR;
            goto Cleanup;
        }

        PkLibContext->OutgoingOutCache = Out;
        if (PkpFreeBytes(DataBytesInRing, In, Out) < TotalPacketSize)
        {
            //
            // There is still not enough free space to send this packet. Verify
            // that this isn't because the requested packet size is larger
            // than the ring size.
            //

            if (TotalPacketSize >= DataBytesInRing)
            {
                status = STATUS_INVALID_PARAMETER;
                goto Cleanup;
            }

            //
            // Update the pending send size in the control region and then
            // check one more time to avoid a race where enough space was freed
            // just after setting the pending size.
            //
            // Note that the currently pending commit is added to the total
            // packet size in order to avoid the race where a packet is
            // removed  by the other endpoint before the next commit occurs.
            // As a result, there may be a longer delay than absolutely
            // necessary before the signal arrives.
            //
            // FUTURE-jostarks: Consider changing callers and the contract
            // so that this is not an issue.
            //

            if (PkLibContext->PendingSendSize == 0)
            {
                PkpExpectInterrupt(PkLibContext, FALSE);
            }

            pendingSendSize = PkpDataAvailable(DataBytesInRing,
                                               In,
                                               PkLibContext->OutgoingIn) +
                              TotalPacketSize;

            if (pendingSendSize >= DataBytesInRing)
            {
                pendingSendSize = DataBytesInRing - 1;
            }

            WriteNoFence((LONG*)&control->PendingSendSize, pendingSendSize);

            //
            // Store the actual send size so that it can be retrieved by
            // users of the library.
            //

            PkLibContext->PendingSendSize = TotalPacketSize -
                sizeof(PREVIOUS_PACKET_OFFSET);

            //
            // A memory barrier is necessary to ensure that PendingSendSize
            // is set before re-reading the Out portion of the control
            // region. Otherwise a processor (or compiler) reordering could
            // occur.
            //

            MemoryBarrier();
            Out = ReadNoFence((LONG*)&control->Out);
            if (!PkpValidatePointer(DataBytesInRing, Out))
            {
                status = STATUS_FILE_CORRUPT_ERROR;
                goto Cleanup;
            }

            PkLibContext->OutgoingOutCache = Out;
            if (PkpFreeBytes(DataBytesInRing, In, Out) < TotalPacketSize)
            {
                //
                // The ring buffer is really full.
                //

                status = STATUS_BUFFER_OVERFLOW;
                goto Cleanup;
            }

            //
            // The ring buffer is no longer full. Note that now we may receive
            // an extra interrupt, but this is a small enough race that it
            // is acceptable.
            //
        }
    }

Cleanup:

    return status;
}


/// Update the outgoing ring's In pointer (offset) and checks to see if the
/// opposite endpoint must be notified. This entails checking the Out pointer
/// after the In pointer has been set; this requires a memory barrier.
///
/// \param PkLibContext Pointer to the packet library context structure.
/// \param NewIn New In value.
///
/// \retval STATUS_SUCCESS The packet was successfully inserted into the ring
///     buffer and the number of bytes that were in the ring before insertion
///     was larger than NonEmptyThreshold.
/// \retval STATUS_RING_SIGNAL_OPPOSITE_ENDPOINT The packet was successfully
///     inserted into the ring buffer and the number of bytes that were in the
///     ring buffer before insertion was less than or equal to NonEmptyThreshold
///     and the number of bytes after insertion was greater than
///     NonEmptyThreshold and the opposite endpoint's InterruptMask was clear.
/// \retval STATUS_FILE_CORRUPT_ERROR The ring buffer itself has become corrupt.
///     No further processing on it is likely to succeed.
NTSTATUS
PkCompleteInsertion(
    _In_  PPACKET_LIB_CONTEXT   PkLibContext,
    _In_  UINT32                NewIn
    )
{
    UINT32 oldIn;
    UINT32 currentOut;
    PVMRCB control;
    UINT32 dataBytesInRing;
    UINT32 interruptMask;

    control = PkLibContext->Outgoing.Control;
    dataBytesInRing = PkLibContext->Outgoing.DataBytesInRing;

    NT_ASSERT(PkpValidatePointer(dataBytesInRing, NewIn));

    //
    // Update the stored In pointer.
    //

    oldIn = PkLibContext->OutgoingIn;
    PkLibContext->OutgoingIn = NewIn;

    //
    // Update the public In pointer.
    //
    // NB: This must be a release operation (i.e. no reads or writes will be
    // reordered after this write) so that all writes to the packet are completed
    // before updating the In pointer and the other endpoint seeing the change.
    //

    WriteRelease((LONG*)&control->In, NewIn);

    //
    // Ensure that the write to the public In pointer is visible before
    // reading the Out pointer. This is necessary to avoiding missing sending
    // a notification.
    //

    MemoryBarrier();

    //
    // Read the interrupt mask bit.
    //

    interruptMask = ReadNoFence((LONG*)&control->InterruptMask);

    //
    // Read and cache the public Out pointer.
    //

    currentOut = ReadNoFence((LONG*)&control->Out);
    if (!PkpValidatePointer(dataBytesInRing, currentOut))
    {
        return STATUS_FILE_CORRUPT_ERROR;
    }

    PkLibContext->OutgoingOutCache = currentOut;

    //
    // Determine if the ring buffer may have been previously empty. If
    // the old In pointer exactly the current Out pointer, send a signal.
    // Otherwise, the opposite endpoint still has data to consume or has
    // already consumed past the insertion point; in either case, no signal
    // is necessary.
    //

    if (oldIn == currentOut)
    {
        if (interruptMask == 0)
        {
            return STATUS_RING_SIGNAL_OPPOSITE_ENDPOINT;
        }
        else
        {
            (*PkLibContext->InterruptMaskSkips) += 1;
            return STATUS_SUCCESS;
        }
    }
    else
    {
        return STATUS_SUCCESS;
    }
}


/// Update the ring's Out pointer (offset). Analogous to PkCompletionInsertion;
/// see comments there for synchronization notes.
///
/// \param PkLibContext Pointer to the packet library context structure.
/// \param NewOut New Out value.
///
/// \retval STATUS_SUCCESS The packet was successfully removed from the ring
///     buffer and the number of bytes that were in the ring before removal was
///     smaller than NonFullThreshold.
/// \retval STATUS_RING_SIGNAL_OPPOSITE_ENDPOINT The packet was successfully
///     removed from the ring buffer and the number of bytes that were in the
///     ring buffer before removal was greater than or equal to NonFullThreshold
///     and the number of bytes after insertion was less than NonFullThreshold
///     and the opposite endpoint's InterruptMask was clear. This code is not an
///     error, i.e. NT_SUCCESS(STATUS_RING_SIGNAL_OPPOSITE_ENDPOINT) is true.
/// \retval STATUS_RING_NEWLY_EMPTY The incoming ring is now empty.  This can be
///     used as a hint by the client to avoid immediately trying to receive
///     another packet, thus avoiding a pointless lock acquisition.    This code
///     is not an error, i.e. NT_SUCCESS(STATUS_RING_NEWLY_EMPTY) is true.
NTSTATUS
PkCompleteRemoval(
    _In_  PPACKET_LIB_CONTEXT   PkLibContext,
    _In_  UINT32                NewOut
    )
{
    UINT32 oldFreeBytes;
    UINT32 oldOut;
    UINT32 newFreeBytes;
    UINT32 currentIn;
    UINT32 dataBytesInRing;
    PVMRCB control;
    UINT32 pendingSendSize;

    control = PkLibContext->Incoming.Control;
    dataBytesInRing = PkLibContext->Incoming.DataBytesInRing;

    NT_ASSERT(PkpValidatePointer(dataBytesInRing, NewOut));

    //
    // Mark that an interrupt is expected if the ring is now empty.
    //

    if ((UINT64)ReadNoFence((LONG*)&control->In) == NewOut)
    {
        PkpExpectInterrupt(PkLibContext, TRUE);
    }

    //
    // Update the stored and public Out pointer.
    //

    oldOut = PkLibContext->IncomingOut;
    PkLibContext->IncomingOut = NewOut;
    WriteNoFence((LONG*)&control->Out, NewOut);

    //
    // Flush the write to the public Out pointer to ensure that the subsequent
    // read of In will be up-to-date. This is necessary to avoid missing
    // notifications.
    //

    MemoryBarrier();

    //
    // Determine whether an interrupt may be necessary.
    //

    pendingSendSize = ReadNoFence((LONG*)&control->PendingSendSize);

    //
    // Read and cache the public In pointer.
    //

    currentIn = ReadNoFence((LONG*)&control->In);
    if (!PkpValidatePointer(dataBytesInRing, currentIn))
    {
        return STATUS_FILE_CORRUPT_ERROR;
    }

    PkLibContext->IncomingInCache = currentIn;

    //
    // Check to see if this removal frees up enough space for the opposite
    // endpoint to write into the ring.
    //

    if (pendingSendSize != 0)
    {
        //
        // N.B. If the opposite endpoint has produced past the insertion
        // point, the number of free bytes before the send will appear larger
        // than the number of free bytes before the send. In this case, no
        // signal is necessary since the opposite endpoint has already
        // "noticed" the extra free space.
        //

        oldFreeBytes = PkpFreeBytes(dataBytesInRing, currentIn, oldOut);
        newFreeBytes = PkpFreeBytes(dataBytesInRing, currentIn, NewOut);
        if (newFreeBytes >= pendingSendSize && oldFreeBytes < pendingSendSize)
        {
            return STATUS_RING_SIGNAL_OPPOSITE_ENDPOINT;
        }
    }

    if (NewOut == currentIn)
    {
        return STATUS_RING_NEWLY_EMPTY;
    }
    else
    {
        return STATUS_SUCCESS;
    }
}


/// Retrieves the outgoing ring's current offset, appropriate for passing to
/// PkGetSendBuffer.
///
/// \param PkLibContext Packet library context.
///
/// \returns A 32-bit offset in bytes from where the next packet will be read.
UINT32
PkGetOutgoingRingOffset(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    )
{
    return PkLibContext->OutgoingIn;
}


/// Retrieves the incoming ring's current offset, appropriate for passing to
/// PkGetReceiveBuffer.
///
/// \param PkLibContext Packet library context.
///
/// \returns A 32-bit offset in bytes from where the next packet will be read.
UINT32
PkGetIncomingRingOffset(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    )
{
    return PkLibContext->IncomingOut;
}


/// Determines whether the incoming ring is empty.
///
/// \param PkLibContext Packet library context.
///
/// \returns TRUE if the ring is empty, FALSE otherwise.
BOOLEAN
PkIsIncomingRingEmpty(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    )
{
    return (PkLibContext->IncomingOut == PkLibContext->Incoming.Control->In);
}


/// Determines whether there is enough space in the outgoing ring to send a
/// packet of a given size.
///
/// \param PkLibContext Packet library context.
///
/// \returns TRUE if the ring buffer cannot fit the last queued packet, FALSE
///     otherwise.
BOOLEAN
PkIsOutgoingRingFull(
    _In_ PPACKET_LIB_CONTEXT PkLibContext,
    _In_ UINT32 SendSize
    )
{
    UINT32 totalSendSize;

    totalSendSize = ALIGN_UP(SendSize, UINT64) + sizeof(PREVIOUS_PACKET_OFFSET);
    if (totalSendSize > PkGetOutgoingRingFreeBytes(PkLibContext))
    {
        return TRUE;
    }

    return FALSE;
}


/// Retrieves the size of the previous send that failed because the ring was too
/// full, or 0 if the most recent send was successful.
///
/// \param PkLibContext Packet library context.
///
/// \returns The send size as an unsigned integer.
UINT32
PkGetLastFailedSendSize(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    )
{
    return PkLibContext->PendingSendSize;
}


/// Gets a pointer to a buffer in the outgoing ring to store a packet. Checks to
/// ensure enough space is available and prepares some control data.
///
/// \param PkLibContext Pointer to the packet library context structure.
/// \param Offset A pointer to the ring offset to write to. Returns the offset
///     of the next free location.
/// \param PacketSize Total number of bytes that will be consumed by the packet
///     that we want to insert into the ring buffer.
/// \param Buffer Returns a pointer to the buffer.
///
/// \returns NTSTATUS value
NTSTATUS
PkGetSendBuffer(
    _In_ PPACKET_LIB_CONTEXT PkLibContext,
    _Inout_ PUINT32 Offset,
    _In_range_(>, 0) UINT32 PacketSize,
    _Out_ _Outptr_result_bytebuffer_(PacketSize) PVOID *Buffer
    )
{
    NTSTATUS status;
    PVMRCB control;
    UINT32 totalPacketSize;
    UINT32 in;
    UINT32 out;
    volatile UINT64 *buffer;
    PREVIOUS_PACKET_OFFSET packetOffset;
    UINT32 dataBytesInRing;
#ifndef VMBUS_RING_BUFFER_SINGLE_MAPPED
    UINT32 PacketOneBeforeEndOffset;
    UINT32 PacketEndOffset;
#else
    INT64 PacketOneBeforeEndOffset;
    INT64 PacketEndOffset;
#endif

    PacketSize = ALIGN_UP(PacketSize, UINT64);
    totalPacketSize = PacketSize + sizeof(PREVIOUS_PACKET_OFFSET);

    //
    // Grab the In/Out pointers from the cache.
    //

    dataBytesInRing = PkLibContext->Outgoing.DataBytesInRing;
    in = *Offset;
    out = PkLibContext->OutgoingOutCache;

    //
    // Get the correct buffer offset we will write into.
    //

#ifndef VMBUS_RING_BUFFER_SINGLE_MAPPED
        PacketOneBeforeEndOffset = PacketSize / sizeof(UINT64) - 1;
        PacketEndOffset = PacketSize / sizeof(UINT64);
#else
        PacketOneBeforeEndOffset =
            ((INT64)(PkModuloReduce(in + PacketSize - sizeof(UINT64), dataBytesInRing)) - (INT64)in) / (INT64)sizeof(UINT64);
        PacketEndOffset =
            ((INT64)(PkModuloReduce(in + PacketSize, dataBytesInRing)) - (INT64)in) / (INT64)sizeof(UINT64);

        NT_ASSERT(PacketOneBeforeEndOffset <= UINT32_MAX);
        NT_ASSERT(PacketEndOffset <= UINT32_MAX);
#endif

    //
    // Pull the buffer into the processor cache.
    //

    buffer = (PUINT64)&PkLibContext->Outgoing.Data[in];
    PrefetchForWrite(&buffer[PacketOneBeforeEndOffset]);
    PrefetchForWrite(buffer);

    //
    // Check if there is enough space in the send buffer for this packet.
    //

    status = PkpCheckSendBufferFreeBytes(PkLibContext,
                                         totalPacketSize,
                                         in,
                                         out,
                                         dataBytesInRing);

    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

    //
    // Zero out the tail parts of the buffer in case of a non-multiple of 8
    // packet size.
    //

    buffer[PacketOneBeforeEndOffset] = 0;

    //
    // Record the original IN pointer for debugging.
    // N.B. Although this is only used in debugging, it must be there to
    // work on old chk-built VSPs and VSCs, which assert on it.
    //

    packetOffset.Reserved = 0;
    packetOffset.Offset = in;
    buffer[PacketEndOffset] = packetOffset.AsUINT64;

    //
    // No send is pending anymore.
    //

    if (PkLibContext->PendingSendSize != 0)
    {
        PkLibContext->PendingSendSize = 0;
        control = PkLibContext->Outgoing.Control;
        WriteNoFence((LONG*)&control->PendingSendSize, 0);
    }

    //
    // Return the new offset and the buffer.
    //

    *Offset = PkModuloReduce(in + totalPacketSize, dataBytesInRing);
    *Buffer = (PVOID)buffer;
    status = STATUS_SUCCESS;

Cleanup:
    return status;
}


/// Retrieves a pointer to a buffer where the next packet can be read from. The
/// caller must take care to avoid security holes due to double-fetching values
/// from this buffer; a malicious remote endpoint can change the data at any
/// time.
///
/// \param PkLibContext Packet library context.
/// \param Offset A pointer to the offset to read from, which can be retrieved
///     from PkGetIncomingRingOffset. Is updated on success with the next
///     packet's offset. Should be passed to PkGetReceiveBuffer to read more
///     packets or PkCompleteRemoval to finish reading.
/// \param Buffer Buffer pointing to ring data to receive.
/// \param Length Returns the length of the buffer.
NTSTATUS
PkGetReceiveBuffer(
    _In_ PPACKET_LIB_CONTEXT PkLibContext,
    _Inout_ PUINT32 Offset,
    _Out_ __deref_bcount(*Length) PVOID *Buffer,
    _Out_ _Deref_out_range_(>=, sizeof(VMPACKET_DESCRIPTOR)) PUINT32 Length
    )
{
    UINT32 in;
    UINT32 out;
    UINT32 bytesInRing;
    NTSTATUS status;
    PVMPACKET_DESCRIPTOR header;
    UINT32 packetLength;
    UINT32 totalPacketSize;
    UINT32 dataBytesInRing;

    //
    // Grab the In/NewOut pointers from the cache.
    //

    dataBytesInRing = PkLibContext->Incoming.DataBytesInRing;
    in = PkLibContext->IncomingInCache;
    out = *Offset;

    NT_ASSERT(out < dataBytesInRing);

    if (in == out)
    {
        //
        // The cached version of In did not yield enough space. Try again with
        // the public version of In.
        //
        // NB: This must be an acquire operation (i.e. no following reads or
        // writes will be reordered before this read) so that the header
        // contents below do not get prefetched with stale data.
        //

        in = ReadAcquire((LONG*)&PkLibContext->Incoming.Control->In);
        if (!PkpValidatePointer(dataBytesInRing, in))
        {
            status = STATUS_FILE_CORRUPT_ERROR;
            goto Cleanup;
        }

        PkLibContext->IncomingInCache = in;
        if (in == out)
        {
            status = STATUS_END_OF_FILE;
            goto Cleanup;
        }
    }

    bytesInRing = PkpDataAvailable(dataBytesInRing, in, out);
    header = (PVMPACKET_DESCRIPTOR)&PkLibContext->Incoming.Data[out];

#ifdef VMBUS_RING_BUFFER_SINGLE_MAPPED
    //
    // Since packets are aligned to the size of UINT64, as long as the packet length field
    // offset is less than sizeof(UINT64), we don't have to worry about wrapping around the
    // end of the ring buffer. We assert here to keep the assertion with the relevant code.
    //
    static_assert(FIELD_OFFSET(VMPACKET_DESCRIPTOR, Length8) < sizeof(UINT64), "");
#endif

    //
    // Capture the length field and shift it to a byte count.
    //
    // N.B. at this point it's not guaranteed that bytesInRing is bigger than
    // sizeof(VMPACKET_DESCRIPTOR), but the buffer is safe to read in any
    // case.
    //

    packetLength = header->Length8 * 8;

    //
    // Prevent double fetches of the packet length.
    //

    _ReadWriteBarrier();
    totalPacketSize = packetLength + sizeof(PREVIOUS_PACKET_OFFSET);

    //
    // Capture corruptions: length must cover at least the size of the
    // packet descriptor and should not be more than the buffer size.
    //

    if (packetLength < sizeof(VMPACKET_DESCRIPTOR) ||
        totalPacketSize > bytesInRing)
    {
        status = STATUS_FILE_CORRUPT_ERROR;
        goto Cleanup;
    }

    //
    // Verify the previous packet offset. Only perform this check in checked
    // builds, since the result does not affect anything (it's just a debugging
    // mechanism).
    //

#if DBG
    if (((PPREVIOUS_PACKET_OFFSET)((PUINT8)
#ifndef VMBUS_RING_BUFFER_SINGLE_MAPPED
        header + packetLength
#else
        PkLibContext->Incoming.Data + PkModuloReduce((UINT32)((PUCHAR)header - PkLibContext->Incoming.Data + packetLength), dataBytesInRing)
#endif
        ))->Offset != out)
    {
        status = STATUS_FILE_CORRUPT_ERROR;
        goto Cleanup;
    }
#endif

    *Buffer = header;
    *Length = packetLength;
    *Offset = PkModuloReduce(out + totalPacketSize, dataBytesInRing);
    status = STATUS_SUCCESS;

Cleanup:
    return status;
}


#if defined(VMBUS_RING_BUFFER_SINGLE_MAPPED)

/// This function writes to a single-mapped ring buffer, folding around if
/// necessary.
///
/// Assumes that the caller will synchronize access to the outgoing ring.
///
/// \param PkLibContext Pointer to the packet library context structure.
/// \param PacketBuf Pointer to a buffer containing the packet to be sent.
/// \param PacketBufSize Size of the buffer in bytes.
/// \param Offset The offset in the ring buffer to write. This offset should be
///     (the offset obtained by PkGetOutgoingRingOffset) + an offset into the
///     packet to write.
VOID
PkWritePacketSingleMapped(
    _In_                        PPACKET_LIB_CONTEXT PkLibContext,
    _In_reads_bytes_(PacketBufSize)  PVOID               PacketBuf,
    _In_range_(>, 0)            UINT32              PacketBufSize,
    _In_                        UINT32              Offset
    )
{
    UINT32 in;
    UINT32 dataBytesInRing;
    UINT32 ringBufferEndOffset;
    volatile UCHAR *buffer;

    buffer = PkLibContext->Outgoing.Data;

    dataBytesInRing = PkLibContext->Outgoing.DataBytesInRing;
    in = PkModuloReduce(Offset, dataBytesInRing);

    ringBufferEndOffset = dataBytesInRing - in;
    if (ALIGN_UP(PacketBufSize, UINT64) <= ringBufferEndOffset)
    {
        RtlCopyMemory((PVOID)(buffer + in),
                      PacketBuf,
                      PacketBufSize);
    }
    else
    {
        //
        // Handle the case where we must copy a packet around the end of the ring buffer.
        //
        RtlCopyMemory((PVOID)(buffer + in),
                      PacketBuf,
                      ringBufferEndOffset);

        RtlCopyMemory((PVOID)buffer,
                      (PUCHAR)PacketBuf + ringBufferEndOffset,
                      PacketBufSize - ringBufferEndOffset);
    }
}


/// This function peeks a packet from the endpoint's incoming packet ring.
/// PkGetIncomingPacketSize must be called before this is called to get the
/// values for Out and PacketBufSize.
///
/// Assumes that the caller will synchronize access to the incoming ring.
///
/// \param PkLibContext Pointer to the packet library context structure.
/// \param PacketBuf Pointer to a buffer into which part of the incoming packet
///     can be copied.
/// \param PacketBufSize Size of the buffer in bytes.
/// \param Out Ring buffer pointer indicating where the next packet is located,
///     as retrieved from PkGetIncomingPacketSize.
VOID
PkReadPacketSingleMapped(
    _In_                        PPACKET_LIB_CONTEXT     PkLibContext,
    _Out_writes_bytes_(PacketBufSize) PVOID                   PacketBuf,
    _In_                        UINT32                  PacketBufSize,
    _In_                        UINT32                  Out
    )
{
    volatile UCHAR *buffer;
    UINT32 dataBytesInRing;
    UINT32 ringBufferEndOffset;

    dataBytesInRing = PkLibContext->Incoming.DataBytesInRing;
    buffer = PkLibContext->Incoming.Data;

    Out = PkModuloReduce(Out, dataBytesInRing);

    //
    // See how much space is available up to the end of the ring buffer.
    //
    ringBufferEndOffset = dataBytesInRing - Out;

    //
    // Now grab the packet.
    //

    if (PacketBufSize <= ringBufferEndOffset)
    {
        RtlCopyMemory(PacketBuf, (PVOID)(buffer + Out), PacketBufSize);
    }
    else
    {
        //
        // Handle the case where we must copy a packet from around
        // the end of the ring buffer.
        //

        RtlCopyMemory(PacketBuf,
                      (PVOID)(buffer + Out),
                      ringBufferEndOffset);

        RtlCopyMemory((PUCHAR)PacketBuf + ringBufferEndOffset,
                      (PVOID)buffer,
                      PacketBufSize - ringBufferEndOffset);

        NT_ASSERT(PacketBufSize - ringBufferEndOffset + sizeof(PREVIOUS_PACKET_OFFSET) ==
            PkModuloReduce(Out + PacketBufSize + sizeof(PREVIOUS_PACKET_OFFSET), dataBytesInRing));
    }
}


#endif


/// This function inserts a packet into an endpoint's outgoing ring buffer.  It
/// does not use any side-band buffer management.
///
/// Assumes that the caller will synchronize access to the outgoing ring.
///
/// \param PkLibContext Pointer to the packet library context structure.
/// \param PacketBuf Pointer to a buffer containing the packet to be sent.
/// \param PacketBufSize Size of the buffer in bytes.
/// \param TransactionId ID for this packet assigned by the next layer up in the
///     stack.  It will be used for completion, cancelation and synchronization.
/// \param RequestCompletion Indicates whether a completion packet should be
///     required.  A simple packet may or may not need one, as it doesn't refer
///     to external memory mappings with lifetimes separate from the packet
///     itself.
///
/// \retval STATUS_SUCCESS The packet was successfully inserted into the ring
///     buffer and the number of bytes that were in the ring before insertion
///     was larger than NonEmptyThreshold.
/// \retval STATUS_RING_SIGNAL_OPPOSITE_ENDPOINT The packet was successfully
///     inserted into the ring buffer and the number of bytes that were in the
///     ring buffer before insertion was smaller than NonEmptyThreshold.
/// \retval STATUS_INVALID_PARAMETER The packet is larger than the total size of
///     the ring.  This packet can never be sent through this ring.
/// \retval STATUS_BUFFER_OVERFLOW The packet is larger than the current free
///     space in the ring.  The packet could be sent through this ring later,
///     when the opposite endpoint consumes some or all of the currently waiting
///     packets.
/// \retval STATUS_FILE_CORRUPT_ERROR The ring buffer itself has become corrupt.
///     No further processing on it is likely to succeed.
_Must_inspect_result_
NTSTATUS
PkSendPacketSimple(
    _In_                        PPACKET_LIB_CONTEXT PkLibContext,
    _In_reads_bytes_(PacketBufSize)  PVOID               PacketBuf,
    _In_                        UINT32              PacketBufSize,
    _In_                        UINT64              TransactionId,
    _In_                        BOOLEAN             RequestCompletion
    )
{
    PVMPACKET_DESCRIPTOR header;
    NTSTATUS             status;
    UINT32               messageLength;
    UINT32               newIn;

    messageLength = sizeof(*header) + PacketBufSize;
    newIn = PkLibContext->OutgoingIn;
    status = PkGetSendBuffer(PkLibContext,
                             &newIn,
                             messageLength,
                             &header);

    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

    //
    // Fill in the header.
    //

    header->Type = VmbusPacketTypeDataInBand;
    header->DataOffset8 = sizeof(*header) / 8;
    header->Length8 = (UINT16)(ALIGN_UP(messageLength, UINT64) / 8);
    header->Flags = RequestCompletion ? VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED : 0;
    PkWriteRingBufferField(header->TransactionId, TransactionId);

    //
    // Ensure the compiler does not touch the packet data again for either read
    // or write.
    //

    _ReadWriteBarrier();

    //
    // Copy the caller supplied data to the ring.
    //

    PkWriteRingBuffer(PkLibContext, header + 1, PacketBuf, PacketBufSize);

    //
    // Finally, update the control structure so the data is visible to the
    // other end of the pipe.
    //

    status = PkCompleteInsertion(PkLibContext, newIn);

Cleanup:
    return status;
}


/// This function inserts a packet into an endpoint's outgoing ring buffer.  It
/// uses transfer pages for containing actual data and the packet for metadata
/// (probably commands.)
///
/// Assumes that the caller will synchronize access to the outgoing ring.
///
/// \param PkLibContext Pointer to the packet library context structure.
/// \param PacketBuf Pointer to a buffer containing the packet to be sent.
/// \param PacketBufSize Size of the buffer in bytes.
/// \param TransactionId ID for this packet assigned by the next layer up in the
///     stack.  It will be used for completion, cancelation and synchronization.
/// \param TransferPageSetId The transfer page set ID, to be interpretted by the
///     opposite endpoint.
/// \param Ranges A set of ordered offsets and lengths within the the transfer
///     page set that contain data associated with this transaction.
/// \param SenderOwnsSet If true, this indicates that the TransferPageSetId is
///     one generated by the sending endpoint.  If false, the ID referred to is
///     one generated by the receiving endpoint.
///
/// \retval STATUS_SUCCESS The packet was successfully inserted into the ring
///     buffer and the number of bytes that were in the ring before insertion
///     was larger than NonEmptyThreshold.
/// \retval STATUS_RING_SIGNAL_OPPOSITE_ENDPOINT The packet was successfully
///     inserted into the ring buffer and the number of bytes that were in the
///     ring buffer before insertion was smaller than NonEmptyThreshold.
/// \retval STATUS_INVALID_PARAMETER The packet is larger than the total size of
///     the ring.  This packet can never be sent through this ring.
/// \retval STATUS_BUFFER_OVERFLOW The packet is larger than the current free
///     space in the ring.  The packet could be sent through this ring later,
///     when the opposite endpoint consumes some or all of the currently waiting
///     packets.
_Must_inspect_result_
NTSTATUS
PkSendPacketTransferPage(
    _In_                        PPACKET_LIB_CONTEXT     PkLibContext,
    _In_reads_bytes_(PacketBufSize)  PVOID                   PacketBuf,
    _In_                        UINT32                  PacketBufSize,
    _In_                        UINT64                  TransactionId,
    _In_                        UINT16                  TransferPageSetId,
    _In_                        PVMTRANSFER_PAGE_RANGES Ranges,
    _In_                        BOOLEAN                 SenderOwnsSet
    )
{
    PVMTRANSFER_PAGE_RANGES       subRanges;
    NTSTATUS                      status;
    UINT32                        newIn;
    ULONG                         rangeCount;
    UINT32                        headerLength;
    UINT32                        messageLength;
    PVMTRANSFER_PAGE_PACKET_HEADER header;
    UINT32                        rangesWritten;

    //
    // Count up all the offset/length tuples that will be part of this packet.
    //

    rangeCount = 0;
    for (subRanges = Ranges; subRanges != NULL; subRanges = subRanges->Next)
    {
        rangeCount += subRanges->RangeCount;
    }

    headerLength = FIELD_OFFSET(VMTRANSFER_PAGE_PACKET_HEADER, Ranges) +
        sizeof(VMTRANSFER_PAGE_RANGE) * rangeCount;

    messageLength = headerLength + PacketBufSize;
    newIn = PkLibContext->OutgoingIn;
    status = PkGetSendBuffer(PkLibContext,
                             &newIn,
                             messageLength,
                             &header);

    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

    //
    // Build the packet header.
    //

    header->Descriptor.Type         = VmbusPacketTypeDataUsingTransferPages;
    header->Descriptor.DataOffset8  = (UINT16)(headerLength / 8);
    header->Descriptor.Length8      = (UINT16)(ALIGN_UP(messageLength, UINT64) / 8);
    header->Descriptor.Flags        = VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED;
    PkWriteRingBufferField(header->Descriptor.TransactionId , TransactionId);
    PkWriteRingBufferField(header->TransferPageSetId        , TransferPageSetId);
    PkWriteRingBufferField(header->SenderOwnsSet            , SenderOwnsSet);
    PkWriteRingBufferField(header->Reserved                 , 0);
    PkWriteRingBufferField(header->RangeCount               , rangeCount);

    //
    // Ensure the compiler does not touch the packet data again for either read
    // or write.
    //

    _ReadWriteBarrier();

    //
    // Copy the transfer page ranges onto the ring.
    //

    rangesWritten = 0;
    for (subRanges = Ranges; subRanges != NULL; subRanges = subRanges->Next)
    {
        __assume(rangesWritten + subRanges->RangeCount <= rangeCount);

        PkWriteRingBuffer(
            PkLibContext,
            &header->Ranges[rangesWritten],
            subRanges->Range,
            subRanges->RangeCount * sizeof(VMTRANSFER_PAGE_RANGE));
        rangesWritten += subRanges->RangeCount;
    }

    NT_ASSERT(rangesWritten == rangeCount);

    //
    // Copy the caller supplied data to the ring.
    //

    PkWriteRingBuffer(PkLibContext, (PUINT8)header + headerLength, PacketBuf, PacketBufSize);

    //
    // Finally, update the control structure so the data is visible to the
    // other end of the pipe.
    //

    status = PkCompleteInsertion(PkLibContext, newIn);

Cleanup:

    return status;
}


/// This function inserts a cancel packet into an endpoint's outgoing ring
/// buffer.
///
/// Assumes that the caller will synchronize access to the outgoing ring.
///
/// \param PkLibContext Pointer to the packet library context structure.
/// \param TransactionId Transaction ID of the previous packet which is now
///     being cancelled.
///
/// \retval STATUS_SUCCESS The packet was successfully inserted into the ring
///     buffer and the number of bytes that were in the ring before insertion
///     was larger than NonEmptyThreshold.
/// \retval STATUS_RING_SIGNAL_OPPOSITE_ENDPOINT The packet was successfully
///     inserted into the ring buffer and the caller should signal the opposite
///     endpoint.
/// \retval STATUS_INVALID_PARAMETER The packet is larger than the total size of
///     the ring.  This packet can never be sent through this ring.
/// \retval STATUS_BUFFER_OVERFLOW The packet is larger than the current free
///     space in the ring.  The packet could be sent through this ring later,
///     when the opposite endpoint consumes some or all of the currently waiting
///     packets.
/// \retval STATUS_FILE_CORRUPT_ERROR The ring buffer itself has become corrupt.
///     No further processing on it is likely to succeed.
_Must_inspect_result_
NTSTATUS
PkSendCancel(
    _In_  PPACKET_LIB_CONTEXT   PkLibContext,
    _In_  UINT64                TransactionId
    )
{
    PVMPACKET_DESCRIPTOR    header;
    NTSTATUS                status;
    UINT32                  newIn;

    newIn = PkLibContext->OutgoingIn;
    status = PkGetSendBuffer(PkLibContext,
                             &newIn,
                             sizeof(*header),
                             &header);

    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

    header->Type = VmbusPacketTypeCancelRequest;
    header->Flags = 0;
    header->DataOffset8 = sizeof(VMPACKET_DESCRIPTOR) / 8;
    header->Length8 = sizeof(VMPACKET_DESCRIPTOR) / 8;
    PkWriteRingBufferField(header->TransactionId, TransactionId);

    //
    // Ensure the compiler does not touch the packet data again for either read
    // or write.
    //

    _ReadWriteBarrier();

    //
    // Finally, update the control structure so the data is visible to the
    // other end of the pipe.
    //

    status = PkCompleteInsertion(PkLibContext, newIn);

Cleanup:

    return status;
}


/// This function inserts a completion packet into an endpoint's outgoing ring
/// buffer.
///
/// Assumes that the caller will synchronize access to the outgoing ring.
///
/// \param PkLibContext Pointer to the packet library context structure.
/// \param TransactionId Transaction ID of the previous packet which is now
///     being completed.
/// \param PacketBuf Pointer to a buffer containing the packet to be sent.
/// \param PacketBufSize Size of the buffer in bytes.
/// \param IsLongTransaction The caller has already called PkDropCompletionCount
///     for this transaction.
///
/// \retval STATUS_SUCCESS The packet was successfully inserted into the ring
///     buffer and the number of bytes that were in the ring before insertion
///     was larger than NonEmptyThreshold.
/// \retval STATUS_RING_SIGNAL_OPPOSITE_ENDPOINT The packet was successfully
///     inserted into the ring buffer and the number of bytes that were in the
///     ring buffer before insertion was smaller than NonEmptyThreshold.
/// \retval STATUS_INVALID_PARAMETER The packet is larger than the total size of
///     the ring.  This packet can never be sent through this ring.
/// \retval STATUS_BUFFER_OVERFLOW The packet is larger than the current free
///     space in the ring.  The packet could be sent through this ring later,
///     when the opposite endpoint consumes some or all of the currently waiting
///     packets.
/// \retval STATUS_FILE_CORRUPT_ERROR The ring buffer itself has become corrupt.
///     No further processing on it is likely to succeed.
_Must_inspect_result_
NTSTATUS
PkSendCompletion(
    _In_                            PPACKET_LIB_CONTEXT PkLibContext,
    _In_                            UINT64              TransactionId,
    _In_reads_bytes_opt_(PacketBufSize)  PVOID               PacketBuf,
    _In_                            UINT32              PacketBufSize,
    _In_                            BOOLEAN             IsLongTransaction
    )
{
    PVMPACKET_DESCRIPTOR header;
    NTSTATUS status;
    UINT32 messageLength;
    UINT32 newIn;

    UNREFERENCED_PARAMETER(IsLongTransaction);

    messageLength = sizeof(*header) + PacketBufSize;
    if (!ARGUMENT_PRESENT(PacketBuf))
    {
        NT_ASSERT(PacketBufSize == 0);
    }

    newIn = PkLibContext->OutgoingIn;
    status = PkGetSendBuffer(PkLibContext, &newIn, messageLength, &header);
    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

    //
    // Push the header onto the ring.
    //

    header->Type = VmbusPacketTypeCompletion;
    header->Flags = 0;
    header->DataOffset8 = sizeof(*header) / 8;
    header->Length8 = (UINT16)(ALIGN_UP(messageLength, UINT64) / 8);
    PkWriteRingBufferField(header->TransactionId, TransactionId);

    //
    // Ensure the compiler does not touch the packet data again for either read
    // or write.
    //

    _ReadWriteBarrier();

    //
    // Copy the caller supplied data to the ring.
    //

    PkWriteRingBuffer(PkLibContext, header + 1, PacketBuf, PacketBufSize);

    //
    // Finally, update the control structure so the data is visible to the
    // other end of the pipe.
    //

    status = PkCompleteInsertion(PkLibContext, newIn);

Cleanup:

    return status;
}


/// This function inserts an entire raw packet into an endpoint's outgoing ring
/// buffer.  It does not use any side-band buffer management.  It does not
/// attempt to manage the packet's header.
///
/// Assumes that the caller will synchronize access to the outgoing ring.
///
/// \param PkLibContext Pointer to the packet library context structure.
/// \param PacketBuf Pointer to a buffer containing the packet to be sent.
/// \param PacketBufSize Size of the buffer in bytes.
///
/// \retval STATUS_SUCCESS The packet was successfully inserted into the ring
///     buffer and the number of bytes that were in the ring before insertion
///     was larger than NonEmptyThreshold.
/// \retval STATUS_RING_SIGNAL_OPPOSITE_ENDPOINT The packet was successfully
///     inserted into the ring buffer and the number of bytes that were in the
///     ring buffer before insertion was smaller than NonEmptyThreshold.
/// \retval STATUS_INVALID_PARAMETER The packet is larger than the total size of
///     the ring.  This packet can never be sent through this ring.
/// \retval STATUS_BUFFER_OVERFLOW The packet is larger than the current free
///     space in the ring.  The packet could be sent through this ring later,
///     when the opposite endpoint consumes some or all of the currently waiting
///     packets.
/// \retval STATUS_FILE_CORRUPT_ERROR The ring buffer itself has become corrupt.
///     No further processing on it is likely to succeed.
_Must_inspect_result_
NTSTATUS
PkSendPacketRaw(
    _In_                        PPACKET_LIB_CONTEXT PkLibContext,
    _In_reads_bytes_(PacketBufSize)  PVOID               PacketBuf,
    _In_range_(>, 0)            UINT32              PacketBufSize
    )
{
    NTSTATUS        status;
    UINT32          newIn;
    PUINT8          buffer;

    NT_ASSERT(PacketBufSize > 0);

    newIn = PkLibContext->OutgoingIn;
    status = PkGetSendBuffer(PkLibContext, &newIn, PacketBufSize, &buffer);
    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

    //
    // Copy the caller supplied data to the ring.
    //

    PkWriteRingBuffer(PkLibContext, buffer, PacketBuf, PacketBufSize);

    //
    // Finally, update the control structure so the data is visible to the
    // other end of the pipe.
    //

    status = PkCompleteInsertion(PkLibContext, newIn);

Cleanup:

    return status;
}


/// This function removes a packet from the endpoint's incoming packet ring.
///
/// Assumes that the caller will synchronize access to the incoming ring.
///
/// \param PkLibContext Pointer to the packet library context structure.
/// \param PacketBuf Pointer to a buffer into which the incoming packet can be
///     copied.
/// \param PacketBufSize Size of the buffer in bytes.
/// \param PacketBufSizeNeeded Out parameter allowing the caller to know how big
///     the packet is.
///
/// \retval STATUS_SUCCESS The packet was successfully removed from the ring
///     buffer and the number of bytes that were in the ring before removal was
///     smaller than NonFullThreshold.
/// \retval STATUS_RING_SIGNAL_OPPOSITE_ENDPOINT The packet was successfully
///     removed from the ring buffer and the number of bytes that were in the
///     ring buffer before removal was greater than or equal to NonFullThreshold
///     and the number of bytes after insertion was less than NonFullThreshold.
///     This code is not an error, i.e.
///     NT_SUCCESS(STATUS_RING_SIGNAL_OPPOSITE_ENDPOINT) is true.
/// \retval STATUS_RING_NEWLY_EMPTY The incoming ring is now empty.  This can be
///     used as a hint by the client to avoid immediately trying to receive
///     another packet, thus avoiding a pointless lock acquisition.  This code
///     is not an error, i.e. NT_SUCCESS(STATUS_RING_NEWLY_EMPTY) is true.
/// \retval STATUS_BUFFER_TOO_SMALL PacketBuf was too small to contain the next
///     packet in the ring buffer.
/// \retval STATUS_END_OF_FILE The ring was empty and there are no packets to
///     receive.
/// \retval STATUS_FILE_CORRUPT_ERROR The ring buffer itself has become corrupt.
///     No further processing on it is likely to succeed.
_Must_inspect_result_
NTSTATUS
PkReceivePacket(
    _In_                        PPACKET_LIB_CONTEXT     PkLibContext,
    _Out_writes_bytes_(PacketBufSize) PVMPACKET_DESCRIPTOR    PacketBuf,
    _In_range_(>=, sizeof(VMPACKET_DESCRIPTOR))
                                UINT32                  PacketBufSize,
    _Out_opt_                   PUINT32                 PacketBufSizeNeeded
    )
{
    NTSTATUS status;
    UINT32 newOut;
    PVOID buffer;
    UINT32 incomingPacketSize;

    newOut = PkLibContext->IncomingOut;
    status = PkGetReceiveBuffer(PkLibContext, &newOut, &buffer, &incomingPacketSize);
    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

    if (ARGUMENT_PRESENT(PacketBufSizeNeeded))
    {
        *PacketBufSizeNeeded = incomingPacketSize;
    }

    if (incomingPacketSize > PacketBufSize)
    {
        status = STATUS_BUFFER_TOO_SMALL;
        goto Cleanup;
    }

    //
    // Now grab the packet.
    //

    PkReadRingBuffer(PkLibContext, PacketBuf, buffer, incomingPacketSize);

    //
    // To prevent the opposite endpoint from writing a different packet size
    // into the header after we've read the header length but before we've
    // removed the packet from the ring buffer, we'll just write over the
    // length field with the (already validated) value we read initially.
    //

    PacketBuf->Length8 = (UINT16)(incomingPacketSize / 8);
    status = PkCompleteRemoval(PkLibContext, newOut);

Cleanup:

    return status;
}


/// This function returns total number of bytes in the ring, regardless of
/// whether they are currently in use.
///
/// \param PkLibContext Pointer to the packet library context structure.
///
/// \returns The number of bytes in the ring.
UINT32
PkGetIncomingRingSize(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    )
{
    return PkLibContext->Incoming.DataBytesInRing;
}


/// This function returns total number of bytes in the ring, regardless of
/// whether they are currently in use.
///
/// \param PkLibContext Pointer to the packet library context structure.
///
/// \returns The number of bytes in the ring.
UINT32
PkGetOutgoingRingSize(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    )
{
    return PkLibContext->Outgoing.DataBytesInRing;
}


/// This function returns a snapshot of the number of bytes that are free within
/// the ring.  The number returned may be inaccurate by the time the function
/// returns, as packets may have been inserted or removed while the function was
/// executing.  Consequently, this function is only useful as a general gauge of
/// activity.
///
/// \param PkLibContext Pointer to the packet library context structure.
///
/// \returns The number of bytes that were free in the ring at some point during
///     the execution of this function.
UINT32
PkGetIncomingRingFreeBytes(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    )
{
    UINT32          currentOut;
    UINT32          currentIn;
    UINT32          dataBytesInRing;

    dataBytesInRing = PkLibContext->Incoming.DataBytesInRing;
    currentOut = PkLibContext->IncomingOut;
    currentIn = ReadNoFence((LONG*)&PkLibContext->Incoming.Control->In);
    if (!PkpValidatePointer(dataBytesInRing, currentIn))
    {
        return 0;
    }

    return PkpFreeBytes(dataBytesInRing, currentIn, currentOut);
}


/// This function returns a snapshot of the number of bytes that are free within
/// the ring.  The number returned may be inaccurate by the time the function
/// returns, as packets may have been inserted or removed while the function was
/// executing.  Consequently, this function is only useful as a general gauge of
/// activity.
///
/// \param PkLibContext Pointer to the packet library context structure.
///
/// \returns The number of bytes that were free in the ring at some point during
///     the execution of this function.
UINT32
PkGetOutgoingRingFreeBytes(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    )
{
    UINT32          currentOut;
    UINT32          currentIn;
    UINT32          dataBytesInRing;

    dataBytesInRing = PkLibContext->Outgoing.DataBytesInRing;
    currentIn = PkLibContext->OutgoingIn;
    currentOut = ReadNoFence((LONG*)&PkLibContext->Outgoing.Control->Out);
    if (!PkpValidatePointer(dataBytesInRing, currentOut))
    {
        return 0;
    }

    return PkpFreeBytes(dataBytesInRing, currentIn, currentOut);
}


/// This function returns a snapshot of the number of bytes that are available
/// to be consumed from the ring.  The number returned may be inaccurate by the
/// time the function returns, as packets may have been inserted or removed
/// while the function was executing.  Consequently, this function is only
/// useful as a general gauge of activity.
///
/// \param PkLibContext Pointer to the packet library context structure.
///
/// \returns The number of bytes that were available in the ring at some point
///     during the execution of this function.
UINT32
PkGetIncomingRingAvailableBytes(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    )
{
    UINT32          currentOut;
    UINT32          currentIn;
    UINT32          dataBytesInRing;

    dataBytesInRing = PkLibContext->Incoming.DataBytesInRing;
    currentOut = PkLibContext->IncomingOut;
    currentIn = ReadNoFence((LONG*)&PkLibContext->Incoming.Control->In);
    if (!PkpValidatePointer(dataBytesInRing, currentIn))
    {
        return 0;
    }

    return PkpDataAvailable(dataBytesInRing, currentIn, currentOut);
}


/// This function returns a snapshot of the number of bytes that are available
/// to be consumed from the ring.  The number returned may be inaccurate by the
/// time the function returns, as packets may have been inserted or removed
/// while the function was executing.  Consequently, this function is only
/// useful as a general gauge of activity.
///
/// \param PkLibContext Pointer to the packet library context structure.
///
/// \returns The number of bytes that were available in the ring at some point
///     during the execution of this function.
UINT32
PkGetOutgoingRingAvailableBytes(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    )
{
    UINT32          currentOut;
    UINT32          currentIn;
    UINT32          dataBytesInRing;

    dataBytesInRing = PkLibContext->Outgoing.DataBytesInRing;
    currentIn = PkLibContext->OutgoingIn;
    currentOut = ReadNoFence((LONG*)&PkLibContext->Outgoing.Control->Out);
    if (!PkpValidatePointer(dataBytesInRing, currentOut))
    {
        return 0;
    }

    return PkpDataAvailable(dataBytesInRing, currentIn, currentOut);
}


/// This routine masks/unmasks the interrupts on this endpoint. This involves
/// setting a flag for the opposite endpoint to look at when deciding whether to
/// send an interrupt to this endpoint.
///
/// \param PkLibContext Struct containing all of the lib's context.
/// \param Mask Supplies the Mask value to mask or unmask the interrupts.
VOID
PkSetInterruptMask(
    _In_    PPACKET_LIB_CONTEXT PkLibContext,
    _In_    BOOLEAN             Mask
    )
{
    WriteNoFence((LONG*)&PkLibContext->Incoming.Control->InterruptMask, Mask);
}


/// This routine determines whether incoming interrupts are currently masked.
///
/// \param PkLibContext A pointer to the packet library context.
///
/// \returns TRUE if interrupts are currently masked.
BOOLEAN
PkAreIncomingInterruptsMasked(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    )
{
    return PkLibContext->Incoming.Control->InterruptMask != 0;
}


/// This routine computes the number of interrupts that are expected to arrive
/// from the opposite endpoint.
///
/// \param PkLibContext A pointer to the packet library context.
///
/// \returns TRUE if interrupts are currently masked.
UINT32
PkpExpectedInterruptCount(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    )
{
    return PkLibContext->EmptyRingBufferCount +
           PkLibContext->FullRingBufferCount -
           PkLibContext->NonspuriousInterruptCount;

}


/// This function is invoked when an interrupt arrives.  It records the event
/// and, by examining other ring-related state, determines whether the interrupt
/// was valid or possibly spurious.
///
/// Note that there are race conditions in interrupt delivery and the return
/// value from this function is not 100% reliable.  It is, however, good enough
/// to be used as the basis for a spurious interrupt counter.
///
/// \param PkLibContext Struct containing all of the lib's context.
///
/// \retval TRUE There was a condition which should have prompted an interrupt.
///     FALSE   - There was not a condition which should have prompted an
///     interrupt.
BOOLEAN
PkInterruptArrived(
    _In_    PPACKET_LIB_CONTEXT PkLibContext
    )
{
    if (PkpExpectedInterruptCount(PkLibContext) > 0)
    {
        PkLibContext->NonspuriousInterruptCount += 1;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/// This routine increments the number of interrupts that are expected, up to an
/// upper bound of MAXIMUM_EXPECTED_INTERRUPT_COUNT. This count is decremented
/// in PkInterruptArrived.
///
/// \param PkLibContext Struct containing all of the lib's context.
/// \param IsIncoming If TRUE, expect an interrupt because the ring buffer is
///     empty. Otherwise, expect one because it is full.
VOID
PkpExpectInterrupt(
    _In_ PPACKET_LIB_CONTEXT PkLibContext,
    _In_ BOOLEAN IsIncoming
    )
{
    if (PkpExpectedInterruptCount(PkLibContext) < MAXIMUM_EXPECTED_INTERRUPT_COUNT)
    {
        if (IsIncoming)
        {
            PkLibContext->EmptyRingBufferCount += 1;
        }
        else
        {
            PkLibContext->FullRingBufferCount += 1;
        }
    }
}


/// This routine calculates the location of the data inside a packet that was
/// just received via PkReceivePacket.  At this point we know that Length8
/// represents the size of the buffer (and that it is at least the size of the
/// header) but we need to verify that DataOffset8 is valid and that the packet
/// contains at least MinimumDataSize bytes.
///
/// \param PacketBuf Pointer to the header of a packet that was received from
///     the ring buffer
/// \param MinimumDataSize Minimum size required for success
/// \param ActualDataSize Optional pointer to location that will receive the
///     actual data size
/// \param StartOfData Pointer to location that will point to the start of the
///     payload if the function succeeds.
///
/// \retval STATUS_SUCCESS StartOfData points the the beginning of the data, and
///     is guaranteed to be at least MinimumDataSize bytes long.
/// \retval STATUS_BUFFER_TOO_SMALL The packet does contain the minimum number
///     of bytes in the payload.  ActualDataSize is computed.
/// \retval STATUS_INVALID_PARAMETER DataOffset8 is invalid. ActualDataSize is
///     not set.
NTSTATUS
PkGetPacketData(
    _In_reads_bytes_(PacketBuf->Length8 * 8) PVMPACKET_DESCRIPTOR  PacketBuf,
    _In_                                UINT32                MinimumDataSize,
    _Out_opt_                           PUINT32               ActualDataSize,
    _Out_                               PVOID*                StartOfData
    )
{
    ULONG dataSize;

    if (PacketBuf->DataOffset8 > PacketBuf->Length8)
    {
        return STATUS_INVALID_PARAMETER;
    }

    dataSize = (PacketBuf->Length8 - PacketBuf->DataOffset8) * 8;
    if (ActualDataSize != NULL)
    {
        *ActualDataSize = dataSize;
    }

    if (dataSize < MinimumDataSize)
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    *StartOfData = (PVOID)((PUCHAR)PacketBuf + PacketBuf->DataOffset8 * 8);
    return STATUS_SUCCESS;
}


/// Returns whether the opposite endpoint will deterministically request and
/// send ring-full interrupts when appropriate.
///
/// If this returns FALSE, the caller must ensure that a retry mechanism is in
/// use when the ring buffer fills up or make other arrangements to find out
/// when the ring buffer has enough space available.
///
/// \param PkLibContext A pointer to the packet library context.
///
/// \returns TRUE if the opposite endpoint fully supports ring-full interrupts.
BOOLEAN
PkSupportsRingFullInterrupts(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    )
{
    PVMRCB control;

    control = PkLibContext->Incoming.Control;
    return control->FeatureBits.SupportsPendingSendSize != 0;
}

