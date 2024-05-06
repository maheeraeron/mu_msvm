/** @file

    Header file for public definitions shared between kernel and user mode.
    The definitions are used for communication between StorVSP and StorVSC.

    Copyright (c) Microsoft Corporation.
    Licensed under the BSD-2-Clause-Patent license.

**/

#pragma once

#pragma warning(push)
#pragma warning(disable: 4201) // Allow nameless structs
#pragma warning(disable: 4214) // bit field types other than int

//
//  Public interface to the server
//

//
//  StorVSP device interface guid
//
DEFINE_GUID(GUID_STORVSP, \
    0x66cbde6f, 0x1828, 0x47de, 0x8f, 0x3d, 0xe4, 0x15, 0xb3, 0x12, 0x91, 0xb9);

//
//  VMBUS guid for channel and hardware id for the client
//  ba6163d9-04a1-4d29-b605-72e2ffb1dc7f
//
DEFINE_GUID(GUID_STORAGE_CHANNEL_TYPE, \
    0xba6163d9, 0x04a1, 0x4d29, 0xb6, 0x05, 0x72, 0xe2, 0xff, 0xb1, 0xdc, 0x7f);

//
// VMBUS guid for emulated channel and hardware id for the client
// 32412632-86cb-44a2-9b5c-50d1417354f5
//
DEFINE_GUID(GUID_EMULATED_STORAGE_CHANNEL_TYPE, \
    0x32412632, 0x86cb, 0x44a2, 0x9b, 0x5c, 0x50, 0xd1, 0x41, 0x73, 0x54, 0xf5);

//
// VMBUS guid for channel and hardware id for the synthetic fibre channel
// 2f9bcc4a-0069-4af3-b76b-6fd0be528cda
//
DEFINE_GUID(GUID_SYNTHETIC_FIBRE_CHANNEL_TYPE, \
    0x2f9bcc4a, 0x0069, 0x4af3, 0xb7, 0x6b, 0x6f, 0xd0, 0xbe, 0x52, 0x8c, 0xda);

#define VMSTOR_MAXIMUM_SUBCHANNEL_COUNT 15

//
//  Protocol versions.
//

//
// Major/minor macros.  Minor version is in LSB, meaning that earlier flat
// version numbers will be interpreted as "0.x" (i.e., 1 becomes 0.1).
//

#define VMSTOR_PROTOCOL_MAJOR(VERSION_)         (((VERSION_) >> 8) & 0xff)
#define VMSTOR_PROTOCOL_MINOR(VERSION_)         (((VERSION_)     ) & 0xff)
#define VMSTOR_PROTOCOL_VERSION(MAJOR_, MINOR_) ((((MAJOR_) & 0xff) << 8) | \
                                                 (((MINOR_) & 0xff)     ))

//
// Invalid version.
//
#define VMSTOR_INVALID_PROTOCOL_VERSION  -1

//
// Version history:
// V1 (Win2k8 Server)
//   Beta                    0.1
//   RC < 2008/1/31          1.0
//   RC > 2008/1/31          2.0
//   Servicing               3.0 (reserved)
//
// Win7
//   M3                      3.0 (deprecated)
//   Beta                    4.0
//   Release                 4.2
//
// Win8
//   M3                      5.0
//   Beta                    5.1
//   RC                      5.1 (added multi-channel support flag)
//
// Win Blue
//   MQ                      6.0 (added Asynchronous Notification)
//
//
#define VMSTOR_PROTOCOL_VERSION_WIN6            VMSTOR_PROTOCOL_VERSION(2, 0)
#define VMSTOR_PROTOCOL_VERSION_WIN7            VMSTOR_PROTOCOL_VERSION(4, 2)
#define VMSTOR_PROTOCOL_VERSION_WIN8            VMSTOR_PROTOCOL_VERSION(5, 1)
#define VMSTOR_PROTOCOL_VERSION_BLUE            VMSTOR_PROTOCOL_VERSION(6, 0)
#define VMSTOR_PROTOCOL_VERSION_CURRENT         VMSTOR_PROTOCOL_VERSION_BLUE

//
//  The max transfer length will be published when we offer a vmbus channel.
//  Max transfer bytes - this determines the reserved MDL size and how large
//  requests can be that the clients will forward.
//
#define MAX_TRANSFER_LENGTH (8*1024*1024)

//
// Indicates that the device supports Asynchronous Notifications (AN)
//
#define VMSTOR_PROPERTY_AN_CAPABLE 0x1

//
//  Packet structure describing virtual storage requests.
//
typedef enum
{
    VStorOperationCompleteIo            = 1,
    VStorOperationRemoveDevice          = 2,
    VStorOperationExecuteSRB            = 3,
    VStorOperationResetLun              = 4,
    VStorOperationResetAdapter          = 5,
    VStorOperationResetBus              = 6,
    VStorOperationBeginInitialization   = 7,
    VStorOperationEndInitialization     = 8,
    VStorOperationQueryProtocolVersion  = 9,
    VStorOperationQueryProperties       = 10,
    VStorOperationEnumerateBus          = 11,
    VStorOperationFcHbaData             = 12,
    VStorOperationCreateSubChannels     = 13,
    VStorOperationEventNotification     = 14,
    VStorOperationMaximum               = 14,
} VSTOR_PACKET_OPERATION;


