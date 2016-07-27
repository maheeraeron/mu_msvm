#include "UefiHavoc.h"

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

#include <NetvscDxe/NvspProtocol.h>
#include <NetvscDxe/vmrndis.h>

#define PERM_NODE_ADDR_REQUEST_ID    0xFAAD

UINT32 mNvspMessageSize = sizeof(NVSP_MESSAGE);
NVSP_MESSAGE mNvspMessage;

UINT32 mNicMsgSize = (RNDIS_MESSAGE_SIZE(RNDIS_QUERY_REQUEST));
UINT8  mNicMsgBuffer[(RNDIS_MESSAGE_SIZE(RNDIS_QUERY_REQUEST))];

#define NUM_ENTRIES (1024*1024)

struct
{
    RNDIS_MESSAGE RndisMessage;
    UINT8  Buffer[6*NUM_ENTRIES];
} mMulticastBuffer;

UINT32  mMulticastBufferSize = sizeof(mMulticastBuffer);

EFI_EXTERNAL_BUFFER mNicBuffer;

EFI_STATUS
EFIAPI
NicInit(
    )
{
    PRNDIS_MESSAGE pRndisMessage = (PRNDIS_MESSAGE)mNicMsgBuffer;
    PRNDIS_QUERY_REQUEST pQueryRequest;
    UINT32 rndisMsgSize;

    Print(L"NicInit...\n");

    rndisMsgSize = RNDIS_MESSAGE_SIZE(RNDIS_QUERY_REQUEST);

    pQueryRequest = &pRndisMessage->Message.QueryRequest;
    pQueryRequest->RequestId = PERM_NODE_ADDR_REQUEST_ID;
    pQueryRequest->Oid = RNDIS_OID_802_3_CURRENT_ADDRESS;
    pQueryRequest->InformationBufferLength = 0;
    pQueryRequest->InformationBufferOffset = sizeof(RNDIS_QUERY_REQUEST);
    pQueryRequest->DeviceVcHandle = 0;

    pRndisMessage->NdisMessageType = REMOTE_NDIS_QUERY_MSG;
    pRndisMessage->MessageLength = rndisMsgSize;

    mNvspMessage.Header.MessageType = NvspMessage1TypeSendRNDISPacket;
    mNvspMessage.Messages.Version1Messages.SendRNDISPacket.ChannelType = 1;
    mNvspMessage.Messages.Version1Messages.SendRNDISPacket.SendBufferSectionIndex = 0xFFFFFFFF;
    mNvspMessage.Messages.Version1Messages.SendRNDISPacket.SendBufferSectionSize = rndisMsgSize;

    mNicBuffer.Buffer = NULL;
    mNicBuffer.BufferSize = 0;

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
NicInitLargeMulticast(
    )
{
    PRNDIS_MESSAGE pRndisMessage = &mMulticastBuffer.RndisMessage;
    PRNDIS_SET_REQUEST   pSetRequest;
    UINT32 rndisMsgSize;
    PUINT8 Data;
    PUINT32 pLast4;
    UINT32 next;

    Print(L"NicInit...\n");

    rndisMsgSize = RNDIS_MESSAGE_SIZE(RNDIS_SET_REQUEST) + sizeof(mMulticastBuffer.Buffer);

    pSetRequest = &pRndisMessage->Message.SetRequest;
    pSetRequest->RequestId = PERM_NODE_ADDR_REQUEST_ID;
    pSetRequest->Oid = RNDIS_OID_802_3_MULTICAST_LIST;
    pSetRequest->InformationBufferLength = sizeof(mMulticastBuffer.Buffer);
    pSetRequest->InformationBufferOffset = sizeof(RNDIS_SET_REQUEST);
    pSetRequest->DeviceVcHandle = 0;

    pRndisMessage->NdisMessageType = REMOTE_NDIS_SET_MSG;
    pRndisMessage->MessageLength = rndisMsgSize;

    mNvspMessage.Header.MessageType = NvspMessage1TypeSendRNDISPacket;
    mNvspMessage.Messages.Version1Messages.SendRNDISPacket.ChannelType = 1;
    mNvspMessage.Messages.Version1Messages.SendRNDISPacket.SendBufferSectionIndex = 0xFFFFFFFF;
    mNvspMessage.Messages.Version1Messages.SendRNDISPacket.SendBufferSectionSize = rndisMsgSize;

    mNicBuffer.Buffer = pRndisMessage;
    mNicBuffer.BufferSize = rndisMsgSize;

    Data = ((PUINT8)(pSetRequest) + pSetRequest->InformationBufferOffset);
    pLast4 = (PUINT32)&Data[2];

    Data[0] = 0x90;
    Data[1] = 0x60;
    Data[2] = 0x4B;
    Data[3] = 0x6D;
    Data[4] = 0x88;
    Data[5] = 0xD0;

    for (next = 1; next < NUM_ENTRIES; next++)
    {
        *(pLast4) = *(pLast4)+1;
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
NicHavoc(
    _In_  HAVOC_PLUGIN   *Plugin
    )
{
    EFI_STATUS status;

    if (mNicBuffer.BufferSize != mNicMsgSize)
    {
        Print(L"BufferSize = %d\n", mNicMsgSize);
        mNicBuffer.BufferSize = mNicMsgSize;
    }

    if (mNicBuffer.Buffer != mNicMsgBuffer)
    {
        Print(L"Buffer = %p\n", mNicMsgBuffer);
        mNicBuffer.Buffer = mNicMsgBuffer;
    }

    status =
        Plugin->DeviceInfo.EmclInstance->SendPacket(Plugin->DeviceInfo.EmclInstance,
                        &mNvspMessage,
                        mNvspMessageSize,
                        &mNicBuffer,
                        1,
                        NULL,
                        NULL);

    return status;
}
