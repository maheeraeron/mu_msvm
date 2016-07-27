/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    hvgdk_mini.h

Abstract:

    Type definitions for the hypervisor guest interface to kernel.

--*/

#if !defined(_HVGDK_MINI_)
#define _HVGDK_MINI_

#if _MSC_VER > 1000
#pragma once
#endif

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning(disable:4200) // zero length array
#pragma warning(disable:4214) // bit field types other than int
#pragma warning(disable:4324) // structure was padded due to __declspec(align())

//
// Define hypervisor constants.
//

//
// Define a 128bit type.
//
typedef union DECLSPEC_ALIGN(16) _HV_UINT128
{
    struct
    {
        UINT64  Low64;
        UINT64  High64;
    };

    UINT32  Dword[4];
} HV_UINT128, *PHV_UINT128;


//
// Define a 256bit type.
//
typedef struct DECLSPEC_ALIGN(32) _HV_UINT256
{
    HV_UINT128  Low128;
    HV_UINT128  High128;

} HV_UINT256, *PHV_UINT256;


//
// Define a 512bit type.
//
typedef struct DECLSPEC_ALIGN(32) _HV_UINT512
{
    HV_UINT256  Low128;
    HV_UINT256  High128;

} HV_UINT512, *PHV_UINT512;

//
// Define an alignment for structures passed via hypercall.
//
#define HV_CALL_ALIGNMENT   8

#define HV_CALL_ATTRIBUTES DECLSPEC_ALIGN(HV_CALL_ALIGNMENT)
#define HV_CALL_ATTRIBUTES_ALIGNED(__alignment__) DECLSPEC_ALIGN(__alignment__)

#ifndef HV_STATUS_SUCCESS
//
// Status codes for hypervisor operations.
//
typedef UINT16 HV_STATUS, *PHV_STATUS;

//
// Standard Success values
//

#define HV_STATUS_SUCCESS                ((HV_STATUS)0x0000)
#endif

//
// MessageId: HV_STATUS_INVALID_PARAMETER
//
// MessageText:
//
// An invalid parameter was specified.
//
#define HV_STATUS_INVALID_PARAMETER      ((HV_STATUS)0x0005)

//
// MessageId: HV_STATUS_ACCESS_DENIED
//
// MessageText:
//
// Access to the specified object was denied.
//
#define HV_STATUS_ACCESS_DENIED          ((HV_STATUS)0x0006)

//
// MessageId: HV_STATUS_INSUFFICIENT_MEMORY
//
// MessageText:
//
// There is not enough memory in the hypervisor pool to complete the operation.
//
#define HV_STATUS_INSUFFICIENT_MEMORY    ((HV_STATUS)0x000B)

//
// MessageId: HV_STATUS_NOT_FOUND
//
// MessageText:
//
// The iteration is complete; no addition items in the iteration could be found.
//
#define HV_STATUS_NOT_FOUND              ((HV_STATUS)0x0010)

//
// MessageId: HV_STATUS_NO_RESOURCES
//
// MessageText:
//
// There are not enough resources to complete the operation.
//
#define HV_STATUS_NO_RESOURCES           ((HV_STATUS)0x001D)

//
// MessageId: HV_STATUS_PARTIAL_PACKET
//
// MessageText:
//
// The debug packet returned is only a partial packet due to an io error.
//
#define HV_STATUS_PARTIAL_PACKET         ((HV_STATUS)0x001F)

//
// MessageId: HV_STATUS_INSUFFICIENT_BUFFER
//
// MessageText:
//
// The specified buffer was too small to contain all of the requested data.
//
#define HV_STATUS_INSUFFICIENT_BUFFER    ((HV_STATUS)0x0033)

//
// MessageId: HV_STATUS_CPUID_FEATURE_VALIDATION_ERROR
//
// MessageText:
//
// Generic logical processor CPUID feature set validation error.
//
#define HV_STATUS_CPUID_FEATURE_VALIDATION_ERROR ((HV_STATUS)0x003C)

//
// Memory Types
//
// Guest virtual addresses (GVAs) are used within the guest when it enables address
// translation and provides a valid guest page table.
//

typedef UINT64 HV_GVA, *PHV_GVA;

#define HV_X64_PAGE_SIZE           4096
#define HV_X64_LARGE_PAGE_SIZE     0x200000
#define HV_X64_LARGE_PAGE_SIZE_1GB 0x40000000
#define HV_PAGE_SIZE               HV_X64_PAGE_SIZE
#define HV_LARGE_PAGE_SIZE         HV_X64_LARGE_PAGE_SIZE
#define HV_LARGE_PAGE_SIZE_1GB     HV_X64_LARGE_PAGE_SIZE_1GB

typedef UINT64 HV_GVA_PAGE_NUMBER, *PHV_GVA_PAGE_NUMBER;
typedef UINT64 HV_PARTITION_ID, *PHV_PARTITION_ID;

//
// Define invalid partition identifier.
//
#define HV_PARTITION_ID_INVALID ((HV_PARTITION_ID) 0x0)

//
// Time in the hypervisor is measured in 100 nanosecond units
//
typedef UINT64 HV_NANO100_TIME,     *PHV_NANO100_TIME;
typedef UINT64 HV_NANO100_DURATION, *PHV_NANO100_DURATION;

//
// Versioning definitions used for guests reporting themselves to the
// hypervisor, and visa versa.
// ==================================================================
//

//
// Microsoft hypervisor interface signature.
//
typedef enum _HV_HYPERVISOR_INTERFACE
{
    HvMicrosoftHypervisorInterface = '1#vH'

} HV_HYPERVISOR_INTERFACE, *PHV_HYPERVISOR_INTERFACE;


//
// Version info reported by both guest OS's and hypervisors
//
typedef enum _HV_SERVICE_BRANCH
{
    //
    // [General Distribution Release (GDR) Branch]
    //
    // This branch extends main releases and service pack releases with
    // patches that are generally distributed and recommended to all customers,
    // such as critical fixes.
    //
    // Unmodified main releases and service pack releases are members of this
    // branch.
    //
    HvServiceBranchGdr = 0x00000000,

    //
    // [Quality Fix Engineering (QFE) Branch]
    //
    // This branch extends main releases and service pack releases with
    // patches that are not generally distributed to all customers, such as
    // feature enhancements.
    //
    HvServiceBranchQfe = 0x00000001

} HV_SERVICE_BRANCH, *PHV_SERVICE_BRANCH;

//
// Version info reported by guest OS's
//
typedef enum _HV_GUEST_OS_VENDOR
{
    HvGuestOsVendorMicrosoft        = 0x0001

} HV_GUEST_OS_VENDOR, *PHV_GUEST_OS_VENDOR;

typedef enum _HV_GUEST_OS_MICROSOFT_IDS
{
    HvGuestOsMicrosoftUndefined     = 0x00,
    HvGuestOsMicrosoftMSDOS         = 0x01,
    HvGuestOsMicrosoftWindows3x     = 0x02,
    HvGuestOsMicrosoftWindows9x     = 0x03,
    HvGuestOsMicrosoftWindowsNT     = 0x04,
    HvGuestOsMicrosoftWindowsCE     = 0x05

} HV_GUEST_OS_MICROSOFT_IDS, *PHV_GUEST_OS_MICROSOFT_IDS;

typedef enum _HV_GUEST_OS_OPENSOURCE_IDS
{
    HvGuestOsOpenSourceLinux        = 0x01,

} HV_GUEST_OS_OPENSOURCE_IDS, *PHV_GUEST_OS_OPENSOURCE_IDS;

//
// Declare the MSR used to identify the guest OS.
//
#define HV_X64_MSR_GUEST_OS_ID 0x40000000

typedef union _HV_X64_MSR_GUEST_OS_ID_CONTENTS
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 BuildNumber    : 16;
        UINT64 ServiceVersion : 8; // Service Pack, etc.
        UINT64 MinorVersion   : 8;
        UINT64 MajorVersion   : 8;
        UINT64 OsId           : 8; // HV_GUEST_OS_MICROSOFT_IDS (If Vendor=MS)
        UINT64 VendorId       : 16; // HV_GUEST_OS_VENDOR. We only support using
                                    // the least significant 15 bits, the top
                                    // bit must be zero. If 1, refers to the
                                    // open source format.
    };

    struct
    {
        UINT64 VendorSpecific1 : 16;
        UINT64 Version         : 32;
        UINT64 VendorSpecific2 : 8;
        UINT64 OsId            : 7;
        UINT64 IsOpenSource    : 1;
    } OpenSource;

} HV_X64_MSR_GUEST_OS_ID_CONTENTS, *PHV_X64_MSR_GUEST_OS_ID_CONTENTS;

