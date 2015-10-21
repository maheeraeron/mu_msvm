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
//
// System physical addresses (SPAs) define the physical address space of the underlying
// hardware. There is only one system physical address space for the entire machine.
//
// Guest physical addresses (GPAs) define the guest's view of physical memory.
// GPAs can be mapped to underlying SPAs. There is one guest physical address space per
// partition.
//


typedef UINT64 HV_SPA, *PHV_SPA;
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
#define HV_LARGE_PAGE_MASK (HV_LARGE_PAGE_SIZE - 1)
#define HV_LARGE_PAGE_MASK_1GB (HV_LARGE_PAGE_SIZE_1GB - 1)

typedef UINT64 HV_SPA_PAGE_NUMBER, *PHV_SPA_PAGE_NUMBER;
typedef UINT64 HV_GPA_PAGE_NUMBER, *PHV_GPA_PAGE_NUMBER;

typedef const HV_SPA_PAGE_NUMBER *PCHV_SPA_PAGE_NUMBER;
typedef const HV_GPA_PAGE_NUMBER *PCHV_GPA_PAGE_NUMBER;

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

    StHvCounterMAXIMUM

} HV_HYPERVISOR_COUNTER;

#define HV_STATISTICS_GROUP_HVA_LENGTH 40
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

    StLpCounterMAXIMUM

} HV_CPU_COUNTER;

#define HV_STATISTICS_GROUP_LPA_LENGTH 184
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
    StPtCounterMAXIMUM

} HV_PROCESS_COUNTER;

#define HV_STATISTICS_GROUP_PTA_LENGTH 8
#define HV_STATISTICS_GROUP_PTV_LENGTH 184

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
    StVpCounterMAXIMUM

} HV_THREAD_COUNTER;

#define HV_STATISTICS_GROUP_VPA_LENGTH 32
#define HV_STATISTICS_GROUP_VPV_LENGTH 544

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
    StVpHcCounterGlobalReferenceTime = 10,
    StVpHcCounterMAXIMUM

} HV_THREAD_HYPERCALLBASED_COUNTER;



#define HV_X64_MSR_STATS_PARTITION_RETAIL_PAGE      0x400000E0
#if defined(_PERF_FEATURES_ENABLED_)
#define HV_X64_MSR_STATS_PARTITION_INTERNAL_PAGE    0x400000E1
#endif

#define HV_X64_MSR_STATS_VP_RETAIL_PAGE             0x400000E2
#if defined(_PERF_FEATURES_ENABLED_)
#define HV_X64_MSR_STATS_VP_INTERNAL_PAGE           0x400000E3
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



typedef union _HV_PICO100_DURATION
{
    UINT64 AsUINT64;
} HV_PICO100_DURATION;

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
    // Profile sources for all processors
    //
    HvProfileInvalid,
    HvProfileCyclesNotHalted = 1,
    HvProfileCacheMisses,
    HvProfileBranchMispredictions,
    
