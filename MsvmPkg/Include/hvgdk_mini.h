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
#pragma warning(disable:4201) // nameless struct/union
#pragma warning(disable:4214) // bit field types other than int
#pragma warning(disable:4324) // structure was padded due to __declspec(align())

//
// Define hypervisor constants.
//

#if defined(XBOX_SYSTEMOS)

#define HV_LNM_MAX_VPS_PER_PARTITION 8
#define HV_LNM_MAX_PARTITIONS 4

//
// Define the overlay region top, base, and size.
//

#define HV_OVERLAY_ADDRESS_TOP 0x0000002000000000UI64

#define HV_OVERLAY_REGION_SIZE 128

#define HV_OVERLAY_ADDRESS_BASE                                              \
    (HV_OVERLAY_ADDRESS_TOP - (HV_PAGE_SIZE * HV_OVERLAY_REGION_SIZE))

//
// Define the page number of each page type.
//

#define HV_OVERLAY_APIC_PAGE_NUMBER(p) (((p) * 3) + 0)
#define HV_OVERLAY_SIEF_PAGE_NUMBER(p) (((p) * 3) + 1)
#define HV_OVERLAY_SIM_PAGE_NUMBER(p) (((p) * 3) + 2)
#define HV_OVERLAY_TRIGGER_PAGE_NUMBER(p)                                    \
    ((HV_LNM_MAX_VPS_PER_PARTITION * 3) + p)

#define HV_OVERLAY_VM_STATS_PAGE_NUMBER                                      \
    ((HV_LNM_MAX_VPS_PER_PARTITION * 3) + HV_LNM_MAX_PARTITIONS)

//
// Define the total number of preallocated overlay pages.
//

#define HV_OVERLAY_TOTAL_PAGES_PREALLOCATED                                  \
    ((3 * HV_LNM_MAX_VPS_PER_PARTITION) + HV_LNM_MAX_PARTITIONS + 1)

//
// Define the per partition global TLB flush counter index.
//

#define HV_COUNTER_TLB_GLOBAL_FLUSH 11

#endif

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
    HV_UINT256  Low256;
    HV_UINT256  High256;

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
// MessageId: HV_STATUS_INVALID_HYPERCALL_CODE
//
// MessageText:
//
// The hypervisor does not support the operation because the specified hypercall code is not supported.
//
#define HV_STATUS_INVALID_HYPERCALL_CODE ((HV_STATUS)0x0002)

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
// MessageId: HV_STATUS_FIRMWARE_EXCEPTION
//
// MessageText:
//
// Architecture- or platform- specific firmware generated an exception
//
#define HV_STATUS_FIRMWARE_EXCEPTION    ((HV_STATUS)0x003F)

//
// MessageId: HV_STATUS_INVALID_DEVICE_ID
//
// MessageText:
//
// The supplied device ID is invalid.
//
#define HV_STATUS_INVALID_DEVICE_ID ((HV_STATUS)0x0057)

//
// MessageId: HV_STATUS_INVALID_DEVICE_STATE
//
// MessageText:
//
// The operation is not allowed in the current device state.
//
#define HV_STATUS_INVALID_DEVICE_STATE ((HV_STATUS)0x0058)

//
// MessageId: HV_STATUS_PENDING_PAGE_REQUESTS
//
// MessageText:
//
// The device had pending page requests which were discarded.
//
#define HV_STATUS_PENDING_PAGE_REQUESTS ((HV_STATUS)0x0059)

//
// MessageId: HV_STATUS_PAGE_REQUEST_INVALID
//
// MessageText:
//
// The supplied page request specifies a memory access that the guest does not
// have permissions to perform.
//
#define HV_STATUS_PAGE_REQUEST_INVALID ((HV_STATUS)0x0060)


//
// Memory Types
//
// Guest virtual addresses (GVAs) are used within the guest when it enables address
// translation and provides a valid guest page table.
//
// Guest physical addresses (GPAs) define the guest's view of physical memory.
// GPAs can be mapped to underlying SPAs. There is one guest physical address space per
// partition.
//
typedef UINT64 HV_GVA, *PHV_GVA;

typedef UINT64 HV_GPA, *PHV_GPA;
typedef UINT64 HV_GPA_PAGE_NUMBER, *PHV_GPA_PAGE_NUMBER;

typedef UINT64 HV_DEVICE_VA, *PHV_DEVICE_VA;

#if defined(_ARM64_) || defined(_ARM_)

#define HV_ARM64_PAGE_SIZE              4096
#define HV_ARM64_LARGE_PAGE_SIZE        0x200000
#define HV_ARM64_LARGE_PAGE_SIZE_1GB    0x40000000
#define HV_PAGE_SIZE                    HV_ARM64_PAGE_SIZE
#define HV_LARGE_PAGE_SIZE              HV_ARM64_LARGE_PAGE_SIZE
#define HV_LARGE_PAGE_SIZE_1GB          HV_ARM64_LARGE_PAGE_SIZE_1GB

#define HV_ARM64_HVC_IMM16              1
#define HV_ARM64_HVC_VTLENTRY_IMM16     2
#define HV_ARM64_HVC_VTLEXIT_IMM16      3
#define HV_ARM64_HVC_LAUNCH_IMM16       4
#define HV_ARM64_HVC_LAUNCH_SL_IMM16    5

//
// The HVC immediate below is handled by the Microvisor
// for GICv3 support in the absence of the full Hypervisor.
//
#define HV_ARM64_ENABLE_SRE             2

#elif defined(_AMD64_) || defined(_X86_)

#define HV_X64_PAGE_SIZE                4096
#define HV_X64_LARGE_PAGE_SIZE          0x200000
#define HV_X64_LARGE_PAGE_SIZE_1GB      0x40000000
#define HV_PAGE_SIZE                    HV_X64_PAGE_SIZE
#define HV_LARGE_PAGE_SIZE              HV_X64_LARGE_PAGE_SIZE
#define HV_LARGE_PAGE_SIZE_1GB          HV_X64_LARGE_PAGE_SIZE_1GB

#else

#error Unknown/Unsupported architecture

#endif

typedef UINT64 HV_GVA_PAGE_NUMBER, *PHV_GVA_PAGE_NUMBER;
typedef UINT64 HV_PARTITION_ID, *PHV_PARTITION_ID;

//
// Define invalid partition identifier and "self" partition identifier
//
#define HV_PARTITION_ID_INVALID ((HV_PARTITION_ID) 0x0)
#define HV_PARTITION_ID_SELF    ((HV_PARTITION_ID) -1)

//
// Time in the hypervisor is measured in 100 nanosecond units
//
typedef UINT64 HV_NANO100_TIME,     *PHV_NANO100_TIME;
typedef UINT64 HV_NANO100_DURATION, *PHV_NANO100_DURATION;

#define HV_NANO100_TIME_NEVER ((HV_NANO100_TIME)-1)

//
// Declare the type for hardware ID of a processor.
//
#if defined(_AMD64_) || defined(_X86_) || defined(_ARM_)

typedef UINT32 HV_APIC_ID, *PHV_APIC_ID;

typedef HV_APIC_ID HV_PROCESSOR_HW_ID;

#elif defined(_ARM64_)

typedef UINT64 HV_PROCESSOR_HW_ID;

#endif

typedef HV_PROCESSOR_HW_ID *PHV_PROCESSOR_HW_ID;

typedef UINT32 HV_IOMMU_ID, *PHV_IOMMU_ID;

//
// Define the intercept access types.
//

typedef UINT8 HV_INTERCEPT_ACCESS_TYPE;

#define HV_INTERCEPT_ACCESS_READ            0
#define HV_INTERCEPT_ACCESS_WRITE           1
#define HV_INTERCEPT_ACCESS_EXECUTE         2

typedef UINT32 HV_INTERCEPT_ACCESS_TYPE_MASK;

#define HV_INTERCEPT_ACCESS_MASK_NONE           0x00
#define HV_INTERCEPT_ACCESS_MASK_READ           0X01
#define HV_INTERCEPT_ACCESS_MASK_WRITE          0x02
#define HV_INTERCEPT_ACCESS_MASK_EXECUTE        0x04

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
    HvMicrosoftHypervisorInterface = '1#vH',
    HvMicrosoftXboxNanovisor = 'vnbX'

} HV_HYPERVISOR_INTERFACE, *PHV_HYPERVISOR_INTERFACE;


//
// Microsoft hypervisor vendor 64-bit signature ('MsHyperV')
//
#define MSHYPERV_SIGNATURE_64BIT    0x567265707948734D

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

#if defined(_AMD64_) || defined(_X86_)


//
// This enumeration collates MSR indices for ease of maintainability.
// DOs:
//  1. Always keep the enumeration sorted by their values.
//  2. To preserve consistency, developers are encouraged to define macros.
//      as shown below:
//
//          #define HV_X64_MSR_FOO_BAR  HvSyntheticMsrFooBar
//
//  3. Use Xb prefix for MSRs specific to XBOX_SYSTEMOS.
//
// DONTs:
//  1. Don't add duplicate values to the enum.
//
// MSRs private to Xbox System OS have been included here to ensure an index
// isn't mulitply defined.
//
typedef enum _HV_X64_SYNTHETIC_MSR
{
    HvSyntheticMsrGuestOsId                 =    0x40000000,
    HvSyntheticMsrHypercall                 =    0x40000001,
    HvSyntheticMsrVpIndex                   =    0x40000002,
    HvSyntheticMsrReset                     =    0x40000003,
    HvSyntheticMsrCpuMgmtVersion            =    0x40000004,
    XbSyntheticMsrTbFlushHost               =    0x40000006,
    XbSyntheticMsrTbFlush                   =    0x40000007,
    XbSyntheticMsrCrash                     =    0x40000008,
    XbSyntheticMsrGuestOsType               =    0x40000009,
    HvSyntheticMsrVpRuntime                 =    0x40000010,
    XbSyntheticMsrRefTimeOffset             =    0x40000011,
    XbSyntheticMsrRefTscScale               =    0x40000012,
    XbSyntheticMsrVpCount                   =    0x40000013,
    XbSyntheticMsrWbinvd                    =    0x40000014,
    XbSyntheticMsrFlushWb                   =    0x40000015,
    XbSyntheticMsrFlushTbCurrent            =    0x40000016,
    XbSyntheticMsrKcfgDone                  =    0x40000017,
    HvSyntheticMsrTimeRefCount              =    0x40000020,
    HvSyntheticMsrReferenceTsc              =    0x40000021,
    HvSyntheticMsrTscFrequency              =    0x40000022,
    HvSyntheticMsrApicFrequency             =    0x40000023,
    HvSyntheticMsrNpiepConfig               =    0x40000040,
    HvSyntheticMsrEoi                       =    0x40000070,
    HvSyntheticMsrIcr                       =    0x40000071,
    HvSyntheticMsrTpr                       =    0x40000072,
    HvSyntheticMsrVpAssistPage              =    0x40000073,
    HvSyntheticMsrSControl                  =    0x40000080,
    HvSyntheticMsrSVersion                  =    0x40000081,
    HvSyntheticMsrSiefp                     =    0x40000082,
    HvSyntheticMsrSimp                      =    0x40000083,
    HvSyntheticMsrEom                       =    0x40000084,
    HvSyntheticMsrSirb                      =    0x40000085,
    HvSyntheticMsrSint0                     =    0x40000090,
    HvSyntheticMsrSint1                     =    0x40000091,
    HvSyntheticMsrSint2                     =    0x40000092,
    HvSyntheticMsrSint3                     =    0x40000093,
    HvSyntheticMsrSint4                     =    0x40000094,
    HvSyntheticMsrSint5                     =    0x40000095,
    HvSyntheticMsrSint6                     =    0x40000096,
    HvSyntheticMsrSint7                     =    0x40000097,
    HvSyntheticMsrSint8                     =    0x40000098,
    HvSyntheticMsrSint9                     =    0x40000099,
    HvSyntheticMsrSint10                    =    0x4000009A,
    HvSyntheticMsrSint11                    =    0x4000009B,
    HvSyntheticMsrSint12                    =    0x4000009C,
    HvSyntheticMsrSint13                    =    0x4000009D,
    HvSyntheticMsrSint14                    =    0x4000009E,
    HvSyntheticMsrSint15                    =    0x4000009F,
    HvSyntheticMsrSTimer0Config             =    0x400000B0,
    HvSyntheticMsrSTimer0Count              =    0x400000B1,
    HvSyntheticMsrSTimer1Config             =    0x400000B2,
    HvSyntheticMsrSTimer1Count              =    0x400000B3,
    HvSyntheticMsrSTimer2Config             =    0x400000B4,
    HvSyntheticMsrSTimer2Count              =    0x400000B5,
    HvSyntheticMsrSTimer3Config             =    0x400000B6,
    HvSyntheticMsrSTimer3Count              =    0x400000B7,
    HvSyntheticMsrPerfFeedbackTrigger       =    0x400000C1,
    HvSyntheticMsrGuestSchedulerEvent       =    0x400000C2,
    HvSyntheticMsrGuestIdle                 =    0x400000F0,
    HvSyntheticMsrSynthDebugControl         =    0x400000F1,
    HvSyntheticMsrSynthDebugStatus          =    0x400000F2,
    HvSyntheticMsrSynthDebugSendBuffer      =    0x400000F3,
    HvSyntheticMsrSynthDebugReceiveBuffer   =    0x400000F4,
    HvSyntheticMsrSynthDebugPendingBuffer   =    0x400000F5,
    XbSyntheticMsrSynthDebugTransition      =    0x400000F6,
    HvSyntheticMsrDebugDeviceOptions        =    0x400000FF,
    HvSyntheticMsrCrashP0                   =    0x40000100,
    HvSyntheticMsrCrashP1                   =    0x40000101,
    HvSyntheticMsrCrashP2                   =    0x40000102,
    HvSyntheticMsrCrashP3                   =    0x40000103,
    HvSyntheticMsrCrashP4                   =    0x40000104,
    HvSyntheticMsrCrashCtl                  =    0x40000105,
    HvSyntheticMsrReenlightenmentControl    =    0x40000106,
    HvSyntheticMsrTscEmulationControl       =    0x40000107,
    HvSyntheticMsrTscEmulationStatus        =    0x40000108,
    HvSyntheticMsrSWatchdogConfig           =    0x40000110,
    HvSyntheticMsrSWatchdogCount            =    0x40000111,
    HvSyntheticMsrSWatchdogStatus           =    0x40000112,
    HvSyntheticMsrSTimeUnhaltedTimerConfig  =    0x40000114,
    HvSyntheticMsrSTimeUnhaltedTimerCount   =    0x40000115,
    HvSyntheticMsrMemoryZeroingControl      =    0x40000116,
    XbSyntheticMsrFsBase                    =    0x40000122,
    XbSyntheticMsrXOnly                     =    0x40000123,
    HvSyntheticMsrBelow1MbPage              =    0x40000200,
    HvSyntheticMsrNestedVpIndex             =    0x40001002,
    HvSyntheticMsrNestedSControl            =    0x40001080,
    HvSyntheticMsrNestedSVersion            =    0x40001081,
    HvSyntheticMsrNestedSiefp               =    0x40001082,
    HvSyntheticMsrNestedSimp                =    0x40001083,
    HvSyntheticMsrNestedEom                 =    0x40001084,
    HvSyntheticMsrNestedSirb                =    0x40001085,
    HvSyntheticMsrNestedSint0               =    0x40001090,
    HvSyntheticMsrNestedSint1               =    0x40001091,
    HvSyntheticMsrNestedSint2               =    0x40001092,
    HvSyntheticMsrNestedSint3               =    0x40001093,
    HvSyntheticMsrNestedSint4               =    0x40001094,
    HvSyntheticMsrNestedSint5               =    0x40001095,
    HvSyntheticMsrNestedSint6               =    0x40001096,
    HvSyntheticMsrNestedSint7               =    0x40001097,
    HvSyntheticMsrNestedSint8               =    0x40001098,
    HvSyntheticMsrNestedSint9               =    0x40001099,
    HvSyntheticMsrNestedSint10              =    0x4000109A,
    HvSyntheticMsrNestedSint11              =    0x4000109B,
    HvSyntheticMsrNestedSint12              =    0x4000109C,
    HvSyntheticMsrNestedSint13              =    0x4000109D,
    HvSyntheticMsrNestedSint14              =    0x4000109E,
    HvSyntheticMsrNestedSint15              =    0x4000109F,

} HV_X64_SYNTHETIC_MSR, *PHV_X64_SYNTHETIC_MSR;

#else

typedef enum _HV_ARM64_SYNTHETIC_MSR
{
    HvSyntheticMsrGuestOsId                 =    0x40000000,
    HvSyntheticMsrHypercall                 =    0x40000001,
    HvSyntheticMsrVpIndex                   =    0x40000002,
    HvSyntheticMsrReset                     =    0x40000003,
    HvSyntheticMsrCpuMgmtVersion            =    0x40000004,
    HvSyntheticMsrTimeRefCount              =    0x40000020,
    HvSyntheticMsrReferenceTsc              =    0x40000021,
    HvSyntheticMsrTscFrequency              =    0x40000022,
    HvSyntheticMsrVpAssistPage              =    0x40000073,
    HvSyntheticMsrSControl                  =    0x40000080,
    HvSyntheticMsrSVersion                  =    0x40000081,
    HvSyntheticMsrSiefp                     =    0x40000082,
    HvSyntheticMsrSimp                      =    0x40000083,
    HvSyntheticMsrEom                       =    0x40000084,
    HvSyntheticMsrSirb                      =    0x40000085,
    HvSyntheticMsrSint0                     =    0x40000090,
    HvSyntheticMsrSint1                     =    0x40000091,
    HvSyntheticMsrSint2                     =    0x40000092,
    HvSyntheticMsrSint3                     =    0x40000093,
    HvSyntheticMsrSint4                     =    0x40000094,
    HvSyntheticMsrSint5                     =    0x40000095,
    HvSyntheticMsrSint6                     =    0x40000096,
    HvSyntheticMsrSint7                     =    0x40000097,
    HvSyntheticMsrSint8                     =    0x40000098,
    HvSyntheticMsrSint9                     =    0x40000099,
    HvSyntheticMsrSint10                    =    0x4000009A,
    HvSyntheticMsrSint11                    =    0x4000009B,
    HvSyntheticMsrSint12                    =    0x4000009C,
    HvSyntheticMsrSint13                    =    0x4000009D,
    HvSyntheticMsrSint14                    =    0x4000009E,
    HvSyntheticMsrSint15                    =    0x4000009F,
    HvSyntheticMsrSTimer0Config             =    0x400000B0,
    HvSyntheticMsrSTimer0Count              =    0x400000B1,
    HvSyntheticMsrSTimer1Config             =    0x400000B2,
    HvSyntheticMsrSTimer1Count              =    0x400000B3,
    HvSyntheticMsrSTimer2Config             =    0x400000B4,
    HvSyntheticMsrSTimer2Count              =    0x400000B5,
    HvSyntheticMsrSTimer3Config             =    0x400000B6,
    HvSyntheticMsrSTimer3Count              =    0x400000B7,
    HvSyntheticMsrPerfFeedbackTrigger       =    0x400000C1,
    HvSyntheticMsrGuestSchedulerEvent       =    0x400000C2,
    HvSyntheticMsrGuestIdle                 =    0x400000F0,
    HvSyntheticMsrDebugDeviceOptions        =    0x400000FF,
    HvSyntheticMsrCrashP0                   =    0x40000100,
    HvSyntheticMsrCrashP1                   =    0x40000101,
    HvSyntheticMsrCrashP2                   =    0x40000102,
    HvSyntheticMsrCrashP3                   =    0x40000103,
    HvSyntheticMsrCrashP4                   =    0x40000104,
    HvSyntheticMsrCrashCtl                  =    0x40000105,
    HvSyntheticMsrReenlightenmentControl    =    0x40000106,
    HvSyntheticMsrTscEmulationControl       =    0x40000107,
    HvSyntheticMsrTscEmulationStatus        =    0x40000108,
    HvSyntheticMsrSWatchdogConfig           =    0x40000110,
    HvSyntheticMsrSWatchdogCount            =    0x40000111,
    HvSyntheticMsrSWatchdogStatus           =    0x40000112,
    HvSyntheticMsrSTimeUnhaltedTimerConfig  =    0x40000114,
    HvSyntheticMsrSTimeUnhaltedTimerCount   =    0x40000115,
    HvSyntheticMsrMemoryZeroingControl      =    0x40000116,

} HV_ARM64_SYNTHETIC_MSR, *PHV_ARM64_SYNTHETIC_MSR;

#endif

#if defined(_AMD64_) || defined(_X86_)

//
// Declare the MSR used to identify the guest OS.
//
#define HV_X64_MSR_GUEST_OS_ID HvSyntheticMsrGuestOsId

#endif

typedef union _HV_GUEST_OS_ID_CONTENTS
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

} HV_GUEST_OS_ID_CONTENTS, *PHV_GUEST_OS_ID_CONTENTS;

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
// VM Partition Privileges mask. Please also update the bitmask along
// with the below union.
//
typedef union _HV_PARTITION_PRIVILEGE_MASK
{
    UINT64 AsUINT64;
    struct
    {

#if defined(XBOX_SYSTEMOS)

        //
        // Access to virtual MSRs
        //

        UINT64 Reserved1 : 32;

        //
        // Access to hypercalls
        //

        UINT64 CreatePartitions : 1;
        UINT64 Reserved2 : 5;
        UINT64 CreatePort : 1;
        UINT64 Reserved3 : 1;
        UINT64 AccessStats : 1;
        UINT64 Reserved4 : 2;
        UINT64 Debugging : 1;
        UINT64 CpuManagement : 1;
        UINT64 Reserved5 : 19;

#else

        //
        // Access to virtual MSRs
        //
        UINT64  AccessVpRunTimeReg:1;
        UINT64  AccessPartitionReferenceCounter:1;
        UINT64  AccessSynicRegs:1;
        UINT64  AccessSyntheticTimerRegs:1;
        UINT64  AccessIntrCtrlRegs:1;
        UINT64  AccessHypercallMsrs:1;
        UINT64  AccessVpIndex:1;
        UINT64  AccessResetReg:1;
        UINT64  AccessStatsReg:1;
        UINT64  AccessPartitionReferenceTsc:1;
        UINT64  AccessGuestIdleReg:1;
        UINT64  AccessFrequencyRegs:1;
        UINT64  AccessDebugRegs:1;
        UINT64  AccessReenlightenmentControls:1;
        UINT64  AccessRootSchedulerReg:1;
        UINT64  Reserved1:17;

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
        UINT64  AccessVpExitTracing:1;
        UINT64  EnableExtendedGvaRangesForFlushVirtualAddressList:1;
        UINT64  AccessVsm:1;
        UINT64  AccessVpRegisters:1;
        UINT64  UnusedBit:1;
        UINT64  FastHypercallOutput:1;
        UINT64  EnableExtendedHypercalls:1;
        UINT64  StartVirtualProcessor:1;
        UINT64  Isolation:1;
        UINT64  Reserved3:9;

#endif

    };

} HV_PARTITION_PRIVILEGE_MASK, *PHV_PARTITION_PRIVILEGE_MASK;

