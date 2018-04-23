/*++

Copyright (c) Microsoft Corporation

Module Name:

    HvHdk.h

Abstract:

    Type definitions for the hypervisor host interface.

Author:

    Hypervisor Engineering Team (hvet) 12-Jul-2013

--*/

#if !defined(_HVHDK_)
#define _HVHDK_

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

#ifdef __cplusplus
extern "C" {
#endif

//
// Include header which defines things that must be consumed (and thus
// published) from minkernel.
//

#include <HvHdk_mini.h>
#include <HvGdk.h>


//
// Memory Types
//
// Guest physical addresses (GPAs) define the guest's view of physical memory.
// GPAs can be mapped to underlying SPAs. There is one guest physical address space per
// partition.
//

typedef UINT64 HV_GPA, *PHV_GPA;

#ifndef X64_PAGE_SIZE
#define X64_PAGE_SIZE 0x1000
#endif

#ifndef X64_LARGE_PAGE_SIZE
#define X64_LARGE_PAGE_SIZE 0x200000
#endif

#ifndef X64_LARGE_PAGE_SIZE_1GB
#define X64_LARGE_PAGE_SIZE_1GB 0x40000000
#endif

#define HV_X64_MAX_PAGE_NUMBER (MAXUINT64/X64_PAGE_SIZE)
#define HV_PAGE_MASK (HV_PAGE_SIZE - 1)
#define HV_PAGE_SHIFT (12)
#define HV_LARGE_PAGE_MASK (HV_LARGE_PAGE_SIZE - 1)
#define HV_LARGE_PAGE_MASK_1GB (HV_LARGE_PAGE_SIZE_1GB - 1)
#define HV_PAGES_PER_LARGE_PAGE (HV_LARGE_PAGE_SIZE / HV_PAGE_SIZE)

typedef UINT64 HV_SPA_PAGE_NUMBER, *PHV_SPA_PAGE_NUMBER;
typedef UINT32 HV_SPA_PAGE_OFFSET, *PHV_SPA_PAGE_OFFSET;

typedef const HV_SPA_PAGE_NUMBER *PCHV_SPA_PAGE_NUMBER;

typedef UINT16 HV_IO_PORT, *PHV_IO_PORT;

//
// Forward declare the loader block.
//
typedef struct _HV_LOADER_BLOCK *PHV_LOADER_BLOCK;
//
// Hypervisor global counter set
//
typedef enum _HV_HYPERVISOR_COUNTER 
{

    StHvCounterLogicalProcessors = 1,
    StHvCounterPartitions = 2,
    StHvCounterTotalPages = 3,
    StHvCounterVirtualProcessors = 4,
    StHvCounterMonitoredNotifications = 5,
    StHvCounterModernStandbyEntries = 6,
    StHvCounterPlatformIdleTransitions = 7,
    StHvCounterHypervisorStartupCost = 8,

    StHvCounterMAXIMUM

} HV_HYPERVISOR_COUNTER;

#define HV_STATISTICS_GROUP_HVA_LENGTH 64
#define HV_STATISTICS_GROUP_HVV_LENGTH 0

//
// Hypervisor Logical Processor counter set
//
typedef enum _HV_CPU_COUNTER 
{

    StLpCounterGlobalTime = 1,
    StLpCounterTotalRunTime = 2,
    StLpCounterHypervisorRunTime = 3,
    StLpCounterHardwareInterrupts = 4,
    StLpCounterContextSwitches = 5,
    StLpCounterInterProcessorInterrupts = 6,
    StLpCounterSchedulerInterrupts = 7,
    StLpCounterTimerInterrupts = 8,
    StLpCounterInterProcessorInterruptsSent = 9,
    StLpCounterProcessorHalts = 10,
    StLpCounterMonitorTransitionCost = 11,
    StLpCounterContextSwitchTime = 12,
    StLpCounterC1TransitionsCount = 13,
    StLpCounterC1RunTime = 14,
    StLpCounterC2TransitionsCount = 15,
    StLpCounterC2RunTime = 16,
    StLpCounterC3TransitionsCount = 17,
    StLpCounterC3RunTime = 18,
    StLpCounterFrequency = 19,
    StLpCounterPercentMaxFrequency = 20,
    StLpCounterParkingStatus = 21,
    StLpCounterProcessorStateFlags = 22,
    StLpCounterRootVpIndex = 23,
    StLpCounterIdleSequenceNumber = 24,
    StLpCounterGlobalTscCount = 25,
    StLpCounterActiveTscCount = 26,
    StLpCounterIdleAccumulation = 27,
    StLpCounterReferenceCycleCount0 = 28,
    StLpCounterActualCycleCount0 = 29,
    StLpCounterReferenceCycleCount1 = 30,
    StLpCounterActualCycleCount1 = 31,
    StLpCounterProximityDomainId = 32,
    StLpCounterPostedInterruptNotifications = 33,
    StLpCounterBranchPredictorFlushes = 34,

    StLpCounterMAXIMUM

} HV_CPU_COUNTER;

#define HV_STATISTICS_GROUP_LPA_LENGTH 272
#define HV_STATISTICS_GROUP_LPV_LENGTH 0

//
// Hypervisor Hypercall-Based Logical Processor counter set
//
typedef enum _HV_CPU_HYPERCALLBASED_COUNTER 
{

    StLpHcCounterGuestRunTime = 1,
    StLpHcCounterIdleTime = 2,
    StLpHcCounterTotalRunTimePercent = 3,
    StLpHcCounterHypervisorRunTimePercent = 4,
    StLpHcCounterGuestRunTimePercent = 5,
    StLpHcCounterIdleTimePercent = 6,
    StLpHcCounterTotalInterrupts = 7,
    StLpHcCounterMAXIMUM

} HV_CPU_HYPERCALLBASED_COUNTER;

//
// Partition counter set
//
typedef enum _HV_PROCESS_COUNTER 
{

    StPtCounterVirtualProcessors = 1,

    StPtCounterTlbSize = 3,
    StPtCounterAddressSpaces = 4,
    StPtCounterDepositedPages = 5,
    StPtCounterGpaPages = 6,
    StPtCounterGpaSpaceModifications = 7,
    StPtCounterVirtualTlbFlushEntires = 8,
    StPtCounterRecommendedTlbSize = 9,
    StPtCounterGpaPages4K = 10,
    StPtCounterGpaPages2M = 11,
    StPtCounterGpaPages1G = 12,
    StPtCounterGpaPages512G = 13,
    StPtCounterDevicePages4K = 14,
    StPtCounterDevicePages2M = 15,
    StPtCounterDevicePages1G = 16,
    StPtCounterDevicePages512G = 17,
    StPtCounterAttachedDevices = 18,
    StPtCounterDeviceInterruptMappings = 19,
    StPtCounterIoTlbFlushes = 20,
    StPtCounterIoTlbFlushCost = 21,
    StPtCounterDeviceInterruptErrors = 22,
    StPtCounterDeviceDmaErrors = 23,
    StPtCounterDeviceInterruptThrottleEvents = 24,
    StPtCounterSkippedTimerTicks = 25,
    StPtCounterPartitionId = 26,
    StPtCounterNestedTlbSize = 27,
    StPtCounterRecommendedNestedTlbSize = 28,
    StPtCounterNestedTlbFreeListSize = 29,
    StPtCounterNestedTlbTrimmedPages = 30,
    StPtCounterMAXIMUM

} HV_PROCESS_COUNTER;

#define HV_STATISTICS_GROUP_PTA_LENGTH 8
#define HV_STATISTICS_GROUP_PTV_LENGTH 224

//
// Hypervisor Virtual Processor counter set
//
typedef enum _HV_THREAD_COUNTER 
{

    StVpCounterTotalRunTime = 1,
    StVpCounterHypervisorRunTime = 2,
    StVpCounterRemoteNodeRunTime = 3,
    StVpCounterNormalizedRunTime = 4,

    StVpCounterHypercallsCount = 6,
    StVpCounterHypercallsTime = 7,
    StVpCounterPageInvalidationsCount = 8,
    StVpCounterPageInvalidationsTime = 9,
    StVpCounterControlRegisterAccessesCount = 10,
    StVpCounterControlRegisterAccessesTime = 11,
    StVpCounterIoInstructionsCount = 12,
    StVpCounterIoInstructionsTime = 13,
    StVpCounterHltInstructionsCount = 14,
    StVpCounterHltInstructionsTime = 15,
    StVpCounterMwaitInstructionsCount = 16,
    StVpCounterMwaitInstructionsTime = 17,
    StVpCounterCpuidInstructionsCount = 18,
    StVpCounterCpuidInstructionsTime = 19,
    StVpCounterMsrAccessesCount = 20,
    StVpCounterMsrAccessesTime = 21,
    StVpCounterOtherInterceptsCount = 22,
    StVpCounterOtherInterceptsTime = 23,
    StVpCounterExternalInterruptsCount = 24,
    StVpCounterExternalInterruptsTime = 25,
    StVpCounterPendingInterruptsCount = 26,
    StVpCounterPendingInterruptsTime = 27,
    StVpCounterEmulatedInstructionsCount = 28,
    StVpCounterEmulatedInstructionsTime = 29,
    StVpCounterDebugRegisterAccessesCount = 30,
    StVpCounterDebugRegisterAccessesTime = 31,
    StVpCounterPageFaultInterceptsCount = 32,
    StVpCounterPageFaultInterceptsTime = 33,
    StVpCounterGuestPageTableMaps = 34,
    StVpCounterLargePageTlbFills = 35,
    StVpCounterSmallPageTlbFills = 36,
    StVpCounterReflectedGuestPageFaults = 37,
    StVpCounterApicMmioAccesses = 38,
    StVpCounterIoInterceptMessages = 39,
    StVpCounterMemoryInterceptMessages = 40,
    StVpCounterApicEoiAccesses = 41,
    StVpCounterOtherMessages = 42,
    StVpCounterPageTableAllocations = 43,
    StVpCounterLogicalProcessorMigrations = 44,
    StVpCounterAddressSpaceEvictions = 45,
    StVpCounterAddressSpaceSwitches = 46,
    StVpCounterAddressDomainFlushes = 47,
    StVpCounterAddressSpaceFlushes = 48,
    StVpCounterGlobalGvaRangeFlushes = 49,
    StVpCounterLocalGvaRangeFlushes = 50,
    StVpCounterPageTableEvictions = 51,
    StVpCounterPageTableReclamations = 52,
    StVpCounterPageTableResets = 53,
    StVpCounterPageTableValidations = 54,
    StVpCounterApicTprAccesses = 55,
    StVpCounterPageTableWriteIntercepts = 56,
    StVpCounterSyntheticInterrupts = 57,
    StVpCounterVirtualInterrupts = 58,
    StVpCounterApicIpisSent = 59,
    StVpCounterApicSelfIpisSent = 60,
    StVpCounterGpaSpaceHypercalls = 61,
    StVpCounterLogicalProcessorHypercalls = 62,
    StVpCounterLongSpinWaitHypercalls = 63,
    StVpCounterOtherHypercalls = 64,
    StVpCounterSyntheticInterruptHypercalls = 65,
    StVpCounterVirtualInterruptHypercalls = 66,
    StVpCounterVirtualMmuHypercalls = 67,
    StVpCounterVirtualProcessorHypercalls = 68,
    StVpCounterHardwareInterrupts = 69,
    StVpCounterNestedPageFaultInterceptsCount = 70,
    StVpCounterNestedPageFaultInterceptsTime = 71,
    StVpCounterLogicalProcessorDispatches = 72,
    StVpCounterWaitingForCpuTime = 73,
    StVpCounterExtendedHypercalls = 74,
    StVpCounterExtendedHypercallInterceptMessages = 75,
    StVpCounterMbecNestedPageTableSwitches = 76,
    StVpCounterOtherReflectedGuestExceptions = 77,
    StVpCounterGlobalIoTlbFlushes = 78,
    StVpCounterGlobalIoTlbFlushCost = 79,
    StVpCounterLocalIoTlbFlushes = 80,
    StVpCounterLocalIoTlbFlushCost = 81,
    StVpCounterHypercallsForwardedCount = 82,
    StVpCounterHypercallsForwardingTime = 83,
    StVpCounterPageInvalidationsForwardedCount = 84,
    StVpCounterPageInvalidationsForwardingTime = 85,
    StVpCounterControlRegisterAccessesForwardedCount = 86,
    StVpCounterControlRegisterAccessesForwardingTime = 87,
    StVpCounterIoInstructionsForwardedCount = 88,
    StVpCounterIoInstructionsForwardingTime = 89,
    StVpCounterHltInstructionsForwardedCount = 90,
    StVpCounterHltInstructionsForwardingTime = 91,
    StVpCounterMwaitInstructionsForwardedCount = 92,
    StVpCounterMwaitInstructionsForwardingTime = 93,
    StVpCounterCpuidInstructionsForwardedCount = 94,
    StVpCounterCpuidInstructionsForwardingTime = 95,
    StVpCounterMsrAccessesForwardedCount = 96,
    StVpCounterMsrAccessesForwardingTime = 97,
    StVpCounterOtherInterceptsForwardedCount = 98,
    StVpCounterOtherInterceptsForwardingTime = 99,
    StVpCounterExternalInterruptsForwardedCount = 100,
    StVpCounterExternalInterruptsForwardingTime = 101,
    StVpCounterPendingInterruptsForwardedCount = 102,
    StVpCounterPendingInterruptsForwardingTime = 103,
    StVpCounterEmulatedInstructionsForwardedCount = 104,
    StVpCounterEmulatedInstructionsForwardingTime = 105,
    StVpCounterDebugRegisterAccessesForwardedCount = 106,
    StVpCounterDebugRegisterAccessesForwardingTime = 107,
    StVpCounterPageFaultInterceptsForwardedCount = 108,
    StVpCounterPageFaultInterceptsForwardingTime = 109,
    StVpCounterVmclearEmulationCount = 110,
    StVpCounterVmclearEmulationTime = 111,
    StVpCounterVmptrldEmulationCount = 112,
    StVpCounterVmptrldEmulationTime = 113,
    StVpCounterVmptrstEmulationCount = 114,
    StVpCounterVmptrstEmulationTime = 115,
    StVpCounterVmreadEmulationCount = 116,
    StVpCounterVmreadEmulationTime = 117,
    StVpCounterVmwriteEmulationCount = 118,
    StVpCounterVmwriteEmulationTime = 119,
    StVpCounterVmxoffEmulationCount = 120,
    StVpCounterVmxoffEmulationTime = 121,
    StVpCounterVmxonEmulationCount = 122,
    StVpCounterVmxonEmulationTime = 123,
    StVpCounterNestedVMEntriesCount = 124,
    StVpCounterNestedVMEntriesTime = 125,
    StVpCounterNestedSLATSoftPageFaultsCount = 126,
    StVpCounterNestedSLATSoftPageFaultsTime = 127,
    StVpCounterNestedSLATHardPageFaultsCount = 128,
    StVpCounterNestedSLATHardPageFaultsTime = 129,
    StVpCounterInvEptAllContextEmulationCount = 130,
    StVpCounterInvEptAllContextEmulationTime = 131,
    StVpCounterInvEptSingleContextEmulationCount = 132,
    StVpCounterInvEptSingleContextEmulationTime = 133,
    StVpCounterInvVpidAllContextEmulationCount = 134,
    StVpCounterInvVpidAllContextEmulationTime = 135,
    StVpCounterInvVpidSingleContextEmulationCount = 136,
    StVpCounterInvVpidSingleContextEmulationTime = 137,
    StVpCounterInvVpidSingleAddressEmulationCount = 138,
    StVpCounterInvVpidSingleAddressEmulationTime = 139,
    StVpCounterNestedTlbPageTableReclamations = 140,
    StVpCounterNestedTlbPageTableEvictions = 141,
    StVpCounterFlushGuestPhysicalAddressSpaceHypercalls = 142,
    StVpCounterFlushGuestPhysicalAddressListHypercalls = 143,
    StVpCounterPostedInterruptNotifications = 144,
    StVpCounterPostedInterruptScans = 145,
    StVpCounterMAXIMUM

} HV_THREAD_COUNTER;

#define HV_STATISTICS_GROUP_VPA_LENGTH 32
#define HV_STATISTICS_GROUP_VPV_LENGTH 1120

//
// Hypervisor Hypercall-Based Virtual Processor counter set
//
typedef enum _HV_THREAD_HYPERCALLBASED_COUNTER 
{

    StVpHcCounterGuestRunTime = 1,
    StVpHcCounterTotalRunTimePercent = 2,
    StVpHcCounterHypervisorRunTimePercent = 3,
    StVpHcCounterGuestRunTimePercent = 4,
    StVpHcCounterTotalMessages = 5,
    StVpHcCounterTotalInterceptsBase = 6,
    StVpHcCounterTotalIntercepts = 7,
    StVpHcCounterTotalInterceptsCost = 8,
    StVpHcCounterRemoteRunTimePercent = 9,
    StVpHcCounterTotalVirtualizationInstructionsEmulatedBase = 13,
    StVpHcCounterTotalVirtualizationInstructionsEmulated = 14,
    StVpHcCounterTotalVirtualizationInstructionsEmulationCost = 15,
    StVpHcCounterGlobalReferenceTime = 16,
    StVpHcCounterMAXIMUM

} HV_THREAD_HYPERCALLBASED_COUNTER;



#define HV_MSR_STATS_PARTITION_RETAIL_PAGE          0x400000E0
#if defined(_PERF_FEATURES_ENABLED_)
#define HV_MSR_STATS_PARTITION_INTERNAL_PAGE        0x400000E1
#endif

#define HV_MSR_STATS_VP_RETAIL_PAGE                 0x400000E2
#if defined(_PERF_FEATURES_ENABLED_)
#define HV_MSR_STATS_VP_INTERNAL_PAGE               0x400000E3
#endif

typedef UINT16  HV_STATISTICS_GROUP_TYPE;
typedef UINT16  HV_STATISTICS_GROUP_LENGTH;

typedef struct _HV_STATISTICS_GROUP_VERSION
{
    UINT16    Minor;
    UINT16    Major;

} HV_STATISTICS_GROUP_VERSION;

//
// Group header
//
typedef struct DECLSPEC_ALIGN(2) _HV_STATISTICS_GROUP_HEADER
{

    HV_STATISTICS_GROUP_TYPE    Type;
    HV_STATISTICS_GROUP_VERSION Version;
    HV_STATISTICS_GROUP_LENGTH  Length;

} HV_STATISTICS_GROUP_HEADER, *PHV_STATISTICS_GROUP_HEADER;

#define HV_STATISTICS_GROUP_END_OF_LIST         0
#define HV_STATISTICS_GROUP_END_OF_PAGE         1

//
// Pseudo-group to use in manifest for counters accessible through hypercalls.
//
#define HV_STATISTICS_GROUP_HYPERCALL_BASED     15

//
// Definitions for the hypervisor counters statistics page
//
#define HV_STATISTICS_GROUP_HVA_ID              2
#define HV_STATISTICS_GROUP_HVA_VERSION         0x00010000
#define HV_STATISTICS_GROUP_HVV_ID              3
#define HV_STATISTICS_GROUP_HVV_VERSION         0x00010000
#define HV_STATISTICS_GROUP_HVI_ID              14
#define HV_STATISTICS_GROUP_HVI_VERSION         0x00010000

//
// Definitions for the logical processor counters statistics page
//
#define HV_STATISTICS_GROUP_LPA_ID              2
#define HV_STATISTICS_GROUP_LPA_VERSION         0x00010000
#define HV_STATISTICS_GROUP_LPV_ID              3
#define HV_STATISTICS_GROUP_LPV_VERSION         0x00010000
#define HV_STATISTICS_GROUP_LPI_ID              14
#define HV_STATISTICS_GROUP_LPI_VERSION         0x00010000

//
// Definitions for the partition counters statistics page
//
#define HV_STATISTICS_GROUP_PTA_ID              2
#define HV_STATISTICS_GROUP_PTA_VERSION         0x00010000
#define HV_STATISTICS_GROUP_PTV_ID              3
#define HV_STATISTICS_GROUP_PTV_VERSION         0x00010000
#define HV_STATISTICS_GROUP_PTI_ID              14
#define HV_STATISTICS_GROUP_PTI_VERSION         0x00010000

//
// Definitions for the virtual processor statistics page
//
#define HV_STATISTICS_GROUP_VPA_ID              2
#define HV_STATISTICS_GROUP_VPA_VERSION         0x00010000
#define HV_STATISTICS_GROUP_VPV_ID              3
#define HV_STATISTICS_GROUP_VPV_VERSION         0x00010000
#define HV_STATISTICS_GROUP_VPI_ID              14
#define HV_STATISTICS_GROUP_VPI_VERSION         0x00010000

//
// Maximum counters allowed per group. Calculated for the case when group
// occupies full page: there will be two headers (one for the group and one
// terminating the list).
//

#define HV_ST_MAX_COUNTERS_PER_GROUP \
    ((HV_PAGE_SIZE - 2 * sizeof(HV_STATISTICS_GROUP_HEADER)) / sizeof(UINT64))

//
// Definition of the counters structure.
//

typedef struct _HV_STATS_PAGE
{
    UINT64      Data[HV_PAGE_SIZE / sizeof(UINT64)];

} HV_STATS_PAGE, *PHV_STATS_PAGE;

//
// Definition for the stats map/unmap MSR value.
//

typedef union _HV_ST_MAP_LOCATION
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 MapEnabled           : 1;
        UINT64 ReservedP            : 11;
        UINT64 BaseMapGpaPageNumber : 52;
    };
} HV_ST_MAP_LOCATION, *PHV_ST_MAP_LOCATION;


#define HV_PICO100_TO_NANO100(x) (((x).AsUINT64 + 500ULL) / 1000ULL)

//
// Declare the timestamp type.
//
typedef UINT64 HV_TIME_STAMP, *PHV_TIME_STAMP;


//
// Define profile sources.
//
// N.B. The total number of profile sources is limited to 2048.
//

typedef enum _HV_PROFILE_SOURCE
{
    //
    // Profile sources for all processors.
    //
    HvProfileInvalid,
    HvProfileCyclesNotHalted,
    HvProfileCacheMisses,
    HvProfileBranchMispredictions,

#if defined(_PERF_FEATURES_ENABLED_)
#if defined(_AMD64_)
    //
    // Profile sources for Intel processors.
    //
    HvProfileInstructionsRetired = 512,
    HvProfileUnhaltedReferenceCycles,
    HvProfileLLCReference,
    HvProfileLLCMisses,
    HvProfileBranchInstuctionRetired,
    HvProfileBranchMispredict,
#endif
#endif

#if !defined(_ARM64_)
    //
    // Synthetic profile source.
    //
    HvProfileTimeUnhalted = 1536,
#endif

} HV_PROFILE_SOURCE;

//
// The HV_PROCESSOR_INFO structures contains additional information about
// each physical processor
//

typedef struct _HV_PROCESSOR_INFO
{
    //
    // The Local APIC ID for the processor.
    //
    UINT32 LocalApicId;

    //
    // The proximity domain the processor resides in
    //
    HV_PROXIMITY_DOMAIN_ID ProximityDomainId;

} HV_PROCESSOR_INFO, *PHV_PROCESSOR_INFO;

//
// The following structure contains the definition of a memory range.
//

typedef struct _HV_MEMORY_RANGE_INFO
{
    //
    // The system physical address where this range begins
    //
    HV_SPA BaseAddress;

    //
    // The length of this range of memory in bytes.
    //
    UINT64 Length;

    //
    // Whether the range behaves like RAM.
    //
    BOOLEAN IsRam;

} HV_MEMORY_RANGE_INFO, *PHV_MEMORY_RANGE_INFO;


typedef struct _HV_MEMORY_PROXIMITY_INFO
{
    //
    // The system physical address where this range begins
    //
    HV_SPA BaseAddress;

    //
    // The end address for this range (inclusive).
    //
    HV_SPA EndAddress;

    //
    // The proximity domain this memory range resides in.
    //
    HV_PROXIMITY_DOMAIN_ID ProximityDomainId;

} HV_MEMORY_PROXIMITY_INFO, *PHV_MEMORY_PROXIMITY_INFO;


//
// Trace types.
//
// Must be kept in sync with the HV traces manifest:
//
//      minkernel\manifests\hvl\hvtraces.man
//
// which is used to build
//
//      sdpublic / %SDXROOT%.public.%FLAVOR% ...\internal\minwin\priv_sdk\inc\hvtraces.h
//

#define HV_EVENTLOG_EVENT_IOMMU_WARNING_SCOPE_CONFLICT      0x0090 // HV_EVENTLOG_IOMMU_WARNING_SCOPE_CONFLICT_value
#define HV_EVENTLOG_EVENT_IOMMU_FAILED_RID_CONFLICT         0x0091 // HV_EVENTLOG_IOMMU_FAILED_RID_CONFLICT_value
#define HV_EVENTLOG_EVENT_IOMMU_FAILED_NO_RESOURCES         0x0092 // HV_EVENTLOG_IOMMU_FAILED_NO_RESOURCES_value
#define HV_EVENTLOG_EVENT_IOMMU_FAILED_INVALID_IOAPIC       0x0093 // HV_EVENTLOG_IOMMU_FAILED_INVALID_IOAPIC_value
#define HV_EVENTLOG_EVENT_IOMMU_FAILED_NO_DEVICE_ASSIGNMENT 0x0094 // HV_EVENTLOG_IOMMU_FAILED_NO_DEVICE_ASSIGNMENT_value
#define HV_EVENTLOG_EVENT_IOMMU_FAILED_RESERVED_DEVICE      0x0095 // HV_EVENTLOG_IOMMU_FAILED_RESERVED_DEVICE_value
#define HV_EVENTLOG_EVENT_PARTITION_CREATED                 0x4101 // HV_EVENTLOG_PARTITION_CREATED_value
#define HV_EVENTLOG_EVENT_PARTITION_DELETED                 0x4102 // HV_EVENTLOG_PARTITION_DELETED_value
#define HV_EVENTLOG_EVENT_PARTITION_CREATION_FAILED         0x2103 // HV_EVENTLOG_PARTITION_CREATION_FAILED_value
#define HV_EVENTLOG_EVENT_RESTRICTED_MSR_ACCESS             0x3106 // HV_EVENTLOG_RESTRICTED_MSR_ACCESS_value
#define HV_EVENTLOG_BUFFER_INDEX_NONE 0xffffffff

