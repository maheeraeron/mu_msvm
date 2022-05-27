/*++

Copyright (c) Microsoft Corporation

Module Name:

    kdnetinterface.h

Abstract:

    Defines the public interface for kdnet used by the kdnet library consumers.

Author:

    Ben Leis (benleis) 5-17-2010

--*/

#pragma once

typedef struct {
    UCHAR Type;  //CmResourceType
    BOOLEAN Valid;

    union {
        UCHAR Reserved[2];

        struct {
            UCHAR BitWidth;
            UCHAR AccessSize;
        };
    };

    PUCHAR    TranslatedAddress;
    ULONG     Length;
} DEBUG_DEVICE_ADDRESS, *PDEBUG_DEVICE_ADDRESS;

typedef struct {
    PHYSICAL_ADDRESS  Start;
    PHYSICAL_ADDRESS  MaxEnd;
    PVOID             VirtualAddress;
    ULONG             Length;
    BOOLEAN           Cached;
    BOOLEAN           Aligned;
} DEBUG_MEMORY_REQUIREMENTS, *PDEBUG_MEMORY_REQUIREMENTS;

typedef enum {
    KdNameSpacePCI,
    KdNameSpaceACPI,
    KdNameSpaceAny,
    KdNameSpaceNone,

    //
    // Maximum namespace enumerator.
    //

    KdNameSpaceMax,
} KD_NAMESPACE_ENUM, *PKD_NAMESPACE_ENUM;

//
// Debug transport specific data for use by the transport.
//

typedef struct _DEBUG_TRANSPORT_DATA {
    ULONG HwContextSize;
    ULONG SharedVisibleDataSize;
    BOOLEAN UseSerialFraming;
    BOOLEAN ValidUSBCoreId;
    UCHAR USBCoreId;
} DEBUG_TRANSPORT_DATA, *PDEBUG_TRANSPORT_DATA;

//
// IOMMU-DMA debug transport data required by the EFI_PCI_PROTOCOL
//

typedef struct _DEBUG_IOMMU_EFI_DATA {
  PVOID PciIoProtocolHandle;
  PVOID Mapping;
} DEBUG_EFI_IOMMU_DATA, *PDEBUG_EFI_IOMMU_DATA;

#define MAXIMUM_DEBUG_BARS 6

#define DBG_DEVICE_FLAG_HAL_SCRATCH_ALLOCATED 0x01
#define DBG_DEVICE_FLAG_BARS_MAPPED           0x02
#define DBG_DEVICE_FLAG_SCRATCH_ALLOCATED     0x04
#define DBG_DEVICE_FLAG_UNCACHED_MEMORY       0x08
#define DBG_DEVICE_FLAG_SYNTHETIC             0x10
#define DBG_DEVICE_FLAG_HOST_VISIBLE_ALLOCATED 0x20

typedef struct _DEBUG_DEVICE_DESCRIPTOR {
    ULONG     Bus;
    ULONG     Slot;
    USHORT    Segment;
    USHORT    VendorID;
    USHORT    DeviceID;
    UCHAR     BaseClass;
    UCHAR     SubClass;
    UCHAR     ProgIf;
    union {
        UCHAR     Flags;
        struct {
            UCHAR DbgHalScratchAllocated : 1;
            UCHAR DbgBarsMapped : 1;
            UCHAR DbgScratchAllocated : 1;
            UCHAR DbgUncachedMemory : 1;
            UCHAR DbgSynthetic : 1;
        };
    };
    BOOLEAN   Initialized;
    BOOLEAN   Configured;
    DEBUG_DEVICE_ADDRESS BaseAddress[MAXIMUM_DEBUG_BARS];
    DEBUG_MEMORY_REQUIREMENTS Memory;
    ULONG     Dbg2TableIndex;
    USHORT    PortType;
    USHORT    PortSubtype;
    PVOID     OemData;
    ULONG     OemDataLength;
    KD_NAMESPACE_ENUM NameSpace;
    PWCHAR    NameSpacePath;
    ULONG     NameSpacePathLength;
    ULONG     TransportType;
    DEBUG_TRANSPORT_DATA TransportData;
    DEBUG_EFI_IOMMU_DATA EfiIoMmuData;
} DEBUG_DEVICE_DESCRIPTOR, *PDEBUG_DEVICE_DESCRIPTOR;

