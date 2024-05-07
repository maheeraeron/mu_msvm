/** @file
  This file contains the definitions for the Hyper-V synthetic video protocol.

  Copyright (c) Microsoft Corporation.
  Licensed under the BSD-2-Clause-Patent license.
**/

#pragma once

//
// Maximum packet payload to VMBus is currently 16k.
//
#define MAX_VMBUS_PACKET_SIZE 0x4000

//
// The maximum amount of data we'll send for a cursor in one packet is 8k.
//
#define CURSOR_MAX_PAYLOAD_SIZE (MAX_VMBUS_PACKET_SIZE / 2)

//
// Maximum supported cursor is 96 pixels x 96 pixels in ARGB 32-bit format.
//
#define CURSOR_MAX_X 96
#define CURSOR_MAX_Y 96
#define CURSOR_ARGB_PIXEL_SIZE (4 * sizeof(BYTE))
#define CURSOR_MAX_SIZE (CURSOR_MAX_X * CURSOR_MAX_Y * CURSOR_ARGB_PIXEL_SIZE)


//
// Maximum supported number of dirty regions in a single dirt message
//
#define MAX_DIRTY_REGIONS 255

//
// Largest message possible in each direction.
//
#define MAX_VSC_TO_VSP_MESSAGE_SIZE (FIELD_OFFSET(SYNTHVID_POINTER_SHAPE_MESSAGE, PixelData) + \
                                     CURSOR_MAX_PAYLOAD_SIZE)

#define MAX_VSP_TO_VSC_MESSAGE_SIZE (sizeof(SYNTHVID_BIOS_INFO_RESPONSE_MESSAGE))

//
// Emergency reset notification I/O Port
//
#define EMERGENCY_RESET_IO_PORT 0x100

//
// Latest version of the SynthVid protocol.
//
// History:
// Beta, RC < 2008/1/22 1
// RC > 2008/1/22       2
// RTM > 2008/2/25      3.0
//
#define SYNTHVID_VERSION_MAJOR 3
#define SYNTHVID_VERSION_MINOR 5

#define SYNTHVID_VERSION_CURRENT ((SYNTHVID_VERSION_MINOR << 16) | (SYNTHVID_VERSION_MAJOR))
#define SYNTHVID_FEATURE_LEVEL(VersionMajor, VersionMinor) \
               ((ULONG)(((VersionMajor) << 16) | (VersionMinor&SynthVidFeatureMinorMask)))
#define TRUE_WITH_VERSION_EXCHANGE (TRUE + 1)

#pragma once
#pragma pack(push,1)

//
// Mask to be applied to the minor version to determine feature support.
//
#define SynthVidFeatureMinorMask 0xff

//
// SynthVid features by version.
//
typedef enum
{

    //
    // Win7 RTM.
    //
    SynthVidFeatureWin7Rtm = SYNTHVID_FEATURE_LEVEL(3, 0),
    SynthVidFeatureBasic = SynthVidFeatureWin7Rtm,

    //
    // Win8 RTM.
    //
    SynthVidFeatureWin8Rtm = SYNTHVID_FEATURE_LEVEL(3, 2),

    //
    // Support for resolutions above 1600W or 1200H.
    //
    SynthVidFeatureHighResolutions = SynthVidFeatureWin8Rtm,

    //
    // Support for 32bpp color depth.
    //
    SynthVidFeatureSupports32bpp = SynthVidFeatureWin8Rtm,

    //
    // Support for protocol version reinitialization.
    //
    SynthVidFeatureSupportsReinit = SynthVidFeatureWin8Rtm,

    //
    // Win BLUE
    //
    SynthVidFeatureWinBlue = SYNTHVID_FEATURE_LEVEL(3, 3),
    SynthVidFeatureQueryBiosInfo = SynthVidFeatureWinBlue,

} SYNTHVID_FEATURE;


//
// SynthVid Message Types
//
typedef enum
{
    SynthvidError                  = 0,
    SynthvidVersionRequest         = 1,
    SynthvidVersionResponse        = 2,
    SynthvidVramLocation           = 3,
    SynthvidVramLocationAck        = 4,
    SynthvidSituationUpdate        = 5,
    SynthvidSituationUpdateAck     = 6,
    SynthvidPointerPosition        = 7,
    SynthvidPointerShape           = 8,
    SynthvidFeatureChange          = 9,
    SynthvidDirt                   = 10,
    SynthvidBiosInfoRequest        = 11,
    SynthvidBiosInfoResponse       = 12,

    SynthvidMax                    = 13
} SYNTHVID_MESSAGE_TYPE;