#ifdef PRERELEASE
    //
    // Profile sources for for AMD processors.
    //

    HvProfileDispatchedFPUOps = 64,
    HvProfileCyclesNoFPUOpsRetired,
    HvProfileDispathedFPUOpsWithFastFlag,
    HvProfileRetiredSSEOps,
    HvProfileRetiredMoveOps,
    HvProfileSegmentRegisterLoad,
    HvProfileResyncBySelfModifyingCode,
    HvProfileResyncBySnoop,
    HvProfileBuffer2Full,
    HvProfileLockedOperation,
    HvProfileRetiredCLFLUSH,
    HvProfileRetiredCPUID,
    HvProfileLSDispatch,
    HvProfileCancelledStoreToLoadFwdOperations,
    HvProfileSMIReceived,
    HvProfileDataAccess,
    HvProfileDataMiss,
    HvProfileDCRefillFromL2,
    HvProfileDCRefillFromSystem,
    HvProfileDCRefillCopyBack,
    HvProfileDCL1DTLBMissL2DTLBHit,
    HvProfileDCL1DTLBMissL2DTLBMiss,
    HvProfileDCMisalignedDataReference,
    HvProfileDCLateCancelOfAnAccess,
    HvProfileDCEarlyCancelOfAnAccess,
    HvProfileDCOneBitECCError,
    HvProfileDCDispatchedPrefetchInstructions,
    HvProfileDCacheMissByLockedInstructions,
    HvProfileL1DTLBHit,
    HvProfileL1DTLBReloadLatency,
    HvProfileIneffectiveSoftwarePrefetches,
    HvProfileGlobalTLBFlushes,
    HvProfileRetiredINVLPGAndINVLPGA,
    HvProfileMemoryAccessesToUC,
    HvProfileMemoryAccessesToWCAndWCBufferFlushToWB,
    HvProfileStreamStoreToWB,
    HvProfileDataPrefetchCancelled,
    HvProfileDataPrefetchAttempts,
    HvProfileMABRequests,
    HvProfileNBReadResponsesForForCacheRefill,
    HvProfileOctwordsWriitenToSystem,
    HvProfilePageTableWalkerPDPERefillHitInL2,
    HvProfilePageTableWalkerPDPELookupMissedInPDC,
    HvProfilePageTableWalkerPML4ERefillHitInL2,
    HvProfilePageTableWalkerPML4ELookupMissedInPDC,
    HvProfilePageTableWalkerPTERefillHitInL2,
    HvProfilePageTableWalkerPDERefillHitInL2,
    HvProfilePageTableWalkerPDELookupMissedInPDC,
    HvProfilePageTableWalkerPDELookupInPDC,
    HvProfileProbeHits,
    HvProfileCacheCrossInvalidates,
    HvProfileTLBFlushEvents,
    HvProfileL2Request,
    HvProfileL2CacheMisses,
    HvProfileL2CacheMissesICFill,
    HvProfileL2CacheMissesDCFill,
    HvProfileL2CacheMissesTLBPageTableWalk,
    HvProfileL2Fill,
    HvProfileICFetch,
    HvProfileICMiss,
    HvProfileICRefillFromL2,
    HvProfileICRefillFromSystem,
    HvProfileICL1TLBMissL2TLBHit,
    HvProfileICL1TLBMissL2TLBMiss,
    HvProfileICResyncBySnoop,
    HvProfileICInstructionFetchStall,
    HvProfileICReturnStackHit,
    HvProfileICReturnStackOverflow,
    HvProfileInstructionCacheVictims,
    HvProfileInstructionCacheLinesInvalidated,
    HvProfileITLBReloads,
    HvProfileITLBReloadsAborted,
    HvProfileRetiredInstructions,
    HvProfileRetireduops,
    HvProfileRetiredBranches,
    HvProfileRetiredBranchesMispredicted,
    HvProfileTakenBranches,
    HvProfileTakenBranchesMispredicted,
    HvProfileRetiredFarControlTransfers,
    HvProfileRetiredResyncsNonControlTransferBranches,
    HvProfileRetiredNearReturns,
    HvProfileRetiredNearReturnsMispredicted,
    HvProfileTakenBranchesMispredictedByAddressMiscompare,
    HvProfileRetiredFPUInstructions,
    HvProfileRetiredFastpathDoubleOpInstructions,
    HvProfileInterruptsMaskedCycles,
    HvProfileInterruptsMaskedWhilePendingCycles,
    HvProfileTakenHardwareInterrupts,
    HvProfileNothingToDispatch,
    HvProfileDispatchStalls,
    HvProfileDispatchStallsFromBranchAbortToRetire,
    HvProfileDispatchStallsForSerialization,
    HvProfileDispatchStallsForSegmentLoad,
    HvProfileDispatchStallsWhenReorderBufferFull,
    HvProfileDispatchStallsWhenReservationStationsFull,
    HvProfileDispatchStallsWhenFPUFull,
    HvProfileDispatchStallsWhenLSFull,
    HvProfileDispatchStallsWhenWaitingForAllQuiet,
    HvProfileDispatchStallsWhenFarControlOrResyncBranchPending,
    HvProfileFPUExceptions,
    HvProfileNumberOfBreakPointsForDR0,
    HvProfileNumberOfBreakPointsForDR1,
    HvProfileNumberOfBreakPointsForDR2,
    HvProfileNumberOfBreakPointsForDR3,
    HvProfileDRAMAccess,
    HvProfileDRAMPageTableOverflow,
    HvProfileDRAMDRAMCommandSlotsMissed,
    HvProfileMemoryControllerTurnAround,
    HvProfileMemoryControllerBypassCounter,
    HvProfileSizedCommands,
    HvProfileProbeResult,
    HvProfileUpstreamRequest,
    HvProfileHyperTransportBus0Bandwidth,
    HvProfileHyperTransportBus1Bandwidth,
    HvProfileHyperTransportBus2Bandwidth,
    HvProfileGuestTLBMissesAndInvalidates,
    HvProfileGuestTLBMisses,
    HvProfileGuestTLBInvalidates,
    HvProfileHostPageLargerThanGuestPage_RevC,
    HvProfilePageSizeMismatchCausedByMTRR_RevC,
    HvProfileGuestPageLargerThanHostPage_RevC,
    HvProfileInterruptChecks,
    HvProfileRetiredLFENCE,
    HvProfileRetiredSFENCE,
    HvProfileRetiredMFENCE,
    HvProfileCPUToDRAMRequestsLocalToNode0,
    HvProfileCPUToDRAMRequestsLocalToNode1,
    HvProfileCPUToDRAMRequestsLocalToNode2,
    HvProfileCPUToDRAMRequestsLocalToNode3,
    HvProfileCPUToDRAMRequestsLocalToNode4,
    HvProfileCPUToDRAMRequestsLocalToNode5,
    HvProfileCPUToDRAMRequestsLocalToNode6,
    HvProfileCPUToDRAMRequestsLocalToNode7,
    HvProfileIOToDRAMRequestsLocalToNode0,
    HvProfileIOToDRAMRequestsLocalToNode1,
    HvProfileIOToDRAMRequestsLocalToNode2,
    HvProfileIOToDRAMRequestsLocalToNode3,
    HvProfileIOToDRAMRequestsLocalToNode4,
    HvProfileIOToDRAMRequestsLocalToNode5,
    HvProfileIOToDRAMRequestsLocalToNode6,
    HvProfileIOToDRAMRequestsLocalToNode7,
    HvProfileCPURequestsToAPIC,

    //
    // Profile source descriptors for Intel processors.
    //

    HvProfileInstructionsRetired = 512,
    HvProfileUnhaltedReferenceCycles,
    HvProfileLLCReference,
    HvProfileLLCMisses,
    HvProfileBranchInstuctionRetired,
    HvProfileBranchMispredict
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
    // The proximity domain this memory range resides in.
    //
    HV_PROXIMITY_DOMAIN_ID ProximityDomainId;

} HV_MEMORY_RANGE_INFO, *PHV_MEMORY_RANGE_INFO;


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

#define HV_EVENTLOG_EVENT_IOMMU_WARNING_SCOPE_CONFLICT  0x0090 // HV_EVENTLOG_IOMMU_WARNING_SCOPE_CONFLICT_value
#define HV_EVENTLOG_EVENT_IOMMU_FAILED_RID_CONFLICT     0x0091 // HV_EVENTLOG_IOMMU_FAILED_RID_CONFLICT_value
#define HV_EVENTLOG_EVENT_IOMMU_FAILED_NO_RESOURCES     0x0092 // HV_EVENTLOG_IOMMU_FAILED_NO_RESOURCES_value
#define HV_EVENTLOG_EVENT_IOMMU_FAILED_INVALID_IOAPIC   0x0093 // HV_EVENTLOG_IOMMU_FAILED_INVALID_IOAPIC_value
#define HV_EVENTLOG_EVENT_IOMMU_FAILED_NO_DMA_REMAPPING 0x0094 // HV_EVENTLOG_IOMMU_FAILED_NO_DMA_REMAPPING_value
#define HV_EVENTLOG_EVENT_IOMMU_FAILED_RESERVED_DEVICE  0x0095 // HV_EVENTLOG_IOMMU_FAILED_RESERVED_DEVICE_value
#define HV_EVENTLOG_EVENT_PARTITION_CREATED             0x4101 // HV_EVENTLOG_PARTITION_CREATED_value
#define HV_EVENTLOG_EVENT_PARTITION_DELETED             0x4102 // HV_EVENTLOG_PARTITION_DELETED_value
#define HV_EVENTLOG_EVENT_PARTITION_CREATION_FAILED     0x2103 // HV_EVENTLOG_PARTITION_CREATION_FAILED_value

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
    HvEventLogEntryTimeTsc       = 1
} HV_EVENTLOG_ENTRY_TIME_BASIS;

//
// Define trace parameter constants
//