//
// Define all the trace buffer types.
//

typedef enum
{
    HvEventLogModeRegular            = 0x0000,
    HvEventLogModeCircular           = 0x0001,
    HvEventLogModeSequential         = 0x0002,
    HvEventLogModeMaximum            = 0x0002
} HV_EVENTLOG_MODE, *PHV_EVENTLOG_MODE;

typedef struct _HV_EVENTLOG_INIT_TYPE
{
    UINT16 EventLogType;
    UINT16 EventLogMode;
}HV_EVENTLOG_INIT_TYPE, *PHV_EVENTLOG_INIT_TYPE;

//
// Define all the trace buffer states.
//

typedef enum
{
    HvEventLogBufferStateStandby            = 0,
    HvEventLogBufferStateFree               = 1,
    HvEventLogBufferStateInUse              = 2,
    HvEventLogBufferStateComplete           = 3,
    HvEventLogBufferStateReady              = 4,
    HvEventLogBufferStateUnmapping          = 5,
    HvEventLogBufferStateDeleting           = 6,
    HvEventLogBufferStateProcessorListInUse = 7
} HV_EVENTLOG_BUFFER_STATE;


//
// Define time source enum and structure.
//

typedef enum
{
    HvEventLogEntryTimeReference = 0,
    HvEventLogEntryTimeTsc       = 1,
    HvEventLogEntryTimeQpc       = 2
} HV_EVENTLOG_ENTRY_TIME_BASIS;

//
// Define trace parameter constants
//

#define HV_EVENTLOG_MAX_BUFFER_SIZE_IN_PAGES 512
#define HV_EVENTLOG_MAX_BUFFER_COUNT 640

//
// Define extended trace mode types.
//

typedef enum _HV_EVENTLOG_EXTENDED_TRACE_MODE
{
    HvEventLogExtendedModeNone,
    HvEventLogExtendedModeLegacy,
    HvEventLogExtendedModeScenario,
    HvEventLogExtendedModeGranular,
    HvEventLogExtendedModeCount

} HV_EVENTLOG_EXTENDED_TRACE_MODE, *PHV_EVENTLOG_EXTENDED_TRACE_MODE;

typedef enum _HV_EVENTLOG_EXTENDED_GRANULAR_OPERATION
{
    HvEventLogGranularSet,
    HvEventLogGranularAdd,
    HvEventLogGranularRemove,
    HvEventLogGranularOperationCount

} HV_EVENTLOG_EXTENDED_GRANULAR_OPERATION,
  *PHV_EVENTLOG_EXTENDED_GRANULAR_OPERATION;

typedef union _HV_EVENTLOG_EXTENDED_TRACE_FLAGS
{
    struct
    {
        UINT64 Rsvd: 8;
        UINT64 Id: 8;
        UINT64 RsvdZ: 48;

    } Scenario;

    struct
    {
        UINT64 Rsvd: 8;
        UINT64 Operation: 8;
        UINT64 RsvdZ: 48;

    } Granular;

    struct
    {
        UINT64 Flags;

    } Legacy;

    struct
    {
        UINT64 Extended: 1;
        UINT64 Mode: 7;
        UINT64 Rsvd: 56;

    } Common;

    UINT64 AsUINT64;

} HV_EVENTLOG_EXTENDED_TRACE_FLAGS, *PHV_EVENTLOG_EXTENDED_TRACE_FLAGS;

C_ASSERT(sizeof(HV_EVENTLOG_EXTENDED_TRACE_FLAGS) == 8);

//
// Define trace buffer header.
//

typedef struct _HV_EVENTLOG_BUFFER_HEADER
{
    UINT32                         BufferSize;        // BufferSize
    HV_EVENTLOG_BUFFER_INDEX       BufferIndex;       // SavedOffset
    UINT32                         EventsLost;        // CurrentOffset
    volatile UINT32                ReferenceCounter;  // ReferenceCount

    union
    {
        UINT64                     TimeStamp;         // TimeStamp
        HV_NANO100_TIME            ReferenceTime;
    };
    UINT64                         Reserved1;         // SequenceNumber

    UINT64                         Reserved2;         // Padding0

    struct                                            // ClientContext
    {
        UINT16                     LogicalProcessor;  // ProcessorNumber
        UINT16                     LoggerId;
    };
    volatile HV_EVENTLOG_BUFFER_STATE BufferState;    // State (Free/GeneralLogging/Flush)

    UINT32                         NextBufferOffset;  // Offset

    union
    {
        HV_EVENTLOG_TYPE           Type;              // BufferFlag and BufferType
        struct
        {
            UINT16                 BufferFlag;
            UINT16                 BufferType;
        };
    };
    HV_EVENTLOG_BUFFER_INDEX       NextBufferIndex;   // Padding1
    UINT32                         Reserved3;         // Padding1

    UINT32                         Reserved4[2];      // Padding1

} HV_EVENTLOG_BUFFER_HEADER, *PHV_EVENTLOG_BUFFER_HEADER;


//
// Define trace entry header.
//
typedef struct _HV_EVENTLOG_ENTRY_HEADER
{
    UINT32              Context;    // Marker
    UINT16              Size;       // Size in WMI_TRACE_PACKET
    UINT16              Type;       // HookId in WMI_TRACE_PACKET

    union
    {
        UINT64          TimeStamp;
        HV_NANO100_TIME ReferenceTime;
    };
} HV_EVENTLOG_ENTRY_HEADER, *PHV_EVENTLOG_ENTRY_HEADER;

//
// Definition of the HvCallAllocateBufferGroup hypercall input
// structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_CREATE_EVENTLOG_BUFFER
{

    HV_EVENTLOG_TYPE   EventLogType;
    HV_EVENTLOG_BUFFER_INDEX BufferIndex;
    HV_PROXIMITY_DOMAIN_INFO ProximityInfo;

} HV_INPUT_CREATE_EVENTLOG_BUFFER, *PHV_INPUT_CREATE_EVENTLOG_BUFFER;

//
// Definition of the HvCallCreateEventLogBuffer hypercall input
// structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_DELETE_EVENTLOG_BUFFER
{

    HV_EVENTLOG_TYPE EventLogType;
    HV_EVENTLOG_BUFFER_INDEX BufferIndex;

} HV_INPUT_DELETE_EVENTLOG_BUFFER, *PHV_INPUT_DELETE_EVENTLOG_BUFFER;

//
// Definition of the HvCallRequestEventLogGroupFlush hypercall input
// structure.
//


typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_FLUSH_EVENTLOG_BUFFER
{

    HV_EVENTLOG_TYPE EventLogType;
    HV_EVENTLOG_BUFFER_INDEX BufferIndex;

} HV_INPUT_FLUSH_EVENTLOG_BUFFER, *PHV_INPUT_FLUSH_EVENTLOG_BUFFER;


//
// Definition of the HvCallInitialzeEventLogBufferGroup hypercall input
// structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_INITIALIZE_EVENTLOG_BUFFER_GROUP
{

    HV_EVENTLOG_INIT_TYPE EventLogInitType;
    UINT32 MaximumBufferCount;
    UINT32 BufferSizeInBytes;
    UINT32 Threshold;
    HV_EVENTLOG_ENTRY_TIME_BASIS TimeBasis;
    HV_NANO100_TIME SystemTime;
} HV_INPUT_INITIALIZE_EVENTLOG_BUFFER_GROUP,
*PHV_INPUT_INITIALIZE_EVENTLOG_BUFFER_GROUP;


//
// Definition of the HvCallFinalizeEventLogBufferGroup hypercall input
// structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_FINALIZE_EVENTLOG_BUFFER_GROUP
{

    HV_EVENTLOG_TYPE EventLogType;

} HV_INPUT_FINALIZE_EVENTLOG_BUFFER_GROUP,
*PHV_INPUT_FINALIZE_EVENTLOG_BUFFER_GROUP;

//
// Definition of the HvCallMapEventLogBuffer hypercall input
// structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_MAP_EVENTLOG_BUFFER
{

    HV_EVENTLOG_TYPE EventLogType;
    HV_EVENTLOG_BUFFER_INDEX BufferIndex;

} HV_INPUT_MAP_EVENTLOG_BUFFER, *PHV_INPUT_MAP_EVENTLOG_BUFFER;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_MAP_EVENTLOG_BUFFER
{
    HV_GPA_PAGE_NUMBER GpaPageNumbers[512];

} HV_OUTPUT_MAP_EVENTLOG_BUFFER, *PHV_OUTPUT_MAP_EVENTLOG_BUFFER;


//
// Definition of the HvCallUnmapEventLogBuffer hypercall input
// structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_UNMAP_EVENTLOG_BUFFER
{

    HV_EVENTLOG_TYPE EventLogType;
    HV_EVENTLOG_BUFFER_INDEX BufferIndex;

} HV_INPUT_UNMAP_EVENTLOG_BUFFER, *PHV_INPUT_UNMAP_EVENTLOG_BUFFER;


typedef struct HV_EVENTLOG_EVENTGROUP_CONFIGURATION
{

    UINT32 GroupId;
    UINT16 RsvdZ;
    UINT16 EventCount;
    UINT8 EventId[256];

} HV_EVENTLOG_EVENTGROUP_CONFIGURATION, *PHV_EVENTLOG_EVENTGROUP_CONFIGURATION;

//
// Definition of the HvCallSetEventLogGroupSources hypercall input
// structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_EVENTLOG_SET_EVENTS
{

    HV_EVENTLOG_TYPE EventLogType;
    UINT32           GroupCount;
    UINT64           ConfigurationFlags;

    HV_EVENTLOG_EVENTGROUP_CONFIGURATION Groups[2];

} HV_INPUT_EVENTLOG_SET_EVENTS, *PHV_INPUT_EVENTLOG_SET_EVENTS;

//
// Definition of the HvCallReleaseEventLogBuffer hypercall input
// structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_EVENTLOG_RELEASE_BUFFER
{

    HV_EVENTLOG_TYPE EventLogType;
    HV_EVENTLOG_BUFFER_INDEX BufferIndex;

} HV_INPUT_EVENTLOG_RELEASE_BUFFER, *PHV_INPUT_EVENTLOG_RELEASE_BUFFER;

//
// The following are the two hypervisor event groups defined by ETW. They
// need to be in sync with the two ETW constants defined in
// minkernel\ntos\published\base\ntwmi.w:
//
// sdpublic / %SDXROOT%.public.%FLAVOR% ...\sdk\inc\minwin\ntwmi.h
//
//      #define EVENT_TRACE_GROUP_HYPERVISOR           0x1D00
//      #define EVENT_TRACE_GROUP_HYPERVISORX          0x1E00
//
// Note, only the high byte is used for group. The lower byte is used
// for the event types in the group.
//
#define HV_TR_EVENTLOG_GROUP_HYPERVISOR     0x1D00
#define HV_TR_EVENTLOG_GROUP_HYPERVISORX    0x1E00


//
// Event & Trace Groups.
//
#define HV_TR_GROUP_NONE            0x0000000000000000

//
// Event Groups (Admin, Operational, Audit, ...)
//
#define HV_EVENTLOG_ENABLE_AUDIT_SUCCESS    0x0000000000000001
#define HV_EVENTLOG_ENABLE_AUDIT_FAILURE    0x0000000000000002
#define HV_EVENTLOG_ENABLE_PARTITION        0x0000000000000004
#define HV_EVENTLOG_ENABLE_IOMMU            0x0000000000000008
#define HV_EVENTLOG_ENABLE_TEST             0x8000000000000000

//
// Diagnostic Trace Groups (starting at 0x0000000000000001)
//

//
// Extended trace group mask - if this bit is set we have a completely different
// interpretation (non-bitmask oriented) of the remainder of the fields.
//

#define HV_TR_GROUP_EXTENDED        0x0000000000000001

//
// Available groups
//                                  0x0000000000000002
//                                  0x0000000000000004
//                                  0x0000000000000008
//                                  0x0000000000000010
//                                  0x0000000000000020
//                                  0x0000000000000040
//                                  0x0000000000000080


//
// Retail Performance Trace Groups (starting at 0x0000000000000100)
//
// ISSUE-kbroas-2010/10/23:  The following groups do not appear to be utilized
// and could be reclaimed:
//
// _AM
// _BM
// _DM
// _IC
// _MM
// _SYNIC_TI
// _TI
// _VAL
// _VM
//

// begin diagnostic_groups

#define HV_TR_GROUP_BM              0x0000000000000100
#define HV_TR_GROUP_DM              0x0000000000000200
#define HV_TR_GROUP_HC              0x0000000000000400
#define HV_TR_GROUP_IM              0x0000000000000800
#define HV_TR_GROUP_IC              0x0000000000001000
#define HV_TR_GROUP_OB              0x0000000000002000
#define HV_TR_GROUP_PT              0x0000000000004000
#define HV_TR_GROUP_VP              0x0000000000008000
#define HV_TR_GROUP_SYNIC           0x0000000000010000
#define HV_TR_GROUP_SYNIC_TI        0x0000000000020000
#define HV_TR_GROUP_AM_GVA          0x0000000000040000
#define HV_TR_GROUP_AM              0x0000000000080000
#define HV_TR_GROUP_VAL             0x0000000000100000
#define HV_TR_GROUP_VM              0x0000000000200000
#define HV_TR_GROUP_SCH             0x0000000000400000
#define HV_TR_GROUP_TH              0x0000000000800000
#define HV_TR_GROUP_TI              0x0000000001000000
#define HV_TR_GROUP_KE              0x0000000002000000
#define HV_TR_GROUP_MM              0x0000000004000000
#define HV_TR_GROUP_PROFILER        0x0000000008000000
#define HV_TR_GROUP_USCH            0x0000000010000000
#define HV_TR_GROUP_GENERIC         0x0000000020000000

#define HV_TR_ALL_GROUPS (HV_TR_GROUP_BM | HV_TR_GROUP_DM | HV_TR_GROUP_HC | \
    HV_TR_GROUP_IM | HV_TR_GROUP_IC | HV_TR_GROUP_OB | \
    HV_TR_GROUP_PT | HV_TR_GROUP_VP | HV_TR_GROUP_SYNIC | \
    HV_TR_GROUP_SYNIC_TI | HV_TR_GROUP_AM_GVA | HV_TR_GROUP_AM | \
    HV_TR_GROUP_VAL | HV_TR_GROUP_VM | HV_TR_GROUP_SCH | \
    HV_TR_GROUP_TH | HV_TR_GROUP_TI | HV_TR_GROUP_KE | \
    HV_TR_GROUP_MM | HV_TR_GROUP_PROFILER | HV_TR_GROUP_USCH | HV_TR_GROUP_GENERIC)

#define HV_TR_IS_GROUP_RETAIL(_Group_) \
    (((UINT64)(_Group_) > 0) && \
    (((UINT64)(_Group_) & HV_TR_ALL_GROUPS) != 0) && \
    (((UINT64)(_Group_) & ((UINT64)(_Group_) - 1)) == 0))

//
// Internal Debugging Trace Groups (starting at 0x0000010000000000)
//
// ISSUE-howardt-2012/06/27:  The following groups do not appear to be utilized
// and could be reclaimed:
//
// _BM_INTERNAL
// _HC_INTERNAL
// _OB_INTERNAL
// _PT_INTERNAL
// _VP_INTERNAL
// _VAL_INTERNAL
// _VM_INTERNAL
// _MM_INTERNAL
//

#define HV_TR_GROUP_BM_INTERNAL       0x0000010000000000
#define HV_TR_GROUP_DM_INTERNAL       0x0000020000000000
#define HV_TR_GROUP_HC_INTERNAL       0x0000040000000000
#define HV_TR_GROUP_IM_INTERNAL       0x0000080000000000
#define HV_TR_GROUP_IC_INTERNAL       0x0000100000000000
#define HV_TR_GROUP_OB_INTERNAL       0x0000200000000000
#define HV_TR_GROUP_PT_INTERNAL       0x0000400000000000
#define HV_TR_GROUP_VP_INTERNAL       0x0000800000000000
#define HV_TR_GROUP_SYNIC_INTERNAL    0x0001000000000000
#define HV_TR_GROUP_SYNIC_TI_INTERNAL 0x0002000000000000
#define HV_TR_GROUP_AM_GVA_INTERNAL   0x0004000000000000
#define HV_TR_GROUP_AM_INTERNAL       0x0008000000000000
#define HV_TR_GROUP_VAL_INTERNAL      0x0010000000000000
#define HV_TR_GROUP_VM_INTERNAL       0x0020000000000000
#define HV_TR_GROUP_SCH_INTERNAL      0x0040000000000000
#define HV_TR_GROUP_USCH_INTERNAL     0x0080000000000000
#define HV_TR_GROUP_TI_INTERNAL       0x0100000000000000
#define HV_TR_GROUP_KE_INTERNAL       0x0200000000000000
#define HV_TR_GROUP_MM_INTERNAL       0x0400000000000000
#define HV_TR_GROUP_TR_INTERNAL       0x0800000000000000

//
// Tf, simulate full buffers and cyclic buffers are currently only
// supported for TEST_FEATURES_ENABLED builds.
//
#define HV_TR_GROUP_TF                0x1000000000000000

//
// IceCap Trace Group.
//
#define HV_TR_GROUP_ICE               0x8000000000000000

// end diagnostic_groups

#define HV_TR_ALL_GROUPS_INTERNAL (HV_TR_GROUP_BM_INTERNAL | \
    HV_TR_GROUP_DM_INTERNAL | HV_TR_GROUP_HC_INTERNAL | \
    HV_TR_GROUP_IM_INTERNAL | HV_TR_GROUP_IC_INTERNAL | \
    HV_TR_GROUP_OB_INTERNAL | HV_TR_GROUP_PT_INTERNAL | \
    HV_TR_GROUP_VP_INTERNAL | HV_TR_GROUP_SYNIC_INTERNAL | \
    HV_TR_GROUP_SYNIC_TI_INTERNAL | HV_TR_GROUP_AM_GVA_INTERNAL | \
    HV_TR_GROUP_AM_INTERNAL | HV_TR_GROUP_VAL_INTERNAL | \
    HV_TR_GROUP_VM_INTERNAL | HV_TR_GROUP_SCH_INTERNAL | \
    HV_TR_GROUP_USCH_INTERNAL | HV_TR_GROUP_TI_INTERNAL | \
    HV_TR_GROUP_KE_INTERNAL | HV_TR_GROUP_MM_INTERNAL | \
    HV_TR_GROUP_TF | HV_TR_GROUP_ICE)

#define HV_TR_IS_GROUP_INTERNAL(_Group_) \
    (((UINT64)(_Group_) > 0) && \
    (((UINT64)(_Group_) & HV_TR_ALL_GROUPS_INTERNAL) != 0) && \
    (((UINT64)(_Group_) & ((UINT64)(_Group_) - 1)) == 0))

//
// We support 8 bit event IDs.
//

#define HV_TR_MAX_EVENT_ID                  0xFF

// begin trace_events

//
// Trace Types for the diagnostic group.
//

//
// Retail Bm Trace Types (0x1D 00-0F)
//
// None.
//

//
// Retail Dm Trace Types (0x1D 10-19).
//
// None.
//

//
// Retail Hc Trace Types (0x1D 1A-1F).
//

#define HV_TR_HC_HYPERCALL                  0x1A

//
// Retail Im Trace Types (0x1D 20-3B).
//

#define HV_TR_IM_GUEST_EXCEPTION            0x20
#define HV_TR_IM_MSR_READ                   0x21
#define HV_TR_IM_MSR_WRITE                  0x22
#define HV_TR_IM_CR_READ                    0x23
#define HV_TR_IM_CR_WRITE                   0x24
#define HV_TR_IM_HLT_INSTRUCTION            0x25
#define HV_TR_IM_MWAIT_INSTRUCTION          0x26
#define HV_TR_IM_CPUID_INSTRUCTION          0x27
#define HV_TR_IM_IO_PORT_READ               0x28
#define HV_TR_IM_IO_PORT_WRITE              0x29
#define HV_TR_IM_EXTERNAL_INTERRUPT         0x2A
#define HV_TR_IM_INTERRUPT_PENDING          0x2B
#define HV_TR_IM_GUEST_SHUTDOWN             0x2C
#define HV_TR_IM_EMULATED_INSTRUCTION       0x2D
#define HV_TR_IM_NMI_INTERRUPT              0x2E
#define HV_TR_IM_INVLPG_INSTRUCTION         0x2F
#define HV_TR_IM_IRET_INSTRUCTION           0x30
#define HV_TR_IM_TASK_SWITCH                0x31
#define HV_TR_IM_INVD_INSTRUCTION           0x32
#define HV_TR_IM_DR_ACCESS                  0x33
#define HV_TR_IM_FERR_FREEZE                0x34
#define HV_TR_IM_REAL_MODE_INTERRUPT        0x35
#define HV_TR_IM_MEMORY_INTERCEPT           0x36
#define HV_TR_IM_REFLECTED_EXCEPTION        0x37
#define HV_TR_IM_APIC_EOI                   0x38
#define HV_TR_IM_APIC_WRITE                 0x39
#define HV_TR_IM_APIC_ACCESS                0x3A
#define HV_TR_IM_NESTED_PAGE_FAULT          0x3B
#define HV_TR_IM_PAUSE_LOOP_EXIT            0x3C

//
// Retail Ic Trace Types (0x1D 3D-3F).
//
// None.
//

//
// Retail Ob Trace Types (0x1D 40-45).
//

#define HV_TR_OB_CREATE_PARTITION           0x40
#define HV_TR_OB_DELETE_PARTITION           0x41
#define HV_TR_OB_RESTORE_PARTITION          0x42
#define HV_TR_OB_RUNDOWN_PARTITION          0x43
#define HV_TR_OB_INIT_PARTITION             0x44
#define HV_TR_OB_SCRUB_PARTITION            0x45

//
// Retail Pt Trace Types (0x1D 46-4A).
//

#define HV_TR_PT_REFERENCE_TIME             0x46

//
// Retail Vp Trace Types (0x1D 4B-4F).
//

#define HV_TR_VP_CREATE_VP                  0x4B
#define HV_TR_VP_DELETE_VP                  0x4C

//
// Retail Synic Trace Types (0x1D 50-65).
//

#define HV_TR_SYNIC_DUMP_INT_MAPPING        0x50
#define HV_TR_SYNIC_CREATE_INT_MAPPING      0x51
#define HV_TR_SYNIC_FREE_INT_MAPPING        0x52
#define HV_TR_SYNIC_THROTTLE                0x53
#define HV_TR_SYNIC_UNTHROTTLE              0x54
#define HV_TR_SYNIC_LPSET_INT_MAPPING       0x55

//
// Retail Synic Timer Trace Types (0x1D 66-6F).
//
// None.
//

//
// Retail Am GVA Trace Types (0x1D 70-97).
//

#define HV_TR_AM_GVA_GROW_VIRTUAL_TLB       0x70
#define HV_TR_AM_GVA_SHRINK_VIRTUAL_TLB     0x71
#define HV_TR_AM_GVA_FLUSH_VIRTUAL_TLB      0x72

//
// Retail Am Trace Types (0x1D 98-AF).
//
// None.
//

//
// Retail Val Trace Types (0x1D A0-A7).
//
// None.
//

//
// Retail Vm Trace Types (0x1D A8-AF).
//
// None.
//

//
// Retail Th Trace Types (0x1D B0).
//

#define HV_TR_TH_CONTEXT_SWITCH             0xB0


//
// Retail Sch Trace Types (0x1D B1-BF).
//

#define HV_TR_SCH_THREAD_RUNNABLE_LOCAL     0xB1
#define HV_TR_SCH_THREAD_RUNNABLE_DEFER     0xB2
#define HV_TR_SCH_EXPRESS                   0xB3
#define HV_TR_SCH_EXPRESS_FAIL              0xB4
#define HV_TR_SCH_APPLY_CAP                 0xB5
#define HV_TR_SCH_SET_CAP_TIMER             0xB6
#define HV_TR_SCH_COMPUTE_TIMESLICE         0xB7

//
// Retail Unit Scheduler Trace Types (0x1D C0-CD).
//

#define HV_TR_USCH_SCHEDULING_UNIT_SWITCH               0xC0
#define HV_TR_USCH_SCHEDULING_UNIT_RUNNABLE_LOCAL       0xC1
#define HV_TR_USCH_SCHEDULING_UNIT_RUNNABLE_DEFERRED    0xC2
#define HV_TR_USCH_EXPRESS_SCHEDULE                     0xC3
#define HV_TR_USCH_EXPRESS_SCHEDULE_FAIL                0xC4
#define HV_TR_USCH_SCHEDULING_UNIT_ACTIVATE_CAP         0xC5
#define HV_TR_USCH_SCHEDULING_UNIT_UPDATE_TIMESLICE     0xC6

//
// Retail Ti Trace Types (0x1D CE-D1).
//
// None.
//

//
// Retail Ke Trace Types (0x1D D2-EE).
//