//
// Version info reported by hypervisors
//
typedef struct _HV_HYPERVISOR_VERSION_INFO
{
    UINT32 BuildNumber;

    UINT32 MinorVersion:16;
    UINT32 MajorVersion:16;

    UINT32 ServicePack;

    UINT32 ServiceNumber:24;
    UINT32 ServiceBranch:8; // Type is HV_SERVICE_BRANCH

} HV_HYPERVISOR_VERSION_INFO, *PHV_HYPERVISOR_VERSION_INFO;

//
// Typedefs for CPUID leaves on HvMicrosoftHypercallInterface-supporting
// hypervisors.
// =====================================================================
//

typedef union _HV_PARTITION_PRIVILEGE_MASK
{
    UINT64 AsUINT64;
    struct
    {
        //
        // Access to virtual MSRs
        //
        UINT64  AccessVpRunTimeMsr:1;
        UINT64  AccessPartitionReferenceCounter:1;
        UINT64  AccessSynicMsrs:1;
        UINT64  AccessSyntheticTimerMsrs:1;
        UINT64  AccessApicMsrs:1;
        UINT64  AccessHypercallMsrs:1;
        UINT64  AccessVpIndex:1;
        UINT64  AccessResetMsr:1;
        UINT64  AccessStatsMsr:1;
        UINT64  AccessPartitionReferenceTsc:1;
        UINT64  AccessGuestIdleMsr:1;
        UINT64  AccessFrequencyMsrs:1;
        UINT64  AccessDebugMsrs:1;
        UINT64  Reserved1:19;

        //
        // Access to hypercalls
        //
        UINT64  CreatePartitions:1;
        UINT64  AccessPartitionId:1;
        UINT64  AccessMemoryPool:1;
        UINT64  AdjustMessageBuffers:1;
        UINT64  PostMessages:1;
        UINT64  SignalEvents:1;
        UINT64  CreatePort:1;
        UINT64  ConnectPort:1;
        UINT64  AccessStats:1;
        UINT64  Reserved2:2;
        UINT64  Debugging:1;
        UINT64  CpuManagement:1;
        UINT64  ConfigureProfiler:1;
        UINT64  EnableExpandedStackwalking:1;
        UINT64  EnableExtendedGvaRangesForFlushVirtualAddressList:1;
        UINT64  Reserved3:16;
    };

} HV_PARTITION_PRIVILEGE_MASK, *PHV_PARTITION_PRIVILEGE_MASK;

//
// The below CPUID leaves are present if VersionAndFeatures.HypervisorPresent
// is set by CPUID(HvCpuIdFunctionVersionAndFeatures).
// ==========================================================================
//

typedef enum _HV_CPUID_FUNCTION
{
    HvCpuIdFunctionVersionAndFeatures           = 0x00000001,
    HvCpuIdFunctionHvVendorAndMaxFunction       = 0x40000000,
    HvCpuIdFunctionHvInterface                  = 0x40000001,

    //
    // The remaining functions depend on the value of HvCpuIdFunctionInterface
    //
    HvCpuIdFunctionMsHvVersion                  = 0x40000002,
    HvCpuIdFunctionMsHvFeatures                 = 0x40000003,
    HvCpuIdFunctionMsHvEnlightenmentInformation = 0x40000004,
    HvCpuIdFunctionMsHvImplementationLimits     = 0x40000005,
    HvCpuIdFunctionMsHvHardwareFeatures         = 0x40000006,
    HvCpuIdFunctionMaxReserved                  = 0x40000006

} HV_CPUID_FUNCTION, *PHV_CPUID_FUNCTION;


//
// Hypervisor Vendor Info - HvCpuIdFunctionHvVendorAndMaxFunction Leaf
//

typedef struct _HV_VENDOR_AND_MAX_FUNCTION
{
    //
    // Eax
    //
    UINT32 MaxFunction;

    //
    // Ebx-Edx
    //
    UINT8 VendorName[12];

} HV_VENDOR_AND_MAX_FUNCTION, *PHV_VENDOR_AND_MAX_FUNCTION;


//
// Hypervisor Interface Info - HvCpuIdFunctionHvInterface Leaf
//

typedef struct _HV_HYPERVISOR_INTERFACE_INFO
{
    //
    // Eax
    //
    UINT32 Interface; // HV_HYPERVISOR_INTERFACE

    //
    // Ebx
    //
    UINT32 ReservedEbx;

    //
    // Ecx
    //
    UINT32 ReservedEcx;

    //
    // Edx
    //
    UINT32 ReservedEdx;

} HV_HYPERVISOR_INTERFACE_INFO, *PHV_HYPERVISOR_INTERFACE_INFO;


//
// Hypervisor Feature CPUID Information - HvCpuIdFunctionMsHvFeatures Leaf
//

typedef struct _HV_HYPERVISOR_FEATURES
{
    //
    // Eax-Ebx
    //
    HV_PARTITION_PRIVILEGE_MASK PartitionPrivileges;

    //
    // Ecx - this indicates the power configuration for the current VP.
    //
    UINT32 MaxSupportedCState:4;
    UINT32 HpetNeededForC3PowerState:1;
    UINT32 Reserved:27;

    //
    // Edx
    //
    UINT32 MwaitAvailable:1;
    UINT32 GuestDebuggingAvailable:1;
    UINT32 PerformanceMonitorsAvailable:1;
    UINT32 CpuDynamicPartitioningAvailable:1;
    UINT32 XmmRegistersForFastHypercallAvailable:1;
    UINT32 GuestIdleAvailable:1;
    UINT32 HypervisorSleepStateSupportAvailable:1;
    UINT32 NumaDistanceQueryAvailable:1;
    UINT32 FrequencyMsrsAvailable:1;
    UINT32 SyntheticMachineCheckAvailable:1;
    UINT32 GuestCrashMsrsAvailable:1;
    UINT32 DebugMsrsAvailable:1;
    UINT32 Npiep1Available:1;
    UINT32 DisableHypervisorAvailable:1;
    UINT32 ExtendedGvaRangesForFlushVirtualAddressListAvailable:1;
    UINT32 Reserved1:17;

} HV_HYPERVISOR_FEATURES, *PHV_HYPERVISOR_FEATURES;


//
// Enlightenment Info - HvCpuIdFunctionMsHvEnlightenmentInformation Leaf
//

typedef struct _HV_ENLIGHTENMENT_INFORMATION
{
    //
    // Eax
    //
    UINT32 UseHypercallForAddressSpaceSwitch:1;
    UINT32 UseHypercallForLocalFlush:1;
    UINT32 UseHypercallForRemoteFlush:1;
    UINT32 UseApicMsrs:1;
    UINT32 UseMsrForReset:1;
    UINT32 UseRelaxedTiming:1;
    UINT32 UseDmaRemapping:1;
    UINT32 UseInterruptRemapping:1;
    UINT32 UseX2ApicMsrs:1;
    UINT32 DeprecateAutoEoi:1;
    UINT32 UseSyntheticClusterIpi:1;
    UINT32 Reserved:21;

    //
    // Ebx
    //
    UINT32 LongSpinWaitCount;

    //
    // Ecx
    //
    UINT32 ReservedEcx;

    //
    // Edx
    //
    UINT32 ReservedEdx;

} HV_ENLIGHTENMENT_INFORMATION, *PHV_ENLIGHTENMENT_INFORMATION;


//
// Implementation Limits - HvCpuIdFunctionMsHvImplementationLimits Leaf
//

typedef struct _HV_IMPLEMENTATION_LIMITS
{
    //
    // Eax
    //
    UINT32 MaxVirtualProcessorCount;

    //
    // Ebx
    //
    UINT32 MaxLogicalProcessorCount;

    //
    // Ecx
    //
    UINT32 MaxInterruptMappingCount;

    //
    // Edx
    //
    UINT32 ReservedEdx;

} HV_IMPLEMENTATION_LIMITS, *PHV_IMPLEMENTATION_LIMITS;