#define HV_PARTITION_PRIVILEGE_ACCESS_VP_RUNTIME_MSR                    0x0000000000000001
#define HV_PARTITION_PRIVILEGE_PARTITION_REFERENCE_COUNTER              0x0000000000000002
#define HV_PARTITION_PRIVILEGE_SYNIC_MSRS                               0x0000000000000004
#define HV_PARTITION_PRIVILEGE_ACCESS_SYNTHETIC_TIMER_MSRS              0x0000000000000008
#define HV_PARTITION_PRIVILEGE_ACCESS_APIC_MSRS                         0x0000000000000010
#define HV_PARTITION_PRIVILEGE_ACCESS_HYPERCALL_MSRS                    0x0000000000000020
#define HV_PARTITION_PRIVILEGE_ACCESS_VP_INDEX                          0x0000000000000040
#define HV_PARTITION_PRIVILEGE_ACCESS_RESET_MSR                         0x0000000000000080
#define HV_PARTITION_PRIVILEGE_ACCESS_STATS_MSR                         0x0000000000000100
#define HV_PARTITION_PRIVILEGE_ACCESS_PARTITION_REFERENCE_TSC           0x0000000000000200
#define HV_PARTITION_PRIVILEGE_ACCESS_GUEST_IDLE_MSR                    0x0000000000000400
#define HV_PARTITION_PRIVILEGE_ACCESS_FREQUENCY_MSRS                    0x0000000000000800
#define HV_PARTITION_PRIVILEGE_ACCESS_DEBUG_MSRS                        0x0000000000001000
#define HV_PARTITION_PRIVILEGE_ACCESS_REENLIGHTENMENT_CTRLS             0x0000000000002000
#define HV_PARTITION_PRIVILEGE_ACCESS_ROOT_SCHEDULER_MSR                0x0000000000004000

#define HV_PARTITION_PRIVILEGE_CREATE_PARTITIONS                        0x0000000100000000
#define HV_PARTITION_PRIVILEGE_ACCESS_PARTITION_ID                      0x0000000200000000
#define HV_PARTITION_PRIVILEGE_ACCESS_MEMORY_POOL                       0x0000000400000000
#define HV_PARTITION_PRIVILEGE_ADJUST_MESSAGE_BUFFERS                   0x0000000800000000
#define HV_PARTITION_PRIVILEGE_POST_MESSAGES                            0x0000001000000000
#define HV_PARTITION_PRIVILEGE_SIGNAL_EVENTS                            0x0000002000000000
#define HV_PARTITION_PRIVILEGE_CREATE_PORT                              0x0000004000000000
#define HV_PARTITION_PRIVILEGE_CONNECT_PORT                             0x0000008000000000
#define HV_PARTITION_PRIVILEGE_ACCESS_STATS                             0x0000010000000000
#define HV_PARTITION_PRIVILEGE_DEBUGGING                                0x0000080000000000
#define HV_PARTITION_PRIVILEGE_CPU_MANAGEMENT                           0x0000100000000000
#define HV_PARTITION_PRIVILEGE_CONFIGURE_PROFILER                       0x0000200000000000
#define HV_PARTITION_PRIVILEGE_ACCESS_VP_EXIT_TRACING                   0x0000400000000000
#define HV_PARTITION_PRIVILEGE_ENABLE_EXTENDED_GVA_RANGES_FLUSH_VA_LIST 0x0000800000000000
#define HV_PARTITION_PRIVILEGE_ACCESS_VSM                               0x0001000000000000
#define HV_PARTITION_PRIVILEGE_ACCESS_VP_REGISTERS                      0x0002000000000000

#define HV_PARTITION_PRIVILEGE_FAST_HYPERCALL_OUTPUT                    0x0008000000000000
#define HV_PARTITION_PRIVILEGE_ENABLE_EXTENDED_HYPERCALLS               0x0010000000000000
#define HV_PARTITION_PRIVILEGE_START_VIRTUAL_PROCESSOR                  0x0020000000000000
#define HV_PARTITION_PRIVILEGE_ISOLATION                                0x0040000000000000

#if defined(_AMD64_) || defined(_X86_)

typedef union _HV_X64_PLATFORM_CAPABILITIES {
    UINT64 AsUINT64[2];
    struct {

        //
        // Eax
        //

        UINT32 AllowRedSignedCode : 1;
        UINT32 AllowKernelModeDebugging : 1;
        UINT32 AllowUserModeDebugging : 1;
        UINT32 AllowTelnetServer : 1;
        UINT32 AllowIOPorts : 1;
        UINT32 AllowFullMsrSpace : 1;
        UINT32 AllowPerfCounters : 1;
        UINT32 AllowHost512MB : 1;
        UINT32 ReservedEax1 : 1;
        UINT32 AllowRemoteRecovery : 1;
        UINT32 AllowStreaming : 1;
        UINT32 AllowPushDeployment : 1;
        UINT32 AllowPullDeployment : 1;
        UINT32 AllowProfiling : 1;
        UINT32 AllowJsProfiling : 1;
        UINT32 AllowCrashDump : 1;
        UINT32 AllowVsCrashDump : 1;
        UINT32 AllowToolFileIO : 1;
        UINT32 AllowConsoleMgmt : 1;
        UINT32 AllowTracing : 1;
        UINT32 AllowXStudio : 1;
        UINT32 AllowGestureBuilder : 1;
        UINT32 AllowSpeechLab : 1;
        UINT32 AllowSmartglassStudio : 1;
        UINT32 AllowNetworkTools : 1;
        UINT32 AllowTcrTool : 1;
        UINT32 AllowHostNetworkStack : 1;
        UINT32 AllowSystemUpdateTest : 1;
        UINT32 AllowOffChipPerfCtrStreaming : 1;
        UINT32 AllowToolingMemory : 1;
        UINT32 AllowSystemDowngrade : 1;
        UINT32 AllowGreenDiskLicenses : 1;

        //
        // Ebx
        //

        UINT32 IsLiveConnected : 1;
        UINT32 IsMteBoosted : 1;
        UINT32 IsQaSlt : 1;
        UINT32 IsStockImage : 1;
        UINT32 IsMsTestLab : 1;
        UINT32 IsRetailDebugger : 1;
        UINT32 IsXvdSrt : 1;
        UINT32 IsGreenDebug : 1;
        UINT32 IsHwDevTest : 1;
        UINT32 AllowDiskLicenses : 1;
        UINT32 AllowInstrumentation : 1;
        UINT32 AllowWifiTester : 1;
        UINT32 AllowWifiTesterDFS : 1;
        UINT32 IsHwTest : 1;
        UINT32 AllowHostOddTest : 1;
        UINT32 IsLiveUnrestricted : 1;
        UINT32 AllowDiscLicensesWithoutMediaAuth : 1;
        UINT32 ReservedEbx : 15;

        //
        // Ecx
        //

        UINT32 ReservedEcx;

        //
        // Edx
        //

        UINT32 ReservedEdx : 31;
        UINT32 UseAlternateXvd : 1;
    };

} HV_X64_PLATFORM_CAPABILITIES, *PHV_X64_PLATFORM_CAPABILITIES;

#endif

#if defined(XBOX_SYSTEMOS)

//
// Declare the MSR used to identify the guest OS.
//

#define HV_X64_MSR_GUEST_OS_ID          HvSyntheticMsrGuestOsId

typedef union _HV_X64_MSR_GUEST_OS_ID_CONTENTS {
    UINT64 AsUINT64;
    struct {
        UINT64 BuildNumber : 16;
        UINT64 ServiceVersion : 8;      // service pack, etc.
        UINT64 MinorVersion : 8;
        UINT64 MajorVersion : 8;
        UINT64 OsId : 8;
        UINT64 VendorId : 16;           // HV_GUEST_OS_VENDOR
    };

} HV_X64_MSR_GUEST_OS_ID_CONTENTS, *PHV_X64_MSR_GUEST_OS_ID_CONTENTS;

//
// Support for retrieving the console hardware type.
//

typedef enum _XSYSTEM_TYPE {
    XboxSystemDurango = 0x10,
    XboxSystemSilvertonZorro = 0x20,
    XboxSystemSilvertonManda = 0x21,
    XboxSystemCarmel = 0x30,
    XboxSystemCarmel4k = 0x31,
    XboxSystemEdmonton = 0x40,
    XboxSystemScorpio = 0x50,
    XboxSystemChuckwalla = 0x58,
    XboxSystemUnknown = 0xffff,
} XSYSTEM_TYPE, *PXSYSTEM_TYPE;

//
// Define the platform information data.
//

typedef union _HV_PLATFORM_INFORMATION {
    UINT64 AsUINT64[2];
    struct {

        //
        // Eax and Ebx
        //

        HV_X64_MSR_GUEST_OS_ID_CONTENTS HostOsId;

        //
        // Ecx
        //

        UINT32 SystemType : 16;
        UINT32 ConsoleMode : 16;

        //
        // Edx
        //

        UINT32 QfeVersion : 16;
        UINT32 ReservedEdx : 16;
    };

} HV_PLATFORM_INFORMATION, *PHV_PLATFORM_INFORMATION;

#endif

//
// Typedefs for CPUID leaves on HvMicrosoftHypercallInterface-supporting
// hypervisors.
// =====================================================================
//
// The below CPUID leaves are present if VersionAndFeatures.HypervisorPresent
// is set by CPUID(HvCpuIdFunctionVersionAndFeatures).
// =====================================================================
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

#if !defined(XBOX_SYSTEMOS)

    HvCpuIdFunctionMsHvCpuManagementFeatures    = 0x40000007,
    HvCpuIdFunctionMsHvSvmFeatures              = 0x40000008,
    HvCpuIdFunctionMsHvSkipLevelFeatures        = 0x40000009,
    HvCpuidFunctionMsHvNestedVirtFeatures       = 0x4000000A,
    HvCpuidFunctionMsHvIsolationConfiguration   = 0x4000000C,
    HvCpuIdFunctionMaxReserved                  = 0x4000000C

#else

    HvCpuIdFunctionMsHvConsoleFeatures          = 0x40000007,
    HvCpuIdFunctionMsHvConsoleFeaturesSystem    = 0x40000008,
    HvCpuIdFunctionMsHvConsoleFeaturesTitle     = 0x40000009,
    HvCpuIdFunctionMsHvConsoleSocId             = 0x4000000a,
    HvCpuIdFunctionMsHvConsoleGenerationId      = 0x4000000b,
    HvCpuIdFunctionMsHvVpRuntime                = 0x4000000c,
    HvCpuIdFunctionMsHvConsoleInformation       = 0x4000000d,
    HvCpuIdFunctionMaxReserved                  = 0x4000000d

#endif

} HV_CPUID_FUNCTION, *PHV_CPUID_FUNCTION;


//
// Hypervisor Vendor Info - HvCpuIdFunctionHvVendorAndMaxFunction Leaf
//

typedef struct _HV_VENDOR_AND_MAX_FUNCTION
{
    UINT32 MaxFunction;
    UINT8 VendorName[12];
} HV_VENDOR_AND_MAX_FUNCTION, *PHV_VENDOR_AND_MAX_FUNCTION;


//
// Hypervisor Interface Info - HvCpuIdFunctionHvInterface Leaf
//

typedef struct _HV_HYPERVISOR_INTERFACE_INFO
{
    UINT32 Interface; // HV_HYPERVISOR_INTERFACE
    UINT32 Reserved1;
    UINT32 Reserved2;
    UINT32 Reserved3;
} HV_HYPERVISOR_INTERFACE_INFO, *PHV_HYPERVISOR_INTERFACE_INFO;


//
// Hypervisor Feature Information
//

#if defined(_AMD64_) || defined(_X86_)

// CPUID Information - HvCpuIdFunctionMsHvFeatures Leaf

typedef struct _HV_X64_HYPERVISOR_FEATURES
{
    //
    // Eax-Ebx
    //
    HV_PARTITION_PRIVILEGE_MASK PartitionPrivileges;

    //
    // Ecx - this indicates the power configuration for the current VP.
    //
    UINT32 MaxSupportedCState:4;
    UINT32 HpetNeededForC3PowerState_Deprecated:1;
    UINT32 Reserved:27;

    //
    // Edx
    //
    UINT32 MwaitAvailable_Deprecated:1;
    UINT32 GuestDebuggingAvailable:1;
    UINT32 PerformanceMonitorsAvailable:1;
    UINT32 CpuDynamicPartitioningAvailable:1;
    UINT32 XmmRegistersForFastHypercallAvailable:1;
    UINT32 GuestIdleAvailable:1;
    UINT32 HypervisorSleepStateSupportAvailable:1;
    UINT32 NumaDistanceQueryAvailable:1;
    UINT32 FrequencyRegsAvailable:1;
    UINT32 SyntheticMachineCheckAvailable:1;
    UINT32 GuestCrashRegsAvailable:1;
    UINT32 DebugRegsAvailable:1;
    UINT32 Npiep1Available:1;
    UINT32 DisableHypervisorAvailable:1;
    UINT32 ExtendedGvaRangesForFlushVirtualAddressListAvailable:1;

#if defined(XBOX_SYSTEMOS)

    UINT32 Reserved1:15;
    UINT32 ReferenceTimeMsrsAvailable : 1;
    UINT32 FlushMsrAvailable : 1;

#else

    UINT32 FastHypercallOutputAvailable:1;
    UINT32 SvmFeaturesAvailable:1;
    UINT32 SintPollingModeAvailable:1;
    UINT32 HypercallMsrLockAvailable:1;
    UINT32 DirectSyntheticTimers:1;
    UINT32 RegisterPatAvailable:1;
    UINT32 RegisterBndcfgsAvailable:1;
    UINT32 WatchdogTimerAvailable:1;
    UINT32 SyntheticTimeUnhaltedTimerAvailable:1;
    UINT32 DeviceDomainsAvailable:1; // HDK only.
    UINT32 S1DeviceDomainsAvailable:1; // HDK only.
    UINT32 Reserved1:6;

#endif

} HV_X64_HYPERVISOR_FEATURES, *PHV_X64_HYPERVISOR_FEATURES;

#define _HV_HYPERVISOR_FEATURES     _HV_X64_HYPERVISOR_FEATURES
#define HV_HYPERVISOR_FEATURES      HV_X64_HYPERVISOR_FEATURES
#define PHV_HYPERVISOR_FEATURES     PHV_X64_HYPERVISOR_FEATURES

#endif

#if defined(_ARM64_) || defined(_ARM_)

typedef struct _HV_ARM64_HYPERVISOR_FEATURES
{
    HV_PARTITION_PRIVILEGE_MASK PartitionPrivileges;

    UINT32 GuestDebuggingAvailable:1;
    UINT32 PerformanceMonitorsAvailable:1;
    UINT32 CpuDynamicPartitioningAvailable:1;
    UINT32 GuestIdleAvailable:1;
    UINT32 HypervisorSleepStateSupportAvailable:1;
    UINT32 NumaDistanceQueryAvailable:1;
    UINT32 FrequencyRegsAvailable:1;
    UINT32 SyntheticMachineCheckAvailable:1;
    UINT32 GuestCrashRegsAvailable:1;
    UINT32 DebugRegsAvailable:1;
    UINT32 DisableHypervisorAvailable:1;
    UINT32 ExtendedGvaRangesForFlushVirtualAddressListAvailable:1;
    UINT32 SintPollingModeAvailable:1;
    UINT32 DirectSyntheticTimers:1;
    UINT32 DeviceDomainsAvailable:1; // HDK only.
    UINT32 S1DeviceDomainsAvailable:1; // HDK only.
    UINT32 Reserved1:16;


} HV_ARM64_HYPERVISOR_FEATURES, *PHV_ARM64_HYPERVISOR_FEATURES;

#define _HV_HYPERVISOR_FEATURES     _HV_ARM64_HYPERVISOR_FEATURES
#define HV_HYPERVISOR_FEATURES      HV_ARM64_HYPERVISOR_FEATURES
#define PHV_HYPERVISOR_FEATURES     PHV_ARM64_HYPERVISOR_FEATURES

#endif


//
// Enlightenment Info
//

#if defined(_AMD64_) || defined(_X86_)

// CPUID Information - HvCpuIdFunctionMsHvEnlightenmentInformation Leaf

typedef struct _HV_X64_ENLIGHTENMENT_INFORMATION
{
    //
    // Eax
    //
    UINT32 UseHypercallForAddressSpaceSwitch:1;
    UINT32 UseHypercallForLocalFlush:1;
    UINT32 UseHypercallForRemoteFlushAndLocalFlushEntire:1;
    UINT32 UseApicMsrs:1;
    UINT32 UseHvRegisterForReset:1;
    UINT32 UseRelaxedTiming:1;
    UINT32 UseDmaRemapping_Deprecated:1;
    UINT32 UseInterruptRemapping_Deprecated:1;
    UINT32 UseX2ApicMsrs:1;
    UINT32 DeprecateAutoEoi:1;
    UINT32 UseSyntheticClusterIpi:1;
    UINT32 UseExProcessorMasks:1;
    UINT32 Nested:1;
    UINT32 UseIntForMbecSystemCalls:1;
    UINT32 UseVmcsEnlightenments:1;
    UINT32 UseSyncedTimeline:1;
    UINT32 Available:1;  // Was UseReferencePageForSyncedTimeline but never consumed.
    UINT32 UseDirectLocalFlushEntire:1;
    UINT32 Reserved:14;

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

} HV_X64_ENLIGHTENMENT_INFORMATION, *PHV_X64_ENLIGHTENMENT_INFORMATION;

#define _HV_ENLIGHTENMENT_INFORMATION   _HV_X64_ENLIGHTENMENT_INFORMATION
#define HV_ENLIGHTENMENT_INFORMATION    HV_X64_ENLIGHTENMENT_INFORMATION
#define PHV_ENLIGHTENMENT_INFORMATION   PHV_X64_ENLIGHTENMENT_INFORMATION

#endif

#if defined(_ARM64_) || defined(_ARM_)

typedef struct _HV_ARM64_ENLIGHTENMENT_INFORMATION
{
    UINT32 UseHvRegisterForReset:1;
    UINT32 UseRelaxedTiming:1;
    UINT32 UseSyntheticClusterIpi:1;
    UINT32 UseExProcessorMasks:1;
    UINT32 Nested:1;
    UINT32 UseSyncedTimeline:1;
    UINT32 Reserved:26;

    UINT32 LongSpinWaitCount;

    UINT32 Reserved0;
    UINT32 Reserved1;

} HV_ARM64_ENLIGHTENMENT_INFORMATION, *PHV_ARM64_ENLIGHTENMENT_INFORMATION;

#define _HV_ENLIGHTENMENT_INFORMATION   _HV_ARM64_ENLIGHTENMENT_INFORMATION
#define HV_ENLIGHTENMENT_INFORMATION    HV_ARM64_ENLIGHTENMENT_INFORMATION
#define PHV_ENLIGHTENMENT_INFORMATION   PHV_ARM64_ENLIGHTENMENT_INFORMATION

#endif


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
    UINT32 Reserved;

} HV_IMPLEMENTATION_LIMITS, *PHV_IMPLEMENTATION_LIMITS;


#if defined(_AMD64_) || defined(_X86_)

//
// Hypervisor Hardware Features Info - HvCpuIdFunctionMsHvHardwareFeatures Leaf
//

typedef struct _HV_X64_HYPERVISOR_HARDWARE_FEATURES
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
    UINT32 DmaProtectionInUse:1;
    UINT32 HpetRequested:1;
    UINT32 SyntheticTimersVolatile:1;
    UINT32 HypervisorLevel:4;
    UINT32 PhysicalDestinationModeRequired:1;
    UINT32 UseVmfuncForAliasMapSwitch:1;
    UINT32 HvRegisterForMemoryZeroingSupported:1;
    UINT32 UnrestrictedGuestSupported:1;
    UINT32 L3CachePartitioningSupported:1;
    UINT32 L3CacheMonitoringSupported:1;
    UINT32 Reserved:12;

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

} HV_X64_HYPERVISOR_HARDWARE_FEATURES, *PHV_X64_HYPERVISOR_HARDWARE_FEATURES;

#define _HV_HYPERVISOR_HARDWARE_FEATURES    _HV_X64_HYPERVISOR_HARDWARE_FEATURES
#define HV_HYPERVISOR_HARDWARE_FEATURES     HV_X64_HYPERVISOR_HARDWARE_FEATURES
#define PHV_HYPERVISOR_HARDWARE_FEATURES    PHV_X64_HYPERVISOR_HARDWARE_FEATURES

//
// Hypervisor Cpu Management features - HvCpuIdFunctionMsHvCpuManagementFeatures
// leaf.
//

typedef struct _HV_X64_HYPERVISOR_CPU_MANAGEMENT_FEATURES
{
    //
    // Eax
    //
    UINT32 StartLogicalProcessor:1;
    UINT32 CreateRootVirtualProcessor:1;
    UINT32 PerformanceCounterSync:1;
    UINT32 Reserved0:28;
    UINT32 ReservedIdentityBit:1;

    //
    // Ebx
    //
    UINT32 ProcessorPowerManagement:1;
    UINT32 MwaitIdleStates:1;
    UINT32 LogicalProcessorIdling:1;
    UINT32 Reserved1:29;

    //
    // Ecx
    //
    UINT32 RemapGuestUncached : 1;
    UINT32 ReservedZ2 : 31;

    //
    // Edx
    //
    UINT32 ReservedEdx;

} HV_X64_HYPERVISOR_CPU_MANAGEMENT_FEATURES, *PHV_X64_HYPERVISOR_CPU_MANAGEMENT_FEATURES;

#define _HV_HYPERVISOR_CPU_MANAGEMENT_FEATURES _HV_X64_HYPERVISOR_CPU_MANAGEMENT_FEATURES
#define HV_HYPERVISOR_CPU_MANAGEMENT_FEATURES HV_X64_HYPERVISOR_CPU_MANAGEMENT_FEATURES
#define PHV_HYPERVISOR_CPU_MANAGEMENT_FEATURES PHV_X64_HYPERVISOR_CPU_MANAGEMENT_FEATURES

#endif

#if defined(_ARM64_) || defined(_ARM_)

typedef struct _HV_ARM64_HYPERVISOR_HARDWARE_FEATURES
{
    UINT32 ArchitecturalPerformanceCountersInUse:1;
    UINT32 SecondLevelAddressTranslationInUse:1;
    UINT32 DmaRemappingInUse:1;
    UINT32 InterruptRemappingInUse:1;
    UINT32 MemoryPatrolScrubberPresent:1;
    UINT32 DmaProtectionInUse:1;
    UINT32 SyntheticTimersVolatile:1;
    UINT32 HvRegisterForMemoryZeroingSupported:1;
    UINT32 Reserved:24;

    UINT32 Reserved0;
    UINT32 Reserved1;
    UINT32 Reserved2;

} HV_ARM64_HYPERVISOR_HARDWARE_FEATURES, *PHV_ARM64_HYPERVISOR_HARDWARE_FEATURES;

#define _HV_HYPERVISOR_HARDWARE_FEATURES    _HV_ARM64_HYPERVISOR_HARDWARE_FEATURES
#define HV_HYPERVISOR_HARDWARE_FEATURES     HV_ARM64_HYPERVISOR_HARDWARE_FEATURES
#define PHV_HYPERVISOR_HARDWARE_FEATURES    PHV_ARM64_HYPERVISOR_HARDWARE_FEATURES

//
// Hypervisor Cpu Management features - HvCpuIdFunctionMsHvCpuManagementFeatures
// leaf.
//

typedef struct _HV_ARM64_HYPERVISOR_CPU_MANAGEMENT_FEATURES
{
    //
    // Eax
    //
    UINT32 StartLogicalProcessor:1;
    UINT32 CreateRootVirtualProcessor:1;
    UINT32 PerformanceCounterSync:1;
    UINT32 Reserved0:28;
    UINT32 ReservedIdentityBit:1;

    //
    // Ebx
    //
    UINT32 ProcessorPowerManagement:1;
    UINT32 RootManagedIdleStates:1;
    UINT32 Reserved1:30;

    //
    // Ecx
    //
    UINT32 ReservedEcx;

    //
    // Edx
    //
    UINT32 ReservedEdx;

} HV_ARM64_HYPERVISOR_CPU_MANAGEMENT_FEATURES, *PHV_ARM64_HYPERVISOR_CPU_MANAGEMENT_FEATURES;