#define HV_TR_KE_LIVEDUMP                   0xD2
#define HV_TR_KE_PPM_SUMMARY_INFO           0xD4
#define HV_TR_KE_PPM_CSTATE_INFO            0xD5
#define HV_TR_KE_PPM_PSTATE_INFO            0xD6
#define HV_TR_KE_PPM_TSTATE_INFO            0xD7
#define HV_TR_KE_PPM_RUNDOWN_SUMMARY_INFO   0xD8
#define HV_TR_KE_PPM_RUNDOWN_CSTATE_INFO    0xD9
#define HV_TR_KE_PPM_RUNDOWN_PSTATE_INFO    0xDA
#define HV_TR_KE_PPM_RUNDOWN_TSTATE_INFO    0xDB

#define HV_TR_KE_OVERRUN                    0xE0
#define HV_TR_KE_OVERRUN_PARAMS             0xE1

#define HV_TR_KE_CPU_MAPPING                0xE2

#define HV_TR_KE_PPM_IDLE                   0xE3
#define HV_TR_KE_PPM_POLICY_CHANGE          0xE4
#define HV_TR_KE_PPM_CORE_PARK              0xE5
#define HV_TR_KE_PPM_CORE_UNPARK            0xE6
#define HV_TR_KE_PPM_CAP_EVENT              0xE7
#define HV_TR_KE_PPM_PERF_CHECK_START       0xE8
#define HV_TR_KE_PPM_PERF_CHECK_STOP        0xE9
#define HV_TR_KE_PPM_RECORDED_UTILITY       0xEA
#define HV_TR_KE_PPM_EXPECTED_UTILITY       0xEB
#define HV_TR_KE_PPM_DOMAIN_PERF_STATE_CHANGE           0xEC
#define HV_TR_KE_PPM_PROCESSOR_PERF_STATE_CHANGE        0xED
#define HV_TR_KE_PPM_PLATFORM_STATE_CHANGE  0xEE

//
// Retail Mm Trace Types (0x1D EF).
//
// None.
//

//
// Retail Hypervisor Profiler traces (0x1D F0-F4)
//

#define HV_TR_PROFILER_SAMPLE               0xF0
#define HV_TR_PROFILER_HV_MODULE            0xF1

//
// Generic Hypervisor Event (0x1D F5)
//

#define HV_TR_GENERIC_STRING                0xF5

#if defined(_HV_TEST_FEATURES_ENABLED_) || defined(_PERF_FEATURES_ENABLED_) || \
    defined(_HV_COVERAGE_ENABLED_) || DBG

//
// Internal Bm Trace Types (0x1E 00-0F)
//
// None.
//

//
// Internal Dm Trace Types (0x1E 10-19).
//

#define HV_TR_DMP_INTERCEPT                 0x10
#define HV_TR_DMP_DISPATCH_EVENTS           0x11

//
// Internal Hc Trace Types (0x1E 1A-1F).
//
// None.
//

//
// Internal Im Trace Types (0x1E 20-3B).
//

#define HV_TR_IMP_SEND_INTERCEPT_MESSAGE    0x20

//
// Internal Ic Trace Types (0x1E 3C-3C).
//

#define HV_TR_ICP_EMULATE_INSTR             0x3C

//
// Internal Ob Trace Types (0x1E 3D-3D).
//
// None.
//

//
// Internal Pt Trace Types (0x1E 3E-3E).
//
// None.
//

//
// Internal Vp Trace Types (0x1E 3F-3F).
//
// None.
//

//
// Internal Ke Trace Types (0x40-0x4F).
//

//
// Internal Synic Trace Types (0x1E 50-65).
//

#define HV_TR_SYNICP_ASSERT                 0x50
#define HV_TR_SYNICP_EVAL_LOW_PRI           0x51
#define HV_TR_SYNICP_EVAL_IMMEDIATE         0x52
#define HV_TR_SYNICP_EVAL_QUEUE             0x53
#define HV_TR_SYNICP_PENDING                0x54
#define HV_TR_SYNICP_EOI                    0x55
#define HV_TR_SYNICP_LATENCY_EXCEEDED       0x56
#define HV_TR_SYNICP_MARK_PENDING           0x57
#define HV_TR_SYNICP_EVAL_APIC_INTS         0x58
#define HV_TR_SYNICP_READ_APIC_GPA          0x59
#define HV_TR_SYNICP_WRITE_APIC_GPA         0x5A
#define HV_TR_SYNICP_READ_APIC_MSR          0x5B
#define HV_TR_SYNICP_WRITE_APIC_MSR         0x5C
#define HV_TR_SYNICP_EXTERNAL_INT           0x5D
#define HV_TR_SYNICP_APIC_IPI               0x5E
#define HV_TR_SYNICP_SIGNAL_EVENT_PORT      0x5F
#define HV_TR_SYNICP_POST_MESSAGE           0x60

//
// Internal Synic Timer Trace Types (0x1E 66-6F).
//

#define HV_TR_SYNICP_TI_PERIODIC_TIMER_UPDATE  0x66
#define HV_TR_SYNICP_TI_PERIODIC_TIMER_RESET   0x67
#define HV_TR_SYNICP_TI_SEND_MESSAGE        0x68
#define HV_TR_SYNICP_TI_SCAN_MESSAGE        0x69
#define HV_TR_SYNICP_TI_ASSIST_EXPIRE       0x6A

//
// Internal Am GVA Trace Types (0x1E 70-97).
//

#define HV_TR_AMP_GVA_PAGE_FAULT            0x70
#define HV_TR_AMP_GVA_VTLB_WRITABILITY_CHANGE 0x71
#define HV_TR_AMP_GVA_SWITCH_SPACE          0x72
#define HV_TR_AMP_GVA_FLUSH_VA_LOCAL        0x73
#define HV_TR_AMP_GVA_FLUSH_VA_GLOBAL       0x74
#define HV_TR_AMP_GVA_FLUSH_VTLB_RANGE      0x75
#define HV_TR_AMP_GVA_FLUSH_SPACE           0x76
#define HV_TR_AMP_GVA_FLUSH_DOMAIN          0x77
#define HV_TR_AMP_GVA_RESET_DOMAIN          0x78
#define HV_TR_AMP_GVA_TRANSLATE_VA          0x79
#define HV_TR_AMP_GVA_READ_VM               0x7A
#define HV_TR_AMP_GVA_WRITE_VM              0x7B
#define HV_TR_AMP_GVA_RESET_PT              0x7C
#define HV_TR_AMP_GVA_UNLINK_PT             0x7D
#define HV_TR_AMP_GVA_UNLINK_PT_VALIDATE    0x7E
#define HV_TR_AMP_GVA_UNLINK_PT_FILL        0x7F
#define HV_TR_AMP_GVA_UNLINK_PT_LINK        0x80
#define HV_TR_AMP_GVA_UNLINK_PT_RESET       0x81
#define HV_TR_AMP_GVA_UNLINK_PT_EVICT       0x82
#define HV_TR_AMP_GVA_UNLINK_PT_CLEAN       0x83
#define HV_TR_AMP_GVA_CONSTRUCT_PT          0x84
#define HV_TR_AMP_GVA_DESTRUCT_PT           0x85
#define HV_TR_AMP_GVA_WALK_PT               0x86
#define HV_TR_AMP_GVA_WAIT_FOR_PT_FILL      0x87
#define HV_TR_AMP_GVA_CLEAN_SAS             0x88
#define HV_TR_AMP_GVA_CLEAN_SAS_OVERFLOW    0x89
#define HV_TR_AMP_GVA_EVICT_PTS             0x8A
#define HV_TR_AMP_GVA_RECLAIM_PTS           0x8B
#define HV_TR_AMP_GVA_FREE_RECLAIMED_PTS    0x8C
#define HV_TR_AMP_GVA_HASH_PROCESSED        0x8D

//
// Internal Am Trace Types (0x1E 98-9F).
//

#define HV_TR_AMP_MAP_GPA                   0x98
#define HV_TR_AMP_UNMAP_GPA                 0x99

//
// Internal Val Trace Types (0x1E A0-A7).
//
// None.
//

//
// Internal Vm Trace Types (0x1E A8-AF).
//
// None.
//

//
// Internal Sch Trace Types (0x1E B0-C6).
//

#define HV_TR_SCHP_NEXT_THREAD              0xB0

#define HV_TR_SCHP_TIMESLICE_MIN            0xB3

#define HV_TR_SCHP_TIMESLICE_END            0xB8
#define HV_TR_SCHP_EVAL_SEND                0xB9
#define HV_TR_SCHP_EVAL_RECV                0xBA
#define HV_TR_SCHP_YIELD                    0xBB

#define HV_TR_SCHP_BLOCK_ON_EVENT           0xBE
#define HV_TR_SCHP_UNBLOCK_FROM_EVENT       0xBF
#define HV_TR_SCHP_SIGNAL_EVENT             0xC0

#define HV_TR_SCHP_LOAD_BALANCER            0xC2
#define HV_TR_SCHP_EVAL                     0xC3

#define HV_TR_SCHP_READY_UNBLOCKED_THREAD   0xC5
#define HV_TR_SCHP_EXPRESS_THREAD           0xC6

//
// Internal Unit Scheduler Trace Types (0x1E C7-CF).
//

#define HV_TR_USCHP_SCHEDULING_UNIT_BLOCK   0xC7
#define HV_TR_USCHP_SCHEDULING_UNIT_YIELD   0xC8
#define HV_TR_USCHP_SCHEDULING_UNIT_EXPRESS 0xC9

//
// Internal Ti Trace Types (0x1E D0-DF).
//

#define HV_TR_TIP_SET_APIC                  0xD0
#define HV_TR_TIP_INT                       0xD1
#define HV_TR_TIP_NEW_TIMER                 0xD2
#define HV_TR_TIP_INSERT_TIMER              0xD3
#define HV_TR_TIP_CALLBACK                  0xD4
#define HV_TR_TIP_REMOTE_REMOVE_TIMER       0xD5

//
// Internal Ke Trace Types (0x1E E0-E8).
//

#define HV_TR_KEP_FLUSH_ENTIRE_HW_TLB       0xE0
#define HV_TR_KEP_FLUSH_HV_HW_TLB           0xE1
#define HV_TR_KEP_FLUSH_MULTIPLE_HW_TLB     0xE2
#define HV_TR_KEP_SIGNAL_PROCESSORS         0xE3
#define HV_TR_KEP_RUN_GUEST                 0xE4
#define HV_TR_KEP_SEND_IPI                  0xE5
#define HV_TR_KEP_SEND_IPI_NO_IPI           0xE6
#define HV_TR_KEP_WAIT_FOR_IPI_BARRIER      0xE7
#define HV_TR_KEP_FLUSH_ENTIRE_HW_CACHE     0xE8

//
// Internal Mm Trace Types (0x1E E9-EF).
//
// None.
//

//
// Internal Tf Trace Types (0x1E FA-FD).
//

#define HV_TR_TF_INTERCEPT                  0xFA
#define HV_TR_TF_VIRT_MSR_READ              0xFB
#define HV_TR_TF_VIRT_MSR_WRITE             0xFC

//
// Internal Tr Trace Types (0x1E FE-FF)
//

#define HV_TR_TRP_BUFFER_RUNDOWN            0xFE

#endif // defined(_HV_TEST_FEATURES_ENABLED_) || defined(_PERF_FEATURES_ENABLED_) ||
       // defined(_HV_COVERAGE_ENABLED_)


//
// Internal IceCAP Trace Types (0x1E F0-F9).
//

#define HV_TR_ICE_CALL                      0xF0    // fixed (tffastcap.asm)
#define HV_TR_ICE_RETURN                    0xF1    // fixed (tffastcap.asm)
#define HV_TR_ICE_ACQUIRE                   0xF2
#define HV_TR_ICE_RELEASE                   0xF3

//
// Internal Custom Probe Type.
//
// ISSUE-kbroas-2010/10/23:  These types are unused and could be reclaimed.
//

#define HV_TR_ICE_PROFILE                   0xF6
#define HV_TR_ICE_COMMENT                   0xF8

// end trace_events


//
// Physical nodes are defined by a 32-bit index.
//

typedef UINT32 HV_PHYSICAL_NODE_INDEX, *PHV_PHYSICAL_NODE_INDEX;


typedef enum _HV_SAVE_RESTORE_STATE_RESULT
{
    HvStateComplete                                    = 0,
    HvStateIncomplete                                  = 1,
    HvStateCorruptData                                 = 2,
    HvStateUnsupportedVersion                          = 3,
    HvStateProcessorFeatureMismatch                    = 4,
    HvStateIncompatibleProcessor                       = 5,
    HvStateProcessorCacheLineFlushSizeMismatch         = 6,

} HV_SAVE_RESTORE_STATE_RESULT, *PHV_SAVE_RESTORE_STATE_RESULT;

typedef UINT32 HV_SAVE_RESTORE_STATE_FLAGS, *PHV_SAVE_RESTORE_STATE_FLAGS;

#define HV_SAVE_RESTORE_STATE_START   0x00000001
#define HV_SAVE_RESTORE_STATE_SUMMARY 0x00000002

typedef enum _HV_PROCESSOR_VENDOR
{
    HvProcessorVendorAmd        = 0x0000,
    HvProcessorVendorIntel      = 0x0001,

    HvProcessorVendorGenericArm = 0x0010,
    HvProcessorVendorAPM        = 0x0011,
    HvProcessorVendorQualcomm   = 0x0012,
    HvProcessorVendorMediaTek   = 0x0013,
    HvProcessorVendorNvidia     = 0x0014

} HV_PROCESSOR_VENDOR, *PHV_PROCESSOR_VENDOR;


//
// Define the structure defining the processor related features
// that may be de-featured.
//

typedef union _HV_PARTITION_PROCESSOR_FEATURES
{
#if defined(_ARM64_)

    struct
    {
        UINT64 Haf:1;                       // Hardware updates to Access flag.
        UINT64 Hdbs:1;                      // Hardware updates to Dirty state.
        UINT64 Pan:1;                       // Privileged access never.
        UINT64 AtS1E1:1;                    // AT S1E1RP and AT S1E1WP supported.
    };

#else

    struct
    {
        UINT64 Sse3Support:1;
        UINT64 LahfSahfSupport:1;
        UINT64 Ssse3Support:1;
        UINT64 Sse4_1Support:1;
        UINT64 Sse4_2Support:1;
        UINT64 Sse4aSupport:1;
        UINT64 XopSupport:1;
        UINT64 PopCntSupport:1;
        UINT64 Cmpxchg16bSupport:1;
        UINT64 Altmovcr8Support:1;
        UINT64 LzcntSupport:1;
        UINT64 MisAlignSseSupport:1;
        UINT64 MmxExtSupport:1;
        UINT64 Amd3DNowSupport:1;
        UINT64 ExtendedAmd3DNowSupport:1;
        UINT64 Page1GbSupport:1;
        UINT64 AesSupport:1;
        UINT64 PclmulqdqSupport:1;
        UINT64 PcidSupport:1;
        UINT64 Fma4Support:1;
        UINT64 F16CSupport:1;
        UINT64 RdRandSupport:1;
        UINT64 RdWrFsGsSupport:1;
        UINT64 SmepSupport:1;
        UINT64 EnhancedFastStringSupport:1;
        UINT64 Bmi1Support:1;
        UINT64 Bmi2Support:1;
        UINT64 HleSupportDeprecated:1;
        UINT64 RtmSupportDeprecated:1;
        UINT64 MovbeSupport:1;
        UINT64 Npiep1Support:1;
        UINT64 DepX87FPUSaveSupport:1;
        UINT64 RdSeedSupport:1;
        UINT64 AdxSupport:1;
        UINT64 IntelPrefetchSupport:1;
        UINT64 SmapSupport:1;
        UINT64 HleSupport:1;
        UINT64 RtmSupport:1;
        UINT64 RdtscpSupport:1;
        UINT64 ClflushoptSupport:1;
        UINT64 ClwbSupport:1;
        UINT64 ShaSupport:1;
        UINT64 X87PointersSavedSupport:1;
        UINT64 InvpcidSupport:1;
        UINT64 IbrsSupport:1;
        UINT64 StibpSupport:1;
        UINT64 IbpbSupport: 1;
        UINT64 UnrestrictedGuestSupport:1;
        UINT64 Reserved:16;
    };

#endif

    UINT64 AsUINT64;

} HV_PARTITION_PROCESSOR_FEATURES, *PHV_PARTITION_PROCESSOR_FEATURES;

//
// Masks corresponding to x64 processor features.
//
// ARM64HV_WORKITEM:
//

#define HV_X64_PROCESSOR_FEATURE_SSE3                   (1UI64 << 0)
#define HV_X64_PROCESSOR_FEATURE_LAHF_SAHF              (1UI64 << 1)
#define HV_X64_PROCESSOR_FEATURE_SSSE3                  (1UI64 << 2)
#define HV_X64_PROCESSOR_FEATURE_SSE4_1                 (1UI64 << 3)
#define HV_X64_PROCESSOR_FEATURE_SSE4_2                 (1UI64 << 4)
#define HV_X64_PROCESSOR_FEATURE_SSE4_A                 (1UI64 << 5)
#define HV_X64_PROCESSOR_FEATURE_XOP                    (1UI64 << 6)
#define HV_X64_PROCESSOR_FEATURE_POPCNT                 (1UI64 << 7)
#define HV_X64_PROCESSOR_FEATURE_CMPXCHG16B             (1UI64 << 8)
#define HV_X64_PROCESSOR_FEATURE_ALTMOVCR8              (1UI64 << 9)
#define HV_X64_PROCESSOR_FEATURE_LZCNT                  (1UI64 << 10)
#define HV_X64_PROCESSOR_FEATURE_MISALIGNSSE            (1UI64 << 11)
#define HV_X64_PROCESSOR_FEATURE_MMX_EXT                (1UI64 << 12)
#define HV_X64_PROCESSOR_FEATURE_AMD_3DNOW              (1UI64 << 13)
#define HV_X64_PROCESSOR_FEATURE_EXT_AMD_3DNOW          (1UI64 << 14)
#define HV_X64_PROCESSOR_FEATURE_PAGE_1GB               (1UI64 << 15)
#define HV_X64_PROCESSOR_FEATURE_AES                    (1UI64 << 16)
#define HV_X64_PROCESSOR_FEATURE_PCLMULQDQ              (1UI64 << 17)
#define HV_X64_PROCESSOR_FEATURE_PCID                   (1UI64 << 18)
#define HV_X64_PROCESSOR_FEATURE_FMA4                   (1UI64 << 19)
#define HV_X64_PROCESSOR_FEATURE_F16C                   (1UI64 << 20)
#define HV_X64_PROCESSOR_FEATURE_RDRAND                 (1UI64 << 21)
#define HV_X64_PROCESSOR_FEATURE_RDWRFSGS               (1UI64 << 22)
#define HV_X64_PROCESSOR_FEATURE_SMEP                   (1UI64 << 23)
#define HV_X64_PROCESSOR_FEATURE_ENHANCED_FAST_STRING   (1UI64 << 24)
#define HV_X64_PROCESSOR_FEATURE_BMI1                   (1UI64 << 25)
#define HV_X64_PROCESSOR_FEATURE_BMI2                   (1UI64 << 26)
#define HV_X64_PROCESSOR_FEATURE_HLE_DEPRECATED         (1UI64 << 27)
#define HV_X64_PROCESSOR_FEATURE_RTM_DEPRECATED         (1UI64 << 28)
#define HV_X64_PROCESSOR_FEATURE_MOVBE                  (1UI64 << 29)
#define HV_X64_PROCESSOR_FEATURE_NPIEP1                 (1UI64 << 30)
#define HV_X64_PROCESSOR_FEATURE_DEP_X87FPU_SAVE        (1UI64 << 31)
#define HV_X64_PROCESSOR_FEATURE_RDSEED                 (1UI64 << 32)
#define HV_X64_PROCESSOR_FEATURE_ADX                    (1UI64 << 33)
#define HV_X64_PROCESSOR_FEATURE_INTEL_PREFETCH         (1UI64 << 34)
#define HV_X64_PROCESSOR_FEATURE_SMAP                   (1UI64 << 35)
#define HV_X64_PROCESSOR_FEATURE_HLE                    (1UI64 << 36)
#define HV_X64_PROCESSOR_FEATURE_RTM                    (1UI64 << 37)
#define HV_X64_PROCESSOR_FEATURE_RDTSCP                 (1UI64 << 38)
#define HV_X64_PROCESSOR_FEATURE_CLFLUSHOPT             (1UI64 << 39)
#define HV_X64_PROCESSOR_FEATURE_CLWB                   (1UI64 << 40)
#define HV_X64_PROCESSOR_FEATURE_SHA                    (1UI64 << 41)
#define HV_X64_PROCESSOR_FEATURE_X87_POINTERS_SAVED     (1UI64 << 42)
#define HV_X64_PROCESSOR_FEATURE_INVPCID                (1UI64 << 43)
#define HV_X64_PROCESSOR_FEATURE_IBRS                   (1UI64 << 44)
#define HV_X64_PROCESSOR_FEATURE_STIBP                  (1UI64 << 45)
#define HV_X64_PROCESSOR_FEATURE_IBPB                   (1UI64 << 46)
#define HV_X64_PROCESSOR_FEATURE_UNRESTRICTED_GUEST     (1UI64 << 47)


//
// Define the processor features available in Intel and AMD compatibility mode.
//
// N.B. - Support for STIBP, IBRS, and IBPB was added to the general compatibility mask
// but will effectively only be made available to RS1 and above VMs since those
// processor features are versioned RS1 and above.
//

#define HV_PARTITION_PROCESSOR_FEATURES_INTEL_COMPATIBILITY_MODE \
    ( HV_X64_PROCESSOR_FEATURE_SSE3                     \
    | HV_X64_PROCESSOR_FEATURE_LAHF_SAHF                \
    | HV_X64_PROCESSOR_FEATURE_CMPXCHG16B               \
    | HV_X64_PROCESSOR_FEATURE_IBRS                     \
    | HV_X64_PROCESSOR_FEATURE_IBPB                     \
    )


#define HV_PARTITION_PROCESSOR_FEATURES_AMD_COMPATIBILITY_MODE \
    ( HV_X64_PROCESSOR_FEATURE_SSE3                     \
    | HV_X64_PROCESSOR_FEATURE_LAHF_SAHF                \
    | HV_X64_PROCESSOR_FEATURE_CMPXCHG16B               \
    | HV_X64_PROCESSOR_FEATURE_ALTMOVCR8                \
    | HV_X64_PROCESSOR_FEATURE_MMX_EXT                  \
    | HV_X64_PROCESSOR_FEATURE_NPIEP1                   \
    | HV_X64_PROCESSOR_FEATURE_IBPB                     \
    )


//
// Define the structure defining the processor XSAVE related features
// that may be de-featured.
//

typedef union _HV_PARTITION_PROCESSOR_XSAVE_FEATURES
{
    struct
    {
        UINT64 XsaveSupport:1;
        UINT64 XsaveoptSupport:1;
        UINT64 AvxSupport:1;
        UINT64 Avx2Support:1;
        UINT64 FmaSupport:1;
        UINT64 MpxSupport:1;
        UINT64 Avx512Support:1;
        UINT64 Avx512DQSupport:1;
        UINT64 Avx512CDSupport:1;
        UINT64 Avx512BWSupport:1;
        UINT64 Avx512VLSupport:1;
        UINT64 XsaveCompSupport:1;
        UINT64 XsaveSupervisorSupport:1;
        UINT64 Xcr1Support:1;
        UINT64 Reserved:50;
    };
    UINT64 AsUINT64;

} HV_PARTITION_PROCESSOR_XSAVE_FEATURES, *PHV_PARTITION_PROCESSOR_XSAVE_FEATURES;

#define HV_X64_PROCESSOR_XSAVE_FEATURE_XSAVE                (1UI64 << 0)
#define HV_X64_PROCESSOR_XSAVE_FEATURE_XSAVE_OPT            (1UI64 << 1)
#define HV_X64_PROCESSOR_XSAVE_FEATURE_AVX                  (1UI64 << 2)
#define HV_X64_PROCESSOR_XSAVE_FEATURE_AVX2                 (1UI64 << 3)
#define HV_X64_PROCESSOR_XSAVE_FEATURE_FMA                  (1UI64 << 4)
#define HV_X64_PROCESSOR_XSAVE_FEATURE_MPX                  (1UI64 << 5)
#define HV_X64_PROCESSOR_XSAVE_FEATURE_AVX512               (1UI64 << 6)
#define HV_X64_PROCESSOR_XSAVE_FEATURE_AVX512DQ             (1UI64 << 7)
#define HV_X64_PROCESSOR_XSAVE_FEATURE_AVX512CD             (1UI64 << 8)
#define HV_X64_PROCESSOR_XSAVE_FEATURE_AVX512BW             (1UI64 << 9)
#define HV_X64_PROCESSOR_XSAVE_FEATURE_AVX512VL             (1UI64 << 10)
#define HV_X64_PROCESSOR_XSAVE_FEATURE_XSAVE_COMP           (1UI64 << 11)
#define HV_X64_PROCESSOR_XSAVE_FEATURE_XSAVE_SUPERVISOR     (1UI64 << 12)
#define HV_X64_PROCESSOR_XSAVE_FEATURE_XCR1                 (1UI64 << 13)

//
// Mask for processor XSAVE features supported in TH2. Components managing the
// features should use this mask against returned processor features for any
// guest version TH2 or earlier.
//
#define HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_TH2_AND_EARLIER \
    ( HV_X64_PROCESSOR_XSAVE_FEATURE_XSAVE              \
    | HV_X64_PROCESSOR_XSAVE_FEATURE_XSAVE_OPT          \
    | HV_X64_PROCESSOR_XSAVE_FEATURE_AVX                \
    | HV_X64_PROCESSOR_XSAVE_FEATURE_AVX2               \
    | HV_X64_PROCESSOR_XSAVE_FEATURE_FMA                \
    )

//
// Mask for processor XSAVE features supported in Server 2016 TP5 VMs.
//
#define HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_SERVER2016_TP5 \
    HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_TH2_AND_EARLIER