//
// Hypervisor Hardware Features Info - HvCpuIdFunctionMsHvHardwareFeatures Leaf
//

typedef struct _HV_HYPERVISOR_HARDWARE_FEATURES
{
    //
    // Eax
    //
    UINT32 ApicOverlayAssistInUse:1;
    UINT32 MsrBitmapsInUse:1;
    UINT32 ArchitecturalPerformanceCountersInUse:1;
    UINT32 SecondLevelAddressTranslationInUse:1;
    UINT32 DmaRemappingInUse:1;
    UINT32 InterruptRemappingInUse:1;
    UINT32 MemoryPatrolScrubberPresent:1;
    UINT32 Reserved:25;

    //
    // Ebx
    //
    UINT32 ReservedEbx;

    //
    // Ecx
    //
    UINT32 ReservedEcx;

    //
    // Edx
    //
    UINT32 ReservedEdx;

} HV_HYPERVISOR_HARDWARE_FEATURES, *PHV_HYPERVISOR_HARDWARE_FEATURES;


//
// Typedefs for CPUID leaves on HvMicrosoftHypercallInterface-supporting
// hypervisors.
// =====================================================================
//

typedef union _HV_CPUID_RESULT
{
    struct
    {
        UINT32 Eax;
        UINT32 Ebx;
        UINT32 Ecx;
        UINT32 Edx;
    };

    UINT32 AsUINT32[4];

    struct
    {
        //
        // Eax
        //
        UINT32 ReservedEax;

        //
        // Ebx
        //
        UINT32 ReservedEbx:24;
        UINT32 InitialApicId:8;

        //
        // Ecx
        //
        UINT32 ReservedEcx:31;
        UINT32 HypervisorPresent:1;

        //
        // Edx
        //
        UINT32 ReservedEdx;

    } VersionAndFeatures;

    HV_VENDOR_AND_MAX_FUNCTION HvVendorAndMaxFunction;

    HV_HYPERVISOR_INTERFACE_INFO HvInterface;

    //
    // Eax-Edx.
    //
    HV_HYPERVISOR_VERSION_INFO MsHvVersion;

    HV_HYPERVISOR_FEATURES MsHvFeatures;

    HV_ENLIGHTENMENT_INFORMATION MsHvEnlightenmentInformation;

    HV_IMPLEMENTATION_LIMITS MsHvImplementationLimits;

    HV_HYPERVISOR_HARDWARE_FEATURES MsHvHardwareFeatures;

} HV_CPUID_RESULT, *PHV_CPUID_RESULT;

//
// Declare the MSR used to setup pages used to communicate with the hypervisor.
//
#define HV_X64_MSR_HYPERCALL 0x40000001

typedef union _HV_X64_MSR_HYPERCALL_CONTENTS
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Enable               : 1;
        UINT64 ReservedP            : 11;
        UINT64 GpaPageNumber        : 52;
    };
} HV_X64_MSR_HYPERCALL_CONTENTS, *PHV_X64_MSR_HYPERCALL_CONTENTS;

//
// Virtual Processor Indices
//
typedef UINT32 HV_VP_INDEX, *PHV_VP_INDEX;

#define HV_MAX_VP_INDEX (63)
#define HV_VP_INDEX_CURRENT_VP ((UINT32)-1)

//
// Declare the MSR for determining the current VP index.
//
#define HV_X64_MSR_VP_INDEX             (0x40000002)
#define HV_X64_MSR_RESET                (0x40000003)

//
// Declare the MSR used to reset partition
//

typedef union _HV_X64_MSR_RESET_CONTENTS
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Reset        :1;
        UINT64 ReservedZ    :63;
    };
} HV_X64_MSR_RESET_CONTENTS, *PHV_X64_MSR_RESET_CONTENTS;

#define HV_X64_MSR_EOI                  (0x40000070)
#define HV_X64_MSR_ICR                  (0x40000071)
#define HV_X64_MSR_TPR                  (0x40000072)
#define HV_X64_MSR_APIC_ASSIST_PAGE     (0x40000073)

#define HV_VIRTUAL_APIC_NO_EOI_REQUIRED 0x0