#define _HV_HYPERVISOR_CPU_MANAGEMENT_FEATURES _HV_ARM64_HYPERVISOR_CPU_MANAGEMENT_FEATURES
#define HV_HYPERVISOR_CPU_MANAGEMENT_FEATURES HV_ARM64_HYPERVISOR_CPU_MANAGEMENT_FEATURES
#define PHV_HYPERVISOR_CPU_MANAGEMENT_FEATURES PHV_ARM64_HYPERVISOR_CPU_MANAGEMENT_FEATURES

#endif

//
// SVM features - HvCpuIdFunctionMsHvSvmFeatures leaf.
//

typedef struct _HV_HYPERVISOR_SVM_FEATURES
{
    // Eax
    UINT32 SvmSupported : 1;
    UINT32 Reserved0 : 10;
    UINT32 MaxPasidSpacePasidCount : 21;

    // Ebx
    UINT32 MaxPasidSpaceCount;

    // Ecx
    UINT32 MaxDevicePrqSize;

    // Edx
    UINT32 Reserved1;

} HV_HYPERVISOR_SVM_FEATURES, *PHV_HYPERVISOR_SVM_FEATURES;


#if defined(_AMD64_) || defined(_X86_)

//
// Nested virtualization features (Vmx) -
//

typedef struct _HV_HYPERVISOR_NESTED_VIRT_FEATURES
{
    // Eax
    UINT32 EnlightenedVmcsVersionLow                : 8;
    UINT32 EnlightenedVmcsVersionHigh               : 8;
    UINT32 FlushGuestPhysicalHypercall_Deprecated   : 1;
    UINT32 NestedFlushVirtualHypercall              : 1;
    UINT32 FlushGuestPhysicalHypercall              : 1;
    UINT32 MsrBitmap                                : 1;
    UINT32 VirtualizationException                  : 1;
    UINT32 Reserved0                                : 11;

    // Ebx
    UINT32 ReservedEbx;

    // Ecx
    UINT32 ReservedEcx;

    // Edx
    UINT32 ReservedEdx;

} HV_HYPERVISOR_NESTED_VIRT_FEATURES, *PHV_HYPERVISOR_NESTED_VIRT_FEATURES;

#endif

//
// Isolated VM configuration - HvCpuidFunctionMsHvIsolationConfiguration leaf.
//

typedef struct _HV_HYPERVISOR_ISOLATION_CONFIGURATION
{
    // Eax
    UINT32 ParavisorPresent : 1;
    UINT32 Reserved0 : 31;

    // Ebx
    UINT32 IsolationType : 4;
    UINT32 Reserved11 : 1;
    UINT32 SharedGpaBoundaryActive : 1;
    UINT32 SharedGpaBoundaryBits : 6;
    UINT32 Reserved12 : 20;

    // Ecx
    UINT32 Reserved2;

    // Edx
    UINT32 Reserved3;

} HV_HYPERVISOR_ISOLATION_CONFIGURATION, *PHV_HYPERVISOR_ISOLATION_CONFIGURATION;

#define HV_PARTITION_ISOLATION_TYPE_NONE            0
#define HV_PARTITION_ISOLATION_TYPE_VBS             1
#define HV_PARTITION_ISOLATION_TYPE_SNP             2
#define HV_PARTITION_ISOLATION_TYPE_TDX             3

//
// Typedefs for CPUID leaves on HvMicrosoftHypercallInterface-supporting
// hypervisors.
// =====================================================================
//

typedef union _HV_CPUID_RESULT
{

    UINT32 AsUINT32[4];

#if defined(_AMD64_) || defined(_X86_)

    struct
    {
        UINT32 Eax;
        UINT32 Ebx;
        UINT32 Ecx;
        UINT32 Edx;
    };

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

    HV_X64_PLATFORM_CAPABILITIES MsHvPlatformCapabilities;

    HV_HYPERVISOR_NESTED_VIRT_FEATURES MsHvNestedVirtFeatures;

#endif

    //
    // Eax-Edx on x86/x64
    //

    HV_VENDOR_AND_MAX_FUNCTION HvVendorAndMaxFunction;

    HV_HYPERVISOR_INTERFACE_INFO HvInterface;

    HV_HYPERVISOR_VERSION_INFO MsHvVersion;

    HV_HYPERVISOR_FEATURES MsHvFeatures;

    HV_ENLIGHTENMENT_INFORMATION MsHvEnlightenmentInformation;

    HV_IMPLEMENTATION_LIMITS MsHvImplementationLimits;

    HV_HYPERVISOR_HARDWARE_FEATURES MsHvHardwareFeatures;

    HV_HYPERVISOR_CPU_MANAGEMENT_FEATURES MsHvCpuManagementFeatures;

    HV_HYPERVISOR_SVM_FEATURES MsHvSvmFeatures;

    HV_HYPERVISOR_ISOLATION_CONFIGURATION MsHvIsolationConfiguration;

#if defined(XBOX_SYSTEMOS)
    HV_PLATFORM_INFORMATION MsHvPlatformInformation;
#endif

} HV_CPUID_RESULT, *PHV_CPUID_RESULT;

//
// Declare the MSR used to setup pages used to communicate with the hypervisor.
//
#define HV_X64_MSR_HYPERCALL HvSyntheticMsrHypercall

typedef union _HV_X64_MSR_HYPERCALL_CONTENTS
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Enable               : 1;
        UINT64 Locked               : 1;
        UINT64 ReservedP            : 10;
        UINT64 GpaPageNumber        : 52;
    };
} HV_X64_MSR_HYPERCALL_CONTENTS, *PHV_X64_MSR_HYPERCALL_CONTENTS;


//
// Virtual Processor Indices
//
typedef UINT32 HV_VP_INDEX, *PHV_VP_INDEX;

#define HV_MAX_VP_INDEX  (319)
#define HV_VP_INDEX_SELF ((HV_VP_INDEX)-2)
#define HV_ANY_VP        ((HV_VP_INDEX)-1)

//
// Define the format of the VP_ASSIST virtual register (and synthetic MSR).
//

typedef union _HV_REGISTER_VP_ASSIST_PAGE
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Enable                   : 1;
        UINT64 ReservedP                : 11;
        UINT64 GpaPageNumber            : 52;
    };
} HV_REGISTER_VP_ASSIST_PAGE, *PHV_REGISTER_VP_ASSIST_PAGE;

#if defined(_ARM64_)

//
// Declare the MSR used to reset partition
//

typedef union _HV_ARM64_MSR_RESET_CONTENTS
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Reset        :1;
        UINT64 ReservedZ    :63;
    };
} HV_ARM64_MSR_RESET_CONTENTS, *PHV_ARM64_MSR_RESET_CONTENTS;

#define HV_MSR_RESET_CONTENTS  HV_ARM64_MSR_RESET_CONTENTS
#define PHV_MSR_RESET_CONTENTS PHV_ARM64_MSR_RESET_CONTENTS

#elif defined(_AMD64_) || defined(_X86_)

//
// Declare the MSR for determining the current VP index.
//
#define HV_X64_MSR_VP_INDEX             (HvSyntheticMsrVpIndex)
#define HV_X64_MSR_RESET                (HvSyntheticMsrReset)

#define HV_X64_MSR_NESTED_VP_INDEX      (HvSyntheticMsrNestedVpIndex)

#if defined(XBOX_SYSTEMOS)

//
// Declare the MSR for host VM TB flush.
//

#define HV_X64_MSR_TBFLUSH_HOST         (XbSyntheticMsrTbFlushHost)

//
// Declare the MSR for TB flush.
//
// N.B. Writes to this MSR take two 64-bit values in RAX and RDX. Reads
//      from this MSR are invalid.
//

#define HV_X64_MSR_TBFLUSH              (XbSyntheticMsrTbFlush)

//
// Declare TB flush VA structure (RAX).
//

typedef union _HV_MSR_TBFLUSH_VA {
    UINT64 AsUINT64;
    struct {
        UINT64 AdditionalPages : 12;
        UINT64 VaPageNumber : 52;
    };

} HV_MSR_TBFLUSH_VA, *PHV_MSR_TBFLUSH_VA;

//
// Declare TB flush structure (RDX).
//

typedef union _HV_MSR_TBFLUSH_CONTROL {
    UINT64 AsUINT64;
    struct {
        INT32 CpuSet;
        UINT32 Global : 1;
        UINT32 All : 1;
        UINT32 Reserved : 30;
    };

} HV_MSR_TBFLUSH_CONTROL, *PHV_MSR_TBFLUSH_CONTROL;

//
// Declare the MSR to cause a system crash.
//

#define HV_X64_MSR_CRASH                (XbSyntheticMsrCrash)

//
// Declare the guest OS type MSR.
//
// Guests can be either host OS based (host/ERA) or system OS based (SRA/gamecore).
//

#define HV_GUEST_OS_TYPE_HOST           (0x0)
#define HV_GUEST_OS_TYPE_SYSTEM         (0x1)

#define HV_X64_MSR_GUEST_OS_TYPE        (XbSyntheticMsrGuestOsType)

//
// Declare reference time offset MSR.
//

#define HV_X64_MSR_REF_TIME_OFFSET      (XbSyntheticMsrRefTimeOffset)

//
// Declare reference time TSC scale MSR.
//

#define HV_X64_MSR_REF_TSC_SCALE        (XbSyntheticMsrRefTscScale)

//
// Declare the VP count MSR.
//

#define HV_X64_MSR_VP_COUNT             (XbSyntheticMsrVpCount)

//
// Declare write back invalidate cache MSR.
//

#define HV_X64_MSR_WBINVD               (XbSyntheticMsrWbinvd)

//
// Declare flush write buffers MSR.
//

#define HV_X64_MSR_FLUSHWB              (XbSyntheticMsrFlushWb)

//
// Declare flush TB current MSR.
//

#define HV_X64_MSR_FLUSHTB_CURRENT      (XbSyntheticMsrFlushTbCurrent)

//
// Declare kernel CFG initialization done.
//

#define HV_X64_MSR_KCFG_DONE            (XbSyntheticMsrKcfgDone)

#endif

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

#define HV_MSR_RESET_CONTENTS  HV_X64_MSR_RESET_CONTENTS
#define PHV_MSR_RESET_CONTENTS PHV_X64_MSR_RESET_CONTENTS

#define HV_X64_MSR_EOI                  (HvSyntheticMsrEoi)
#define HV_X64_MSR_ICR                  (HvSyntheticMsrIcr)
#define HV_X64_MSR_TPR                  (HvSyntheticMsrTpr)

//
// Starting with Windows 10, the APIC assist page has been extended to be a more
// generic VP assist page that provides additional features beyond just the APIC
// assist enlightenment.  Thus, it's been renamed on Windows 10.  However, the
// format and usage of the MSR has not changed.  So a Windows 10 guest can still
// use the page on an older hypervisor without problems.
//

#define HV_X64_MSR_VP_ASSIST_PAGE       (HvSyntheticMsrVpAssistPage)

//
// To avoid churning XBOX code, the legacy names are defined for XBOX builds.
//

#if defined(XBOX_SYSTEMOS)

#define HV_X64_MSR_APIC_ASSIST_PAGE     (HV_X64_MSR_VP_ASSIST_PAGE)
typedef HV_REGISTER_VP_ASSIST_PAGE HV_X64_MSR_APIC_ASSIST_CONTENTS;

#endif

#define HV_VIRTUAL_APIC_NO_EOI_REQUIRED 0x0

#endif

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

#if defined(XBOX_SYSTEMOS)

    //
    // Statistics hypercalls.
    //

    HvCallMapStatsPage                  = 0x0001,
    HvCallUnmapStatsPage                = 0x0002,

    //
    // Flush virtual address list.
    //

    HvCallFlushVirtualAddressList       = 0x0003,

    //
    // Debug hypercalls.
    //

    HvCallPostDebugData                 = 0x0004,
    HvCallRetrieveDebugData             = 0x0005,
    HvCallResetDebugSession             = 0x0006,

    //
    // Virtual processor management hypercalls.
    //

    HvCallCreateVp                      = 0x0007,
    HvCallDeleteVp                      = 0x0008,
    HvCallGetVpRegisters                = 0x0009,
    HvCallSetVpRegisters                = 0x000a,

    //
    // Save and restore title VM state.
    //

    HvCallRestoreTitleState             = 0x000b,
    HvCallSaveTitleState                = 0x000c,

    //
    // Read code bytes.
    //

    HvCallReadCodeBytes                 = 0x000d,

    //
    // IOMMU hypercalls.
    //

    HvCallCreateDeviceMap               = 0x000e,
    HvCallDeleteDeviceMap               = 0x000f,
    HvCallMapIdentityTransfer           = 0x0010,
    HvCallUnmapIdentityTransfer         = 0x0011,
    HvCallMapRelocatedTransfer          = 0x0012,
    HvCallUnmapRelocatedTransfer        = 0x0013,
    HvCallUnused0014                    = 0x0014,

    //
    // Port hypercalls.
    //

    HvCallCreatePort                    = 0x0015,
    HvCallDeletePort                    = 0x0016,
    HvCallConnectPort                   = 0x0017,
    HvCallDisconnectPort                = 0x0018,
    HvCallPostMessage                   = 0x0019,
    HvCallSignalEvent                   = 0x001a,

    //
    // Guest physical address space management hypercalls.
    //

    HvCallMapGpaPages                   = 0x001b,
    HvCallUnmapGpaPages                 = 0x001c,

    //
    // Intercept management hypercall.
    //

    HvCallInstallIntercept              = 0x001d,

    //
    // PCI config hypercalls.
    //

    HvCallGetPciConfigSpace             = 0x001e,
    HvCallSetPciConfigSpace             = 0x001f,

    //
    // Switch focus.
    //

    HvCallSwitchFocus                   = 0x0020,

    //
    // Map GPA pages extended hypercall.
    //

    HvCallMapGpaPagesEx                 = 0x0021,

    //
    // Partition management hypercalls.
    //

    HvCallCreatePartition               = 0x0022,
    HvCallInitializePartition           = 0x0023,
    HvCallFinalizePartition             = 0x0024,
    HvCallDeletePartition               = 0x0025,
    HvCallGetPartitionProperty          = 0x0026,
    HvCallSetPartitionProperty          = 0x0027,
    HvCallGetPartitionId                = 0x0028,

    //
    // Code security hypercalls.
    //

    HvCallVerifySignaturePage           = 0x0029,
    HvCallVerifyAndMapCodePage          = 0x002a,
    HvCallUnmapCodePage                 = 0x002b,
    HvCallCopyCodePage                  = 0x002c,
    HvCallMakeCodePageWriteable         = 0x002d,

    //
    // Map/unmap interrupt hypercalls.
    //

    HvCallMapInterrupt                  = 0x002e,
    HvCallUnmapInterrupt                = 0x002f,

    //
    // Debugger data block information hypercalls.
    //

    HvCallSetDebuggerDataBlockInfo      = 0x0030,
    HvCallGetDebuggerDataBlockInfo      = 0x0031,

    //
    // Make data page read only.
    //

    HvCallMakeDataPageReadOnly          = 0x0032,

    //
    // Lock GDTR, IDTR, TR, and LDTR registers.
    //

    HvCallLockVpRegisters               = 0x0033,

    //
    // Make data page writeable.
    //

    HvCallMakeDataPageWriteable         = 0x0034,

    //
    // Set scheduling internal.
    //

    HvCallSetScheduleInterval           = 0x0035,

    //
    // Apply code page fixup.
    //

    HvCallApplyCodePageFixup            = 0x0036,

#else

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

    //
    // Deprectated index, now repurposed for updating processor features.
    //

    HvCallUpdateHvProcessorFeatures     = 0x0005,

    //
    //  Deprecated index, now repurposed for switching to and from alias map.
    //

    HvCallSwitchAliasMap                = 0x0006,

    //
    // Deprecated index, now repurposed for dynamic microcode updates.
    //
    HvCallUpdateMicrocode               = 0x0007,

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
    // V5 Virtual Secure Mode (VSM) hypercalls.
    //

    HvCallModifyVtlProtectionMask       = 0x000c,
    HvCallEnablePartitionVtl            = 0x000d,
    HvCallDisablePartitionVtl           = 0x000e,
    HvCallEnableVpVtl                   = 0x000f,
    HvCallDisableVpVtl                  = 0x0010,
    HvCallVtlCall                       = 0x0011,
    HvCallVtlReturn                     = 0x0012,

    //
    // V5 Extended Flush Routines and Cluster IPIs.
    //

    HvCallFlushVirtualAddressSpaceEx    = 0x0013,
    HvCallFlushVirtualAddressListEx     = 0x0014,
    HvCallSendSyntheticClusterIpiEx     = 0x0015,

    //
    // V1 enlightenment name space reservation.
    //

    HvCallQueryImageInfo                = 0x0016,
    HvCallMapImagePages                 = 0x0017,
    HvCallCommitPatch                   = 0x0018,
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

    HvCallAssertVirtualInterruptDeprecated = 0x0055,
    HvCallClearVirtualInterrupt            = 0x0056,

    //
    // V1 Port IDs
    //

    HvCallCreatePortDeprecated          = 0x0057,
    HvCallDeletePort                    = 0x0058,
    HvCallConnectPortDeprecated         = 0x0059,
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
    // V3 IOMMU hypercalls
    //

    HvCallMapDeviceInterrupt            = 0x007c,
    HvCallUnmapDeviceInterrupt          = 0x007d,

    //
    // V5 IOMMU hypercalls
    //

    HvCallRetargetDeviceInterrupt       = 0x007e,
    HvCallRetargetRootDeviceInterrupt   = 0x007f,

    //
    // V6 IOMMU hypercalls
    //

    HvCallAssertDeviceInterrupt         = 0x0080,

    //
    // V3 IOMMU hypercalls
    //

    HvCallReserved0081                  = 0x0081,
    HvCallAttachDevice                  = 0x0082,
    HvCallDetachDevice                  = 0x0083,

    //
    // V3 Sleep state transition hypercall
    //

    HvCallEnterSleepState              = 0x0084,
    HvCallNotifyStandbyTransition      = 0x0085,
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
    // V5 Synic Hypercalls (slightly extended from V1)
    //

    HvCallAssertVirtualInterrupt        = 0x0094,
    HvCallCreatePort                    = 0x0095,
    HvCallConnectPort                   = 0x0096,

    //
    // V5 Resource Management Hypercalls.
    //

    HvCallGetSpaPageList                = 0x0097,

    //
    // V5 ARM64 Startup Stub interface
    //

    HvCallArm64GetStartStub             = 0x0098,

    //
    // V5 VP Startup Enlightment
    //

    HvCallStartVirtualProcessor         = 0x0099,

    //
    // V5 VP Index retrieval for VP startup
    //

    HvCallGetVpIndexFromApicId          = 0x009A,

    //
    // V5 Power management
    //

    HvCallGetPowerProperty              = 0x009B,
    HvCallSetPowerProperty              = 0x009C,

    //
    // V5 SVM hypercalls
    //

    HvCallCreatePasidSpace              = 0x009D,
    HvCallDeletePasidSpace              = 0x009E,
    HvCallSetPasidAddressSpace          = 0x009F,
    HvCallFlushPasidAddressSpace        = 0x00A0,
    HvCallFlushPasidAddressList         = 0x00A1,
    HvCallAttachPasidSpace              = 0x00A2,
    HvCallDetachPasidSpace              = 0x00A3,
    HvCallEnablePasid                   = 0x00A4,
    HvCallDisablePasid                  = 0x00A5,
    HvCallAcknowledgeDevicePageRequest  = 0x00A6,
    HvCallCreateDevicePrQueue           = 0x00A7,
    HvCallDeleteDevicePrQueue           = 0x00A8,
    HvCallSetDevicePrqProperty          = 0x00A9,
    HvCallGetPhysicalDeviceProperty     = 0x00AA,
    HvCallSetPhysicalDeviceProperty     = 0x00AB,

#endif

    //
    // V5 Virtual TLB IDs
    //

    HvCallTranslateVirtualAddressEx     = 0x00AC,

    //
    // V5 I/O port intercepts
    //

    HvCallCheckForIoIntercept           = 0x00AD,

    //
    // V5 GPA page attributes.
    //

    HvCallSetGpaPageAttributes          = 0x00AE,

    //
    // V5 Enlightened nested page table flush.
    //

    HvCallFlushGuestPhysicalAddressSpace = 0x00AF,
    HvCallFlushGuestPhysicalAddressList  = 0x00B0,

    //
    // V5 Device Domains.
    //

    HvCallCreateDeviceDomain            = 0x00B1,
    HvCallAttachDeviceDomain            = 0x00B2,
    HvCallMapDeviceGpaPages             = 0x00B3,
    HvCallUnmapDeviceGpaPages           = 0x00B4,

    //
    // V5 CPU group management.
    //

    HvCallCreateCpuGroup                = 0x00B5,
    HvCallDeleteCpuGroup                = 0x00B6,
    HvCallGetCpuGroupProperty           = 0x00B7,
    HvCallSetCpuGroupProperty           = 0x00B8,
    HvCallGetCpuGroupAffinity           = 0x00B9,
    HvCallGetNextCpuGroup               = 0x00BA,
    HvCallGetNextCpuGroupPartition      = 0x00BB,

    //
    // V5 Nested Dynamic Memory.
    //

    HvCallAddPhysicalMemory             = 0x00BC,

    //
    // V6 Intercept Completion.
    //

    HvCallCompleteIntercept             = 0x00BD,

    //
    // V6 Guest Physical Address Space Management IDs
    //

    HvCallPrecommitGpaPages             = 0x00BE,
    HvCallUncommitGpaPages              = 0x00BF,

    //
    // ARM64 Parent asserted interrupt hypercalls.
    //

    HvCallConfigureVirtualInterruptLine = 0x00C0,
    HvCallSetVirtualInterruptLineState  = 0x00C1,

    //
    // V6 Integrated (Root) Scheduler
    //

    HvCallDispatchVp                    = 0x00C2,
    HvCallProcessIommuPrq               = 0x00C3,

    //
    // V6 Device Domains.
    //

    HvCallDetachDeviceDomain            = 0x00C4,
    HvCallDeleteDeviceDomain            = 0x00C5,
    HvCallQueryDeviceDomain             = 0x00C6,
    HvCallMapSparseDeviceGpaPages       = 0x00C7,
    HvCallUnmapSparseDeviceGpaPages     = 0x00C8,

    //
    // V6 Page Access Tracking.
    //

    HvCallGetGpaPagesAccessState        = 0x00C9,
    HvCallGetSparseGpaPagesAccessState  = 0x00CA,

    //
    // V6 TestFramework hypercall
    //

    HvCallInvokeTestFramework           = 0x00CB,

    //
    // V7 Virtual Secure Mode (VSM) hypercalls.
    //

    HvCallQueryVtlProtectionMaskRange   = 0x00CC,
    HvCallModifyVtlProtectionMaskRange  = 0x00CD,

    //
    // V7 Device Domains.
    //

    HvCallConfigureDeviceDomain         = 0x00CE,
    HvCallQueryDeviceDomainProperties   = 0x00CF,
    HvCallFlushDeviceDomain             = 0x00D0,
    HvCallFlushDeviceDomainList         = 0x00D1,

    //
    // V7 Virtual Secure Mode (VSM) hypercalls.
    //

    HvCallAcquireSparseGpaPageHostAccess = 0x00D2,
    HvCallReleaseSparseGpaPageHostAccess = 0x00D3,
    HvCallCheckSparseGpaPageVtlAccess    = 0x00D4,

    //
    // V7 Device Domains.
    //

    HvCallEnableDeviceInterrupt          = 0x00D5,

    //
    // V7 ARM enlightened TLB flush.
    //

    HvCallFlushTlb                       = 0x00D6,

    //
    // V8 Isolated VM (IVM)
    //

    HvCallAcquireSparseSpaPageHostAccess    = 0x00D7,
    HvCallReleaseSparseSpaPageHostAccess    = 0x00D8,
    HvCallAcceptGpaPages                    = 0x00D9,
    HvCallUnacceptGpaPages                  = 0x00DA,
    HvCallModifySparseGpaPageHostVisibility = 0x00DB,
    HvCallLockSparseGpaPageMapping          = 0x00DC,
    HvCallUnlockSparseGpaPageMapping        = 0x00DD,

    //
    // V8 Hypervisor Idle State Management hypercalls.
    //

    HvCallRequestProcessorHalt = 0x00DE,

    //
    // V8 Intercept Completion.
    //

    HvCallGetInterceptData              = 0x00DF,

    //
    // Total of all hypercalls
    //

    HvCallCount

} HV_CALL_CODE, *PHV_CALL_CODE;