//
//  Platform neutral description of a SCSI request
//
#pragma pack(push,1)

#define CDB16GENERIC_LENGTH 0x10

#define MAX_DATA_BUFFER_LENGTH_WITH_PADDING 0x14

#define VMSCSI_SENSE_BUFFER_SIZE_REVISION_1 0x12
#define VMSCSI_SENSE_BUFFER_SIZE 0x14

typedef struct _VMSCSI_REQUEST
{
    USHORT Length;
    UCHAR SrbStatus;
    UCHAR ScsiStatus;

    UCHAR Reserved1;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;

    UCHAR CdbLength;
    UCHAR SenseInfoExLength;
    UCHAR DataIn;
    UCHAR Properties;

    ULONG DataTransferLength;

    union
    {
        UCHAR Cdb[CDB16GENERIC_LENGTH];

        UCHAR SenseDataEx[VMSCSI_SENSE_BUFFER_SIZE];

        UCHAR ReservedArray[MAX_DATA_BUFFER_LENGTH_WITH_PADDING];
    };

    //
    // The following were added in Windows 8
    //
    USHORT  Reserve;
    UCHAR   QueueTag;
    UCHAR   QueueAction;
    ULONG   SrbFlags;
    ULONG   TimeOutValue;
    ULONG   QueueSortKey;
} VMSCSI_REQUEST, *PVMSCSI_REQUEST;

C_ASSERT((sizeof(VMSCSI_REQUEST) % 4) == 0);

#define VMSTORAGE_SIZEOF_VMSCSI_REQUEST_REVISION_1 FIELD_OFFSET(VMSCSI_REQUEST, Reserve)

C_ASSERT(VMSTORAGE_SIZEOF_VMSCSI_REQUEST_REVISION_1 == 0x24);

#define VMSTORAGE_SIZEOF_VMSCSI_REQUEST_REVISION_2 (RTL_SIZEOF_THROUGH_FIELD(VMSCSI_REQUEST, QueueSortKey))

C_ASSERT(VMSTORAGE_SIZEOF_VMSCSI_REQUEST_REVISION_2 == 0x34);


//
// This structure is sent during the intialization phase to get the different
// properties of the channel.
//
// The reserved properties are not guaranteed to be zero before protocol version
// 5.1.
//
typedef struct _VMSTORAGE_CHANNEL_PROPERTIES
{
    ULONG Reserved;
    UINT16 MaximumSubChannelCount;
    UINT16 Reserved2;
    ULONG Flags;
    ULONG MaxTransferBytes;
    ULONGLONG Reserved3;
} VMSTORAGE_CHANNEL_PROPERTIES, *PVMSTORAGE_CHANNEL_PROPERTIES;

//
// Channel Property Flags
//

#define STORAGE_CHANNEL_SUPPORTS_MULTI_CHANNEL          0x1

C_ASSERT((sizeof(VMSTORAGE_CHANNEL_PROPERTIES) % 4) == 0);

//
// This structure is sent as part of the channel offer. It exists for old
// versions of the VSC that used this to determine the IDE channel that
// matched up with the VMBus channel.
//
// The reserved properties are not guaranteed to be zero.
//
typedef struct _VMSTORAGE_OFFER_PROPERTIES
{
    USHORT Reserved;
    UCHAR PathId;
    UCHAR TargetId;
    ULONG Reserved2;
    ULONG Flags;
    ULONG Reserved3[3];
} VMSTORAGE_OFFER_PROPERTIES, *PVMSTORAGE_OFFER_PROPERTIES;

#define STORAGE_OFFER_EMULATED_IDE_FLAG               0x2

//
//  This structure is sent during the storage protocol negotiations.
//
typedef struct _VMSTORAGE_PROTOCOL_VERSION
{
    //
    // Major (MSW) and minor (LSW) version numbers.
    //
    USHORT MajorMinor;

    //
    // Windows build number. Purely informative.
    //
    USHORT Build;

} VMSTORAGE_PROTOCOL_VERSION, *PVMSTORAGE_PROTOCOL_VERSION;

C_ASSERT((sizeof(VMSTORAGE_PROTOCOL_VERSION) % 4) == 0);

//
//  This structure is for fibre channel Wwn Packets.
//
typedef struct _VMFC_WWN_PACKET
{
    BOOLEAN PrimaryWwnActive;
    CHAR    Reserved1;
    USHORT  Reserved2;

    CHAR    PrimaryPortWwn[8];
    CHAR    PrimaryNodeWwn[8];
    CHAR    SecondaryPortWwn[8];
    CHAR    SecondaryNodeWwn[8];
} VMFC_WWN_PACKET, *PVMFC_WWN_PACKET;

C_ASSERT((sizeof(VMFC_WWN_PACKET) % 4) == 0);