typedef struct _HV_VIRTUAL_APIC_ASSIST
{
    union
    {
        INT32 ApicFlags;
        struct
        {
            UINT32 NoEOIRequired : 1;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;
} HV_VIRTUAL_APIC_ASSIST, *PHV_VIRTUAL_APIC_ASSIST;

//
// Declare the various hypercall operations.
//
typedef enum _HV_CALL_CODE
{
    //
    // Reserved Feature Code
    //

    HvCallReserved0000                  = 0x0000,

    //
    // V1 Address space enlightment IDs
    //

    HvCallSwitchVirtualAddressSpace     = 0x0001,
    HvCallFlushVirtualAddressSpace      = 0x0002,
    HvCallFlushVirtualAddressList       = 0x0003,

    //
    // V1 Power Management and Run time metrics IDs
    //

    HvCallGetLogicalProcessorRunTime    = 0x0004,
    HvCallDeprecated0005                = 0x0005,
    HvCallDeprecated0006                = 0x0006,
    HvCallDeprecated0007                = 0x0007,

    //
    // V1 Spinwait enlightenment IDs
    //

    HvCallNotifyLongSpinWait            = 0x0008,

    //
    // V2 Core parking IDs.
    // Reused Hypercall code: Previously was
    // HvCallParkLogicalProcessors.
    //

    HvCallParkedVirtualProcessors       = 0x0009,

    //
    // V2 Invoke Hypervisor debugger
    //

    HvCallInvokeHypervisorDebugger      = 0x000a,

    //
    // V4 Send Synthetic Cluster Ipi
    //

    HvCallSendSyntheticClusterIpi       = 0x000b,

    //
    // V1 enlightenment name space reservation.
    //

    HvCallReserved000c                  = 0x000c,
    HvCallReserved000d                  = 0x000d,
    HvCallReserved000e                  = 0x000e,
    HvCallReserved000f                  = 0x000f,
    HvCallReserved0010                  = 0x0010,
    HvCallReserved0011                  = 0x0011,
    HvCallReserved0012                  = 0x0012,
    HvCallReserved0013                  = 0x0013,
    HvCallReserved0014                  = 0x0014,
    HvCallReserved0015                  = 0x0015,
    HvCallReserved0016                  = 0x0016,
    HvCallReserved0017                  = 0x0017,
    HvCallReserved0018                  = 0x0018,
    HvCallReserved0019                  = 0x0019,
    HvCallReserved001a                  = 0x001a,
    HvCallReserved001b                  = 0x001b,
    HvCallReserved001c                  = 0x001c,
    HvCallReserved001d                  = 0x001d,
    HvCallReserved001e                  = 0x001e,
    HvCallReserved001f                  = 0x001f,
    HvCallReserved0020                  = 0x0020,
    HvCallReserved0021                  = 0x0021,
    HvCallReserved0022                  = 0x0022,
    HvCallReserved0023                  = 0x0023,
    HvCallReserved0024                  = 0x0024,
    HvCallReserved0025                  = 0x0025,
    HvCallReserved0026                  = 0x0026,
    HvCallReserved0027                  = 0x0027,
    HvCallReserved0028                  = 0x0028,
    HvCallReserved0029                  = 0x0029,
    HvCallReserved002a                  = 0x002a,
    HvCallReserved002b                  = 0x002b,
    HvCallReserved002c                  = 0x002c,
    HvCallReserved002d                  = 0x002d,
    HvCallReserved002e                  = 0x002e,
    HvCallReserved002f                  = 0x002f,
    HvCallReserved0030                  = 0x0030,
    HvCallReserved0031                  = 0x0031,
    HvCallReserved0032                  = 0x0032,
    HvCallReserved0033                  = 0x0033,
    HvCallReserved0034                  = 0x0034,
    HvCallReserved0035                  = 0x0035,
    HvCallReserved0036                  = 0x0036,
    HvCallReserved0037                  = 0x0037,
    HvCallReserved0038                  = 0x0038,
    HvCallReserved0039                  = 0x0039,
    HvCallReserved003a                  = 0x003a,
    HvCallReserved003b                  = 0x003b,
    HvCallReserved003c                  = 0x003c,
    HvCallReserved003d                  = 0x003d,
    HvCallReserved003e                  = 0x003e,
    HvCallReserved003f                  = 0x003f,

    //
    // V1 Partition Management IDs
    //

    HvCallCreatePartition               = 0x0040,
    HvCallInitializePartition           = 0x0041,
    HvCallFinalizePartition             = 0x0042,
    HvCallDeletePartition               = 0x0043,
    HvCallGetPartitionProperty          = 0x0044,
    HvCallSetPartitionProperty          = 0x0045,
    HvCallGetPartitionId                = 0x0046,
    HvCallGetNextChildPartition         = 0x0047,

    //
    // V1 Resource Management IDs
    //

    HvCallDepositMemory                 = 0x0048,
    HvCallWithdrawMemory                = 0x0049,
    HvCallGetMemoryBalance              = 0x004a,

    //
    // V1 Guest Physical Address Space Management IDs
    //

    HvCallMapGpaPages                   = 0x004b,
    HvCallUnmapGpaPages                 = 0x004c,

    //
    // V1 Intercept Management IDs
    //

    HvCallInstallIntercept              = 0x004d,

    //
    // V1 Virtual Processor Management IDs
    //

    HvCallCreateVp                      = 0x004e,
    HvCallDeleteVp                      = 0x004f,
    HvCallGetVpRegisters                = 0x0050,
    HvCallSetVpRegisters                = 0x0051,

    //
    // V1 Virtual TLB IDs
    //

    HvCallTranslateVirtualAddress       = 0x0052,
    HvCallReadGpa                       = 0x0053,
    HvCallWriteGpa                      = 0x0054,

    //
    // V1 Interrupt Management IDs
    //

    HvCallAssertVirtualInterrupt        = 0x0055,
    HvCallClearVirtualInterrupt         = 0x0056,

    //
    // V1 Port IDs
    //

    HvCallCreatePort                    = 0x0057,
    HvCallDeletePort                    = 0x0058,
    HvCallConnectPort                   = 0x0059,
    HvCallGetPortProperty               = 0x005a,
    HvCallDisconnectPort                = 0x005b,
    HvCallPostMessage                   = 0x005c,
    HvCallSignalEvent                   = 0x005d,

    //
    // V1 Partition State IDs
    //

    HvCallSavePartitionState            = 0x005e,
    HvCallRestorePartitionState         = 0x005f,

    //
    // V1 Trace IDs
    //

    HvCallInitializeEventLogBufferGroup = 0x0060,
    HvCallFinalizeEventLogBufferGroup   = 0x0061,
    HvCallCreateEventLogBuffer          = 0x0062,
    HvCallDeleteEventLogBuffer          = 0x0063,
    HvCallMapEventLogBuffer             = 0x0064,
    HvCallUnmapEventLogBuffer           = 0x0065,
    HvCallSetEventLogGroupSources       = 0x0066,
    HvCallReleaseEventLogBuffer         = 0x0067,
    HvCallFlushEventLogBuffer           = 0x0068,

    //
    // V1 Dbg Call IDs
    //

    HvCallPostDebugData                 = 0x0069,
    HvCallRetrieveDebugData             = 0x006a,
    HvCallResetDebugSession             = 0x006b,

    //
    // V1 Stats IDs
    //

    HvCallMapStatsPage                  = 0x006c,
    HvCallUnmapStatsPage                = 0x006d,

    //
    // V2 Guest Physical Address Space Management IDs
    //

    HvCallMapSparseGpaPages             = 0x006e,

    //
    // V2 Set System Property
    //

    HvCallSetSystemProperty             = 0x006f,

    //
    // V2 Port Ids.
    //

    HvCallSetPortProperty               = 0x0070,

    //
    // V2 Test IDs
    //

    HvCallOutputDebugCharacter          = 0x0071,
    HvCallEchoIncrement                 = 0x0072,

    //
    // V2 Performance IDs
    //

    HvCallPerfNop                       = 0x0073,
    HvCallPerfNopInput                  = 0x0074,
    HvCallPerfNopOutput                 = 0x0075,

    //
    // V3 Logical Processor Management IDs
    //

    HvCallAddLogicalProcessor           = 0x0076,
    HvCallRemoveLogicalProcessor        = 0x0077,

    HvCallQueryNumaDistance             = 0x0078,

    HvCallSetLogicalProcessorProperty   = 0x0079,
    HvCallGetLogicalProcessorProperty   = 0x007a,

    //
    // V3 Get System Property
    //

    HvCallGetSystemProperty             = 0x007b,

    //
    // V3 IOMMU Hypercall IDs
    //

    HvCallMapDeviceInterrupt            = 0x007c,
    HvCallUnmapDeviceInterrupt          = 0x007d,
    HvCallReserved007e                  = 0x007e,
    HvCallReserved007f                  = 0x007f,
    HvCallReserved0080                  = 0x0080,
    HvCallReserved0081                  = 0x0081,
    HvCallAttachDevice                  = 0x0082,
    HvCallDetachDevice                  = 0x0083,

    //
    // V3 Sleep state transition hypercall
    //

    HvCallEnterSleepState              = 0x0084,
    HvCallPrepareForSleep              = 0x0085,
    HvCallPrepareForHibernate          = 0x0086,

    HvCallNotifyPartitionEvent         = 0x0087,

    //
    // V3 Logical Processor State IDs
    //

    HvCallGetLogicalProcessorRegisters  = 0x0088,
    HvCallSetLogicalProcessorRegisters  = 0x0089,

    //
    // V3 MCA specific Hypercall IDs
    //

    HvCallQueryAssociatedLpsForMca  = 0x008A,

    //
    // V3 Event ring flush.
    //

    HvCallNotifyPortRingEmpty = 0x008B,

    //
    // V3 Synthetic machine check injection
    //

    HvCallInjectSyntheticMachineCheck = 0x008C,

    //
    // V4 Parition Management IDs
    //

    HvCallScrubPartition = 0x008D,

    //
    // V4 Debugger and livedump hypercalls.
    //

    HvCallCollectLivedump = 0x008E,

    //
    // V4 Turn off virtualization.
    //

    HvCallDisableHypervisor = 0x008F,

    //
    // V4 Guest Physical Address Space Management IDs
    //

    HvCallModifySparseGpaPages = 0x0090,

    //
    // V4 Intercept result registration hypercalls.
    //

    HvCallRegisterInterceptResult = 0x0091,
    HvCallUnregisterInterceptResult = 0x0092,

    //
    // V5 Test Only Coverage Hypercall
    //
    HvCallGetCoverageData = 0x0093,

    //
    // Total of all hypercalls
    //
    HvCallCount

} HV_CALL_CODE, *PHV_CALL_CODE;

//
// Declare constants and structures for submitting hypercalls.
//
#define HV_X64_MAX_HYPERCALL_ELEMENTS ((1<<12) - 1)

typedef union _HV_X64_HYPERCALL_INPUT
{
    //
    // Input: The call code, argument sizes and calling convention
    //
    struct
    {
        UINT32 CallCode        : 16; // Least significant bits
        UINT32 IsFast          : 1;  // Uses the register based form
        UINT32 Reserved1       : 15;
        UINT32 CountOfElements : 12;
        UINT32 Reserved2       : 4;
        UINT32 RepStartIndex   : 12;
        UINT32 Reserved3       : 4;  // Most significant bits
    };
    UINT64 AsUINT64;

} HV_X64_HYPERCALL_INPUT, *PHV_X64_HYPERCALL_INPUT;

typedef union _HV_X64_HYPERCALL_OUTPUT
{
    //
    // Output: The result and returned data size
    //
    struct
    {
        UINT16 CallStatus;             // Least significant bits
        UINT16 Reserved1;
        UINT32 ElementsProcessed : 12;
        UINT32 Reserved2         : 20; // Most significant bits
    };
    UINT64 AsUINT64;

} HV_X64_HYPERCALL_OUTPUT, *PHV_X64_HYPERCALL_OUTPUT;

//
// Address spaces presented by the guest.
//
typedef UINT64 HV_ADDRESS_SPACE_ID, *PHV_ADDRESS_SPACE_ID;

//
// Address space flush flags.
//
typedef UINT64 HV_FLUSH_FLAGS, *PHV_FLUSH_FLAGS;

#define HV_FLUSH_ALL_PROCESSORS              (0x00000001)
#define HV_FLUSH_ALL_VIRTUAL_ADDRESS_SPACES  (0x00000002)
#define HV_FLUSH_NON_GLOBAL_MAPPINGS_ONLY    (0x00000004)
#define HV_FLUSH_USE_EXTENDED_RANGE_FORMAT   (0x00000008)
#define HV_FLUSH_MASK                        (HV_FLUSH_ALL_PROCESSORS | \
                                              HV_FLUSH_ALL_VIRTUAL_ADDRESS_SPACES | \
                                              HV_FLUSH_NON_GLOBAL_MAPPINGS_ONLY | \
                                              HV_FLUSH_USE_EXTENDED_RANGE_FORMAT)

//
// Common header used by both list and space flush routines.
//

typedef struct _HV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE_HEADER
{
    HV_ADDRESS_SPACE_ID AddressSpace;
    HV_FLUSH_FLAGS      Flags;
    UINT64              ProcessorMask;
} HV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE_HEADER,
  *PHV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE_HEADER;

//
// Definition of the HvCallFlushVirtualAddressSpace hypercall input
// structure.  This call flushes the virtual TLB entries which belong
// to the indicated address space, on one or more processors.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE
{
    HV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE_HEADER Header;
} HV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE, *PHV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE;

//
// Definition of the HvCallFlushVirtualAddressList hypercall input
// structure.  This call invalidates portions of the virtual TLB which
// belong to the indicates address space, on one more more processors.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_FLUSH_VIRTUAL_ADDRESS_LIST
{
    HV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE_HEADER Header;
    HV_CALL_ATTRIBUTES HV_GVA GvaList[];
} HV_INPUT_FLUSH_VIRTUAL_ADDRESS_LIST, *PHV_INPUT_FLUSH_VIRTUAL_ADDRESS_LIST;

//
// Declare the output structure for the HvGetPartitionId hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_PARTITION_ID
{
    HV_PARTITION_ID             PartitionId;
} HV_OUTPUT_GET_PARTITION_ID, *PHV_OUTPUT_GET_PARTITION_ID;

//
// Definition of the HvCallSendSyntheticClusterIpi hypercall input structure.
// This call sends a fixed virtual interrupt to a synthetic cluster specified as
// a processor mask.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SEND_SYNTHETIC_CLUSTER_IPI
{
    UINT32 Vector;
    UINT32 RsvdZ;
    UINT64 ProcessorMask;
} HV_INPUT_SEND_SYNTHETIC_CLUSTER_IPI, *PHV_INPUT_SEND_SYNTHETIC_CLUSTER_IPI;

//
// Define guest idle MSR. A guest virtual processor can enter idle state by
// reading this MSR, and will be woken up when an interrupt arrives
// regarldess interrupt is enabled or not.
//

#define HV_X64_MSR_GUEST_IDLE               0x400000F0

//
// Define index of the reference time MSR.
//

#define HV_X64_MSR_TIME_REF_COUNT      (0x40000020)
#define HV_X64_MSR_STIMER0_CONFIG      (0x400000b0)
#define HV_X64_MSR_STIMER0_COUNT       (0x400000b1)
#define HV_X64_MSR_STIMER1_CONFIG      (0x400000b2)
#define HV_X64_MSR_STIMER1_COUNT       (0x400000b3)
#define HV_X64_MSR_STIMER2_CONFIG      (0x400000b4)
#define HV_X64_MSR_STIMER2_COUNT       (0x400000b5)
#define HV_X64_MSR_STIMER3_CONFIG      (0x400000b6)
#define HV_X64_MSR_STIMER3_COUNT       (0x400000b7)

//
// Define index of the reference TSC page MSR.
//

#define HV_X64_MSR_REFERENCE_TSC       (0x40000021)

//
// Define index of the TSC and APIC frequency MSRs.
//

#define HV_X64_MSR_TSC_FREQUENCY       (0x40000022)
#define HV_X64_MSR_APIC_FREQUENCY      (0x40000023)

//
// Define contents of the reference TSC MSR.
//

typedef union _HV_X64_MSR_REFERENCE_TSC_CONTENTS
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Enable                   : 1;
        UINT64 ReservedP                : 11;
        UINT64 GpaPageNumber            : 52;
    };
} HV_X64_MSR_REFERENCE_TSC_CONTENTS, *PHV_X64_MSR_REFERENCE_TSC_CONTENTS;