//
// Define maximum argument count and size for using volatile and nonvolatile
// XMM registers to pass arguments in hypercalls.
//

#if defined(XBOX_SYSTEMOS)

#define HV_MAX_VOLATILE_XMM_COUNT 14    // (6 * 2) + 2
#define HV_MAX_NONVOLATILE_XMM_COUNT 34 // (16 * 2 ) + 2
#define HV_MAX_VOLATILE_XMM_SIZE (HV_MAX_VOLATILE_XMM_COUNT * 8)
#define HV_MAX_NONVOLATILE_XMM_SIZE (HV_MAX_NONVOLATILE_XMM_COUNT * 8)

#endif

//
// Declare constants and structures for submitting hypercalls.
//
#define HV_X64_MAX_HYPERCALL_ELEMENTS ((1<<12) - 1)

typedef union _HV_HYPERCALL_INPUT
{
    //
    // Input: The call code, argument sizes and calling convention
    //
    struct
    {
        UINT32 CallCode        : 16; // Least significant bits
        UINT32 IsFast          : 1;  // Uses the register based form
        UINT32 Reserved1       : 14;
        UINT32 IsNested        : 1;  // The outer hypervisor handles this call.
        UINT32 CountOfElements : 12;
        UINT32 Reserved2       : 4;
        UINT32 RepStartIndex   : 12;
        UINT32 Reserved3       : 4;  // Most significant bits
    };

    UINT64 AsUINT64;

} HV_HYPERCALL_INPUT, *PHV_HYPERCALL_INPUT;

typedef union _HV_HYPERCALL_OUTPUT
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

} HV_HYPERCALL_OUTPUT, *PHV_HYPERCALL_OUTPUT;

//
// Added to let legacy code compile without requiring update
//
typedef HV_HYPERCALL_INPUT  HV_X64_HYPERCALL_INPUT;
typedef HV_HYPERCALL_OUTPUT HV_X64_HYPERCALL_OUTPUT;

#if defined(_AMD64_) || defined(_ARM64_)

#define HVCALL_FAST_BUFFER(_Variable, _ByteCount) \
    __declspec(align(16)) UINT64 _Variable[(((_ByteCount) + 15) / 16) * 2]


#endif


//
// Define GVA range structures.
//
// A GVA range describes a contiguous range of GVA pages. HV_GVA_RANGE_EXTENDED
// describes up to 0x800 4K/2M/1G pages. HV_GVA_RANGE_SIMPLE describes up to
// 0x1000 4K pages. For ranges that fit in both formats, both have the same
// value.
//

typedef union _HV_GVA_RANGE_SIMPLE
{
    UINT64 AsUINT64;

    struct
    {
        //
        // AdditionalPages supplies the number of pages beyond one.
        //

        UINT64 AdditionalPages : 12;

        //
        // GvaPageNumber supplies the top 52 most significant bits of the guest
        // virtual address.
        //

        UINT64 GvaPageNumber   : 52;
    };
} HV_GVA_RANGE_SIMPLE, *PHV_GVA_RANGE_SIMPLE;

typedef union _HV_GVA_RANGE_EXTENDED
{
    UINT64 AsUINT64;

    struct
    {
        //
        // AdditionalPages supplies the number of pages beyond one.
        //

        UINT64 AdditionalPages      : 11;

        //
        // LargePage indicates page size greater than 4 KB.
        //

        UINT64 LargePage            : 1;

        //
        // GvaPageNumber supplies the top 52 most significant bits of the guest
        // virtual address when LargePage is clear.
        //

        UINT64 GvaPageNumber        : 52;
    };

    struct
    {
        UINT64                      : 12;

        //
        // PageSize supplies the page size when LargePage is set.
        //
        //  0b0 - 2 MB
        //  0b1 - 1 GB
        //

        UINT64 PageSize             : 1;

        UINT64 Reserved             : 8;

        //
        // GvaLargePageNumber supplies the top 43 most significant bits of the
        // guest virtual address when LargePage is set.
        //

        UINT64 GvaLargePageNumber   : 43;
    };
} HV_GVA_RANGE_EXTENDED, *PHV_GVA_RANGE_EXTENDED;

#define HV_GVA_RANGE_LARGE_PAGE_SIZE_2MB    0
#define HV_GVA_RANGE_LARGE_PAGE_SIZE_1GB    1

#define HV_GVA_RANGE_SIMPLE_ADDITIONAL_PAGES_MASK   0xFFF
#define HV_GVA_RANGE_EXTENDED_ADDITIONAL_PAGES_MASK 0x7FF

typedef union _HV_GVA_RANGE
{
    UINT64 AsUINT64;

    HV_GVA_RANGE_SIMPLE   Simple;
    HV_GVA_RANGE_EXTENDED Extended;

} HV_GVA_RANGE, *PHV_GVA_RANGE;

//
// The following structure contains the definition of a GPA page range.
//

typedef union _HV_GPA_PAGE_RANGE
{
    struct
    {
        UINT64 AdditionalPages : 11;
        UINT64 LargePage : 1;
        UINT64 BasePfn : 52;
    };

    struct
    {
        UINT64 : 12;
        UINT64 PageSize : 1;
        UINT64 Reserved : 8;
        UINT64 BaseLargePfn : 43;
    };

    UINT64 AsUINT64;

} HV_GPA_PAGE_RANGE, *PHV_GPA_PAGE_RANGE;

#define HV_GPA_PAGE_RANGE_MAX_PAGES         2048
#define HV_GPA_PAGE_RANGE_PAGE_SIZE_2MB     0
#define HV_GPA_PAGE_RANGE_PAGE_SIZE_1GB     1

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

#if defined(XBOX_SYSTEMOS)

#define HV_FLUSH_MASK                        (HV_FLUSH_ALL_PROCESSORS | \
                                              HV_FLUSH_ALL_VIRTUAL_ADDRESS_SPACES | \
                                              HV_FLUSH_NON_GLOBAL_MAPPINGS_ONLY)

#define HV_FLUSH_VA_MAXIMUM (31)        // maximum flush multiple entry count

#endif

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
// Define guest idle MSR. A guest virtual processor can enter idle state by
// reading this MSR, and will be woken up when an interrupt arrives
// regarldess interrupt is enabled or not.
//

#define HV_X64_MSR_GUEST_IDLE               HvSyntheticMsrGuestIdle

//
// Define guest scheduler MSR. A guest virtual processor can update
// its QoS on the corresponding VP-Backing thread in the root
// partition by writing to this MSR.
//

#define HV_X64_MSR_GUEST_SCHEDULER_EVENT HvSyntheticMsrGuestSchedulerEvent

//
//  Define the MSR value to indicate that the scheduler assist action is meant
//  to be applied on the current running processor.
//

#define HV_X64_MSR_GUEST_VALUE_CURRENT_PROCESSOR (UINT64)(HV_VP_INDEX_SELF)

#if defined(XBOX_SYSTEMOS)

#define HV_X64_MSR_SYNTH_DEBUG_TRANSITION XbSyntheticMsrSynthDebugTransition

#define HV_SYNTH_DEBUG_TRANSITION_ENTERED_DEBUGGER   0x1
#define HV_SYNTH_DEBUG_TRANSITION_HANDLING_EXCEPTION 0x2

#endif

#define HV_X64_MSR_STIMER0_CONFIG      (HvSyntheticMsrSTimer0Config)
#define HV_X64_MSR_STIMER0_COUNT       (HvSyntheticMsrSTimer0Count)
#define HV_X64_MSR_STIMER1_CONFIG      (HvSyntheticMsrSTimer1Config)
#define HV_X64_MSR_STIMER1_COUNT       (HvSyntheticMsrSTimer1Count)
#define HV_X64_MSR_STIMER2_CONFIG      (HvSyntheticMsrSTimer2Config)
#define HV_X64_MSR_STIMER2_COUNT       (HvSyntheticMsrSTimer2Count)
#define HV_X64_MSR_STIMER3_CONFIG      (HvSyntheticMsrSTimer3Config)
#define HV_X64_MSR_STIMER3_COUNT       (HvSyntheticMsrSTimer3Count)

#define HV_X64_MSR_STIME_UNHALTED_TIMER_CONFIG  HvSyntheticMsrSTimeUnhaltedTimerConfig
#define HV_X64_MSR_STIME_UNHALTED_TIMER_COUNT   HvSyntheticMsrSTimeUnhaltedTimerCount

//
// Define index of the reference time and reference TSC page MSRs.
//

#define HV_X64_MSR_TIME_REF_COUNT      (HvSyntheticMsrTimeRefCount)
#define HV_X64_MSR_REFERENCE_TSC       (HvSyntheticMsrReferenceTsc)

//
// Define index of the TSC and APIC frequency MSRs.
//

#define HV_X64_MSR_TSC_FREQUENCY       (HvSyntheticMsrTscFrequency)
#define HV_X64_MSR_APIC_FREQUENCY      (HvSyntheticMsrApicFrequency)

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
    volatile UINT64 TimelineBias;
    UINT64 Reserved2[508];
} HV_REFERENCE_TSC_PAGE, *PHV_REFERENCE_TSC_PAGE;

//
// Define the synthetic interrupt source index type.
//

typedef UINT32 HV_SYNIC_SINT_INDEX, *PHV_SYNIC_SINT_INDEX;

//
// Define the number of synthetic interrupt sources.
//

#define HV_SYNIC_SINT_COUNT (16)

//
// Define index of synthetic interrupt source that receives intercept messages.
//

#define HV_SYNIC_INTERCEPTION_SINT_INDEX    ((HV_SYNIC_SINT_INDEX)0)
#define HV_SYNIC_SHARED_SINT_INDEX          ((HV_SYNIC_SINT_INDEX)1)
#define HV_SYNIC_VMBUS_SINT_INDEX           ((HV_SYNIC_SINT_INDEX)2)
#define HV_SYNIC_HAL_HV_TIMER_SINT_INDEX    ((HV_SYNIC_SINT_INDEX)3)
#define HV_SYNIC_HVL_SHARED_SINT_INDEX      ((HV_SYNIC_SINT_INDEX)4)
#define HV_SYNIC_IOMMU_FAULT_SINT_INDEX     ((HV_SYNIC_SINT_INDEX)5)
#define HV_SYNIC_FIRST_UNUSED_SINT_INDEX    ((HV_SYNIC_SINT_INDEX)6)

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

#if defined(_ARM64_)

    HvArm64InterruptTypeFixed             = 0x0000,

    //
    // Maximum (exclusive) value of interrupt type.
    //
    HvArm64InterruptTypeMaximum           = 0x008,

#else

    HvX64InterruptTypeFixed             = 0x0000,
    HvX64InterruptTypeLowestPriority    = 0x0001,
    HvX64InterruptTypeSmi               = 0x0002,
    HvX64InterruptTypeNmi               = 0x0004,
    HvX64InterruptTypeInit              = 0x0005,
    HvX64InterruptTypeSipi              = 0x0006,
    HvX64InterruptTypeExtInt            = 0x0007,
    HvX64InterruptTypeLocalInt0         = 0x0008,
    HvX64InterruptTypeLocalInt1         = 0x0009,

    //
    // Maximum (exclusive) value of interrupt type.
    //
    HvX64InterruptTypeMaximum           = 0x000A

#endif

} HV_INTERRUPT_TYPE, *PHV_INTERRUPT_TYPE;

#if defined(_ARM64_)

#define HvInterruptTypeFixed HvArm64InterruptTypeFixed

#else

#define HvInterruptTypeFixed HvX64InterruptTypeFixed

#endif

//
// Define synthetic interrupt source.
//

typedef union _HV_SYNIC_SINT
{
    UINT64 AsUINT64;
    struct
    {

#if defined(_ARM64_)

        UINT64 Vector       :10;
        UINT64 ReservedP1   :6;

#else

        UINT64 Vector       :8;
        UINT64 ReservedP1   :8;

#endif

        UINT64 Masked       :1;
        UINT64 AutoEoi      :1;
        UINT64 Polling      :1;
        UINT64 AsIntercept  :1;
        UINT64 Proxy        :1;
        UINT64 ReservedP2   :43;
    };
} HV_SYNIC_SINT, *PHV_SYNIC_SINT;

//
// Define version of the synthetic interrupt controller.
//

#define HV_SYNIC_VERSION        (1)


//
// Define synthetic interrupt controller model specific registers.
//

#define HV_X64_MSR_SCONTROL   (HvSyntheticMsrSControl)
#define HV_X64_MSR_SVERSION   (HvSyntheticMsrSVersion)
#define HV_X64_MSR_SIEFP      (HvSyntheticMsrSiefp)
#define HV_X64_MSR_SIMP       (HvSyntheticMsrSimp)
#define HV_X64_MSR_EOM        (HvSyntheticMsrEom)
#define HV_X64_MSR_SIRBP      (HvSyntheticMsrSirb)
#define HV_X64_MSR_SINT0      (HvSyntheticMsrSint0)
#define HV_X64_MSR_SINT1      (HvSyntheticMsrSint1)
#define HV_X64_MSR_SINT2      (HvSyntheticMsrSint2)
#define HV_X64_MSR_SINT3      (HvSyntheticMsrSint3)
#define HV_X64_MSR_SINT4      (HvSyntheticMsrSint4)
#define HV_X64_MSR_SINT5      (HvSyntheticMsrSint5)
#define HV_X64_MSR_SINT6      (HvSyntheticMsrSint6)
#define HV_X64_MSR_SINT7      (HvSyntheticMsrSint7)
#define HV_X64_MSR_SINT8      (HvSyntheticMsrSint8)
#define HV_X64_MSR_SINT9      (HvSyntheticMsrSint9)
#define HV_X64_MSR_SINT10     (HvSyntheticMsrSint10)
#define HV_X64_MSR_SINT11     (HvSyntheticMsrSint11)
#define HV_X64_MSR_SINT12     (HvSyntheticMsrSint12)
#define HV_X64_MSR_SINT13     (HvSyntheticMsrSint13)
#define HV_X64_MSR_SINT14     (HvSyntheticMsrSint14)
#define HV_X64_MSR_SINT15     (HvSyntheticMsrSint15)

#define HV_X64_MSR_NESTED_SCONTROL   (HvSyntheticMsrNestedSControl)
#define HV_X64_MSR_NESTED_SVERSION   (HvSyntheticMsrNestedSVersion)
#define HV_X64_MSR_NESTED_SIEFP      (HvSyntheticMsrNestedSiefp)
#define HV_X64_MSR_NESTED_SIMP       (HvSyntheticMsrNestedSimp)
#define HV_X64_MSR_NESTED_EOM        (HvSyntheticMsrNestedEom)
#define HV_X64_MSR_NESTED_SIRBP      (HvSyntheticMsrNestedSirb)
#define HV_X64_MSR_NESTED_SINT0      (HvSyntheticMsrNestedSint0)
#define HV_X64_MSR_NESTED_SINT1      (HvSyntheticMsrNestedSint1)
#define HV_X64_MSR_NESTED_SINT2      (HvSyntheticMsrNestedSint2)
#define HV_X64_MSR_NESTED_SINT3      (HvSyntheticMsrNestedSint3)
#define HV_X64_MSR_NESTED_SINT4      (HvSyntheticMsrNestedSint4)
#define HV_X64_MSR_NESTED_SINT5      (HvSyntheticMsrNestedSint5)
#define HV_X64_MSR_NESTED_SINT6      (HvSyntheticMsrNestedSint6)
#define HV_X64_MSR_NESTED_SINT7      (HvSyntheticMsrNestedSint7)
#define HV_X64_MSR_NESTED_SINT8      (HvSyntheticMsrNestedSint8)
#define HV_X64_MSR_NESTED_SINT9      (HvSyntheticMsrNestedSint9)
#define HV_X64_MSR_NESTED_SINT10     (HvSyntheticMsrNestedSint10)
#define HV_X64_MSR_NESTED_SINT11     (HvSyntheticMsrNestedSint11)
#define HV_X64_MSR_NESTED_SINT12     (HvSyntheticMsrNestedSint12)
#define HV_X64_MSR_NESTED_SINT13     (HvSyntheticMsrNestedSint13)
#define HV_X64_MSR_NESTED_SINT14     (HvSyntheticMsrNestedSint14)
#define HV_X64_MSR_NESTED_SINT15     (HvSyntheticMsrNestedSint15)


//
// Define the expected SynIC version.
//
#define HV_SYNIC_VERSION_1 (0x1)

//
// Reenlightenment controls.
//

typedef union _HV_REENLIGHTENMENT_CONTROL
{
    UINT64 AsUINT64;

    struct
    {
#if defined(_ARM64_)

        UINT64 Vector       :10;
        UINT64 RsvdZ1       :6;

#else

        UINT64 Vector       :8;
        UINT64 RsvdZ1       :8;

#endif
        UINT64 Enabled      :1;
        UINT64 RsvdZ2       :15;
        UINT64 TargetVp     :32;
    };

} HV_REENLIGHTENMENT_CONTROL, *PHV_REENLIGHTENMENT_CONTROL;

typedef union _HV_TSC_EMULATION_CONTROL
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 Enabled      :1;
        UINT64 RsvdZ        :63;
    };

} HV_TSC_EMULATION_CONTROL, *PHV_TSC_EMULATION_CONTROL;

typedef union _HV_TSC_EMULATION_STATUS
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 InProgress   : 1;
        UINT64 RsvdP1       : 63;
    };

} HV_TSC_EMULATION_STATUS, *PHV_TSC_EMULATION_STATUS;

#define HV_X64_MSR_REENLIGHTENMENT_CONTROL (HvSyntheticMsrReenlightenmentControl)
#define HV_X64_MSR_TSC_EMULATION_CONTROL   (HvSyntheticMsrTscEmulationControl)
#define HV_X64_MSR_TSC_EMULATION_STATUS    (HvSyntheticMsrTscEmulationStatus)

//
// An architecture is a set of processor instruction sets and operating modes
//

typedef enum _HV_ARCHITECTURE
{
    HvArchitectureX64,
    HvArchitectureX86,
    HvArchitectureARM64,
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
        UINT64 MpxBndreg:1;
        UINT64 MpxBndcsr:1;
        UINT64 Avx512Opmask:1;
        UINT64 Avx512Zmmhi:1;
        UINT64 Avx512Zmm16_31:1;
        UINT64 Reserved:56;
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
        UINT8  Reserved;
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
// XSAVE header (64).
//
#define HV_X64_XSAVE_AREA_HEADER_SIZE  64
#define HV_X64_XSAVE_AREA_SIZE         (HV_X64_FXSAVE_AREA_SIZE + \
                                        HV_X64_XSAVE_AREA_HEADER_SIZE)

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
        HV_UINT128                          Xmm[16];
        HV_UINT128                          Reserved[6];

        HV_X64_XSAVE_HEADER                 Header;
    };

    UINT8 XSaveArea[HV_X64_XSAVE_AREA_SIZE];
} HV_X64_X_REGISTERS, *PHV_X64_X_REGISTERS;


//
// XSAVE save area - The size of the XSAVE save area will vary depending on the
// supported processor features on a system.
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

typedef struct _HV_VP_CONTEXT
{

#if defined(_ARM64_)

    UINT64 Pc;
    UINT64 Sctlr;
    UINT64 Ttbr0;
    UINT64 Ttbr1;
    UINT64 Tcr;
    UINT64 Mair;

#else

    UINT64 Rip;
    UINT64 Cr0;
    UINT64 Cr3;
    UINT64 Cr4;
    UINT64 Efer;
    UINT64 MsrCrPat;

#endif

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
            UINT64 ApicVector   : 8;
            UINT64 DirectMode   : 1;
            UINT64 ReservedZ1   : 3;
            UINT64 SINTx        : 4;
            UINT64 ReservedZ2   :44;
        };
    };
} HV_X64_MSR_STIMER_CONFIG_CONTENTS, *PHV_X64_MSR_STIMER_CONFIG_CONTENTS;

//
// Define the number of synthetic timers.
//

#define HV_SYNIC_STIMER_COUNT (4)

//
// Define the synthetic time unhalted timer configuration structure.
//

typedef union _HV_X64_MSR_STIME_UNHALTED_TIMER_CONFIG_CONTENTS
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 Vector : 8;
        UINT64 Enabled : 1;
        UINT64 Reserved : 55;
    };

} HV_X64_MSR_STIME_UNHALTED_TIMER_CONFIG_CONTENTS, *PHV_X64_MSR_STIME_UNHALTED_TIMER_CONFIG_CONTENTS;

#define HV_STIME_UNHALTED_TIMER_MIN_COUNT       1000 // 100us

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

#if defined(_ARM64_)
    HvMessageTypeMmioIntercept = 0x80000002,
#endif

    HvMessageTypeUnacceptedGpa = 0x80000003,
    HvMessageTypeGpaAttributeIntercept = 0x80000004,

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
    HvMessageTypeIommuFault = 0x80000024,

    //
    // Trace buffer complete messages.
    //
    HvMessageTypeEventLogBufferComplete = 0x80000040,

    //
    // Hypercall intercept.
    //
    HvMessageTypeHypercallIntercept = 0x80000050,

    //
    // Synic intercepts.
    //
    HvMessageTypeSynicEventIntercept = 0x80000060,

    //
    // Integrated (root) scheduler signal VP-backing thread(s) messages.
    //
    // N.B. Message id range [0x80000100, 0x800001FF] inclusively is reserved for
    //      the integrated (root) scheduler messages.
    //
    HvMessageTypeSchedulerIdRangeStart = 0x80000100,
    HvMessageTypeSchedulerVpSignalBitset = 0x80000100,
    HvMessageTypeSchedulerVpSignalPair = 0x80000101,
    HvMessageTypeSchedulerIdRangeEnd = 0x800001FF,

    //
    // Platform-specific processor intercept messages.
    //

    HvMessageTypeMsrIntercept = 0x80010001,
    HvMessageTypeExceptionIntercept = 0x80010003,
    HvMessageTypeRegisterIntercept = 0x80010006,

