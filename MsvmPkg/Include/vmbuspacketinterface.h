/*----------------------------------------------------------------------------
 $Microsoft Confidential$
 $Copyright (C) 2004 Microsoft Corporation.  All Rights Reserved.$

 File: VmbusPacketInterface.h

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


NTSTATUS
PkInitializeDoubleMappedRingBuffer(
    _Out_ PPACKET_LIB_CONTEXT Context,
    _In_reads_bytes_(PAGE_SIZE) PVOID IncomingControl,
    _In_reads_bytes_(PAGE_SIZE * IncomingDataPageCount) PVOID IncomingDataPages,
    _In_range_(>, 0) UINT32 IncomingDataPageCount,
    _In_reads_bytes_(PAGE_SIZE) PVOID OutgoingControl,
    _In_reads_bytes_(PAGE_SIZE * OutgoingDataPageCount) PVOID OutgoingDataPages,
    _In_range_(>, 0) UINT32 OutgoingDataPageCount
    );

NTSTATUS
PkInitializeRingBuffer(
    _Out_ PPACKET_LIB_CONTEXT Context,
    _In_reads_bytes_(PAGE_SIZE) PVOID IncomingControl,
    _In_reads_bytes_(PAGE_SIZE * IncomingDataPageCount) PVOID IncomingDataPages,
    _In_range_(>, 0) UINT32 IncomingDataPageCount,
    _In_reads_bytes_(PAGE_SIZE) PVOID OutgoingControl,
    _In_reads_bytes_(PAGE_SIZE * OutgoingDataPageCount) PVOID OutgoingDataPages,
    _In_range_(>, 0) UINT32 OutgoingDataPageCount
    );

VOID
PkUninitializeRingBuffer(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    );

NTSTATUS
PkInit(
    _In_reads_bytes_(PAGE_SIZE * RingBufferPageCount)
                PVOID                  RingBufferPages,
    _In_        UINT32                 RingBufferPageCount,
    _In_        UINT32                 ClientToServerPages,
    _In_        ENDPOINT_TYPE          EndpointType,
    _In_opt_    UINT32                 IncomingTransactionQuota,
    _Out_ __deref __drv_allocatesMem(Mem)
                PACKET_LIB_HANDLE      *PkLibContext
    );

VOID
PkCleanup(
    _In_ __drv_freesMem(Mem) PACKET_LIB_HANDLE PkLibContext
    );

#ifndef VMBUS_RING_BUFFER_SINGLE_MAPPED

#define PkWriteRingBuffer(_LibContext_,_Dest_,_Src_,_Length_) RtlCopyMemory((_Dest_), (_Src_), (_Length_))
#define PkReadRingBuffer(_LibContext_,_Dest_,_Src_,_Length_) RtlCopyMemory((_Dest_), (_Src_), (_Length_))

#define PkWriteRingBufferField(singledest, singlesrc) \
    (singledest) = (singlesrc)

#define PkReadRingBufferField(singledest, singlesrc) \
    (singledest) = (singlesrc)

#else

static_assert(FIELD_OFFSET(VMPACKET_DESCRIPTOR, Type) < 8,
    "VMPACKET_DESCRIPTOR->Type is assumed to be within first 8 bytes of the structure.");
static_assert(FIELD_OFFSET(VMPACKET_DESCRIPTOR, DataOffset8) < 8,
    "VMPACKET_DESCRIPTOR->DataOffset8 is assumed to be within first 8 bytes of the structure.");
static_assert(FIELD_OFFSET(VMPACKET_DESCRIPTOR, Length8) < 8,
    "VMPACKET_DESCRIPTOR->Length8 is assumed to be within first 8 bytes of the structure.");
static_assert(FIELD_OFFSET(VMPACKET_DESCRIPTOR, Flags) < 8,
    "VMPACKET_DESCRIPTOR->Flags is assumed to be within first 8 bytes of the structure.");

#define PkWriteRingBuffer(_LibContext_,_Dest_,_Src_,_Length_) \
    PkWritePacketSingleMapped((_LibContext_), \
        (_Src_), \
        (_Length_), \
        (UINT32)((PUINT8)(_Dest_) - (PUINT8)((_LibContext_)->Outgoing.Data))); \

#define PkReadRingBuffer(_LibContext_,_Dest_,_Src_,_Length_) \
    PkReadPacketSingleMapped((_LibContext_), \
        (_Dest_), \
        (_Length_), \
        (UINT32)((PUINT8)(_Src_) - (PUINT8)((_LibContext_)->Incoming.Data))); \

#define PkWriteRingBufferField(singledest, singlesrc) \
    { \
        UINT64 _local_value_ = (singlesrc); \
        static_assert(sizeof((singledest)) <= 8, "PkWriteRingBufferField requires the field to be <= size 8"); \
        PkWriteRingBuffer(PkLibContext, &(singledest), &_local_value_, sizeof((singlesrc))); \
    }

#define PkReadRingBufferField(singledest, singlesrc) \
    PkReadRingBuffer(PkLibContext, &(singledest), &(singlesrc), sizeof((singlesrc)))

VOID
PkWritePacketSingleMapped(
    _In_                        PPACKET_LIB_CONTEXT PkLibContext,
    _In_reads_bytes_(PacketBufSize)  PVOID               PacketBuf,
    _In_range_(>, 0)            UINT32              PacketBufSize,
    _In_                        UINT32              Offset
    );

VOID
PkReadPacketSingleMapped(
    _In_                        PPACKET_LIB_CONTEXT     PkLibContext,
    _Out_writes_bytes_(PacketBufSize) PVOID                   PacketBuf,
    _In_                        UINT32                  PacketBufSize,
    _In_                        UINT32                  Out
    );

NTSTATUS
PkInitializeSingleMappedRingBuffer(
    _Out_ PPACKET_LIB_CONTEXT Context,
    _In_reads_bytes_(PAGE_SIZE) PVOID IncomingControl,
    _In_reads_bytes_(PAGE_SIZE * IncomingDataPageCount) PVOID IncomingDataPages,
    _In_range_(>, 0) UINT32 IncomingDataPageCount,
    _In_reads_bytes_(PAGE_SIZE) PVOID OutgoingControl,
    _In_reads_bytes_(PAGE_SIZE * OutgoingDataPageCount) PVOID OutgoingDataPages,
    _In_range_(>, 0) UINT32 OutgoingDataPageCount
    );

#define PkSendPacketSingleMapped PkSendPacketRaw
#define PkReceivePacketSingleMapped(PkLibContext, PacketBuf, PacketBufSize) \
    PkReceivePacket((PkLibContext), (PacketBuf), *(PacketBufSize), (PacketBufSize))

#endif

_Must_inspect_result_
NTSTATUS
PkSendPacketSimple(
    _In_                        PACKET_LIB_HANDLE   PkLibContext,
    _In_reads_bytes_(PacketBufSize)  PVOID               PacketBuf,
    _In_                        UINT32              PacketBufSize,
    _In_                        UINT64              TransactionId,
    _In_                        BOOLEAN             RequestCompletion
    );

_Must_inspect_result_
NTSTATUS
PkSendCancel(
    _In_  PACKET_LIB_HANDLE PkLibContext,
    _In_  UINT64            TransactionId
    );

_Must_inspect_result_
NTSTATUS
PkSendCompletion(
    _In_                            PACKET_LIB_HANDLE  PkLibContext,
    _In_                            UINT64             TransactionId,
    _In_reads_bytes_opt_(PacketBufSize)  PVOID              PacketBuf,
    _In_                            UINT32             PacketBufSize,
    _In_                            BOOLEAN            CompletionAlreadyCounted
    );

_Must_inspect_result_
NTSTATUS
PkReceivePacket(
    _In_                            PACKET_LIB_HANDLE       PkLibContext,
    _Out_writes_bytes_(PacketBufSize)     PVMPACKET_DESCRIPTOR    PacketBuf,
    _In_range_(>=, sizeof(VMPACKET_DESCRIPTOR))
                                    UINT32                  PacketBufSize,
    _Out_opt_                       PUINT32                 PacketBufSizeNeeded
    );

_Must_inspect_result_
NTSTATUS
PkSendPacketRaw(
    _In_                        PACKET_LIB_HANDLE  PkLibContext,
    _In_reads_bytes_(PacketBufSize)  PVOID              PacketBuf,
    _In_range_(>, 0)            UINT32             PacketBufSize
    );

NTSTATUS
PkGetReceiveBuffer(
    _In_ PACKET_LIB_HANDLE PkLibContext,
    _Inout_ PUINT32 Offset,
    _Out_ __deref_bcount(*Length) PVOID *Buffer,
    _Out_ _Deref_out_range_(>=, sizeof(VMPACKET_DESCRIPTOR)) PUINT32 Length
    );

NTSTATUS
PkGetSendBuffer(
    _In_ PACKET_LIB_HANDLE PkLibContext,
    _Inout_ PUINT32 Offset,
    _In_range_(>, 0) UINT32 PacketSize,
    _Out_ _Outptr_result_bytebuffer_(PacketSize) PVOID *Buffer
    );

UINT32
PkGetIncomingRingSize(
    _In_ PACKET_LIB_HANDLE PkLibContext
    );

UINT32
PkGetOutgoingRingSize(
    _In_ PACKET_LIB_HANDLE PkLibContext
    );

UINT32
PkGetIncomingRingFreeBytes(
    _In_ PACKET_LIB_HANDLE PkLibContext
    );

UINT32
PkGetOutgoingRingFreeBytes(
    _In_ PACKET_LIB_HANDLE PkLibContext
    );

UINT32
PkGetIncomingRingAvailableBytes(
    _In_ PACKET_LIB_HANDLE PkLibContext
    );

UINT32
PkGetOutgoingRingAvailableBytes(
    _In_ PACKET_LIB_HANDLE PkLibContext
    );

VOID
PkSetInterruptMask(
    _In_    PACKET_LIB_HANDLE   PkLibContext,
    _In_    BOOLEAN             Mask
    );

BOOLEAN
PkAreIncomingInterruptsMasked(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    );

VOID
PkSetInterruptMaskSkipCount(
    _In_ PPACKET_LIB_CONTEXT PkLibContext,
    _In_ PUINT64 MaskSkips
    );

BOOLEAN
PkInterruptArrived(
    _In_ PACKET_LIB_HANDLE PkLibContext
    );

//
// Transfer pages are deprecated; new packet library clients should manage
// GPADL ranges internally.
//

DECLSPEC_DEPRECATED
_Must_inspect_result_
NTSTATUS
PkSendPacketTransferPage(
    _In_                        PACKET_LIB_HANDLE       PkLibContext,
    _In_reads_bytes_(PacketBufSize)  PVOID                   PacketBuf,
    _In_                        UINT32                  PacketBufSize,
    _In_                        UINT64                  TransactionId,
    _In_                        UINT16                  TransferPageSetId,
    _In_                        PVMTRANSFER_PAGE_RANGES Ranges,
    _In_                        BOOLEAN                 SenderOwnsSet
    );

NTSTATUS
PkParseGpaRanges(
    _In_reads_bytes_(RangeBufSize)       PGPA_RANGE  GpaRanges,
    _In_                            UINT32      RangeBufSize,
    _In_                            UINT32      RangeCount,
    _Out_writes_opt_(RangeCount)    PGPA_RANGE* RangeArray,
    _Out_opt_                       UINT32*     TotalPfnCount
    );

NTSTATUS
PkParseGpaDirectRanges(
    _In_reads_bytes_(PacketBufSize)  PVMDATA_GPA_DIRECT  GpaDirectPacket,
    _In_                        UINT32              PacketBufSize,
    _Out_writes_(ArrayCount)    PGPA_RANGE*         RangeArray,
    _Out_                       UINT32*             ArrayCount,
    _Out_opt_                   UINT32*             TotalPfnCount
    );

#if !defined(VMBUS_RING_BUFFER_SINGLE_MAPPED) || defined(VMBUS_RING_BUFFER_FULL_SUPPORT)

_Must_inspect_result_
NTSTATUS
PkSendPacketGpaDirect(
    _In_                        PACKET_LIB_HANDLE  PkLibContext,
    _In_reads_bytes_(PacketBufSize)  PVOID              PacketBuf,
    _In_                        UINT32             PacketBufSize,
    _In_                        UINT64             TransactionId,
    _In_                        PMDL               Mdl,
    _In_opt_                    UINT32             StartOffsetWithinMdl,
    _In_opt_                    UINT32             DataLengthWithinMdl,
    _In_opt_                    BOOLEAN            IsDataLengthWithinMdlForced
    );

NTSTATUS
PkCalculateMdlOffsets(
    _In_                        PMDL        MdlBuffer,
    _In_                        UINT32      MdlBufSize,
    _In_                        PGPA_RANGE  GpaRanges,
    _In_                        UINT32      RangeCount,
    _Out_writes_(RangeCount)    PMDL*       MdlArray
    );

UINT32
PkCalculateGpaRangesSize(
    _In_        PMDL    Mdl,
    _Out_opt_   PUINT32 RangeCount,
                UINT32  DataStartOffset,
    _In_        UINT32  DataLength,
    _In_        BOOLEAN IsDataLengthForced
    );

#define PkCreateGpaRanges(_mdl, _rangebuf, _rangebufsize, _datastartoffset, _datalength, _isdatalengthforced) \
    PkCreateGpaRangesEx(NULL, _mdl, _rangebuf, _rangebufsize, _datastartoffset, _datalength, _isdatalengthforced)

VOID
PkCreateGpaRangesEx(
    _In_opt_                    PPACKET_LIB_CONTEXT PkLibContext,
    _In_                        PMDL       Mdl,
    __out_bcount_full(RangeBufSize)
                                PGPA_RANGE RangeBuf,
    _In_                        UINT32     RangeBufSize,
    _In_                        UINT32     DataStartOffset,
    _In_                        UINT32     DataLength,
    _In_                        BOOLEAN    IsDataLengthForced
    );

#endif

NTSTATUS
PkGetPacketData(
    _In_reads_bytes_(PacketBuf->Length8 * 8) PVMPACKET_DESCRIPTOR  PacketBuf,
    _In_                                UINT32                MinimumDataSize,
    _Out_opt_                           PUINT32               ActualDataSize,
    _Out_                               PVOID*                StartOfData
    );

BOOLEAN
PkIsIncomingRingEmpty(
    _In_ PACKET_LIB_HANDLE PkLibContext
    );

BOOLEAN
PkIsOutgoingRingFull(
    _In_ PACKET_LIB_HANDLE PkLibContext,
    _In_ UINT32 SendSize
    );

UINT32
PkGetLastFailedSendSize(
    _In_ PACKET_LIB_HANDLE PkLibContext
    );

UINT32
PkGetIncomingRingOffset(
    _In_ PACKET_LIB_HANDLE PkLibContext
    );

UINT32
PkGetOutgoingRingOffset(
    _In_ PACKET_LIB_HANDLE PkLibContext
    );

NTSTATUS
PkCompleteRemoval(
    _In_  PACKET_LIB_HANDLE   PkLibContext,
    _In_  UINT32                NewOut
    );

NTSTATUS
PkCompleteInsertion(
    _In_  PACKET_LIB_HANDLE PkLibContext,
    _In_  UINT32            NewIn
    );

BOOLEAN
PkSupportsRingFullInterrupts(
    _In_ PPACKET_LIB_CONTEXT PkLibContext
    );