#define HV_EVENTLOG_MAX_BUFFER_SIZE_IN_PAGES 512
#define HV_EVENTLOG_MAX_BUFFER_COUNT 640

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
//
// Definition of the HvCallSetEventLogGroupSources hypercall input
// structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_EVENTLOG_SET_EVENTS
{

    HV_EVENTLOG_TYPE EventLogType;
    UINT64           EnableFlags;

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

#if defined(_HV_STACKWALK_ENABLED_) || defined(_PERF_FEATURES_ENABLED_)

//
// Definition of the stackwalk reason structure
//

typedef union _HV_STACKWALK_REASON
{
    UINT64 AsUINT64;
    struct
    {
        UINT32 AdditionalReason;
        UINT8  Reason;
        UINT8  Reserved[3];
    };
} HV_STACKWALK_REASON, *PHV_STACKWALK_REASON;

#endif // defined(_HV_STACKWALK_ENABLED_) || defined(_PERF_FEATURES_ENABLED_)

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
// ISSUE-kbroas-2010/10/23:  These groups do not appear to be utilized and
// could be reclaimed.
//
#define HV_TR_GROUP_ADMIN           0x0000000000000001
#define HV_TR_GROUP_DIAG            0x0000000000000002
#define HV_TR_GROUP_WARN            0x0000000000000003

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
// _SYNIC
// _SYNIC_TI
// _TI
// _VAL
// _VM
//
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

#define HV_TR_ALL_GROUPS (HV_TR_GROUP_BM | HV_TR_GROUP_DM | HV_TR_GROUP_HC | \
    HV_TR_GROUP_IM | HV_TR_GROUP_IC | HV_TR_GROUP_OB | \
    HV_TR_GROUP_PT | HV_TR_GROUP_VP | HV_TR_GROUP_SYNIC | \
    HV_TR_GROUP_SYNIC_TI | HV_TR_GROUP_AM_GVA | HV_TR_GROUP_AM | \
    HV_TR_GROUP_VAL | HV_TR_GROUP_VM | HV_TR_GROUP_SCH | \
    HV_TR_GROUP_TH | HV_TR_GROUP_TI | HV_TR_GROUP_KE | \
    HV_TR_GROUP_MM | HV_TR_GROUP_PROFILER)

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
// _TH_INTERNAL
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
#define HV_TR_GROUP_TH_INTERNAL       0x0080000000000000
#define HV_TR_GROUP_TI_INTERNAL       0x0100000000000000
#define HV_TR_GROUP_KE_INTERNAL       0x0200000000000000
#define HV_TR_GROUP_MM_INTERNAL       0x0400000000000000
#define HV_TR_GROUP_TR_INTERNAL       0x0800000000000000

//
// Tf, simulate full buffers and cyclic buffers are currently only
// supported for TEST_FEATURES_ENABLED builds.
//
// ISSUE-howardt-2012/06/27:  The following groups do not appear to be utilized
// and could be reclaimed:
//
// SIMULATE_FULL
// _CYCLIC
//
#define HV_TR_GROUP_TF                0x1000000000000000
#define HV_TR_GROUP_SIMULATE_FULL     0x2000000000000000
#define HV_TR_GROUP_CYCLIC            0x4000000000000000

//
// IceCap Trace Group.
//
#define HV_TR_GROUP_ICE               0x8000000000000000

#define HV_TR_ALL_GROUPS_INTERNAL (HV_TR_GROUP_BM_INTERNAL | \
    HV_TR_GROUP_DM_INTERNAL | HV_TR_GROUP_HC_INTERNAL | \
    HV_TR_GROUP_IM_INTERNAL | HV_TR_GROUP_IC_INTERNAL | \
    HV_TR_GROUP_OB_INTERNAL | HV_TR_GROUP_PT_INTERNAL | \
    HV_TR_GROUP_VP_INTERNAL | HV_TR_GROUP_SYNIC_INTERNAL | \
    HV_TR_GROUP_SYNIC_TI_INTERNAL | HV_TR_GROUP_AM_GVA_INTERNAL | \
    HV_TR_GROUP_AM_INTERNAL | HV_TR_GROUP_VAL_INTERNAL | \
    HV_TR_GROUP_VM_INTERNAL | HV_TR_GROUP_SCH_INTERNAL | \
    HV_TR_GROUP_TH_INTERNAL | HV_TR_GROUP_TI_INTERNAL | \
    HV_TR_GROUP_KE_INTERNAL | HV_TR_GROUP_MM_INTERNAL | \
    HV_TR_GROUP_TF | HV_TR_GROUP_SIMULATE_FULL | \
    HV_TR_GROUP_CYCLIC | HV_TR_GROUP_ICE)

#define HV_TR_IS_GROUP_INTERNAL(_Group_) \
    (((UINT64)(_Group_) > 0) && \
    (((UINT64)(_Group_) & HV_TR_ALL_GROUPS_INTERNAL) != 0) && \
    (((UINT64)(_Group_) & ((UINT64)(_Group_) - 1)) == 0))


#if defined(_HV_STACKWALK_ENABLED_) || defined(_PERF_FEATURES_ENABLED_)

//
// Expanded Stackwalking Event Types
//

#define HV_TR_STACKWALK_NONE                        0x0000000000000000

#define HV_TR_STACKWALK_HYPERCALL                   0x0000000000000001
#define HV_TR_STACKWALK_GUEST_EXCEPTION             0x0000000000000002
#define HV_TR_STACKWALK_MSR_READ                    0x0000000000000004
#define HV_TR_STACKWALK_MSR_WRITE                   0x0000000000000008
#define HV_TR_STACKWALK_CR_READ                     0x0000000000000010
#define HV_TR_STACKWALK_CR_WRITE                    0x0000000000000020
#define HV_TR_STACKWALK_HLT_INSTRUCTION             0x0000000000000040
#define HV_TR_STACKWALK_MWAIT_INSTRUCTION           0x0000000000000080
#define HV_TR_STACKWALK_CPUID_INSTRUCTION           0x0000000000000100
#define HV_TR_STACKWALK_IO_PORT_READ                0x0000000000000200
#define HV_TR_STACKWALK_IO_PORT_WRITE               0x0000000000000400
#define HV_TR_STACKWALK_EMULATED_INSTRUCTION        0x0000000000000800
#define HV_TR_STACKWALK_INVLPG_INSTRUCTION          0x0000000000001000
#define HV_TR_STACKWALK_IRET_INSTRUCTION            0x0000000000002000
#define HV_TR_STACKWALK_TASK_SWITCH                 0x0000000000004000
#define HV_TR_STACKWALK_INVD_INSTRUCTION            0x0000000000008000
#define HV_TR_STACKWALK_DR_ACCESS                   0x0000000000010000
#define HV_TR_STACKWALK_FERR_FREEZE                 0x0000000000020000
#define HV_TR_STACKWALK_MEMORY_INTERCEPT            0x0000000000040000
#define HV_TR_STACKWALK_REFLECTED_EXCEPTION         0x0000000000080000
#define HV_TR_STACKWALK_APIC_EOI                    0x0000000000100000
#define HV_TR_STACKWALK_APIC_WRITE                  0x0000000000200000
#define HV_TR_STACKWALK_APIC_ACCESS                 0x0000000000400000
#define HV_TR_STACKWALK_NESTED_PAGE_FAULT           0x0000000000800000
#define HV_TR_STACKWALK_PAUSE_LOOP_EXIT             0x0000000001000000
#define HV_TR_STACKWALK_CONTEXT_SWITCH              0x0000000002000000

