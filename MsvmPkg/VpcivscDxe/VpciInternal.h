/*++

Copyright (c) 2008 Microsoft Corporation. All Rights Reserved.

Module Name:

    vpciinternal.h

Abstract:

    This file defines structures and interfaces that are internal to the
    Virtual PCI implementation

Author:

    Ishai Ben Aroya (ishaib) 12-May-2008

--*/
#pragma once

#include <EfiNt.h>
#include "ntconfig.h"

#define ALIGN_UP(x, y) ALIGN_VALUE((x), sizeof(y))

// #include <vpcidefs.h>
// Taken from vpcidefs.w
typedef struct _VPCI_PNP_ID
{
    USHORT  VendorID;
    USHORT  DeviceID;
    UCHAR   RevisionID;
    UCHAR   ProgIf;
    UCHAR   SubClass;
    UCHAR   BaseClass;
    USHORT  SubVendorID;
    USHORT  SubSystemID;

} VPCI_PNP_ID, *PVPCI_PNP_ID;

#define PCI_TYPE0_BAR_COUNT     6

#define VPCI_DEFS_DEFINED 1
// end of vpcidefs.w

// Taken from ntpoapi.w
typedef enum _DEVICE_POWER_STATE {
    PowerDeviceUnspecified = 0,
    PowerDeviceD0,
    PowerDeviceD1,
    PowerDeviceD2,
    PowerDeviceD3,
    PowerDeviceMaximum
} DEVICE_POWER_STATE, *PDEVICE_POWER_STATE;
// end of ntpoapi.w

// #include <pcivirt.h>

// allow nameless unions
#pragma warning(push)
#pragma warning(disable : 4201)


// {AF311B6D-E5C1-49b8-83B6-FAAC9FC0C346}
DEFINE_GUID(GUID_DEVINTERFACE_VPCI_VSP,
0xaf311b6d, 0xe5c1, 0x49b8, 0x83, 0xb6, 0xfa, 0xac, 0x9f, 0xc0, 0xc3, 0x46);

// {44C4F61D-4444-4400-9D52-802E27EDE19F}
DEFINE_GUID(GUID_VPCI_VSP_CHANNEL_TYPE,
0x44c4f61d, 0x4444, 0x4400, 0x9d, 0x52, 0x80, 0x2e, 0x27, 0xed, 0xe1, 0x9f);

// {331ca44a-f8f7-4eb6-b4fd-e35eb7aaf49a}
DEFINE_GUID(GUID_VPCI_VSP_TEST_CHANNEL_TYPE,
0x331ca44a, 0xf8f7, 0x4eb6, 0xb4, 0xfd, 0xe3, 0x5e, 0xb7, 0xaa, 0xf4, 0x9a);

// {c6d175ed-567c-4ba4-98f2-33b43cc9baf6}
DEFINE_GUID(GUID_VPCI_DEVICE_SAVE_TYPE_ID,
0xc6d175ed, 0x567c, 0x4ba4, 0x98, 0xf2, 0x33, 0xb4, 0x3c, 0xc9, 0xba, 0xf6);

// {44ca1bed-b121-4c9f-b6f8-fea8746c8b2d}
DEFINE_GUID(GUID_VPCI_BUS_SAVE_TYPE_ID,
0x44ca1bed, 0xb121, 0x4c9f, 0xb6, 0xf8, 0xfe, 0xa8, 0x74, 0x6c, 0x8b, 0x2d);

#ifndef PCI_TYPE0_ADDRESSES
#define PCI_TYPE0_ADDRESSES 6
#endif

//
// Version history:  <major><minor>, 1 WORD each
// V1 (Win2012 Server)
//   M3                      0x00010000
//
// V2 (Win10 Server)         0x00010001
//      Indicates that the guest need not unmap the MMIO regions on an S5
//      transition (which is necessary for a GPU.)
//
#define VPCI_PROTOCOL_VERSION_WIN8            0x00010000
#define VPCI_PROTOCOL_VERSION_WIN10           0x00010001
#define VPCI_PROTOCOL_VERSION_RS1             0x00010002
#define VPCI_PROTOCOL_VERSION_CURRENT         VPCI_PROTOCOL_VERSION_RS1