#if defined(_AMD64_) || defined(_X86_)

    //
    // (AMD64/X86).
    //
    HvMessageTypeX64IoPortIntercept = 0x80010000,
    HvMessageTypeX64CpuidIntercept = 0x80010002,
    HvMessageTypeX64ApicEoi = 0x80010004,
    HvMessageTypeX64IommuPrq = 0x80010006,
    HvMessageTypeX64Halt = 0x80010007,
    HvMessageTypeX64InterruptionDeliverable = 0x80010008,
    HvMessageTypeX64SipiIntercept = 0x80010009,

#elif defined(_ARM64_)

    HvMessageTypeArm64ResetIntercept = 0x80010000,

#endif

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
// Define IOMMU PRQ message payload structure.
//

typedef struct _HV_IOMMU_PRQ_MESSAGE_PAYLOAD
{
    HV_IOMMU_ID IommuId;
} HV_IOMMU_PRQ_MESSAGE_PAYLOAD, *PHV_IOMMU_PRQ_MESSAGE_PAYLOAD;

//
// Define IOMMU fault message payload structure.
//

typedef enum _HV_IOMMU_FAULT_TYPE
{
    //
    // The IOMMU did not obtain a translation for a DMA transaction.
    //
    HvIommuTranslationFault,

    //
    // Translation request, translated request or untranslated request
    // explicitly blocked.
    //
    HvIommuTranslationBlocked,

    //
    // Hardware blocked an interrupt request.
    //
    HvIommuInterruptFault,

#if defined(_ARM64_)

    //
    // The IOMMU retrieved a transation for a DMA transaction, but the
    // transaction has insufficient privileges.
    //
    HvIommuPermissionFault,

    //
    // An output address contained an unexpected number of bits.
    //
    HvIommuAddressSizeFault,

    //
    // A TLB match conflict was detected.
    //
    HvIommuTlbMatchConflict,

    //
    // An external abort / unsupported upstream transaction was reported to
    // the IOMMU during transaction processing.
    //
    HvIommuExternalFault,
    HvIommuUnsupportedUpstreamTransaction,

#endif

} HV_IOMMU_FAULT_TYPE, *PHV_IOMMU_FAULT_TYPE;

typedef struct _HV_IOMMU_FAULT_MESSAGE_PAYLOAD
{
    //
    // Indicates the type of the fault.
    //
    HV_IOMMU_FAULT_TYPE Type;

    //
    // Access type of the DMA transaction.
    //
    HV_INTERCEPT_ACCESS_TYPE AccessType;

    //
    // Fault flags.
    //
    struct
    {
        //
        // Indicates that the fault address is valid.
        //
        UINT32 FaultAddressValid : 1;

        //
        // Indicates that the logical device ID is valid.
        //
        UINT32 DeviceIdValid : 1;

    } Flags;

    //
    // Logical ID of the device that caused the fault.
    //
    UINT64 LogicalDeviceId;

    //
    // Device virtual address that caused the fault (if known).
    //
    HV_GVA FaultAddress;

} HV_IOMMU_FAULT_MESSAGE_PAYLOAD, *PHV_IOMMU_FAULT_MESSAGE_PAYLOAD;

//
// Define synthetic interrupt controller message format.
// N.B. The Payload may contain XMM registers that the compiler might expect to
// to be aligned. Therefore, this structure must be 16-byte aligned. The header
// is 16B already.
//

typedef struct DECLSPEC_ALIGN(16) _HV_MESSAGE
{
    HV_MESSAGE_HEADER Header;
    union
    {
        UINT64 Payload[HV_MESSAGE_PAYLOAD_QWORD_COUNT];
        HV_TIMER_MESSAGE_PAYLOAD TimerPayload;
        HV_EVENTLOG_MESSAGE_PAYLOAD TracePayload;
        HV_IOMMU_PRQ_MESSAGE_PAYLOAD IommuPrqPayload;
        HV_IOMMU_FAULT_MESSAGE_PAYLOAD IommuFaultPayload;
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

#define HV_X64_MSR_NPIEP_CONFIG HvSyntheticMsrNpiepConfig

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

#if !defined(KDNET_IPV6_ADDRESS)

#define KDNET_IPV6_ADDRESS
typedef struct _IPV6_ADDRESS
{
    union {
        struct {
            UINT64 QW0;
            UINT64 QW1;
        };

        struct {
            UINT32 DW0;
            UINT32 DW1;
            UINT32 DW2;
            UINT32 DW3;
        };

        struct {
            UINT16 W0;
            UINT16 W1;
            UINT16 W2;
            UINT16 W3;
            UINT16 W4;
            UINT16 W5;
            UINT16 W6;
            UINT16 W7;
        };
    };
} IPV6_ADDRESS, *PIPV6_ADDRESS;

#endif

typedef struct _HV_DEBUG_NET_DATA
{
    IPV6_ADDRESS HostIp;
    IPV6_ADDRESS TargetIp;
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

//
// ARM64HV_WORKITEM
// rename??
//
#define HV_X64_MSR_CRASH_P0                       HvSyntheticMsrCrashP0
#define HV_X64_MSR_CRASH_P1                       HvSyntheticMsrCrashP1
#define HV_X64_MSR_CRASH_P2                       HvSyntheticMsrCrashP2
#define HV_X64_MSR_CRASH_P3                       HvSyntheticMsrCrashP3
#define HV_X64_MSR_CRASH_P4                       HvSyntheticMsrCrashP4
#define HV_X64_MSR_CRASH_CTL                      HvSyntheticMsrCrashCtl

#define HV_X64_MSR_CRASH_ADDR_TO_INDEX(_Addr)   ((_Addr) - HV_X64_MSR_CRASH_P0)
#define HV_X64_MSR_CRASH_INDEX_TO_ADDR(_Idx)    ((_Idx) + HV_X64_MSR_CRASH_P0)
#define HV_X64_MSR_CRASH_NUM_PARAMETER_REGS     5

typedef union _HV_CRASH_CTL_REG_CONTENTS
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Reserved       : 63; // Reserved bits
        UINT64 CrashNotify    : 1;  // Log contents of crash parameter system registers
    };
} HV_CRASH_CTL_REG_CONTENTS, *PHV_CRASH_CTL_REG_CONTENTS;

//
// Define hypervisor memory zeroing enlightenment interface
//
#define HV_MSR_MEMORY_ZEROING_CONTROL         HvSyntheticMsrMemoryZeroingControl

typedef union _HV_MEMORY_ZEROING_CTRL_REG_CONTENTS
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 ZeroMemoryOnReset : 1;  // Zero memory on reset
        UINT64 Reserved          : 63; // Reserved bits
    };
} HV_MEMORY_ZEROING_CTRL_REG_CONTENTS, *PHV_MEMORY_ZEROING_CTRL_REG_CONTENTS;

//
// Define FS base MSR and execute only enable MSR.
//

#if defined(XBOX_SYSTEMOS)

#define HV_X64_MSR_FSBASE               XbSyntheticMsrFsBase

//
// N.B. The use of distinguish values is to work around a backward compatibility
//      problem caused by code that enabled execute only in builds that were not
//      compiled with a compiler that supported the features necessary for switch
//      tables.
//

#define HV_X64_MSR_XONLY_ON             0x11
#define HV_X64_MSR_XONLY_OFF            0x10

#define HV_X64_MSR_XONLY                XbSyntheticMsrXOnly

#endif

//
// VSM Definitions
//
// Define a virtual trust level (VTL)
//

typedef UINT8 HV_VTL, *PHV_VTL;
#define HV_NUM_VTLS 3
#define HV_NUM_VTLS_ROOT 2
#define HV_INVALID_VTL ((HV_VTL) -1)
#define HV_VTL_ALL 0xF

//
// Input for targeting a specific VTL.
//

typedef union _HV_INPUT_VTL
{
    UINT8 AsUINT8;
    struct
    {
        UINT8 TargetVtl    : 4;
        UINT8 UseTargetVtl : 1;
        UINT8 ReservedZ    : 3;
    };
} HV_INPUT_VTL, *PHV_INPUT_VTL;

//
// Initial X64 VP context for a newly enabled VTL
//

typedef struct _HV_INITIAL_VP_CONTEXT
{

#if defined(_ARM64_)

    UINT64 Pc;
    UINT64 Sp_ELh;
    UINT64 SCTLR_EL1;
    UINT64 MAIR_EL1;
    UINT64 TCR_EL1;
    UINT64 VBAR_EL1;
    UINT64 TTBR0_EL1;
    UINT64 TTBR1_EL1;
    UINT64 X18;

#else

    UINT64 Rip;
    UINT64 Rsp;
    UINT64 Rflags;

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
    // Global and Interrupt Descriptor tables
    //

    HV_X64_TABLE_REGISTER Idtr;
    HV_X64_TABLE_REGISTER Gdtr;

    //
    // Control registers and MSR's
    //

    UINT64 Efer;
    UINT64 Cr0;
    UINT64 Cr3;
    UINT64 Cr4;
    UINT64 MsrCrPat;

#endif

} HV_INITIAL_VP_CONTEXT, *PHV_INITIAL_VP_CONTEXT;

//
// Definition of the HvCallEnablePartitionVtl input structure.  This hypercall
// enables a VTL for a partition.
//

typedef union _HV_ENABLE_PARTITION_VTL_FLAGS
{
    UINT8 AsUINT8;
    struct
    {
        UINT8 EnableMbec:1;
        UINT8 Reserved:7;
    };
} HV_ENABLE_PARTITION_VTL_FLAGS, *PHV_ENABLE_PARTITION_VTL_FLAGS;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_ENABLE_PARTITION_VTL
{
    HV_PARTITION_ID                 PartitionId;
    HV_VTL                          TargetVtl;
    HV_ENABLE_PARTITION_VTL_FLAGS   Flags;
    UINT16                          ReservedZ0;
    UINT32                          ReservedZ1;
} HV_INPUT_ENABLE_PARTITION_VTL, *PHV_INPUT_ENABLE_PARTITION_VTL;


//
// Definition of the HvCallDisablePartitionVtl input structure.  This hypercall
// disables a VTL for a partition.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_DISABLE_PARTITION_VTL
{
    HV_PARTITION_ID       PartitionId;
    HV_VTL                TargetVtl;
    UINT8                 ReservedZ0;
    UINT16                ReservedZ1;
    UINT32                ReservedZ2;
} HV_INPUT_DISABLE_PARTITION_VTL, *PHV_INPUT_DISABLE_PARTITION_VTL;


//
// Definition of the HvCallEnableVpVtl input structure.  This hypercall enables a
// VTL on a virtual processor.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_ENABLE_VP_VTL
{
    HV_PARTITION_ID           PartitionId;
    HV_VP_INDEX               VpIndex;
    HV_VTL                    TargetVtl;
    UINT8                     ReservedZ0;
    UINT16                    ReservedZ1;
    HV_INITIAL_VP_CONTEXT     VpVtlContext;
} HV_INPUT_ENABLE_VP_VTL, *PHV_INPUT_ENABLE_VP_VTL;

//
// Definition of the HvCallDisableVpVtl input structure.  This hypercall disables
// a VTL on a virtual processor.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_DISABLE_VP_VTL
{
    HV_PARTITION_ID           PartitionId;
    HV_VP_INDEX               VpIndex;
    HV_VTL                    TargetVtl;
    UINT8                     ReservedZ0;
    UINT16                    ReservedZ1;
} HV_INPUT_DISABLE_VP_VTL, *PHV_INPUT_DISABLE_VP_VTL;


//
// Definition of input to a VTL return
//
typedef union _HV_VTL_RETURN_INPUT
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 FastReturn  : 1;
        UINT64 ReservedZ   : 63;
    };
} HV_VTL_RETURN_INPUT, *PHV_VTL_RETURN_INPUT;


//
// VSM Register Content Definitions
//

//
// HvRegisterVsmCodePageOffsets - this register provides the byte offsets into
//   the hypercall code page for the various VSM functions provided by the
//   hypercall code page.  This is a read-only partition-wide register.  There
//   is a separate instance of this register for each VTL.
//

typedef union _HV_REGISTER_VSM_CODE_PAGE_OFFSETS
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 VtlCallOffset   : 12;
        UINT64 VtlReturnOffset : 12;
        UINT64 ReservedZ       : 40;
    };
} HV_REGISTER_VSM_CODE_PAGE_OFFSETS, *PHV_REGISTER_VSM_CODE_PAGE_OFFSETS;


//
// HvRegisterVsmPartitionStatus - this register provides VSM status for a
//   partition, including which VTL's are enabled.  This is a read-only
//   partition-wide register that is shared across all VTL's.
//

typedef union _HV_REGISTER_VSM_PARTITION_STATUS
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 EnabledVtlSet    : 16;
        UINT64 MaximumVtl       : 4;
        UINT64 ReservedZ        : 44;
    };
} HV_REGISTER_VSM_PARTITION_STATUS, *PHV_REGISTER_VSM_PARTITION_STATUS;


//
// HvRegisterVsmVpStatus - this register provides VSM status for a virtual
//   processor, including which VTL's have been enabled.  This is a read-only
//   per-VP register that is shared across all VTL's.
//

typedef union _HV_REGISTER_VSM_VP_STATUS
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 ActiveVtl                    : 4;
        UINT64 ActiveMbecEnabled            : 1;
        UINT64 ReservedZ0                   : 11;
        UINT64 EnabledVtlSet                : 16;
        UINT64 ReservedZ1                   : 32;
    };
} HV_REGISTER_VSM_VP_STATUS, *PHV_REGISTER_VSM_VP_STATUS;


//
// HvRegisterVsmVina - this register configures the virtual interrupt
//   notification assist (VINA) facility for a VTL on a VP.
//

typedef union _HV_REGISTER_VSM_VINA
{
    UINT64 AsUINT64;
    struct
    {

#if defined(_ARM_) || defined(_ARM64_)

        UINT64 Vector          : 10;

#else

        UINT64 Vector          : 8;

#endif

        UINT64 Enabled         : 1;
        UINT64 AutoReset       : 1;

#if defined(_ARM_) || defined(_ARM64_)

        UINT64 ReservedP       : 52;

#else

        UINT64 AutoEoi         : 1;
        UINT64 ReservedP       : 53;

#endif

    };
} HV_REGISTER_VSM_VINA, *PHV_REGISTER_VSM_VINA;


//
// HvRegisterVsmCapabilities - this register reports capabilities about VSM.
//   This is a read-only partition-wide register.
//

#if defined(_AMD64_) || defined(_X86_)

typedef union _HV_X64_REGISTER_VSM_CAPABILITIES
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Dr6Shared            : 1;
        UINT64 MbecVtlMask          : 16;
        UINT64 DenyLowerVtlStartup  : 1;
        UINT64 ReservedZ            : 46;
    };
} HV_X64_REGISTER_VSM_CAPABILITIES, *PHV_X64_REGISTER_VSM_CAPABILITIES;

typedef HV_X64_REGISTER_VSM_CAPABILITIES HV_REGISTER_VSM_CAPABILITIES, *PHV_REGISTER_VSM_CAPABILITIES;

#else

typedef union _HV_ARM64_REGISTER_VSM_CAPABILITIES
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 MbecVtlMask          : 16;
        UINT64 DenyLowerVtlStartup  : 1;
        UINT64 ReservedZ            : 47;
    };
} HV_ARM64_REGISTER_VSM_CAPABILITIES, *PHV_ARM64_REGISTER_VSM_CAPABILITIES;

typedef HV_ARM64_REGISTER_VSM_CAPABILITIES HV_REGISTER_VSM_CAPABILITIES, *PHV_REGISTER_VSM_CAPABILITIES;

#endif

//
// [HvRegisterVsmVpSecureConfigVtl0...HvRegisterVsmVpSecureConfigVtl14]
// This defines the register contents for per-VTL config registers that can be
// used by a higher VTL to configure a lower VTL.  These registers are
// read/write and are per-VP.
//
// Each secure VTL has N secure VTL config registers, one for each lower VTL.
// For example, VTL 1 has a single secure VTL config register for VTL 0.  VTL 2
// has two secure VTL config registers, one for VTL 0 and one for VTL 1.
//

typedef union _HV_REGISTER_VSM_VP_SECURE_VTL_CONFIG
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 MbecEnabled  : 1;
        UINT64 TlbLocked    : 1;
        UINT64 ReservedZ    : 62;
    };

} HV_REGISTER_VSM_VP_SECURE_VTL_CONFIG, *PHV_REGISTER_VSM_VP_SECURE_VTL_CONFIG;

//
// HvRegisterVsmVpWaitForTlbLock - setting this register causes the target VTL to wait on the TLB
// locks of all VP-VTL pairs.
//

typedef union _HV_REGISTER_VSM_VP_WAIT_FOR_TLB_LOCK
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Wait         : 1;
        UINT64 ReservedZ    : 63;
    };

} HV_REGISTER_VSM_VP_WAIT_FOR_TLB_LOCK, *PHV_REGISTER_VSM_VP_WAIT_FOR_TLB_LOCK;

//
// HvRegisterVsmPartitionConfig - this register configures partition wide
// attributes for a VTL. There is one instance of this register for each
// secure VTL (VTL > 0) on each partition.
//

typedef union _HV_REGISTER_VSM_PARTITION_CONFIG
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 EnableVtlProtection      : 1;
        UINT64 DefaultVtlProtectionMask : 4;
        UINT64 ZeroMemoryOnReset        : 1;
        UINT64 DenyLowerVtlStartup      : 1;
        UINT64 InterceptAcceptance      : 1;
        UINT64 InterceptEnableVtlProtection : 1;
        UINT64 InterceptVpStartup       : 1;
        UINT64 ReservedZ                : 54;
    };

} HV_REGISTER_VSM_PARTITION_CONFIG, *PHV_REGISTER_VSM_PARTITION_CONFIG;

//
// HvRegisterIsolationCapabilities - This is a read-only partition-wide register that reports
// capabilities about isolated VM.
//

typedef union _HV_REGISTER_ISOLATION_CAPABILITIES
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 ReservedZ    : 64;
    };

} HV_REGISTER_ISOLATION_CAPABILITIES, *PHV_REGISTER_ISOLATION_CAPABILITIES;


//
// This indicates the reason why a VTL was entered.
//

typedef enum _HV_VTL_ENTRY_REASON
{
    //
    // This reason is reserved and is not used.
    //

    HvVtlEntryReserved             = 0,

    //
    // Indicates entry due to a VTL call from a lower VTL.
    //

    HvVtlEntryVtlCall              = 1,

    //
    // Indicates entry due to an interrupt targeted to the VTL.
    //

    HvVtlEntryInterrupt            = 2
} HV_VTL_ENTRY_REASON, *PHV_VTL_ENTRY_REASON;

typedef struct _HV_VP_VTL_CONTROL
{
    //
    // The hypervisor updates the entry reason with an indication as to why the
    // VTL was entered on the virtual processor.
    //

    HV_VTL_ENTRY_REASON     EntryReason;

    //
    // This flag determines whether the VINA interrupt line is asserted.
    //

    union
    {
        UINT8               AsUINT8;
        struct
        {
            UINT8           VinaAsserted  :1;
            UINT8           VinaReservedZ :7;
        };
    } VinaStatus;

    UINT8                   ReservedZ00;
    UINT16                  ReservedZ01;

#if defined(_AMD64_)

    //
    // A guest updates the VtlReturn* fields to provide the register values to
    // restore on VTL return.  The specific register values that are restored
    // will vary based on whether the VTL is 32-bit or 64-bit.
    //

    union
    {
        struct
        {
            UINT64          VtlReturnX64Rax;
            UINT64          VtlReturnX64Rcx;
        };

        struct
        {
            UINT32          VtlReturnX86Eax;
            UINT32          VtlReturnX86Ecx;
            UINT32          VtlReturnX86Edx;
            UINT32          ReservedZ1;
        };
    };

#else

    //
    // Return control registers not needed on ARM64.
    //

    UINT64 ReservedZ2;
    UINT64 ReservedZ3;

#endif

} HV_VP_VTL_CONTROL, *PHV_VP_VTL_CONTROL;

//
// Control structure that allows a hypervisor to indicate to its parent
// hypervisor which nested enlightenment privileges are to be granted to the
// current nested guest context.
//

typedef struct _HV_NESTED_ENLIGHTENMENTS_CONTROL
{
    union
    {
        UINT32 AsUINT32;

        struct
        {
            UINT32 DirectHypercall:1;
            UINT32 VirtualizationException:1;
            UINT32 Reserved:30;
        };

    } Features;

    union
    {
        UINT32 AsUINT32;

        struct
        {
            UINT32 InterPartitionCommunication:1;
            UINT32 Reserved:31;
        };

    } HypercallControls;

} HV_NESTED_ENLIGHTENMENTS_CONTROL, *PHV_NESTED_ENLIGHTENMENTS_CONTROL;

//
// The virtualization fault information area contains the current fault code and
// fault parameters for the VP. It must be 16 byte aligned.
//

typedef union _HV_VIRTUALIZATION_FAULT_INFORMATION
{
    struct
    {
        UINT16 Parameter0;
        UINT16 Reserved0;
        UINT32 Code;
        UINT64 Parameter1;
    };

    //
    // Reserved for future use when we add support for Intel's architecturally-
    // defined #VE information area.
    //

    struct
    {
        UINT8 VeInformationArea[34];

        //
        // Explicit padding to make the whole structure 40 bytes in size. These
        // last 6 bytes are not included in save state.
        //

        UINT8 Reserved1[6];

    };

} HV_VIRTUALIZATION_FAULT_INFORMATION, *PHV_VIRTUALIZATION_FAULT_INFORMATION;

//
// The VP-assist page provides a shared memory page that can be used for
// bi-directional communication between a guest VP and the hypervisor.  Each
// VTL on a VP has its own assist page.
//

typedef union _HV_VP_ASSIST_PAGE
{
    struct
    {
        //
        // APIC assist for optimized EOI processing.
        //

        HV_VIRTUAL_APIC_ASSIST ApicAssist;

        UINT32 ReservedZ0;

        //
        // VP-VTL control information
        //

        HV_VP_VTL_CONTROL VtlControl;

        HV_NESTED_ENLIGHTENMENTS_CONTROL NestedEnlightenmentsControl;

        BOOLEAN EnlightenVmEntry;

        UINT8 ReservedZ1[7];

        HV_GPA CurrentNestedVmcs;

        BOOLEAN SyntheticTimeUnhaltedTimerExpired;

        UINT8 ReservedZ2[7];

        //
        // VirtualizationFaultInformation must be 16 byte aligned.
        //

        HV_VIRTUALIZATION_FAULT_INFORMATION VirtualizationFaultInformation;

    };

    UINT8 ReservedZBytePadding[HV_PAGE_SIZE];

} HV_VP_ASSIST_PAGE, *PHV_VP_ASSIST_PAGE;

//
// Hypervisor register names for accessing a virtual processor's registers.
//