#define HV_TR_STACKWALK_VAILD_BITS                  0x0000000003FFFFFF

#define HV_TR_STACKWALK_ALL                         0xFFFFFFFFFFFFFFFF

#endif // defined(_HV_STACKWALK_ENABLED_) || defined(_PERF_FEATURES_ENABLED_)

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
// Retail Sch Trace Types (0x1D B1-CD).
//

#define HV_TR_SCH_THREAD_RUNNABLE_LOCAL     0xB1
#define HV_TR_SCH_THREAD_RUNNABLE_DEFER     0xB2
#define HV_TR_SCH_EXPRESS                   0xB3
#define HV_TR_SCH_EXPRESS_FAIL              0xB4
#define HV_TR_SCH_APPLY_CAP                 0xB5
#define HV_TR_SCH_SET_CAP_TIMER             0xB6
#define HV_TR_SCH_COMPUTE_TIMESLICE         0xB7


//
// Retail Ti Trace Types (0x1D CE-D1).
//
// None.
//

//
// Retail Ke Trace Types (0x1D D2-EC).
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

//
// Retail Mm Trace Types (0x1D EE-EF).
//
// None.
//

//
// Retail Hypervisor Profiler traces (0x1D F0-F4)
//

#define HV_TR_PROFILER_SAMPLE               0xF0
#define HV_TR_PROFILER_HV_MODULE            0xF1

#if defined(_HV_TEST_FEATURES_ENABLED_) || defined(_PERF_FEATURES_ENABLED_) || \
    defined(_HV_COVERAGE_ENABLED_)

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

#define HV_TR_SYNICP_PERIODIC_TIMER_UPDATE  0x66
#define HV_TR_SYNICP_PERIODIC_TIMER_RESET   0x67
#define HV_TR_SYNICP_TIMER_SEND_MESSAGE     0x68
#define HV_TR_SYNICP_TIMER_SCAN_MESSAGE     0x69
#define HV_TR_SYNICP_TIMER_ASSIST_EXPIRE    0x6A

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
// Internal Sch Trace Types (0x1E B0-CD).
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
// Internal Th Trace Types (0x1E CE-CF).
//

#define HV_TR_THP_SEND_WORK                 0xCE

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


//
// Physical nodes are defined by a 32-bit index.
//

typedef UINT32 HV_PHYSICAL_NODE_INDEX, *PHV_PHYSICAL_NODE_INDEX;


typedef enum _HV_SAVE_RESTORE_STATE_RESULT
{
    HvStateComplete                                    = 0, 
    HvStateIncomplete                                  = 1, 
    HvStateRestorable                                  = 2, 
    HvStateCorruptData                                 = 3, 
    HvStateUnsupportedVersion                          = 4, 
    HvStateProcessorFeatureMismatch                    = 5, 
    HvStateHardwareFeatureMismatch                     = 6, 
    HvStateProcessorCountMismatch                      = 7, 
    HvStateProcessorFlagsMismatch                      = 8, 
    HvStateProcessorIndexMismatch                      = 9, 
    HvStateProcessorInsufficientMemory                 = 10,
    HvStateIncompatibleProcessor                       = 11,
    HvStateProcessorFeatureSse3Mismatch                = 12,
    HvStateProcessorFeatureLahfSahfMismatch            = 13,
    HvStateProcessorFeatureSsse3Mismatch               = 14,
    HvStateProcessorFeatureSse41Mismatch               = 15,
    HvStateProcessorFeatureSse42Mismatch               = 16,
    HvStateProcessorFeatureSse4aMismatch               = 17,
    HvStateProcessorFeatureXopMismatch                 = 18,
    HvStateProcessorFeaturePopcntMismatch              = 19,
    HvStateProcessorFeatureCmpxchg16bMismatch          = 20,
    HvStateProcessorFeatureAltmovcr8Mismatch           = 21,
    HvStateProcessorFeatureLzcntMismatch               = 22,
    HvStateProcessorFeatureMisalignedSseMismatch       = 23,
    HvStateProcessorFeatureMmxExtMismatch              = 24,
    HvStateProcessorFeature3DNowMismatch               = 25,
    HvStateProcessorFeatureExtended3DNowMismatch       = 26,
    HvStateProcessorFeaturePage1GBMismatch             = 27,
    HvStateProcessorCacheLineFlushSizeMismatch         = 28,
    HvStateProcessorFeatureXsaveMismatch               = 29,
    HvStateProcessorFeatureXsaveoptMismatch            = 30,
    // Deprecated: HvStateProcessorFeatureXsaveLegacySseMismatch = 31,
    HvStateProcessorFeatureXsaveAvxMismatch            = 32,
    HvStateProcessorFeatureXsaveFeatureMismatch        = 33,
    HvStateProcessorXsaveSaveAreaMismatch              = 34,
    HvStateProcessorFeatureAesMismatch                 = 35,
    HvStateProcessorFeaturePclmulqdqMismatch           = 36,
    HvStateProcessorFeaturePcidMismatch                = 37,
    HvStateProcessorFeatureFma4Mismatch                = 38,
    HvStateProcessorFeatureF16CMismatch                = 39,
    HvStateProcessorFeatureRdRandMismatch              = 40,
    HvStateProcessorFeatureRdWrFsGsMismatch            = 41,
    HvStateProcessorFeatureSmepMismatch                = 42,
    HvStateProcessorFeatureEnhancedFastStringMismatch  = 43,
    HvStateProcessorFeatureXsaveFmaMismatch            = 44,
    HvStateProcessorFeatureXsaveAvx2Mismatch           = 45,
    HvStateProcessorFeatureBmi1Mismatch                = 46,
    HvStateProcessorFeatureBmi2Mismatch                = 47,
    HvStateProcessorFeatureHleMismatch                 = 48,
    HvStateProcessorFeatureRtmMismatch                 = 49,
    HvStateProcessorFeatureMovbeMismatch               = 50,
    HvStateProcessorFeatureNpiep1Mismatch              = 51

} HV_SAVE_RESTORE_STATE_RESULT, *PHV_SAVE_RESTORE_STATE_RESULT;