static const ULONG VscSupportedVersions[] =
{
    VPCI_PROTOCOL_VERSION_RS1,
    VPCI_PROTOCOL_VERSION_WIN10,
    VPCI_PROTOCOL_VERSION_WIN8
};

#define VSC_NUMBER_SUPPORTED_VERSIONS \
        (sizeof(VscSupportedVersions) / sizeof(VscSupportedVersions[0]))

#define VPCI_DEVICE_LOGICAL_ID(__GUID__, __function__)      \
    ((__GUID__.Data2 << 16) | (__GUID__.Data3 & 0xFFF8) | (__function__))


//
// Messages between the Virtual PCI driver and its VSP
//

typedef enum _VPCI_MESSAGE
{
    VpciMsgBusRelations = 0x42490000,
    VpciMsgQueryBusRelations,
    VpciMsgInvalidateDevice,
    VpciMsgInvalidateBus,
    VpciMsgDevicePowerStateChange,
    VpciMsgCurrentResourceRequirements,
    VpciMsgGetResources,
    VpciMsgFdoD0Entry,
    VpciMsgFdoD0Exit,
    VpciMsgReadBlock,
    VpciMsgWriteBlock,
    VpciMsgEject,
    VpciMsgQueryStop,
    VpciMsgReEnable,
    VpciMsgQueryStopFailed,
    VpciMsgEjectComplete,
    VpciMsgAssignedResources,
    VpciMsgReleaseResources,
    VpciMsgInvalidateBlock,
    VpciMsgQueryProtocolVersion,
    VpciMsgCreateInterruptMessage,
    VpciMsgDeleteInterruptMessage,
    VpciMsgAssignedResources2,
    VpciMsgCreateInterruptMessage2,
    VpciMsgDeleteInterruptMessage2
} VPCI_MESSAGE, *PVPCI_MESSAGE;

// begin_wpp config
// CUSTOM_TYPE(VPCIMSG, ItemEnum(_VPCI_MESSAGE));
// end_wpp


typedef struct _VPCI_PACKET_HEADER
{
    UINT32      MessageType;
} VPCI_PACKET_HEADER, *PVPCI_PACKET_HEADER;


typedef struct _VPCI_REPLY_HEADER
{
    UINT32      Status;
} VPCI_REPLY_HEADER, *PVPCI_REPLY_HEADER;


//
// IOCTL codes for the PCI proxy driver.
//
#define VPCI_PROXY_IOCTL(_index_) \
    CTL_CODE (FILE_DEVICE_UNKNOWN, _index_, METHOD_BUFFERED, FILE_READ_DATA)


//
// This IOCTL is used by the proxy driver in order to supply the local unique
// ID of the discrete PCI device.
//
#define IOCTL_VPCI_PROXY_QUERY_LUID VPCI_PROXY_IOCTL(0x1)

typedef struct _VPCI_DEVICE_DESCRIPTION
{
    VPCI_PNP_ID     IDs;
    ULONG           Slot;
    UINT32          SerialNumber;

} VPCI_DEVICE_DESCRIPTION, *PVPCI_DEVICE_DESCRIPTION;

typedef struct _VPCI_QUERY_BUS_RELATIONS
{
    VPCI_PACKET_HEADER      Header;
    ULONG                   DeviceCount;

    VPCI_DEVICE_DESCRIPTION Devices[1];

} VPCI_QUERY_BUS_RELATIONS, *PVPCI_QUERY_BUS_RELATIONS;

#define VPCI_MAX_DEVICES_PER_BUS 255
//
// Header (or complete message, depending on message type) for
// messages directed to the PDO (e.g. query-stop, eject, etc.)
//
typedef struct _VPCI_PDO_MESSAGE
{
    VPCI_PACKET_HEADER      Header;

    ULONG                   Slot;
} VPCI_PDO_MESSAGE, *PVPCI_PDO_MESSAGE;

typedef struct _VPCI_EJECT_COMPLETE
#ifdef __cplusplus
    : VPCI_PDO_MESSAGE
#endif
{
#ifndef __cplusplus
    VPCI_PDO_MESSAGE;
#endif

    NTSTATUS                Status;
} VPCI_EJECT_COMPLETE, *PVPCI_EJECT_COMPLETE;