typedef enum _HV_REGISTER_NAME
{
    // Suspend Registers
    HvRegisterExplicitSuspend           = 0x00000000,
    HvRegisterInterceptSuspend          = 0x00000001,
    HvRegisterInstructionEmulationHints = 0x00000002,
    HvRegisterDispatchSuspend           = 0x00000003,

    // Version
    HvRegisterHypervisorVersion = 0x00000100,   // 128-bit result same as CPUID 0x40000002

    // Feature Access (registers are 128 bits)
    HvRegisterPrivilegesAndFeaturesInfo = 0x00000200,   // 128-bit result same as CPUID 0x40000003
    HvRegisterFeaturesInfo              = 0x00000201,   // 128-bit result same as CPUID 0x40000004
    HvRegisterImplementationLimitsInfo  = 0x00000202,   // 128-bit result same as CPUID 0x40000005
    HvRegisterHardwareFeaturesInfo      = 0x00000203,   // 128-bit result same as CPUID 0x40000006
    HvRegisterCpuManagementFeaturesInfo = 0x00000204,   // 128-bit result same as CPUID 0x40000007
    HvRegisterSvmFeaturesInfo           = 0x00000205,   // 128-bit result same as CPUID 0x40000008
    HvRegisterSkipLevelFeaturesInfo     = 0x00000206,   // 128-bit result same as CPUID 0x40000009
    HvRegisterNestedVirtFeaturesInfo    = 0x00000207,   // 128-bit result same as CPUID 0x4000000A
    HvRegisterIsolationConfiguration    = 0x00000209,   // 128-bit result same as CPUID 0x4000000C

    // Guest Crash Registers
    HvRegisterGuestCrashP0  = 0x00000210,
    HvRegisterGuestCrashP1  = 0x00000211,
    HvRegisterGuestCrashP2  = 0x00000212,
    HvRegisterGuestCrashP3  = 0x00000213,
    HvRegisterGuestCrashP4  = 0x00000214,
    HvRegisterGuestCrashCtl = 0x00000215,

    // Power State Configuration
    HvRegisterPowerStateConfigC1  = 0x00000220,
    HvRegisterPowerStateTriggerC1 = 0x00000221,
    HvRegisterPowerStateConfigC2  = 0x00000222,
    HvRegisterPowerStateTriggerC2 = 0x00000223,
    HvRegisterPowerStateConfigC3  = 0x00000224,
    HvRegisterPowerStateTriggerC3 = 0x00000225,

    // Frequency Registers
    HvRegisterProcessorClockFrequency = 0x00000240,
    HvRegisterInterruptClockFrequency = 0x00000241,

    // Idle Register
    HvRegisterGuestIdle = 0x00000250,

    // Guest Debug
    HvRegisterDebugDeviceOptions = 0x00000260,

    // Memory Zeroing Conrol Register
    HvRegisterMemoryZeroingControl = 0x00000270,

    // Pending Interruption Register
    HvRegisterPendingInterruption = 0x00010002,

    // Interrupt State register
    HvRegisterInterruptState = 0x00010003,

    // Pending Event Register
    HvRegisterPendingEvent0 = 0x00010004,
    HvRegisterPendingEvent1 = 0x00010005,

    // Misc
    HvRegisterVpRuntime            = 0x00090000,
    HvRegisterGuestOsId            = 0x00090002,
    HvRegisterVpIndex              = 0x00090003,
    HvRegisterTimeRefCount         = 0x00090004,
    HvRegisterCpuManagementVersion = 0x00090007,
    HvRegisterVpAssistPage         = 0x00090013,
    HvRegisterVpRootSignalCount    = 0x00090014,

    // Performance statistics Registers
    HvRegisterStatsPartitionRetail   = 0x00090020,
    HvRegisterStatsPartitionInternal = 0x00090021,
    HvRegisterStatsVpRetail          = 0x00090022,
    HvRegisterStatsVpInternal        = 0x00090023,

    HvRegisterNestedVpIndex          = 0x00091003,

    // Hypervisor-defined Registers (Synic)
    HvRegisterSint0    = 0x000A0000,
    HvRegisterSint1    = 0x000A0001,
    HvRegisterSint2    = 0x000A0002,
    HvRegisterSint3    = 0x000A0003,
    HvRegisterSint4    = 0x000A0004,
    HvRegisterSint5    = 0x000A0005,
    HvRegisterSint6    = 0x000A0006,
    HvRegisterSint7    = 0x000A0007,
    HvRegisterSint8    = 0x000A0008,
    HvRegisterSint9    = 0x000A0009,
    HvRegisterSint10   = 0x000A000A,
    HvRegisterSint11   = 0x000A000B,
    HvRegisterSint12   = 0x000A000C,
    HvRegisterSint13   = 0x000A000D,
    HvRegisterSint14   = 0x000A000E,
    HvRegisterSint15   = 0x000A000F,
    HvRegisterScontrol = 0x000A0010,
    HvRegisterSversion = 0x000A0011,
    HvRegisterSifp     = 0x000A0012,
    HvRegisterSipp     = 0x000A0013,
    HvRegisterEom      = 0x000A0014,
    HvRegisterSirbp    = 0x000A0015,

    HvRegisterNestedSint0    = 0x000A1000,
    HvRegisterNestedSint1    = 0x000A1001,
    HvRegisterNestedSint2    = 0x000A1002,
    HvRegisterNestedSint3    = 0x000A1003,
    HvRegisterNestedSint4    = 0x000A1004,
    HvRegisterNestedSint5    = 0x000A1005,
    HvRegisterNestedSint6    = 0x000A1006,
    HvRegisterNestedSint7    = 0x000A1007,
    HvRegisterNestedSint8    = 0x000A1008,
    HvRegisterNestedSint9    = 0x000A1009,
    HvRegisterNestedSint10   = 0x000A100A,
    HvRegisterNestedSint11   = 0x000A100B,
    HvRegisterNestedSint12   = 0x000A100C,
    HvRegisterNestedSint13   = 0x000A100D,
    HvRegisterNestedSint14   = 0x000A100E,
    HvRegisterNestedSint15   = 0x000A100F,
    HvRegisterNestedScontrol = 0x000A1010,
    HvRegisterNestedSversion = 0x000A1011,
    HvRegisterNestedSifp     = 0x000A1012,
    HvRegisterNestedSipp     = 0x000A1013,
    HvRegisterNestedEom      = 0x000A1014,
    HvRegisterNestedSirbp    = 0x000A1015,


    // Hypervisor-defined Registers (Synthetic Timers)
    HvRegisterStimer0Config            = 0x000B0000,
    HvRegisterStimer0Count             = 0x000B0001,
    HvRegisterStimer1Config            = 0x000B0002,
    HvRegisterStimer1Count             = 0x000B0003,
    HvRegisterStimer2Config            = 0x000B0004,
    HvRegisterStimer2Count             = 0x000B0005,
    HvRegisterStimer3Config            = 0x000B0006,
    HvRegisterStimer3Count             = 0x000B0007,
    HvRegisterStimeUnhaltedTimerConfig = 0x000B0100,
    HvRegisterStimeUnhaltedTimerCount  = 0x000B0101,

    //
    // Synthetic VSM registers
    //

    // 0x000D0000-1 are available for future use.
    HvRegisterVsmCodePageOffsets     = 0x000D0002,
    HvRegisterVsmVpStatus            = 0x000D0003,
    HvRegisterVsmPartitionStatus     = 0x000D0004,
    HvRegisterVsmVina                = 0x000D0005,
    HvRegisterVsmCapabilities        = 0x000D0006,
    HvRegisterVsmPartitionConfig     = 0x000D0007,

    HvRegisterVsmVpSecureConfigVtl0  = 0x000D0010,
    HvRegisterVsmVpSecureConfigVtl1  = 0x000D0011,
    HvRegisterVsmVpSecureConfigVtl2  = 0x000D0012,
    HvRegisterVsmVpSecureConfigVtl3  = 0x000D0013,
    HvRegisterVsmVpSecureConfigVtl4  = 0x000D0014,
    HvRegisterVsmVpSecureConfigVtl5  = 0x000D0015,
    HvRegisterVsmVpSecureConfigVtl6  = 0x000D0016,
    HvRegisterVsmVpSecureConfigVtl7  = 0x000D0017,
    HvRegisterVsmVpSecureConfigVtl8  = 0x000D0018,
    HvRegisterVsmVpSecureConfigVtl9  = 0x000D0019,
    HvRegisterVsmVpSecureConfigVtl10 = 0x000D001A,
    HvRegisterVsmVpSecureConfigVtl11 = 0x000D001B,
    HvRegisterVsmVpSecureConfigVtl12 = 0x000D001C,
    HvRegisterVsmVpSecureConfigVtl13 = 0x000D001D,
    HvRegisterVsmVpSecureConfigVtl14 = 0x000D001E,

    HvRegisterVsmVpWaitForTlbLock    = 0x000D0020,

    HvRegisterIsolationCapabilities  = 0x000D0100,

#if defined(_AMD64_) || defined(_X86_)

    // Interruptible notification register
    HvX64RegisterDeliverabilityNotifications = 0x00010006,

    // X64 User-Mode Registers
    HvX64RegisterRax                = 0x00020000,
    HvX64RegisterRcx                = 0x00020001,
    HvX64RegisterRdx                = 0x00020002,
    HvX64RegisterRbx                = 0x00020003,
    HvX64RegisterRsp                = 0x00020004,
    HvX64RegisterRbp                = 0x00020005,
    HvX64RegisterRsi                = 0x00020006,
    HvX64RegisterRdi                = 0x00020007,
    HvX64RegisterR8                 = 0x00020008,
    HvX64RegisterR9                 = 0x00020009,
    HvX64RegisterR10                = 0x0002000A,
    HvX64RegisterR11                = 0x0002000B,
    HvX64RegisterR12                = 0x0002000C,
    HvX64RegisterR13                = 0x0002000D,
    HvX64RegisterR14                = 0x0002000E,
    HvX64RegisterR15                = 0x0002000F,
    HvX64RegisterRip                = 0x00020010,
    HvX64RegisterRflags             = 0x00020011,

    // X64 Floating Point and Vector Registers
    HvX64RegisterXmm0               = 0x00030000,
    HvX64RegisterXmm1               = 0x00030001,
    HvX64RegisterXmm2               = 0x00030002,
    HvX64RegisterXmm3               = 0x00030003,
    HvX64RegisterXmm4               = 0x00030004,
    HvX64RegisterXmm5               = 0x00030005,
    HvX64RegisterXmm6               = 0x00030006,
    HvX64RegisterXmm7               = 0x00030007,
    HvX64RegisterXmm8               = 0x00030008,
    HvX64RegisterXmm9               = 0x00030009,
    HvX64RegisterXmm10              = 0x0003000A,
    HvX64RegisterXmm11              = 0x0003000B,
    HvX64RegisterXmm12              = 0x0003000C,
    HvX64RegisterXmm13              = 0x0003000D,
    HvX64RegisterXmm14              = 0x0003000E,
    HvX64RegisterXmm15              = 0x0003000F,
    HvX64RegisterFpMmx0             = 0x00030010,
    HvX64RegisterFpMmx1             = 0x00030011,
    HvX64RegisterFpMmx2             = 0x00030012,
    HvX64RegisterFpMmx3             = 0x00030013,
    HvX64RegisterFpMmx4             = 0x00030014,
    HvX64RegisterFpMmx5             = 0x00030015,
    HvX64RegisterFpMmx6             = 0x00030016,
    HvX64RegisterFpMmx7             = 0x00030017,
    HvX64RegisterFpControlStatus    = 0x00030018,
    HvX64RegisterXmmControlStatus   = 0x00030019,

    // X64 Control Registers
    HvX64RegisterCr0                = 0x00040000,
    HvX64RegisterCr2                = 0x00040001,
    HvX64RegisterCr3                = 0x00040002,
    HvX64RegisterCr4                = 0x00040003,
    HvX64RegisterCr8                = 0x00040004,
    HvX64RegisterXfem               = 0x00040005,

    // X64 Intermediate Control Registers
    HvX64RegisterIntermediateCr0    = 0x00041000,
    HvX64RegisterIntermediateCr4    = 0x00041003,
    HvX64RegisterIntermediateCr8    = 0x00041004,

    // X64 Debug Registers
    HvX64RegisterDr0                = 0x00050000,
    HvX64RegisterDr1                = 0x00050001,
    HvX64RegisterDr2                = 0x00050002,
    HvX64RegisterDr3                = 0x00050003,
    HvX64RegisterDr6                = 0x00050004,
    HvX64RegisterDr7                = 0x00050005,

    // X64 Segment Registers
    HvX64RegisterEs                 = 0x00060000,
    HvX64RegisterCs                 = 0x00060001,
    HvX64RegisterSs                 = 0x00060002,
    HvX64RegisterDs                 = 0x00060003,
    HvX64RegisterFs                 = 0x00060004,
    HvX64RegisterGs                 = 0x00060005,
    HvX64RegisterLdtr               = 0x00060006,
    HvX64RegisterTr                 = 0x00060007,

    // X64 Table Registers
    HvX64RegisterIdtr               = 0x00070000,
    HvX64RegisterGdtr               = 0x00070001,

    // X64 Virtualized MSRs
    HvX64RegisterTsc                = 0x00080000,
    HvX64RegisterEfer               = 0x00080001,
    HvX64RegisterKernelGsBase       = 0x00080002,
    HvX64RegisterApicBase           = 0x00080003,
    HvX64RegisterPat                = 0x00080004,
    HvX64RegisterSysenterCs         = 0x00080005,
    HvX64RegisterSysenterEip        = 0x00080006,
    HvX64RegisterSysenterEsp        = 0x00080007,
    HvX64RegisterStar               = 0x00080008,
    HvX64RegisterLstar              = 0x00080009,
    HvX64RegisterCstar              = 0x0008000A,
    HvX64RegisterSfmask             = 0x0008000B,
    HvX64RegisterInitialApicId      = 0x0008000C,

    //
    // X64 Cache control MSRs
    //
    HvX64RegisterMsrMtrrCap         = 0x0008000D,
    HvX64RegisterMsrMtrrDefType     = 0x0008000E,
    HvX64RegisterMsrMtrrPhysBase0   = 0x00080010,
    HvX64RegisterMsrMtrrPhysBase1   = 0x00080011,
    HvX64RegisterMsrMtrrPhysBase2   = 0x00080012,
    HvX64RegisterMsrMtrrPhysBase3   = 0x00080013,
    HvX64RegisterMsrMtrrPhysBase4   = 0x00080014,
    HvX64RegisterMsrMtrrPhysBase5   = 0x00080015,
    HvX64RegisterMsrMtrrPhysBase6   = 0x00080016,
    HvX64RegisterMsrMtrrPhysBase7   = 0x00080017,
    HvX64RegisterMsrMtrrPhysBase8   = 0x00080018,
    HvX64RegisterMsrMtrrPhysBase9   = 0x00080019,
    HvX64RegisterMsrMtrrPhysBaseA   = 0x0008001A,
    HvX64RegisterMsrMtrrPhysBaseB   = 0x0008001B,
    HvX64RegisterMsrMtrrPhysBaseC   = 0x0008001C,
    HvX64RegisterMsrMtrrPhysBaseD   = 0x0008001D,
    HvX64RegisterMsrMtrrPhysBaseE   = 0x0008001E,
    HvX64RegisterMsrMtrrPhysBaseF   = 0x0008001F,
    HvX64RegisterMsrMtrrPhysMask0   = 0x00080040,
    HvX64RegisterMsrMtrrPhysMask1   = 0x00080041,
    HvX64RegisterMsrMtrrPhysMask2   = 0x00080042,
    HvX64RegisterMsrMtrrPhysMask3   = 0x00080043,
    HvX64RegisterMsrMtrrPhysMask4   = 0x00080044,
    HvX64RegisterMsrMtrrPhysMask5   = 0x00080045,
    HvX64RegisterMsrMtrrPhysMask6   = 0x00080046,
    HvX64RegisterMsrMtrrPhysMask7   = 0x00080047,
    HvX64RegisterMsrMtrrPhysMask8   = 0x00080048,
    HvX64RegisterMsrMtrrPhysMask9   = 0x00080049,
    HvX64RegisterMsrMtrrPhysMaskA   = 0x0008004A,
    HvX64RegisterMsrMtrrPhysMaskB   = 0x0008004B,
    HvX64RegisterMsrMtrrPhysMaskC   = 0x0008004C,
    HvX64RegisterMsrMtrrPhysMaskD   = 0x0008004D,
    HvX64RegisterMsrMtrrPhysMaskE   = 0x0008004E,
    HvX64RegisterMsrMtrrPhysMaskF   = 0x0008004F,
    HvX64RegisterMsrMtrrFix64k00000 = 0x00080070,
    HvX64RegisterMsrMtrrFix16k80000 = 0x00080071,
    HvX64RegisterMsrMtrrFix16kA0000 = 0x00080072,
    HvX64RegisterMsrMtrrFix4kC0000  = 0x00080073,
    HvX64RegisterMsrMtrrFix4kC8000  = 0x00080074,
    HvX64RegisterMsrMtrrFix4kD0000  = 0x00080075,
    HvX64RegisterMsrMtrrFix4kD8000  = 0x00080076,
    HvX64RegisterMsrMtrrFix4kE0000  = 0x00080077,
    HvX64RegisterMsrMtrrFix4kE8000  = 0x00080078,
    HvX64RegisterMsrMtrrFix4kF0000  = 0x00080079,
    HvX64RegisterMsrMtrrFix4kF8000  = 0x0008007A,

    HvX64RegisterTscAux             = 0x0008007B,
    HvX64RegisterBndcfgs            = 0x0008007C,
    HvX64RegisterDebugCtl           = 0x0008007D,

    //
    // Available
    //
    HvX64RegisterAvailable0008007E  = 0x0008007E,
    HvX64RegisterAvailable0008007F  = 0x0008007F,

    HvX64RegisterSgxLaunchControl0  = 0x00080080,
    HvX64RegisterSgxLaunchControl1  = 0x00080081,
    HvX64RegisterSgxLaunchControl2  = 0x00080082,
    HvX64RegisterSgxLaunchControl3  = 0x00080083,
    HvX64RegisterSpecCtrl           = 0x00080084,
    HvX64RegisterPredCmd            = 0x00080085,

    //
    // Other MSRs
    //
    HvX64RegisterMsrIa32MiscEnable  = 0x000800A0,
    HvX64RegisterIa32FeatureControl = 0x000800A1,
    HvX64RegisterIa32VmxBasic          = 0x000800A2,
    HvX64RegisterIa32VmxPinbasedCtls   = 0x000800A3,
    HvX64RegisterIa32VmxProcbasedCtls  = 0x000800A4,
    HvX64RegisterIa32VmxExitCtls       = 0x000800A5,
    HvX64RegisterIa32VmxEntryCtls      = 0x000800A6,
    HvX64RegisterIa32VmxMisc           = 0x000800A7,
    HvX64RegisterIa32VmxCr0Fixed0      = 0x000800A8,
    HvX64RegisterIa32VmxCr0Fixed1      = 0x000800A9,
    HvX64RegisterIa32VmxCr4Fixed0      = 0x000800AA,
    HvX64RegisterIa32VmxCr4Fixed1      = 0x000800AB,
    HvX64RegisterIa32VmxVmcsEnum       = 0x000800AC,
    HvX64RegisterIa32VmxProcbasedCtls2 = 0x000800AD,
    HvX64RegisterIa32VmxEptVpidCap     = 0x000800AE,
    HvX64RegisterIa32VmxTruePinbasedCtls   = 0x000800AF,
    HvX64RegisterIa32VmxTrueProcbasedCtls  = 0x000800B0,
    HvX64RegisterIa32VmxTrueExitCtls       = 0x000800B1,
    HvX64RegisterIa32VmxTrueEntryCtls      = 0x000800B2,

    //
    // Performance monitoring MSRs
    //
    HvX64RegisterPerfGlobalCtrl     = 0x00081000,
    HvX64RegisterPerfGlobalStatus   = 0x00081001,
    HvX64RegisterPerfGlobalInUse    = 0x00081002,
    HvX64RegisterFixedCtrCtrl       = 0x00081003,
    HvX64RegisterDsArea             = 0x00081004,
    HvX64RegisterPebsEnable         = 0x00081005,
    HvX64RegisterPebsLdLat          = 0x00081006,
    HvX64RegisterPebsFrontend       = 0x00081007,
    HvX64RegisterPerfEvtSel0        = 0x00081100,
    HvX64RegisterPmc0               = 0x00081200,
    HvX64RegisterFixedCtr0          = 0x00081300,

    HvX64RegisterLbrTos             = 0x00082000,
    HvX64RegisterLbrSelect          = 0x00082001,
    HvX64RegisterLerFromLip         = 0x00082002,
    HvX64RegisterLerToLip           = 0x00082003,
    HvX64RegisterLbrFrom0           = 0x00082100,
    HvX64RegisterLbrTo0             = 0x00082200,
    HvX64RegisterLbrInfo0           = 0x00083300,

    //
    // Hypervisor-defined registers (Misc)
    //
    HvX64RegisterHypercall          = 0x00090001,

    //
    // X64 Virtual APIC registers MSRs
    //
    HvX64RegisterEoi                = 0x00090010,
    HvX64RegisterIcr                = 0x00090011,
    HvX64RegisterTpr                = 0x00090012,

    //
    // Partition Timer Assist Registers
    //
    HvX64RegisterEmulatedTimerPeriod    = 0x00090030,
    HvX64RegisterEmulatedTimerControl   = 0x00090031,
    HvX64RegisterPmTimerAssist          = 0x00090032,

    //
    // Intercept Control Registers
    //

    HvX64RegisterCrInterceptControl            = 0x000E0000,
    HvX64RegisterCrInterceptCr0Mask            = 0x000E0001,
    HvX64RegisterCrInterceptCr4Mask            = 0x000E0002,
    HvX64RegisterCrInterceptIa32MiscEnableMask = 0x000E0003,

#elif defined(_ARM64_)
    // ARM64 Registers

    HvArm64RegisterX0 = 0x00020000,
    HvArm64RegisterX1 = 0x00020001,
    HvArm64RegisterX2 = 0x00020002,
    HvArm64RegisterX3 = 0x00020003,
    HvArm64RegisterX4 = 0x00020004,
    HvArm64RegisterX5 = 0x00020005,
    HvArm64RegisterX6 = 0x00020006,
    HvArm64RegisterX7 = 0x00020007,
    HvArm64RegisterX8 = 0x00020008,
    HvArm64RegisterX9 = 0x00020009,
    HvArm64RegisterX10 = 0x0002000A,
    HvArm64RegisterX11 = 0x0002000B,
    HvArm64RegisterX12 = 0x0002000C,
    HvArm64RegisterX13 = 0x0002000D,
    HvArm64RegisterX14 = 0x0002000E,
    HvArm64RegisterX15 = 0x0002000F,
    HvArm64RegisterX16 = 0x00020010,
    HvArm64RegisterX17 = 0x00020011,
    HvArm64RegisterX18 = 0x00020012,
    HvArm64RegisterX19 = 0x00020013,
    HvArm64RegisterX20 = 0x00020014,
    HvArm64RegisterX21 = 0x00020015,
    HvArm64RegisterX22 = 0x00020016,
    HvArm64RegisterX23 = 0x00020017,
    HvArm64RegisterX24 = 0x00020018,
    HvArm64RegisterX25 = 0x00020019,
    HvArm64RegisterX26 = 0x0002001A,
    HvArm64RegisterX27 = 0x0002001B,
    HvArm64RegisterX28 = 0x0002001C,
    HvArm64RegisterXFp = 0x0002001D,
    HvArm64RegisterXLr = 0x0002001E,
    HvArm64RegisterXSp = 0x0002001F, // alias for either El0/x depending on Cpsr.SPSel
    HvArm64RegisterXSpEl0 = 0x00020020,
    HvArm64RegisterXSpElx = 0x00020021,
    HvArm64RegisterXPc = 0x00020022,
    HvArm64RegisterCpsr = 0x00020023,

    HvArm64RegisterQ0 = 0x00030000,
    HvArm64RegisterQ1 = 0x00030001,
    HvArm64RegisterQ2 = 0x00030002,
    HvArm64RegisterQ3 = 0x00030003,
    HvArm64RegisterQ4 = 0x00030004,
    HvArm64RegisterQ5 = 0x00030005,
    HvArm64RegisterQ6 = 0x00030006,
    HvArm64RegisterQ7 = 0x00030007,
    HvArm64RegisterQ8 = 0x00030008,
    HvArm64RegisterQ9 = 0x00030009,
    HvArm64RegisterQ10 = 0x0003000A,
    HvArm64RegisterQ11 = 0x0003000B,
    HvArm64RegisterQ12 = 0x0003000C,
    HvArm64RegisterQ13 = 0x0003000D,
    HvArm64RegisterQ14 = 0x0003000E,
    HvArm64RegisterQ15 = 0x0003000F,
    HvArm64RegisterQ16 = 0x00030010,
    HvArm64RegisterQ17 = 0x00030011,
    HvArm64RegisterQ18 = 0x00030012,
    HvArm64RegisterQ19 = 0x00030013,
    HvArm64RegisterQ20 = 0x00030014,
    HvArm64RegisterQ21 = 0x00030015,
    HvArm64RegisterQ22 = 0x00030016,
    HvArm64RegisterQ23 = 0x00030017,
    HvArm64RegisterQ24 = 0x00030018,
    HvArm64RegisterQ25 = 0x00030019,
    HvArm64RegisterQ26 = 0x0003001A,
    HvArm64RegisterQ27 = 0x0003001B,
    HvArm64RegisterQ28 = 0x0003001C,
    HvArm64RegisterQ29 = 0x0003001D,
    HvArm64RegisterQ30 = 0x0003001E,
    HvArm64RegisterQ31 = 0x0003001F,
    HvArm64RegisterFpControl = 0x00030020,
    HvArm64RegisterFpStatus = 0x00030021,

    // Debug Registers
    HvArm64RegisterBcr0 = 0x00050000,
    HvArm64RegisterBcr1 = 0x00050001,
    HvArm64RegisterBcr2 = 0x00050002,
    HvArm64RegisterBcr3 = 0x00050003,
    HvArm64RegisterBcr4 = 0x00050004,
    HvArm64RegisterBcr5 = 0x00050005,
    HvArm64RegisterBcr6 = 0x00050006,
    HvArm64RegisterBcr7 = 0x00050007,
    HvArm64RegisterBcr8 = 0x00050008,
    HvArm64RegisterBcr9 = 0x00050009,
    HvArm64RegisterBcr10 = 0x0005000A,
    HvArm64RegisterBcr11 = 0x0005000B,
    HvArm64RegisterBcr12 = 0x0005000C,
    HvArm64RegisterBcr13 = 0x0005000D,
    HvArm64RegisterBcr14 = 0x0005000E,
    HvArm64RegisterBcr15 = 0x0005000F,
    HvArm64RegisterWcr0 = 0x00050010,
    HvArm64RegisterWcr1 = 0x00050011,
    HvArm64RegisterWcr2 = 0x00050012,
    HvArm64RegisterWcr3 = 0x00050013,
    HvArm64RegisterWcr4 = 0x00050014,
    HvArm64RegisterWcr5 = 0x00050015,
    HvArm64RegisterWcr6 = 0x00050016,
    HvArm64RegisterWcr7 = 0x00050017,
    HvArm64RegisterWcr8 = 0x00050018,
    HvArm64RegisterWcr9 = 0x00050019,
    HvArm64RegisterWcr10 = 0x0005001A,
    HvArm64RegisterWcr11 = 0x0005001B,
    HvArm64RegisterWcr12 = 0x0005001C,
    HvArm64RegisterWcr13 = 0x0005001D,
    HvArm64RegisterWcr14 = 0x0005001E,
    HvArm64RegisterWcr15 = 0x0005001F,
    HvArm64RegisterBvr0 = 0x00050020,
    HvArm64RegisterBvr1 = 0x00050021,
    HvArm64RegisterBvr2 = 0x00050022,
    HvArm64RegisterBvr3 = 0x00050023,
    HvArm64RegisterBvr4 = 0x00050024,
    HvArm64RegisterBvr5 = 0x00050025,
    HvArm64RegisterBvr6 = 0x00050026,
    HvArm64RegisterBvr7 = 0x00050027,
    HvArm64RegisterBvr8 = 0x00050028,
    HvArm64RegisterBvr9 = 0x00050029,
    HvArm64RegisterBvr10 = 0x0005002A,
    HvArm64RegisterBvr11 = 0x0005002B,
    HvArm64RegisterBvr12 = 0x0005002C,
    HvArm64RegisterBvr13 = 0x0005002D,
    HvArm64RegisterBvr14 = 0x0005002E,
    HvArm64RegisterBvr15 = 0x0005002F,
    HvArm64RegisterWvr0 = 0x00050030,
    HvArm64RegisterWvr1 = 0x00050031,
    HvArm64RegisterWvr2 = 0x00050032,
    HvArm64RegisterWvr3 = 0x00050033,
    HvArm64RegisterWvr4 = 0x00050034,
    HvArm64RegisterWvr5 = 0x00050035,
    HvArm64RegisterWvr6 = 0x00050036,
    HvArm64RegisterWvr7 = 0x00050037,
    HvArm64RegisterWvr8 = 0x00050038,
    HvArm64RegisterWvr9 = 0x00050039,
    HvArm64RegisterWvr10 = 0x0005003A,
    HvArm64RegisterWvr11 = 0x0005003B,
    HvArm64RegisterWvr12 = 0x0005003C,
    HvArm64RegisterWvr13 = 0x0005003D,
    HvArm64RegisterWvr14 = 0x0005003E,
    HvArm64RegisterWvr15 = 0x0005003F,

    // Control Registers
    HvArm64RegisterMidr = 0x00040000,
    HvArm64RegisterMpidr = 0x00040001,
    HvArm64RegisterSctlr = 0x00040002,
    HvArm64RegisterActlr = 0x00040003,
    HvArm64RegisterCpacr = 0x00040004,
    HvArm64RegisterTtbr0 = 0x00040005,
    HvArm64RegisterTtbr1 = 0x00040006,
    HvArm64RegisterTcr = 0x00040007,
    HvArm64RegisterEsrEl1 = 0x00040008,
    HvArm64RegisterFarEl1 = 0x00040009,
    HvArm64RegisterParEl1 = 0x0004000A,
    HvArm64RegisterMair = 0x0004000B,
    HvArm64RegisterVbar = 0x0004000C,
    HvArm64RegisterContextIdr = 0x0004000D,
    HvArm64RegisterTpidr = 0x0004000E,
    HvArm64RegisterCntkctl = 0x0004000F,
    HvArm64RegisterTpidrroEl0 = 0x00040010,
    HvArm64RegisterTpidrEl0 = 0x00040011,
    HvArm64RegisterFpcrEl1 = 0x00040012,
    HvArm64RegisterFpsrEl1 = 0x00040013,
    HvArm64RegisterSpsrEl1 = 0x00040014,
    HvArm64RegisterElrEl1 = 0x00040015,
    HvArm64RegisterAfsr0 = 0x00040016,
    HvArm64RegisterAfsr1 = 0x00040017,
    HvArm64RegisterAMair = 0x00040018,
    HvArm64RegisterMdscr = 0x00040019,

    // Trap control
    HvArm64RegisterMdcr = 0x00040101,
    HvArm64RegisterCptr = 0x00040102,
    HvArm64RegisterHstr = 0x00040103,
    HvArm64RegisterHacr = 0x00040104,

    // GIT Registers
    HvArm64RegisterCnthCtl = 0x000B0400,
    HvArm64RegisterCntkCtl = 0x000B0401,
    HvArm64RegisterCntpCtl = 0x000B0402,
    HvArm64RegisterCntpCval = 0x000B0403,
    HvArm64RegisterCntvOffset = 0x000B0404,
    HvArm64RegisterCntvCtl = 0x000B0405,
    HvArm64RegisterCntvCval = 0x000B0406,

    // Status Registers
    HvArm64RegisterCtr = 0x00040300,
    HvArm64RegisterDczid = 0x00040301,
    HvArm64RegisterRevidr = 0x00040302,
    HvArm64RegisterIdAa64pfr0 = 0x00040303,
    HvArm64RegisterIdAa64pfr1 = 0x00040304,
    HvArm64RegisterIdAa64dfr0 = 0x00040305,
    HvArm64RegisterIdAa64dfr1 = 0x00040306,
    HvArm64RegisterIdAa64afr0 = 0x00040307,
    HvArm64RegisterIdAa64afr1 = 0x00040308,
    HvArm64RegisterIdAa64isar0 = 0x00040309,
    HvArm64RegisterIdAa64isar1 = 0x0004030A,
    HvArm64RegisterIdAa64mmfr0 = 0x0004030B,
    HvArm64RegisterIdAa64mmfr1 = 0x0004030C,
    HvArm64RegisterClidr = 0x0004030D,
    HvArm64RegisterAidr = 0x0004030E,
    HvArm64RegisterCsselr = 0x0004030F,
    HvArm64RegisterCcsidr = 0x00040310,

    // Address to use for synthetic exceptions
    HvArm64RegisterSyntheticException = 0x00040400,

    // Misc
    HvArm64RegisterInterfaceVersion = 0x00090006,      // low 32 bits result same as CPUID 0x40000001
    HvArm64RegisterPartitionInfoPage = 0x00090015,
    HvArm64RegisterTlbiControl = 0x00090016,

#elif !defined(_ARM_)

#error Unsupported Architecture

#endif

} HV_REGISTER_NAME, *PHV_REGISTER_NAME;