//
// Ethernet hardware address.
//

typedef struct _ETHERNET_ADDRESS {
    UCHAR Address[6];
} ETHERNET_ADDRESS, *PETHERNET_ADDRESS;

//
// IPv6 address.
//

#if !defined(KDNET_IPV6_ADDRESS)

#define KDNET_IPV6_ADDRESS
typedef struct _IPV6_ADDRESS {
    union {
        struct {
            ULONG64 QW0;
            ULONG64 QW1;
        };

        struct {
            ULONG DW0;
            ULONG DW1;
            ULONG DW2;
            ULONG DW3;
        };

        struct {
            USHORT W0;
            USHORT W1;
            USHORT W2;
            USHORT W3;
            USHORT W4;
            USHORT W5;
            USHORT W6;
            USHORT W7;
        };
    };
} IPV6_ADDRESS, *PIPV6_ADDRESS;

#endif

typedef
NTSTATUS
(*KD_POWER_ROUTINE) (
    ULONG State,
    __inout PVOID KdContext
    );

typedef struct _DHCP_STATE {
    ULONG DhcpTransactionID;
    ULONG DhcpSeconds;
    ULONG DhcpServer;
    ULONG DhcpIPAddress;
    ULONG DhcpSubnetMask;
    ULONG DhcpRouterIP;
    ULONG DhcpState;
    ULONG DhcpRenewTime;
    ULONG DhcpRebindTime;
    ULONG DhcpLeaseTime;
    ULONG DhcpTimer;
    ULONG DhcpLeaseRenewed;
    __declspec(align(4))
    ETHERNET_ADDRESS DhcpServerMac;
} DHCP_STATE, *PDHCP_STATE;

C_ASSERT((FIELD_OFFSET(DHCP_STATE, DhcpServerMac) & 0x3) == 0);

typedef struct _IPV6_STATE {
    IPV6_ADDRESS LinkLocalAddress;
    IPV6_ADDRESS GlobalAddress;
    IPV6_ADDRESS Prefix;
    IPV6_ADDRESS RouterLinkLocalAddress;
    __declspec(align(4))
    ETHERNET_ADDRESS RouterEthernetAddress;
    ULONG ReachableTime;
    ULONG RetransmitTime;
    ULONG Mtu;
    USHORT RouterLifetime;
    UCHAR HopLimit;
    UCHAR NoRouterSolicitations;
} IPV6_STATE, *PIPV6_STATE;

C_ASSERT((FIELD_OFFSET(IPV6_STATE, RouterEthernetAddress) & 0x3) == 0);

#define KD_NET_KEY_SIZE_DWORDS 8
#define KD_NET_KEY_SIZE (KD_NET_KEY_SIZE_DWORDS * sizeof(ULONG))
#define KD_NET_TARGET_RANDOM_SIZE 32
#define KD_NET_HOST_CONNECTION_INFO_SIZE 256
#define KD_NET_MACHINE_ID_SIZE 32