//
// Define invalid and maximum values of the reference TSC sequence.
//

#define HV_REFERENCE_TSC_SEQUENCE_INVALID   (0x00000000)

//
// Define structure of the reference TSC page.
//

typedef struct _HV_REFERENCE_TSC_PAGE
{
    volatile UINT32 TscSequence;
    UINT32 Reserved1;
    volatile UINT64 TscScale;
    volatile INT64 TscOffset;
    UINT64 Reserved2[509];
} HV_REFERENCE_TSC_PAGE, *PHV_REFERENCE_TSC_PAGE;

//
// Define the synthetic interrupt source index type.
//

typedef UINT32 HV_SYNIC_SINT_INDEX, *PHV_SYNIC_SINT_INDEX;

//
// Define index of synthetic interrupt source that receives intercept messages.
//

#define HV_SYNIC_INTERCEPTION_SINT_INDEX ((HV_SYNIC_SINT_INDEX)0)
#define HV_SYNIC_SHARED_SINT_INDEX       ((HV_SYNIC_SINT_INDEX)1)

//
// Define interrupt vector type.
//
typedef UINT32 HV_INTERRUPT_VECTOR, *PHV_INTERRUPT_VECTOR;

//
// Define special "no interrupt vector" value used by hypercalls that indicate
// whether the previous virtual interrupt was acknowledged.
//
#define HV_INTERRUPT_VECTOR_NONE 0xFFFFFFFF

//
// Define interrupt types.
//
typedef enum _HV_INTERRUPT_TYPE
{
    //
    // Explicit interrupt types.
    //
    HvX64InterruptTypeFixed             = 0x0000,
    HvX64InterruptTypeLowestPriority    = 0x0001,
    HvX64InterruptTypeSmi               = 0x0002,
    HvX64InterruptTypeNmi               = 0x0004,
    HvX64InterruptTypeInit              = 0x0005,
    HvX64InterruptTypeSipi              = 0x0006,
    HvX64InterruptTypeExtInt            = 0x0007,

    //
    // Maximum (exclusive) value of interrupt type.
    //
    HvX64InterruptTypeMaximum           = 0x008

} HV_INTERRUPT_TYPE, *PHV_INTERRUPT_TYPE;

typedef enum _HV_INTERRUPT_DESTINATION_MODE
{
    HvX64InterruptDestinationPhysical       = 0x0000,
    HvX64InterruptDestinationLogical        = 0x0001,
    HvX64InterruptDestinationLogicalFlat    = 0x0002,
    HvX64InterruptDestinationProcessorMask  = 0x0003

} HV_INTERRUPT_DESTINATION_MODE, *PHV_INTERRUPT_DESTINATION_MODE;

//
// Define Interrupt Trigger modes.
//
typedef enum _HV_INTERRUPT_TRIGGER_MODE
{
    HvX64InterruptTriggerModeEdge       = 0x0000,
    HvX64InterruptTriggerModeLevel      = 0x0001

} HV_INTERRUPT_TRIGGER_MODE, *PHV_INTERRUPT_TRIGGER_MODE;

//
// Define synthetic interrupt source.
//

typedef union _HV_SYNIC_SINT
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Vector       :8;
        UINT64 ReservedP1   :8;
        UINT64 Masked       :1;
        UINT64 AutoEoi      :1;
        UINT64 ReservedP2   :46;
    };
} HV_SYNIC_SINT, *PHV_SYNIC_SINT;

//
// Define version of the synthetic interrupt controller.
//

#define HV_SYNIC_VERSION        (1)


//
// Define synthetic interrupt controller model specific registers.
//