//
// General Hypervisor Register Content Definitions
//

typedef union _HV_EXPLICIT_SUSPEND_REGISTER
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Suspended:1;
        UINT64 Reserved:63;
    };
} HV_EXPLICIT_SUSPEND_REGISTER, *PHV_EXPLICIT_SUSPEND_REGISTER;

typedef union _HV_INTERCEPT_SUSPEND_REGISTER
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Suspended:1;
        UINT64 Reserved:63;
    };
} HV_INTERCEPT_SUSPEND_REGISTER, *PHV_INTERCEPT_SUSPEND_REGISTER;

typedef union _HV_DISPATCH_SUSPEND_REGISTER
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Suspended:1;
        UINT64 Reserved:63;
    };
} HV_DISPATCH_SUSPEND_REGISTER, *PHV_DISPATCH_SUSPEND_REGISTER;

typedef union _HV_X64_INTERRUPT_STATE_REGISTER
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 InterruptShadow:1;
        UINT64 NmiMasked:1;
        UINT64 Reserved:62;
    };
} HV_X64_INTERRUPT_STATE_REGISTER, *PHV_X64_INTERRUPT_STATE_REGISTER;

#if !defined(_ARM64_)

typedef enum _HV_X64_PENDING_EVENT_TYPE
{
    HvX64PendingEventException                  = 0,
    HvX64PendingEventMemoryIntercept            = 1,
    HvX64PendingEventNestedMemoryIntercept      = 2,
    HvX64PendingEventVirtualizationFault        = 3,
    HvX64PendingEventHypercallOutput            = 4,

} HV_X64_PENDING_EVENT_TYPE, *PHV_X64_PENDING_EVENT_TYPE;


//
// Provides information about an exception.
//
typedef union _HV_X64_PENDING_EXCEPTION_EVENT
{
    UINT64 AsUINT64[2];
    struct
    {
        UINT32 EventPending         : 1;
        UINT32 EventType            : 3;
        UINT32 Reserved0            : 4;

        UINT32 DeliverErrorCode     : 1;
        UINT32 Reserved1            : 7;
        UINT32 Vector               : 16;
        UINT32 ErrorCode;
        UINT64 ExceptionParameter;
    };

} HV_X64_PENDING_EXCEPTION_EVENT, *PHV_X64_PENDING_EXCEPTION_EVENT;


//
// Provides information about a virtualization fault.
//
typedef union _HV_X64_PENDING_VIRTUALIZATION_FAULT_EVENT
{
    UINT64 AsUINT64[2];
    struct
    {
        UINT32 EventPending         : 1;
        UINT32 EventType            : 3;
        UINT32 Reserved0            : 4;

        UINT32 Reserved1            : 8;
        UINT32 Parameter0           : 16;
        UINT32 Code;
        UINT64 Parameter1;
    };

} HV_X64_PENDING_VIRTUALIZATION_FAULT_EVENT, *PHV_X64_PENDING_VIRTUALIZATION_FAULT_EVENT;


//
// Provides information about a memory intercept.
//
typedef union _HV_X64_PENDING_MEMORY_INTERCEPT_EVENT
{
    UINT64 AsUINT64[4];
    struct
    {
        UINT8 EventPending  : 1;
        UINT8 EventType     : 3;
        UINT8 Reserved0     : 4;

        //
        // VTL at which the memory intercept is targeted.
        // Note: This field must be in Reg0.
        //
        HV_VTL TargetVtl;

        //
        // Type of the memory access.
        //
        HV_INTERCEPT_ACCESS_TYPE AccessType;

        //
        // Flag indicating if the guest linear address is valid.
        //
        UINT8 GuestLinearAddressValid   : 1;

        //
        // Indicates that the memory intercept was caused by an access to a guest physical address
        // (instead of a page table as part of a page table walk).
        //
        UINT8 CausedByGpaAccess         : 1;

        UINT8 Reserved1                 : 6;

        UINT32 Reserved2;

        //
        // The guest linear address that caused the fault.
        //
        HV_GVA GuestLinearAddress;

        //
        // The guest physical address that caused the memory intercept.
        //
        HV_GPA GuestPhysicalAddress;

        UINT64 Reserved3;
    };

} HV_X64_PENDING_MEMORY_INTERCEPT_EVENT, *PHV_X64_PENDING_MEMORY_INTERCEPT_EVENT;


//
// Provides information about a memory intercept by a nested
// hypervisor (i.e. guest nested page fault).
//
typedef union _HV_X64_PENDING_NESTED_MEMORY_INTERCEPT_EVENT
{
    UINT64 AsUINT64[4];
    struct
    {
        UINT8 EventPending  : 1;
        UINT8 EventType     : 3;
        UINT8 Reserved0     : 4;

        //
        // Indicates if the access causing the guest nested page fault was a
        // data read, write or instruction fetch.
        //
        HV_INTERCEPT_ACCESS_TYPE AccessType;

        //
        // Access permission of the accessed page according to the guest
        // hypervisor's nested page table.
        //
        UINT8 GuestAccessPermission;

        //
        // Flag indicating if the guest linear address is valid.
        //
        UINT8 GuestLinearAddressValid   : 1;

        //
        // Indicates that the guest nested page fault was caused by an access
        // to a guest physical address (instead of a page table as part of a
        // page table walk).
        //
        UINT8 CausedByGpaAccess         : 1;

        //
        // Indicates that the guest nested page fault was caused by a
        // misconfiguration of the nested page table by the guest hypervisor.
        //
        UINT8 PageTableMisconfiguration : 1;

        UINT8 Reserved1                 : 5;

        UINT32 Reserved2;

        //
        // The guest linear address that caused the fault.
        //
        HV_GVA GuestLinearAddress;

        //
        // The guest nested physical address that caused the guest nested page
        // fault.
        //
        HV_GPA GuestNestedPhysicalAddress;

        UINT64 Reserved3;
    };

} HV_X64_PENDING_NESTED_MEMORY_INTERCEPT_EVENT, *PHV_X64_PENDING_NESTED_MEMORY_INTERCEPT_EVENT;


//
// Provides information about pending hypercall output.
//
typedef union _HV_X64_PENDING_HYPERCALL_OUTPUT_EVENT
{
    UINT64 AsUINT64[2];
    struct
    {
        UINT32 EventPending         : 1;
        UINT32 EventType            : 3;
        UINT32 Reserved0            : 4;

        //
        // Whether the hypercall has been retired.
        //
        UINT32 Retired              : 1;

        UINT32 Reserved1            : 23;

        //
        // Indicates the number of bytes to be written starting from OutputGpa.
        //
        UINT32 OutputSize;

        //
        // Indicates the output GPA, which is not required to be page-aligned.
        //
        HV_GPA OutputGpa;
    };

} HV_X64_PENDING_HYPERCALL_OUTPUT_EVENT, *PHV_X64_PENDING_HYPERCALL_OUTPUT_EVENT;


typedef union _HV_X64_PENDING_EVENT
{
    struct
    {
        HV_UINT128 Reg0;
        HV_UINT128 Reg1;
    };

    struct
    {
        UINT8 EventPending      : 1;
        UINT8 EventType         : 3;
        UINT8 Reserved          : 4;
        UINT8 EventData[15];
    };

    HV_X64_PENDING_EXCEPTION_EVENT Exception;
    HV_X64_PENDING_MEMORY_INTERCEPT_EVENT MemoryIntercept;
    HV_X64_PENDING_NESTED_MEMORY_INTERCEPT_EVENT NestedMemoryIntercept;
    HV_X64_PENDING_VIRTUALIZATION_FAULT_EVENT VirtualizationFault;
    HV_X64_PENDING_HYPERCALL_OUTPUT_EVENT HypercallOutput;

} HV_X64_PENDING_EVENT, *PHV_X64_PENDING_EVENT;


typedef enum _HV_X64_PENDING_INTERRUPTION_TYPE
{
    HvX64PendingInterrupt                   = 0,
    HvX64PendingNmi                         = 2,
    HvX64PendingException                   = 3,
    HvX64PendingSoftwareInterrupt           = 4,
    HvX64PendingPrivilegedSoftwareException = 5,
    HvX64PendingSoftwareException           = 6

} HV_X64_PENDING_INTERRUPTION_TYPE, *PHV_X64_PENDING_INTERRUPTION_TYPE;


typedef union _HV_X64_PENDING_INTERRUPTION_REGISTER
{
    UINT64 AsUINT64;
    struct
    {
        UINT32 InterruptionPending:1;
        UINT32 InterruptionType:3;
        UINT32 DeliverErrorCode:1;
        UINT32 InstructionLength:4;
        UINT32 NestedEvent:1;
        UINT32 Reserved:6;
        UINT32 InterruptionVector:16;
        UINT32 ErrorCode;
    };
} HV_X64_PENDING_INTERRUPTION_REGISTER, *PHV_X64_PENDING_INTERRUPTION_REGISTER;

#else

typedef enum _HV_ARM64_PENDING_EVENT_TYPE
{
    HvArm64PendingEventException                = 0,
    HvArm64PendingEventSecureException          = 1,
    HvArm64PendingEventCrashdumpException       = 2,
    HvArm64PendingEventHypercallOutput          = 3,
} HV_ARM64_PENDING_EVENT_TYPE, *PHV_ARM64_PENDING_EVENT_TYPE;

#define HV_ARM64_PENDING_EVENT_HEADER \
    UINT8 EventPending      : 1; \
    UINT8 EventType         : 3; \
    UINT8 Reserved          : 4

//
// Provides information about a PendingEventexception.
//
typedef union _HV_ARM64_PENDING_EXCEPTION_EVENT
{
    UINT64 AsUINT64[2];
    struct
    {
        HV_ARM64_PENDING_EVENT_HEADER;

        //
        // Syndrome detail (ESR_EL1)
        //
        UINT32 ExceptionSyndrome;

        //
        // For data abort syndromes.
        //
        UINT64 FaultAddress;
    };

} HV_ARM64_PENDING_EXCEPTION_EVENT, *PHV_ARM64_PENDING_EXCEPTION_EVENT;

//
// Provides information about a PendingEventSecureException
//
typedef union _HV_ARM64_PENDING_SECURE_EXCEPTION_EVENT
{
    UINT64 AsUINT64[2];
    struct
    {
        HV_ARM64_PENDING_EVENT_HEADER;
    };

} HV_ARM64_PENDING_SECURE_EXCEPTION_EVENT, *PHV_ARM64_PENDING_SECURE_EXCEPTION_EVENT;

//
// Provides information about pending hypercall output.
//
typedef union _HV_ARM64_PENDING_HYPERCALL_OUTPUT_EVENT
{
    UINT64 AsUINT64[2];
    struct
    {
        HV_ARM64_PENDING_EVENT_HEADER;

        //
        // Whether the hypercall has been retired.
        //
        UINT8 Retired : 1;

        UINT8 Reserved1 : 7;
        UINT16 Reserved2;

        //
        // Indicates the number of bytes to be written starting from OutputGpa.
        //
        UINT32 OutputSize;

        //
        // Indicates the output GPA, which is not required to be page-aligned.
        //
        HV_GPA OutputGpa;
    };

} HV_ARM64_PENDING_HYPERCALL_OUTPUT_EVENT, *PHV_ARM64_PENDING_HYPERCALL_OUTPUT_EVENT;

typedef union _HV_ARM64_PENDING_EVENT
{
    struct
    {
        HV_UINT128 Reg0;
    };

    struct
    {
        HV_ARM64_PENDING_EVENT_HEADER;
        UINT8 EventData[15];
    };

    HV_ARM64_PENDING_EXCEPTION_EVENT Exception;
    HV_ARM64_PENDING_SECURE_EXCEPTION_EVENT SecureException;
    HV_ARM64_PENDING_HYPERCALL_OUTPUT_EVENT HypercallOutput;

} HV_ARM64_PENDING_EVENT, *PHV_ARM64_PENDING_EVENT;

typedef union _HV_ARM64_INTERRUPT_STATE_REGISTER
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 InterruptShadow:1;
        UINT64 Reserved:63;
    };
} HV_ARM64_INTERRUPT_STATE_REGISTER, *PHV_ARM64_INTERRUPT_STATE_REGISTER;

typedef enum _HV_ARM64_PENDING_INTERRUPTION_TYPE
{
    HvArm64PendingInterrupt      = 0,
    HvArm64PendingException      = 1
} HV_ARM64_PENDING_INTERRUPTION_TYPE, *PHV_ARM64_PENDING_INTERRUPTION_TYPE;

typedef union _HV_ARM64_PENDING_INTERRUPTION_REGISTER
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 InterruptionPending : 1;
        UINT64 InterruptionType    : 1;
        UINT64 Reserved            : 30;
        UINT64 ErrorCode           : 32;   // ESR_ELx
    };
} HV_ARM64_PENDING_INTERRUPTION_REGISTER, *PHV_ARM64_PENDING_INTERRUPTION_REGISTER;

#endif

typedef union _HV_REGISTER_VALUE
{
    HV_UINT128                                  Reg128;
    UINT64                                      Reg64;
    UINT32                                      Reg32;
    UINT16                                      Reg16;
    UINT8                                       Reg8;

#if defined(_AMD64_) || defined(_X86_)

    HV_X64_FP_REGISTER                          Fp;
    HV_X64_FP_CONTROL_STATUS_REGISTER           FpControlStatus;
    HV_X64_XMM_CONTROL_STATUS_REGISTER          XmmControlStatus;
    HV_X64_SEGMENT_REGISTER                     Segment;
    HV_X64_TABLE_REGISTER                       Table;

#endif

    HV_EXPLICIT_SUSPEND_REGISTER                ExplicitSuspend;
    HV_INTERCEPT_SUSPEND_REGISTER               InterceptSuspend;
    HV_DISPATCH_SUSPEND_REGISTER                DispatchSuspend;

#if defined(_AMD64_) || defined(_X86_)

    HV_X64_INTERRUPT_STATE_REGISTER             InterruptState;
    HV_X64_PENDING_INTERRUPTION_REGISTER        PendingInterruption;
    HV_X64_MSR_NPIEP_CONFIG_CONTENTS            NpiepConfig;
    HV_X64_PENDING_EXCEPTION_EVENT              PendingExceptionEvent;
    HV_X64_PENDING_VIRTUALIZATION_FAULT_EVENT   PendingVirtualizationFaultEvent;

#elif defined(_ARM64_)

    HV_ARM64_PENDING_INTERRUPTION_REGISTER      PendingInterruption;
    HV_ARM64_INTERRUPT_STATE_REGISTER           InterruptState;
    HV_ARM64_PENDING_EXCEPTION_EVENT            PendingExceptionEvent;
    HV_ARM64_PENDING_SECURE_EXCEPTION_EVENT     PendingSecureExceptionEvent;

#endif

} HV_REGISTER_VALUE, *PHV_REGISTER_VALUE;

