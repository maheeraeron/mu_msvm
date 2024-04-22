/** @file
    This files contains definitions for messages that are sent between
    instances of the Channel Management Library in separate partitions, or
    in some cases, back to itself.

    Copyright (c) Microsoft Corporation.
    Licensed under the BSD-2-Clause-Patent license.

**/

#pragma once

#pragma warning(push)
#pragma warning(disable: 4214)
#pragma pack(push,1)

//
// A revision number of vmbus that is used for ensuring both ends on a
// partition are using compatible versions.
//
#define VMBUS_MAKE_VERSION(Major, Minor) (((Major) << 16) | (Minor))

#define VMBUS_VERSION_V1           VMBUS_MAKE_VERSION(0, 13)
#define VMBUS_VERSION_WIN7         VMBUS_MAKE_VERSION(1, 1)
#define VMBUS_VERSION_WIN8         VMBUS_MAKE_VERSION(2, 4)
#define VMBUS_VERSION_WIN8_1       VMBUS_MAKE_VERSION(3, 0)

#define VMBUS_VERSION_LATEST       VMBUS_VERSION_WIN8_1

//
// Version 1 messages
//
typedef enum _VMBUS_CHANNEL_MESSAGE_TYPE
{
    ChannelMessageInvalid                   =  0,
    ChannelMessageOfferChannel              =  1,
    ChannelMessageRescindChannelOffer       =  2,
    ChannelMessageRequestOffers             =  3,
    ChannelMessageAllOffersDelivered        =  4,
    ChannelMessageOpenChannel               =  5,
    ChannelMessageOpenChannelResult         =  6,
    ChannelMessageCloseChannel              =  7,
    ChannelMessageGpadlHeader               =  8,
    ChannelMessageGpadlBody                 =  9,
    ChannelMessageGpadlCreated              = 10,
    ChannelMessageGpadlTeardown             = 11,
    ChannelMessageGpadlTorndown             = 12,
    ChannelMessageRelIdReleased             = 13,
    ChannelMessageInitiateContact           = 14,
    ChannelMessageVersionResponse           = 15,
    ChannelMessageUnload                    = 16,
    ChannelMessageUnloadComplete            = 17,
    ChannelMessageCount
} VMBUS_CHANNEL_MESSAGE_TYPE, *PVMBUS_CHANNEL_MESSAGE_TYPE;


typedef struct _VMBUS_CHANNEL_MESSAGE_HEADER
{
    VMBUS_CHANNEL_MESSAGE_TYPE  MessageType;
    UINT32                      Padding;
} VMBUS_CHANNEL_MESSAGE_HEADER, *PVMBUS_CHANNEL_MESSAGE_HEADER;

//
// Offer flags. The flags parameter is 8 bits, and any undefined bits are
// available, since bits that were not defined are masked out when using an
// older protocol version.
//
#define VMBUS_OFFER_FLAG_ENUMERATE_DEVICE_INTERFACE     1
#define VMBUS_OFFER_FLAG_NAMED_PIPE_MODE                0x10

#define VMBUS_OFFER_FLAGS_WIN6 (VMBUS_OFFER_FLAG_ENUMERATE_DEVICE_INTERFACE | \
                                VMBUS_OFFER_FLAG_NAMED_PIPE_MODE)

#define VMBUS_OFFER_FLAGS_WIN8 VMBUS_OFFER_FLAGS_WIN6

//
// Offer Channel parameters
//
typedef struct _VMBUS_CHANNEL_OFFER_CHANNEL
{
    VMBUS_CHANNEL_MESSAGE_HEADER;

    GUID InterfaceType;
    GUID InterfaceInstance;

    //
    // These reserved fields may be non-zero before Windows 8.
    //
    UINT64 Reserved;
    UINT64 Reserved2;

    UINT16 Flags;
    UINT16 MmioMegabytes;

    UCHAR UserDefined[MAX_USER_DEFINED_BYTES];

    UINT16 SubChannelIndex; // Defined in Win8
    UINT16 Reserved3;
    UINT32 ChildRelId;

    UINT8 MonitorId;
    UINT8 MonitorAllocated:1;
    UINT8 Reserved4:7;

    //
    // The following fields are only available in Windows 7 and later.
    //
    union
    {
        struct
        {
            UINT16 IsDedicatedInterrupt:1;
            UINT16 Reserved5:15;

            UINT32 ConnectionId;
        };

        UCHAR Windows6Offset;
    };

} VMBUS_CHANNEL_OFFER_CHANNEL, *PVMBUS_CHANNEL_OFFER_CHANNEL;