#define HV_X64_MSR_SCONTROL   (0x40000080)
#define HV_X64_MSR_SVERSION   (0x40000081)
#define HV_X64_MSR_SIEFP      (0x40000082)
#define HV_X64_MSR_SIMP       (0x40000083)
#define HV_X64_MSR_EOM        (0x40000084)
#define HV_X64_MSR_SIRBP      (0x40000085)
#define HV_X64_MSR_SINT0      (0x40000090)
#define HV_X64_MSR_SINT1      (0x40000091)
#define HV_X64_MSR_SINT2      (0x40000092)
#define HV_X64_MSR_SINT3      (0x40000093)
#define HV_X64_MSR_SINT4      (0x40000094)
#define HV_X64_MSR_SINT5      (0x40000095)
#define HV_X64_MSR_SINT6      (0x40000096)
#define HV_X64_MSR_SINT7      (0x40000097)
#define HV_X64_MSR_SINT8      (0x40000098)
#define HV_X64_MSR_SINT9      (0x40000099)
#define HV_X64_MSR_SINT10     (0x4000009A)
#define HV_X64_MSR_SINT11     (0x4000009B)
#define HV_X64_MSR_SINT12     (0x4000009C)
#define HV_X64_MSR_SINT13     (0x4000009D)
#define HV_X64_MSR_SINT14     (0x4000009E)
#define HV_X64_MSR_SINT15     (0x4000009F)

//
// Define the expected SynIC version.
//
#define HV_SYNIC_VERSION_1 (0x1)

//
// An architecture is a set of processor instruction sets and operating modes
//

typedef enum _HV_ARCHITECTURE
{
    HvArchitectureX64,
    HvArchitectureX86,
    HvArchitectureMaximum
} HV_ARCHITECTURE, *PHV_ARCHITECTURE;

typedef struct _HV_X64_SEGMENT_REGISTER
{
    UINT64 Base;
    UINT32 Limit;
    UINT16 Selector;
    union
    {
        struct
        {
            UINT16 SegmentType:4;
            UINT16 NonSystemSegment:1;
            UINT16 DescriptorPrivilegeLevel:2;
            UINT16 Present:1;
            UINT16 Reserved:4;
            UINT16 Available:1;
            UINT16 Long:1;
            UINT16 Default:1;
            UINT16 Granularity:1;
        };
        UINT16 Attributes;
    };

} HV_X64_SEGMENT_REGISTER, *PHV_X64_SEGMENT_REGISTER;

typedef struct _HV_X64_TABLE_REGISTER
{
    UINT16     Pad[3];
    UINT16     Limit;
    UINT64     Base;
} HV_X64_TABLE_REGISTER, *PHV_X64_TABLE_REGISTER;

//
// XSAVE area definitions.
//

//
// The XSAVE XFEM (XSAVE Feature Enabled Mask) register.
//
typedef union _HV_X64_XSAVE_XFEM_REGISTER
{
    UINT64 AsUINT64;

    struct
    {
        UINT32 LowUINT32;
        UINT32 HighUINT32;
    };

    struct
    {
        UINT64 LegacyX87:1;
        UINT64 LegacySse:1;
        UINT64 Avx:1;
        UINT64 Reserved:61;
    };

} HV_X64_XSAVE_XFEM_REGISTER, *PHV_X64_XSAVE_XFEM_REGISTER;

//
// This structure represents the header area of an XSAVE area.
// This must be aligned on a 64 byte boundary.
//
typedef struct DECLSPEC_ALIGN(64) _HV_X64_XSAVE_HEADER
{
    //
    // Bit vector indicating which features have state store in the XSAVE
    // area.
    //
    HV_X64_XSAVE_XFEM_REGISTER  XstateBv;          // Bit 63 MBZ
    UINT64  Reserved0MBZ;      // Must be 0.

    UINT16  RevisionID;
    UINT16  Reserved1MBZ;      // Must be 0.
    UINT32  Reserved2;
    UINT64  Reserved3;

    UINT64  Reserved4;
    UINT64  Reserved5;

    UINT64  Reserved6;
    UINT64  Reserved7;
} HV_X64_XSAVE_HEADER, *PHV_X64_XSAVE_HEADER;

//
// The FX Save Area is defined to be 512 bytes in size
//
#define HV_X64_FXSAVE_AREA_SIZE  512

typedef union _HV_X64_FP_CONTROL_STATUS_REGISTER
{
    HV_UINT128 AsUINT128;
    struct
    {
        UINT16 FpControl;
        UINT16 FpStatus;
        UINT8  FpTag;
        UINT8  IgnNe:1;
        UINT8  Reserved:7;
        UINT16 LastFpOp;
        union
        {
            // Long Mode
            UINT64 LastFpRip;
            // 32 Bit Mode
            struct
            {
                UINT32 LastFpEip;
                UINT16 LastFpCs;
            };
        };
    };
} HV_X64_FP_CONTROL_STATUS_REGISTER, *PHV_X64_FP_CONTROL_STATUS_REGISTER;

typedef union _HV_X64_XMM_CONTROL_STATUS_REGISTER
{
    HV_UINT128 AsUINT128;
    struct
    {
        union
        {
            // Long Mode
            UINT64 LastFpRdp;
            // 32 Bit Mode
            struct
            {
                UINT32 LastFpDp;
                UINT16 LastFpDs;
            };
        };
        UINT32 XmmStatusControl;
        UINT32 XmmStatusControlMask;
    };
} HV_X64_XMM_CONTROL_STATUS_REGISTER, *PHV_X64_XMM_CONTROL_STATUS_REGISTER;

typedef union _HV_X64_FP_REGISTER
{
    HV_UINT128 AsUINT128;
    struct
    {
        UINT64 Mantissa;
        UINT64 BiasedExponent:15;
        UINT64 Sign:1;
        UINT64 Reserved:48;
    };
} HV_X64_FP_REGISTER, *PHV_X64_FP_REGISTER;

typedef union _HV_X64_FP_MMX_REGISTER
{
    HV_UINT128          AsUINT128;
    HV_X64_FP_REGISTER  Fp;
    UINT64              Mmx;
} HV_X64_FP_MMX_REGISTER, *PHV_X64_FP_MMX_REGISTER;

//
// FX registers are legacy extended state registers managed
// by the FXSAVE and and FXRSTOR instructions. This includes
// leagacy FP and SSE registers.
//
typedef union DECLSPEC_ALIGN(16) _HV_X64_FX_REGISTERS
{
    struct
    {
        HV_X64_FP_CONTROL_STATUS_REGISTER   FpControlStatus;
        HV_X64_XMM_CONTROL_STATUS_REGISTER  XmmControlStatus;
        HV_X64_FP_MMX_REGISTER              FpMmx[8];
        HV_UINT128                          Xmm[16];
    };

    UINT8 FxSaveArea[HV_X64_FXSAVE_AREA_SIZE];

} HV_X64_FX_REGISTERS, *PHV_X64_FX_REGISTERS;

//
// This is the size of the legacy save area (512) plus the size of the
// XSAVE header (64) plus the size of the AVX context (16 128-bit
// registers.
//
#define HV_X64_XSAVE_AREA_HEADER_SIZE  64
#define HV_X64_XSAVE_AREA_AVX_SIZE     256
#define HV_X64_XSAVE_AREA_SIZE         (HV_X64_FXSAVE_AREA_SIZE + HV_X64_XSAVE_AREA_HEADER_SIZE + HV_X64_XSAVE_AREA_AVX_SIZE)

//
// This structure defines the format of the XSAVE save area, the area
// used to save and restore the context of processor extended state
// (including legacy FP and SSE state) by the XSAVE and XRSTOR instructions.
//
// N.B. The XSAVE header must be aligned on a 64 byte boundary. Therefore
// this structure must be 64 byte aligned,
//
typedef union DECLSPEC_ALIGN(64) _HV_X64_X_REGISTERS
{
    struct
    {
        HV_X64_FP_CONTROL_STATUS_REGISTER   FpControlStatus;
        HV_X64_XMM_CONTROL_STATUS_REGISTER  XmmControlStatus;
        HV_X64_FP_MMX_REGISTER              FpMmx[8];

        union
        {
            HV_UINT128                      Xmm[16];
            HV_UINT128                      YmmLow[16];
        };

        HV_UINT128                          Reserved[6];

        HV_X64_XSAVE_HEADER                 Header;

        //
        // AVX context: the upper 128 bits of the YMM registers. The
        // lower 128 bits overlay the XMM registers.
        //
        HV_UINT128                          YmmHigh[16];
    };

    UINT8 XSaveArea[HV_X64_XSAVE_AREA_SIZE];
} HV_X64_X_REGISTERS, *PHV_X64_X_REGISTERS;


