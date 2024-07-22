/** @file
    EFI Driver for Synthetic Network Controller

    Copyright (c) Microsoft Corporation.
    Licensed under the BSD-2-Clause-Patent license.
**/

#pragma once

#include <Uefi.h>
#include <EfiNt.h>

#include <Protocol/Vmbus.h>
#include <Protocol/Emcl.h>
#include <Protocol/SimpleNetwork.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/EmclLib.h>
#include <Library/SerialPortLib.h>
#include <Library/Printlib.h>

// TODO SCRUB Make generic protocol definitions, remove EfiNt.h?
//
// The following base NDIS types are referenced by nvspprotocol.h.
// Including the NT header (ntddndis.h) that defines them will pull
// in a lot of other unworkable include file dependencies. These
// types are not going to change and are only relevant to this
// network driver so simply define them here.
//
// Begin duplicated types from ntddndis.h
//
typedef struct _NDIS_OBJECT_HEADER
{
    UCHAR   Type;
    UCHAR   Revision;
    USHORT  Size;
} NDIS_OBJECT_HEADER, *PNDIS_OBJECT_HEADER;

typedef ULONG NDIS_OID, *PNDIS_OID;

typedef int NDIS_STATUS, *PNDIS_STATUS;
//
// End duplicated types from ntddndis.h
//

typedef UINT32 GPADL_HANDLE;

#include <NvspProtocol.h>

#define MAXIMUM_ETHERNET_PACKET_SIZE        1514

// TODO: Make the number of packets in the buffer a PCD variable.
#define NVSC_DEFAULT_RECEIVE_BUFFER_SIZE    MAXIMUM_ETHERNET_PACKET_SIZE * 128
#define NVSC_DEFAULT_SEND_BUFFER_SIZE       MAXIMUM_ETHERNET_PACKET_SIZE * 128

//
// VMBUS guid for synthetic NIC
//
DEFINE_GUID(GUID_NETWORK_CHANNEL_TYPE, 0xf8615163, 0xdf3e, 0x46c5, 0x91,
    0x3f, 0xf2, 0xd2, 0xf9, 0x65, 0xed, 0xe);

#define NETVSC_VERSION 1

typedef struct _ETHERNET_HEADER
{
    UINT8 DestAddr[PXE_HWADDR_LEN_ETHER];
    UINT8 SrcAddr[PXE_HWADDR_LEN_ETHER];
    UINT16 Type;
} ETHERNET_HEADER;

typedef struct _RX_PACKET_INSTANCE
{
    VOID * PacketContext;
    VOID * Buffer;
    UINT32 BufferLength;
    BOOLEAN CompletionNeeded;
} RX_PACKET_INSTANCE;

typedef struct _RX_QUEUE
{
    RX_PACKET_INSTANCE    *Buffer;
    UINT32                Length;
    UINT32                Head;
    UINT32                Tail;
} RX_QUEUE;

typedef struct _TX_QUEUE
{
    VOID      **Buffer;
    UINT32    Length;
    UINT32    Head;
    UINT32    Tail;
} TX_QUEUE;

typedef struct _NIC_DATA_INSTANCE
{
    EFI_EMCL_PROTOCOL         *Emcl;
    EFI_NETWORK_STATISTICS    Statistics;
    UINTN                     SupportedStatisticsSize;
    BOOLEAN                   MediaPresent;
    BOOLEAN                   EmclStarted;

    UINT8                     PermNodeAddress[PXE_MAC_LENGTH];
    UINT8                     CurrentNodeAddress[PXE_MAC_LENGTH];
    UINT8                     BroadcastNodeAddress[PXE_MAC_LENGTH];

    EFI_EVENT                 RxFilterEvt;
    EFI_STATUS                SetRxFilterStatus;
    EFI_EVENT                 StnAddrEvt;
    EFI_STATUS                GetStnAddrStatus;
    EFI_EVENT                 InitRndisEvt;
    EFI_STATUS                InitRndisStatus;

    VOID                      *RxBufferAllocation;
    VOID                      *RxBuffer;
    UINT32                    RxBufferPageCount;
    UINT32                    RxQueueCount;
    EFI_EMCL_GPADL            *RxGpadl;
    BOOLEAN                   RxInterrupt;
    BOOLEAN                   ReceiveStarted;
    UINT8                     RxFilter;

    VOID                      *TxBufferAllocation;
    VOID                      *TxBuffer;
    UINT32                    TxBufferPageCount;
    UINT32                    TxBufCount;
    UINT32                    TxSectionSize;
    EFI_EMCL_GPADL            *TxGpadl;
    BOOLEAN                   TxedInterrupt;

    RX_QUEUE                  RxPacketQueue;
    TX_QUEUE                  FreeTxBuffersQueue;
    TX_QUEUE                  TxedBuffersQueue;
} NIC_DATA_INSTANCE;