//
// Mask for processor XSAVE features supported in Redstone 1 VMs.
//
#define HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_REDSTONE_1 \
    ( HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_SERVER2016_TP5 \
    | HV_X64_PROCESSOR_XSAVE_FEATURE_MPX                        \
    | HV_X64_PROCESSOR_XSAVE_FEATURE_AVX512                     \
    | HV_X64_PROCESSOR_XSAVE_FEATURE_AVX512DQ                   \
    | HV_X64_PROCESSOR_XSAVE_FEATURE_AVX512CD                   \
    | HV_X64_PROCESSOR_XSAVE_FEATURE_AVX512BW                   \
    | HV_X64_PROCESSOR_XSAVE_FEATURE_AVX512VL                   \
    | HV_X64_PROCESSOR_XSAVE_FEATURE_XSAVE_COMP                 \
    | HV_X64_PROCESSOR_XSAVE_FEATURE_XSAVE_SUPERVISOR           \
    )

//
// Mask for processor XSAVE features supported in Redstone 2 VMs.
//
#define HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_REDSTONE_2 \
    ( HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_REDSTONE_1 \
    | HV_X64_PROCESSOR_XSAVE_FEATURE_XCR1                   \
    )

//
// Mask for processor XSAVE features supported in Redstone 3 VMs.
//
#define HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_REDSTONE_3 \
    ( HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_REDSTONE_2 \
    )

//
// Mask for processor XSAVE features supported in Redstone 4 VMs.
//
#define HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_REDSTONE_4 \
    ( HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_REDSTONE_3 \
    )

//
// Mask for processor XSAVE features supported in Redstone 5 VMs.
//
#define HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_REDSTONE_5 \
    ( HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_REDSTONE_4 \
    )

//
// Mask for processor XSAVE features supported in Prerelease VMs.
//
#define HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_PRERELEASE \
    ( HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_REDSTONE_4 \
    )

//
// Mask for processor XSAVE features supported in Experimental VMs.
//
#define HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_EXPERIMENT \
    ( HV_PARTITION_PROCESSOR_XSAVE_FEATURES_MASK_PRERELEASE \
    )

//
// Define the xsave processor features avaialble in Intel and AMD compatibility
// mode.
//

#define HV_PARTITION_PROCESSOR_XSAVE_FEATURES_INTEL_COMPATIBILITY_MODE (0)

#define HV_PARTITION_PROCESSOR_XSAVE_FEATURES_AMD_COMPATIBILITY_MODE (0)

//
// Define the processor cache line flush size Intel and AMD compatibility mode.
//

#define HV_PARTITION_PROCESSOR_CL_FLUSHSIZE_INTEL_COMPATIBILITY_MODE (8)
#define HV_PARTITION_PROCESSOR_CL_FLUSHSIZE_AMD_COMPATIBILITY_MODE (8)

//
// Define the VSM capabilities that are version controlled.
//

#define HV_PARTITION_VSM_CAPABILITIES_NONE              (0UI64)
#define HV_PARTITION_VSM_CAPABILITIES_VTL0_MBEC         (1UI64 << 1)
#define HV_PARTITION_VSM_CAPABILITIES_DENY_VP_STARTUP   (1UI64 << 17)

//
// Mask of VSM capabilities supported in VM versions prior to RS1.
//
#define HV_COMPATIBILITY_VSM_CAPABILITIES_MASK_TP5_AND_EARLIER \
    ( HV_PARTITION_VSM_CAPABILITIES_NONE )

//
// Mask of VSM capabilities supported in RS1 VMs.
//
#define HV_COMPATIBILITY_VSM_CAPABILITIES_MASK_REDSTONE_1   \
    ( HV_PARTITION_VSM_CAPABILITIES_NONE                    \
    | HV_PARTITION_VSM_CAPABILITIES_VTL0_MBEC               \
    )

//
// Mask of VSM capabilities supported in RS2 VMs.
//
#define HV_COMPATIBILITY_VSM_CAPABILITIES_MASK_REDSTONE_2   \
    ( HV_COMPATIBILITY_VSM_CAPABILITIES_MASK_REDSTONE_1     \
    | HV_PARTITION_VSM_CAPABILITIES_DENY_VP_STARTUP         \
    )

//
// Mask of VSM capabilities supported in RS3 VMs.
//
#define HV_COMPATIBILITY_VSM_CAPABILITIES_MASK_REDSTONE_3   \
    ( HV_COMPATIBILITY_VSM_CAPABILITIES_MASK_REDSTONE_2 )

//
// Mask of VSM capabilities supported in RS4 VMs.
//
#define HV_COMPATIBILITY_VSM_CAPABILITIES_MASK_REDSTONE_4   \
    ( HV_COMPATIBILITY_VSM_CAPABILITIES_MASK_REDSTONE_3 )

//
// Mask of VSM capabilities supported in RS5 VMs.
//
#define HV_COMPATIBILITY_VSM_CAPABILITIES_MASK_REDSTONE_5   \
    ( HV_COMPATIBILITY_VSM_CAPABILITIES_MASK_REDSTONE_4 )

//
// Mask of VSM capabilities supported in Prerelease VMs.
//
#define HV_COMPATIBILITY_VSM_CAPABILITIES_MASK_PRERELEASE   \
    ( HV_COMPATIBILITY_VSM_CAPABILITIES_MASK_REDSTONE_4 )

//
// Mask of VSM capabilities supported in Experimental VMs.
//
#define HV_COMPATIBILITY_VSM_CAPABILITIES_MASK_EXPERIMENT \
    ( HV_COMPATIBILITY_VSM_CAPABILITIES_MASK_PRERELEASE )

//
// Definition of Partition Compatibility Version values
//

#define HV_MAKE_COMPATIBILITY_VERSION(major_, minor_) \
    ((UINT32)((major_) << 8 | (minor_)))
#define HV_COMPATIBILITY_VERSION_MAJOR(version_)      \
    (((UINT32)(version_) >> 8) & 0x000000FF)
#define HV_COMPATIBILITY_VERSION_MINOR(version_)      \
    ((UINT32)(version_) & 0x000000FF)

typedef enum _HV_COMPATIBILITY_VERSION
{
    HvCompatibilityVersionWinBlue              = HV_MAKE_COMPATIBILITY_VERSION(0x4, 0x0),
    HvCompatibilityVersionWinThreshold         = HV_MAKE_COMPATIBILITY_VERSION(0x5, 0x0),
    HvCompatibilityVersionWinTh2               = HV_MAKE_COMPATIBILITY_VERSION(0x5, 0x1),
    HvCompatibilityVersionWinServer2016TP5     = HV_MAKE_COMPATIBILITY_VERSION(0x5, 0x2),
    HvCompatibilityVersionRedstone1            = HV_MAKE_COMPATIBILITY_VERSION(0x6, 0x0),
    HvCompatibilityVersionRedstone2            = HV_MAKE_COMPATIBILITY_VERSION(0x6, 0x1),
    HvCompatibilityVersionRedstone3            = HV_MAKE_COMPATIBILITY_VERSION(0x6, 0x2),
    HvCompatibilityVersionRedstone4            = HV_MAKE_COMPATIBILITY_VERSION(0x6, 0x3),
    HvCompatibilityVersionRedstone5            = HV_MAKE_COMPATIBILITY_VERSION(0x6, 0x4),
    HvCompatibilityVersionPrerelease           = HV_MAKE_COMPATIBILITY_VERSION(0xFE, 0x0),
    HvCompatibilityVersionExperiment           = HV_MAKE_COMPATIBILITY_VERSION(0xFF, 0x0),
} HV_COMPATIBILITY_VERSION, *PHV_COMPATIBILITY_VERSION;

//
// For every release, we need to decide what are the min and current version we support
// in the HV.
//
#define HV_COMPATIBILITY_VERSION_MIN                    HvCompatibilityVersionWinBlue
#define HV_COMPATIBILITY_VERSION_CURRENT                HvCompatibilityVersionRedstone4
#define HV_COMPATIBILITY_VERSION_MAX                    HvCompatibilityVersionExperiment

#define HV_COMPATIBILITY_PRIVILEGE_MASK_WINBLUE \
    ( HV_PARTITION_PRIVILEGE_ACCESS_VP_RUNTIME_MSR \
    | HV_PARTITION_PRIVILEGE_PARTITION_REFERENCE_COUNTER \
    | HV_PARTITION_PRIVILEGE_SYNIC_MSRS \
    | HV_PARTITION_PRIVILEGE_ACCESS_SYNTHETIC_TIMER_MSRS \
    | HV_PARTITION_PRIVILEGE_ACCESS_APIC_MSRS \
    | HV_PARTITION_PRIVILEGE_ACCESS_HYPERCALL_MSRS \
    | HV_PARTITION_PRIVILEGE_ACCESS_VP_INDEX \
    | HV_PARTITION_PRIVILEGE_ACCESS_RESET_MSR \
    | HV_PARTITION_PRIVILEGE_ACCESS_STATS_MSR \
    | HV_PARTITION_PRIVILEGE_ACCESS_PARTITION_REFERENCE_TSC \
    | HV_PARTITION_PRIVILEGE_ACCESS_GUEST_IDLE_MSR \
    | HV_PARTITION_PRIVILEGE_ACCESS_FREQUENCY_MSRS \
    | HV_PARTITION_PRIVILEGE_ACCESS_DEBUG_MSRS \
    | HV_PARTITION_PRIVILEGE_CREATE_PARTITIONS \
    | HV_PARTITION_PRIVILEGE_ACCESS_PARTITION_ID \
    | HV_PARTITION_PRIVILEGE_ACCESS_MEMORY_POOL \
    | HV_PARTITION_PRIVILEGE_ADJUST_MESSAGE_BUFFERS \
    | HV_PARTITION_PRIVILEGE_POST_MESSAGES \
    | HV_PARTITION_PRIVILEGE_SIGNAL_EVENTS \
    | HV_PARTITION_PRIVILEGE_CREATE_PORT \
    | HV_PARTITION_PRIVILEGE_CONNECT_PORT \
    | HV_PARTITION_PRIVILEGE_ACCESS_STATS \
    | HV_PARTITION_PRIVILEGE_DEBUGGING \
    | HV_PARTITION_PRIVILEGE_CPU_MANAGEMENT \
    | HV_PARTITION_PRIVILEGE_CONFIGURE_PROFILER )

#define HV_COMPATIBILITY_PRIVILEGE_MASK_WINTHRESHOLD \
    ( HV_COMPATIBILITY_PRIVILEGE_MASK_WINBLUE \
    | HV_PARTITION_PRIVILEGE_ACCESS_VP_EXIT_TRACING \
    | HV_PARTITION_PRIVILEGE_ENABLE_EXTENDED_GVA_RANGES_FLUSH_VA_LIST \
    | HV_PARTITION_PRIVILEGE_FAST_HYPERCALL_OUTPUT \
    | HV_PARTITION_PRIVILEGE_ENABLE_EXTENDED_HYPERCALLS \
    | HV_PARTITION_PRIVILEGE_START_VIRTUAL_PROCESSOR )

//
// Th2 mask.
//

#define HV_COMPATIBILITY_PRIVILEGE_MASK_WINTH2 \
    ( HV_COMPATIBILITY_PRIVILEGE_MASK_WINTHRESHOLD )

//
// Server 2016 TP5 mask.
//

#define HV_COMPATIBILITY_PRIVILEGE_MASK_SERVER2016_TP5 \
    ( HV_COMPATIBILITY_PRIVILEGE_MASK_WINTH2 )

//
// Redstone 1 mask.
//

#define HV_COMPATIBILITY_PRIVILEGE_MASK_REDSTONE_1 \
    ( HV_COMPATIBILITY_PRIVILEGE_MASK_SERVER2016_TP5 \
    | HV_PARTITION_PRIVILEGE_ACCESS_REENLIGHTENMENT_CTRLS \
    | HV_PARTITION_PRIVILEGE_ACCESS_VSM \
    | HV_PARTITION_PRIVILEGE_ACCESS_VP_REGISTERS )

//
// Redstone 2 mask.
//

#define HV_COMPATIBILITY_PRIVILEGE_MASK_REDSTONE_2 \
    ( HV_COMPATIBILITY_PRIVILEGE_MASK_REDSTONE_1 )

//
// Redstone 3 mask.
//

#define HV_COMPATIBILITY_PRIVILEGE_MASK_REDSTONE_3 \
    ( HV_COMPATIBILITY_PRIVILEGE_MASK_REDSTONE_2 \
    | HV_PARTITION_PRIVILEGE_ACCESS_ROOT_SCHEDULER_MSR)

//
// Redstone 4 mask.
//

#define HV_COMPATIBILITY_PRIVILEGE_MASK_REDSTONE_4 \
    ( HV_COMPATIBILITY_PRIVILEGE_MASK_REDSTONE_3 )

//
// Redstone 5 mask.
//

#define HV_COMPATIBILITY_PRIVILEGE_MASK_REDSTONE_5 \
    ( HV_COMPATIBILITY_PRIVILEGE_MASK_REDSTONE_4 )

//
// Prerelease mask.
//

#define HV_COMPATIBILITY_PRIVILEGE_MASK_PRERELEASE \
    ( HV_COMPATIBILITY_PRIVILEGE_MASK_REDSTONE_4 )

//
// Experiment version should always encompass all existing
// privileges
//

#define HV_COMPATIBILITY_PRIVILEGE_MASK_EXPERIMENT \
    ( HV_COMPATIBILITY_PRIVILEGE_MASK_PRERELEASE \
    | HV_PARTITION_PRIVILEGE_ISOLATION)

//
// Define the nested virtualization features that may be exposed to a
// partition.
//

typedef union _HV_PARTITION_PROCESSOR_VIRTUALIZATION_FEATURES
{
    struct
    {
        UINT64 HideHypervisorPresent:1;
        UINT64 RsvdZ:63;
    };

    UINT64 AsUINT64;

} HV_PARTITION_PROCESSOR_VIRTUALIZATION_FEATURES, *PHV_PARTITION_PROCESSOR_VIRTUALIZATION_FEATURES;

//
// Define the structure providing hints to the hypervisor indicating
// which enlightenments should be favored over competing hardware assists.
// No default values are provided for compatibility mode since enlightenments
// are supported on all hardware platforms.
//

typedef union _HV_PARTITION_ENLIGHTENMENT_MODIFICATIONS
{
    struct
    {
        UINT64 FavorAutoEoi:1;
        UINT64 RsvdZ1:63;
    };
    UINT64 AsUINT64;

} HV_PARTITION_ENLIGHTENMENT_MODIFICATIONS, *PHV_PARTITION_ENLIGHTENMENT_MODIFICATIONS;


//
// Define the partition VTL config
//

typedef union _HV_PARTITION_VSM_CONFIG
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 MaximumVtl  : 4;
        UINT64 ReservedZ   : 60;
    };
} HV_PARTITION_VSM_CONFIG, *PHV_PARTITION_VSM_CONFIG;

#if defined(_AMD64_)

//
// Define the partition page access tracking state (see
// HvPartitionPropertyPageAccessTracking).
//

typedef union _HV_PARTITION_PAGE_ACCESS_TRACKING_CONFIG
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 Enabled : 1;
        UINT64 ReservedZ : 63;
    };

} HV_PARTITION_PAGE_ACCESS_TRACKING_CONFIG, *PHV_PARTITION_PAGE_ACCESS_TRACKING_CONFIG;

#endif

//
// Definition of the partition isolation state. Used for
// HvPartitionPropertyIsolationState.
//

typedef enum _HV_PARTITION_ISOLATION_STATE
{
    HvPartitionIsolationInvalid         = 0,
    HvPartitionIsolationInsecureClean   = 1,
    HvPartitionIsolationInsecureDirty   = 2,
    HvPartitionIsolationSecure          = 3,
} HV_PARTITION_ISOLATION_STATE, *PHV_PARTITION_ISOLATION_STATE;

//
// Definition of the partition isolation control structure. Used for
// HvPartitionPropertyIsolationControl.
//

typedef union _HV_PARTITION_ISOLATION_CONTROL
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 Runnable : 1;

        UINT64 ReservedZ : 63;
    };

} HV_PARTITION_ISOLATION_CONTROL, *PHV_PARTITION_ISOLATION_CONTROL;


//
// Declare the VP run time MSR.
//
#define HV_X64_MSR_VP_RUNTIME   (HvSyntheticMsrVpRuntime)

#define HV_CPUID_HV_VENDOR_MICROSOFT_EBX 'rciM'
#define HV_CPUID_HV_VENDOR_MICROSOFT_ECX 'foso'
#define HV_CPUID_HV_VENDOR_MICROSOFT_EDX 'vH t'

//
// Synthetic machine check definitions.
//

typedef union _HV_X64_MSR_SYNMC_STATUS_CONTENTS
{
    struct
    {
        UINT16 McaErrorCode;

        union
        {
            UINT16 ModelSpecificErrorCode;

            struct
            {
                UINT16 ErrorDetail      : 14;
                UINT16 HypervisorError  : 1;
                UINT16 SoftwareError    : 1;
            };
        };

        struct
        {
            UINT32 Reserved             : 23;
            UINT32 ActionRequired       : 1;
            UINT32 Signaling            : 1;
            UINT32 ContextCorrupt       : 1;  // Hypervisor/virt stack context corrupt
            UINT32 AddressValid         : 1;
            UINT32 MiscValid            : 1;
            UINT32 ErrorEnabled         : 1;
            UINT32 Uncorrected          : 1;  // Uncorrected error
            UINT32 Overflow             : 1;  // Error overflow
            UINT32 Valid                : 1;  // Register valid
        };
    };

    UINT64 AsUINT64;

} HV_X64_MSR_SYNMC_STATUS_CONTENTS, *PHV_X64_MSR_SYNMC_STATUS_CONTENTS;

#define HV_SYNMC_MCA_ERROR_CODE (0x0001)        // Unclassified error

typedef UINT64 HV_X64_MSR_SYNMC_ADDR_CONTENTS, PHV_X64_MSR_SYNMC_ADDR_CONTENTS;

typedef UINT64 HV_X64_MSR_SYNMC_MISC_CONTENTS, PHV_X64_MSR_SYNMC_MISC_CONTENTS;

typedef struct _HV_SYNMC_EVENT
{
    HV_X64_MSR_SYNMC_STATUS_CONTENTS Status;
    HV_X64_MSR_SYNMC_ADDR_CONTENTS   Addr;
    HV_X64_MSR_SYNMC_MISC_CONTENTS   Misc;
    BOOLEAN                          RipValid;
    BOOLEAN                          EipValid;

} HV_SYNMC_EVENT, *PHV_SYNMC_EVENT;


//
// Maximum physical address width for child partitions.
//
#define HV_MAX_CHILD_PHYSICAL_ADDRESS_WIDTH 44


//
// Address translation flags.
//

#define HV_TRANSLATE_GVA_VALIDATE_READ       (0x0001)
#define HV_TRANSLATE_GVA_VALIDATE_WRITE      (0x0002)
#define HV_TRANSLATE_GVA_VALIDATE_EXECUTE    (0x0004)
#define HV_TRANSLATE_GVA_PRIVILEGE_EXEMPT    (0x0008)
#define HV_TRANSLATE_GVA_SET_PAGE_TABLE_BITS (0x0010)
#define HV_TRANSLATE_GVA_TLB_FLUSH_INHIBIT   (0x0020)
#define HV_TRANSLATE_GVA_CONTROL_MASK        (0x003F)
#define HV_TRANSLATE_GVA_INPUT_VTL_MASK      (0xFF00000000000000UI64)

typedef UINT64 HV_TRANSLATE_GVA_CONTROL_FLAGS, *PHV_TRANSLATE_GVA_CONTROL_FLAGS;

typedef enum _HV_TRANSLATE_GVA_RESULT_CODE
{
    HvTranslateGvaSuccess                 = 0,

    // Translation Failures
    HvTranslateGvaPageNotPresent          = 1,
    HvTranslateGvaPrivilegeViolation      = 2,
    HvTranslateGvaInvalidPageTableFlags   = 3,

    // GPA access failures
    HvTranslateGvaGpaUnmapped             = 4,
    HvTranslateGvaGpaNoReadAccess         = 5,
    HvTranslateGvaGpaNoWriteAccess        = 6,
    HvTranslateGvaGpaIllegalOverlayAccess = 7,

    //
    // Intercept of the memory access by either
    // - a higher VTL
    // - a nested hypervisor (due to a violation of the nested page table)
    //
    HvTranslateGvaIntercept               = 8,

    HvTranslateGvaGpaUnaccepted           = 9,

} HV_TRANSLATE_GVA_RESULT_CODE, *PHV_TRANSLATE_GVA_RESULT_CODE;

typedef union _HV_TRANSLATE_GVA_RESULT
{
    UINT64 AsUINT64;

    struct
    {
        HV_TRANSLATE_GVA_RESULT_CODE ResultCode;
        UINT32 CacheType : 8;
        UINT32 OverlayPage : 1;
        UINT32 Reserved : 23;
    };

} HV_TRANSLATE_GVA_RESULT, *PHV_TRANSLATE_GVA_RESULT;

typedef struct _HV_TRANSLATE_GVA_RESULT_EX
{
    HV_TRANSLATE_GVA_RESULT_CODE ResultCode;
    UINT32 CacheType : 8;
    UINT32 OverlayPage : 1;
    UINT32 Reserved : 23;

#if !defined(_ARM64_)
    HV_X64_PENDING_EVENT EventInfo;
#endif

} HV_TRANSLATE_GVA_RESULT_EX, *PHV_TRANSLATE_GVA_RESULT_EX;

//
// Read and write GPA access flags.
//

typedef union _HV_ACCESS_GPA_CONTROL_FLAGS
{
    UINT64 AsUINT64;
    struct
    {
        //
        // Cache type for access
        //
        UINT8 CacheType;

        //
        // VTL whose GPA is to be accessed
        //
        HV_INPUT_VTL InputVtl;

        UINT16 ReservedZ0;
        UINT32 ReservedZ1;
    };
} HV_ACCESS_GPA_CONTROL_FLAGS, *PHV_ACCESS_GPA_CONTROL_FLAGS;

typedef enum _HV_ACCESS_GPA_RESULT_CODE
{
    HvAccessGpaSuccess              = 0,

    // GPA access failures
    HvAccessGpaUnmapped             = 1,
    HvAccessGpaReadIntercept        = 2,
    HvAccessGpaWriteIntercept       = 3,
    HvAccessGpaIllegalOverlayAccess = 4

} HV_ACCESS_GPA_RESULT_CODE, *PHV_ACCESS_GPA_RESULT_CODE;

typedef union _HV_ACCESS_GPA_RESULT
{
    UINT64 AsUINT64;
    struct
    {
        HV_ACCESS_GPA_RESULT_CODE ResultCode;
        UINT32                    Reserved;
    };
} HV_ACCESS_GPA_RESULT, *PHV_ACCESS_GPA_RESULT;

//
// Cache types.
//
typedef enum _HV_CACHE_TYPE
{
    HvCacheTypeX64Uncached       = 0,
    HvCacheTypeX64WriteCombining = 1,
    HvCacheTypeX64WriteThrough   = 4,
    HvCacheTypeX64WriteProtected = 5,
    HvCacheTypeX64WriteBack      = 6
} HV_CACHE_TYPE, *PHV_CACHE_TYPE;

//
// Recommended number of pages to specify in a single HvUnmapGpaPages hypercall
// when unmapping a large range of pages.
//
#define HV_UNMAP_GPA_RECOMMENDED_PAGE_COUNT     512

//
// Recommended page alignment of the GPA page range to specify in
// HvUnmapGpaPages hypercalls when unmapping a large range of pages.
//
#define HV_UNMAP_GPA_RECOMMENDED_PAGE_ALIGNMENT 512

//
// Maximum number of pages that can be specified in a single
// HvModifySparseGpaPages hypercall.
//
#define HV_MODIFY_SPARSE_GPA_MAX_PAGE_COUNT     \
    ((HV_PAGE_SIZE - sizeof(HV_INPUT_MODIFY_SPARSE_GPA_PAGES)) / \
     sizeof(HV_GPA_PAGE_NUMBER))

//
// Maximum number of pages that can be specified in a single
// HvModifyVtlProtectionMask hypercall.
//
#define HV_MODIFY_VTL_PROTECTIONS_GPA_MAX_PAGE_COUNT     \
    ((HV_PAGE_SIZE - sizeof(HV_INPUT_MODIFY_VTL_PROTECTION_MASK)) / \
     sizeof(HV_GPA_PAGE_NUMBER))

//
// Definitions for the HvCallQueryVtlProtectionMaskRange hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_QUERY_VTL_PROTECTION_MASK_RANGE
{
    //
    // Supplies the partition ID of the partition this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies the base guest physical page number for the query.
    //

    HV_GPA_PAGE_NUMBER TargetGpaBase;

    //
    // Supplies the target set of VTLs for the query.
    //

    UINT16 TargetVtlSet;

    UINT16 Reserved0;
    UINT32 Reserved1;

} HV_INPUT_QUERY_VTL_PROTECTION_MASK_RANGE, *PHV_INPUT_QUERY_VTL_PROTECTION_MASK_RANGE;

//
// Definitions for the HvCallModifyVtlProtectionMaskRange hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_MODIFY_VTL_PROTECTION_MASK_RANGE
{
    //
    // Supplies the partition ID of the partition this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies the base guest physical page number for the modifications.
    //

    HV_GPA_PAGE_NUMBER TargetGpaBase;

    //
    // Supplies the target set of VTLs for the modifications.
    //

    UINT16 TargetVtlSet;

    UINT16 Reserved0;
    UINT32 Reserved1;

    //
    // Supplies an array of VTL permissions to set.
    //

    HV_CALL_ATTRIBUTES HV_VTL_PERMISSION_SET VtlPermissionList[];

} HV_INPUT_MODIFY_VTL_PROTECTION_MASK_RANGE, *PHV_INPUT_MODIFY_VTL_PROTECTION_MASK_RANGE;

