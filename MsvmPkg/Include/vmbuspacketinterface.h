/*----------------------------------------------------------------------------
 $Microsoft Confidential$
 $Copyright (C) 2004 Microsoft Corporation.  All Rights Reserved.$

 File: VmbusPacketInterface.w

 Abstract:

     This file contains the structures that define the transport-related
     interfaces exported by the VMBus driver.

----------------------------------------------------------------------------*/

#pragma once

#include <VmbusPacketFormat.h>

//
// These structures should all be treated as opaque. Use the accessor methods
// below.
//

//
// The entire ring context is immutable.
//

typedef struct _PACKET_RING_CONTEXT
{
    __volatile VMRCB *          Control;
    volatile UCHAR *            Data;
    UINT32                      DataBytesInRing;
} PACKET_RING_CONTEXT, *PPACKET_RING_CONTEXT;

typedef struct _PACKET_LIB_CONTEXT
{
    //
    // R/O or near R/O fields. Try to keep these together in a cache line.
    //

    PACKET_RING_CONTEXT     Outgoing;
    PACKET_RING_CONTEXT     Incoming;
#ifndef VMBUS_RING_BUFFER_SINGLE_MAPPED
    PMDL                    IncomingDoubleMappedMdl;
    PMDL                    OutgoingDoubleMappedMdl;
#endif

    //
    // Incoming loop mutable fields. Keep these on their own cache line.
    //

    DECLSPEC_CACHEALIGN
    UINT32                  IncomingInCache;
    UINT32                  IncomingOut;
    UINT32                  EmptyRingBufferCount;
    UINT32                  NonspuriousInterruptCount;

    //
    // Outgoing loop mutable fields. Keep these on their own cache line.
    //

    DECLSPEC_CACHEALIGN
    UINT32                  OutgoingIn;
    UINT32                  OutgoingOutCache;
    UINT32                  PendingSendSize;
    UINT32                  FullRingBufferCount;
    UINT64                  StaticInterruptMaskSkips;
    PUINT64                 InterruptMaskSkips;

} PACKET_LIB_CONTEXT, *PPACKET_LIB_CONTEXT;

typedef PPACKET_LIB_CONTEXT PACKET_LIB_HANDLE;

#ifndef VMBUS_RING_BUFFER_SINGLE_MAPPED

NTSTATUS
PkInitializeDoubleMappedRingBuffer(
    __out PPACKET_LIB_CONTEXT Context,
    __in_bcount(PAGE_SIZE) PVOID IncomingControl,
    __in_bcount(PAGE_SIZE * IncomingDataPageCount) PVOID IncomingDataPages,
    __in_range(>, 0) UINT32 IncomingDataPageCount,
    __in_bcount(PAGE_SIZE) PVOID OutgoingControl,
    __in_bcount(PAGE_SIZE * OutgoingDataPageCount) PVOID OutgoingDataPages,
    __in_range(>, 0) UINT32 OutgoingDataPageCount
    );

NTSTATUS
PkInitializeRingBuffer(
    __out PPACKET_LIB_CONTEXT Context,
    __in_bcount(PAGE_SIZE) PVOID IncomingControl,
    __in_bcount(PAGE_SIZE * IncomingDataPageCount) PVOID IncomingDataPages,
    __in_range(>, 0) UINT32 IncomingDataPageCount,
    __in_bcount(PAGE_SIZE) PVOID OutgoingControl,
    __in_bcount(PAGE_SIZE * OutgoingDataPageCount) PVOID OutgoingDataPages,
    __in_range(>, 0) UINT32 OutgoingDataPageCount
    );

VOID
PkUninitializeRingBuffer(
    __in PPACKET_LIB_CONTEXT PkLibContext
    );

NTSTATUS
PkInit(
    __in_bcount(PAGE_SIZE * RingBufferPageCount)
                PVOID                  RingBufferPages,
    __in        UINT32                 RingBufferPageCount,
    __in        UINT32                 ClientToServerPages,
    __in        ENDPOINT_TYPE          EndpointType,
    __in_opt    UINT32                 IncomingTransactionQuota,
    __out __deref __drv_allocatesMem(Mem)
                PACKET_LIB_HANDLE      *PkLibContext
    );

VOID
PkCleanup(
    __in __drv_freesMem(Mem) PACKET_LIB_HANDLE PkLibContext
    );

_Must_inspect_result_
NTSTATUS
PkSendPacketSimple(
    __in                        PACKET_LIB_HANDLE   PkLibContext,
    __in_bcount(PacketBufSize)  PVOID               PacketBuf,
    __in                        UINT32              PacketBufSize,
    __in                        UINT64              TransactionId,
    __in                        BOOLEAN             RequestCompletion
    );