//
// Definiton of the HvCallSetVpRegister hypercall input structure.
// This call sets a Vp's register state.
//

typedef struct _HV_REGISTER_ASSOC
{
    HV_REGISTER_NAME    Name;
    UINT32              Pad;
    HV_REGISTER_VALUE   Value;
} HV_REGISTER_ASSOC, *PHV_REGISTER_ASSOC;

typedef struct HV_CALL_ATTRIBUTES_ALIGNED(16) _HV_INPUT_SET_VP_REGISTERS
{
    HV_PARTITION_ID      PartitionId;
    HV_VP_INDEX          VpIndex;
    HV_INPUT_VTL         InputVtl;
    UINT8                RsvdZ8;
    UINT16               RsvdZ16;
    HV_CALL_ATTRIBUTES
    HV_REGISTER_ASSOC    Elements[];
} HV_INPUT_SET_VP_REGISTERS, *PHV_INPUT_SET_VP_REGISTERS;

//
// Definiton of the HvCallGetVpRegister hypercall input structure.
// This call retrieves a Vp's register state.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_VP_REGISTERS
{
    HV_PARTITION_ID     PartitionId;
    HV_VP_INDEX         VpIndex;
    HV_INPUT_VTL        InputVtl;
    UINT8               RsvdZ8;
    UINT16              RsvdZ16;
    HV_CALL_ATTRIBUTES
    HV_REGISTER_NAME    Names[];
} HV_INPUT_GET_VP_REGISTERS, *PHV_INPUT_GET_VP_REGISTERS;

typedef union _HV_REGISTER_CR_INTERCEPT_CONTROL
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Cr0Write            : 1;       // 0x0000000000000001
        UINT64 Cr4Write            : 1;       // 0x0000000000000002
        UINT64 XCr0Write           : 1;       // 0x0000000000000004
        UINT64 IA32MiscEnableRead  : 1;       // 0x0000000000000008
        UINT64 IA32MiscEnableWrite : 1;       // 0x0000000000000010
        UINT64 MsrLstarRead        : 1;       // 0x0000000000000020
        UINT64 MsrLstarWrite       : 1;       // 0x0000000000000040
        UINT64 MsrStarRead         : 1;       // 0x0000000000000080
        UINT64 MsrStarWrite        : 1;       // 0x0000000000000100
        UINT64 MsrCstarRead        : 1;       // 0x0000000000000200
        UINT64 MsrCstarWrite       : 1;       // 0x0000000000000400
        UINT64 ApicBaseMsrRead     : 1;       // 0x0000000000000800
        UINT64 ApicBaseMsrWrite    : 1;       // 0x0000000000001000
        UINT64 MsrEferRead         : 1;       // 0x0000000000002000
        UINT64 MsrEferWrite        : 1;       // 0x0000000000004000
        UINT64 GdtrWrite           : 1;       // 0x0000000000008000
        UINT64 IdtrWrite           : 1;       // 0x0000000000010000
        UINT64 LdtrWrite           : 1;       // 0x0000000000020000
        UINT64 TrWrite             : 1;       // 0x0000000000040000
        UINT64 MsrSysenterCsWrite  : 1;       // 0x0000000000080000
        UINT64 MsrSysenterEipWrite : 1;       // 0x0000000000100000
        UINT64 MsrSysenterEspWrite : 1;       // 0x0000000000200000
        UINT64 MsrSfmaskWrite      : 1;       // 0x0000000000400000
        UINT64 MsrTscAuxWrite      : 1;       // 0x0000000000800000
        UINT64 MsrSgxLaunchControlWrite : 1;  // 0x0000000001000000
        UINT64 RsvdZ               : 39;
    };
} HV_REGISTER_CR_INTERCEPT_CONTROL, *PHV_REGISTER_CR_INTERCEPT_CONTROL;

//
// Definition for HvStartVirtualProcessor hypercall input structure.
// This sets the values provided in VpContext and makes the said Vp runnable.
//

#if defined(_AMD64_) || defined(_X86_) || defined(_ARM64_)

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_START_VIRTUAL_PROCESSOR
{
    HV_PARTITION_ID             PartitionId;
    HV_VP_INDEX                 VpIndex;
    HV_VTL                      TargetVtl;
    UINT8                       ReservedZ0;
    UINT16                      ReservedZ1;
    HV_INITIAL_VP_CONTEXT       VpContext;
} HV_INPUT_START_VIRTUAL_PROCESSOR, *PHV_INPUT_START_VIRTUAL_PROCESSOR;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_VP_INDEX_FROM_APIC_ID
{
    HV_PARTITION_ID                PartitionId;
    HV_VTL                         TargetVtl;
    UINT8                          ReservedZ0;
    UINT16                         ReservedZ1;
    UINT32                         ReservedZ2;
    HV_CALL_ATTRIBUTES HV_PROCESSOR_HW_ID  ProcHwIds[];
} HV_INPUT_GET_VP_INDEX_FROM_APIC_ID, *PHV_INPUT_GET_VP_INDEX_FROM_APIC_ID;

#endif

//
// Flags to describe the access a partition has to a GPA page.
//
typedef UINT32 HV_MAP_GPA_FLAGS, *PHV_MAP_GPA_FLAGS;

//
// Flags for unmapping GPA pages.
//
typedef UINT32 HV_UNMAP_GPA_FLAGS, *PHV_UNMAP_GPA_FLAGS;

#define HV_UNMAP_GPA_KEEP_PRECOMMITTED  0x1
#define HV_UNMAP_GPA_LARGE_PAGE         0x2

//
// Definition of the HvCallModifyVtlProtectionMask hypercall input structure.
// This call applies VTL protections to an existing set of GPA pages.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_MODIFY_VTL_PROTECTION_MASK
{
    //
    // Supplies the partition ID of the partition this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies the new mapping flags to apply.
    //

    HV_MAP_GPA_FLAGS MapFlags;

    //
    // Supplies the target VTL.
    //

    HV_INPUT_VTL TargetVtl;

    //
    // Reserved for future use - potentially for an address space ID.
    //

    UINT8 RsvdZ8;
    UINT16 RsvdZ16;

    //
    // Supplies an array of GPA page numbers to modify.
    //

    HV_CALL_ATTRIBUTES HV_GPA_PAGE_NUMBER GpaPageList[];

} HV_INPUT_MODIFY_VTL_PROTECTION_MASK, *PHV_INPUT_MODIFY_VTL_PROTECTION_MASK;

//
// The number of VTLs for which permissions can be specified in a VTL permission set.
//
#define HV_VTL_PERMISSION_SET_SIZE      4

//
// The set of VTLs for which permissions can be specified in a VTL permission set. VTL 0 is never
// included in this set.
//
#define HV_VTL_PERMISSION_SET_VTL_SET   (((1 << HV_VTL_PERMISSION_SET_SIZE) - 1) << 1)

typedef union _HV_VTL_PERMISSION_SET
{
    UINT32 AsUINT32;

    struct
    {
        //
        // VTL permissions for the GPA page, starting from VTL 1.
        //

        UINT8 VtlPermissionFrom1[HV_VTL_PERMISSION_SET_SIZE];
    };

} HV_VTL_PERMISSION_SET, *PHV_VTL_PERMISSION_SET;

//
// Definitions for the HvCallSetGpaPageAttributes hypercall.
//

typedef union _HV_GPA_PAGE_ATTRIBUTES
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 Protectable : 1;
        UINT64 Reserved : 63;
    };

} HV_GPA_PAGE_ATTRIBUTES, *PHV_GPA_PAGE_ATTRIBUTES;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SET_GPA_PAGE_ATTRIBUTES
{
    HV_PARTITION_ID PartitionId;
    HV_GPA_PAGE_ATTRIBUTES Attributes;
    HV_CALL_ATTRIBUTES HV_GPA_PAGE_NUMBER GpaPageList[];

} HV_INPUT_SET_GPA_PAGE_ATTRIBUTES, *PHV_INPUT_SET_GPA_PAGE_ATTRIBUTES;

//
// Definition of the HvCallCheckSparseGpaPageVtlAccess hypercall structures.
// This call determines whether a specific VTL has access to a set of GPA pages.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_CHECK_SPARSE_GPA_PAGE_VTL_ACCESS
{
    //
    // Supplies the partition ID of the partition this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies the target VTL.
    //

    HV_INPUT_VTL TargetVtl;

    //
    // Supplies the desired access to the GPA pages.
    //

    UINT8 DesiredAccess;

    UINT16 Reserved0;
    UINT32 Reserved1;

    //
    // Supplies an array of GPA page numbers to check.
    //

    HV_CALL_ATTRIBUTES HV_GPA_PAGE_NUMBER GpaPageList[];

} HV_INPUT_CHECK_SPARSE_GPA_PAGE_VTL_ACCESS, *PHV_INPUT_CHECK_SPARSE_GPA_PAGE_VTL_ACCESS;

typedef enum _HV_CHECK_GPA_PAGE_VTL_ACCESS_RESULT_CODE
{
    //
    // The GPA page(s) are accessible to the guest VTL.
    //
    HvCheckGpaPageVtlAccessSuccess          = 0,

    //
    // A GPA page is inaccessible to the guest VTL because a higher VTL has
    // restricted access to the page.
    //
    HvCheckGpaPageVtlAccessMemoryIntercept  = 1,

} HV_CHECK_GPA_PAGE_VTL_ACCESS_RESULT_CODE, *PHV_CHECK_GPA_PAGE_VTL_ACCESS_RESULT_CODE;

typedef union _HV_CHECK_GPA_PAGE_VTL_ACCESS_RESULT
{
    UINT64 AsUINT64;

    struct
    {
        UINT32 ResultCode : 8;
        UINT32 DeniedAccess : 8;
        UINT32 InterceptingVtl : 4;
        UINT32 Reserved0 : 12;
        UINT32 Reserved1;
    };
} HV_CHECK_GPA_PAGE_VTL_ACCESS_RESULT, *PHV_CHECK_GPA_PAGE_VTL_ACCESS_RESULT;

//
// Definitions for the HvCallAcceptGpaPages hypercall.
//

typedef enum _HV_ACCEPT_MEMORY_TYPE
{
    HvAcceptMemoryTypeAny = 0,
    HvAcceptMemoryTypeRam = 1,
    HvAcceptMemoryTypeCount
} HV_ACCEPT_MEMORY_TYPE;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_ACCEPT_GPA_PAGES
{
    //
    // Supplies the partition ID of the partition this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies the expected memory type (HV_ACCEPT_MEMORY_TYPE).
    //

    UINT32 MemoryType : 6;

    //
    // Supplies the initial host visibility (exclusive, shared read-only, shared read-write).
    //

    UINT32 HostVisibility : 2;

    //
    // Supplies the set of VTLs for which initial VTL permissions will be set.
    //

    UINT32 VtlSet : 4;

    UINT32 Reserved : 20;

    //
    // Supplies the set of initial VTL permissions.
    //

    HV_VTL_PERMISSION_SET VtlPermissionSet;

    //
    // Supplies the GPA page number of the first page to modify.
    //

    HV_GPA_PAGE_NUMBER GpaPageBase;

} HV_INPUT_ACCEPT_GPA_PAGES, *PHV_INPUT_ACCEPT_GPA_PAGES;

//
// Definitions for the HvCallUnacceptGpaPages hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_UNACCEPT_GPA_PAGES
{
    //
    // Supplies the partition ID of the partition this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies the set of VTLs for which VTL permissions will be checked.
    //

    UINT32 VtlSet : 4;

    UINT32 Reserved : 28;

    //
    // Supplies the set of VTL permissions to check against.
    //

    HV_VTL_PERMISSION_SET VtlPermissionSet;

    //
    // Supplies the GPA page number of the first page to modify.
    //

    HV_GPA_PAGE_NUMBER GpaPageBase;

} HV_INPUT_UNACCEPT_GPA_PAGES, *PHV_INPUT_UNACCEPT_GPA_PAGES;

//
// Definitions for the HvCallModifySparseGpaPageHostVisibility hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_MODIFY_SPARSE_GPA_PAGE_HOST_VISIBILITY
{
    //
    // Supplies the partition ID of the partition this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies the new host visibility.
    //

    UINT32 HostVisibility : 2;

    UINT32 Reserved0 : 30;
    UINT32 Reserved1;

    //
    // Supplies an array of GPA page numbers to modify.
    //

    HV_CALL_ATTRIBUTES HV_GPA_PAGE_NUMBER GpaPageList[];

} HV_INPUT_MODIFY_SPARSE_GPA_PAGE_HOST_VISIBILITY, *PHV_INPUT_MODIFY_SPARSE_GPA_PAGE_HOST_VISIBILITY;

//
// Definition of the HvCallSendSyntheticClusterIpi hypercall input structure.
// This call sends a fixed virtual interrupt to a synthetic cluster specified as
// a processor mask.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SEND_SYNTHETIC_CLUSTER_IPI
{
    UINT32 Vector;
    HV_INPUT_VTL TargetVtl;
    UINT8  RsvdZ0;
    UINT16 RsvdZ1;
    UINT64 ProcessorMask;
} HV_INPUT_SEND_SYNTHETIC_CLUSTER_IPI, *PHV_INPUT_SEND_SYNTHETIC_CLUSTER_IPI;

//
// Specifies how the nested MSR intercepts will be handeled.
//

typedef enum _HV_NESTED_MSR_INTERCEPT_MODE
{
    //
    // MSR bitmap is disabled, all MSR's are intercepted.
    //
    HvNestedMsrInterceptModeSlow        = 0,

    //
    // On every nested vmentry, nested virutal bitmap is compared to the
    // parent physical bitmap.
    // If the virtual bitmap is more restrictive, we will enable parent
    // physical bitmap.
    // Otherwise, we will fall back to intercepting all MSR's.
    //
    HvNestedMsrInterceptModeFast        = 1,

    //
    // Same as HvNestedMsrInterceptModeFast, the only difference is that we
    // will not do the check the bitmap on every vmentry. Guest will set a
    // dirty bit when it modifies the bitmap.
    //
    HvNestedMsrInterceptModeEnlightened = 2,

    HvNestedMsrInterceptModeCount       = 3
} HV_NESTED_MSR_INTERCEPT_MODE, *PHV_NESTED_MSR_INTERCEPT_MODE;

//
// Per Nested-VMCS control register.
//

typedef union _HV_REGISTER_NESTED_CONTROL
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 MsrInterceptMode : 2;
        UINT64 MsrBitmapDirty   : 1;
        UINT64 RsvdZ            : 61;
    };

} HV_REGISTER_NESTED_CONTROL, *PHV_REGISTER_NESTED_CONTROL;

//
// Definitions for device interrupts.
//

typedef union _HV_MSI_ADDRESS_REGISTER
{
    UINT32 AsUINT32;

    struct
    {
        UINT32 Reserved1:2;
        UINT32 DestinationMode:1;
        UINT32 RedirectionHint:1;
        UINT32 Reserved2:8;
        UINT32 DestinationId:8;
        UINT32 MsiBase:12;
    };

} HV_MSI_ADDRESS_REGISTER, *PHV_MSI_ADDRESS_REGISTER;

typedef union _HV_MSI_DATA_REGISTER
{
    UINT32 AsUINT32;

#if defined(_AMD64_) || defined(_X86_)

    struct
    {
        UINT32 Vector:8;
        UINT32 DeliveryMode:3;
        UINT32 Reserved1:3;
        UINT32 LevelAssert:1;
        UINT32 TriggerMode:1;
        UINT32 Reserved2:16;
    };

#else

    UINT32 Vector;

#endif

} HV_MSI_DATA_REGISTER, *PHV_MSI_DATA_REGISTER;

typedef union _HV_MSI_ENTRY
{

#if defined(_AMD64_) || defined(_X86_)

    UINT64 AsUINT64;

    struct
    {
        HV_MSI_ADDRESS_REGISTER Address;
        HV_MSI_DATA_REGISTER Data;
    };

#else

    UINT64 AsUINT64[2];

    struct
    {
        UINT64 Address;
        HV_MSI_DATA_REGISTER Data;
        UINT32 Reserved;
    };

#endif

} HV_MSI_ENTRY, *PHV_MSI_ENTRY;

#if defined(_AMD64_) || defined(_X86_)

#define HV_MSI_ENTRY_IS_ADDRESS_EQUAL(_MsiEntry1_, _MsiEntry2_) \
    ((_MsiEntry1_).Address.AsUINT32 == (_MsiEntry2_).Address.AsUINT32)

#else

#define HV_MSI_ENTRY_IS_ADDRESS_EQUAL(_MsiEntry1_, _MsiEntry2_) \
    ((_MsiEntry1_).Address == (_MsiEntry2_).Address)

#endif

typedef union _HV_IOAPIC_RTE
{
    struct
    {
        UINT32 Vector:8;
        UINT32 DeliveryMode:3;
        UINT32 DestinationMode:1;
        UINT32 DeliveryStatus:1;
        UINT32 InterruptPolarity:1;
        UINT32 RemoteIRR:1;
        UINT32 TriggerMode:1;
        UINT32 InterruptMask:1;
        UINT32 Reserved1:15;

        UINT32 Reserved2:24;
        UINT32 DestinationId:8;
    };

    struct
    {
        UINT32 LowUINT32;
        UINT32 HighUINT32;
    };

    UINT64 AsUINT64;

} HV_IOAPIC_RTE, *PHV_IOAPIC_RTE;

typedef enum _HV_INTERRUPT_SOURCE
{
    HvInterruptSourceMsi = 1,
    HvInterruptSourceIoApic,
    HvInterruptSourceGic
} HV_INTERRUPT_SOURCE, *PHV_INTERRUPT_SOURCE;

typedef struct _HV_INTERRUPT_ENTRY
{
    HV_INTERRUPT_SOURCE InterruptSource;
    UINT32 Reserved;

    union
    {
        HV_MSI_ENTRY MsiEntry;
#if defined(_AMD64_) || defined(_X86_)
        HV_IOAPIC_RTE IoApicEntry;
        UINT64 Data;
#else
        UINT64 Data[2];
#endif
        UINT32 LineNumber;
    };

} HV_INTERRUPT_ENTRY, *PHV_INTERRUPT_ENTRY;

#if defined(_AMD64_) || defined(_X86_)

#define HV_INTERRUPT_ENTRY_IS_DATA_EQUAL(_Entry1_, _Entry2_) \
    ((_Entry1_)->Data == (_Entry2_)->Data)

#else

#define HV_INTERRUPT_ENTRY_IS_DATA_EQUAL(_Entry1_, _Entry2_) \
    (((_Entry1_)->Data[0] == (_Entry2_)->Data[0]) && \
     ((_Entry1_)->Data[1] == (_Entry2_)->Data[1]))

#endif

//
// Definitions for the target (vector and virtual processors) of a device
// interrupt.
//

typedef struct _HV_DEVICE_INTERRUPT_TARGET
{
    HV_INTERRUPT_VECTOR Vector;
    UINT32 Flags;

    union
    {
        UINT64 ProcessorMask;
        UINT64 ProcessorSet[];
    };

} HV_DEVICE_INTERRUPT_TARGET, *PHV_DEVICE_INTERRUPT_TARGET;

#define HV_DEVICE_INTERRUPT_TARGET_MULTICAST        1
#define HV_DEVICE_INTERRUPT_TARGET_PROCESSOR_SET    2

//
// Definitions for the HvRetargetDeviceInterrupt hypercall input and output
// structures.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_RETARGET_DEVICE_INTERRUPT
{
    HV_PARTITION_ID PartitionId;
    UINT64 DeviceId;
    HV_INTERRUPT_ENTRY InterruptEntry;
    UINT64 Reserved;
    HV_DEVICE_INTERRUPT_TARGET InterruptTarget;

} HV_INPUT_RETARGET_DEVICE_INTERRUPT, *PHV_INPUT_RETARGET_DEVICE_INTERRUPT;

//
// Cross-platform MSR definitions, to enable cross-platform code to have single
// Read/Write MSR call using ReadHvMsr/WriteHvMsr functions, avoiding having to
// fork the code across platforms. The ReadHvMsr/WriteHvMsr functions adhere to
// the following prototypes:
//
// VOID
// ReadHvMsr(UINT32       RegisterIndex,
//           PUINT64      RegisterBuffer);
//
// VOID
// WriteHvMsr(UINT32      RegisterIndex,
//            UINT64      RegisterBuffer);
//
// The MSR list below will be expanded as needed, as we move more code to using
// the new functions.
//

#if defined(_ARM64_)

//
// ARM64 code is required to define ReadHvMsr/WriteHvMsr functions based on
// on their implementation of the HvCallGetVpRegisters/HvCallSetVpRegisters
// hypercalls.
//

#define HV_MSR_SINT0                HvRegisterSint0
#define HV_MSR_TIME_REF_COUNT       HvRegisterTimeRefCount
#define HV_MSR_STIMER0_CONFIG       HvRegisterStimer0Config
#define HV_MSR_STIMER0_COUNT        HvRegisterStimer0Count
#define HV_MSR_SIMP                 HvRegisterSipp
#define HV_MSR_EOM                  HvRegisterEom

#else

#define HV_MSR_SINT0                HV_X64_MSR_SINT0
#define HV_MSR_TIME_REF_COUNT       HV_X64_MSR_TIME_REF_COUNT
#define HV_MSR_STIMER0_CONFIG       HV_X64_MSR_STIMER0_CONFIG
#define HV_MSR_STIMER0_COUNT        HV_X64_MSR_STIMER0_COUNT
#define HV_MSR_SIMP                 HV_X64_MSR_SIMP
#define HV_MSR_EOM                  HV_X64_MSR_EOM

#endif

// Definition of the HvCallProcessIommuPrq input structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_PROCESS_IOMMU_PRQ
{
    HV_IOMMU_ID IommuId;
} HV_INPUT_PROCESS_IOMMU_PRQ, *PHV_INPUT_PROCESS_IOMMU_PRQ;

#if defined(_ARM64_)

typedef union _HV_PARTITION_INFO_PAGE
{
    UINT8 Padding[HV_PAGE_SIZE];

    struct
    {
        //
        // If this is non-zero, the guest when issuing a broadcast TLB invalidation must in
        // addition issue a FlushTlb hypercall.
        //
        UINT32 TlbInUse;
    };

} HV_PARTITION_INFO_PAGE, *PHV_PARTITION_INFO_PAGE;

typedef union _HV_REGISTER_PARTITION_INFO_PAGE
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 Enabled : 1;
        UINT64 Reserved : 11;
        UINT64 GpaPage : 52;
    };

} HV_REGISTER_PARTITION_INFO_PAGE, *PHV_REGISTER_PARTITION_INFO_PAGE;

typedef union _HV_ARM64_REGISTER_TLBI_CONTROL
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 TlbiEnlightened : 1;
        UINT64 Reserved : 63;
    };

} HV_ARM64_REGISTER_TLBI_CONTROL, *PHV_ARM64_REGISTER_TLBI_CONTROL;

#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4200) /* nonstandard extension used : zero-sized array in struct/union */
#pragma warning(default:4214) /* nonstandard extension used : bit field types other then int */
#pragma warning(default:4324) /* structure was padded due to __declspec(align()) */
#endif

#endif //_HVGDK_MINI_