//
// Definitions for the HvCallAcquireSparseGpaPageHostAccess and HvCallReleaseSparseGpaPageHostAccess
// hypercalls.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_MODIFY_SPARSE_GPA_PAGE_HOST_ACCESS
{
    //
    // Supplies the partition ID of the partition this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies the new host access mask.
    //

    UINT32 HostAccess : 2;

    UINT32 Reserved0 : 30;
    UINT32 Reserved1;

    //
    // Supplies an array of GPA page numbers to modify.
    //

    HV_CALL_ATTRIBUTES HV_GPA_PAGE_NUMBER GpaPageList[];

} HV_INPUT_MODIFY_SPARSE_GPA_PAGE_HOST_ACCESS, *PHV_INPUT_MODIFY_SPARSE_GPA_PAGE_HOST_ACCESS;

//
// Definitions for the HvCallAcquireSparseSpaPageHostAccess and HvCallReleaseSparseSpaPageHostAccess
// hypercalls.
//

typedef UINT32 HV_MODIFY_SPA_PAGE_HOST_ACCESS_FLAGS, *PHV_MODIFY_SPA_PAGE_HOST_ACCESS_FLAGS;

#define HV_MODIFY_SPA_PAGE_HOST_ACCESS_MAKE_EXCLUSIVE   0x1
#define HV_MODIFY_SPA_PAGE_HOST_ACCESS_MAKE_SHARED      0x2

#define HV_MODIFY_SPA_PAGE_HOST_ACCESS_FLAGS_MASK       \
    ( HV_MODIFY_SPA_PAGE_HOST_ACCESS_MAKE_EXCLUSIVE     \
    | HV_MODIFY_SPA_PAGE_HOST_ACCESS_MAKE_SHARED        \
    )

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_MODIFY_SPARSE_SPA_PAGE_HOST_ACCESS
{
    //
    // Supplies the new host access mask.
    //

    UINT32 HostAccess : 2;

    UINT32 Reserved : 30;

    //
    // Various flags.
    //

    HV_MODIFY_SPA_PAGE_HOST_ACCESS_FLAGS Flags;

    //
    // Supplies an array of SPA page numbers to modify.
    //

    HV_CALL_ATTRIBUTES HV_SPA_PAGE_NUMBER SpaPageList[];

} HV_INPUT_MODIFY_SPARSE_SPA_PAGE_HOST_ACCESS, *PHV_INPUT_MODIFY_SPARSE_SPA_PAGE_HOST_ACCESS;

//
// Maximum number of pages that can be specified in a single HvCallAcquireSparseSpaPageHostAccess or
// HvCallReleaseSparseSpaPageHostAccess hypercall.
//
#define HV_MODIFY_SPARSE_SPA_PAGE_HOST_ACCESS_MAX_PAGE_COUNT \
    ((HV_PAGE_SIZE - sizeof(HV_INPUT_MODIFY_SPARSE_SPA_PAGE_HOST_ACCESS)) / \
     sizeof(HV_SPA_PAGE_NUMBER))

//
// Definitions for the HvCallLockSparseGpaPageMapping and HvCallUnlockSparseGpaPageMapping
// hypercalls.
//

#define HV_GPA_PAGE_ACCEPTANCE_UNACCEPTED HV_MAP_GPA_WRITABLE

typedef enum _HV_SPA_PAGE_OWNERSHIP
{
    HvSpaPageOwnershipShared = 0,
    HvSpaPageOwnershipExclusive = 1,

} HV_SPA_PAGE_OWNERSHIP, *PHV_SPA_PAGE_OWNERSHIP;

typedef struct _HV_GPA_PAGE_MAPPING_DATA
{
    //
    // Specifies the host visibility (exclusive, shared read-only, shared read-write) if the page is
    // accepted, or HV_GPA_PAGE_ACCEPTANCE_UNACCEPTED if the page is unaccepted.
    //

    UINT64 Acceptance : 2;

    //
    // Specifies the VTL 1 permission mask.
    //

    UINT64 Vtl1Permission : 5;

    //
    // Specifies the VTL 2 permission mask.
    //

    UINT64 Vtl2Permission : 5;

    //
    // Specifies the dirty bit.
    //

    UINT64 Dirty : 1;

} HV_GPA_PAGE_MAPPING_DATA, *PHV_GPA_PAGE_MAPPING_DATA;

typedef union _HV_GPA_PAGE_MAPPING_LOCK_INPUT
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 Reserved0 : 13;

        //
        // Specifies the GPA page to lock.
        //

        UINT64 GpaPage : 40;

        //
        // If specified, the associated SPA page is made confidential (i.e. the maximum host access
        // is set to zero). The SPA page must be exclusive and the GPA page must be currently
        // unaccepted.
        //

        UINT64 Confidential : 1;

        UINT64 Reserved1 : 10;
    };

} HV_GPA_PAGE_MAPPING_LOCK_INPUT, *PHV_GPA_PAGE_MAPPING_LOCK_INPUT;

typedef union _HV_GPA_PAGE_MAPPING_LOCK_OUTPUT
{
    UINT64 AsUINT64;

    HV_GPA_PAGE_MAPPING_DATA Data;

    struct
    {
        UINT64 : 13;

        //
        // Specifies the SPA page that the GPA page maps to.
        //

        UINT64 SpaPage : 40;

        //
        // Specifies the ownership type of the SPA page (HV_SPA_PAGE_OWNERSHIP).
        //

        UINT64 SpaPageOwnership : 1;

        //
        // Specifies the memory type of the SPA page (HV_ACCEPT_MEMORY_TYPE).
        //

        UINT64 SpaPageMemoryType : 4;

        //
        // Specifies the read permission bit configured by the root.
        //

        UINT64 RootReadable : 1;

        //
        // Specifies the write permission bit configured by the root.
        //

        UINT64 RootWritable : 1;

        //
        // Specifies the adjust permission bit configured by the root.
        //

        UINT64 RootAdjustable : 1;

        UINT64 Reserved : 3;
    };

} HV_GPA_PAGE_MAPPING_LOCK_OUTPUT, *PHV_GPA_PAGE_MAPPING_LOCK_OUTPUT;

typedef union _HV_GPA_PAGE_MAPPING_UNLOCK_INPUT
{
    UINT64 AsUINT64;

    HV_GPA_PAGE_MAPPING_DATA Data;

    struct
    {
        UINT64 : 13;

        //
        // Specifies the GPA page to unlock.
        //

        UINT64 GpaPage : 40;

        UINT64 Reserved : 11;
    };

} HV_GPA_PAGE_MAPPING_UNLOCK_INPUT, *PHV_GPA_PAGE_MAPPING_UNLOCK_INPUT;

typedef UINT32 HV_LOCK_GPA_PAGE_MAPPING_FLAGS, *PHV_LOCK_GPA_PAGE_MAPPING_FLAGS;

#define HV_LOCK_GPA_PAGE_MAPPING_FLAGS_MASK     \
    ( 0                                         \
    )

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_LOCK_SPARSE_GPA_PAGE_MAPPING
{
    //
    // Supplies the partition ID of the partition this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Various flags.
    //

    HV_LOCK_GPA_PAGE_MAPPING_FLAGS Flags;

    UINT32 Reserved;

    //
    // Supplies an array of GPA page numbers to modify.
    //

    HV_CALL_ATTRIBUTES HV_GPA_PAGE_MAPPING_LOCK_INPUT LockList[];

} HV_INPUT_LOCK_SPARSE_GPA_PAGE_MAPPING, *PHV_INPUT_LOCK_SPARSE_GPA_PAGE_MAPPING;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_UNLOCK_SPARSE_GPA_PAGE_MAPPING
{
    //
    // Supplies the partition ID of the partition this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies an array of GPA page numbers to modify.
    //

    HV_CALL_ATTRIBUTES HV_GPA_PAGE_MAPPING_UNLOCK_INPUT UnlockList[];

} HV_INPUT_UNLOCK_SPARSE_GPA_PAGE_MAPPING, *PHV_INPUT_UNLOCK_SPARSE_GPA_PAGE_MAPPING;


//
// Define synthetic interrupt controller flag constants.
//

#define HV_EVENT_FLAGS_COUNT        (256 * 8)
#define HV_EVENT_FLAGS_BYTE_COUNT   (256)
#define HV_EVENT_FLAGS_DWORD_COUNT  (256 / sizeof(UINT32))

//
// Define lowest permissible vector that can be sent or received by the local
// APIC.
//

#if defined(_ARM64_)

//
// N.B. On ARM64, we can start with vector 0 as we don't face the mix
//      of processor interrupts and general interrupts using the same
//      dispatcher table here.
//

#define HV_SYNIC_APIC_MINIMUM_VECTOR    0x0

#else

#define HV_SYNIC_APIC_MINIMUM_VECTOR    0x10

#endif

#define HV_MESSAGE_TYPE_HYPERVISOR_MASK (0x80000000)

#if defined(_AMD64_) || defined(_X86_)

//
// Define APIC EOI message.
//
typedef struct _HV_X64_APIC_EOI_MESSAGE
{
    UINT32 VpIndex;
    UINT32 InterruptVector;
} HV_X64_APIC_EOI_MESSAGE, *PHV_X64_APIC_EOI_MESSAGE;

#endif

//
// Define intercept event message.
//
typedef struct _HV_SYNIC_EVENT_INTERCEPT_MESSAGE
{
    UINT32 VpIndex;
    UINT32 Vtl      :  4;
    UINT32 Reserved : 28;
    HV_CONNECTION_ID ConnectionId;
    UINT32 FlagNumber;
} HV_SYNIC_EVENT_INTERCEPT_MESSAGE, *PHV_SYNIC_EVENT_INTERCEPT_MESSAGE;

//
// Define partition identifier type.
//

typedef UINT64 HV_PARTITION_ID, *PHV_PARTITION_ID;

//
// Define port type.
//

typedef enum _HV_PORT_TYPE
{
    HvPortTypeMessage   = 1,
    HvPortTypeEvent     = 2,
    HvPortTypeMonitor   = 3
} HV_PORT_TYPE, *PHV_PORT_TYPE;

//
// Define port information structure.
//

typedef struct _HV_PORT_INFO
{
    HV_PORT_TYPE PortType;
    UINT32 Padding;

    union
    {
        struct
        {
            HV_SYNIC_SINT_INDEX TargetSint;
            HV_VP_INDEX TargetVp;
            UINT64 RsvdZ;
        } MessagePortInfo;

        struct
        {
            HV_SYNIC_SINT_INDEX TargetSint;
            HV_VP_INDEX TargetVp;
            UINT16 BaseFlagNumber;
            UINT16 FlagCount;
            UINT32 RsvdZ;
        } EventPortInfo;

        struct
        {
            HV_GPA MonitorAddress;
            UINT64 RsvdZ;
        } MonitorPortInfo;
    };
} HV_PORT_INFO, *PHV_PORT_INFO;

typedef const HV_PORT_INFO *PCHV_PORT_INFO;

typedef struct _HV_CONNECTION_INFO
{
    HV_PORT_TYPE PortType;
    UINT32 Padding;

    union
    {
        struct
        {
            UINT64 RsvdZ;
        } MessageConnectionInfo;

        struct
        {
            UINT64 RsvdZ;
        } EventConnectionInfo;

        struct
        {
            HV_GPA MonitorAddress;
        } MonitorConnectionInfo;
    };
} HV_CONNECTION_INFO, *PHV_CONNECTION_INFO;

typedef const HV_CONNECTION_INFO *PCHV_CONNECTION_INFO;

//
// Define type of port property.
//

typedef UINT64 HV_PORT_PROPERTY, *PHV_PORT_PROPERTY;

//
// Define enumeration of port property codes.
//

typedef enum _HV_PORT_PROPERTY_CODE
{
    HvPortPropertyPostCount = 0x00000000,
    HvPortPropertyPreferredTargetVp = 0x00000001,
    HvPortPropertyMonitorAddress = 0x00000002,
    HvPortPropertyUseRingBuffer = 0x00000003,
    HvPortPropertyTargetVp = 0x00000004,
    HvPortPropertyPreferredTargetDuration = 0x00000005,
    HvPortPropertyRetryPostMessage = 0x00000006,
    HvPortPropertyInterceptOnSignal = 0x00000007,
} HV_PORT_PROPERTY_CODE, *PHV_PORT_PROPERTY_CODE;

//
// Define the maximum possible value for HvPortPropertyPreferredTargetDuration.
//

#define HV_SYNIC_PORT_PREFERRED_DURATION_MAX (0xFFFFFFFFFFFFFFFFUI64)

//
// Define the number of message buffers associated with each port.
//

#define HV_PORT_MESSAGE_BUFFER_COUNT (16)

//
// Define the synthetic interrupt controller event flags format.
//

typedef union _HV_SYNIC_EVENT_FLAGS
{
    UINT8 Flags8[HV_EVENT_FLAGS_BYTE_COUNT];
    UINT32 Flags32[HV_EVENT_FLAGS_DWORD_COUNT];
} HV_SYNIC_EVENT_FLAGS, *PHV_SYNIC_EVENT_FLAGS;

//
// Define the synthetic interrupt flags page layout.
//

typedef struct _HV_SYNIC_EVENT_FLAGS_PAGE
{
    volatile HV_SYNIC_EVENT_FLAGS SintEventFlags[HV_SYNIC_SINT_COUNT];
} HV_SYNIC_EVENT_FLAGS_PAGE, *PHV_SYNIC_EVENT_FLAGS_PAGE;

//
// Define the synthetic interrupt controller event ring format.
//

#define HV_SYNIC_EVENT_RING_MESSAGE_COUNT 63

typedef struct _HV_SYNIC_EVENT_RING
{
    UINT8 SignalMasked;
    UINT8 RingFull;
    UINT16 ReservedZ;
    UINT32 Data[HV_SYNIC_EVENT_RING_MESSAGE_COUNT];
} HV_SYNIC_EVENT_RING, *PHV_SYNIC_EVENT_RING;

//
// Define the synthetic interrupt ring buffer page.
//

typedef struct _HV_SYNIC_EVENT_RING_PAGE
{
    volatile HV_SYNIC_EVENT_RING SintEventRing[HV_SYNIC_SINT_COUNT];
} HV_SYNIC_EVENT_RING_PAGE, *PHV_SYNIC_EVENT_RING_PAGE;

//
// Define SynIC control register.
//
typedef union _HV_SYNIC_SCONTROL
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Enable:1;
        UINT64 ReservedP:63;
    };
} HV_SYNIC_SCONTROL, *PHV_SYNIC_SCONTROL;

//
// Define the format of the SIEFP register
//

typedef union _HV_SYNIC_SIEFP
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 SiefpEnabled : 1;
        UINT64 ReservedP    : 11;
        UINT64 BaseSiefpGpa : 52;
    };
} HV_SYNIC_SIEFP, *PHV_SYNIC_SIEFP;

//
// Define the format of the SIRBP register
//

typedef union _HV_SYNIC_SIRBP
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 SirbpEnabled : 1;
        UINT64 ReservedP    : 11;
        UINT64 BaseSirbpGpa : 52;
    };
} HV_SYNIC_SIRBP, *PHV_SYNIC_SIRBP;

// ARM64HV_TODO: Can we get rid of the concept of HV_INTERRUPT_CONTROL entirely from ARM64?

//
// Define virtual interrupt control structure.
//
typedef union _HV_INTERRUPT_CONTROL
{
    UINT64 AsUINT64;
    struct
    {
        HV_INTERRUPT_TYPE InterruptType;
        UINT32 LevelTriggered:1;
        UINT32 LogicalDestinationMode:1;
        UINT32 Reserved:30;
    };
} HV_INTERRUPT_CONTROL, *PHV_INTERRUPT_CONTROL;

//
// Definitions for the monitored notification facility
//

typedef union _HV_MONITOR_TRIGGER_GROUP
{
    UINT64 AsUINT64;

    struct
    {
        UINT32 Pending;
        UINT32 Armed;
    };

} HV_MONITOR_TRIGGER_GROUP, *PHV_MONITOR_TRIGGER_GROUP;

typedef struct _HV_MONITOR_PARAMETER
{
    HV_CONNECTION_ID    ConnectionId;
    UINT16              FlagNumber;
    UINT16              RsvdZ;
} HV_MONITOR_PARAMETER, *PHV_MONITOR_PARAMETER;

typedef union _HV_MONITOR_TRIGGER_STATE
{
    UINT32 AsUINT32;

    struct
    {
        UINT32 GroupEnable : 4;
        UINT32 RsvdZ       : 28;
    };

} HV_MONITOR_TRIGGER_STATE, *PHV_MONITOR_TRIGGER_STATE;

//
// HV_MONITOR_PAGE Layout
// ------------------------------------------------------
// | 0   | TriggerState (4 bytes) | Rsvd1 (4 bytes)     |
// | 8   | TriggerGroup[0]                              |
// | 10  | TriggerGroup[1]                              |
// | 18  | TriggerGroup[2]                              |
// | 20  | TriggerGroup[3]                              |
// | 28  | Rsvd2[0]                                     |
// | 30  | Rsvd2[1]                                     |
// | 38  | Rsvd2[2]                                     |
// | 40  | NextCheckTime[0][0]    | NextCheckTime[0][1] |
// | ...                                                |
// | 240 | Latency[0][0..3]                             |
// | 340 | Rsvz3[0]                                     |
// | 440 | Parameter[0][0]                              |
// | 448 | Parameter[0][1]                              |
// | ...                                                |
// | 840 | Rsvd4[0]                                     |
// ------------------------------------------------------

typedef struct _HV_MONITOR_PAGE
{
    HV_MONITOR_TRIGGER_STATE TriggerState;
    UINT32                   RsvdZ1;

    HV_MONITOR_TRIGGER_GROUP TriggerGroup[4];
    UINT64                   RsvdZ2[3];

    INT32                    NextCheckTime[4][32];

    UINT16                   Latency[4][32];
    UINT64                   RsvdZ3[32];

    HV_MONITOR_PARAMETER     Parameter[4][32];

    UINT8                    RsvdZ4[1984];

} HV_MONITOR_PAGE, *PHV_MONITOR_PAGE;

typedef volatile HV_MONITOR_PAGE* PVHV_MONITOR_PAGE;

//
// ARM64 SPI configuration and assertion call inputs.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_CONFIGURE_VIRTUAL_INTERRUPT
{
    HV_PARTITION_ID TargetPartition;
    HV_INTERRUPT_VECTOR InterruptLine;

} HV_INPUT_CONFIGURE_VIRTUAL_INTERRUPT, *PHV_INPUT_CONFIGURE_VIRTUAL_INTERRUPT;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SET_VIRTUAL_INTERRUPT_LINE_STATE
{
    HV_PARTITION_ID TargetPartition;
    HV_INTERRUPT_VECTOR InterruptLine;
    BOOLEAN Asserted;

} HV_INPUT_SET_VIRTUAL_INTERRUPT_LINE_STATE, *PHV_INPUT_SET_VIRTUAL_INTERRUPT_LINE_STATE;

//
// Define the GIC addresses for ARM64. These are implementation defined and exposed
// through ACPI tables to a child partition. For simplicity, we statically specify
// these here, rather than have the virtualization stack specify them as partition
// creation parameters (or similar).
//

#define HV_ARM64_CHILD_GICD_BASE_GPA 0x00000000FFFF0000UI64

//
// The exact number of pages depends on which generation of GIC is in use. For now,
// we just encode the maximum possible across the generations we care about (16)
//

#define HV_ARM64_CHILD_GICD_PAGE_COUNT 16

//
// The (common) base address for each child GICC for GicV2.
//

#define HV_ARM64_CHILD_GICC_V2_BASE_GPA 0x00000000FFFEE000UI64
#define HV_ARM64_CHILD_GICC_V2_PAGE_COUNT 2

//
// The start of the GICR region, the size per GICR and the total size of the
// region.
//

#define HV_ARM64_CHILD_GICR_REGION_BASE_GPA 0x00000000EFFEE000UI64
#define HV_ARM64_CHILD_GICR_PAGE_COUNT 16
#define HV_ARM64_CHILD_GICR_REGION_PAGE_COUNT (4096 * HV_ARM64_CHILD_GICR_PAGE_COUNT)

typedef union _HV_X64_DELIVERABILITY_NOTIFICATIONS_REGISTER
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 NmiNotification:1;
        UINT64 InterruptNotification:1;
        UINT64 InterruptPriority:4;
        UINT64 RsvdZ:58;
    };

} HV_X64_DELIVERABILITY_NOTIFICATIONS_REGISTER,
  *PHV_X64_DELIVERABILITY_NOTIFICATIONS_REGISTER;


#define HV_PCI_BUS_COUNT        256
#define HV_PCI_DEVICE_COUNT     32
#define HV_PCI_FUNCTION_COUNT   8
#define HV_PCI_RID_COUNT        65536
#define HV_PCI_RIDS_PER_BUS     256
#define HV_PCI_SEGMENT_COUNT    65536

typedef struct _HV_LOADER_IOMMU_ENTRY
{
    HV_PCI_SEGMENT SegmentNumber;
    HV_SPA RegisterBaseAddress;

    struct
    {
        union
        {
            struct
            {
                UINT8 HtTunEn : 1;
                UINT8 PassPW : 1;
                UINT8 ResPassPW : 1;
                UINT8 Isoc : 1;
                UINT8 IotlbSup : 1;
                UINT8 : 3;
            };

            UINT8 AsUINT8;

        } IvhdFlags;

        HV_PCI_BDF IommuDeviceBdf;
        HV_SPA PciMmConfigAddress;
        UINT64 Efr;

    } Amd;

    HV_SPA ContiguousPagesBaseSpa;
    UINT64 DevicePresentBitmap[HV_PCI_RID_COUNT / 64];

} HV_LOADER_IOMMU_ENTRY, *PHV_LOADER_IOMMU_ENTRY;

typedef struct _HV_LOADER_PCI_SEGMENT_ENTRY
{
    HV_PCI_SEGMENT SegmentNumber;
    UINT64 ReservedDeviceBitmap[HV_PCI_RID_COUNT / 64];

} HV_LOADER_PCI_SEGMENT_ENTRY, *PHV_LOADER_PCI_SEGMENT_ENTRY;

//
// IOMMU extended status definitions.
//
typedef UINT16 HV_IOMMU_STATUS, *PHV_IOMMU_STATUS;
typedef UINT8 HV_IOMMU_STATUS_SEVERITY, *PHV_IOMMU_STATUS_SEVERITY;

#define HV_IOMMU_STATUS_SEVERITY_SUCCESS    0x10
#define HV_IOMMU_STATUS_SEVERITY_WARNING    0x20
#define HV_IOMMU_STATUS_SEVERITY_FAILURE    0x30

//
// IOMMU limits.
//

//
// AMD limits the maximum number of IOAPICs that can share the same RID. Each
// IOAPIC requires up to 121 IRT entries from a total of 512-32 entries
// available per RID.
//
#define HV_IOMMU_AMD_MAX_IOAPICS_PER_RID    3

//
// Pages needed at launch to initialize IOMMUs.
//

//
// Intel needs contiguous pages for the interrupt remapping table of each
// IOMMU.
//
// This has 64k entries, each 16 bytes, for 256 pages.
//
#define HV_IOMMU_INTEL_CONTIGUOUS_PAGES             256

//
// AMD needs contiguous pages for the device table of each IOMMU.
//
// This has 64k entries, each 32 bytes, for 512 pages.
//
#define HV_IOMMU_AMD_CONTIGUOUS_PAGES               512

//
// Pages needed per structure. This includes page table pages needed to map the
// contiguous pages, etc.
//
#define HV_IOMMU_INITIAL_PAGES_PER_IOMMU            32
#define HV_IOMMU_INITIAL_PAGES_PER_SEGMENT          32
#define HV_IOMMU_INITIAL_PAGES_PER_IOAPIC           8

//
// Intel needs additional pages for the device table of each IOMMU. This
// consists of 1 page for the root-entry table and up to 512 pages for the
// context-entry tables if extended context entries are used. These do not have
// to be contiguous.
//
#define HV_IOMMU_INTEL_ADDITIONAL_PAGES_PER_IOMMU   513

//
// IOMMU initialization status definitions.
//

#define HV_IOMMU_INIT_FEATURE_X2APIC                        0x00000001
#define HV_IOMMU_INIT_FEATURE_DMA_PROTECTION                0x00000002
#define HV_IOMMU_INIT_FEATURE_DEVICE_ASSIGNMENT             0x00000004
#define HV_IOMMU_INIT_FEATURE_ROOT_SVM                      0x00000008
#define HV_IOMMU_INIT_FEATURE_CHILD_SVM                     0x00000010
#define HV_IOMMU_INIT_FEATURE_POSTED_INTERRUPTS             0x00000020

#define HV_IOMMU_INIT_STATUS_NO_SLAT                        0x00000001
#define HV_IOMMU_INIT_STATUS_NO_INTERRUPT_REMAPPING         0x00000002
#define HV_IOMMU_INIT_STATUS_NO_DMA_PROTECTION              0x00000004
#define HV_IOMMU_INIT_STATUS_GLOBAL_NO_DEVICE_ASSIGNMENT    0x00000008
#define HV_IOMMU_INIT_STATUS_UNIT_NO_DEVICE_ASSIGNMENT      0x00000010
#define HV_IOMMU_INIT_STATUS_GLOBAL_NO_ROOT_SVM             0x00000020
#define HV_IOMMU_INIT_STATUS_UNIT_NO_ROOT_SVM               0x00000040
#define HV_IOMMU_INIT_STATUS_GLOBAL_NO_CHILD_SVM            0x00000080
#define HV_IOMMU_INIT_STATUS_UNIT_NO_CHILD_SVM              0x00000100
#define HV_IOMMU_INIT_STATUS_UNKNOWN_ACPI_TABLE             0x0000000100000000UI64
#define HV_IOMMU_INIT_STATUS_INTERNAL_ERROR                 0x0000000200000000UI64