static_assert(sizeof(VMBUS_CHANNEL_OFFER_CHANNEL) <= MAXIMUM_SYNIC_MESSAGE_BYTES, "Offer message too large");

#define VMBUS_CHANNEL_OFFER_CHANNEL_SIZE_WIN6 FIELD_OFFSET(VMBUS_CHANNEL_OFFER_CHANNEL, Windows6Offset)

typedef struct _VMBUS_CHANNEL_RESCIND_OFFER
{
    VMBUS_CHANNEL_MESSAGE_HEADER;
    UINT32          ChildRelId;
} VMBUS_CHANNEL_RESCIND_OFFER, *PVMBUS_CHANNEL_RESCIND_OFFER;

typedef struct _VMBUS_CHANNEL_OPEN_CHANNEL
{
    VMBUS_CHANNEL_MESSAGE_HEADER;

    //
    // Identifies the specific VMBus channel that is being opened.
    //
    UINT32          ChildRelId;

    //
    // ID making a particular open request at a channel offer unique.
    //
    UINT32          OpenId;

    //
    // GPADL for the channel's ring buffer.
    //
    UINT32          RingBufferGpadlHandle;

    //
    // Target VP index for the server-to-client interrupt. (>= Win8 only)
    //
    UINT32          TargetVp;

    //
    // The upstream ring buffer begins at offset zero in the memory described
    // by RingBufferGpadlHandle. The downstream ring buffer follows it at this
    // offset (in pages).
    //
    UINT32          DownstreamRingBufferPageOffset;

    //
    // User-specific data to be passed along to the server endpoint.
    //
    UCHAR           UserData[MAX_USER_DEFINED_BYTES];

} VMBUS_CHANNEL_OPEN_CHANNEL, *PVMBUS_CHANNEL_OPEN_CHANNEL;

typedef struct _VMBUS_CHANNEL_OPEN_RESULT
{
    VMBUS_CHANNEL_MESSAGE_HEADER;
    UINT32      ChildRelId;
    UINT32      OpenId;
    UINT32      Status;
} VMBUS_CHANNEL_OPEN_RESULT, *PVMBUS_CHANNEL_OPEN_RESULT;

typedef struct _VMBUS_CHANNEL_CLOSE_CHANNEL
{
    VMBUS_CHANNEL_MESSAGE_HEADER;
    UINT32      ChildRelId;
} VMBUS_CHANNEL_CLOSE_CHANNEL, *PVMBUS_CHANNEL_CLOSE_CHANNEL;

//
// The number of PFNs in a GPADL message is defined by the number of pages
// that would be spanned by ByteCount and ByteOffset.  If the implied number
// of PFNs won't fit in this packet, there will be a follow-up packet that
// contains more.
//
typedef struct _VMBUS_CHANNEL_GPADL_HEADER
{
    VMBUS_CHANNEL_MESSAGE_HEADER;
    UINT32      ChildRelId;
    UINT32      Gpadl;
    UINT16      RangeBufLen;
    UINT16      RangeCount;
    GPA_RANGE   Range[1];
} VMBUS_CHANNEL_GPADL_HEADER, *PVMBUS_CHANNEL_GPADL_HEADER;

//
// This is the followup packet that contains more PFNs.
//
typedef struct _VMBUS_CHANNEL_GPADL_BODY
{
    VMBUS_CHANNEL_MESSAGE_HEADER;
    UINT32              MessageNumber;
    UINT32              Gpadl;
    UINT64              Pfn[1];
} VMBUS_CHANNEL_GPADL_BODY, *PVMBUS_CHANNEL_GPADL_BODY;


typedef struct _VMBUS_CHANNEL_GPADL_CREATED
{
    VMBUS_CHANNEL_MESSAGE_HEADER;
    UINT32              ChildRelId;
    UINT32              Gpadl;
    UINT32              CreationStatus;
} VMBUS_CHANNEL_GPADL_CREATED, *PVMBUS_CHANNEL_GPADL_CREATED;

typedef struct _VMBUS_CHANNEL_GPADL_TEARDOWN
{
    VMBUS_CHANNEL_MESSAGE_HEADER;
    UINT32              ChildRelId;
    UINT32              Gpadl;
} VMBUS_CHANNEL_GPADL_TEARDOWN, *PVMBUS_CHANNEL_GPADL_TEARDOWN;

typedef struct _VMBUS_CHANNEL_GPADL_TORNDOWN
{
    VMBUS_CHANNEL_MESSAGE_HEADER;
    UINT32              Gpadl;
} VMBUS_CHANNEL_GPADL_TORNDOWN, *PVMBUS_CHANNEL_GPADL_TORNDOWN;