//
// XSAVE save area - The size of the XSAVE save area will vary depending on the
// supported processor features on a system.  The save area is defined as the
// maximum possible size, but on a given system, the actual save area size may
// be smaller and only include a subset of the save area.
//
// The save area is always guaranteed to at least include space for the legacy
// floating point registers (LegacyFxRegisters).  Accessing any fields beyond
// that should be prefaced with validation of the size of the save area.
//
// N.B. The save area is cache-aligned.
//
typedef union _HV_X64_XSAVE_AREA
{
    HV_X64_FX_REGISTERS LegacyFxRegisters;
    HV_X64_X_REGISTERS XRegisters;
} HV_X64_XSAVE_AREA, *PHV_X64_XSAVE_AREA;

typedef struct _HV_X64_CONTEXT
{

    //
    // The Initial APIC ID pseudo register. This is the value returned
    // by CPUID.
    //
    UINT64 InitialApicId;

    //
    // 16 64 bit general purpose registers, instruction pointer and
    // flags
    //

    UINT64 Rax;
    UINT64 Rbx;
    UINT64 Rcx;
    UINT64 Rdx;
    UINT64 Rsi;
    UINT64 Rdi;
    UINT64 Rbp;
    UINT64 Rsp;
    UINT64 R8;
    UINT64 R9;
    UINT64 R10;
    UINT64 R11;
    UINT64 R12;
    UINT64 R13;
    UINT64 R14;
    UINT64 R15;

    UINT64 Rip;
    UINT64 Rflags;

    //
    // Control Registers - on 32 bit platforms the upper 32 bits are
    // ignored. Efer is actually an Msr but it acts as an extension to
    // Cr4 and as such is treated as a processor register. Cr8 is only
    // valid on 64 bit systems.
    //

    UINT64 Cr0;
    UINT64 Cr2;
    UINT64 Cr3;
    UINT64 Cr4;
    UINT64 Cr8;
    UINT64 Efer;

    //
    // XSAVE Control Registers - only on platforms that support the
    // XSAVE/XRSTOR feature.
    //

    //
    // XCR0 is XFEM, XSAVE Feature Enabled Mask.
    //

    UINT64 Xfem;

    //
    // Debug Registers - on 32 bit platforms the upper 32 bits are
    // ignored
    //

    UINT64 Dr0;
    UINT64 Dr1;
    UINT64 Dr2;
    UINT64 Dr3;
    UINT64 Dr6;
    UINT64 Dr7;

    //
    // Global and Interrupt Descriptor tables
    //

    HV_X64_TABLE_REGISTER Idtr;
    HV_X64_TABLE_REGISTER Gdtr;

    //
    // Segment selector registers together with their hidden state.
    //

    HV_X64_SEGMENT_REGISTER Cs;
    HV_X64_SEGMENT_REGISTER Ds;
    HV_X64_SEGMENT_REGISTER Es;
    HV_X64_SEGMENT_REGISTER Fs;
    HV_X64_SEGMENT_REGISTER Gs;
    HV_X64_SEGMENT_REGISTER Ss;
    HV_X64_SEGMENT_REGISTER Tr;
    HV_X64_SEGMENT_REGISTER Ldtr;

    //
    // MSRs needed for virtualization
    //

    UINT64 KernelGsBase;
    UINT64 Star;
    UINT64 Lstar;
    UINT64 Cstar;
    UINT64 Sfmask;
    UINT64 SysenterCs;
    UINT64 SysenterEip;
    UINT64 SysenterEsp;

    UINT64 MsrCrPat;

    //
    // Local APIC state.
    //

    UINT32 LocalApicId;
    UINT32 LocalApicVersion;
    UINT32 LocalApicLdr;
    UINT32 LocalApicDfr;
    UINT32 LocalApicSpurious;
    UINT32 LocalApicIcrLow;
    UINT32 LocalApicIcrHigh;
    UINT32 LocalApicIsr[8];
    UINT32 LocalApicTmr[8];
    UINT32 LocalApicLvtTimer;
    UINT32 LocalApicLvtPerfmon;
    UINT32 LocalApicLvtLint0;
    UINT32 LocalApicLvtLint1;
    UINT32 LocalApicCurrentCount;
    UINT32 LocalApicInitialCount;
    UINT32 LocalApicDivider;
    UINT64 LocalApicBaseMsr;

    union
    {
        //
        // x87 Floating point, MMX and XMM registers formatted as by
        // FXSAVE/FXSTOR.
        //

        HV_X64_FX_REGISTERS FxRegisters;

        //
        // x87 Floating point, MMX XMM and YMM registers formatted as by
        // XSAVE/XRSTOR.
        //
        // Only on platforms that support XSAVE/XRSTOR.
        //

        HV_X64_X_REGISTERS XRegisters;
    };

} HV_X64_CONTEXT, *PHV_X64_CONTEXT;

#define HV_VIRTUAL_PROCESSOR_REGISTERS_VERSION 1

typedef struct _HV_VP_CONTEXT
{
    //
    // The version of the HV_VP_CONTEXT structure
    //

    UINT32 Version;

    //
    // The architecture of these registers
    //

    HV_ARCHITECTURE Architecture;

    union
    {
        HV_X64_CONTEXT x64;
    };

} HV_VP_CONTEXT, *PHV_VP_CONTEXT;

//
// Define the synthetic timer configuration structure
//

typedef struct _HV_X64_MSR_STIMER_CONFIG_CONTENTS
{
    union
    {
        UINT64 AsUINT64;
        struct
        {
            UINT64 Enable       : 1;
            UINT64 Periodic     : 1;
            UINT64 Lazy         : 1;
            UINT64 AutoEnable   : 1;
            UINT64 ReservedZ1   :12;
            UINT64 SINTx        : 4;
            UINT64 ReservedZ2   :44;
        };
    };
} HV_X64_MSR_STIMER_CONFIG_CONTENTS, *PHV_X64_MSR_STIMER_CONFIG_CONTENTS;

//
// Define the number of synthetic interrupt sources.
//

#define HV_SYNIC_SINT_COUNT (16)
#define HV_SYNIC_STIMER_COUNT (4)

//
// Define port identifier type.
//

typedef union _HV_PORT_ID
{
    UINT32 AsUINT32;

    struct
    {
        UINT32 Id:24;
        UINT32 Reserved:8;
    };

} HV_PORT_ID, *PHV_PORT_ID;

//
// Define synthetic interrupt controller message constants.
//

#define HV_MESSAGE_SIZE                 (256)
#define HV_MESSAGE_PAYLOAD_BYTE_COUNT   (240)
#define HV_MESSAGE_PAYLOAD_QWORD_COUNT  (30)
#define HV_ANY_VP                       (0xFFFFFFFF)

//
// Define hypervisor message types.
//
typedef enum _HV_MESSAGE_TYPE
{
    HvMessageTypeNone = 0x00000000,

    //
    // Memory access messages.
    //
    HvMessageTypeUnmappedGpa = 0x80000000,
    HvMessageTypeGpaIntercept = 0x80000001,

    //
    // Timer notification messages.
    //
    HvMessageTimerExpired = 0x80000010,

    //
    // Error messages.
    //
    HvMessageTypeInvalidVpRegisterValue = 0x80000020,
    HvMessageTypeUnrecoverableException = 0x80000021,
    HvMessageTypeUnsupportedFeature = 0x80000022,
    HvMessageTypeTlbPageSizeMismatch = 0x80000023,

    //
    // Trace buffer complete messages.
    //
    HvMessageTypeEventLogBufferComplete = 0x80000040,

    //
    // Platform-specific processor intercept messages.
    //
    HvMessageTypeX64IoPortIntercept = 0x80010000,
    HvMessageTypeX64MsrIntercept = 0x80010001,
    HvMessageTypeX64CpuidIntercept = 0x80010002,
    HvMessageTypeX64ExceptionIntercept = 0x80010003,
    HvMessageTypeX64ApicEoi = 0x80010004,
    HvMessageTypeX64LegacyFpError = 0x80010005

} HV_MESSAGE_TYPE, *PHV_MESSAGE_TYPE;

//
// Define the format of the SIMP register
//

typedef union _HV_SYNIC_SIMP
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 SimpEnabled : 1;
        UINT64 Preserved   : 11;
        UINT64 BaseSimpGpa : 52;
    };
} HV_SYNIC_SIMP, *PHV_SYNIC_SIMP;

//
// Define the trace buffer index type.
//