#define HV_IOMMU_INIT_ERROR_BAD_ACPI_TABLE                  0x00000001
#define HV_IOMMU_INIT_ERROR_SCOPE_CONFLICT                  0x00000002
#define HV_IOMMU_INIT_ERROR_IOAPIC_CONFLICT                 0x00000004
#define HV_IOMMU_INIT_ERROR_IOAPIC_MISSING                  0x00000008
#define HV_IOMMU_INIT_ERROR_NOT_RESPONDING                  0x00000010
#define HV_IOMMU_INIT_ERROR_ERRATA                          0x8000000000000000UI64

#define HV_IOMMU_INIT_ERRATA_TYLERSBURG_CLIENT              1
#define HV_IOMMU_INIT_ERRATA_TYLERSBURG_SERVER              2


#if defined(_ARM64_)

//
// Define virtual processor execution state bitfield.
//
typedef union _HV_ARM64_VP_EXECUTION_STATE
{
    UINT16 AsUINT16;
    struct
    {
        UINT16 Cpl:2;
        UINT16 DebugActive:1;
        UINT16 InterruptionPending:1;
        UINT16 Vtl:4;
        UINT16 Reserved:8;
    };
} HV_ARM64_VP_EXECUTION_STATE, *PHV_ARM64_VP_EXECUTION_STATE;

#define _HV_VP_EXECUTION_STATE _HV_ARM64_VP_EXECUTION_STATE
#define HV_VP_EXECUTION_STATE HV_ARM64_VP_EXECUTION_STATE
#define PHV_VP_EXECUTION_STATE PHV_ARM64_VP_EXECUTION_STATE

//
// Define intercept message header structure.
//
typedef struct _HV_ARM64_INTERCEPT_MESSAGE_HEADER
{
    HV_VP_INDEX VpIndex;
    UINT8 InstructionLength;
    HV_INTERCEPT_ACCESS_TYPE InterceptAccessType;
    HV_ARM64_VP_EXECUTION_STATE ExecutionState;
    UINT64 Pc;
    UINT64 Cpsr;
} HV_ARM64_INTERCEPT_MESSAGE_HEADER, *PHV_ARM64_INTERCEPT_MESSAGE_HEADER;

#define _HV_INTERCEPT_MESSAGE_HEADER _HV_ARM64_INTERCEPT_MESSAGE_HEADER
#define HV_INTERCEPT_MESSAGE_HEADER HV_ARM64_INTERCEPT_MESSAGE_HEADER
#define PHV_INTERCEPT_MESSAGE_HEADER PHV_ARM64_INTERCEPT_MESSAGE_HEADER

//
// Define register access information structure.
//
typedef union _HV_ARM64_REGISTER_ACCESS_INFO
{
    HV_REGISTER_VALUE SourceValue;
    HV_REGISTER_NAME DestinationRegister;
} HV_ARM64_REGISTER_ACCESS_INFO, *PHV_ARM64_REGISTER_ACCESS_INFO;

#define _HV_REGISTER_ACCESS_INFO _HV_ARM64_REGISTER_ACCESS_INFO
#define HV_REGISTER_ACCESS_INFO HV_ARM64_REGISTER_ACCESS_INFO
#define PHV_REGISTER_ACCESS_INFO PHV_ARM64_REGISTER_ACCESS_INFO


//
// Define register intercept message structure.
//
typedef struct _HV_ARM64_REGISTER_INTERCEPT_MESSAGE
{
    HV_INTERCEPT_MESSAGE_HEADER Header;
    struct
    {
        UINT8 IsMemoryOp:1;
        UINT8 Reserved:7;
    };
    UINT8 Reserved8;
    UINT16 Reserved16;
    HV_REGISTER_NAME RegisterName;
    HV_REGISTER_ACCESS_INFO AccessInfo;
} HV_ARM64_REGISTER_INTERCEPT_MESSAGE, *PHV_ARM64_REGISTER_INTERCEPT_MESSAGE;

#define _HV_REGISTER_INTERCEPT_MESSAGE _HV_ARM64_REGISTER_INTERCEPT_MESSAGE
#define HV_REGISTER_INTERCEPT_MESSAGE HV_ARM64_REGISTER_INTERCEPT_MESSAGE
#define PHV_REGISTER_INTERCEPT_MESSAGE PHV_ARM64_REGISTER_INTERCEPT_MESSAGE

//
// Define memory access information structure.
//
typedef union _HV_ARM64_MEMORY_ACCESS_INFO
{
    UINT8 AsUINT8;
    struct
    {
        UINT8 GvaValid:1;
        UINT8 GvaGpaValid:1;
        UINT8 HypercallOutputPending:1;
        UINT8 Reserved:5;
    };
} HV_ARM64_MEMORY_ACCESS_INFO, *PHV_ARM64_MEMORY_ACCESS_INFO;

#define _HV_MEMORY_ACCESS_INFO _HV_ARM64_MEMORY_ACCESS_INFO
#define HV_MEMORY_ACCESS_INFO HV_ARM64_MEMORY_ACCESS_INFO
#define PHV_MEMORY_ACCESS_INFO PHV_ARM64_MEMORY_ACCESS_INFO

//
// Define exception information structure.
//
typedef union _HV_ARM64_EXCEPTION_INFO
{
    UINT8 AsUINT8;
    struct
    {
        UINT8 ErrorCodeValid:1;
        UINT8 Reserved:7;
    };
} HV_ARM64_EXCEPTION_INFO, *PHV_ARM64_EXCEPTION_INFO;

#define _HV_EXCEPTION_INFO _HV_ARM64_EXCEPTION_INFO
#define HV_EXCEPTION_INFO HV_ARM64_EXCEPTION_INFO
#define PHV_EXCEPTION_INFO PHV_ARM64_EXCEPTION_INFO

//
// Define memory access message structure. This message structure is used for
// memory intercepts, GPA not present intercepts, GPA not accepted intercepts,
// and SPA access violation intercepts.
//
// N.B. Sending the registers as part of the intercept is only an optimization.
//      Therefore, we only include some "interesting" values here, like
//      volatiles. The worker stack can always query other registers later. This
//      is why we define X[16] registers in this structure.
//
typedef struct _HV_ARM64_MEMORY_INTERCEPT_MESSAGE
{
    HV_ARM64_INTERCEPT_MESSAGE_HEADER Header;
    HV_CACHE_TYPE CacheType;
    UINT8 InstructionByteCount;
    HV_ARM64_MEMORY_ACCESS_INFO MemoryAccessInfo;
    UINT16 Reserved1;
    UINT64 GuestVirtualAddress;
    UINT64 GuestPhysicalAddress;
    UINT8 InstructionBytes[16];
    UINT64 X[16];
    UINT64 Fp;
    UINT64 Lr;
    UINT64 Sp;  // Based on Cpsr.SPSel
    UINT64 Cpsr;
} HV_ARM64_MEMORY_INTERCEPT_MESSAGE, *PHV_ARM64_MEMORY_INTERCEPT_MESSAGE;

C_ASSERT(sizeof(HV_ARM64_MEMORY_INTERCEPT_MESSAGE) <= HV_MESSAGE_PAYLOAD_BYTE_COUNT);

#define _HV_MEMORY_INTERCEPT_MESSAGE _HV_ARM64_MEMORY_INTERCEPT_MESSAGE
#define HV_MEMORY_INTERCEPT_MESSAGE HV_ARM64_MEMORY_INTERCEPT_MESSAGE
#define PHV_MEMORY_INTERCEPT_MESSAGE PHV_ARM64_MEMORY_INTERCEPT_MESSAGE

//
// Define MMIO intercept message structure. This message structure is used for
// MMIO intercepts and contains the access size and data (in case of writes)
// derived from the decoded load / store instruction.
//

#define HV_MMIO_INTERCEPT_MAX_ACCESS_SIZE 32

typedef struct _HV_ARM64_MMIO_INTERCEPT_MESSAGE
{
    HV_ARM64_INTERCEPT_MESSAGE_HEADER Header;
    HV_GPA GuestPhysicalAddress;
    UINT32 AccessSize;
    UINT8 Data[HV_MMIO_INTERCEPT_MAX_ACCESS_SIZE];
} HV_ARM64_MMIO_INTERCEPT_MESSAGE, *PHV_ARM64_MMIO_INTERCEPT_MESSAGE;

C_ASSERT(sizeof(HV_ARM64_MMIO_INTERCEPT_MESSAGE) <= HV_MESSAGE_PAYLOAD_BYTE_COUNT);

#define _HV_MMIO_INTERCEPT_MESSAGE _HV_ARM64_MMIO_INTERCEPT_MESSAGE
#define HV_MMIO_INTERCEPT_MESSAGE HV_ARM64_MMIO_INTERCEPT_MESSAGE
#define PHV_MMIO_INTERCEPT_MESSAGE PHV_ARM64_MMIO_INTERCEPT_MESSAGE

//
// Define MSR intercept message structure.
//
typedef struct _HV_ARM64_MSR_INTERCEPT_MESSAGE
{
    HV_ARM64_INTERCEPT_MESSAGE_HEADER Header;
    UINT64 VirtualRegisterNumber;
    UINT64 X0;
    UINT64 X1;
} HV_ARM64_MSR_INTERCEPT_MESSAGE, *PHV_ARM64_MSR_INTERCEPT_MESSAGE;

#define _HV_MSR_INTERCEPT_MESSAGE _HV_ARM64_MSR_INTERCEPT_MESSAGE
#define HV_MSR_INTERCEPT_MESSAGE HV_ARM64_MSR_INTERCEPT_MESSAGE
#define PHV_MSR_INTERCEPT_MESSAGE PHV_ARM64_MSR_INTERCEPT_MESSAGE


//
// Define exception intercept message.
//
// N.B. Sending the registers as part of the intercept
//      is only an optimization. Therefore, we only
//      include some "interesting" values here, like
//      volatiles.
//      The worker stack can always query other registers
//      later.
//      This is why we define X[16] registers in this
//      structure.
//
typedef struct _HV_ARM64_EXCEPTION_INTERCEPT_MESSAGE
{
    HV_ARM64_INTERCEPT_MESSAGE_HEADER Header;
    UINT16 ExceptionVector;
    HV_ARM64_EXCEPTION_INFO ExceptionInfo;
    UINT8 InstructionByteCount;
    UINT32 ErrorCode;
    UINT64 ExceptionParameter;
    UINT64 Reserved;
    UINT8 InstructionBytes[16];
    UINT64 X[16];
    UINT64 Fp;
    UINT64 Lr;
    UINT64 Sp;  // Based on Cpsr.SPSel
    UINT64 Cpsr;
} HV_ARM64_EXCEPTION_INTERCEPT_MESSAGE, *PHV_ARM64_EXCEPTION_INTERCEPT_MESSAGE;

C_ASSERT(sizeof(HV_ARM64_EXCEPTION_INTERCEPT_MESSAGE) <= HV_MESSAGE_PAYLOAD_BYTE_COUNT);

#define _HV_EXCEPTION_INTERCEPT_MESSAGE _HV_ARM64_EXCEPTION_INTERCEPT_MESSAGE
#define HV_EXCEPTION_INTERCEPT_MESSAGE HV_ARM64_EXCEPTION_INTERCEPT_MESSAGE
#define PHV_EXCEPTION_INTERCEPT_MESSAGE PHV_ARM64_EXCEPTION_INTERCEPT_MESSAGE

//
// Define invalid virtual processor register message.
//
typedef struct _HV_ARM64_INVALID_VP_REGISTER_MESSAGE
{
    UINT32 VpIndex;
    UINT32 Reserved;
} HV_ARM64_INVALID_VP_REGISTER_MESSAGE, *PHV_ARM64_INVALID_VP_REGISTER_MESSAGE;

#define _HV_INVALID_VP_REGISTER_MESSAGE _HV_ARM64_INVALID_VP_REGISTER_MESSAGE
#define HV_INVALID_VP_REGISTER_MESSAGE HV_ARM64_INVALID_VP_REGISTER_MESSAGE
#define PHV_INVALID_VP_REGISTER_MESSAGE PHV_ARM64_INVALID_VP_REGISTER_MESSAGE

//
// Define virtual processor unrecoverable error message.
//
typedef struct _HV_ARM64_UNRECOVERABLE_EXCEPTION_MESSAGE
{
    HV_ARM64_INTERCEPT_MESSAGE_HEADER Header;
} HV_ARM64_UNRECOVERABLE_EXCEPTION_MESSAGE, *PHV_ARM64_UNRECOVERABLE_EXCEPTION_MESSAGE;

#define _HV_UNRECOVERABLE_EXCEPTION_MESSAGE _HV_ARM64_UNRECOVERABLE_EXCEPTION_MESSAGE
#define HV_UNRECOVERABLE_EXCEPTION_MESSAGE HV_ARM64_UNRECOVERABLE_EXCEPTION_MESSAGE
#define PHV_UNRECOVERABLE_EXCEPTION_MESSAGE PHV_ARM64_UNRECOVERABLE_EXCEPTION_MESSAGE

//
// Define the unsupported feature codes.
//
typedef enum _HV_ARM64_UNSUPPORTED_FEATURE_CODE
{
    HvUnsupportedFeatureIntercept = 1,
} HV_ARM64_UNSUPPORTED_FEATURE_CODE, *PHV_ARM64_UNSUPPORTED_FEATURE_CODE;

#define _HV_UNSUPPORTED_FEATURE_CODE _HV_ARM64_UNSUPPORTED_FEATURE_CODE
#define HV_UNSUPPORTED_FEATURE_CODE HV_ARM64_UNSUPPORTED_FEATURE_CODE
#define PHV_UNSUPPORTED_FEATURE_CODE PHV_ARM64_UNSUPPORTED_FEATURE_CODE

//
// Define unsupported feature message.
//
typedef struct _HV_ARM64_UNSUPPORTED_FEATURE_MESSAGE
{
    UINT32 VpIndex;
    HV_UNSUPPORTED_FEATURE_CODE FeatureCode;
    UINT64 FeatureParameter;
} HV_ARM64_UNSUPPORTED_FEATURE_MESSAGE, *PHV_ARM64_UNSUPPORTED_FEATURE_MESSAGE;

#define _HV_UNSUPPORTED_FEATURE_MESSAGE _HV_ARM64_UNSUPPORTED_FEATURE_MESSAGE
#define HV_UNSUPPORTED_FEATURE_MESSAGE HV_ARM64_UNSUPPORTED_FEATURE_MESSAGE
#define PHV_UNSUPPORTED_FEATURE_MESSAGE PHV_ARM64_UNSUPPORTED_FEATURE_MESSAGE

//
// Define TLB page size mismatch message.
//
typedef struct _HV_ARM64_TLB_PAGE_SIZE_MISMATCH_MESSAGE
{
    UINT32 VpIndex;
    UINT32 Reserved;
} HV_ARM64_TLB_PAGE_SIZE_MISMATCH_MESSAGE, *PHV_ARM64_TLB_PAGE_SIZE_MISMATCH_MESSAGE;

#define _HV_TLB_PAGE_SIZE_MISMATCH_MESSAGE _HV_ARM64_TLB_PAGE_SIZE_MISMATCH_MESSAGE
#define HV_TLB_PAGE_SIZE_MISMATCH_MESSAGE HV_ARM64_TLB_PAGE_SIZE_MISMATCH_MESSAGE
#define  PHV_TLB_PAGE_SIZE_MISMATCH_MESSAGE PHV_ARM64_TLB_PAGE_SIZE_MISMATCH_MESSAGE

//
// Hypercall intercept definitions.
//

#define HV_HYPERCALL_INTERCEPT_MAX_X_REGISTERS 16

typedef struct _HV_ARM64_HYPERCALL_INTERCEPT_MESSAGE
{
    HV_INTERCEPT_MESSAGE_HEADER Header;
    HV_HYPERCALL_INPUT_PRIVATE InputControl;
    UINT64 X[HV_HYPERCALL_INTERCEPT_MAX_X_REGISTERS];
    struct
    {
        UINT32 Isolated:1;
        UINT32 Reserved:31;
    };
} HV_ARM64_HYPERCALL_INTERCEPT_MESSAGE, *PHV_ARM64_HYPERCALL_INTERCEPT_MESSAGE;

#define _HV_HYPERCALL_INTERCEPT_MESSAGE _HV_ARM64_HYPERCALL_INTERCEPT_MESSAGE
#define HV_HYPERCALL_INTERCEPT_MESSAGE HV_ARM64_HYPERCALL_INTERCEPT_MESSAGE
#define PHV_HYPERCALL_INTERCEPT_MESSAGE PHV_ARM64_HYPERCALL_INTERCEPT_MESSAGE

//
// System Reset interceft definition
//

typedef enum _HV_ARM64_RESET_TYPE
{
    HvArm64ResetTypePowerOff = 0,
    HvArm64ResetTypeReboot,
    HvArm64ResetTypeMax
} HV_ARM64_RESET_TYPE, *PHV_ARM64_RESET_TYPE;

typedef struct _HV_ARM64_RESET_INTERCEPT_MESSAGE
{
    HV_INTERCEPT_MESSAGE_HEADER Header;
    HV_ARM64_RESET_TYPE ResetType;
} HV_ARM64_RESET_INTERCEPT_MESSAGE, *PHV_ARM64_RESET_INTERCEPT_MESSAGE;

#else

//
// Define virtual processor execution state bitfield.
//
typedef union _HV_X64_VP_EXECUTION_STATE
{
    UINT16 AsUINT16;
    struct
    {
        UINT16 Cpl:2;
        UINT16 Cr0Pe:1;
        UINT16 Cr0Am:1;
        UINT16 EferLma:1;
        UINT16 DebugActive:1;
        UINT16 InterruptionPending:1;
        UINT16 Vtl:4;
        UINT16 EnclaveMode:1;
        UINT16 InterruptShadow:1;
        UINT16 VirtualizationFaultActive:1;
        UINT16 Reserved:2;
    };
} HV_X64_VP_EXECUTION_STATE, *PHV_X64_VP_EXECUTION_STATE;

#define _HV_VP_EXECUTION_STATE _HV_X64_VP_EXECUTION_STATE
#define HV_VP_EXECUTION_STATE HV_X64_VP_EXECUTION_STATE
#define PHV_VP_EXECUTION_STATE PHV_X64_VP_EXECUTION_STATE

//
// Define intercept message header structure.
//
typedef struct _HV_X64_INTERCEPT_MESSAGE_HEADER
{
    HV_VP_INDEX VpIndex;
    UINT8 InstructionLength:4;
    UINT8 Cr8:4; // only set for Exo partitions
    HV_INTERCEPT_ACCESS_TYPE InterceptAccessType;
    HV_X64_VP_EXECUTION_STATE ExecutionState;
    HV_X64_SEGMENT_REGISTER CsSegment;
    UINT64 Rip;
    UINT64 Rflags;
} HV_X64_INTERCEPT_MESSAGE_HEADER, *PHV_X64_INTERCEPT_MESSAGE_HEADER;

#define _HV_INTERCEPT_MESSAGE_HEADER _HV_X64_INTERCEPT_MESSAGE_HEADER
#define HV_INTERCEPT_MESSAGE_HEADER HV_X64_INTERCEPT_MESSAGE_HEADER
#define PHV_INTERCEPT_MESSAGE_HEADER PHV_X64_INTERCEPT_MESSAGE_HEADER


//
// Hypercall intercept definitions.
//

#define HV_HYPERCALL_INTERCEPT_MAX_XMM_REGISTERS 6

typedef struct _HV_X64_HYPERCALL_INTERCEPT_MESSAGE
{
    HV_X64_INTERCEPT_MESSAGE_HEADER Header;
    UINT64 Rax;
    UINT64 Rbx;
    UINT64 Rcx;
    UINT64 Rdx;
    UINT64 R8;
    UINT64 Rsi;
    UINT64 Rdi;
    HV_UINT128 XmmRegisters[HV_HYPERCALL_INTERCEPT_MAX_XMM_REGISTERS];
    struct
    {
        UINT32 Isolated:1;
        UINT32 Reserved:31;
    };
} HV_X64_HYPERCALL_INTERCEPT_MESSAGE, *PHV_X64_HYPERCALL_INTERCEPT_MESSAGE;

#define _HV_HYPERCALL_INTERCEPT_MESSAGE _HV_X64_HYPERCALL_INTERCEPT_MESSAGE
#define HV_HYPERCALL_INTERCEPT_MESSAGE HV_X64_HYPERCALL_INTERCEPT_MESSAGE
#define PHV_HYPERCALL_INTERCEPT_MESSAGE PHV_X64_HYPERCALL_INTERCEPT_MESSAGE


typedef enum _VAL_REGISTER_NAME VAL_REGISTER_NAME, *PVAL_REGISTER_NAME;


//
// Define register access information structure.
//
typedef union _HV_X64_REGISTER_ACCESS_INFO
{
    HV_REGISTER_VALUE SourceValue;
    HV_REGISTER_NAME DestinationRegister;
    UINT64 SourceAddress;
    UINT64 DestinationAddress;
} HV_X64_REGISTER_ACCESS_INFO, *PHV_X64_REGISTER_ACCESS_INFO;

#define _HV_REGISTER_ACCESS_INFO _HV_X64_REGISTER_ACCESS_INFO
#define HV_REGISTER_ACCESS_INFO HV_X64_REGISTER_ACCESS_INFO
#define PHV_REGISTER_ACCESS_INFO PHV_X64_REGISTER_ACCESS_INFO


//
// Define register intercept message structure.
//
typedef struct _HV_X64_REGISTER_INTERCEPT_MESSAGE
{
    HV_INTERCEPT_MESSAGE_HEADER Header;
    struct
    {
        UINT8 IsMemoryOp:1;
        UINT8 Reserved:7;
    };
    UINT8 Reserved8;
    UINT16 Reserved16;
    HV_REGISTER_NAME RegisterName;
    HV_REGISTER_ACCESS_INFO AccessInfo;
} HV_X64_REGISTER_INTERCEPT_MESSAGE, *PHV_X64_REGISTER_INTERCEPT_MESSAGE;

#define _HV_REGISTER_INTERCEPT_MESSAGE _HV_X64_REGISTER_INTERCEPT_MESSAGE
#define HV_REGISTER_INTERCEPT_MESSAGE HV_X64_REGISTER_INTERCEPT_MESSAGE
#define PHV_REGISTER_INTERCEPT_MESSAGE PHV_X64_REGISTER_INTERCEPT_MESSAGE


//
// Define memory access information structure.
//
typedef union _HV_X64_MEMORY_ACCESS_INFO
{
    UINT8 AsUINT8;
    struct
    {
        UINT8 GvaValid:1;
        UINT8 GvaGpaValid:1;
        UINT8 HypercallOutputPending:1;
        UINT8 Reserved:5;
    };
} HV_X64_MEMORY_ACCESS_INFO, *PHV_X64_MEMORY_ACCESS_INFO;

#define _HV_MEMORY_ACCESS_INFO _HV_X64_MEMORY_ACCESS_INFO
#define HV_MEMORY_ACCESS_INFO HV_X64_MEMORY_ACCESS_INFO
#define PHV_MEMORY_ACCESS_INFO PHV_X64_MEMORY_ACCESS_INFO

//
// Define IO port access information structure.
//
typedef union _HV_X64_IO_PORT_ACCESS_INFO
{
    UINT8 AsUINT8;
    struct
    {
        UINT8 AccessSize:3;
        UINT8 StringOp:1;
        UINT8 RepPrefix:1;
        UINT8 Reserved:3;
    };
} HV_X64_IO_PORT_ACCESS_INFO, *PHV_X64_IO_PORT_ACCESS_INFO;


//
// Define exception information structure.
//
typedef union _HV_X64_EXCEPTION_INFO
{
    UINT8 AsUINT8;
    struct
    {
        UINT8 ErrorCodeValid:1;
        UINT8 SoftwareException:1;
        UINT8 Reserved:6;
    };
} HV_X64_EXCEPTION_INFO, *PHV_X64_EXCEPTION_INFO;

#define _HV_EXCEPTION_INFO _HV_X64_EXCEPTION_INFO
#define HV_EXCEPTION_INFO HV_X64_EXCEPTION_INFO
#define PHV_EXCEPTION_INFO PHV_X64_EXCEPTION_INFO

//
// Define memory access message structure. This message structure is used for
// memory intercepts, GPA not present intercepts, GPA not accepted intercepts,
// and SPA access violation intercepts.
//
typedef struct _HV_X64_MEMORY_INTERCEPT_MESSAGE
{
    HV_X64_INTERCEPT_MESSAGE_HEADER Header;
    HV_CACHE_TYPE CacheType;
    UINT8 InstructionByteCount;
    HV_X64_MEMORY_ACCESS_INFO MemoryAccessInfo;
    UINT8 TprPriority;
    UINT8 Reserved1;
    UINT64 GuestVirtualAddress;
    UINT64 GuestPhysicalAddress;
    UINT8 InstructionBytes[16];
} HV_X64_MEMORY_INTERCEPT_MESSAGE, *PHV_X64_MEMORY_INTERCEPT_MESSAGE;

#define _HV_MEMORY_INTERCEPT_MESSAGE _HV_X64_MEMORY_INTERCEPT_MESSAGE
#define HV_MEMORY_INTERCEPT_MESSAGE HV_X64_MEMORY_INTERCEPT_MESSAGE
#define PHV_MEMORY_INTERCEPT_MESSAGE PHV_X64_MEMORY_INTERCEPT_MESSAGE