//
// Basic message structures.
//
typedef struct
{
    SYNTHVID_MESSAGE_TYPE   Type;    // Type of the enclosed message
    UINT32                  Size;    // Size of the enclosed message (size of the data payload)
} SYNTHVID_MESSAGE_HEADER, *PSYNTHVID_MESSAGE_HEADER;

typedef struct
{
    SYNTHVID_MESSAGE_HEADER Header;
    BYTE                    Data[1]; // Enclosed message
} SYNTHVID_MESSAGE, *PSYNTHVID_MESSAGE;


#pragma warning(push)
#pragma warning(disable : 4201)
typedef union
{
    struct
    {
        UINT16 MajorVersion;
        UINT16 MinorVersion;
    };

    UINT32 AsDWORD;
} SYNTHVID_VERSION, *PSYNTHVID_VERSION;
#pragma warning(pop)

//
// The following messages are listed in order of occurance during startup
// and handshaking.
//

// VSC to VSP /////////////////////////////////////////////////////
typedef struct
{
    SYNTHVID_MESSAGE_HEADER Header;
    SYNTHVID_VERSION        Version;
} SYNTHVID_VERSION_REQUEST_MESSAGE, *PSYNTHVID_VERSION_REQUEST_MESSAGE;

// VSP to VSC /////////////////////////////////////////////////////
typedef struct
{
    SYNTHVID_MESSAGE_HEADER Header;
    SYNTHVID_VERSION        Version;
    BOOLEAN IsAccepted;
    UINT8                   MaxVideoOutputs; // 1 in Veridian 1.0
} SYNTHVID_VERSION_RESPONSE_MESSAGE, *PSYNTHVID_VERSION_RESPONSE_MESSAGE;

// VSC to VSP /////////////////////////////////////////////////////
typedef struct
{
    SYNTHVID_MESSAGE_HEADER Header;
    UINT64                  UserContext;
    BOOLEAN                 IsVramGpaAddressSpecified;
    UINT64                  VramGpaAddress;
} SYNTHVID_VRAM_LOCATION_MESSAGE, *PSYNTHVID_VRAM_LOCATION_MESSAGE;


// VSP to VSC ////////////////////////////////////////////////////
// This is called "acknowledge", but in addition, it indicates to the VSC
// that the new physical address location is backed with a memory block
// that the guest can safely write to, knowing that the writes will actually
// be reflected in the VRAM memory block.
typedef struct
{
    SYNTHVID_MESSAGE_HEADER Header;
    UINT64                  UserContext;
} SYNTHVID_VRAM_LOCATION_ACK_MESSAGE, *PSYNTHVID_VRAM_LOCATION_ACK_MESSAGE;




//
// These messages are used to communicate "situation updates" or changes
// in the layout of the primary surface.
//
typedef struct
{
    BOOLEAN Active;
    UINT32  PrimarySurfaceVramOffset;
    UINT8   DepthBits;
    UINT32  WidthPixels;
    UINT32  HeightPixels;
    UINT32  PitchBytes;
} VIDEO_OUTPUT_SITUATION, *PVIDEO_OUTPUT_SITUATION;

// VSC to VSP ////////////////////////////////////////////////////
typedef struct
{
    SYNTHVID_MESSAGE_HEADER Header;
    UINT64                  UserContext;
    UINT8                   VideoOutputCount; // 1 in Veridian 1.0
    VIDEO_OUTPUT_SITUATION  VideoOutput[1];
} SYNTHVID_SITUATION_UPDATE_MESSAGE, *PSYNTHVID_SITUATION_UPDATE_MESSAGE;

// VSP to VSC ////////////////////////////////////////////////////
typedef struct
{
    SYNTHVID_MESSAGE_HEADER Header;
    UINT64                  UserContext;
} SYNTHVID_SITUATION_UPDATE_ACK_MESSAGE, *PSYNTHVID_SITUATION_UPDATE_ACK_MESSAGE;

//
// These messages are used to communicate the BIOS Information of the VM.
//
// VSC to VSP /////////////////////////////////////////////////////
typedef struct
{
    SYNTHVID_MESSAGE_HEADER Header;
} SYNTHVID_BIOS_INFO_REQUEST_MESSAGE, *PSYNTHVID_BIOS_INFO_REQUEST_MESSAGE;