typedef struct _DEBUG_NET_PARAMETERS {

    //
    // device descriptor (pci slot, bus, etc)
    //

    DEBUG_DEVICE_DESCRIPTOR DbgDeviceDescriptor;

    //
    // Target UDP port address.
    //

    USHORT TargetPort;

    //
    // Optional assigned Host UDP port address.
    //

    USHORT AssignedHostPort;

    //
    // Host UDP port address.
    //

    USHORT HostPort;

    //
    // Optional assigned Target IP address.
    //

    __declspec(align(16))
    IPV6_ADDRESS TargetIP;

    //
    // Optional assigned Gateway IP address.
    //

    __declspec(align(16))
    IPV6_ADDRESS GatewayIP;

    //
    // Optional assigned Host IP address.
    //

    __declspec(align(16))
    IPV6_ADDRESS AssignedHostIP;

    //
    // Host IP address.
    //

    __declspec(align(16))
    IPV6_ADDRESS HostIP;

    //
    // Host Ethernet physical address.
    //

    __declspec(align(4))
    ETHERNET_ADDRESS HostMac;

    //
    // is the debugger active?
    //

    __declspec(align(4))
    BOOLEAN DebuggerActive;

    //
    // Encrypted and authenticated KD packets required.
    //

    BOOLEAN EncryptedLink;

    //
    // Dhcp enables DHCP support when TRUE.
    //

    BOOLEAN Dhcp;

    //
    // Checks source MAC address on debugger packets.
    //

    BOOLEAN VerifyHostMac;

    //
    // Connected flag.
    //

    ULONG Connected;

    //
    // Target sequence number must never decrease.  Store it here since
    // KdNetData gets zeroed after hibernate and sleep.
    //

    volatile LONG64 TargetSequenceNumber;

    //
    // Last valid Host sequence number.  Store it here so it won't get zeroed
    // when KdNetData gets zeroed after hibernate and sleep.  That ensures
    // there is never a window for replay attacks.  (After reinitialization but
    // before the host has sent a packet.)
    //

    ULONG64 LastValidHostSequenceNumber;

    //
    // Target debug encryption/decryption key. (TDK)
    //

    UCHAR Key[KD_NET_KEY_SIZE];

    //
    // Debug session encryption/decryption key. (DSK)
    //

    UCHAR SessionKey[KD_NET_KEY_SIZE];

    //
    // Offer timer.  Used for the Offer message sequence number.
    //

    ULONG64 OfferTimer;

    //
    // Target random data.
    //

    __declspec(align(8))
    UCHAR TargetRandom[KD_NET_TARGET_RANDOM_SIZE];

    //
    // Host connection information from the currently connected host if any.
    // This information is included in the OFFER packets.
    //

    UCHAR HostConnectionInfo[KD_NET_HOST_CONNECTION_INFO_SIZE];

    //
    // Offer packets send target machine KdEnteredDebugger state when TRUE.
    // Note that this state is sent in the unencrypted header of the packet.
    //

    BOOLEAN OffersSendStatus;

    //
    // TRUE when KDNET is running over IPv6.  If either the host or the target
    // IP addresses are IPv6 addresses, this will be set TRUE and all KDNET
    // communication will be attempted only over IPv6.
    //

    BOOLEAN Ipv6;

    //
    // GUID specifying which NIC to use for synthetic net debugging.
    //

    GUID SynthNicId;

    //
    // Use SMBIOS UUID and the NIC MAC address to identify machines.
    //

    __declspec(align(8))
    UCHAR MachineId[KD_NET_MACHINE_ID_SIZE];

    //
    // The legacy serial port number for legacy serial ports
    //

    ULONG LegacySerialPortNumber;

    //
    // The baud rate to use if KDNET is running across a serial line.
    //

    ULONG SerialBaudRate;

    //
    // Dhcp state.  Save it here so it doesn't zeroed when KdNetData is
    // reinitialized.  This is required so that KDNET does not have to do DHCP
    // on every power transition - only when power is off long enough to
    // require renewal or reacquisition of the lease.
    //

    DHCP_STATE DhcpState;

    //
    // Track the interrupt time when the controller is initialized and shutdown
    // so that DHCP transactions can be minimized during power transitions.
    //

    ULONG64 InitializeTimestamp;
    ULONG64 ShutdownTimestamp;
    ULONG64 TimestampFrequency;

    //
    // This is the power management routine to call to power manage KDNET for
    // the specific environment where KDNET is being used.  This routine is
    // called to shutdown and reinitialize KDNET when the NIC or debug hardware
    // gets unplugged, and then plugged back in.
    //

    KD_POWER_ROUTINE KdPower;

    //
    // This is the power management context to pass to the KdPower routine.
    //

    PVOID KdPowerContext;

    //
    // IPv6 state.  Used for globally routable IPv6 address support.
    //

    IPV6_STATE Ipv6State;

} DEBUG_NET_PARAMETERS, *PDEBUG_NET_PARAMETERS;

C_ASSERT((FIELD_OFFSET(DEBUG_NET_PARAMETERS, HostMac) & 0x3) == 0);
C_ASSERT((FIELD_OFFSET(DEBUG_NET_PARAMETERS, DebuggerActive) & 0x3) == 0);

#if defined(_KDNET_INTERNAL_)