typedef UINT32 HV_SAVE_RESTORE_STATE_FLAGS, *PHV_SAVE_RESTORE_STATE_FLAGS;

#define HV_SAVE_RESTORE_STATE_START   0x00000001
#define HV_SAVE_RESTORE_STATE_SUMMARY 0x00000002

typedef enum _HV_PROCESSOR_VENDOR
{
    HvProcessorVendorAmd        = 0x0000,
    HvProcessorVendorIntel      = 0x0001

} HV_PROCESSOR_VENDOR, *PHV_PROCESSOR_VENDOR;


//
// Define the structure defining the processor related features
// that may be de-featured.
// 

typedef union _HV_PARTITION_PROCESSOR_FEATURES
{
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
        UINT64 Reserved:1;
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
        UINT64 HleSupport:1;
        UINT64 RtmSupport:1;
        UINT64 MovbeSupport:1;
        UINT64 Npiep1Support:1;
        UINT64 DepX87FPUSaveSupport:1;
        UINT64 Reserved1:32;
    };
    UINT64 AsUINT64;

} HV_PARTITION_PROCESSOR_FEATURES, *PHV_PARTITION_PROCESSOR_FEATURES;

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
// Define the processor features available in Intel and AMD compatibility mode.
//

#define HV_PARTITION_PROCESSOR_FEATURES_INTEL_COMPATIBILITY_MODE \
{   1,   /* Sse3Support */ \
    1,   /* LahfSahfSupport */ \
    0,   /* Ssse3Support */ \
    0,   /* Sse4_1Support */ \
    0,   /* Sse4_2Support */ \
    0,   /* Sse4aSupport */ \
    0,   /* XopSupport */ \
    0,   /* PopCntSupport */ \
    1,   /* Cmpxchg16bSupport */ \
    0,   /* Altmovcr8Support */ \
    0,   /* LzcntSupport */ \
    0,   /* MisAlignSseSupport */ \
    0,   /* MmxExtSupport */ \
    0,   /* Amd3DNowSupport */ \
    0,   /* ExtendedAmd3DNowSupport */ \
    0,   /* Reserved */ \
    0,   /* Aes */ \
    0,   /* Pclmulqdq */ \
    0,   /* Pcid */ \
    0,   /* Fma4 */ \
    0,   /* F16C */ \
    0,   /* RdRand */ \
    0,   /* RdWrFsGs */ \
    0,   /* Smep */ \
    0,   /* EnhancedFastString */ \
    0,   /* Bmi1Support */ \
    0,   /* Bmi2Support */ \
    0,   /* HleSupport */ \
    0,   /* RtmSupport */ \
    0,   /* MovbeSupport */ \
    0,   /* Npiep1Support */ \
    0,   /* DepX87FPUSaveSupport */ \
    0    /* Reserved1 */ \
}

#define HV_PARTITION_PROCESSOR_FEATURES_AMD_COMPATIBILITY_MODE \
{ \
    1,   /* Sse3Support */ \
    1,   /* LahfSahfSupport */ \
    0,   /* Ssse3Support */ \
    0,   /* Sse4_1Support */ \
    0,   /* Sse4_2Support */ \
    0,   /* Sse4aSupport */ \
    0,   /* XopSupport */ \
    0,   /* PopCntSupport */ \
    1,   /* Cmpxchg16bSupport */ \
    1,   /* Altmovcr8Support */ \
    0,   /* LzcntSupport */ \
    0,   /* MisAlignSseSupport */ \
    1,   /* MmxExtSupport */ \
    0,   /* Amd3DNowSupport */ \
    0,   /* ExtendedAmd3DNowSupport */ \
    0,   /* Reserved */ \
    0,   /* Aes */ \
    0,   /* Pclmulqdq */ \
    0,   /* Pcid */ \
    0,   /* Fma4 */ \
    0,   /* F16C */ \
    0,   /* RdRand */ \
    0,   /* RdWrFsGs */ \
    0,   /* Smep */ \
    0,   /* EnhancedFastString */ \
    0,   /* Bmi1Support */ \
    0,   /* Bmi2Support */ \
    0,   /* HleSupport */ \
    0,   /* RtmSupport */ \
    0,   /* MovbeSupport */ \
    1,   /* Npiep1Support */ \
    0,   /* DepX87FPUSaveSupport */ \
    0    /* Reserved1 */ \
}

//
// Define the structure defining the processor XSAVE related features
// that may be de-featured.
//
// N.B. The bit positions for new features must match the bit positions
// for the XFEM register to enable masking feature bits with XFEM register
// values.
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
        UINT64 Reserved1:59;
    };
    UINT64 AsUINT64;

} HV_PARTITION_PROCESSOR_XSAVE_FEATURES, *PHV_PARTITION_PROCESSOR_XSAVE_FEATURES;

//
// Define the processor features avaialble in Intel and AMD compatibility mode.
//

#define HV_PARTITION_PROCESSOR_XSAVE_FEATURES_INTEL_COMPATIBILITY_MODE \
{ \
    0,   /* XsaveSupport */ \
    0,   /* XsaveoptSupport */ \
    0,   /* AvxSupport */ \
    0,   /* Avx2Support */ \
    0,   /* FmaSupport */ \
    0    /* Reserved1 */ \
}

#define HV_PARTITION_PROCESSOR_XSAVE_FEATURES_AMD_COMPATIBILITY_MODE \
{ \
    0,   /* XsaveSupport */ \
    0,   /* XsaveoptSupport */ \
    0,   /* AvxSupport */ \
    0,   /* Avx2Support */ \
    0,   /* FmaSupport */ \
    0    /* Reserved1 */ \
}

//
// Define the processor cache line flush size Intel and AMD compatibility mode.
//

#define HV_PARTITION_PROCESSOR_CL_FLUSHSIZE_INTEL_COMPATIBILITY_MODE (8)
#define HV_PARTITION_PROCESSOR_CL_FLUSHSIZE_AMD_COMPATIBILITY_MODE (8)


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
        UINT64 TlbLocked:1;
        UINT64 Reserved:62;
    };
} HV_INTERCEPT_SUSPEND_REGISTER, *PHV_INTERCEPT_SUSPEND_REGISTER;

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

typedef enum _HV_X64_PENDING_INTERRUPTION_TYPE
{
    HvX64PendingInterrupt           = 0,
    HvX64PendingNmi                 = 2,
    HvX64PendingException           = 3
} HV_X64_PENDING_INTERRUPTION_TYPE, *PHV_X64_PENDING_INTERRUPTION_TYPE;

