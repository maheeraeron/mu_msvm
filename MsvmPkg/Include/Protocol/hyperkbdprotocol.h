/*++

Copyright (c) Microsoft Corporation

Module Name:

    File: hyperkbdprotocol.h

Abstract:

    Definitions of the keyboard message structures used by
    the synthetic keyboard VDEV and its VSC.

Environment:

    Kernel mode

Revision History:

    John Starks (jostarks)
    Bhanu Gogineni (bhanug) 19-Jul-2011

--*/

#pragma once

// f912ad6d-2b17-48ea-bd65-f927a61c7684
DEFINE_GUID(HK_INTERFACE_GUID, 0xf912ad6d, 0x2b17, 0x48ea, 0xbd, 0x65, 0xf9, 0x27, 0xa6, 0x1c, 0x76, 0x84);

// d34b2567-b9b6-42b9-8778-0a4ec0b955bf
DEFINE_GUID(HK_INSTANCE_GUID, 0xd34b2567, 0xb9b6, 0x42b9, 0x87, 0x78, 0x0a, 0x4e, 0xc0, 0xb9, 0x55, 0xbf);

#define HK_MAKE_VERSION(Major, Minor) ((UINT32)(Major) << 16 | (UINT32)(Minor))
#define HK_VERSION_WIN8 HK_MAKE_VERSION(1, 0)

typedef enum _HK_MESSAGE_TYPE
{
    HkMessageProtocolRequest  = 1,
    HkMessageProtocolResponse = 2,
    HkMessageEvent            = 3,
    HkMessageSetLedIndicators = 4,
} HK_MESSAGE_TYPE;

typedef struct _HK_MESSAGE_HEADER
{
    HK_MESSAGE_TYPE MessageType;
} HK_MESSAGE_HEADER, *PHK_MESSAGE_HEADER;

typedef struct _HK_MESSAGE_PROTOCOL_REQUEST
{
    HK_MESSAGE_HEADER Header;
    UINT32 Version;
} HK_MESSAGE_PROTOCOL_REQUEST, *PHK_MESSAGE_PROTOCOL_REQUEST;

typedef struct _HK_MESSAGE_LED_INDICATORS_STATE
{
    HK_MESSAGE_HEADER Header;
    USHORT LedFlags;
} HK_MESSAGE_LED_INDICATORS_STATE, *PHK_MESSAGE_LED_INDICATORS_STATE;

typedef struct _HK_MESSAGE_PROTOCOL_RESPONSE
{
    HK_MESSAGE_HEADER Header;
    UINT32 Accepted:1;
    UINT32 Reserved:31;
} HK_MESSAGE_PROTOCOL_RESPONSE, *PHK_MESSAGE_PROTOCOL_RESPONSE;

typedef struct _HK_MESSAGE_KEYSTROKE
{
    HK_MESSAGE_HEADER Header;
    UINT16 MakeCode;
    UINT32 IsUnicode:1;
    UINT32 IsBreak:1;
    UINT32 IsE0:1;
    UINT32 IsE1:1;
    UINT32 Reserved:28;
} HK_MESSAGE_KEYSTROKE, *PHK_MESSAGE_KEYSTROKE;

#define HK_MAXIMUM_MESSAGE_SIZE 256