typedef struct _VPCI_INVALIDATE_BLOCK
#ifdef __cplusplus
    : VPCI_PDO_MESSAGE
#endif
{
#ifndef __cplusplus
    VPCI_PDO_MESSAGE;
#endif

    UINT64  BlockMask;
} VPCI_INVALIDATE_BLOCK, *PVPCI_INVALIDATE_BLOCK;

#if !defined(_NTDDK_) && !defined(_PCI_X_) && !defined(_NTOSP_)
typedef struct _PCI_SLOT_NUMBER
{
    union
    {
        struct
        {
            ULONG   DeviceNumber:5;
            ULONG   FunctionNumber:3;
            ULONG   Reserved:24;
        } bits;
        ULONG   AsULONG;
    } u;
} PCI_SLOT_NUMBER, *PPCI_SLOT_NUMBER;
#endif

// maximum number to keep VPCI_DEVICE_TRANSLATE under 2 pages in V1
// V2 structure is larger, and 500 resources takes a bit over 8 pages.
// we size the ring buffer accordingly. (currently setting it to 16 pages)
#define MAX_SUPPORTED_RESOURCE_CLAIMS   500
#define MAX_SUPPORTED_INTERRUPT_MESSAGES (MAX_SUPPORTED_RESOURCE_CLAIMS - PCI_TYPE0_ADDRESSES)

typedef struct _VPCI_RESOURCES
{
    VPCI_PACKET_HEADER          Header;
    PCI_SLOT_NUMBER             Slot;
    CM_PARTIAL_RESOURCE_LIST    CmResList;

} VPCI_RESOURCES, *PVPCI_RESOURCES;

typedef struct _VPCI_MESSAGE_RESOURCE
{
    union
    {
        struct
        {
            USHORT    Reserved;
            USHORT    MessageCount;
            ULONG     DataPayload;
            ULONG64   Address;
        } Remapped;

        struct
        {
            UCHAR     Vector;
            UCHAR     DeliveryMode;
            USHORT    VectorCount;
            USHORT    Reserved[2];
            UINT64    ProcessorMask;
        } Descriptor;
    };

} VPCI_MESSAGE_RESOURCE, *PVPCI_MESSAGE_RESOURCE;

#define VPCI_MESSAGE_RESOURCE_2_MAX_CPU_COUNT 32

typedef struct _VPCI_MESSAGE_RESOURCE_2
{
    union
    {
        struct
        {
            USHORT    Reserved;
            USHORT    MessageCount;
            ULONG     DataPayload;
            ULONG64   Address;
            USHORT    Reserved2[27];
        } Remapped;

        struct
        {
            UCHAR     Vector;
            UCHAR     DeliveryMode;
            USHORT    VectorCount;
            USHORT    ProcessorCount;
            USHORT    ProcessorArray[VPCI_MESSAGE_RESOURCE_2_MAX_CPU_COUNT];
        } Descriptor;
    };

} VPCI_MESSAGE_RESOURCE_2, *PVPCI_MESSAGE_RESOURCE_2;


typedef struct _VPCI_QUERY_PROTOCOL_VERSION
{
    VPCI_PACKET_HEADER              Header;
    ULONG                           ProtocolVersion;
} VPCI_QUERY_PROTOCOL_VERSION, *PVPCI_QUERY_PROTOCOL_VERSION;

typedef struct _VPCI_PROTOCOL_VERSION_REPLY
{
    VPCI_REPLY_HEADER               Header;
    ULONG                           ProtocolVersion;
} VPCI_PROTOCOL_VERSION_REPLY, *PVPCI_PROTOCOL_VERSION_REPLY;

typedef struct _VPCI_QUERY_RESOURCE_REQUIREMENTS
{
    VPCI_PACKET_HEADER              Header;
    PCI_SLOT_NUMBER                 Slot;
} VPCI_QUERY_RESOURCE_REQUIREMENTS, *PVPCI_QUERY_RESOURCE_REQUIREMENTS;


typedef struct _VPCI_RESOURCE_REQUIREMENTS_REPLY
{
    VPCI_REPLY_HEADER               Header;
    UINT32                          Bars[PCI_TYPE0_ADDRESSES];
} VPCI_RESOURCE_REQUIREMENTS_REPLY, *PVPCI_RESOURCE_REQUIREMENTS_REPLY;