//
// Define CPUID intercept message structure.
//
typedef struct _HV_X64_CPUID_INTERCEPT_MESSAGE
{
    HV_INTERCEPT_MESSAGE_HEADER Header;
    UINT64 Rax;
    UINT64 Rcx;
    UINT64 Rdx;
    UINT64 Rbx;
    UINT64 DefaultResultRax;
    UINT64 DefaultResultRcx;
    UINT64 DefaultResultRdx;
    UINT64 DefaultResultRbx;
} HV_X64_CPUID_INTERCEPT_MESSAGE, *PHV_X64_CPUID_INTERCEPT_MESSAGE;

//
// Define MSR intercept message structure.
//
typedef struct _HV_X64_MSR_INTERCEPT_MESSAGE
{
    HV_X64_INTERCEPT_MESSAGE_HEADER Header;
    UINT32 MsrNumber;
    UINT32 Reserved;
    UINT64 Rdx;
    UINT64 Rax;
} HV_X64_MSR_INTERCEPT_MESSAGE, *PHV_X64_MSR_INTERCEPT_MESSAGE;

#define _HV_MSR_INTERCEPT_MESSAGE _HV_X64_MSR_INTERCEPT_MESSAGE
#define HV_MSR_INTERCEPT_MESSAGE HV_X64_MSR_INTERCEPT_MESSAGE
#define PHV_MSR_INTERCEPT_MESSAGE PHV_X64_MSR_INTERCEPT_MESSAGE

//
// Define IO access intercept message structure.
//
typedef struct _HV_X64_IO_PORT_INTERCEPT_MESSAGE
{
    HV_X64_INTERCEPT_MESSAGE_HEADER Header;
    UINT16 PortNumber;
    HV_X64_IO_PORT_ACCESS_INFO AccessInfo;
    UINT8 InstructionByteCount;
    UINT32 Reserved;
    UINT64 Rax;
    UINT8 InstructionBytes[16];
    HV_X64_SEGMENT_REGISTER DsSegment;
    HV_X64_SEGMENT_REGISTER EsSegment;
    UINT64 Rcx;
    UINT64 Rsi;
    UINT64 Rdi;
} HV_X64_IO_PORT_INTERCEPT_MESSAGE, *PHV_X64_IO_PORT_INTERCEPT_MESSAGE;

//
// Define exception intercept message.
//
typedef struct _HV_X64_EXCEPTION_INTERCEPT_MESSAGE
{
    HV_X64_INTERCEPT_MESSAGE_HEADER Header;
    UINT16 ExceptionVector;
    HV_X64_EXCEPTION_INFO ExceptionInfo;
    UINT8 InstructionByteCount;
    UINT32 ErrorCode;
    UINT64 ExceptionParameter;
    UINT64 Reserved;
    UINT8 InstructionBytes[16];
    HV_X64_SEGMENT_REGISTER DsSegment;
    HV_X64_SEGMENT_REGISTER SsSegment;
    UINT64 Rax;
    UINT64 Rcx;
    UINT64 Rdx;
    UINT64 Rbx;
    UINT64 Rsp;
    UINT64 Rbp;
    UINT64 Rsi;
    UINT64 Rdi;
    UINT64 R8;
    UINT64 R9;
    UINT64 R10;
    UINT64 R11;
    UINT64 R12;
    UINT64 R13;
    UINT64 R14;
    UINT64 R15;
} HV_X64_EXCEPTION_INTERCEPT_MESSAGE, *PHV_X64_EXCEPTION_INTERCEPT_MESSAGE;

#define _HV_EXCEPTION_INTERCEPT_MESSAGE _HV_X64_EXCEPTION_INTERCEPT_MESSAGE
#define HV_EXCEPTION_INTERCEPT_MESSAGE HV_X64_EXCEPTION_INTERCEPT_MESSAGE
#define PHV_EXCEPTION_INTERCEPT_MESSAGE PHV_X64_EXCEPTION_INTERCEPT_MESSAGE

//
// Define invalid virtual processor register message.
//
typedef struct _HV_X64_INVALID_VP_REGISTER_MESSAGE
{
    UINT32 VpIndex;
    UINT32 Reserved;
} HV_X64_INVALID_VP_REGISTER_MESSAGE, *PHV_X64_INVALID_VP_REGISTER_MESSAGE;

#define _HV_INVALID_VP_REGISTER_MESSAGE _HV_X64_INVALID_VP_REGISTER_MESSAGE
#define HV_INVALID_VP_REGISTER_MESSAGE HV_X64_INVALID_VP_REGISTER_MESSAGE
#define PHV_INVALID_VP_REGISTER_MESSAGE PHV_X64_INVALID_VP_REGISTER_MESSAGE

//
// Define virtual processor unrecoverable error message.
//
typedef struct _HV_X64_UNRECOVERABLE_EXCEPTION_MESSAGE
{
    HV_X64_INTERCEPT_MESSAGE_HEADER Header;
} HV_X64_UNRECOVERABLE_EXCEPTION_MESSAGE, *PHV_X64_UNRECOVERABLE_EXCEPTION_MESSAGE;

#define _HV_UNRECOVERABLE_EXCEPTION_MESSAGE _HV_X64_UNRECOVERABLE_EXCEPTION_MESSAGE
#define HV_UNRECOVERABLE_EXCEPTION_MESSAGE HV_X64_UNRECOVERABLE_EXCEPTION_MESSAGE
#define PHV_UNRECOVERABLE_EXCEPTION_MESSAGE PHV_X64_UNRECOVERABLE_EXCEPTION_MESSAGE

//
// Define the unsupported feature codes.
//
typedef enum _HV_X64_UNSUPPORTED_FEATURE_CODE
{
    HvUnsupportedFeatureIntercept = 1,
    HvUnsupportedFeatureTaskSwitchTss = 2
}HV_X64_UNSUPPORTED_FEATURE_CODE, *PHV_X64_UNSUPPORTED_FEATURE_CODE;

#define _HV_UNSUPPORTED_FEATURE_CODE _HV_X64_UNSUPPORTED_FEATURE_CODE
#define HV_UNSUPPORTED_FEATURE_CODE HV_X64_UNSUPPORTED_FEATURE_CODE
#define PHV_UNSUPPORTED_FEATURE_CODE PHV_X64_UNSUPPORTED_FEATURE_CODE

//
// Define unsupported feature message.
//
typedef struct _HV_X64_UNSUPPORTED_FEATURE_MESSAGE
{
    UINT32 VpIndex;
    HV_UNSUPPORTED_FEATURE_CODE FeatureCode;
    UINT64 FeatureParameter;
} HV_X64_UNSUPPORTED_FEATURE_MESSAGE, *PHV_X64_UNSUPPORTED_FEATURE_MESSAGE;

#define _HV_UNSUPPORTED_FEATURE_MESSAGE _HV_X64_UNSUPPORTED_FEATURE_MESSAGE
#define HV_UNSUPPORTED_FEATURE_MESSAGE HV_X64_UNSUPPORTED_FEATURE_MESSAGE
#define PHV_UNSUPPORTED_FEATURE_MESSAGE PHV_X64_UNSUPPORTED_FEATURE_MESSAGE

//
// Define TLB page size mismatch message.
//
typedef struct _HV_X64_TLB_PAGE_SIZE_MISMATCH_MESSAGE
{
    UINT32 VpIndex;
    UINT32 Reserved;
} HV_X64_TLB_PAGE_SIZE_MISMATCH_MESSAGE, *PHV_X64_TLB_PAGE_SIZE_MISMATCH_MESSAGE;

#define _HV_TLB_PAGE_SIZE_MISMATCH_MESSAGE _HV_X64_TLB_PAGE_SIZE_MISMATCH_MESSAGE
#define HV_TLB_PAGE_SIZE_MISMATCH_MESSAGE HV_X64_TLB_PAGE_SIZE_MISMATCH_MESSAGE
#define  PHV_TLB_PAGE_SIZE_MISMATCH_MESSAGE PHV_X64_TLB_PAGE_SIZE_MISMATCH_MESSAGE

//
// Define HLT message.
//
typedef struct _HV_X64_HALT_MESSAGE
{
    HV_INTERCEPT_MESSAGE_HEADER Header;
} HV_X64_HALT_MESSAGE, *PHV_X64_HALT_MESSAGE;

//
// Define interruptibility notification message
//
typedef struct _HV_X64_INTERRUPTION_DELIVERABLE_MESSAGE
{
    HV_INTERCEPT_MESSAGE_HEADER Header;
    HV_X64_PENDING_INTERRUPTION_TYPE DeliverableType;
    UINT32 Rsvd;
} HV_X64_INTERRUPTION_DELIVERABLE_MESSAGE, *PHV_X64_INTERRUPTION_DELIVERABLE_MESSAGE;

//
// Define SIPI intercept message.
//
typedef struct _HV_X64_SIPI_INTERCEPT_MESSAGE
{
    HV_INTERCEPT_MESSAGE_HEADER Header;
    HV_VP_INDEX TargetVpIndex;
    HV_INTERRUPT_VECTOR Vector;
} HV_X64_SIPI_INTERCEPT_MESSAGE, *PHV_X64_SIPI_INTERCEPT_MESSAGE;

//
// Definition of the HvCallRegisterInterceptResult hypercall input structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_REGISTER_X64_CPUID_RESULT_PARAMETERS
{
    struct
    {
        UINT32 Eax;
        UINT32 Ecx;
        BOOLEAN SubleafSpecific;
        BOOLEAN AlwaysOverride;
    } Input;

    struct
    {
        UINT32 Eax;
        UINT32 EaxMask;
        UINT32 Ebx;
        UINT32 EbxMask;
        UINT32 Ecx;
        UINT32 EcxMask;
        UINT32 Edx;
        UINT32 EdxMask;
    } Result;
} HV_REGISTER_X64_CPUID_RESULT_PARAMETERS,
  *PHV_REGISTER_X64_CPUID_RESULT_PARAMETERS;

//
// Definition of the HvCallUnregisterInterceptResult hypercall input structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_UNREGISTER_X64_CPUID_RESULT_PARAMETERS
{
    UINT32 Eax;
    UINT32 Ecx;
    BOOLEAN SubleafSpecific;
} HV_UNREGISTER_X64_CPUID_RESULT_PARAMETERS, *PHV_UNREGISTER_X64_CPUID_RESULT_PARAMETERS;

#define HV_UNREGISTER_X64_CPUID_RESULT_PARAMETERS HV_UNREGISTER_X64_CPUID_RESULT_PARAMETERS
#define PHV_UNREGISTER_X64_CPUID_RESULT_PARAMETERS PHV_UNREGISTER_X64_CPUID_RESULT_PARAMETERS


typedef union HV_CALL_ATTRIBUTES _HV_REGISTER_INTERCEPT_RESULT_PARAMETERS
{
    HV_REGISTER_X64_CPUID_RESULT_PARAMETERS Cpuid;
} HV_REGISTER_INTERCEPT_RESULT_PARAMETERS, *PHV_REGISTER_INTERCEPT_RESULT_PARAMETERS;

typedef union HV_CALL_ATTRIBUTES _HV_UNREGISTER_INTERCEPT_RESULT_PARAMETERS
{
    HV_UNREGISTER_X64_CPUID_RESULT_PARAMETERS Cpuid;
} HV_UNREGISTER_INTERCEPT_RESULT_PARAMETERS, *PHV_UNREGISTER_INTERCEPT_RESULT_PARAMETERS;

#endif

//
// Define GPA attribute intercept message structure.
//

#define HV_GPA_ATTRIBUTE_INTERCEPT_MAX_RANGES 29

typedef struct _HV_GPA_ATTRIBUTE_INTERCEPT_MESSAGE
{
    HV_VP_INDEX VpIndex;
    struct
    {
        UINT32 RangeCount : 5;
        UINT32 Adjust : 1;
        UINT32 HostVisibility : 2;
        UINT32 MemoryType : 6;
        UINT32 Reserved : 18;
    };
    HV_GPA_PAGE_RANGE Ranges[HV_GPA_ATTRIBUTE_INTERCEPT_MAX_RANGES];
} HV_GPA_ATTRIBUTE_INTERCEPT_MESSAGE, *PHV_GPA_ATTRIBUTE_INTERCEPT_MESSAGE;

C_ASSERT(sizeof(HV_GPA_ATTRIBUTE_INTERCEPT_MESSAGE) <= HV_MESSAGE_PAYLOAD_BYTE_COUNT);


//
// Hypercall structures, enumerations, and constants.
// ==================================================
//

//
// Partition scheduling property ranges
//
#define HvPartitionPropertyMinimumCpuReserve    (0 << 16)
#define HvPartitionPropertyMaximumCpuReserve    (1 << 16)
#define HvPartitionPropertyMinimumCpuCap        (0 << 16)
#define HvPartitionPropertyMaximumCpuCap        (1 << 16)
#define HvPartitionPropertyMinimumCpuWeight     1
#define HvPartitionPropertyMaximumCpuWeight     10000

//
// Partition Creation Flags.
//

#define HV_PARTITION_CREATION_FLAG_SMT_ENABLED_GUEST                (1UI64 << 0)
#define HV_PARTITION_CREATION_FLAG_NESTED_VIRTUALIZATION_CAPABLE    (1UI64 << 1)
#define HV_PARTITION_CREATION_FLAG_SGX_ENABLED                      (1UI64 << 2)
#define HV_PARTITION_CREATION_FLAG_GPA_LARGE_PAGES_DISABLED         (1UI64 << 3)
#define HV_PARTITION_CREATION_FLAG_GPA_SUPER_PAGES_ENABLED          (1UI64 << 4)
#define HV_PARTITION_CREATION_FLAG_HOST_SYNCED_TIME                 (1UI64 << 5)
#define HV_PARTITION_CREATION_FLAG_VP_OVERCOMMIT_ALLOWED            (1UI64 << 6)
#define HV_PARTITION_CREATION_FLAG_ACCESS_ROOT_SCHEDULER_ALLOWED    (1UI64 << 7)
#define HV_PARTITION_CREATION_FLAG_EXO_PARTITION                    (1UI64 << 8)
#define HV_PARTITION_CREATION_FLAG_VTL1_DISABLED                    (1UI64 << 9)
#define HV_PARTITION_CREATION_FLAG_VTL2_DISABLED                    (1UI64 << 10)
#define HV_PARTITION_CREATION_FLAG_ISOLATED                         (1UI64 << 11)


//
// Partition properties specified at creation.
//

typedef struct _HV_PARTITION_CREATION_PROPERTIES
{
    HV_PARTITION_PROCESSOR_FEATURES DisabledProcessorFeatures;
    HV_PARTITION_PROCESSOR_XSAVE_FEATURES DisabledProcessorXsaveFeatures;
} HV_PARTITION_CREATION_PROPERTIES, *PHV_PARTITION_CREATION_PROPERTIES;

//
// Declare the input and output structures for the HvCreatePartition hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_CREATE_PARTITION
{
    UINT64 Flags;
    HV_PROXIMITY_DOMAIN_INFO ProximityDomainInfo;
    HV_COMPATIBILITY_VERSION CompatibilityVersion;
    HV_PARTITION_CREATION_PROPERTIES Properties;
} HV_INPUT_CREATE_PARTITION, *PHV_INPUT_CREATE_PARTITION;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_CREATE_PARTITION
{
    HV_PARTITION_ID NewPartitionId;
} HV_OUTPUT_CREATE_PARTITION, *PHV_OUTPUT_CREATE_PARTITION;

//
// Declare the input structure for the HvDeletePartition hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_DELETE_PARTITION
{
    HV_PARTITION_ID PartitionId;
} HV_INPUT_DELETE_PARTITION, *PHV_INPUT_DELETE_PARTITION;

//
// Declare the input structure for the HvFinalizePartition hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_FINALIZE_PARTITION
{
    HV_PARTITION_ID PartitionId;
} HV_INPUT_FINALIZE_PARTITION, *PHV_INPUT_FINALIZE_PARTITION;

//
// Declare the input structure for the HvInitializePartition hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_INITIALIZE_PARTITION
{
    HV_PARTITION_ID PartitionId;
} HV_INPUT_INITIALIZE_PARTITION, *PHV_INPUT_INITIALIZE_PARTITION;

//
// Declare the input structure for the HvScrubPartition hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SCRUB_PARTITION
{
    HV_PARTITION_ID PartitionId;
} HV_INPUT_SCRUB_PARTITION, *PHV_INPUT_SCRUB_PARTITION;

//
// Declare the input and output structures for the HvGetPartitionProperty
// hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_PARTITION_PROPERTY
{
    HV_PARTITION_ID             PartitionId;
    HV_PARTITION_PROPERTY_CODE  PropertyCode;

} HV_INPUT_GET_PARTITION_PROPERTY, *PHV_INPUT_GET_PARTITION_PROPERTY;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_PARTITION_PROPERTY
{
    HV_PARTITION_PROPERTY       PropertyValue;

} HV_OUTPUT_GET_PARTITION_PROPERTY, *PHV_OUTPUT_GET_PARTITION_PROPERTY;

//
// Declare the input and output structures for the
// HvGetNextChildPartition hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_NEXT_CHILD_PARTITION
{
    HV_PARTITION_ID ParentId;
    HV_PARTITION_ID PreviousChildId;

} HV_INPUT_GET_NEXT_CHILD_PARTITION, *PHV_INPUT_GET_NEXT_CHILD_PARTITION;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_NEXT_CHILD_PARTITION
{
    HV_PARTITION_ID NextChildId;

} HV_OUTPUT_GET_NEXT_CHILD_PARTITION, *PHV_OUTPUT_GET_NEXT_CHILD_PARTITION;

//
// SGX launch control configuration.
//

typedef union _HV_SGX_LAUNCH_CONTROL_CONFIG
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 Readable : 1;
        UINT64 Writable : 1;
        UINT64 Reserved : 62;
    };

} HV_SGX_LAUNCH_CONTROL_CONFIG, *PHV_SGX_LAUNCH_CONTROL_CONFIG;

C_ASSERT(sizeof(HV_SGX_LAUNCH_CONTROL_CONFIG) == sizeof(UINT64));

//
// Partition save&restore definitions.
//

//
// Definition of the HvCallWithdrawMemory hypercall input and output
// structures.  This call withdraws memory from a child partition's
// memory pool.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_WITHDRAW_MEMORY
{
    //
    // Supplies the partition ID of the child partition from which the
    // memory should be withdrawn.
    //
    HV_PARTITION_ID PartitionId;

    //
    // Supplies the proximity domain from which the memory should be
    // allocated.
    //
    HV_PROXIMITY_DOMAIN_INFO ProximityDomainInfo;

} HV_INPUT_WITHDRAW_MEMORY, *PHV_INPUT_WITHDRAW_MEMORY;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_WITHDRAW_MEMORY
{
    //
    // Returns the GPA of the memory withdrawn.
    //
    HV_GPA_PAGE_NUMBER GpaPageList[];

} HV_OUTPUT_WITHDRAW_MEMORY, *PHV_OUTPUT_WITHDRAW_MEMORY;

//
// Definition of the HvCallTranslateVirtualAddress hypercall input and
// output structures.  This call translates a GVA to a GPA.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_TRANSLATE_VIRTUAL_ADDRESS
{
    //
    // Supplies the partition ID of the partition in which the
    // translation should take place.
    //

    HV_PARTITION_ID PartitionId;

    //
    // Supplies the virtual processor whose GVA space is to be
    // accessed.
    //

    HV_VP_INDEX VpIndex;

    //
    // Supplies the control flags governing the access.
    //

    HV_TRANSLATE_GVA_CONTROL_FLAGS ControlFlags;

    //
    // Supplies the GVA page number to translate.
    //

    HV_GVA_PAGE_NUMBER GvaPage;

} HV_INPUT_TRANSLATE_VIRTUAL_ADDRESS, *PHV_INPUT_TRANSLATE_VIRTUAL_ADDRESS;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_TRANSLATE_VIRTUAL_ADDRESS
{
    //
    // Flags to indicate the disposition of the translation.
    //

    HV_TRANSLATE_GVA_RESULT TranslationResult;

    //
    // The GPA to which the GVA translated.
    //

    HV_GPA_PAGE_NUMBER GpaPage;

} HV_OUTPUT_TRANSLATE_VIRTUAL_ADDRESS, *PHV_OUTPUT_TRANSLATE_VIRTUAL_ADDRESS;

typedef struct _HV_OUTPUT_TRANSLATE_VIRTUAL_ADDRESS_EX
{
    //
    // Flags to indicate the disposition of the translation.
    //

    HV_TRANSLATE_GVA_RESULT_EX TranslationResult;

    //
    // The GPA to which the GVA translated.
    //

    HV_GPA_PAGE_NUMBER GpaPage;

} HV_OUTPUT_TRANSLATE_VIRTUAL_ADDRESS_EX, *PHV_OUTPUT_TRANSLATE_VIRTUAL_ADDRESS_EX;

//
// Definition of the HvCallReadGpa hypercall input and output
// structures.  This call reads from the indicated GPA.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_READ_GPA
{
    //
    // Supplies the partition ID of the partition whose GPA space is
    // to be read.
    //

    HV_PARTITION_ID PartitionId;

    //
    // Supplies the virtual processor whose GPA space is to be read
    // (virtual processor GPA spaces may differ, due to overlays).
    //

    HV_VP_INDEX VpIndex;

    //
    // Supplies the number of bytes to read.
    //

    UINT32 ByteCount;

    //
    // Supplies the start of the GPA range to read.
    //

    HV_GPA BaseGpa;

    //
    // Supplies the control flags governing the read.
    //

    HV_ACCESS_GPA_CONTROL_FLAGS ControlFlags;

} HV_INPUT_READ_GPA, *PHV_INPUT_READ_GPA;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_READ_GPA
{
    //
    // Flags to indicate the disposition of the read.
    //

    HV_ACCESS_GPA_RESULT AccessResult;

    //
    // The data which was read.
    //

    UINT8 Data[16];

} HV_OUTPUT_READ_GPA, *PHV_OUTPUT_READ_GPA;

//
// Definition of the HvCallWriteGpa hypercall input and output
// structures.  This call writes from the indicated GPA.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_WRITE_GPA
{
    //
    // Supplies the partition ID of the partition whose GPA space is
    // to be written.
    //

    HV_PARTITION_ID PartitionId;

    //
    // Supplies the virtual processor whose GPA space is to be written
    // (virtual processor GPA spaces may differ, due to overlays).
    //

    HV_VP_INDEX VpIndex;

    //
    // Supplies the number of bytes to write.
    //

    UINT32 ByteCount;

    //
    // Supplies the start of the GPA range to write.
    //

    HV_GPA BaseGpa;

    //
    // Supplies the control flags governing the write.
    //

    HV_ACCESS_GPA_CONTROL_FLAGS ControlFlags;

    //
    // Supplies the data to write.
    //

    UINT8 Data[16];

} HV_INPUT_WRITE_GPA, *PHV_INPUT_WRITE_GPA;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_WRITE_GPA
{
    //
    // Flags to indicate the disposition of the write.
    //

    HV_ACCESS_GPA_RESULT AccessResult;

} HV_OUTPUT_WRITE_GPA, *PHV_OUTPUT_WRITE_GPA;


#if !defined(_ARM64_)

//
// Definition of the HvCallRegisterInterceptResult hypercall input structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_REGISTER_INTERCEPT_RESULT
{
    HV_PARTITION_ID PartitionId;
    HV_VP_INDEX VpIndex;
    HV_INTERCEPT_TYPE InterceptType;
    HV_REGISTER_INTERCEPT_RESULT_PARAMETERS Parameters;
} HV_INPUT_REGISTER_INTERCEPT_RESULT, *PHV_INPUT_REGISTER_INTERCEPT_RESULT;

//
// Definition of the HvCallUnregisterInterceptResult hypercall input structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_UNREGISTER_INTERCEPT_RESULT
{
    HV_PARTITION_ID PartitionId;
    HV_VP_INDEX VpIndex;
    HV_INTERCEPT_TYPE InterceptType;
    HV_UNREGISTER_INTERCEPT_RESULT_PARAMETERS Parameters;
} HV_INPUT_UNREGISTER_INTERCEPT_RESULT, *PHV_INPUT_UNREGISTER_INTERCEPT_RESULT;

#endif

//
// Definition of the HvCallDeleteVp hypercall input structure.
// This call deletes a virtual processor.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_DELETE_VP
{
    HV_PARTITION_ID PartitionId;
    HV_VP_INDEX     VpIndex;
} HV_INPUT_DELETE_VP, *PHV_INPUT_DELETE_VP;

//
// Definition of the HvAssertVirtualInterrupt hypercall input
// structure.  This call asserts an interrupt in a guest partition.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_ASSERT_VIRTUAL_INTERRUPT
{
    HV_PARTITION_ID         TargetPartition;
    HV_INTERRUPT_CONTROL    InterruptControl;
    UINT64                  DestinationAddress;
    HV_INTERRUPT_VECTOR     RequestedVector;
    HV_VTL                  TargetVtl;
    UINT8                   ReservedZ0;
    UINT16                  ReservedZ1;
} HV_INPUT_ASSERT_VIRTUAL_INTERRUPT, *PHV_INPUT_ASSERT_VIRTUAL_INTERRUPT;

//
// Definition of the HvClearVirtualInterrupt hypercall input
// structure. This call clears the acknowledged status of a previously
// acknowledged vector.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_CLEAR_VIRTUAL_INTERRUPT
{
    HV_PARTITION_ID         TargetPartition;
} HV_INPUT_CLEAR_VIRTUAL_INTERRUPT, *PHV_INPUT_CLEAR_VIRTUAL_INTERRUPT;