typedef struct _NETVSC_ADAPTER_CONTEXT
{
    EFI_HANDLE                  ControllerHandle;
    EFI_HANDLE                  DeviceHandle;
    EFI_DEVICE_PATH_PROTOCOL    *BaseDevPath;
    EFI_DEVICE_PATH_PROTOCOL    *DevPath;
    NIC_DATA_INSTANCE           NicInfo;
} NETVSC_ADAPTER_CONTEXT, *PNETVSC_ADAPTER_CONTEXT;

typedef struct _TX_PACKET_CONTEXT
{
    NIC_DATA_INSTANCE      *AdapterInfo;
    EFI_EXTERNAL_BUFFER    BufferInfo;
    VOID                   *TxBuffer;
} TX_PACKET_CONTEXT;

EFI_STATUS
NetvscInit(
    _In_ NIC_DATA_INSTANCE *AdapterInfo
    );

EFI_STATUS
NetvscShutdown(
    _In_ NIC_DATA_INSTANCE *AdapterInfo
    );

EFI_STATUS
NetvscSetFilter(
    _In_ NIC_DATA_INSTANCE *AdapterInfo,
    _In_ UINT32            newFilter
    );

EFI_STATUS
NetvscTransmit(
    _In_ NIC_DATA_INSTANCE               *AdapterInfo,
    _In_reads_bytes_(BufferSize) VOID    *Buffer,
    _In_ UINT32                          BufferSize
    );

VOID
NetvscReceiveCallback(
    _In_ VOID                                    *ReceiveContext,
    _In_ VOID                                    *PacketContext,
    _In_reads_bytes_(BufferLength) VOID          *Buffer,
    _In_ UINT32                                  BufferLength,
    _In_ UINT16                                  TransferPageSetId,
    _In_ UINT32                                  RangeCount,
    _In_reads_(RangeCount) EFI_TRANSFER_RANGE    *Ranges
    );

EFI_STATUS
NetvscReceive(
    _In_ NIC_DATA_INSTANCE                  *AdapterInfo,
    _Out_writes_bytes_(*BufferSize) VOID    *Buffer,
    _Inout_ UINTN                           *BufferSize,
    _Out_opt_ UINTN                         *HeaderSize,
    _Out_opt_ EFI_MAC_ADDRESS               *SrcAddr,
    _Out_opt_ EFI_MAC_ADDRESS               *DestAddr,
    _Out_opt_ UINT16                        *Protocol
    );

VOID
NetvscResetStatistics(
    _In_ NIC_DATA_INSTANCE *AdapterInfo
    );

EFI_STATUS
NvspStatusToEfiStatus(
    _In_ NVSP_STATUS nvspStatus
);

FORCEINLINE
EFI_STATUS
RxQueueInit(
    _In_ RX_QUEUE    *Queue,
    _In_ UINT32      Length
    );

FORCEINLINE
VOID
RxQueueDestroy(
    _In_ RX_QUEUE *Queue
    );

FORCEINLINE
BOOLEAN
RxQueueIsAlmostFull(
    _In_ RX_QUEUE *Queue
    );

FORCEINLINE
BOOLEAN
RxQueueIsEmpty(
    _In_ RX_QUEUE *Queue
    );

FORCEINLINE
VOID
RxQueueEnqueue(
    _In_ RX_QUEUE              *Queue,
    _In_ RX_PACKET_INSTANCE    *PacketInfo
    );

FORCEINLINE
VOID
RxQueueDequeue(
    _In_ RX_QUEUE               *Queue,
    _Out_ RX_PACKET_INSTANCE    *PacketInfo
    );

FORCEINLINE
EFI_STATUS
TxQueueInit(
    _In_ TX_QUEUE    *Queue,
    _In_ UINT32      Length
    );

FORCEINLINE
VOID
TxQueueDestroy(
    _In_ TX_QUEUE *Queue
    );

FORCEINLINE
BOOLEAN
TxQueueIsFull(
    _In_ TX_QUEUE *Queue
    );

FORCEINLINE
BOOLEAN
TxQueueIsEmpty(
    _In_ TX_QUEUE *Queue
    );

FORCEINLINE
VOID
TxQueueEnqueue(
    _In_ TX_QUEUE    *Queue,
    _In_ VOID        *TxBuffer
    );

FORCEINLINE
VOID
TxQueueDequeue(
    _In_ TX_QUEUE     *Queue,
    _Out_ VOID        **TxBuffer
    );