typedef struct _VPCI_READ_BLOCK_REQUEST
{
    VPCI_PACKET_HEADER              Header;
    ULONG                           BlockId;
    PCI_SLOT_NUMBER                 Slot;
    ULONG                           BytesRequested;
} VPCI_READ_BLOCK_REQUEST, *PVPCI_READ_BLOCK_REQUEST;

typedef struct _VPCI_READ_BLOCK_REPLY
{
    VPCI_REPLY_HEADER               Header;
    UCHAR                           Data[ANYSIZE_ARRAY];
} VPCI_READ_BLOCK_REPLY, *PVPCI_READ_BLOCK_REPLY;

typedef struct _VPCI_WRITE_BLOCK_REQUEST
{
    VPCI_PACKET_HEADER              Header;
    ULONG                           BlockId;
    PCI_SLOT_NUMBER                 Slot;
    ULONG                           DataLength;
    UCHAR                           Data[ANYSIZE_ARRAY];
} VPCI_WRITE_BLOCK_REQUEST, *PVPCI_WRITE_BLOCK_REQUEST;

typedef struct _VPCI_WRITE_BLOCK_REPLY
{
    VPCI_REPLY_HEADER               Header;
} VPCI_WRITE_BLOCK_REPLY, *PVPCI_WRITE_BLOCK_REPLY;


typedef struct _VPCI_DEVICE_POWER_CHANGE
{
    union
    {
        VPCI_PACKET_HEADER         Header;
        VPCI_REPLY_HEADER          ReplyHeader;
    };
    PCI_SLOT_NUMBER                Slot;
    DEVICE_POWER_STATE             TargetState;

} VPCI_DEVICE_POWER_CHANGE, *PVPCI_DEVICE_POWER_CHANGE;

//
// This message indicates which resources the device is "decoding"
// within the child partition at the moment that it is sent.  It is
// valid for the device to be decoding no resources.  Mmio resources
// are configured using Base Address Registers which are limited to 6.
// Unused registers and registers that are used at the high part of
// 64-bit addresses are encoded as CmResourceTypeNull.
//
// The completion packet uses the same structure to return the
// translated MSI resources.
//

typedef struct _VPCI_DEVICE_TRANSLATE
{
    union
    {
        VPCI_PACKET_HEADER         Header;
        VPCI_REPLY_HEADER          ReplyHeader;
    };
    PCI_SLOT_NUMBER                Slot;

    CM_PARTIAL_RESOURCE_DESCRIPTOR MmioResources[PCI_TYPE0_ADDRESSES];

    _Field_range_(0, MAX_SUPPORTED_INTERRUPT_MESSAGES)
    ULONG                          MsiResourceCount;

    _Field_size_full_(MsiResourceCount)
    VPCI_MESSAGE_RESOURCE          MsiResources[1];

} VPCI_DEVICE_TRANSLATE, *PVPCI_DEVICE_TRANSLATE;

typedef struct _VPCI_DEVICE_TRANSLATE_2
{
    union
    {
        VPCI_PACKET_HEADER         Header;
        VPCI_REPLY_HEADER          ReplyHeader;
    };
    PCI_SLOT_NUMBER                Slot;

    CM_PARTIAL_RESOURCE_DESCRIPTOR MmioResources[PCI_TYPE0_ADDRESSES];

    _Field_range_(0, MAX_SUPPORTED_INTERRUPT_MESSAGES)
    ULONG                          MsiResourceCount;

    _Field_size_full_(MsiResourceCount)
    VPCI_MESSAGE_RESOURCE_2          MsiResources[1];

} VPCI_DEVICE_TRANSLATE_2, *PVPCI_DEVICE_TRANSLATE_2;

// NOTE: This also doesn't exist in the windows header. Normally we'd use the
//       the same packet as above for the response as it gives us the remapped
//       MSI interrupts, but in UEFI we don't care about interrupts. Thus we
//       only care about the status, so this is a nice partial packet for that.
typedef struct _VPCI_DEVICE_TRANSLATE_2_REPLY
{
    VPCI_REPLY_HEADER Header;
    PCI_SLOT_NUMBER Slot;
}  VPCI_DEVICE_TRANSLATE_2_REPLY, *PVPCI_DEVICE_TRANSLATE_2_REPLY;