typedef UINT32 HV_EVENTLOG_BUFFER_INDEX, *PHV_EVENTLOG_BUFFER_INDEX;

//
// Define all the trace buffer types.
//

typedef enum
{
    HvEventLogTypeGlobalSystemEvents = 0x00000000,
    HvEventLogTypeLocalDiagnostics   = 0x00000001,

    HvEventLogTypeMaximum            = 0x00000001,
} HV_EVENTLOG_TYPE;

//
// Define trace message header structure.
//

typedef struct _HV_EVENTLOG_MESSAGE_PAYLOAD
{

    HV_EVENTLOG_TYPE EventLogType;
    HV_EVENTLOG_BUFFER_INDEX BufferIndex;

} HV_EVENTLOG_MESSAGE_PAYLOAD, *PHV_EVENTLOG_MESSAGE_PAYLOAD;

//
// Define synthetic interrupt controller message flags.
//

typedef union _HV_MESSAGE_FLAGS
{
    UINT8 AsUINT8;
    struct
    {
        UINT8 MessagePending:1;
        UINT8 Reserved:7;
    };
} HV_MESSAGE_FLAGS, *PHV_MESSAGE_FLAGS;

//
// Define synthetic interrupt controller message header.
//

typedef struct _HV_MESSAGE_HEADER
{
    HV_MESSAGE_TYPE     MessageType;
    UINT8               PayloadSize;
    HV_MESSAGE_FLAGS    MessageFlags;
    UINT8               Reserved[2];
    union
    {
        HV_PARTITION_ID Sender;
        HV_PORT_ID      Port;
    };

} HV_MESSAGE_HEADER, *PHV_MESSAGE_HEADER;

//
// Define timer message payload structure.
//

typedef struct _HV_TIMER_MESSAGE_PAYLOAD
{
    UINT32          TimerIndex;
    UINT32          Reserved;
    HV_NANO100_TIME ExpirationTime;     // When the timer expired
    HV_NANO100_TIME DeliveryTime;       // When the message was delivered
} HV_TIMER_MESSAGE_PAYLOAD, *PHV_TIMER_MESSAGE_PAYLOAD;

//
// Define synthetic interrupt controller message format.
//

typedef struct _HV_MESSAGE
{
    HV_MESSAGE_HEADER Header;
    union
    {
        UINT64 Payload[HV_MESSAGE_PAYLOAD_QWORD_COUNT];
        HV_TIMER_MESSAGE_PAYLOAD TimerPayload;
        HV_EVENTLOG_MESSAGE_PAYLOAD TracePayload;
    };
} HV_MESSAGE, *PHV_MESSAGE;

//
// Define the synthetic interrupt message page layout.
//

typedef struct _HV_MESSAGE_PAGE
{
    volatile HV_MESSAGE SintMessage[HV_SYNIC_SINT_COUNT];
} HV_MESSAGE_PAGE, *PHV_MESSAGE_PAGE;

//
// Define Non-Privileged Instruction Execution Prevention register
//

#define HV_X64_MSR_NPIEP_CONFIG 0x40000040

typedef union _HV_X64_MSR_NPIEP_CONFIG_CONTENTS
{
    UINT64 AsUINT64;
    struct
    {
        //
        // These bits enable instruction execution prevention for specific
        // instructions.
        //

        UINT64 PreventSgdt:1;
        UINT64 PreventSidt:1;
        UINT64 PreventSldt:1;
        UINT64 PreventStr:1;

        //
        // The reserved bits must always be 0.
        //

        UINT64 Reserved:60;
    };
} HV_X64_MSR_NPIEP_CONFIG_CONTENTS, *PHV_X64_MSR_NPIEP_CONFIG_CONTENTS;

//
// Maximum size of the payload
//
#define HV_DEBUG_MAXIMUM_DATA_SIZE 4088

//
// Options flags for HvPostDebugData
//
#define HV_DEBUG_POST_LOOP                  0x00000001
#define HV_DEBUG_POST_RAW_PACKET            0x00000002

//
// Options flags for HvRetrieveDebugData
//
#define HV_DEBUG_RETRIEVE_LOOP              0x00000001
#define HV_DEBUG_RETRIEVE_TEST_ACTIVITY     0x00000002
#define HV_DEBUG_RETRIEVE_RAW_PACKET        0x00000004

//
// Maximum HvRetrieveDebugData timeout value (in NANO100)
//
#define HV_DEBUG_RETRIEVE_MAXIMUM_TIMEOUT   4000000

//
// Debug options for all calls
//
typedef UINT32 HV_DEBUG_OPTIONS;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_POST_DEBUG_DATA
{
    UINT32 Count;
    HV_DEBUG_OPTIONS Options;
    UINT8 Data[HV_DEBUG_MAXIMUM_DATA_SIZE];
} HV_INPUT_POST_DEBUG_DATA, *PHV_INPUT_POST_DEBUG_DATA;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_POST_DEBUG_DATA
{
    UINT32 PendingCount;
} HV_OUTPUT_POST_DEBUG_DATA, *PHV_OUTPUT_POST_DEBUG_DATA;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_RETRIEVE_DEBUG_DATA
{
    UINT32 Count;
    HV_DEBUG_OPTIONS Options;
    HV_NANO100_DURATION Timeout;
} HV_INPUT_RETRIEVE_DEBUG_DATA, *PHV_INPUT_RETRIEVE_DEBUG_DATA;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_RETRIEVE_DEBUG_DATA
{
    UINT32 RetrievedCount;
    UINT32 RemainingCount;
    UINT8 Data[HV_DEBUG_MAXIMUM_DATA_SIZE];
} HV_OUTPUT_RETRIEVE_DEBUG_DATA, *PHV_OUTPUT_RETRIEVE_DEBUG_DATA;

typedef struct _HV_DEBUG_NET_DATA
{
    UINT32 HostIp;
    UINT32 TargetIp;
    UINT16 HostPort;
    UINT16 TargetPort;
    UCHAR HostMac[6];
    UCHAR TargetMac[6];
} HV_DEBUG_NET_DATA, *PHV_DEBUG_NET_DATA;

//
// Options flags for HvResetDebugSession
//
#define HV_DEBUG_PURGE_INCOMING_DATA        0x00000001
#define HV_DEBUG_PURGE_OUTGOING_DATA        0x00000002
#define HV_DEBUG_RETRIEVE_DEBUG_CONFIG      0x00000004
#define HV_DEBUG_USE_RAW_PACKETS            0x00000008

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_RESET_DEBUG_SESSION
{
    HV_DEBUG_OPTIONS Options;
} HV_INPUT_RESET_DEBUG_SESSION, *PHV_INPUT_RESET_DEBUG_SESSION;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_RESET_DEBUG_SESSION
{
    HV_DEBUG_NET_DATA NetData;    
} HV_OUTPUT_RESET_DEBUG_SESSION, *PHV_OUTPUT_RESET_DEBUG_SESSION;

//
// Define guest crash enlightment interface
//

#define HV_X64_MSR_CRASH_P0                       0x40000100
#define HV_X64_MSR_CRASH_P1                       0x40000101
#define HV_X64_MSR_CRASH_P2                       0x40000102
#define HV_X64_MSR_CRASH_P3                       0x40000103
#define HV_X64_MSR_CRASH_P4                       0x40000104
#define HV_X64_MSR_CRASH_CTL                      0x40000105

#define HV_X64_MSR_CRASH_ADDR_TO_INDEX(_Addr)   ((_Addr) - HV_X64_MSR_CRASH_P0)
#define HV_X64_MSR_CRASH_INDEX_TO_ADDR(_Idx)    ((_Idx) + HV_X64_MSR_CRASH_P0)
#define HV_X64_MSR_CRASH_NUM_PARAMETER_REGS     5

typedef union _HV_X64_MSR_CRASH_CTL_CONTENTS
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Reserved       : 63; // Reserved bits
        UINT64 CrashNotify    : 1;  // Log contents of crash parameter MSR's
    };
} HV_X64_MSR_CRASH_CTL_CONTENTS, *PHV_X64_MSR_CRASH_CTL_CONTENTS;

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4200) /* nonstandard extension used : zero-sized array in struct/union */
#pragma warning(default:4214) /* nonstandard extension used : bit field types other then int */
#pragma warning(default:4324) /* structure was padded due to __declspec(align()) */
#endif

#endif //_HVGDK_MINI_