typedef union _HV_X64_PENDING_INTERRUPTION_REGISTER
{
    UINT64 AsUINT64;
    struct
    {
        UINT32 InterruptionPending:1;
        UINT32 InterruptionType:3;
        UINT32 DeliverErrorCode:1;
        UINT32 Reserved:11;
        UINT32 InterruptionVector:16;
        UINT32 ErrorCode;
    };
} HV_X64_PENDING_INTERRUPTION_REGISTER, *PHV_X64_PENDING_INTERRUPTION_REGISTER;

typedef union _HV_REGISTER_VALUE
{
    HV_UINT128                              Reg128;
    UINT64                                  Reg64;
    UINT32                                  Reg32;
    UINT16                                  Reg16;
    UINT8                                   Reg8;
    HV_X64_FP_REGISTER                      Fp;
    HV_X64_FP_CONTROL_STATUS_REGISTER       FpControlStatus;
    HV_X64_XMM_CONTROL_STATUS_REGISTER      XmmControlStatus;
    HV_X64_SEGMENT_REGISTER                 Segment;
    HV_X64_TABLE_REGISTER                   Table;
    HV_EXPLICIT_SUSPEND_REGISTER            ExplicitSuspend;
    HV_INTERCEPT_SUSPEND_REGISTER           InterceptSuspend;
    HV_X64_INTERRUPT_STATE_REGISTER         InterruptState;
    HV_X64_PENDING_INTERRUPTION_REGISTER    PendingInterruption;
    HV_X64_MSR_NPIEP_CONFIG_CONTENTS        NpiepConfig;
} HV_REGISTER_VALUE, *PHV_REGISTER_VALUE;
typedef const HV_REGISTER_VALUE *PCHV_REGISTER_VALUE;

//
// Define the intercept access types.
//

typedef UINT8 HV_INTERCEPT_ACCESS_TYPE;

#define HV_INTERCEPT_ACCESS_READ    0
#define HV_INTERCEPT_ACCESS_WRITE   1
#define HV_INTERCEPT_ACCESS_EXECUTE 2

typedef UINT32 HV_INTERCEPT_ACCESS_TYPE_MASK;

#define HV_INTERCEPT_ACCESS_MASK_NONE       0x00
#define HV_INTERCEPT_ACCESS_MASK_READ       0X01
#define HV_INTERCEPT_ACCESS_MASK_WRITE      0x02
#define HV_INTERCEPT_ACCESS_MASK_EXECUTE    0x04


//
// Define intercept types.
//
typedef enum _HV_INTERCEPT_TYPE
{
    //
    // Platform-specific intercept types.
    //
    HvInterceptTypeX64IoPort = 0x00000000,
    HvInterceptTypeX64Msr = 0x00000001,
    HvInterceptTypeX64Cpuid = 0x00000002,
    HvInterceptTypeX64Exception = 0x00000003,

} HV_INTERCEPT_TYPE, *PHV_INTERCEPT_TYPE;


//
// Define IO port type.
//
typedef UINT16 HV_X64_IO_PORT, *PHV_X64_IO_PORT;


//
// Define intercept parameters.
//
typedef union _HV_INTERCEPT_PARAMETERS
{
    //
    // HV_INTERCEPT_PARAMETERS is defined to be an 8-byte field.
    //
    UINT64 AsUINT64;

    //
    // HvInterceptTypeX64IoPort.
    //
    HV_X64_IO_PORT IoPort;

    //
    // HvInterceptTypeX64Cpuid.
    //
    UINT32 CpuidIndex;

    //
    // HvInterceptTypeX64Exception.
    //
    UINT16 ExceptionVector;

    //
    // N.B. Other intercept types do not have any paramaters.
    //

} HV_INTERCEPT_PARAMETERS, *PHV_INTERCEPT_PARAMETERS;


//
// Define intercept descriptor structure.
//
typedef struct  _HV_INTERCEPT_DESCRIPTOR
{
    HV_INTERCEPT_TYPE Type;
    HV_INTERCEPT_PARAMETERS Parameters;
} HV_INTERCEPT_DESCRIPTOR, *PHV_INTERCEPT_DESCRIPTOR;
typedef const HV_INTERCEPT_DESCRIPTOR *PCHV_INTERCEPT_DESCRIPTOR;

//
// Define GVA range structures used by the TLB flush routines.
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

#define HV_GVA_RANGE_SIMPLE_ADDITIONAL_PAGES_MASK   0xFFF
#define HV_GVA_RANGE_EXTENDED_ADDITIONAL_PAGES_MASK 0x7FF

typedef union _HV_GVA_RANGE
{
    UINT64 AsUINT64;

    HV_GVA_RANGE_SIMPLE   Simple;
    HV_GVA_RANGE_EXTENDED Extended;

} HV_GVA_RANGE, *PHV_GVA_RANGE;

#define HV_GVA_RANGE_LARGEPAGESIZE_2MB      0x00
#define HV_GVA_RANGE_LARGEPAGESIZE_1GB      0x01


//
// Declare the VP run time MSR.
//
#define HV_X64_MSR_VP_RUNTIME   0x40000010

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
// Address translation flags.
//

#define HV_TRANSLATE_GVA_VALIDATE_READ       (0x0001)
#define HV_TRANSLATE_GVA_VALIDATE_WRITE      (0x0002)
#define HV_TRANSLATE_GVA_VALIDATE_EXECUTE    (0x0004)
#define HV_TRANSLATE_GVA_PRIVILEGE_EXEMPT    (0x0008)
#define HV_TRANSLATE_GVA_SET_PAGE_TABLE_BITS (0x0010)
#define HV_TRANSLATE_GVA_TLB_FLUSH_INHIBIT   (0x0020)
#define HV_TRANSLATE_GVA_CONTROL_MASK        (0x003F)

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
    HvTranslateGvaGpaIllegalOverlayAccess = 7

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

//
// Read and write GPA access flags.
//

typedef union _HV_ACCESS_GPA_CONTROL_FLAGS
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 CacheType : 8;  // Cache type for access
        UINT64 Reserved  : 56;
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
// Flags to describe the access a partition has to a GPA page.
//
typedef UINT32 HV_MAP_GPA_FLAGS, *PHV_MAP_GPA_FLAGS;

#define HV_MAP_GPA_READABLE             0x1
#define HV_MAP_GPA_WRITABLE             0x2
#define HV_MAP_GPA_EXECUTABLE           0x4
#define HV_MAP_GPA_PERMISSIONS_MASK     0x7

#define HV_MAP_GPA_NOT_PRESENT          0x8
#define HV_MAP_GPA_FLAGS_MASK           0xF

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