//
// Used to register or unregister Asynchronous Media Event Notification to the client
//
typedef struct _VSTOR_CLIENT_PROPERTIES
{
    ULONG AsyncNotifyCapable : 1;
    ULONG Reserved           : 31;

} VSTOR_CLIENT_PROPERTIES, *PVSTOR_CLIENT_PROPERTIES;

C_ASSERT((sizeof(VSTOR_CLIENT_PROPERTIES) % 4) == 0);

typedef struct _VSTOR_ASYNC_REGISTER_PACKET
{
    UCHAR      Lun;
    UCHAR      Target;
    UCHAR      Path;
    BOOLEAN    Register;
} VSTOR_ASYNC_REGISTER_PACKET, *PVSTOR_ASYNC_REGISTER_PACKET;

C_ASSERT((sizeof(VSTOR_ASYNC_REGISTER_PACKET) % 4) == 0);

//
// Used to send notifications to StorVsc about media change events
//
typedef struct _VSTOR_NOTIFICATION_PACKET
{
    UCHAR    Lun;
    UCHAR    Target;
    UCHAR    Path;
    UCHAR    Flags;
} VSTOR_NOTIFICATION_PACKET, *PVSTOR_NOTIFICATION_PACKET;

C_ASSERT((sizeof(VSTOR_NOTIFICATION_PACKET) % 4) == 0);

typedef struct _VSTOR_PACKET
{
    //
    // Requested operation type
    //
    VSTOR_PACKET_OPERATION Operation;

    //
    //  Flags - see below for values
    //
    ULONG     Flags;

    //
    // Status of the request returned from the server side.
    //
    ULONG     Status;

    //
    // Data payload area
    //
    union
    {
        //
        //  Structure used to forward SCSI commands from the client to the server.
        //  0x34 bytes
        VMSCSI_REQUEST      VmSrb;

        //
        // Structure used to query channel properties.
        //
        VMSTORAGE_CHANNEL_PROPERTIES StorageChannelProperties;

        //
        // Used during version negotiations.
        //
        VMSTORAGE_PROTOCOL_VERSION Version;

        //
        // Used for fibre Channel address packet.
        //
        VMFC_WWN_PACKET FcWwnPacket;

        //
        // Number of subchannels to create via VStorOperationCreateSubChannel.
        //
        UINT16 SubChannelCount;

        //
        // Used to perform Asynchronous Event Notifications
        //
        VSTOR_CLIENT_PROPERTIES      ClientProperties;
        VSTOR_NOTIFICATION_PACKET    NotificationPacket;

        //
        // Buffer. The buffer size will be the maximun of union members. It is
        // used to transfer data.
        //
        UCHAR  Buffer[0x34];
    };

} VSTOR_PACKET, *PVSTOR_PACKET;

C_ASSERT((sizeof(VSTOR_PACKET) % 8) == 0);

#define VMSTORAGE_SIZEOF_VSTOR_PACKET_REVISION_1 (RTL_SIZEOF_THROUGH_FIELD(VSTOR_PACKET, Status) + VMSTORAGE_SIZEOF_VMSCSI_REQUEST_REVISION_1)

C_ASSERT(VMSTORAGE_SIZEOF_VSTOR_PACKET_REVISION_1 == 0x30);

#define VMSTORAGE_SIZEOF_VSTOR_PACKET_REVISION_2 (RTL_SIZEOF_THROUGH_FIELD(VSTOR_PACKET, Status) + VMSTORAGE_SIZEOF_VMSCSI_REQUEST_REVISION_2)

C_ASSERT(VMSTORAGE_SIZEOF_VSTOR_PACKET_REVISION_2 == 0x40);


//
//  Packet flags
//

//
//  This flag indicates that the server should send back a completion for this
//  packet.
//
#define REQUEST_COMPLETION_FLAG 0x1

//
//  This is the set of flags that the VSC can set in any packets it sends
//
#define VSC_LEGAL_FLAGS (REQUEST_COMPLETION_FLAG)


#pragma pack(pop)

typedef struct _ADAPTER_ADDRESS
{
    ULONGLONG PartitionId;

    GUID ChannelInstanceGUID;

    //
    //  SCSI address
    //
    UCHAR Reserved;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;

    //
    //  Flags
    //
    ULONG Flags;

    //
    // World wide names for SynthFc
    //
    BOOLEAN PrimaryWwnActive;
    UCHAR   PrimaryPortWwn[8];
    UCHAR   PrimaryNodeWwn[8];
    UCHAR   SecondaryPortWwn[8];
    UCHAR   SecondaryNodeWwn[8];

} ADAPTER_ADDRESS, *PADAPTER_ADDRESS;

//
//  Flags for ADAPTER_ADDRESS
//
#define ADAPTER_ADDRESS_EMULATED_DEVICE            0x1
#define ADAPTER_ADDRESS_SYNTHFC_DEVICE             0x2


//
// Alignment information
//
#define VSTORAGE_ALIGNMENT_MASK 0x01

#pragma warning(pop)