// VSP to VSC /////////////////////////////////////////////////////
typedef struct
{
    SYNTHVID_MESSAGE_HEADER Header;
    UINT32                  VmGeneration;
    UINT32                  ReservedInt32;
    UINT64                  ReservedInt64;
} SYNTHVID_BIOS_INFO_RESPONSE_MESSAGE, *PSYNTHVID_BIOS_INFO_RESPONSE_MESSAGE;


//
// These messages are used to communicate changes in the pointer position or
// shape.
//
// VSC to VSP ////////////////////////////////////////////////////
// This message is ignored unless we're in relative mouse mode.
typedef struct
{
    SYNTHVID_MESSAGE_HEADER Header;

    // LDDM may specify FALSE here, XDDM generally will probably always specify TRUE.
    BOOLEAN                 IsVisible;

    // 0 is the only valid value for 2D Video VSP 1.0
    UINT8                   VideoOutput;

    // Coordinates of upper-left pixel of pointer image.
    INT32                   ImageX;
    INT32                   ImageY;

} SYNTHVID_POINTER_POSITION_MESSAGE, *PSYNTHVID_POINTER_POSITION_MESSAGE;


// VSC to VSP ////////////////////////////////////////////////////
typedef struct
{
    SYNTHVID_MESSAGE_HEADER Header;

    //
    // When a cursor is larger than the maximum VMBus payload size,
    // it is split up.  This 0-based index indicates which portion
    // of the cursor payload is in this message.  -1 means final
    // portion.  If the cursor is not split, this field contains
    // -1 as the completion sentinel value.
    //
    UINT8   PartialIndex;

    //
    // FALSE means and/xor masks (2 bits per pixel), TRUE means
    // ARGB (32 bits per pixel)
    //
    BOOLEAN IsArgb;

    //
    // Max legal value is CURSOR_MAX_X
    //
    UINT32  WidthPixels;

    //
    // Max legal value is CURSOR_MAX_Y
    //
    UINT32  HeightPixels;

    //
    // Stride is implicit based on smallest possible value given width
    // in pixels and format.
    //

    //
    // Pointer hotspot relative to upper-left of pointer image
    //
    UINT32  HotspotX;
    UINT32  HotspotY;

    //
    // Max length of pixel data is 36k based on IsArgb == TRUE,
    //  WidthPixels == 96, HeightPixels == 96.
    // However, we'll send a maximum of 8k at a time.
    //
    // Pointer data length can be calculated as follows:
    // if (IsArgb)
    // {
    //     strideBytes  = WidthPixels * 4;
    //     maskCount    = 1;
    // }
    // else
    // {
    //     strideBytes  = (WidthPixels + 7) / 8;
    //     maskCount    = 2;
    // }
    // pointerDataLength = strideBytes * HeightPixels * maskCount;
    //
    BYTE    PixelData[1];

} SYNTHVID_POINTER_SHAPE_MESSAGE, *PSYNTHVID_POINTER_SHAPE_MESSAGE;

#define CURSOR_COMPLETE ((UINT8)-1)


//
// This message is used to squelch portions of the synthvid protocol
//
// Can be sent from VSP to VSC at any time after handshaking is complete.
// VSC responsible for bringing VSP up-to-date with at least one message
// of the relevant type if one of these goes from FALSE to TRUE.
//

// VSP to VSC ////////////////////////////////////////////////////
typedef struct
{
    SYNTHVID_MESSAGE_HEADER Header;
    BOOLEAN                 IsDirtNeeded;
    BOOLEAN                 IsPointerPositionUpdatesNeeded;
    BOOLEAN                 IsPointerShapeUpdatesNeeded;
    BOOLEAN                 IsVideoSituationUpdatesNeeded;
} SYNTHVID_FEATURE_CHANGE_MESSAGE, *PSYNTHVID_FEATURE_CHANGE_MESSAGE;




//
// This message is used to communicate dirty regions to the VSP.
//

// VSC to VSP /////////////////////////////////////////////////////
typedef struct
{
    SYNTHVID_MESSAGE_HEADER Header;

    // 0 is the only valid value for 2D Video VSP 1.0
    UINT8                   VideoOutput;
    UINT8                   DirtCount;
    RECT                    Dirt[1];
} SYNTHVID_DIRT_MESSAGE, *PSYNTHVID_DIRT_MESSAGE;

#pragma pack(pop)