#if defined(_PERF_FEATURES_ENABLED_) || defined(_HV_STACKWALK_ENABLED_)

#define HV_SYNIC_EXPANDED_STACKWALK_DISABLED 0

//
// Declare the MSRs used to read the hypervisor stackwalk reason
// and to set the stackwalk vector for a partition.
//
#define HV_X64_MSR_STACKWALK_REASON (0x40000074)
#define HV_X64_MSR_STACKWALK_VECTOR (0x40000075)

#endif

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
#define HV_SYNIC_APIC_MINIMUM_VECTOR    0x10

#define HV_MESSAGE_TYPE_HYPERVISOR_MASK (0x80000000)

//
// Define APIC EOI message.
//
typedef struct _HV_X64_APIC_EOI_MESSAGE
{
    UINT32 VpIndex;
    UINT32 InterruptVector;
} HV_X64_APIC_EOI_MESSAGE, *PHV_X64_APIC_EOI_MESSAGE;

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
} HV_PORT_PROPERTY_CODE, *PHV_PORT_PROPERTY_CODE;

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
// Emulated timer period
//
typedef union _HV_EMULATED_TIMER_PERIOD
{
    UINT64              AsUINT64;
    HV_PICO100_DURATION Period;

} HV_EMULATED_TIMER_PERIOD, *PHV_EMULATED_TIMER_PERIOD;

//
// Periodic Timer route
//
typedef union _HV_EMULATED_TIMER_CONTROL
{
    UINT64  AsUINT64;

    struct
    {
        UINT32  Vector                  :  8;
        UINT32  DeliveryMode            :  3;
        UINT32  LogicalDestinationMode  :  1;
        UINT32  Enabled                 :  1;
        UINT32  Reserved1               : 19;
        UINT32  Reserved2               : 24;
        UINT32  Mda                     :  8;
    };

} HV_EMULATED_TIMER_CONTROL, *PHV_EMULATED_TIMER_CONTROL;

//
// ACPI PM timer
//
typedef union _HV_PM_TIMER_INFO
{
    UINT64  AsUINT64;

    struct
    {
        UINT32  Port                : 16;
        UINT32  Width24             :  1;
        UINT32  Enabled             :  1;
        UINT32  Reserved1           : 14;
        UINT32  Reserved2           : 32;
    };

} HV_PM_TIMER_INFO, *PHV_PM_TIMER_INFO;

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

#if defined(_PERF_FEATURES_ENABLED_) || defined(_HV_STACKWALK_ENABLED_)
//
// Declare structure used to enable expanded stackwalking on a VP
//
typedef union _HV_STACKWALK_VECTOR_CONTROL
{
    UINT64 AsUINT64;
    struct
    {
        UINT8 Vector;
        UINT8 Reserved[7];
    };
} HV_STACKWALK_VECTOR_CONTROL, *PHV_STACKWALK_VECTOR_CONTROL;
#endif


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
// IOAPIC requires up to 121 IRT entries from a total of 1536-32 entries
// available per RID.
//
#define HV_IOMMU_AMD_MAX_IOAPICS_PER_RID    12

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
// consists of 1 page for the root-entry table and 256 pages for the
// context-entry tables. These do not have to be contiguous.
//
#define HV_IOMMU_INTEL_ADDITIONAL_PAGES_PER_IOMMU   257

//
// Number of 8 KB memory blocks needed on AMD for each PCI segment.
//
// This is used to supply a pool of large (2-page) interrupt remapping tables
// used for root devices.
//
#define HV_IOMMU_AMD_8K_BLOCKS_PER_SEGMENT          256



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
        UINT16 Reserved:9;
    };
} HV_X64_VP_EXECUTION_STATE, *PHV_X64_VP_EXECUTION_STATE;


//
// Define intercept message header structure.
//
typedef struct _HV_X64_INTERCEPT_MESSAGE_HEADER
{
    HV_VP_INDEX VpIndex;
    UINT8 InstructionLength;
    HV_INTERCEPT_ACCESS_TYPE InterceptAccessType;
    HV_X64_VP_EXECUTION_STATE ExecutionState;
    HV_X64_SEGMENT_REGISTER CsSegment;
    UINT64 Rip;
    UINT64 Rflags;
} HV_X64_INTERCEPT_MESSAGE_HEADER, *PHV_X64_INTERCEPT_MESSAGE_HEADER;


//
// Define memory access information structure.
//
typedef union _HV_X64_MEMORY_ACCESS_INFO
{
    UINT8 AsUINT8;
    struct
    {
        UINT8 GvaValid:1;
        UINT8 Reserved:7;
    };
} HV_X64_MEMORY_ACCESS_INFO, *PHV_X64_MEMORY_ACCESS_INFO;


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
        UINT8 Reserved:7;
    };
} HV_X64_EXCEPTION_INFO, *PHV_X64_EXCEPTION_INFO;


//
// Define memory access message structure. This message structure is used
// for memory intercepts, GPA not present intercepts and SPA access violation
// intercepts.
//
typedef struct _HV_X64_MEMORY_INTERCEPT_MESSAGE
{
    HV_X64_INTERCEPT_MESSAGE_HEADER Header;
    HV_CACHE_TYPE CacheType;
    UINT8 InstructionByteCount;
    HV_X64_MEMORY_ACCESS_INFO MemoryAccessInfo;
    UINT16 Reserved1;
    UINT64 GuestVirtualAddress;
    UINT64 GuestPhysicalAddress;
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
} HV_X64_MEMORY_INTERCEPT_MESSAGE, *PHV_X64_MEMORY_INTERCEPT_MESSAGE;