typedef struct _VPCI_DEVICE_INVALIDATE
{
    VPCI_PACKET_HEADER      Header;
    PCI_SLOT_NUMBER         Slot;

} VPCI_DEVICE_INVALIDATE, *PVPCI_DEVICE_INVALIDATE;


typedef struct _VPCI_FDO_D0_ENTRY
{
    VPCI_PACKET_HEADER      Header;
    ULONG                   Padding;
    ULONG64                 MmioStart;
} VPCI_FDO_D0_ENTRY, *PVPCI_FDO_D0_ENTRY;

// NOTE: This doesn't exist in the corresponding windows header. But it's nicer
// to have this way, as this is what the response is.
typedef struct _VPCI_FDO_D0_ENTRY_REPLY
{
    UINT32 NtStatus;
    UINT32 Pad;
} VPCI_FDO_D0_ENTRY_REPLY, *PVPCI_FDO_D0_ENTRY_REPLY;

typedef struct _VPCI_FDO_D0_EXIT
{
    VPCI_PACKET_HEADER      Header;
} VPCI_FDO_D0_EXIT, *PVPCI_FDO_D0_EXIT;


typedef struct _ROOT_PORT_ADDRESS
{
    USHORT  Segment;
    UCHAR   Bus;
    UCHAR   Device;
    UCHAR   Function;
} ROOT_PORT_ADDRESS, *PROOT_PORT_ADDRESS;

typedef struct _VPCI_CREATE_INTERRUPT_MESSAGE
{
    VPCI_PACKET_HEADER      Header;
    PCI_SLOT_NUMBER         Slot;

    VPCI_MESSAGE_RESOURCE   InterruptMessage;

} VPCI_CREATE_INTERRUPT_MESSAGE, *PVPCI_CREATE_INTERRUPT_MESSAGE;

typedef struct _VPCI_CREATE_INTERRUPT_MESSAGE_2
{
    VPCI_PACKET_HEADER      Header;
    PCI_SLOT_NUMBER         Slot;

    VPCI_MESSAGE_RESOURCE_2 InterruptMessage;

} VPCI_CREATE_INTERRUPT_MESSAGE_2, *PVPCI_CREATE_INTERRUPT_MESSAGE_2;

typedef struct _VPCI_CREATE_INTERRUPT_REPLY
{
    VPCI_REPLY_HEADER       ReplyHeader;
    ULONG                   Reserved;
    VPCI_MESSAGE_RESOURCE   TranslatedMessage;

} VPCI_CREATE_INTERRUPT_REPLY, *PVPCI_CREATE_INTERRUPT_REPLY;

typedef struct _VPCI_CREATE_INTERRUPT_REPLY_2
{
    VPCI_REPLY_HEADER       ReplyHeader;
    ULONG                   Reserved;
    VPCI_MESSAGE_RESOURCE_2 TranslatedMessage;

} VPCI_CREATE_INTERRUPT_REPLY_2, *PVPCI_CREATE_INTERRUPT_REPLY_2;

typedef struct _VPCI_DELETE_INTERRUPT_MESSAGE
{
    VPCI_PACKET_HEADER      Header;
    PCI_SLOT_NUMBER         Slot;
    VPCI_MESSAGE_RESOURCE   InterruptMessage;

} VPCI_DELETE_INTERRUPT_MESSAGE, *PVPCI_DELETE_INTERRUPT_MESSAGE;

typedef struct _VPCI_DELETE_INTERRUPT_MESSAGE_2
{
    VPCI_PACKET_HEADER      Header;
    PCI_SLOT_NUMBER         Slot;
    VPCI_MESSAGE_RESOURCE_2 InterruptMessage;

} VPCI_DELETE_INTERRUPT_MESSAGE_2, *PVPCI_DELETE_INTERRUPT_MESSAGE_2;

#define MAX_VIRT_PCI_BUS_PACKET_SIZE \
ALIGN_UP((sizeof(VPCI_DEVICE_TRANSLATE_2) + (sizeof(VPCI_MESSAGE_RESOURCE_2) * (MAX_SUPPORTED_INTERRUPT_MESSAGES - 1))), UINT64)

#pragma warning(pop)