typedef struct _VMBUS_CHANNEL_RELID_RELEASED
{
    VMBUS_CHANNEL_MESSAGE_HEADER;
    UINT32              ChildRelId;
} VMBUS_CHANNEL_RELID_RELEASED, *PVMBUS_CHANNEL_RELID_RELEASED;

typedef struct _VMBUS_CHANNEL_INITIATE_CONTACT
{
    VMBUS_CHANNEL_MESSAGE_HEADER;
    UINT32              VMBusVersionRequested;
    UINT32              TargetMessageVp;
    UINT64              InterruptPage;
    UINT64              ParentToChildMonitorPageGpa;
    UINT64              ChildToParentMonitorPageGpa;
} VMBUS_CHANNEL_INITIATE_CONTACT, *PVMBUS_CHANNEL_INITIATE_CONTACT;

typedef struct _VMBUS_CHANNEL_VERSION_RESPONSE
{
    VMBUS_CHANNEL_MESSAGE_HEADER;
    BOOLEAN     VersionSupported;
    UINT8       ConnectionState;
    UINT8       Pad[2];
    UINT32      SelectedVersion;
} VMBUS_CHANNEL_VERSION_RESPONSE, *PVMBUS_CHANNEL_VERSION_RESPONSE;

//
// Status codes for the ConnectionState field of
// VMBUS_CHANNEL_VERSION_RESPONSE.
//
// N.B. If VersionSupported is FALSE, do not consult this value.
// If the requested version is less than VMBUS_VERSION_WIN8, these values
// may be uninitialized memory, cannot be consulted, and the effective value
// must be assumed to be VmbusChannelConnectionSuccessful.
//
// All non-zero values should be taken to mean a failure. The specific values
// are merely used to better provide information to the guest about the cause
// of the failure.
//
enum
{
    VmbusChannelConnectionSuccessful = 0,
    VmbusChannelConnectionFailedLowResources = 1,
    VmbusChannelConnectionFailedUnknownFailure = 2,
};

typedef VMBUS_CHANNEL_MESSAGE_HEADER VMBUS_CHANNEL_UNLOAD, *PVMBUS_CHANNEL_UNLOAD;

typedef VMBUS_CHANNEL_MESSAGE_HEADER VMBUS_CHANNEL_UNLOAD_COMPLETE, *PVMBUS_CHANNEL_UNLOAD_COMPLETE;

//
// Kind of a table to use the preprocessor to get us the right type for a
// specified message ID. Used with ChAllocateSendMessage()
//
#define ChannelMessageOfferChannel_TYPE         VMBUS_CHANNEL_OFFER_CHANNEL
#define ChannelMessageRescindChannelOffer_TYPE  VMBUS_CHANNEL_RESCIND_OFFER
#define ChannelMessageRequestOffers_TYPE        VMBUS_CHANNEL_MESSAGE_HEADER
#define ChannelMessageAllOffersDelivered_TYPE   VMBUS_CHANNEL_MESSAGE_HEADER
#define ChannelMessageOpenChannel_TYPE          VMBUS_CHANNEL_OPEN_CHANNEL
#define ChannelMessageOpenChannelResult_TYPE    VMBUS_CHANNEL_OPEN_RESULT
#define ChannelMessageCloseChannel_TYPE         VMBUS_CHANNEL_CLOSE_CHANNEL
#define ChannelMessageGpadlHeader_TYPE          VMBUS_CHANNEL_GPADL_HEADER
#define ChannelMessageGpadlBody_TYPE            VMBUS_CHANNEL_GPADL_BODY
#define ChannelMessageGpadlCreated_TYPE         VMBUS_CHANNEL_GPADL_CREATED
#define ChannelMessageGpadlTeardown_TYPE        VMBUS_CHANNEL_GPADL_TEARDOWN
#define ChannelMessageGpadlTorndown_TYPE        VMBUS_CHANNEL_GPADL_TORNDOWN
#define ChannelMessageRelIdReleased_TYPE        VMBUS_CHANNEL_RELID_RELEASED
#define ChannelMessageInitiateContact_TYPE      VMBUS_CHANNEL_INITIATE_CONTACT
#define ChannelMessageVersionResponse_TYPE      VMBUS_CHANNEL_VERSION_RESPONSE
#define ChannelMessageUnload_TYPE               VMBUS_CHANNEL_UNLOAD
#define ChannelMessageUnloadComplete_TYPE       VMBUS_CHANNEL_UNLOAD_COMPLETE

#pragma pack(pop)
#pragma warning(pop)