//
// Define CPUID intercept message structure.
//
typedef struct _HV_X64_CPUID_INTERCEPT_MESSAGE
{
    HV_X64_INTERCEPT_MESSAGE_HEADER Header;
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


//
// Define legacy floating point error message.
//
typedef struct _HV_X64_LEGACY_FP_ERROR_MESSAGE
{
    UINT32 VpIndex;
    UINT32 Reserved;
} HV_X64_LEGACY_FP_ERROR_MESSAGE, *PHV_X64_LEGACY_FP_ERROR_MESSAGE;


//
// Define invalid virtual processor register message.
//
typedef struct _HV_X64_INVALID_VP_REGISTER_MESSAGE
{
    UINT32 VpIndex;
    UINT32 Reserved;
} HV_X64_INVALID_VP_REGISTER_MESSAGE, *PHV_X64_INVALID_VP_REGISTER_MESSAGE;


//
// Define virtual processor unrecoverable error message.
//
typedef struct _HV_X64_UNRECOVERABLE_EXCEPTION_MESSAGE
{
    HV_X64_INTERCEPT_MESSAGE_HEADER Header;
} HV_X64_UNRECOVERABLE_EXCEPTION_MESSAGE, *PHV_X64_UNRECOVERABLE_EXCEPTION_MESSAGE;


//
// Define the unsupported feature codes.
//
typedef enum _HV_X64_UNSUPPORTED_FEATURE_CODE
{
    HvUnsupportedFeatureIntercept = 1,
    HvUnsupportedFeatureTaskSwitchTss = 2
}HV_X64_UNSUPPORTED_FEATURE_CODE, *PHV_X64_UNSUPPORTED_FEATURE_CODE;


//
// Define unsupported feature message.
//
typedef struct _HV_X64_UNSUPPORTED_FEATURE_MESSAGE
{
    UINT32 VpIndex;
    HV_X64_UNSUPPORTED_FEATURE_CODE FeatureCode;
    UINT64 FeatureParameter;
} HV_X64_UNSUPPORTED_FEATURE_MESSAGE, *PHV_X64_UNSUPPORTED_FEATURE_MESSAGE;


//
// Define TLB page size mismatch message.
//
typedef struct _HV_X64_TLB_PAGE_SIZE_MISMATCH_MESSAGE
{
    UINT32 VpIndex;
    UINT32 Reserved;
} HV_X64_TLB_PAGE_SIZE_MISMATCH_MESSAGE, *PHV_X64_TLB_PAGE_SIZE_MISMATCH_MESSAGE;

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

typedef union HV_CALL_ATTRIBUTES _HV_REGISTER_INTERCEPT_RESULT_PARAMETERS
{
    HV_REGISTER_X64_CPUID_RESULT_PARAMETERS Cpuid;
} HV_REGISTER_INTERCEPT_RESULT_PARAMETERS, *PHV_REGISTER_INTERCEPT_RESULT_PARAMETERS;

//
// Definition of the HvCallUnregisterInterceptResult hypercall input structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_UNREGISTER_X64_CPUID_RESULT_PARAMETERS
{
    UINT32 Eax;
    UINT32 Ecx;
    BOOLEAN SubleafSpecific;
} HV_UNREGISTER_X64_CPUID_RESULT_PARAMETERS, *PHV_UNREGISTER_X64_CPUID_RESULT_PARAMETERS;

typedef union HV_CALL_ATTRIBUTES _HV_UNREGISTER_INTERCEPT_RESULT_PARAMETERS
{
    HV_UNREGISTER_X64_CPUID_RESULT_PARAMETERS Cpuid;
} HV_UNREGISTER_INTERCEPT_RESULT_PARAMETERS, *PHV_UNREGISTER_INTERCEPT_RESULT_PARAMETERS;


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
// Declare the input and output structures for the HvCreatePartition hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_CREATE_PARTITION
{
    UINT64 Flags;
    HV_PROXIMITY_DOMAIN_INFO ProximityDomainInfo;
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
// Definition of the HvCallMapGpaPages hypercall input structure.
// This call maps a range of GPA to a supplied range of SPA.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_MAP_GPA_PAGES
{
    //
    // Supplies the partition ID of the partition that this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies the base guest physical page number where the mapping
    // will begin.
    //

    HV_GPA_PAGE_NUMBER TargetGpaBase;

    //
    // Supplies the flags to use for the mapping.
    //

    HV_MAP_GPA_FLAGS MapFlags;

    //
    // Supplies an array of guest physical page numbers in the calling
    // partition that the range of GPA will be mapped to.
    //

    HV_CALL_ATTRIBUTES HV_GPA_PAGE_NUMBER SourceGpaPageList[];

} HV_INPUT_MAP_GPA_PAGES, *PHV_INPUT_MAP_GPA_PAGES;


//
// Definition of the HvCallMapSparseGpaPages hypercall input structure.
// This call maps a range of GPA to a supplied range of SPA.
//

typedef struct _HV_GPA_MAPPING
{
    HV_GPA_PAGE_NUMBER TargetGpaPageNumber;
    HV_GPA_PAGE_NUMBER SourceGpaPageNumber;
} HV_GPA_MAPPING, *PHV_GPA_MAPPING;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_MAP_SPARSE_GPA_PAGES
{
    //
    // Supplies the partition ID of the partition that this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies the flags to use for the mapping.
    //

    HV_MAP_GPA_FLAGS MapFlags;

    //
    // Supplies an array of pairs of physical page numbers.
    //

    HV_CALL_ATTRIBUTES HV_GPA_MAPPING PageList[];

} HV_INPUT_MAP_SPARSE_GPA_PAGES, *PHV_INPUT_MAP_SPARSE_GPA_PAGES;

//
// Definition of the HvCallUnmapGpaPages hypercall input structure.
// This call unmaps a range of GPA.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_UNMAP_GPA_PAGES
{

    //
    // Supplies the partition ID of the partition that this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies the base guest physical page number where the GPA
    // space will be removed.
    //

    HV_GPA_PAGE_NUMBER TargetGpaBase;

} HV_INPUT_UNMAP_GPA_PAGES, *PHV_INPUT_UNMAP_GPA_PAGES;

//
// Definition of the HvCallModifySparseGpaPages hypercall input structure. This
// call modifies the access mask of an existing set of GPA pages.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_MODIFY_SPARSE_GPA_PAGES
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
    // Supplies an array of GPA page numbers to modify.
    //

    HV_CALL_ATTRIBUTES HV_GPA_PAGE_NUMBER GpaPageList[];

} HV_INPUT_MODIFY_SPARSE_GPA_PAGES, *PHV_INPUT_MODIFY_SPARSE_GPA_PAGES;

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

//
// Definition of the HvCallInstallIntercept hypercall input
// structure.  This call sets an intercept.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_INSTALL_INTERCEPT
{
    HV_PARTITION_ID PartitionId;
    HV_INTERCEPT_ACCESS_TYPE_MASK AccessType;
    HV_INTERCEPT_TYPE InterceptType;
    HV_INTERCEPT_PARAMETERS InterceptParameter;
} HV_INPUT_INSTALL_INTERCEPT, *PHV_INPUT_INSTALL_INTERCEPT;

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
    UINT32               RsvdZ;
    HV_CALL_ATTRIBUTES
    HV_REGISTER_ASSOC    Elements[];
} HV_INPUT_SET_VP_REGISTERS, *PHV_INPUT_SET_VP_REGISTERS;

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
    UINT32                  Reserved;
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
    UINT32                      Padding;
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
    UINT32                      Reserved1;
    HV_PARTITION_ID             PortPartition;
    HV_PORT_ID                  PortId;
    UINT32                      Reserved2;
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