_Must_inspect_result_
NTSTATUS
PkSendPacketGpaDirect(
    __in                        PACKET_LIB_HANDLE  PkLibContext,
    __in_bcount(PacketBufSize)  PVOID              PacketBuf,
    __in                        UINT32             PacketBufSize,
    __in                        UINT64             TransactionId,
    __in                        PMDL               Mdl,
    __in_opt                    UINT32             StartOffsetWithinMdl,
    __in_opt                    UINT32             DataLengthWithinMdl,
    __in_opt                    BOOLEAN            IsDataLengthWithinMdlForced
    );

_Must_inspect_result_
NTSTATUS
PkSendCancel(
    __in  PACKET_LIB_HANDLE PkLibContext,
    __in  UINT64            TransactionId
    );

_Must_inspect_result_
NTSTATUS
PkSendCompletion(
    __in                            PACKET_LIB_HANDLE  PkLibContext,
    __in                            UINT64             TransactionId,
    __in_bcount_opt(PacketBufSize)  PVOID              PacketBuf,
    __in                            UINT32             PacketBufSize,
    __in                            BOOLEAN            CompletionAlreadyCounted
    );

_Must_inspect_result_
NTSTATUS
PkReceivePacket(
    __in                            PACKET_LIB_HANDLE       PkLibContext,
    __out_bcount(PacketBufSize)     PVMPACKET_DESCRIPTOR    PacketBuf,
    __in_range(>=, sizeof(VMPACKET_DESCRIPTOR))
                                    UINT32                  PacketBufSize,
    __out_opt                       PUINT32                 PacketBufSizeNeeded
    );

NTSTATUS
PkGetReceiveBuffer(
    __in PACKET_LIB_HANDLE PkLibContext,
    __inout PUINT32 Offset,
    __out __deref_bcount(*Length) PVOID *Buffer,
    __out __deref_out_range(>=, sizeof(VMPACKET_DESCRIPTOR)) PUINT32 Length
    );

NTSTATUS
PkGetSendBuffer(
    __in PACKET_LIB_HANDLE PkLibContext,
    __inout PUINT32 Offset,
    __in_range(>, 0) UINT32 PacketSize,
    __out __deref_out_bcount(PacketSize) PVOID *Buffer
    );

_Must_inspect_result_
NTSTATUS
PkSendPacketRaw(
    __in                        PACKET_LIB_HANDLE  PkLibContext,
    __in_bcount(PacketBufSize)  PVOID              PacketBuf,
    __in_range(>, 0)            UINT32             PacketBufSize
    );

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
    );

_Must_inspect_result_
NTSTATUS
PkSendPacketSingleMapped(
    __in                        PPACKET_LIB_CONTEXT PkLibContext,
    __in_bcount(PacketBufSize)  PVOID               PacketBuf,
    __in_range(>, 0)            UINT32              PacketBufSize
    );

_Must_inspect_result_
NTSTATUS
PkGetIncomingPacketSize(
    _In_  PPACKET_LIB_CONTEXT PkLibContext,
    _Out_ PUINT32             PacketSize,
    _Out_ PUINT32             OutPointer
    );

_Must_inspect_result_
NTSTATUS
PkReceivePacketSingleMapped(
    __in                        PPACKET_LIB_CONTEXT     PkLibContext,
    __out_bcount(PacketBufSize) PVMPACKET_DESCRIPTOR    PacketBuf,
    __in_range(>=, sizeof(VMPACKET_DESCRIPTOR))
                                UINT32                  PacketBufSize,
    _In_                        UINT32                  Out
    );

#endif

UINT32
PkGetIncomingRingSize(
    __in PACKET_LIB_HANDLE PkLibContext
    );

UINT32
PkGetOutgoingRingSize(
    __in PACKET_LIB_HANDLE PkLibContext
    );

UINT32
PkGetIncomingRingFreeBytes(
    __in PACKET_LIB_HANDLE PkLibContext
    );

UINT32
PkGetOutgoingRingFreeBytes(
    __in PACKET_LIB_HANDLE PkLibContext
    );

UINT32
PkGetIncomingRingAvailableBytes(
    __in PACKET_LIB_HANDLE PkLibContext
    );

UINT32
PkGetOutgoingRingAvailableBytes(
    __in PACKET_LIB_HANDLE PkLibContext
    );