//
// Definition of the HvCreatePort hypercall input structure.  This
// call allocates a port object.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_CREATE_PORT
{
    HV_PARTITION_ID             PortPartition;
    HV_PORT_ID                  PortId;
    HV_VTL                      PortVtl;
    HV_VTL                      MinConnectionVtl;
    UINT16                      ReservedZ0;
    HV_PARTITION_ID             ConnectionPartition;
    HV_PORT_INFO                PortInfo;
    HV_PROXIMITY_DOMAIN_INFO    ProximityDomainInfo;
} HV_INPUT_CREATE_PORT, *PHV_INPUT_CREATE_PORT;

//
// Definition of the HvDeletePort hypercall input structure.  This
// call deletes a port object.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_DELETE_PORT
{
    HV_PARTITION_ID PortPartition;
    HV_PORT_ID      PortId;
    UINT32          Reserved;
} HV_INPUT_DELETE_PORT, *PHV_INPUT_DELETE_PORT;

//
// Definition of the HvConnectPort hypercall input structure.  This
// call creates a connection to a previously-created port in another
// partition.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_CONNECT_PORT
{
    HV_PARTITION_ID             ConnectionPartition;
    HV_CONNECTION_ID            ConnectionId;
    HV_VTL                      ConnectionVtl;
    UINT8                       ReservedZ0;
    UINT16                      ReservedZ1;
    HV_PARTITION_ID             PortPartition;
    HV_PORT_ID                  PortId;
    UINT32                      ReservedZ2;
    HV_CONNECTION_INFO          ConnectionInfo;
    HV_PROXIMITY_DOMAIN_INFO    ProximityDomainInfo;
} HV_INPUT_CONNECT_PORT, *PHV_INPUT_CONNECT_PORT;

//
// Definition of the HvGetPortProperty hypercall input and output
// structures.  This call retrieves a property of a previously-created
// port in the current or another partition.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_PORT_PROPERTY
{
    HV_PARTITION_ID       PortPartition;
    HV_PORT_ID            PortId;
    UINT32                Reserved;
    HV_PORT_PROPERTY_CODE PropertyCode;
} HV_INPUT_GET_PORT_PROPERTY, *PHV_INPUT_GET_PORT_PROPERTY;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_PORT_PROPERTY
{
    HV_PORT_PROPERTY PropertyValue;
} HV_OUTPUT_GET_PORT_PROPERTY, *PHV_OUTPUT_GET_PORT_PROPERTY;

//
// Definition of the HvSetPortProperty hypercall input structure.
// This call sets a property of a previously-created port in the
// current or another partition.
//
typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SET_PORT_PROPERTY
{
    HV_PARTITION_ID       PortPartition;
    HV_PORT_ID            PortId;
    UINT32                Reserved;
    HV_PORT_PROPERTY_CODE PropertyCode;
    HV_PORT_PROPERTY      PropertyValue;
} HV_INPUT_SET_PORT_PROPERTY, *PHV_INPUT_SET_PORT_PROPERTY;

//
// Definition of the HvDisconnectPort hypercall input structure.  This
// call disconnects an existing connection.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_DISCONNECT_PORT
{
    HV_PARTITION_ID     ConnectionPartition;
    HV_CONNECTION_ID    ConnectionId;
    UINT32              Reserved;
} HV_INPUT_DISCONNECT_PORT, *PHV_INPUT_DISCONNECT_PORT;

//
// Definition of the HvNotifyEventRingEmpty hypercall input structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_NOTIFY_EVENT_RING_EMPTY
{
    HV_SYNIC_SINT_INDEX SintIndex;
    UINT32              RsvdZ;
} HV_INPUT_NOTIFY_EVENT_RING_EMPTY, *PHV_INPUT_NOTIFY_EVENT_RING_EMPTY;

//
// Definition of the HvSavePartitionState hypercall input and output
// structures.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SAVE_PARTITION_STATE
{
    HV_PARTITION_ID PartitionId;
    HV_SAVE_RESTORE_STATE_FLAGS Flags;
} HV_INPUT_SAVE_PARTITION_STATE, *PHV_INPUT_SAVE_PARTITION_STATE;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_SAVE_PARTITION_STATE
{
    UINT32 SaveDataCount;
    HV_SAVE_RESTORE_STATE_RESULT SaveState;
    UINT8 SaveData[4080];
} HV_OUTPUT_SAVE_PARTITION_STATE, *PHV_OUTPUT_SAVE_PARTITION_STATE;

//
// Definition of the HvRestorePartitionState hypercall input and
// output structures.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_RESTORE_PARTITION_STATE
{
    HV_PARTITION_ID PartitionId;
    HV_SAVE_RESTORE_STATE_FLAGS Flags;
    UINT32 RestoreDataCount;
    UINT8 RestoreData[4080];
} HV_INPUT_RESTORE_PARTITION_STATE, *PHV_INPUT_RESTORE_PARTITION_STATE;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_RESTORE_PARTITION_STATE
{
    HV_SAVE_RESTORE_STATE_RESULT RestoreState;
    UINT32 RestoreDataConsumed;
} HV_OUTPUT_RESTORE_PARTITION_STATE, *PHV_OUTPUT_RESTORE_PARTITION_STATE;

//
// Definition of the HcpHvParkedVirtualProcessors hypercall input structure.
// This call notifies the hypervisor of the set of virtual processors that
// the root partition decides to park.
//
typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_PARKED_VIRTUAL_PROCESSORS
{
    UINT64 ProcessorMask;
} HV_INPUT_PARKED_VIRTUAL_PROCESSORS, *PHV_INPUT_PARKED_VIRTUAL_PROCESSORS;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_INJECT_SYNTHETIC_MACHINE_CHECK
{
    HV_PARTITION_ID PartitionId;
    HV_VP_INDEX VpIndex;
    HV_SYNMC_EVENT SynmcEvent;
} HV_INPUT_INJECT_SYNTHETIC_MACHINE_CHECK, *PHV_INPUT_INJECT_SYNTHETIC_MACHINE_CHECK;

//
// Definition of the HvCallCheckForIoIntercept hypercall input and output
// structures.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_CHECK_FOR_IO_INTERCEPT
{
    HV_PARTITION_ID PartitionId;
    HV_VP_INDEX VpIndex;
    HV_INPUT_VTL TargetVtl;
    HV_IO_PORT Port;
    UINT8 Size;
    BOOLEAN IsWrite;

} HV_INPUT_CHECK_FOR_IO_INTERCEPT, *PHV_INPUT_CHECK_FOR_IO_INTERCEPT;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_CHECK_FOR_IO_INTERCEPT
{
    BOOLEAN Intercept;

} HV_OUTPUT_CHECK_FOR_IO_INTERCEPT, *PHV_OUTPUT_CHECK_FOR_IO_INTERCEPT;

//
// Definition of the HvCallFlushGuestPhysicalAddressSpace and
// HvCallFlushGuestPhysicalAddressList hypercall input structures.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_FLUSH_GUEST_PHYSICAL_ADDRESS_SPACE
{
    HV_SPA AddressSpace;
    UINT64 Flags;

} HV_INPUT_FLUSH_GUEST_PHYSICAL_ADDRESS_SPACE, *PHV_INPUT_FLUSH_GUEST_PHYSICAL_ADDRESS_SPACE;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_FLUSH_GUEST_PHYSICAL_ADDRESS_LIST
{
    HV_SPA AddressSpace;
    UINT64 Flags;
    HV_CALL_ATTRIBUTES HV_GPA_PAGE_RANGE GpaRangeList[];

} HV_INPUT_FLUSH_GUEST_PHYSICAL_ADDRESS_LIST, *PHV_INPUT_FLUSH_GUEST_PHYSICAL_ADDRESS_LIST;

//
// Definition of the HvRegisterInstructionEmulationHints register.
//

typedef union _HV_INSTRUCTION_EMULATION_HINTS_REGISTER
{
    UINT64 AsUINT64;

    struct
    {
        //
        // Indicates whether any secure VTL is enabled for the partition.
        //

        UINT64 PartitionSecureVtlEnabled : 1;

        //
        // Indicates whether kernel or user execute control architecturally
        // applies to execute accesses.
        //

        UINT64 MbecUserExecuteControl : 1;

#if defined(_AMD64_)

        //
        // Indicates whether MPX is enabled with BNDPRESERVE=0 (near jump
        // instructions without BND/REPNE prefix reset bound registers).
        //

        UINT64 MpxEnabledNoPreserve : 1;

#endif

    };

} HV_INSTRUCTION_EMULATION_HINTS_REGISTER, *PHV_INSTRUCTION_EMULATION_HINTS_REGISTER;

//
// Definitions for HvCallGetInterceptData.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_INTERCEPT_DATA
{
    //
    // Supplies the partition ID of the child partition.
    //
    HV_PARTITION_ID PartitionId;

    //
    // Supplies the index of the virtual processor that generated an intercept.
    //
    HV_VP_INDEX VpIndex;

} HV_INPUT_GET_INTERCEPT_DATA, *PHV_INPUT_GET_INTERCEPT_DATA;

//
// Definition of the HvCallCompleteIntercept hypercall input structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_COMPLETE_INTERCEPT
{
    //
    // Supplies the partition ID of the child partition.
    //
    HV_PARTITION_ID PartitionId;

    //
    // Supplies the index of the virtual processor that generated an intercept.
    //
    HV_VP_INDEX VpIndex;

    //
    // Supplies the size of the intercept result.
    //
    UINT32 DataSize;

    //
    // Supplies the intercept result.
    //
    UINT8 Data[];

} HV_INPUT_COMPLETE_INTERCEPT, *PHV_INPUT_COMPLETE_INTERCEPT;

#define HV_COMPLETE_INTERCEPT_MAX_DATA_SIZE (HV_PAGE_SIZE - sizeof(HV_INPUT_COMPLETE_INTERCEPT))

//
// Define hypercall intercept completion data.
//

typedef struct _HV_HYPERCALL_INTERCEPT_COMPLETION_DATA
{
    HV_STATUS Status;
    UINT8 OutputData[];
} HV_HYPERCALL_INTERCEPT_COMPLETION_DATA, *PHV_HYPERCALL_INTERCEPT_COMPLETION_DATA;

#define HV_HYPERCALL_INTERCEPT_COMPLETION_MAX_OUTPUT_DATA_SIZE \
    (HV_COMPLETE_INTERCEPT_MAX_DATA_SIZE - sizeof(HV_HYPERCALL_INTERCEPT_COMPLETION_DATA))

//
// Define GPA attribute intercept completion data.
//

typedef struct _HV_GPA_ATTRIBUTE_INTERCEPT_COMPLETION_DATA
{
    HV_STATUS Status;
} HV_GPA_ATTRIBUTE_INTERCEPT_COMPLETION_DATA, *PHV_GPA_ATTRIBUTE_INTERCEPT_COMPLETION_DATA;

//
// Definition of the HvCallUpdateHvProcessorFeatures input structures.
//

#if defined(_AMD64_) || defined(_X86_)

typedef union _HV_DETECT_X64_PROCESSOR_FEATURE_LIST
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 Btb      : 1;
        UINT64 RsvdZ    : 63;
    };

} HV_DETECT_X64_PROCESSOR_FEATURE_LIST, *PHV_DETECT_X64_PROCESSOR_FEATURE_LIST;

typedef enum _HV_X64_UPDATE_PROCESSOR_FEATURES_PROPERTY
{
    HvX64UpdateProcessorFeaturesDetect          = 0,
    HvX64UpdateProcessorFeaturesHvIbrsValue     = 1,
    HvX64UpdateProcessorFeaturesIbpbFlushRoot   = 2,
    HvX64UpdateProcessorFeaturesIbpbFlushGuest  = 3,

} HV_X64_UPDATE_PROCESSOR_FEATURES_PROPERTY,
  *PHV_X64_UPDATE_PROCESSOR_FEATURES_PROPERTY;

#define _HV_UPDATE_PROCESSOR_FEATURES_PROPERTY _HV_X64_UPDATE_PROCESSOR_FEATURES_PROPERTY
#define HV_UPDATE_PROCESSOR_FEATURES_PROPERTY HV_X64_UPDATE_PROCESSOR_FEATURES_PROPERTY
#define PHV_UPDATE_PROCESSOR_FEATURES_PROPERTY PHV_X64_UPDATE_PROCESSOR_FEATURES_PROPERTY

#else

#define _HV_UPDATE_PROCESSOR_FEATURES_PROPERTY UINT32
#define HV_UPDATE_PROCESSOR_FEATURES_PROPERTY UINT32
#define PHV_UPDATE_PROCESSOR_FEATURES_PROPERTY PUINT32

#endif

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_UPDATE_PROCESSOR_FEATURES
{
    HV_UPDATE_PROCESSOR_FEATURES_PROPERTY Property;
    UINT32 RsvdZ;
    UINT64 Value;

} HV_INPUT_UPDATE_PROCESSOR_FEATURES, *PHV_INPUT_UPDATE_PROCESSOR_FEATURES;


//
// Hypercall structures, enumerations, and constants.
// ==================================================
//

typedef UINT64 HV_CPU_GROUP_ID, *PHV_CPU_GROUP_ID;

//
// Define invalid CPU group identifier
//
#define HV_CPU_GROUP_ID_INVALID ((HV_CPU_GROUP_ID) 0x0)

//
// CPU Group Properties
//
typedef UINT64 HV_CPU_GROUP_PROPERTY, *PHV_CPU_GROUP_PROPERTY;

typedef enum
{
    //
    // Scheduling control properties
    //
    HvCpuGroupPropertyCpuCap                       = 0x00010000,

} HV_CPU_GROUP_PROPERTY_CODE, *PHV_CPU_GROUP_PROPERTY_CODE;

//
// Bitset of logical processor indexes.
//

#define CPU_SET_SHIFT 6
#define CPU_SET_MASK 63
#define CPU_SET_QWORD_COUNT (((HV_MAXIMUM_PROCESSORS - 1) >> CPU_SET_SHIFT) + 1)

typedef struct _HV_LOGICAL_PROCESSOR_BITSET
{
    UINT64 ProcessorSet[CPU_SET_QWORD_COUNT];

} HV_LOGICAL_PROCESSOR_BITSET, *PHV_LOGICAL_PROCESSOR_BITSET;

//
// Input and output structures for the HvCreateCpuGroup hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_CREATE_CPU_GROUP
{
    HV_LOGICAL_PROCESSOR_BITSET CpuGroupAffinity;

} HV_INPUT_CREATE_CPU_GROUP, *PHV_INPUT_CREATE_CPU_GROUP;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_CREATE_CPU_GROUP
{
    HV_CPU_GROUP_ID NewCpuGroupId;

} HV_OUTPUT_CREATE_CPU_GROUP, *PHV_OUTPUT_CREATE_CPU_GROUP;

//
// Input structure for the HvDeleteCpuGroup hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_DELETE_CPU_GROUP
{
    HV_CPU_GROUP_ID CpuGroupId;

} HV_INPUT_DELETE_CPU_GROUP, *PHV_INPUT_DELETE_CPU_GROUP;

//
// Input and output structures for the HvGetCpuGroupProperty hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_CPU_GROUP_PROPERTY
{
    HV_CPU_GROUP_ID CpuGroupId;
    HV_CPU_GROUP_PROPERTY_CODE PropertyCode;

} HV_INPUT_GET_CPU_GROUP_PROPERTY, *PHV_INPUT_GET_CPU_GROUP_PROPERTY;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_CPU_GROUP_PROPERTY
{
    HV_CPU_GROUP_PROPERTY PropertyValue;

} HV_OUTPUT_GET_CPU_GROUP_PROPERTY, *PHV_OUTPUT_GET_CPU_GROUP_PROPERTY;

//
// Input structure for the HvSetCpuGroupProperty hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SET_CPU_GROUP_PROPERTY
{
    HV_CPU_GROUP_ID CpuGroupId;
    HV_CPU_GROUP_PROPERTY_CODE PropertyCode;
    HV_CPU_GROUP_PROPERTY PropertyValue;

} HV_INPUT_SET_CPU_GROUP_PROPERTY, *PHV_INPUT_SET_CPU_GROUP_PROPERTY;

//
// Input and output structures for the HvGetCpuGroupAffinity hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_CPU_GROUP_AFFINITY
{
    HV_CPU_GROUP_ID CpuGroupId;

} HV_INPUT_GET_CPU_GROUP_AFFINITY, *PHV_INPUT_GET_CPU_GROUP_AFFINITY;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_CPU_GROUP_AFFINITY
{
    HV_LOGICAL_PROCESSOR_BITSET CpuGroupAffinity;
    UINT32 LpCount;

} HV_OUTPUT_GET_CPU_GROUP_AFFINITY, *PHV_OUTPUT_GET_CPU_GROUP_AFFINITY;

//
// Input and output structures for the HvGetNextCpuGroup hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_NEXT_CPU_GROUP
{
    HV_CPU_GROUP_ID PreviousCpuGroupId;

} HV_INPUT_GET_NEXT_CPU_GROUP, *PHV_INPUT_GET_NEXT_CPU_GROUP;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_NEXT_CPU_GROUP
{
    HV_CPU_GROUP_ID NextCpuGroupId;

} HV_OUTPUT_GET_NEXT_CPU_GROUP, *PHV_OUTPUT_GET_NEXT_CPU_GROUP;

//
// Input and output structures for the HvGetNextCpuGroupPartition hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_NEXT_CPU_GROUP_PARTITION
{
    HV_CPU_GROUP_ID CpuGroupId;
    HV_PARTITION_ID PreviousPartitionId;

} HV_INPUT_GET_NEXT_CPU_GROUP_PARTITION, *PHV_INPUT_GET_NEXT_CPU_GROUP_PARTITION;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_NEXT_CPU_GROUP_PARTITION
{
    HV_PARTITION_ID NextPartitionId;

} HV_OUTPUT_GET_NEXT_CPU_GROUP_PARTITION, *PHV_OUTPUT_GET_NEXT_CPU_GROUP_PARTITION;


#define HV_MESSAGE_TYPE_SCHEDULER_GROUP_ID \
                (UINT32)HvMessageTypeSchedulerIdRangeStart

#define HV_MESSAGE_TYPE_SCHEDULER_GROUP_ID_MASK 0xFFFFFF00

C_ASSERT(((UINT32)HvMessageTypeSchedulerIdRangeEnd &
                HV_MESSAGE_TYPE_SCHEDULER_GROUP_ID_MASK) ==
          HV_MESSAGE_TYPE_SCHEDULER_GROUP_ID);

#define IS_X_SCHEDULER_MESSAGE_TYPE(messageType) \
    (((UINT32)messageType & HV_MESSAGE_TYPE_SCHEDULER_GROUP_ID_MASK) == \
                HV_MESSAGE_TYPE_SCHEDULER_GROUP_ID)

//
// Dispatch state for the VP communicated by the hypervisor to the VP-dispatch
// thread in the root on return from HvCallDispatchVp.
//
typedef enum _HV_VP_DISPATCH_STATE
{
    HvVpDispatchStateInvalid = 0,
    HvVpDispatchStateBlocked = 1,
    HvVpDispatchStateReady = 2,

} HV_VP_DISPATCH_STATE, *PHV_VP_DISPATCH_STATE;

//
// Dispatch event that caused the current dispatch state on return from
// HvCallDispatchVp.
//
// The following are the only valid combinations for dispatch states and
// events:
//
//      HvVpDispatchStateBlocked
//
//          HvVpDispatchEventSuspend
//          HvVpDispatchEventHalt
//          HvVpDispatchEventStartup
//          HvVpDispatchEventDelete
//          HvVpDispatchEventMachineCheck
//          HvVpDispatchEventIdle
//          HvVpDispatchEventSystem
//          HvVpDispatchEventPartition
//          HvVpDispatchEventTerminate
//          HvVpDispatchEventInternal
//
//      HvVpDispatchStateReady
//
//          HvVpDispatchEventIntercept
//          HvVpDispatchEventPreempted
//          HvVpDispatchEventCancelled
//          HvVpDispatchEventScheduler
//          HvVpDispatchEventLongSpinWait
//          HvVpDispatchEventTimeSliceEnd
//
typedef enum _HV_VP_DISPATCH_EVENT
{
    HvVpDispatchEventInvalid =          0x00000000,

    //
    // These event ids must match to VP_ACTIVITY_STATE suspend flags (e.g.,
    // the bit mask for each suspend flag is the id for the corresponding
    // event).
    //

    HvVpDispatchEventSuspend =          0x00000001,
    HvVpDispatchEventIntercept =        0x00000002,
    HvVpDispatchEventHalt =             0x00000004,
    HvVpDispatchEventStartup =          0x00000008,
    HvVpDispatchEventDelete =           0x00000020,
    HvVpDispatchEventMachineCheck =     0x00000040,
    HvVpDispatchEventIdle =             0x00000080,
    HvVpDispatchEventSystem =           0x00000100,
    HvVpDispatchEventPartition =        0x00000200, // GenericSuspend
    HvVpDispatchEventTerminate =        0x00004000, // XSchedulerDispatchSuspend

    //
    // These events do not have any correlation with VP_ACTIVITY_STATE
    // suspend flags.
    //

    HvVpDispatchEventInternal =         0x10000001,
    HvVpDispatchEventPreempted =        0x10000002,
    HvVpDispatchEventCancelled =        0x10000003,
    HvVpDispatchEventScheduler =        0x10000004,
    HvVpDispatchEventLongSpinWait =     0x10000005,
    HvVpDispatchEventTimeSliceEnd =     0x10000006,

} HV_VP_DISPATCH_EVENT, *PHV_VP_DISPATCH_EVENT;

//
// Event that caused an exit from WinHv's VP-dispatch loop.
//

typedef enum _HV_VP_DISPATCH_LOOP_EVENT
{
    HvVpDispatchLoopEventInvalid = 0,

    HvVpDispatchLoopEventIntercept = 1,
    HvVpDispatchLoopEventCancelled = 2,
    HvVpDispatchLoopEventTerminated = 3

} HV_VP_DISPATCH_LOOP_EVENT, *PHV_VP_DISPATCH_LOOP_EVENT;

//
// Result returned on exit from WinHv's VP-dispatch loop.
//

typedef union _HV_VP_DISPATCH_LOOP_RESULT
{
    UINT64 AsUINT64;

    struct
    {
        HV_VP_DISPATCH_LOOP_EVENT ExitEvent;
        UINT32                    Reserved;
    };
} HV_VP_DISPATCH_LOOP_RESULT, *PHV_VP_DISPATCH_LOOP_RESULT;

//
// Bitset of virtual processor indexes.
//

#define VP_BITSET_SHIFT       (6)
#define VP_BITSET_MASK        (63)
#define VP_BITSET_QWORD_COUNT (((HV_MAX_VPS_PER_PARTITION - 1) >> VP_BITSET_SHIFT) + 1)

typedef struct _HV_VIRTUAL_PROCESSOR_BITSET
{
    UINT64 VpSet[VP_BITSET_QWORD_COUNT];

} HV_VIRTUAL_PROCESSOR_BITSET, *PHV_VIRTUAL_PROCESSOR_BITSET;

//
// Signaling message for a single partition using the virtual processor bitset.
//

typedef struct _HV_VP_SIGNAL_BITSET_SCHEDULER_MESSAGE
{
    HV_PARTITION_ID PartitionId;
    UINT32 OverflowCount;
    UINT16 VpCount;
    HV_VIRTUAL_PROCESSOR_BITSET VpBitset;

} HV_VP_SIGNAL_BITSET_SCHEDULER_MESSAGE, *PHV_VP_SIGNAL_BITSET_SCHEDULER_MESSAGE;

C_ASSERT(sizeof(HV_VP_SIGNAL_BITSET_SCHEDULER_MESSAGE) <=
    (sizeof(HV_MESSAGE) - sizeof(HV_MESSAGE_HEADER)));

//
// Signaling message for multiple partitions using a partition and VP index pair.
//
// IMPORTANT: If the data structure gets changed, XmepGetSignalPairMessageSize()
//            must be checked and updated accordingly.
//

#define HV_MESSAGE_MAX_PARTITION_VP_PAIR_COUNT \
    (((sizeof(HV_MESSAGE) - sizeof(HV_MESSAGE_HEADER)) / \
      (sizeof(HV_PARTITION_ID) + sizeof(HV_VP_INDEX))) - 1)

typedef struct _HV_VP_SIGNAL_PAIR_SCHEDULER_MESSAGE
{
    UINT32 OverflowCount;
    UINT8 VpCount;
    UINT8 Reserved1[3];

    HV_PARTITION_ID PartitionIds[HV_MESSAGE_MAX_PARTITION_VP_PAIR_COUNT];
    HV_VP_INDEX VpIndexes[HV_MESSAGE_MAX_PARTITION_VP_PAIR_COUNT];

    UINT8 Reserved2[4];

} HV_VP_SIGNAL_PAIR_SCHEDULER_MESSAGE, *PHV_VP_SIGNAL_PAIR_SCHEDULER_MESSAGE;

C_ASSERT(sizeof(HV_VP_SIGNAL_PAIR_SCHEDULER_MESSAGE) ==
    (sizeof(HV_MESSAGE) - sizeof(HV_MESSAGE_HEADER)));

//
// Input and output structures for the HvDispatchVp hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_DISPATCH_VP
{
    HV_PARTITION_ID PartitionId;
    HV_VP_INDEX VpIndex;
    HV_NANO100_DURATION TimeSlice;
    BOOLEAN ClearInterceptSuspend;

} HV_INPUT_DISPATCH_VP, *PHV_INPUT_DISPATCH_VP;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_DISPATCH_VP
{
    HV_VP_DISPATCH_STATE DispatchState;
    HV_VP_DISPATCH_EVENT DispatchEvent;

} HV_OUTPUT_DISPATCH_VP, *PHV_OUTPUT_DISPATCH_VP;


#ifdef __cplusplus
}
#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4200)
#pragma warning(default:4201)
#pragma warning(default:4214)
#pragma warning(default:4324)
#endif

#endif //_HVHDK_