typedef struct _DEBUG_NET_DATA *PDEBUG_NET_ADAPTER;

#else

typedef PVOID PDEBUG_NET_ADAPTER;

#endif

#define PCI_VID_MSHV_NET                0xfffd
#define PCI_VID_MSHV_SYNTH              0xfffc
#define PCI_VID_MSHV_SYNTH_NET          0xfff9

typedef struct _KD_CONTEXT {
    ULONG KdpDefaultRetries;
    BOOLEAN KdpControlCPending;
    UCHAR Reserved;
    USHORT Flags;
    PDEBUG_DEVICE_DESCRIPTOR DebugDevice;
    PVOID TransportContext;
} KD_CONTEXT, *PKD_CONTEXT;

typedef
PHYSICAL_ADDRESS
(*KDNET_GET_PHYSICAL_ADDRESS) (
    __in PVOID Va
    );

typedef
VOID
(*KDNET_STALL_EXECUTION_PROCESSOR) (
    ULONG Microseconds
    );

typedef
UCHAR
(*KDNET_READ_REGISTER_UCHAR) (
    __in PUCHAR Register
    );

typedef
USHORT
(*KDNET_READ_REGISTER_USHORT) (
    __in PUSHORT Register
    );

typedef
ULONG
(*KDNET_READ_REGISTER_ULONG) (
    __in PULONG Register
    );

typedef
VOID
(*KDNET_WRITE_REGISTER_UCHAR) (
    __in PUCHAR Register,
    __in UCHAR Value
    );

typedef
VOID
(*KDNET_WRITE_REGISTER_USHORT) (
    __in PUSHORT Register,
    __in USHORT Value
    );

typedef
VOID
(*KDNET_WRITE_REGISTER_ULONG) (
    __in PULONG Register,
    __in ULONG Value
    );

typedef
UCHAR
(*KDNET_READ_PORT_UCHAR) (
    __in PUCHAR Port
    );

typedef
USHORT
(*KDNET_READ_PORT_USHORT) (
    __in PUSHORT Port
    );

typedef
ULONG
(*KDNET_READ_PORT_ULONG) (
    __in PULONG Port
    );

typedef
VOID
(*KDNET_WRITE_PORT_UCHAR) (
    __in PUCHAR Port,
    __in UCHAR Value
    );

typedef
VOID
(*KDNET_WRITE_PORT_USHORT) (
    __in PUSHORT Port,
    __in USHORT Value
    );

typedef
VOID
(*KDNET_WRITE_PORT_ULONG) (
    __in PULONG Port,
    __in ULONG Value
    );

typedef
ULONG
(*KDNET_GET_PCI_DATA_BY_OFFSET) (
    __in ULONG BusNumber,
    __in ULONG SlotNumber,
    __out_bcount(Length) PVOID Buffer,
    __in ULONG Offset,
    __in ULONG Length
    );

typedef
ULONG
(*KDNET_SET_PCI_DATA_BY_OFFSET) (
    __in ULONG BusNumber,
    __in ULONG SlotNumber,
    __in_bcount(Length) PVOID Buffer,
    __in ULONG Offset,
    __in ULONG Length
    );

typedef
VOID
(*KDNET_SET_DEBUGGER_NOT_PRESENT) (
    __in BOOLEAN NotPresent
    );

typedef
VOID
(*KDNET_BUGCHECK_EX) (
    __in ULONG BugCheckCode,
    __in ULONG_PTR BugCheckParameter1,
    __in ULONG_PTR BugCheckParameter2,
    __in ULONG_PTR BugCheckParameter3,
    __in ULONG_PTR BugCheckParameter4
    );

typedef
PVOID
(*KDNET_MAP_PHYSICAL_MEMORY_64) (
    _In_ PHYSICAL_ADDRESS PhysicalAddress,
    _In_ ULONG NumberPages,
    _In_ BOOLEAN FlushCurrentTLB
    );

typedef
VOID
(*KDNET_UNMAP_VIRTUAL_ADDRESS) (
    _In_ PVOID VirtualAddress,
    _In_ ULONG NumberPages,
    _In_ BOOLEAN FlushCurrentTLB
    );

