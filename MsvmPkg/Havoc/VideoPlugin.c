#include "UefiHavoc.h"

//
// These are needed by SynthVidProtocol.h
//
typedef UINT8   BYTE, *PBYTE;
typedef struct _RECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *PRECT;

#include <VideoDxe/SynthVidProtocol.h>

#define MAX_DIRT_COUNT             255

UINT32 mMsgSize = sizeof(SYNTHVID_DIRT_MESSAGE) + ((MAX_DIRT_COUNT-1) * sizeof(RECT));
UINT8  mMsgBuffer[sizeof(SYNTHVID_DIRT_MESSAGE) + ((MAX_DIRT_COUNT-1) * sizeof(RECT))];

EFI_STATUS
EFIAPI
VideoInit(
    )
{
    PSYNTHVID_DIRT_MESSAGE message = (PSYNTHVID_DIRT_MESSAGE)mMsgBuffer;
    UINT32 i;
    
    message->Header.Type = SynthvidDirt;
    message->Header.Size = mMsgSize;
    message->VideoOutput = 0;
    message->DirtCount   = MAX_DIRT_COUNT;
    
    for (i = 0; i < MAX_DIRT_COUNT; i++)
    {
        message->Dirt[i].top    = 0x0;
        message->Dirt[i].left   = 0x0;
        message->Dirt[i].bottom = 0xffff;
        message->Dirt[i].right  = 0xffff;
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
VideoHavoc(
    _In_  HAVOC_PLUGIN   *Plugin
    )
{
    PSYNTHVID_DIRT_MESSAGE message = (PSYNTHVID_DIRT_MESSAGE)mMsgBuffer;

    Plugin->DeviceInfo.EmclInstance->SendPacket(Plugin->DeviceInfo.EmclInstance,
                        message,
                        mMsgSize,
                        NULL,
                        0,
                        NULL,
                        NULL);

    return EFI_SUCCESS;
}