VOID
PkSetInterruptMask(
    __in    PACKET_LIB_HANDLE   PkLibContext,
    __in    BOOLEAN             Mask
    );

BOOLEAN
PkAreIncomingInterruptsMasked(
    __in PPACKET_LIB_CONTEXT PkLibContext
    );

VOID
PkSetInterruptMaskSkipCount(
    __in PPACKET_LIB_CONTEXT PkLibContext,
    __in PUINT64 MaskSkips
    );

BOOLEAN
PkInterruptArrived(
    __in PACKET_LIB_HANDLE PkLibContext
    );

//
// Transfer pages are deprecated; new packet library clients should manage
// GPADL ranges internally.
//

DECLSPEC_DEPRECATED
_Must_inspect_result_
NTSTATUS
PkSendPacketTransferPage(
    __in                        PACKET_LIB_HANDLE       PkLibContext,
    __in_bcount(PacketBufSize)  PVOID                   PacketBuf,
    __in                        UINT32                  PacketBufSize,
    __in                        UINT64                  TransactionId,
    __in                        UINT16                  TransferPageSetId,
    __in                        PVMTRANSFER_PAGE_RANGES Ranges,
    __in                        BOOLEAN                 SenderOwnsSet
    );

NTSTATUS
PkParseGpaRanges(
    __in_bcount(RangeBufSize)       PGPA_RANGE  GpaRanges,
    __in                            UINT32      RangeBufSize,
    __in                            UINT32      RangeCount,
    __out_ecount_opt(ArrayCount)    PGPA_RANGE* RangeArray,
    __out_opt                       UINT32*     TotalPfnCount
    );

NTSTATUS
PkParseGpaDirectRanges(
    __in_bcount(PacketBufSize)  PVMDATA_GPA_DIRECT  GpaDirectPacket,
    __in                        UINT32              PacketBufSize,
    __out_ecount(ArrayCount)    PGPA_RANGE*         RangeArray,
    __out                       UINT32*             ArrayCount,
    __out_opt                   UINT32*             TotalPfnCount
    );

#ifndef VMBUS_RING_BUFFER_SINGLE_MAPPED

NTSTATUS
PkCalculateMdlOffsets(
    __in                        PMDL        MdlBuffer,
    __in                        UINT32      MdlBufSize,
    __in                        PGPA_RANGE  GpaRanges,
    __in                        UINT32      RangeCount,
    __out_ecount(RangeCount)    PMDL*       MdlArray
    );

UINT32
PkCalculateGpaRangesSize(
    __in        PMDL    Mdl,
    __out_opt   PUINT32 RangeCount,
                UINT32  DataStartOffset,
    __in        UINT32  DataLength,
    __in        BOOLEAN IsDataLengthForced
    );

VOID
PkCreateGpaRanges(
    __in                        PMDL       Mdl,
    __out_bcount_full(RangeBufSize)
                                PGPA_RANGE RangeBuf,
    __in                        UINT32     RangeBufSize,
    __in                        UINT32     DataStartOffset,
    __in                        UINT32     DataLength,
    __in                        BOOLEAN    IsDataLengthForced
    );

#endif

NTSTATUS
PkGetPacketData(
    __in_bcount(PacketBuf->Length8 * 8) PVMPACKET_DESCRIPTOR  PacketBuf,
    __in                                UINT32                MinimumDataSize,
    __out_opt                           PUINT32               ActualDataSize,
    __out                               PVOID*                StartOfData
    );

BOOLEAN
PkIsIncomingRingEmpty(
    __in PACKET_LIB_HANDLE PkLibContext
    );

BOOLEAN
PkIsOutgoingRingFull(
    __in PACKET_LIB_HANDLE PkLibContext,
    __in UINT32 SendSize
    );

UINT32
PkGetLastFailedSendSize(
    __in PACKET_LIB_HANDLE PkLibContext
    );

UINT32
PkGetIncomingRingOffset(
    __in PACKET_LIB_HANDLE PkLibContext
    );

UINT32
PkGetOutgoingRingOffset(
    __in PACKET_LIB_HANDLE PkLibContext
    );

NTSTATUS
PkCompleteRemoval(
    __in  PACKET_LIB_HANDLE   PkLibContext,
    __in  UINT32                NewOut
    );

NTSTATUS
PkCompleteInsertion(
    __in  PACKET_LIB_HANDLE PkLibContext,
    __in  UINT32            NewIn
    );

BOOLEAN
PkSupportsRingFullInterrupts(
    __in PPACKET_LIB_CONTEXT PkLibContext
    );