typedef
BOOLEAN
(*KDNET_VMBUS_INITIALIZE) (
    _Out_ PVOID Context,
    _In_ PVOID Parameters,
    _In_ BOOLEAN UnreserveChannels,
    _In_ PVOID ArrivalRoutine,
    _In_opt_ PVOID ArrivalRoutineContext,
    _In_ UINT32 RequestedVersion
    );

typedef struct _KDNET_IMPORTS
{
    KDNET_GET_PHYSICAL_ADDRESS GetPhysicalAddress;
    KDNET_STALL_EXECUTION_PROCESSOR StallExecutionProcessor;
    KDNET_READ_REGISTER_UCHAR ReadRegisterUChar;
    KDNET_READ_REGISTER_USHORT ReadRegisterUShort;
    KDNET_READ_REGISTER_ULONG ReadRegisterULong;
    KDNET_READ_PORT_UCHAR ReadPortUChar;
    KDNET_READ_PORT_USHORT ReadPortUShort;
    KDNET_READ_PORT_ULONG ReadPortULong;
    KDNET_WRITE_REGISTER_UCHAR WriteRegisterUChar;
    KDNET_WRITE_REGISTER_USHORT WriteRegisterUShort;
    KDNET_WRITE_REGISTER_ULONG WriteRegisterULong;
    KDNET_GET_PCI_DATA_BY_OFFSET GetPciDataByOffset;
    KDNET_SET_PCI_DATA_BY_OFFSET SetPciDataByOffset;
    KDNET_SET_DEBUGGER_NOT_PRESENT SetDebuggerNotPresent;
    KDNET_WRITE_PORT_UCHAR WritePortUChar;
    KDNET_WRITE_PORT_USHORT WritePortUShort;
    KDNET_WRITE_PORT_ULONG WritePortULong;
    KDNET_BUGCHECK_EX BugCheckEx;
    KDNET_MAP_PHYSICAL_MEMORY_64 MapPhysicalMemory64;
    KDNET_UNMAP_VIRTUAL_ADDRESS UnmapVirtualAddress;
    KDNET_VMBUS_INITIALIZE VmbusInitialize;
} KDNET_IMPORTS, *PKDNET_IMPORTS;

typedef
VOID
(*KDNET_INITIALIZE_EARLY)(
    _In_ PVOID AllocatedData,
    _In_ PDEBUG_NET_PARAMETERS DebugParameters
    );

typedef
VOID
(*KDNET_INITIALIZE_LIBRARY)(
    _In_ PKDNET_IMPORTS ImportTable
    );

typedef
NTSTATUS
(*KDNET_INITIALIZE_DEBUGGING)(
    VOID
    );

typedef
ULONG
(*KDNET_GET_HARDWARE_CONTEXT_SIZE)(
    _In_ PDEBUG_DEVICE_DESCRIPTOR Device
    );

typedef
ULONG
(*KDNET_GET_NET_DATA_SIZE)(
    __in PDEBUG_DEVICE_DESCRIPTOR Device
    );

typedef
ULONG
(*KDNET_RECEIVE_PACKET)(
    _In_ ULONG PacketType,
    _Inout_opt_ PSTRING MessageHeader,
    _Inout_opt_ PSTRING MessageData,
    _Out_opt_ PULONG DataLength,
    _Inout_ PKD_CONTEXT KdContext
    );

typedef
VOID
(*KDNET_SEND_PACKET)(
    _In_ ULONG PacketType,
    _In_ PSTRING MessageHeader,
    _In_opt_ PSTRING MessageData,
    _Inout_ PKD_CONTEXT KdContext
    );

typedef
ULONG
(*KDNET_GET_PACKET_COUNT)(
    VOID
    );

typedef struct _EFI_KDNET_HOB {
    KDNET_INITIALIZE_LIBRARY InitializeLibrary;
    KDNET_INITIALIZE_DEBUGGING InitializeDebugging;
    KDNET_SEND_PACKET SendPacket;
    KDNET_RECEIVE_PACKET ReceivePacket;
    KDNET_GET_PACKET_COUNT GetSentPacketCount;
    KDNET_GET_PACKET_COUNT GetReceivedPacketCount;
    UINT64 CanonicalizationMask;
} EFI_KDNET_HOB;
