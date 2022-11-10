/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    hvhdk_mini.h

Abstract:

    Type definitions for the hypervisor host interface to kernel.

--*/

#if !defined(_HVHDK_MINI_)
#define _HVHDK_MINI_

#include <hvgdk_mini.h>

#if _MSC_VER > 1000
#pragma once
#endif

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning(disable:4200) // zero length array
#pragma warning(disable:4214) // bit field types other than int
#pragma warning(disable:4324) // structure was padded due to __declspec(align())

// Define maximum number of processors supported by the hypervisor on the
// AMD64 platform.
//
//
#define HV_MAXIMUM_PROCESSORS       2048

//
// Define maximum number of virtual processors per partition supported by the
// hypervisor on the AMD64 platform.
//
#define HV_MAX_VPS_PER_PARTITION    320
#define HV_MAX_VPS_PER_CHILD_PARTITION 240
#define HV_MAX_VPS_PER_LEGACY_CHILD_PARTITION 64

#define HV_VP_INDEX_INVALID HV_ANY_VP

//
// Declare the maximum number of NUMA nodes.
//
#define HV_MAXIMUM_NODES            64

//
// Declare the maximum number of COS slots.
// This was calculated using the range of reserved MSR indices.
//
#define HV_MAXIMUM_COS_SLOTS        128

//
//
// Define the facility codes
//
#define HV_FACILITY_HYPERVISOR           0x35


//
// Define the severity codes
//
#define HV_SEVERITY_WARNING              0x2
#define HV_SEVERITY_SUCCESS              0x0
#define HV_SEVERITY_INFORMATIONAL        0x1
#define HV_SEVERITY_ERROR                0x3


//
// The encoding of BAL_STATUS is wide enough to support environment-specific
// types (NTSTATUS, DOSERROR, etc.) as a subset.
//
typedef UINT64 BAL_STATUS, *PBAL_STATUS;

//
// BAL status values. These are done as #define's instead of enums as
// the latter are restricted to 32bit values.
//

//
// The operation completed successfully.
//
#define BAL_STATUS_SUCCESS                      0x0

//
// The operation did not complete successfully.
//
// NOTE: This status value should *ONLY* be used in non-production code, as
//       it provides no meaning to the user.
//
#define BAL_STATUS_UNSUCCESSFUL                 0x1

//
// The specified image was corrupt or invalid.
//
#define BAL_STATUS_INVALID_IMAGE                0x2

//
// The virtualization platform on this system is unknown or absent.
//
#define BAL_STATUS_UNKNOWN_PLATFORM             0x3

//
// The virtualization platform is not the same on all processors.
//
#define BAL_STATUS_PLATFORM_MISMATCH            0x4

//
// The parameters provided were invalid.
//
#define BAL_STATUS_INVALID_PARAMETERS           0x5

//
// The specified firmware table was not found.
//
#define BAL_STATUS_FIRMWARE_TABLE_NOT_FOUND     0x6

//
// The firmware information is invalid.
//
#define BAL_STATUS_FIRMWARE_INFO_INVALID        0x7

//
// Hardware is incompatible - general processor incompatibility.
//
#define BAL_STATUS_HW_INCOMPAT_GENERAL          0x8

//
// Hypervisor version mismatch.
//
#define BAL_STATUS_VERSION_MISMATCH             0x9

//
// Hardware is incompatible - SVM feature incompatible
//
#define BAL_STATUS_HW_INCOMPAT_SVM_COMPAT      0xA

//
// Hardware is incompatible - SVM feature disabled
//
#define BAL_STATUS_HW_INCOMPAT_SVM_DISABLED    0xB

//
// Hardware is incompatible - VMX feature incompatibility
//
#define BAL_STATUS_HW_INCOMPAT_VMX_COMPAT      0xC

//
// Hardware is incompatible - VMX feature disabled
//
#define BAL_STATUS_HW_INCOMPAT_VMX_DISABLED    0xD

//
// Hardware is incompatible - cache topology
//
#define BAL_STATUS_HW_INCOMPAT_CACHE           0xE

//
// The rebasing of the hypervisor image failed.
//
#define BAL_STATUS_REBASE_FAILED               0xF

//
// An internal allocation overflowed its maximal size.
//
#define BAL_STATUS_BUFFER_OVERFLOW             0x10

//
// No execute is not detected.
//
#define BAL_STATUS_NX_NOT_DETECTED             0x11

//
// SMX has been enabled by the BIOS.
//
#define BAL_STATUS_SMX_ENABLED                 0x12

//
// VMX is not detected.
//
#define BAL_STATUS_VMX_NOT_DETECTED            0x13

//
// SVM is not detected.
//
#define BAL_STATUS_SVM_NOT_DETECTED            0x14

//
// Invalid processor.
//
#define BAL_STATUS_INVALID_PROCESSOR           0x15

//
// Required hardware features not supported.
//
#define BAL_STATUS_FEATURES_NOT_SUPPORTED      0x16

//
// Required hardware features not present.
//
#define BAL_STATUS_FEATURES_NOT_PRESENT        0x17

//
// Insufficient capability.
//
#define BAL_STATUS_INSUFFICIENT_CAPABILITY     0x18

//
// Internal logic error.
//
#define BAL_STATUS_INTERNAL_LOGIC_ERROR        0x19

//
// SLAT is not present on the machine.
//
#define BAL_STATUS_SLAT_NOT_PRESENT            0x1A

//
// Hypervisor launch disabled.
//
#define BAL_STATUS_HYPERVISOR_LAUNCH_DISABLED  0x1B

//
// IOMMU has been disabled.
//
#define BAL_STATUS_IOMMU_DISABLED              0x1C

//
// BAL winload failures.
//
// Generic winload failure causing hypervisor launch failure
//
#define BAL_STATUS_WINLOAD_UNSUCCESSFUL        0x1D

//
// Winload unable to locate a required resource.
//
#define BAL_STATUS_WINLOAD_STATUS_NOT_FOUND    0x1E

//
// Winload failure due to insufficient buffer size.
//
#define BAL_STATUS_WINLOAD_BUFFER_TOO_SMALL    0x1F

//
// Winload was unable to allocate sufficient memory to complete and operation.
//
#define BAL_STATUS_WINLOAD_NO_MEMORY           0x20

//
// Winload unable to allocate a required resource.
//
#define BAL_STATUS_WINLOAD_INSUFFICIENT_RESOURCES  0x21

//
// Winload encountered a memory map conflict.
//
#define BAL_STATUS_WINLOAD_CONFLICTING_ADDRESSES   0x22

//
// Hypervisor initialization failed.
//
#define BAL_STATUS_HYPERVISOR_INIT_FAILED          0x23

//
// BSP does not support minimum required CPUID leaves.
//
#define BAL_STATUS_CPUID_MIN_FEATURE_LEAFS_NOT_SUPPORTED    0x24

//
// Physical address limit that we support exceeded.
//
#define BAL_STATUS_CPUID_PROCESSOR_PHY_ADDR_LIMIT_EXCEEDED  0x25

//
// Too many runtime services memory ranges.
//
#define BAL_STATUS_TOO_MANY_RS_MEMORY_RANGES        0x26

//
// A VMX instruction failed
//
// Note: this value must match the corresponding VMX hardware error code.
//
#define BAL_STATUS_VMX_INSTRUCTION_FAILED               0x80000000UI64

//
// A VMX instruction failed and an error code indicating why is in the
// current VMCS's VMX_VMCS_INSTRUCTION_ERROR_FIELD
//
// Note: this value must match the corresponding VMX hardware error code.
//
#define BAL_STATUS_VMX_INSTRUCTION_FAILED_WITH_ERROR    0x80000001UI64

//
// IOMMU extended status definitions.
//

//
// Severity value indicating no extended status.
//
#define HV_IOMMU_STATUS_SEVERITY_NONE 0

//
// Device operation succeeded with warning - the device is not reported under
// the scope of a unique I/O remapping unit.
//
#define HV_IOMMU_STATUS_WARNING_SCOPE_CONFLICT      1

//
// Device operation failed - the Requester IDs reported for the device overlap
// with those reported for another device.
//
#define HV_IOMMU_STATUS_FAILED_RID_CONFLICT         2

//
// Device operation failed - the hypervisor does not have enough resources.
//
#define HV_IOMMU_STATUS_FAILED_NO_RESOURCES         3

//
// Device operation failed - an IOAPIC is not correctly reported.
//
#define HV_IOMMU_STATUS_FAILED_INVALID_IOAPIC       4

//
// Device operation failed - the I/O remapping unit that controls the device
// does not support all capabilities required for device assignment.
//
#define HV_IOMMU_STATUS_FAILED_NO_DEVICE_ASSIGNMENT 5

//
// Device operation failed - the device cannot be securely used by a child
// partition.
//
#define HV_IOMMU_STATUS_FAILED_RESERVED_DEVICE      6


//
// Global Hypervisor System Events: {52FC89F8-995E-434c-A91E-199986449890}
//
#define  HV_ADMIN_ETW_GUID \
{ 0x52fc89f8, 0x995e, 0x434c, \
{ 0xa9, 0x1e, 0x19, 0x99, 0x86, 0x44, 0x98, 0x90 } };

//
// Local Hypervisor Diagnostic Events: {910C653D-A4EB-4719-B909-4588E3BAEC91}
//
#define HV_LOCAL_ETW_GUID \
{ 0x910c653d, 0xa4eb, 0x4719, \
{ 0xb9, 0x9, 0x45, 0x88, 0xe3, 0xba, 0xec, 0x91 } };


//
// System physical addresses (SPAs) define the physical address space of the underlying
// hardware. There is only one system physical address space for the entire machine.
//
typedef UINT64 HV_SPA, *PHV_SPA;
typedef UINT64 HV_SPA_PAGE_NUMBER, *PHV_SPA_PAGE_NUMBER;

#define HV_INVALID_SPA              ((HV_SPA)-1)
#define HV_INVALID_SPA_PAGE_NUMBER  ((HV_SPA_PAGE_NUMBER)-1)

//
// A private version of the hypercall input control format that is accessible to the
// root only. Make sure this remains in sync with the guest accessible format.
//
#if !defined(XBOX_SYSTEMOS)

typedef union _HV_HYPERCALL_INPUT_PRIVATE
{
    //
    // Input: The call code, argument sizes and calling convention
    //
    struct
    {
        UINT32 CallCode        : 14; // Least significant bits
        UINT32 IsIsolated      : 1;
        UINT32 IsExtended      : 1;
        UINT32 IsFast          : 1;  // Uses the register based form
        UINT32 VariableHeaderSize : 9; // Size in QWORDs
        UINT32 Reserved1       : 5;
        UINT32 IsNested        : 1;  // The outer hypervisor handles this call.
        UINT32 CountOfElements : 12;
        UINT32 Reserved2       : 4;
        UINT32 RepStartIndex   : 12;
        UINT32 Reserved3       : 4;  // Most significant bits
    };

    UINT64 AsUINT64;

} HV_HYPERCALL_INPUT_PRIVATE, *PHV_HYPERCALL_INPUT_PRIVATE;

//
// Bit shifts for fields in HV_HYPERCALL_INPUT_PRIVATE.
//

#define HV_HYPERCALL_INPUT_PRIVATE_IS_ISOLATED_SHIFT 14
#define HV_HYPERCALL_INPUT_PRIVATE_IS_EXTENDED_SHIFT 15

#endif

//
// Definitions for processor set structures. These are used in variable sized
// headers.
//
typedef enum _HV_GENERIC_SET_FORMAT
{
    HvGenericSetSparse4k,
    HvGenericSetAll,
    HvGenericSetInvalid
} HV_GENERIC_SET_FORMAT, *PHV_GENERIC_SET_FORMAT;

typedef struct _HV_GENERIC_SET
{
    UINT64 Format;
    UINT64 ValidBanksMask;
    UINT64 BankContents[];
} HV_GENERIC_SET, *PHV_GENERIC_SET;

typedef HV_GENERIC_SET HV_VP_SET;
typedef HV_VP_SET* PHV_VP_SET;

//
// The following structure contains the definition of a page range.
//

#define HV_SPA_PAGE_RANGE_ADDITIONAL_PAGES_BITS 24
#define HV_SPA_PAGE_RANGE_MAX_PAGES \
    (1 << HV_SPA_PAGE_RANGE_ADDITIONAL_PAGES_BITS)

typedef union _HV_SPA_PAGE_RANGE
{
    UINT64 AsUINT64;

    //
    // Base PFN and page count starting at the PFN.
    //
    struct
    {
        UINT64 BasePfn : 64 - HV_SPA_PAGE_RANGE_ADDITIONAL_PAGES_BITS;
        UINT64 AdditionalPages : HV_SPA_PAGE_RANGE_ADDITIONAL_PAGES_BITS;
    };
} HV_SPA_PAGE_RANGE, *PHV_SPA_PAGE_RANGE;

//
// Define special implementation limit values.
//
#define HV_IMPLEMENTATION_LIMIT_NOT_REPORTED 0
#define HV_IMPLEMENTATION_LIMIT_NONE 0xFFFFFFFF

//
// Definition for the stats object types.
//

typedef enum _HV_STATS_OBJECT_TYPE
{
    //
    // Global stats objects
    //

    HvStatsObjectHypervisor       = 0x00000001,
    HvStatsObjectLogicalProcessor = 0x00000002,

    //
    // Local stats objects
    //

    HvStatsObjectPartition        = 0x00010001,
    HvStatsObjectVp               = 0x00010002

} HV_STATS_OBJECT_TYPE;

//
// Logical processors are defined by a 32-bit index
//

typedef UINT32 HV_LOGICAL_PROCESSOR_INDEX, *PHV_LOGICAL_PROCESSOR_INDEX;

#define HV_INVALID_LP_INDEX    ((UINT32) -1)

//
// Declare the IO APIC ID type.
//
typedef UINT8 HV_IOAPIC_ID, *PHV_IOAPIC_ID;

//
// Flags used for specifying the stats object when making mapping/unmapping
// stats page hypercall.
//

typedef UINT16 HV_STATS_OBJECT_FLAG;

#define HvStatsObjectSelfStats      0x0001

//
// Definitions for the stats hypercall structures.
//

typedef union _HV_STATS_OBJECT_IDENTITY
{
    //
    // HvStatsObjectHypervisor
    //

    struct
    {
        UINT64  ReservedZ0;
        UINT32  ReservedZ1;
        UINT16  ReservedZ2;
        UINT8   ReservedZ3;
        UINT8   ReservedZ4;
    } Hypervisor;

    //
    // HvStatsObjectLogicalProcessor
    //

    struct
    {
        HV_LOGICAL_PROCESSOR_INDEX      LogicalProcessorIndex;
        UINT32                          ReservedZ0;
        UINT32                          ReservedZ1;
        UINT16                          ReservedZ2;
        UINT8                           ReservedZ3;
        UINT8                           ReservedZ4;
    } LogicalProcessor;

    //
    // HvStatsObjectPartition
    //

    struct
    {
        HV_PARTITION_ID         PartitionId;
        UINT32                  ReservedZ1;
        HV_STATS_OBJECT_FLAG    Flags;
        UINT8                   ReservedZ3;
        UINT8                   ReservedZ4;
    } Partition;

    //
    // HvStatsObjectVp
    //

    struct
    {
        HV_PARTITION_ID         PartitionId;
        HV_VP_INDEX             VpIndex;
        HV_STATS_OBJECT_FLAG    Flags;
        UINT8                   ReservedZ3;
        UINT8                   ReservedZ4;
    } Vp;

} HV_STATS_OBJECT_IDENTITY, *PHV_STATS_OBJECT_IDENTITY;

typedef const HV_STATS_OBJECT_IDENTITY *PCHV_STATS_OBJECT_IDENTITY;

//
// Input structure for the disable hypervisor hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_X64_INPUT_DISABLE_HYPERVISOR
{
    UINT64 TrampolineCr3;
    UINT64 KernelCr3;
    UINT64 Rip;
} HV_X64_INPUT_DISABLE_HYPERVISOR, *PHV_X64_INPUT_DISABLE_HYPERVISOR;

//
// Define the scheduler run time hypercall input/output structures.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_LOGICAL_PROCESSOR_RUN_TIME
{
    HV_LOGICAL_PROCESSOR_INDEX LpIndex;
} HV_INPUT_GET_LOGICAL_PROCESSOR_RUN_TIME,
  *PHV_INPUT_GET_LOGICAL_PROCESSOR_RUN_TIME;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_LOGICAL_PROCESSOR_RUN_TIME
{
    HV_NANO100_TIME GlobalTime;
    HV_NANO100_TIME LocalRunTime;
    UINT64          RsvdZ;
    HV_NANO100_TIME HypervisorTime;
} HV_OUTPUT_GET_LOGICAL_PROCESSOR_RUN_TIME,
  *PHV_OUTPUT_GET_LOGICAL_PROCESSOR_RUN_TIME;

//
// This describes the various methods for changing power state.
//

typedef enum _HV_X64_PPM_IDLE_STATE_CHANGE_METHOD
{
    HvX64PowerChangeIssueHlt,
    HvX64PowerChangeReadIoThenIssueHlt,
    HvX64PowerChangeReadIo,
    HvX64PowerChangeIssueMwait

} HV_X64_PPM_IDLE_STATE_CHANGE_METHOD, *PHV_X64_PPM_IDLE_STATE_CHANGE_METHOD;

//
// This describes a recipe to take the processor to a specific low power state.
//

typedef union _HV_X64_PPM_IDLE_STATE_CONFIG
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 TypeSpecific:52;
        UINT64 ChangeType:4;    // use HV_X64_PPM_IDLE_STATE_CHANGE_METHOD values
        UINT64 CheckBM_STS:1;
        UINT64 SetBM_RST:1;
        UINT64 ClearBM_RST:1;
        UINT64 SetARB_DIS:1;
        UINT64 ReservedZ:4;
    };

    struct
    {
        UINT64 ReservedZ:52;
    } Hlt;

    struct
    {
        UINT16 Port;
        UINT16 ReservedZ1;
        UINT32 ReservedZ2:20;
    } Io;

    struct
    {
        UINT32 Hints;
        UINT32 BreakOnMaskedInterrupt:1;
        UINT32 ReservedZ:19;
    } Mwait;

} HV_X64_PPM_IDLE_STATE_CONFIG, *PHV_X64_PPM_IDLE_STATE_CONFIG;

//
// These describe power states available on the processor.
//

typedef struct _HV_X64_PPM_IDLE_STATE
{
    HV_X64_PPM_IDLE_STATE_CONFIG Config;
    UINT32 Type;
    UINT32 HardwareLatency;
    UINT32 PowerConsumption;

} HV_X64_PPM_IDLE_STATE, *PHV_X64_PPM_IDLE_STATE;

#define HV_PPM_IDLE_STATE_MAX_COUNT 16

typedef struct _HV_X64_PPM_IDLE_STATE_CONFIG_PROPERTY
{
    UINT32 StateCount;
    HV_X64_PPM_IDLE_STATE States[HV_PPM_IDLE_STATE_MAX_COUNT];

} HV_X64_PPM_IDLE_STATE_CONFIG_PROPERTY, *PHV_X64_PPM_IDLE_STATE_CONFIG_PROPERTY;

//
// These describe method to transition to performance, throttle or PCC state.
//

typedef enum _HV_X64_PPM_PERF_STATE_REGISTER_TYPE
{
    HvX64PerfStateRegisterNone,
    HvX64PerfStateRegisterMsr,
    HvX64PerfStateRegisterIo,
    HvX64PerfStateRegisterMemory

} HV_X64_PPM_PERF_STATE_REGISTER_TYPE;

typedef enum _HV_X64_PPM_PERF_STATE_PORT_ACCESS_SIZE
{
    HvX64PerfStatePort8Bit,
    HvX64PerfStatePort16Bit,
    HvX64PerfStatePort32Bit

} HV_X64_PPM_PERF_STATE_PORT_ACCESS_SIZE;

typedef union _HV_X64_PPM_PERF_STATE_REGISTER
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 TypeSpecific:52;
        UINT64 RegisterType:12; // use HV_X64_PPM_PERF_STATE_REGISTER_TYPE values
    };

    struct
    {
        UINT32 RegisterNumber;
        UINT8 BitWidth;         // only used for feedback counters.
        UINT8 BitOffset;        // only used for feedback counters.
        UINT16 ReservedZ:4;
    } Msr;

    struct
    {
        UINT16 Port;
        UINT16 AccessSize;      // use HV_X64_PPM_PERF_STATE_PORT_ACCESS_SIZE values
        UINT8 BitWidth;         // only used for feedback counters.
        UINT8 BitOffset;        // only used for feedback counters.
        UINT16 ReservedZ:4;
    } Io;

    struct
    {
        UINT64 Address:52;      // x64 has a 52 bit physical address space
    } Memory;

} HV_X64_PPM_PERF_STATE_REGISTER, *PHV_X64_PPM_PERF_STATE_REGISTER;

//
// This describes how the performance/throttle/PCC state decision
// is coordinated within the domain.
//

typedef enum _HV_X64_PPM_PERF_DOMAIN_COORDINATION
{
    HvX64PerfDomainCoordinationSwAll,
    HvX64PerfDomainCoordinationSwAny,
    HvX64PerfDomainCoordinationHwAll

} HV_X64_PPM_PERF_DOMAIN_COORDINATION;

typedef struct _HV_X64_PPM_FEEDBACK_REGISTER_PAIR
{
    HV_X64_PPM_PERF_STATE_REGISTER ReferenceCount;
    HV_X64_PPM_PERF_STATE_REGISTER ActualCount;
    BOOLEAN ResetRegisters;

} HV_X64_PPM_FEEDBACK_REGISTER_PAIR, *PHV_X64_PPM_FEEDBACK_REGISTER_PAIR;

//
// These describe performance states available on the processor and
// the domain topology.
//

typedef struct _HV_X64_PPM_PERF_STATE
{
    UINT64 Control;
    UINT64 Status;
    UINT64 ControlMask;
    UINT64 StatusMask;

    UINT32 Frequency;
    UINT32 TransitionLatency;
    UINT32 BusMasterLatency;
    UINT32 PowerConsumption;

} HV_X64_PPM_PERF_STATE, *PHV_X64_PPM_PERF_STATE;

#define HV_PPM_PERF_STATE_MAX_COUNT 64

typedef struct _HV_X64_PPM_PERF_STATE_CONFIG_PROPERTY
{
    UINT32 StateCount;
    HV_X64_PPM_PERF_STATE States[HV_PPM_PERF_STATE_MAX_COUNT];

    UINT32 DomainId;
    UINT32 NumProcInDomain;
    HV_X64_PPM_PERF_DOMAIN_COORDINATION Coordination;

    HV_X64_PPM_PERF_STATE_REGISTER ControlRegister;
    HV_X64_PPM_PERF_STATE_REGISTER StatusRegister;

} HV_X64_PPM_PERF_STATE_CONFIG_PROPERTY, *PHV_X64_PPM_PERF_STATE_CONFIG_PROPERTY;

//
// These describe throttle states available on the processor and
// the domain topology.
//

typedef struct _HV_X64_PPM_THROTTLE_STATE
{
    UINT64 Control;
    UINT64 Status;

    UINT32 PercentFrequency;
    UINT32 TransitionLatency;
    UINT32 PowerConsumption;

} HV_X64_PPM_THROTTLE_STATE, *PHV_X64_PPM_THROTTLE_STATE;

#define HV_PPM_THROTTLE_STATE_MAX_COUNT 100

typedef struct _HV_X64_PPM_THROTTLE_STATE_CONFIG_PROPERTY
{
    UINT32 StateCount;
    HV_X64_PPM_THROTTLE_STATE States[HV_PPM_THROTTLE_STATE_MAX_COUNT];

    UINT32 DomainId;
    UINT32 NumProcInDomain;
    HV_X64_PPM_PERF_DOMAIN_COORDINATION Coordination;

    HV_X64_PPM_PERF_STATE_REGISTER ControlRegister;
    HV_X64_PPM_PERF_STATE_REGISTER StatusRegister;
    UINT64 ControlMask;
    UINT64 StatusMask;

} HV_X64_PPM_THROTTLE_STATE_CONFIG_PROPERTY, *PHV_X64_PPM_THROTTLE_STATE_CONFIG_PROPERTY;

//
// These describe PCC configuration on the system and the domain topology.
//

#define HV_PPM_PCC_DISABLE 0x00000001

typedef struct _HV_X64_PPM_PCC_CONFIG_PROPERTY
{
    UINT32 Flags;

    UINT32 DomainId;
    UINT32 NumProcInDomain;
    HV_X64_PPM_PERF_DOMAIN_COORDINATION Coordination;

    UINT64 SharedMemoryAddress;
    UINT32 SharedMemoryLength;
    HV_X64_PPM_PERF_STATE_REGISTER Doorbell;
    UINT64 DoorbellPreserve;
    UINT64 DoorbellWrite;
    UINT64 DoorbellMask;

    UINT32 InputBufferOffset;
    UINT32 OutputBufferOffset;

} HV_X64_PPM_PCC_CONFIG_PROPERTY, *PHV_X64_PPM_PCC_CONFIG_PROPERTY;

//
// This describes caps for performance/throttle/PCC state.
//

typedef struct _HV_X64_PPM_PERF_STATE_CAP_PROPERTY
{
    UINT32 PStateCap;
    UINT32 TStateCap;
    UINT32 ThermalCap;

} HV_X64_PPM_PERF_STATE_CAP_PROPERTY, *PHV_X64_PPM_PERF_STATE_CAP_PROPERTY;

//
// These describe power policy.
//

typedef enum _HV_PPM_POWER_POLICY_SETTING_ID
{
    HvPowerPolicyIdleDisable,
    HvPowerPolicyIdleTimeCheck,
    HvPowerPolicyIdlePromoteThreshold,
    HvPowerPolicyIdleDemoteThreshold,
    HvPowerPolicyIdleStateMaximum,

    HvPowerPolicyThrottleMaximum,
    HvPowerPolicyThrottleMinimum,
    HvPowerPolicyPerfIncreaseThreshold,
    HvPowerPolicyPerfDecreaseThreshold,
    HvPowerPolicyPerfIncreasePolicy,
    HvPowerPolicyPerfDecreasePolicy,
    HvPowerPolicyCoreParkingIncreasePolicy,
    HvPowerPolicyCoreParkingDecreasePolicy,
    HvPowerPolicyCoreParkingMaxCores,
    HvPowerPolicyCoreParkingMinCores,
    HvPowerPolicyPerfTimeCheck,
    HvPowerPolicyPerfIncreaseTime,
    HvPowerPolicyPerfDecreaseTime,
    HvPowerPolicyPerfBoostPolicy,
    HvPowerPolicyPerfBoostMode,
    HvPowerPolicyMax

} HV_PPM_POWER_POLICY_SETTING_ID;

typedef struct _HV_PPM_POWER_POLICY_SETTING
{
    HV_PPM_POWER_POLICY_SETTING_ID SettingId;
    UINT32 Value;

} HV_PPM_POWER_POLICY_SETTING, *PHV_PPM_POWER_POLICY_SETTING;

#define HV_X64_MSR_PERF_FEEDBACK_TRIGGER    HvSyntheticMsrPerfFeedbackTrigger

//
// This describes machine Check context info property..
//

typedef enum _HV_MACHINE_CHECK_SOURCE
{
    HV_MACHINE_CHECK_SOURCE_NONE = 0,
    HV_MACHINE_CHECK_SOURCE_HV,
    HV_MACHINE_CHECK_SOURCE_ROOT_VP,
    HV_MACHINE_CHECK_SOURCE_NON_ROOT_VP

} HV_MACHINE_CHECK_SOURCE, *PHV_MACHINE_CHECK_SOURCE;

typedef union _HV_MACHINE_CHECK_CONTEXT
{
    struct
    {
        HV_PARTITION_ID PartitionId;
        HV_VP_INDEX VpIndex;
        HV_VTL Vtl;
    };
} HV_MACHINE_CHECK_CONTEXT, *PHV_MACHINE_CHECK_CONTEXT;

typedef struct _HV_MACHINE_CHECK_CONTEXT_INFO
{
    HV_MACHINE_CHECK_SOURCE MachineCheckSource;
    HV_MACHINE_CHECK_CONTEXT MachineCheckContext;

} HV_MACHINE_CHECK_CONTEXT_INFO, *PHV_MACHINE_CHECK_CONTEXT_INFO;

//
// Structure def for imported microcode update data.
//

typedef struct _HV_MCUPDATE_DATA_HEADER
{
    UINT32  DataSize;
    UINT32 TableOffset;
} HV_MCUPDATE_DATA_HEADER, *PHV_MCUPDATE_DATA_HEADER;

//
// Structure def for microcode update status.
//

typedef struct _HV_MCUPDATE_UPDATE_STATUS
{
    BOOLEAN Valid;
    UINT32 UpdateLoadStatus;
    UINT64 UpdateRevision;
    UINT64 PreviousUpdateRevision;
    UINT32 PlatformSpecificField1;
    UINT32 PlatformSpecificField2;
} HV_MCUPDATE_UPDATE_STATUS, *PHV_MCUPDATE_UPDATE_STATUS;

typedef struct _HV_PROCESSOR_PERF_FEEDBACK_COUNTERS_CONFIG
{
    UINT32 Count;
    HV_X64_PPM_FEEDBACK_REGISTER_PAIR FeedbackCounters[4];

} HV_PROCESSOR_PERF_FEEDBACK_COUNTERS_CONFIG,
  *PHV_PROCESSOR_PERF_FEEDBACK_COUNTERS_CONFIG;

//
// Definition of the set/get logical processor property hypercall structures.
//

typedef enum _HV_LOGICAL_PROCESSOR_PROPERTY_TYPE
{
    HvLogicalProcessorPerfStateConfig,
    HvLogicalProcessorThrottleStateConfig,
    HvLogicalProcessorPccConfig,
    HvLogicalProcessorPerfStateCap,
    HvLogicalProcessorMachineCheckContextInfo,
    HvLogicalProcessorMcUpdateUpdateStatus,

} HV_LOGICAL_PROCESSOR_PROPERTY_TYPE;

typedef union _HV_LOGICAL_PROCESSOR_PROPERTY
{

#if !defined(XBOX_SYSTEMOS)

    // Performance state configuration property.
    HV_X64_PPM_PERF_STATE_CONFIG_PROPERTY PerfStateConfig;

    // Throttle state configuration property.
    HV_X64_PPM_THROTTLE_STATE_CONFIG_PROPERTY ThrottleStateConfig;

    // PCC configuration property.
    HV_X64_PPM_PCC_CONFIG_PROPERTY PccConfig;

    // Perfromance/Throttle/PCC cap property.
    HV_X64_PPM_PERF_STATE_CAP_PROPERTY PerfStateCap;

#endif

    // Machine Check context info property.
    HV_MACHINE_CHECK_CONTEXT_INFO MachineCheckContextInfo;

    // Microcode update status information.
    HV_MCUPDATE_UPDATE_STATUS UpdateStatus;

} HV_LOGICAL_PROCESSOR_PROPERTY, *PHV_LOGICAL_PROCESSOR_PROPERTY;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SET_LOGICAL_PROCESSOR_PROPERTY
{
    HV_LOGICAL_PROCESSOR_INDEX LpIndex;
    HV_LOGICAL_PROCESSOR_PROPERTY_TYPE Type;
    HV_LOGICAL_PROCESSOR_PROPERTY Property;
} HV_INPUT_SET_LOGICAL_PROCESSOR_PROPERTY, *PHV_INPUT_SET_LOGICAL_PROCESSOR_PROPERTY;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_LOGICAL_PROCESSOR_PROPERTY
{
    HV_LOGICAL_PROCESSOR_INDEX LpIndex;
    HV_LOGICAL_PROCESSOR_PROPERTY_TYPE Type;
} HV_INPUT_GET_LOGICAL_PROCESSOR_PROPERTY, *PHV_INPUT_GET_LOGICAL_PROCESSOR_PROPERTY;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_LOGICAL_PROCESSOR_PROPERTY
{
    HV_LOGICAL_PROCESSOR_PROPERTY Property;
} HV_OUTPUT_GET_LOGICAL_PROCESSOR_PROPERTY, *PHV_OUTPUT_GET_LOGICAL_PROCESSOR_PROPERTY;


typedef enum _HV_PLATFORM_STATE_INDEX
{
    HvPlatformStateNonIdle,
    HvPlatformStateIdle

} HV_PLATFORM_STATE_INDEX;

#if !defined(XBOX_SYSTEMOS)

//
// Definition of the set/get logical processor power property hypercall structures.
//

typedef enum _HV_POWER_PROPERTY_TYPE
{
    HvPowerPropertyLpIdleStateConfig,
    HvPowerPropertyLpPerfFeedbackCounters,
    HvPowerPropertyLpPercentageFrequency,
    HvPowerPropertyLpNextPlatformStateIndex,
    HvPowerPropertyStatsOffsets,
    HvPowerPropertyPolicySetting

} HV_POWER_PROPERTY_TYPE;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SET_POWER_PROPERTY
{
    HV_POWER_PROPERTY_TYPE Type;

    union
    {
        struct
        {
            HV_LOGICAL_PROCESSOR_INDEX LpIndex;

            union
            {
                HV_X64_PPM_IDLE_STATE_CONFIG_PROPERTY IdleStateConfig;

                UINT32 PercentFrequency;

                HV_PLATFORM_STATE_INDEX NextPlatformStateIndex;

                HV_PROCESSOR_PERF_FEEDBACK_COUNTERS_CONFIG FeedbackCounterConfig;
            };

        } Processor;

        union
        {
            HV_PPM_POWER_POLICY_SETTING Policy;

        } Global;
    };

} HV_INPUT_SET_POWER_PROPERTY, *PHV_INPUT_SET_POWER_PROPERTY;

typedef struct _HV_POWER_STATS_OFFSETS
{
    UINT32 GlobalTime;
    UINT32 TotalRunTime;
    UINT32 IdleSequenceNumber;
    UINT32 GlobalTscCount;
    UINT32 ActiveTscCount;
    UINT32 IdleAccumulation;
    struct
    {
        UINT32 Reference;
        UINT32 Actual;

    } CycleCount[2];

} HV_POWER_STATS_OFFSETS, *PHV_POWER_STATS_OFFSETS;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_POWER_PROPERTY
{
    HV_POWER_PROPERTY_TYPE Type;

    union
    {
        union
        {
            HV_PPM_POWER_POLICY_SETTING_ID PolicySettingId;

        } Global;

        struct
        {
            HV_LOGICAL_PROCESSOR_INDEX LpIndex;

        } Processor;

    };

} HV_INPUT_GET_POWER_PROPERTY, *PHV_INPUT_GET_POWER_PROPERTY;

typedef union HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_POWER_PROPERTY
{
    HV_X64_PPM_IDLE_STATE_CONFIG_PROPERTY IdleStateConfig;

    UINT32 PercentFrequency;

    UINT32 NextPlatformStateIndex;

    HV_PROCESSOR_PERF_FEEDBACK_COUNTERS_CONFIG FeedbackCounterConfig;

    struct
    {
        UINT32 Value;

    } Policy;

    HV_POWER_STATS_OFFSETS StatsOffsets;
} HV_OUTPUT_GET_POWER_PROPERTY, *PHV_OUTPUT_GET_POWER_PROPERTY;

#endif

//
// Partition Properties
//
typedef UINT64 HV_PARTITION_PROPERTY, *PHV_PARTITION_PROPERTY;

typedef enum
{
    //
    // Privilege properties
    //
    HvPartitionPropertyPrivilegeFlags              = 0x00010000,

    //
    // Scheduling properties
    //
    HvPartitionPropertySuspend                     = 0x00020000,
    HvPartitionPropertyCpuReserve                  = 0x00020001,
    HvPartitionPropertyCpuCap                      = 0x00020002,
    HvPartitionPropertyCpuWeight                   = 0x00020003,
    HvPartitionPropertyCpuGroupId                  = 0x00020004,

    //
    // Time properties
    //
    HvPartitionPropertyTimeFreeze                  = 0x00030003,

    //
    // Debugging properties
    //
    HvPartitionPropertyDebugChannelId              = 0x00040000,

    //
    // Resource properties
    //
    HvPartitionPropertyVirtualTlbPageCount         = 0x00050000,
    HvPartitionPropertyVsmConfig                   = 0x00050001,
    HvPartitionPropertyZeroMemoryOnReset           = 0x00050002,
    HvPartitionPropertyProcessorsPerSocket         = 0x00050003,
    HvPartitionPropertyNestedTlbSize               = 0x00050004,
    HvPartitionPropertyGpaPageAccessTracking       = 0x00050005,
    HvPartitionPropertyVsmPermissionsDirtySinceLastQuery = 0x00050006,
    HvPartitionPropertySgxLaunchControlConfig      = 0x00050007,
    HvPartitionPropertyDefaultSgxLaunchControl0    = 0x00050008,
    HvPartitionPropertyDefaultSgxLaunchControl1    = 0x00050009,
    HvPartitionPropertyDefaultSgxLaunchControl2    = 0x0005000A,
    HvPartitionPropertyDefaultSgxLaunchControl3    = 0x0005000B,
    HvPartitionPropertyIsolationState              = 0x0005000C,
    HvPartitionPropertyIsolationControl            = 0x0005000D,
    HvPartitionPropertyRdtL3CosIndex               = 0x0005000E,
    HvPartitionPropertyRdtRmid                     = 0x0005000F,

    //
    // Compatibility properties
    //
    HvPartitionPropertyProcessorVendor             = 0x00060000,
    HvPartitionPropertyProcessorFeatures           = 0x00060001,
    HvPartitionPropertyProcessorXsaveFeatures      = 0x00060002,
    HvPartitionPropertyProcessorCLFlushSize        = 0x00060003,
    HvPartitionPropertyEnlightenmentModifications  = 0x00060004,
    HvPartitionPropertyCompatibilityVersion        = 0x00060005,
    HvPartitionPropertyPhysicalAddressWidth        = 0x00060006,

    //
    // Guest software properties
    //
    HvPartitionPropertyGuestOsId                   = 0x00070000,

    //
    // Nested virtualization properties
    //
    HvPartitionPropertyProcessorVirtualizationFeatures = 0x00080000,

} HV_PARTITION_PROPERTY_CODE, *PHV_PARTITION_PROPERTY_CODE;

//
// Declare the input structure for the HvSetPartitionProperty hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SET_PARTITION_PROPERTY
{
    HV_PARTITION_ID             PartitionId;
    HV_PARTITION_PROPERTY_CODE  PropertyCode;
    HV_PARTITION_PROPERTY       PropertyValue;

} HV_INPUT_SET_PARTITION_PROPERTY, *PHV_INPUT_SET_PARTITION_PROPERTY;

//
// Definition of the HvCallQueryAssociatedLpsForMca hypercall.input/output
// structures.
// This call queries the associated LPs for a given VP for MCE and CMCI
// initialization and handling.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_QUERY_ASSOCIATED_LP_FOR_MCA
{
    HV_VP_INDEX VpIndex;
} HV_INPUT_QUERY_ASSOCIATED_LP_FOR_MCA, *PHV_INPUT_QUERY_ASSOCIATED_LP_FOR_MCA;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_QUERY_ASSOCIATED_LP_FOR_MCA
{
    UINT32 Count;
    UINT32 AssociatedLpList[HV_MAXIMUM_PROCESSORS];
} HV_OUTPUT_QUERY_ASSOCIATED_LP_FOR_MCA, *PHV_OUTPUT_QUERY_ASSOCIATED_LP_FOR_MCA;


//
// Hypervisor synthetic MSRs on AMD for querying machine check faulting LP and
// for notifying machine check faulting LP that has been handled.
//

#define HV_X64_AMD_MSR_MCA_FAULTING_LP_INDEX      (0x40000030)
#define HV_X64_AMD_MSR_MCA_FAULTING_LP_HANDLED    (0x40000031)

typedef union _HV_MCA_FAULTING_LP_MSR
{
    UINT64 AsUINT64;
    struct
    {
        HV_LOGICAL_PROCESSOR_INDEX LpIndex;
        UINT32 Reserved;
    };
} HV_MCA_FAULTING_LP_MSR, *PHV_MCA_FAULTING_LP_MSR;

//
// Definition of the HvEnterSleepState hypercall.input structure.  This
// call allows a partition to enter a specified sleep state supported by
// hypervisor.
//

#define HV_MAX_SLEEP_STATE 5

typedef enum _HV_SLEEP_STATE
{
    HvSleepStateS1 = 1,
    HvSleepStateS2 = 2,
    HvSleepStateS3 = 3,
    HvSleepStateS4 = 4,
    HvSleepStateS5 = 5,
    HvSleepStateLock = 6
} HV_SLEEP_STATE, *PHV_SLEEP_STATE;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_ENTER_SLEEP_STATE
{
    HV_SLEEP_STATE SleepState;
} HV_INPUT_ENTER_SLEEP_STATE, *PHV_INPUT_ENTER_SLEEP_STATE;

//
// Definition of the HvPrepareForSleep hypercall.input structure.  This
// call notifies hypervisor to prepare for impending sleep state transition.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_PREPARE_FOR_SLEEP
{
    BOOLEAN Entering;
    BOOLEAN ModernStandby;
} HV_INPUT_PREPARE_FOR_SLEEP, *PHV_INPUT_PREPARE_FOR_SLEEP;

//
// Definition of the HvPrepareForHibernate hypercall.input/output structures.
// This call notifies hypervisor to prepare for impending hibernate state
// transition.
//

#if defined(_ARM64_) || defined(_ARM_)

typedef struct _HV_ARM64_PROC_STATE_GP_REGS
{
    //
    // ARM64HV_WORKITEM
    //  Specifics during implementation.  Pretty sure these are just nonvolatile regs
    //
    UINT64 R16;
    UINT64 R17;
    UINT64 R18;
    UINT64 R19;
    UINT64 R20;
    UINT64 R21;
    UINT64 R22;
    UINT64 R23;
    UINT64 R24;
    UINT64 R25;
    UINT64 R26;
    UINT64 R27;
    UINT64 R28;
    UINT64 R29;
    UINT64 R30;
} HV_ARM64_PROC_STATE_GP_REGS, *PHV_ARM64_PROC_STATE_GP_REGS;

typedef struct _HV_ARM64_VOLATILE_GP_REGS
{
    //
    // ARM64HV_WORKITEM
    //
    // Dummy field to allow compilation.
    //

    UINT64 RsvdZ;
} HV_ARM64_VOLATILE_GP_REGS, *PHV_ARM64_VOLATILE_GP_REGS;

#define _HV_PROC_STATE_GP_REGS _HV_ARM64_PROC_STATE_GP_REGS
#define  HV_PROC_STATE_GP_REGS  HV_ARM64_PROC_STATE_GP_REGS
#define PHV_PROC_STATE_GP_REGS PHV_ARM64_PROC_STATE_GP_REGS
#define  HV_VOLATILE_GP_REGS    HV_ARM64_VOLATILE_GP_REGS
#define PHV_VOLATILE_GP_REGS   PHV_ARM64_VOLATILE_GP_REGS

#elif defined(_AMD64_) || defined(_X86_)

typedef struct _HV_X64_PROC_STATE_GP_REGS
{
    UINT64 Rip;
    UINT64 Rsp;

    UINT64 Rbp;
    UINT64 Rbx;
    UINT64 Rdi;
    UINT64 Rsi;
    UINT64 R12;
    UINT64 R13;
    UINT64 R14;
    UINT64 R15;
} HV_X64_PROC_STATE_GP_REGS, *PHV_X64_PROC_STATE_GP_REGS;

typedef struct _HV_X64_VOLATILE_GP_REGS
{
    UINT64 Rax;
    UINT64 Rcx;
    UINT64 Rdx;
    UINT64 R8;
    UINT64 R9;
    UINT64 R10;
    UINT64 R11;
} HV_X64_VOLATILE_GP_REGS, *PHV_X64_VOLATILE_GP_REGS;

#define _HV_PROC_STATE_GP_REGS _HV_X64_PROC_STATE_GP_REGS
#define  HV_PROC_STATE_GP_REGS  HV_X64_PROC_STATE_GP_REGS
#define PHV_PROC_STATE_GP_REGS PHV_X64_PROC_STATE_GP_REGS
#define  HV_VOLATILE_GP_REGS    HV_X64_VOLATILE_GP_REGS
#define PHV_VOLATILE_GP_REGS   PHV_X64_VOLATILE_GP_REGS

#else

#error Unknown/Unsupported architecture

#endif

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_PREPARE_FOR_HIBERNATE
{
    HV_PROC_STATE_GP_REGS  ResumeGpRegs;
    HV_VTL RestrictApVtl;
} HV_INPUT_PREPARE_FOR_HIBERNATE, *PHV_INPUT_PREPARE_FOR_HIBERNATE;

#if defined(_ARM64_) || defined(_ARM_)

typedef struct HV_CALL_ATTRIBUTES _HV_ARM64_OUTPUT_PREPARE_FOR_HIBERNATE
{
    //TODO: Specifics during implementation.
    UINT64 HvTTBR;
    UINT64 HvEntryPoint;
    UINT64 HvReservedTransitionAddress;
    UINT64 HvReservedTransitionAddressSize;
} HV_ARM64_OUTPUT_PREPARE_FOR_HIBERNATE, *PHV_ARM64_OUTPUT_PREPARE_FOR_HIBERNATE;

#define _HV_OUTPUT_PREPARE_FOR_HIBERNATE _HV_ARM64_OUTPUT_PREPARE_FOR_HIBERNATE
#define  HV_OUTPUT_PREPARE_FOR_HIBERNATE  HV_ARM64_OUTPUT_PREPARE_FOR_HIBERNATE
#define PHV_OUTPUT_PREPARE_FOR_HIBERNATE PHV_ARM64_OUTPUT_PREPARE_FOR_HIBERNATE

#elif defined(_AMD64_) || defined(_X86_)

typedef struct HV_CALL_ATTRIBUTES _HV_X64_OUTPUT_PREPARE_FOR_HIBERNATE
{
    UINT64 HvCr3;
    UINT64 HvEntryPoint;
    UINT64 HvReservedTransitionAddress;
    UINT64 HvReservedTransitionAddressSize;
} HV_X64_OUTPUT_PREPARE_FOR_HIBERNATE, *PHV_X64_OUTPUT_PREPARE_FOR_HIBERNATE;

#define _HV_OUTPUT_PREPARE_FOR_HIBERNATE _HV_X64_OUTPUT_PREPARE_FOR_HIBERNATE
#define  HV_OUTPUT_PREPARE_FOR_HIBERNATE  HV_X64_OUTPUT_PREPARE_FOR_HIBERNATE
#define PHV_OUTPUT_PREPARE_FOR_HIBERNATE PHV_X64_OUTPUT_PREPARE_FOR_HIBERNATE

#else

#error Unknown/Unsupported architecture

#endif

//
// Definition of the HvGet/SetLogicalProcessorRegisters structures.
// These calls retrieve a logical processor's CPUID and read/write MSRs.
//

typedef enum _HV_LOGICAL_PROCESSOR_REGISTER_TYPE
{
    // CPUID
    HvX64LpRegisterTypeCpuid    = 0x00010000,

    // MSR
    HvX64LpRegisterTypeMsr      = 0x00010001,

    // WBINVD
    HvX64LpRegisterTypeWbinvd   = 0x00010002

    //TODO: Add HvArm64 register type(s)

} HV_LOGICAL_PROCESSOR_REGISTER_TYPE, *PHV_LOGICAL_PROCESSOR_REGISTER_TYPE;

typedef const HV_LOGICAL_PROCESSOR_REGISTER_TYPE
        *PCHV_LOGICAL_PROCESSOR_REGISTER_TYPE;

#if defined(_ARM64_) || defined(_ARM_)

typedef union _HV_ARM64_LOGICAL_PROCESSOR_REGISTER_ADDRESS
{
    //TODO: Specifics during implementation.
    // System Register Index
    struct
    {
        UINT32 SysRegIx;
    };

    // Coprocessor Register Index
    struct
    {
        UINT32 CoProcIx;
        UINT32 CoProcRegIx;
    };

} HV_ARM64_LOGICAL_PROCESSOR_REGISTER_ADDRESS, *PHV_ARM64_LOGICAL_PROCESSOR_REGISTER_ADDRESS;

typedef const HV_ARM64_LOGICAL_PROCESSOR_REGISTER_ADDRESS *PCHV_ARM64_LOGICAL_PROCESSOR_REGISTER_ADDRESS;

#define  _HV_LOGICAL_PROCESSOR_REGISTER_ADDRESS  _HV_ARM64_LOGICAL_PROCESSOR_REGISTER_ADDRESS
#define   HV_LOGICAL_PROCESSOR_REGISTER_ADDRESS   HV_ARM64_LOGICAL_PROCESSOR_REGISTER_ADDRESS
#define  PHV_LOGICAL_PROCESSOR_REGISTER_ADDRESS  PHV_ARM64_LOGICAL_PROCESSOR_REGISTER_ADDRESS
#define PCHV_LOGICAL_PROCESSOR_REGISTER_ADDRESS PCHV_ARM64_LOGICAL_PROCESSOR_REGISTER_ADDRESS

#elif defined(_AMD64_) || defined(_X86_)

typedef union _HV_X64_LOGICAL_PROCESSOR_REGISTER_ADDRESS
{
    // CPUID address
    struct
    {
        UINT32 Eax;
        UINT32 Ecx;
    };

    // MSR address
    struct
    {
        UINT32 MsrIndex;
    };

} HV_X64_LOGICAL_PROCESSOR_REGISTER_ADDRESS, *PHV_X64_LOGICAL_PROCESSOR_REGISTER_ADDRESS;

typedef const HV_X64_LOGICAL_PROCESSOR_REGISTER_ADDRESS *PCHV_X64_LOGICAL_PROCESSOR_REGISTER_ADDRESS;

#define  _HV_LOGICAL_PROCESSOR_REGISTER_ADDRESS  _HV_X64_LOGICAL_PROCESSOR_REGISTER_ADDRESS
#define   HV_LOGICAL_PROCESSOR_REGISTER_ADDRESS   HV_X64_LOGICAL_PROCESSOR_REGISTER_ADDRESS
#define  PHV_LOGICAL_PROCESSOR_REGISTER_ADDRESS  PHV_X64_LOGICAL_PROCESSOR_REGISTER_ADDRESS
#define PCHV_LOGICAL_PROCESSOR_REGISTER_ADDRESS PCHV_X64_LOGICAL_PROCESSOR_REGISTER_ADDRESS

#else

#error Unknown/Unsupported architecture

#endif

typedef struct _HV_LOGICAL_PROCESSOR_REGISTER_ID
{
    HV_LOGICAL_PROCESSOR_INDEX              LpIndex;
    HV_LOGICAL_PROCESSOR_REGISTER_TYPE      Type;
    HV_LOGICAL_PROCESSOR_REGISTER_ADDRESS   Address;

} HV_LOGICAL_PROCESSOR_REGISTER_ID, *PHV_LOGICAL_PROCESSOR_REGISTER_ID;

typedef const HV_LOGICAL_PROCESSOR_REGISTER_ID
        *PCHV_LOGICAL_PROCESSOR_REGISTER_ID;

typedef union _HV_LOGICAL_PROCESSOR_REGISTER_VALUE
{
    HV_UINT128          Reg128;
    UINT64              Reg64;
    UINT32              Reg32;
    UINT16              Reg16;
    UINT8               Reg8;

#if !defined(_ARM64_) && !defined(_ARM_)

    HV_CPUID_RESULT     Cpuid;

#endif

} HV_LOGICAL_PROCESSOR_REGISTER_VALUE, *PHV_LOGICAL_PROCESSOR_REGISTER_VALUE;

typedef const HV_LOGICAL_PROCESSOR_REGISTER_VALUE
        *PCHV_LOGICAL_PROCESSOR_REGISTER_VALUE;

typedef struct _HV_LOGICAL_PROCESSOR_REGISTER_ASSOC
{
    HV_LOGICAL_PROCESSOR_REGISTER_ID        Id;
    HV_LOGICAL_PROCESSOR_REGISTER_VALUE     Value;

} HV_LOGICAL_PROCESSOR_REGISTER_ASSOC, *PHV_LOGICAL_PROCESSOR_REGISTER_ASSOC;

typedef const HV_LOGICAL_PROCESSOR_REGISTER_ASSOC
        *PCHV_LOGICAL_PROCESSOR_REGISTER_ASSOC;

//
// Definitions for IOMMU support.
//

#define HV_PCI_FUNCTION_NUMBER_BIT_COUNT    0x3
#define HV_PCI_DEVICE_NUMBER_BIT_COUNT      0x5

typedef UINT16 HV_PCI_RID, *PHV_PCI_RID;
typedef UINT16 HV_PCI_SEGMENT, *PHV_PCI_SEGMENT;

typedef union _HV_PCI_BDF
{
    UINT16 AsUINT16;

    struct
    {
        UINT8 Function:3;
        UINT8 Device:5;
        UINT8 Bus;
    };

} HV_PCI_BDF, *PHV_PCI_BDF;

typedef union _HV_PCI_ID
{
    UINT32 AsUINT32;

    struct
    {
        union
        {
            HV_PCI_RID Rid;
            HV_PCI_BDF Bdf;
        };

        HV_PCI_SEGMENT Segment;
    };

} HV_PCI_ID, *PHV_PCI_ID;

typedef union _HV_PCI_BUS_RANGE
{
    UINT16 AsUINT16;

    struct
    {
        UINT8 SubordinateBus;
        UINT8 SecondaryBus;
    };

} HV_PCI_BUS_RANGE;

typedef enum _HV_DEVICE_TYPE
{
    HV_DEVICE_TYPE_LOGICAL = 0,
    HV_DEVICE_TYPE_PCI = 1,
#if defined(_AMD64_)
    HV_DEVICE_TYPE_IOAPIC = 2,
#elif defined(_ARM64_)
    HV_DEVICE_TYPE_GIC = 2,
#endif
    HV_DEVICE_TYPE_ACPI = 3,

} HV_DEVICE_TYPE, *PHV_DEVICE_TYPE;

#define HV_SOURCE_SHADOW_NONE               0x0
#define HV_SOURCE_SHADOW_BRIDGE_BUS_RANGE   0x1

typedef union _HV_DEVICE_ID
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 : 62;
        UINT64 DeviceType : 2;
    };

    // HV_DEVICE_TYPE_LOGICAL
    struct
    {
        UINT64 Id : 62;
        UINT64 DeviceType : 2;

    } Logical;

    // HV_DEVICE_TYPE_PCI
    struct
    {
        union
        {
            HV_PCI_RID Rid;
            HV_PCI_BDF Bdf;
        };

        HV_PCI_SEGMENT Segment;
        HV_PCI_BUS_RANGE ShadowBusRange;

        UINT16 PhantomFunctionBits : 2;
        UINT16 SourceShadow : 1;

        UINT16 RsvdZ0 : 11;
        UINT16 DeviceType : 2;
    } Pci;

#if defined(_AMD64_)

    // HV_DEVICE_TYPE_IOAPIC
    struct
    {
        UINT8 IoApicId;
        UINT8 RsvdZ0;
        UINT16 RsvdZ1;
        UINT16 RsvdZ2;

        UINT16 RsvdZ3 : 14;
        UINT16 DeviceType : 2;
    } IoApic;

#elif defined(_ARM64_)

    // HV_DEVICE_TYPE_GIC
    struct
    {
        UINT32 LineNumber;
        UINT16 RsvdZ0;

        UINT16 RsvdZ1 : 14;
        UINT16 DeviceType : 2;
    } Gic;

#endif

    // HV_DEVICE_TYPE_ACPI
    struct
    {
        UINT32 InputMappingBase;
        UINT32 InputMappingsCount : 30;
        UINT32 DeviceType : 2;
    } Acpi;

} HV_DEVICE_ID, *PHV_DEVICE_ID;

typedef struct _HV_PCI_PATH
{
    UINT8 PathLength;
    HV_PCI_SEGMENT Segment;
    UINT8 StartBus;
    union
    {
        struct
        {
            UINT8 Function : 3;
            UINT8 Device : 5;
        };
        UINT8 AsUINT8;

    } Path[];

} HV_PCI_PATH, *PHV_PCI_PATH;

#define HV_PCI_PATH_SIZE(_PathLength_) \
    (FIELD_OFFSET(HV_PCI_PATH, Path) + \
     (_PathLength_) * FIELD_SIZE(HV_PCI_PATH, Path[0]))

//
// IOMMU extended status record.
//

typedef struct _HV_IOMMU_EXTENDED_STATUS
{
    UINT8 Severity;
    UINT16 Status;
    UINT64 Params[4];

} HV_IOMMU_EXTENDED_STATUS, *PHV_IOMMU_EXTENDED_STATUS;

//
// IOMMU initialization status record.
//

typedef struct _HV_IOMMU_INIT_STATUS
{
    BOOLEAN HardwarePresent;
    BOOLEAN Enabled;
    UINT64 Policy;
    UINT64 Features;
    UINT64 InitStatus;
    UINT64 Errors;
    UINT64 Errata;

} HV_IOMMU_INIT_STATUS, *PHV_IOMMU_INIT_STATUS;

//
// IOMMU initialization status definitions.
//

#define HV_IOMMU_INIT_POLICY_DEFAULT    0
#define HV_IOMMU_INIT_POLICY_ENABLE     1
#define HV_IOMMU_INIT_POLICY_DISABLE    2

//
// TSC sync status recorded by Hypervisor.
//

typedef struct _HV_TSC_SYNC_STATUS
{
    BOOLEAN SyncFailed;
    BOOLEAN SyncNeeded;
    INT64 MaxDelta;
    INT64 MinDelta;
} HV_TSC_SYNC_STATUS, *PHV_TSC_SYNC_STATUS;

//
// Definition of the HvNotifyPartitionEvent hypercall.input/output structures.
// This hypercall allows a partition to notify the hypervisor that it is in
// the process of a system state change.
//

typedef enum _HV_CRASHDUMP_ACTION
{
    HvCrashdumpNone = 0,
    HvCrashdumpSuspendAllVps,
    HvCrashdumpPrepareForStateSave,
    HvCrashdumpStateSaved
} HV_CRASHDUMP_ACTION, *PHV_CRASHDUMP_ACTION;

typedef struct _HV_PARTITION_EVENT_ROOT_CRASHDUMP_INPUT
{
    HV_CRASHDUMP_ACTION CrashdumpAction;
} HV_PARTITION_EVENT_ROOT_CRASHDUMP_INPUT, *PHV_PARTITION_EVENT_ROOT_CRASHDUMP_INPUT;

typedef union _HV_PARTITION_EVENT_INPUT
{
    //
    // Input for the root crashdump partition event.
    //

    HV_PARTITION_EVENT_ROOT_CRASHDUMP_INPUT CrashdumpInput;
} HV_PARTITION_EVENT_INPUT, *PHV_PARTITION_EVENT_INPUT;

typedef enum _HV_PARTITION_EVENT
{
    HvPartitionEventDebugDeviceAvailable = 1,
    HvPartitionEventRootCrashdump = 2,
    HvPartitionEventAcpiReenabled = 3
} HV_PARTITION_EVENT, *PHV_PARTITION_EVENT;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_NOTIFY_PARTITION_EVENT
{
    HV_PARTITION_EVENT Event;
    HV_PARTITION_EVENT_INPUT Input;
} HV_INPUT_NOTIFY_PARTITION_EVENT, *PHV_INPUT_NOTIFY_PARTITION_EVENT;

//
// In a NUMA system processors, memory and devices may reside in different
// firmware described proximity domains.
//
// On a non-NUMA system everything resides in proximity domain 0.
//

typedef UINT32 HV_PROXIMITY_DOMAIN_ID, *PHV_PROXIMITY_DOMAIN_ID;
#if !defined(XBOX_SYSTEMOS)

//
// Definition of the HvSetSystemProperty/HvGetSystemProperty structures.
// This call sets a generic system wide property.
//

typedef enum _HV_SYSTEM_PROPERTY
{
    HvPerfCounterProperty = 1,
    HvLegacyPowerPolicySettingProperty = 2,
    HvSleepStateProperty = 3,
    HvMachineCheckProperty = 4,
    HvIommuInitStatusProperty = 5,
    HvHpetConfigProperty = 6,
    HvHpetInterruptProperty = 7,
    HvHpetEnabledProperty = 8,
    HvHypervisorLaunchStatsProperty = 9,
    HvHypervisorDebugProperty = 10,
    HvRootSvmCapabilitiesProperty = 11,
    HvRootNumaCostPagesProperty = 12,
    HvHostPageTableRootProperty = 13,
    HvTscSyncStatusProperty = 14,
    HvSchedulerTypeProperty = 15,
    HvPlatformVirtualizationSupportProperty = 16,
    HvHostDebugTransferPages = 17,
    HvHostTimelineBiasProperty = 18,
    HvSmcDataPathProperty = 19,
    HvDmaInitializeBlockedProperty = 20,
    HvSpeculationControlConfigProperty = 21,
    HvRdtCapabilitiesProperty = 22,
    HvRdtCosBitmaskProperty = 23,
    HvRdtCmtProperty = 24

} HV_SYSTEM_PROPERTY, *PHV_SYSTEM_PROPERTY;

typedef struct _HV_SLEEP_STATE_INFO
{
    HV_SLEEP_STATE SleepState;
    UINT8 Pm1a_SLP_TYP;
    UINT8 Pm1b_SLP_TYP;
} HV_SLEEP_STATE_INFO, *PHV_SLEEP_STATE_INFO;

typedef enum _HV_MACHINE_CHECK_PROPERTY_TYPE
{
    MachineCheckHandlerState = 1,
    MachineCheckDeferredRecoveryState = 2,

} HV_MACHINE_CHECK_PROPERTY_TYPE, *PHV_MACHINE_CHECK_PROPERTY_TYPE;

typedef enum _HV_MACHINE_CHECK_RECOVERY_FLAG
{
    InvalidRecoveryFlag = 0,
    NewDeferredRecoveryQueued = 1,
    DeferredRecoveryCompleted = 2,

} HV_MACHINE_CHECK_RECOVERY_FLAG, *PHV_MACHINE_CHECK_RECOVERY_FLAG;

typedef struct _HV_MACHINE_CHECK_PROPERTY_INFO
{
    HV_MACHINE_CHECK_PROPERTY_TYPE PropertyType;

    union
    {
        //
        // Applicable for PropertyType = MachineCheckHandlerState
        //
        BOOLEAN MachineCheckHandlerReady;

        //
        // Applicable for PropertyType = MachineCheckDeferredRecoveryState
        //
        HV_MACHINE_CHECK_RECOVERY_FLAG MachineCheckRecoveryFlag;
    };

} HV_MACHINE_CHECK_PROPERTY_INFO, *PHV_MACHINE_CHECK_PROPERTY_INFO;

typedef struct _HV_HPET_CONFIG_INFO
{
    HV_SPA BaseAddress;
    UINT32 TimerIndex;
    HV_DEVICE_ID DeviceId;
    UINT8 TimerInterruptPin;

} HV_HPET_CONFIG_INFO, *PHV_HPET_CONFIG_INFO;

typedef struct _HV_HPET_INTERRUPT_INFO
{
    HV_INTERRUPT_ENTRY InterruptEntry;

} HV_HPET_INTERRUPT_INFO, *PHV_HPET_INTERRUPT_INFO;

typedef struct _HV_HPET_ENABLED_INFO
{
    UINT8 HpetEnabled : 1;
    UINT8 Reserved : 7;

} HV_HPET_ENABLED_INFO, *PHV_HPET_ENABLED_INFO;

typedef struct _HV_HYPERVISOR_LAUNCH_STATS
{
    UINT64 RootBspTscAdjustment;

} HV_HYPERVISOR_LAUNCH_STATS, *PHV_HYPERVISOR_LAUNCH_STATS;

typedef struct _HV_HYPERVISOR_DEBUG_PROPERTY
{
    BOOLEAN PowerStateEnabled;

} HV_HYPERVISOR_DEBUG_PROPERTY,
  *PHV_HYPERVISOR_DEBUG_PROPERTY;

typedef struct _HV_ROOT_SVM_CAPABILITIES_PROPERTY
{
    UINT32 IommuCount;
    UINT32 MinIommuPasidCount;

} HV_ROOT_SVM_CAPABILITIES_PROPERTY, *PHV_ROOT_SVM_CAPABILITIES_PROPERTY;

//
// This structure passes information about NUMA node reserved pages from the
// hypervisor to loader and the otherway around.
//
typedef struct _HV_NUMA_NODE_RESERVED_PAGE_INFO
{
    HV_SPA PhysicalAddress;
    HV_PROXIMITY_DOMAIN_ID ProximityDomainId;

} HV_NUMA_NODE_RESERVED_PAGE_INFO, *PHV_NUMA_NODE_RESERVED_PAGE_INFO;

typedef struct _HV_ROOT_NUMA_COST_PAGES_PROPERTY
{
    //
    // Number of NUMA nodes known to the current hypervisor.
    //
    UINT32 NumaNodeCount;

    //
    // Reserved value to convey versioning requirement in the future.
    //
    UINT32 ReservedP;

    //
    // Array of PFNs, indexed by node number, for every node known to the
    // current hypervisor.
    //
    HV_NUMA_NODE_RESERVED_PAGE_INFO NumaNodePfn[HV_MAXIMUM_NODES];

} HV_ROOT_NUMA_COST_PAGES_PROPERTY , *PHV_ROOT_NUMA_COST_PAGES_PROPERTY;

typedef enum _HV_SCHEDULER_TYPE
{
    HvSchedulerLp = 1,
    HvSchedulerLpSmt = 2,
    HvSchedulerCoreSmt = 3,
    HvSchedulerRoot = 4,

} HV_SCHEDULER_TYPE, *PHV_SCHEDULER_TYPE;

//
// This structure returns information about the virtualization features
// supported by the platform.
//
typedef struct _HV_PLATFORM_VIRTUALIZATION_SUPPORT_INFO
{
    BOOLEAN NestedVirtualization;

} HV_PLATFORM_VIRTUALIZATION_SUPPORT_INFO,
  *PHV_PLATFORM_VIRTUALIZATION_SUPPORT_INFO;

//
// This structure supplies information about the host timeline.
//
typedef struct _HV_HOST_TIMELINE_INFO
{
    UINT64 TimelineBias;

} HV_HOST_TIMELINE_INFO, *PHV_HOST_TIMELINE_INFO;

//
// Lock state for the SMC data path, applies to SMCs that
// are subject to SocData serialized handling.
//
typedef enum _HV_SMC_DATA_PATH_LOCK_STATE
{
    //
    // The path is unlocked. Only the entity that last locked the
    // path can unlock it.
    //
    HvSmcDataPathUnlocked = 0,
    //
    // The path is locked by the Hypervisor. This value can only be
    // set by the Hypervisor itself.
    //
    HvSmcDataPathLockedByHvx = 1,
    //
    // The path is locked by VTL1.
    //
    HvSmcDataPathLockedByVtl1 = 2,
} HV_SMC_DATA_PATH_LOCK_STATE, *PHV_SMC_DATA_PATH_LOCK_STATE;

//
// This structure supplies information about the SMC data path
// property. Update of the property is synchronized across all
// processors.
//
typedef struct _HV_SMC_DATA_PATH_PROPERTY
{
    //
    // The lock state for the data path.
    //
    UINT16 LockState;
    //
    // This optionally enables the test capability that simulates
    // when TZ interrupts an SMC. When set, all SMCs are interrupted.
    // In order to enable testing of the full breadth of interrupted
    // cases, the last value set is saved regardless of whether the lock
    // state was succesfully updated.
    //
    UINT16 InterruptAllSmcs:1;
    UINT16 Reserved:15;
} HV_SMC_DATA_PATH_PROPERTY, *PHV_SMC_DATA_PATH_PROPERTY;

#if defined(_AMD64_)

//
// BTB MSR controls.
//
typedef enum _HV_X64_IBPB_FLUSH_GUEST_TYPE
{
    HvX64IbpbFlushGuestOff = 0,
    HvX64IbpbFlushGuestAll = 1,
    HvX64IbpbFlushGuestOptIn = 2,

    HvX64IbpbFlushGuestTypeCount = 3

} HV_X64_IBPB_FLUSH_GUEST_TYPE, *PHV_X64_IBPB_FLUSH_GUEST_TYPE;

//
// This structure returns information about the speculation control
// configuration currently used by the hypervisor.
//
typedef struct _HV_SPECULATION_CONTROL_CONFIG
{
    UINT64 IbrsValue;
    HV_X64_IBPB_FLUSH_GUEST_TYPE IbpbFlushGuest;
    BOOLEAN IbpbFlushRoot;

} HV_SPECULATION_CONTROL_CONFIG, *PHV_SPECULATION_CONTROL_CONFIG;

#endif

//
// This structure supplies information about a single COS
// bitmask present on the system.
//
typedef struct _HV_RDT_COS_BITMASK_PROPERTY
{
    UINT32 CosIndex;
    UINT32 CosBitmask;
} HV_RDT_COS_BITMASK_PROPERTY, *PHV_RDT_COS_BITMASK_PROPERTY;

//
// This structure supplies information about all COS
// bitmasks present on the system.
//
typedef struct _HV_RDT_COS_BITMASKS_PROPERTY
{
    //
    // The number of bitmask slots returned.
    //
    UINT32 NumL3CosSlots;
    UINT32 CosSlots[HV_MAXIMUM_COS_SLOTS];
} HV_RDT_COS_BITMASKS_PROPERTY, *PHV_RDT_COS_BITMASKS_PROPERTY;

//
// This structure supplies information about Intel RDT support.
//
typedef struct _HV_RDT_CAPABILITIES_PROPERTY
{

    UINT32 L3CatSupported:1;
    UINT32 L3CmtSupported:1;
    UINT32 L3OccupancyMonitoringSupported:1;
    UINT32 L3TotalBwMonitoringSupported:1;
    UINT32 L3LocalBwMonitoringSupported:1;
    UINT32 Reserved:27;

    UINT32 L3CosBitmaskWidth;
    UINT32 L3CosBitmaskHwReserved;
    UINT32 NumL3CosSlots;
    UINT32 MaxL3Rmid;
} HV_RDT_CAPABILITIES_PROPERTY, *PHV_RDT_CAPABILITIES_PROPERTY;

//
// Structure definition for Intel CMT event data
//
typedef struct _HV_RDT_CMT_EVENT_DATA
{
    UINT64 L3OccupancyBytes;
    UINT64 L3TotalBwBytes;
    UINT64 L3LocalBwBytes;
} HV_RDT_CMT_EVENT_DATA, *PHV_RDT_CMT_EVENT_DATA;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SET_SYSTEM_PROPERTY
{
    UINT32 PropertyId;

    union
    {
        struct
        {
            UINT16 EventType;
            UINT16 ReservedZ1;
            UINT32 ReservedZ2;
            UINT64 Period;
        } PerfCounter;

        HV_PPM_POWER_POLICY_SETTING PowerPolicySetting;

        HV_SLEEP_STATE_INFO SetSleepStateInformation;

        HV_MACHINE_CHECK_PROPERTY_INFO MachineCheckStateInformation;

        HV_HPET_CONFIG_INFO HpetConfigInfo;
        HV_HPET_ENABLED_INFO HpetEnabledInfo;
        HV_HYPERVISOR_DEBUG_PROPERTY Debug;
        HV_HOST_TIMELINE_INFO HostTimeline;
        HV_SMC_DATA_PATH_PROPERTY SmcDataPath;
        HV_RDT_COS_BITMASK_PROPERTY CosBitmask;
    } Property;

} HV_INPUT_SET_SYSTEM_PROPERTY, *PHV_INPUT_SET_SYSTEM_PROPERTY;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_SYSTEM_PROPERTY
{

    UINT32 PropertyId;

    union
    {
        HV_PPM_POWER_POLICY_SETTING_ID SettingId;
        UINT32 ResourceMonitoringId;
    } Property;

} HV_INPUT_GET_SYSTEM_PROPERTY, *PHV_INPUT_GET_SYSTEM_PROPERTY;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_SYSTEM_PROPERTY
{
    union
    {
        struct
        {
            UINT16 EventType;
            UINT16 ReservedZ1;
            UINT32 ReservedZ2;
            UINT64 Period;
        } PerfCounter;

        HV_PPM_POWER_POLICY_SETTING PowerPolicySetting;

        HV_IOMMU_INIT_STATUS IommuInitStatus;

        HV_TSC_SYNC_STATUS TscSyncStatus;

        HV_HPET_CONFIG_INFO HpetConfigInfo;
        HV_HPET_INTERRUPT_INFO HpetInterruptInfo;
        HV_HPET_ENABLED_INFO HpetEnabledInfo;

        HV_HYPERVISOR_LAUNCH_STATS HvLaunchStats;

        HV_ROOT_SVM_CAPABILITIES_PROPERTY RootSvmCapabilities;

        HV_ROOT_NUMA_COST_PAGES_PROPERTY RootNumaCostPages;

        HV_SPA HvHostPageTableRoot;

        HV_SCHEDULER_TYPE SchedulerType;

        HV_PLATFORM_VIRTUALIZATION_SUPPORT_INFO VirtualizationInfo;

        HV_SPA_PAGE_RANGE HvHostDbgTransferPagesRange;

        HV_SMC_DATA_PATH_PROPERTY SmcDataPath;

        BOOLEAN DmaInitializeBlocked;

#if defined(_AMD64_)

        HV_SPECULATION_CONTROL_CONFIG SpeculationControlConfig;

#endif

        HV_RDT_CAPABILITIES_PROPERTY HvRdtCapabilities;

        HV_RDT_COS_BITMASKS_PROPERTY HvRdtCosBitmasks;

        HV_RDT_CMT_EVENT_DATA CmtEventData;
    } Property;

} HV_OUTPUT_GET_SYSTEM_PROPERTY, *PHV_OUTPUT_GET_SYSTEM_PROPERTY;

#endif

//
// Definition of the HvMapStatsPage hypercall input structure.  This
// call allows a partition to map the page with statistics into
// the caller's GPA space.for child partition or for itself.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_MAP_STATS_PAGE
{
    HV_STATS_OBJECT_TYPE       StatsType;
    HV_STATS_OBJECT_IDENTITY   ObjectIdentity;
} HV_INPUT_MAP_STATS_PAGE, *PHV_INPUT_MAP_STATS_PAGE;

//
// Definition of the HvMapStatsPage hypercall output structure.  This
// call allows a partition to map the page with statistics into
// the caller's GPA space.for child partition or for itself.
//

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_MAP_STATS_PAGE
{
    HV_GPA_PAGE_NUMBER MapLocation;
} HV_OUTPUT_MAP_STATS_PAGE, *PHV_OUTPUT_MAP_STATS_PAGE;

//
// Definition of the HvUnmapStatsPage hypercall input structure.  This
// call allows a partition to unmap the page with statistics from
// the caller's GPA space.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_UNMAP_STATS_PAGE
{
    HV_STATS_OBJECT_TYPE       StatsType;
    HV_STATS_OBJECT_IDENTITY   ObjectIdentity;
} HV_INPUT_UNMAP_STATS_PAGE, *PHV_INPUT_UNMAP_STATS_PAGE;

//
// Define the proximity domain information flags.
//

typedef struct _HV_PROXIMITY_DOMAIN_FLAGS
{
    //
    // This flag specifies whether the proximity information is preferred. If
    // so, then the memory allocations are done preferentially from the
    // specified proximity domain. In case there is insufficient memory in the
    // specified domain, other domains are tried. If this flag is false, then
    // all memory allocation must come from the specified domain.
    //
    UINT32 ProximityPreferred:1;

    UINT32 Reserved:30;

    //
    // This flag specifies that the specified proximity domain is valid. If
    // this flag is false then the memory allocation can come from any
    // proximity domain.
    //
    UINT32 ProximityInfoValid:1;

} HV_PROXIMITY_DOMAIN_FLAGS, *PHV_PROXIMITY_DOMAIN_FLAGS;

//
// Define the proximiy domain information structure.
//

typedef struct _HV_PROXIMITY_DOMAIN_INFO
{
    HV_PROXIMITY_DOMAIN_ID Id;
    HV_PROXIMITY_DOMAIN_FLAGS Flags;

} HV_PROXIMITY_DOMAIN_INFO, *PHV_PROXIMITY_DOMAIN_INFO;

//
// Definition of the HvCallDepositMemory hypercall input structure.
// This call deposits memory into a child partition's memory pool.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_DEPOSIT_MEMORY
{
    //
    // Supplies the partition ID of the child partition to deposit the
    // memory into.
    //
    HV_PARTITION_ID PartitionId;

    //
    // Supplies the GPAs of the pages to be deposited.
    //
    HV_CALL_ATTRIBUTES HV_GPA_PAGE_NUMBER GpaPageList[];

} HV_INPUT_DEPOSIT_MEMORY, *PHV_INPUT_DEPOSIT_MEMORY;

//
// Definition of the HvCallGetMemoryBalance hypercall input and output
// structures.  This call determines the hypervisor memory resource
// usage of a child partition's memory pool.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_MEMORY_BALANCE
{
    //
    // Supplies the partition ID of the child partition whose memory
    // pool should be queried.
    //
    HV_PARTITION_ID PartitionId;

    //
    // Supplies the proximity domain to query.
    //
    HV_PROXIMITY_DOMAIN_INFO ProximityDomainInfo;

} HV_INPUT_GET_MEMORY_BALANCE, *PHV_INPUT_GET_MEMORY_BALANCE;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_MEMORY_BALANCE
{
    //
    // Returns the number of pages available.
    //
    UINT64 PagesAvailable;

    //
    // Returns the number of pages actively being used for hypercall
    // datastructures.
    //
    UINT64 PagesInUse;

} HV_OUTPUT_GET_MEMORY_BALANCE, *PHV_OUTPUT_GET_MEMORY_BALANCE;

//
// Definition of the add logical processor hypercall input structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_ADD_LOGICAL_PROCESSOR
{
    HV_LOGICAL_PROCESSOR_INDEX LpIndex;
    HV_PROCESSOR_HW_ID ProcHwId;
    HV_PROXIMITY_DOMAIN_INFO ProximityDomainInfo;
    UINT64 Flags;
} HV_INPUT_ADD_LOGICAL_PROCESSOR, *PHV_INPUT_ADD_LOGICAL_PROCESSOR;

//
// Definition of the add logical processor hypercall output structure.
//

typedef struct _HV_LP_STARTUP_STATUS
{
    HV_STATUS Status;
    UINT64 SubStatus1;
    UINT64 SubStatus2;
    UINT64 SubStatus3;
    UINT64 SubStatus4;
    UINT64 SubStatus5;
    UINT64 SubStatus6;
} HV_LP_STARTUP_STATUS, *PHV_LP_STARTUP_STATUS;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_ADD_LOGICAL_PROCESSOR
{
    HV_LP_STARTUP_STATUS StartupStatus;
} HV_OUTPUT_ADD_LOGICAL_PROCESSOR, *PHV_OUTPUT_ADD_LOGICAL_PROCESSOR;

//
// Definition of the remove logical processor hypercall input structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_REMOVE_LOGICAL_PROCESSOR
{
    HV_LOGICAL_PROCESSOR_INDEX LpIndex;
    UINT64 Flags;
} HV_INPUT_REMOVE_LOGICAL_PROCESSOR, *PHV_INPUT_REMOVE_LOGICAL_PROCESSOR;

//
// Definition of the query NUMA topology hypercall input and output structures.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_QUERY_NUMA_DISTANCE
{
    HV_PROXIMITY_DOMAIN_ID CpuProximityId;
    HV_PROXIMITY_DOMAIN_ID MemoryProximityId;
} HV_INPUT_QUERY_NUMA_DISTANCE, *PHV_INPUT_QUERY_NUMA_DISTANCE;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_QUERY_NUMA_TOPOLOGY
{
    UINT64 Distance;
} HV_OUTPUT_QUERY_NUMA_DISTANCE, *PHV_OUTPUT_QUERY_NUMA_DISTANCE;

//
// Definition of the HvCallCreateVp hypercall input structure.
// This call creates a virtual processor.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_CREATE_VP
{
    HV_PARTITION_ID          PartitionId;
    HV_VP_INDEX              VpIndex;
    UINT32                   Padding;
    HV_PROXIMITY_DOMAIN_INFO ProximityDomainInfo;
    UINT64                   Flags;
} HV_INPUT_CREATE_VP, *PHV_INPUT_CREATE_VP;

#define HV_DEBUG_INVOKE_REASON_CLOCK_WATCHDOG   0x01
#define HV_DEBUG_INVOKE_REASON_DPC_WATCHDOG     0x02
#define HV_DEBUG_INVOKE_REASON_DPC_TIMEOUT      0x03

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_INVOKE_HYPERVISOR_DEBUGGER
{
    UINT64 Reason;
    UINT64 SupplementalCode;
} HV_INPUT_INVOKE_HYPERVISOR_DEBUGGER, *PHV_INPUT_INVOKE_HYPERVISOR_DEBUGGER;

//
// Define Interrupt Trigger modes.
//
typedef enum _HV_INTERRUPT_TRIGGER_MODE
{
    HvInterruptTriggerModeEdge  = 0x0000,
    HvInterruptTriggerModeLevel = 0x0001

} HV_INTERRUPT_TRIGGER_MODE, *PHV_INTERRUPT_TRIGGER_MODE;

//
// Define Interrupt Descriptor
//

typedef struct _HV_DEVICE_INTERRUPT_DESCRIPTOR
{
    HV_INTERRUPT_TYPE           InterruptType;
    HV_INTERRUPT_TRIGGER_MODE   TriggerMode;
    UINT32                      VectorCount;
    UINT32                      Reserved;
    HV_DEVICE_INTERRUPT_TARGET  Target;

} HV_DEVICE_INTERRUPT_DESCRIPTOR, *PHV_DEVICE_INTERRUPT_DESCRIPTOR;

//
// Definitions needed for IOMMU support.
//

#define HV_INTERRUPT_REMAPPING_MASK_FAILURE     1
#define HV_INTERRUPT_REMAPPING_NO_MEMORY        2
#define HV_INTERRUPT_REMAPPING_LOGICAL_ENTRY    4

#define HV_INTERRUPT_REMAPPING_VALID_FLAGS \
    (HV_INTERRUPT_REMAPPING_MASK_FAILURE | \
     HV_INTERRUPT_REMAPPING_NO_MEMORY | \
     HV_INTERRUPT_REMAPPING_LOGICAL_ENTRY)

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_MAP_DEVICE_INTERRUPT
{
    HV_PARTITION_ID                 PartitionId;
    UINT64                          DeviceId;
    UINT64                          Flags;
    HV_INTERRUPT_ENTRY              LogicalInterruptEntry;
    HV_DEVICE_INTERRUPT_DESCRIPTOR  InterruptDescriptor;
} HV_INPUT_MAP_DEVICE_INTERRUPT, *PHV_INPUT_MAP_DEVICE_INTERRUPT;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_MAP_DEVICE_INTERRUPT
{
    HV_INTERRUPT_ENTRY          InterruptEntry;
    HV_IOMMU_EXTENDED_STATUS    ExtendedStatus;
} HV_OUTPUT_MAP_DEVICE_INTERRUPT, *PHV_OUTPUT_MAP_DEVICE_INTERRUPT;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_UNMAP_DEVICE_INTERRUPT
{
    HV_PARTITION_ID     PartitionId;
    UINT64              DeviceId;
    HV_INTERRUPT_ENTRY  InterruptEntry;
} HV_INPUT_UNMAP_DEVICE_INTERRUPT, *PHV_INPUT_UNMAP_DEVICE_INTERRUPT;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_ENABLE_DEVICE_INTERRUPT
{
    HV_PARTITION_ID PartitionId;
    UINT32          InterruptNumber;
    UINT32          Priority;
    BOOLEAN         Enable;
} HV_INPUT_ENABLE_DEVICE_INTERRUPT, *PHV_INPUT_ENABLE_DEVICE_INTERRUPT;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_RETARGET_ROOT_DEVICE_INTERRUPT
{
    HV_INTERRUPT_ENTRY InterruptEntry;

} HV_OUTPUT_RETARGET_ROOT_DEVICE_INTERRUPT, *PHV_OUTPUT_RETARGET_ROOT_DEVICE_INTERRUPT;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_ASSERT_DEVICE_INTERRUPT
{
    HV_PARTITION_ID PartitionId;
    UINT64 DeviceId;
    HV_INTERRUPT_ENTRY InterruptEntry;

} HV_INPUT_ASSERT_DEVICE_INTERRUPT, *PHV_INPUT_ASSERT_DEVICE_INTERRUPT;

//
// Common header used by both extended list and space flush routines.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE_HEADER_EX
{
    HV_ADDRESS_SPACE_ID AddressSpace;
    HV_FLUSH_FLAGS      Flags;
    HV_VP_SET    ProcessorSet;
} HV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE_HEADER_EX,
  *PHV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE_HEADER_EX;

//
// Definition of the HvCallFlushVirtualAddressSpaceEx hypercall input
// structure.  This call flushes the virtual TLB entries which belong
// to the indicated address space, on one or more processors.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE_EX
{
    HV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE_HEADER_EX Header;
} HV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE_EX,
  *PHV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE_EX;

//
// Definition of the HvCallFlushVirtualAddressList hypercall input
// structure.  This call invalidates portions of the virtual TLB which
// belong to the indicates address space, on one more more processors.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_FLUSH_VIRTUAL_ADDRESS_LIST_EX
{
    HV_INPUT_FLUSH_VIRTUAL_ADDRESS_SPACE_HEADER_EX Header;
    // HV_CALL_ATTRIBUTES HV_GVA GvaList;
} HV_INPUT_FLUSH_VIRTUAL_ADDRESS_LIST_EX,
  *PHV_INPUT_FLUSH_VIRTUAL_ADDRESS_LIST_EX;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SEND_SYNTHETIC_CLUSTER_IPI_EX
{
    UINT32 Vector;
    HV_INPUT_VTL TargetVtl;
    UINT8 RsvdZ0;
    UINT16 RsvdZ1;
    HV_VP_SET ProcessorSet;
} HV_INPUT_SEND_SYNTHETIC_CLUSTER_IPI_EX,
  *PHV_INPUT_SEND_SYNTHETIC_CLUSTER_IPI_EX;

typedef union _HV_INPUT_REQUEST_PROCESSOR_HALT_FLAGS
{
    UINT32 AsUINT32;
    struct
    {
        UINT32 UseDefaultSuspend : 1;
        UINT32 RsvdZ : 31;
    };
} HV_INPUT_REQUEST_PROCESSOR_HALT_FLAGS, *PHV_INPUT_REQUEST_PROCESSOR_HALT_FLAGS;

C_ASSERT(sizeof(HV_INPUT_REQUEST_PROCESSOR_HALT_FLAGS) == sizeof(UINT32));

typedef union HV_CALL_ATTRIBUTES _HV_INPUT_REQUEST_PROCESSOR_HALT
{
    struct
    {
        UINT32 PowerState;
        HV_INPUT_REQUEST_PROCESSOR_HALT_FLAGS Flags;
    };
    UINT64 AsUINT64;
} HV_INPUT_REQUEST_PROCESSOR_HALT, *PHV_INPUT_REQUEST_PROCESSOR_HALT;

C_ASSERT(sizeof(HV_INPUT_REQUEST_PROCESSOR_HALT) == sizeof(UINT64));

typedef enum _HV_BOOT_DEBUG_PORT_TYPE
{
    HvBootDbgPortNone,
    HvBootDbgPortCom,
    HvBootDbgPortFirewire,
    HvBootDbgPortNet,

} HV_BOOT_DEBUG_PORT_TYPE, *PHV_BOOT_DEBUG_PORT_TYPE;

typedef enum _HV_BOOT_DEBUG_COM_PORT_TYPE
{
    HvBootDbgPortComIoPort,
    HvBootDbgPortComMemoryMapped
} HV_BOOT_DEBUG_COM_PORT_TYPE, *PHV_BOOT_DEBUG_COM_PORT_TYPE;

//
// Also update if needed the default & min/max baudrate below if changing the
// baudrates we support
//

#define HV_BOOT_COM_PORT_BAUD19200      (19200)
#define HV_BOOT_COM_PORT_BAUD38400      (38400)
#define HV_BOOT_COM_PORT_BAUD57600      (57600)
#define HV_BOOT_COM_PORT_BAUD115200     (115200)

#define HV_BOOT_FW_FLAG_ROOT_CHANNEL    1

#define HV_BOOT_NET_FLAG_ROOT_CHANNEL   0x1
#define HV_BOOT_NET_FLAG_USE_ENCRYPTION 0x2
#define HV_BOOT_NET_FLAG_USE_DHCP       0x4

#define HV_BOOT_NET_BAR_COUNT           6

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

typedef struct _HV_BOOT_DEBUG_PARAMETERS
{
    HV_BOOT_DEBUG_PORT_TYPE PortType;

    //
    //  must be under 4gb for 1394 debugging. These are expected to
    //  to be physically continguous
    //

    HV_SPA TransferPagesSpa;
    UINT32 TransferPagesCount;

    union
    {
        struct
        {
            HV_BOOT_DEBUG_COM_PORT_TYPE ComPortType;
            ULONG InterfaceType;
            union
            {
                UINT16 IoPort;
                HV_SPA RegistersSpa;
            };
            UINT32 BaudRate;
            BOOLEAN ExclusiveMode;
        } Com;

        struct
        {
            UINT32 Bus;
            UINT32 Slot;
            UINT16 Flags;
            UINT8 HvChannelId;
            UINT8 RootChannelId;
            HV_SPA OhciRegistersSpa;
        } Firewire;

        struct
        {
            ULONG  NameSpace;
            USHORT PortSubtype;
            UINT32 Bus;
            UINT32 Slot;
            IPV6_ADDRESS HostIP;
            UINT32 Flags;
            UINT16 VendorID;
            UINT16 DeviceID;
            UINT16 HostHvPort;
            UINT16 HostRootPort;
            UINT64 Key[4];
            HV_SPA BaseAddressRegisterSPA[HV_BOOT_NET_BAR_COUNT];
            UINT32 BaseAddressRegisterByteCount[HV_BOOT_NET_BAR_COUNT];
            UINT16 BaseAddressRegisterType[HV_BOOT_NET_BAR_COUNT];
            UINT32 KdNetDataSize;
#if defined (_ARM64_)
            HV_SPA KdNetPciMmCfgAddr;
#endif
            UINT8  BaseClass;
            UINT8  SubClass;
            UINT8  ProgIf;
            UINT16 PortType;
            UINT16 Segment;
            UCHAR  DefaultTargetMacAddress[6];
        } Net;
    };

} HV_BOOT_DEBUG_PARAMETERS, *PHV_BOOT_DEBUG_PARAMETERS;

//
// Define boot flags.
//

#define HV_BOOT_INITIAL_BREAKPOINT              (0x00000001)
#define HV_BOOT_DISABLE_INVARIANT_TSC           (0x00000002)
#define HV_BOOT_FORCE_APIC_PHYSICAL_MODE        (0x00000004)
#define HV_BOOT_FORCE_APIC_CLUSTER_MODE         (0x00000008)
#define HV_BOOT_DISABLE_X2APIC                  (0x00000010)
#define HV_BOOT_ENABLE_X2APIC                   (0x00000020)
#define HV_BOOT_DISABLE_XSAVE                   (0x00000040)
#define HV_BOOT_USE_LEGACY_TSC_SYNC             (0x00000080)
#define HV_BOOT_USE_ENHANCED_TSC_SYNC           (0x00000100)
#define HV_BOOT_ENABLE_APIC_EMULATION           (0x00000200)
#define HV_BOOT_IOMMU_X2APIC_ONLY               (0x00000400)
#define HV_BOOT_ENABLE_MSR_FILTERING            (0x00000800)
#define HV_BOOT_DISABLE_MMIO_NX                 (0x00001000)
#define HV_BOOT_OFFER_ROOT_VSM                  (0x00002000)
#define HV_BOOT_ERRATA_IOMMU_X58                (0x00004000)
#define HV_BOOT_SOFT_REBOOT                     (0x00008000)
#define HV_BOOT_ROOT_SCHEDULER                  (0x00010000)
#define HV_BOOT_IS_CLIENT_SKU                   (0x00020000)
#define HV_BOOT_SECURE_LAUNCH                   (0x00040000)
#define HV_BOOT_FORCE_DEFAULT_BLOCKED_DMA       (0x00080000)
#define HV_BOOT_DISABLE_APIC_EMULATION          (0x00100000)
#define HV_BOOT_CLASSIC_SCHEDULER               (0x00200000)
#define HV_BOOT_CORE_SCHEDULER                  (0x00400000)

#define HV_BOOT_ALL_SCHEDULER_TYPES (HV_BOOT_CLASSIC_SCHEDULER | \
                                     HV_BOOT_CORE_SCHEDULER | \
                                     HV_BOOT_ROOT_SCHEDULER)

//
// Define the configurable Hypervisor Boot Parameters.
//

typedef struct _HV_BOOT_PARAMETERS
{
    HV_BOOT_DEBUG_PARAMETERS DebugParams;
    UINT32 Flags;

} HV_BOOT_PARAMETERS, *PHV_BOOT_PARAMETERS;

#if defined (_ARM64_)

//
// No known errata for the current platform.
//
#define HV_HARDWARE_ERRATA_TYPE_NONE            0x0

//
// Platform is subject to Qualcomm 8996 errata.
//
#define HV_HARDWARE_ERRATA_TYPE_8996            0x1

//
// Platform is subject to Qualcomm 8998 errata.
//
#define HV_HARDWARE_ERRATA_TYPE_8998            0x2

//
// Platform is subject to Qualcomm SDM845 errata.
//
#define HV_HARDWARE_ERRATA_TYPE_SDM845            0x3

typedef struct _HV_HARDWARE_DETAILS
{
    UINT32 ErrataType;
    UINT32 SupportPsci:1;
    UINT32 SupportBranchToHypervisor:1;
    UINT32 SupportSecureLaunch:1;
    UINT32 Reserved:29;
} HV_HARDWARE_DETAILS, *PHV_HARDWARE_DETAILS;

#define HV_MAX_SECTION_COUNT 0xF

typedef struct _HV_IMAGE_SECTION_DETAIL
{
    UINT64  SectionPhysicalAddress;
    UINT32  SectionSize;
    UINT64  Writeable:1;
    UINT64  Executable:1;
} HV_IMAGE_SECTION_DETAIL, *PHV_IMAGE_SECTION_DETAIL;

typedef struct _HV_IMAGE_DETAIL
{
    UINT32  SectionCount;
    HV_IMAGE_SECTION_DETAIL Sections[HV_MAX_SECTION_COUNT];
} HV_IMAGE_DETAIL, *PHV_IMAGE_DETAIL;

#endif

//
// Maximum number of runtime services memory ranges passed to the hypervisor in
// the loader block.
//
#define HV_BOOT_MAX_RUNTIME_SERVICES_RANGES 512

//
// The loader block contains all the state required by the hypervisor to
// initialize and launch the root partition.
//
// It *must* be allocated from physically contiguous memory.
//
#define HV_LOADER_BLOCK_VERSION     10

typedef struct _HV_MINI_LOADER_BLOCK
{
    //
    // The size of the loader block in bytes.
    //

    UINT32 LoaderBlockSize;

    //
    // The version this loader block conforms to.
    //

    UINT32 LoaderBlockVersion;

    //
    // The base physical address of the loader block so it can be easily
    // accessed from multiple address spaces and with paging off.
    //

    HV_SPA LoaderBlockPhysicalAddress;

    //
    // The virtual address (in hypervisor VA space) of the hypervisor entry
    // pointer root page.
    //

    UINT64 HypervisorEntryPoint;

    //
    // Hypervisor CR3 or TTBR, i.e. the physical address of the hypervisor page table / translation table
    // root page.
    //

    UINT64 HypervisorTransTabBasePhys;

    //
    // Configurable Boot Parameters.
    //

    HV_BOOT_PARAMETERS BootParameters;

    //
    // The number of processors in the root partition which may be less than
    // or equal to the PhysicalProcessorCount as some processors may not have
    // been started for licensing reasons.  An array containing the state to
    // restore can be found at RootPartitionProcessorsRegistersOffset from the
    // base of the loader block with the following format:
    //
    //  HV_VP_CONTEXT
    //      RootPartitionProcessorsContexts[RootPartitionProcessorCount];
    //

    UINT32 RootPartitionProcessorCount;
    UINT32 RootPartitionProcessorsContextsOffset;
    UINT32 VpContextSize;
    UINT32 MiniVpContextSize;

    //
    // Physical address of Hypervisor Crashdump Header pages, and size.  This
    // region must consist of SPA-contiguous pages, to ease static extraction
    // from Windows crashdump by post-processing tools.
    //

    UINT32 HypervisorCrashdumpAreaPageCount;
    UINT64 HypervisorCrashdumpAreaSpa;

    //
    // The below 1Mb page that is used for executing real mode code during
    // AP startup.
    //

    UINT64 Below1MbPage;

    //
    // Number of memory ranges that contain either EFI runtime services code or
    // data.
    //
    UINT32 RuntimeServicesMemoryRangesCount;

    //
    // Memory ranges that contain either EFI runtime services code or data.
    //
    struct
    {
        //
        // The PFN where this range begins.
        //
        HV_SPA_PAGE_NUMBER BasePage;

        //
        // The number of pages in this range.
        //
        UINT64 PageCount;

    } RuntimeServicesMemoryRanges[HV_BOOT_MAX_RUNTIME_SERVICES_RANGES];

#if defined (_ARM64_)

    //
    // Details about the specific platform used for boot and errata.
    //
    HV_HARDWARE_DETAILS HardwareDetails;

    //
    // Used for launching Hypervisor on Qualcomm platforms.
    //
    HV_IMAGE_DETAIL HypervisorImageDetail;

#endif

} HV_MINI_LOADER_BLOCK, *PHV_MINI_LOADER_BLOCK;

//
// This structure records the hypervisor launch status to be passed back
// to OsLoader.
//

typedef struct _HVL_STATUS
{
    BAL_STATUS Status;
    UINT64 Argument1;
    UINT64 Argument2;
    UINT64 Argument3;
    UINT64 Argument4;
} HVL_STATUS, *PHVL_STATUS;

//
// Defines the structure that will be passed back to OsLoader using BL
// persistent data API. This structure records the hypervisor loader block
// metadata.
//

typedef struct _HVL_PERSISTENT_DATA
{

    //
    // The physical address of the loader block.
    //

    UINT64 LoaderBlockPhysicalAddress;

    //
    // The size of loader block in bytes.
    //

    UINT32 LoaderBlockSize;

    //
    // If true the below 1MB page has been allocated, else it needs to be
    // reserved during the kernel memory map construction.
    //

    BOOLEAN Below1MbPageAllocated;

    //
    // The address of the below 1MB page.
    //

    UINT64 Below1MbPage;

} HVL_PERSISTENT_DATA, * PHVL_PERSISTENT_DATA;

//
// This structure is used to transfer crashdump information between the
// Hypervisor and the HvBoot.sys driver in the root Windows instance at the
// time of a Hypervisor BugCheck.  It is allocated by HvBoot.sys during the
// Hypervisor launch process, and its SPA is handed in to the Hypervisor via
// the loader block.
//

//
// 2014-10-02 - adityabh: The following fields of the crashdump area are no
// longer needed and should be removed when the version is revised to v5.
//
// MaxPfn
// PfnEntrySize
// AllocatedPfnEntryMask
//
// Remove these fields and this comment whenever the next breaking change is
// made (i.e. one in which some new fields are added and which would thus
// necessitate a new version).
//

#define HV_CRASHDUMP_AREA_VERSION   4
#define HV_IMAGE_NAME_MAX_LENGTH    32
#define HV_DEFAULT_CRASHDUMP_SPA_PAGES 5
#define CODE_CHUNK_SIZE     0x200

typedef struct _HV_CRASHDUMP_AREA
{
    //
    // Version of the Crashdump Area structure
    //

    UINT32 Version;

    //
    // Flags indicating content validity and other attributes of the
    // Crashdump Area
    //

    union
    {
        UINT32 FlagsAsUINT32;
        struct
        {
            //
            // Indicates the contents of the Crashdump Area are valid
            //

            UINT32  Valid:1;
            UINT32  Reserved:31;

        };

    };

    //
    // Loaded Module Information.
    //

    UINT64 HypervisorBase;
    UINT32 SizeOfImage;
    UINT32 CheckSum;

    //
    // Partition State.
    //

    UINT64 CurrentPartition;
    UINT64 PartitionsCreated;
    UINT32 PartitionsRunning;
    UINT64 CompartmentFreePfns;

    UINT16 ImageNameLength;
    WCHAR ImageName[HV_IMAGE_NAME_MAX_LENGTH];

    //
    // Bugcheck error code fields.
    //

    UINT64 BugCheckData[5];
    UINT64 BugCheckErrorReturnAddress;

    //
    // The root of the page table needed to lookup virtual addresses
    // and the debugger data block. The debugger data block contains
    // all the information necc. for the debugger to interpret the
    // dump file. Of particular interest within it is the prcb address
    // that contain the processor state.
    //

    UINT64 PageTableBase;
    UINT64 PfnDataBase;
    UINT64 MaxPfn;
    UINT64 DebuggerDataBlock;
    UINT32 NumberProcessors;
    UINT32 CurrentProcessor;

    //
    // Code page data. If we know the source of the fault this
    // will have the in memory copy of the code and its spa.
    //

    UINT64 CodeSpa;
    UINT8 CodeChunk[CODE_CHUNK_SIZE];

    //
    // Processor contexts. This is the offset to a set of
    // HV_CRASHDUMP_PROCESSOR_STATE structs. The crashing processor
    // should always be included.
    //

    UINT32 ContextCount;
    UINT32 ContextOffset;
    UINT32 ContextSize;

    UINT32 PfnEntrySize;
    UINT16 AllocatedPfnEntryMask;

    //
    // If set for a livedump, indicates a potentially incomplete dump
    //

    BOOLEAN Partial;

} HV_CRASHDUMP_AREA, *PHV_CRASHDUMP_AREA;

//
// Define the memory descriptor type used for hypervisor live dump.
//

typedef struct _HV_MEMORY_DESCRIPTOR
{
    //
    // The number of described memory pages.
    //

    UINT64 Pages;

    //
    // Number of levels in the page table.
    //

    UINT32 Levels;

    //
    // The GPA page number of the page table root.
    //

    HV_GPA_PAGE_NUMBER PageTableRoot;
} HV_MEMORY_DESCRIPTOR, *PHV_MEMORY_DESCRIPTOR;

typedef struct _HV_INPUT_COLLECT_LIVE_DUMP
{
    //
    // Flags that modify the live dump operation.
    //

    UINT64 Flags;

    //
    // Memory descriptor describing the destination of the mirror operation.
    //

    HV_MEMORY_DESCRIPTOR Descriptor;
} HV_INPUT_COLLECT_LIVE_DUMP, *PHV_INPUT_COLLECT_LIVE_DUMP;

typedef struct _HV_OUTPUT_COLLECT_LIVE_DUMP
{
    //
    // The number of pages that were actually copied out.
    //

    UINT64 PagesCopied;

    //
    // The total number of pages currently deposited in the hypervisor.
    //

    UINT64 DepositedPages;
} HV_OUTPUT_COLLECT_LIVE_DUMP, *PHV_OUTPUT_COLLECT_LIVE_DUMP;

//
// Definitions for fetching and resetting hypervisor code-coverage on
// coverage builds.
//

#define HV_MAX_COVERAGE_PAGES 128

typedef enum _HV_COVERAGE_OPERATION
{
    HvCoverageFetchInfo,
    HvCoverageFetchCovHeader,
    HvCoverageFetchCovSection,
    HvCoverageResetCovVector

} HV_COVERAGE_OPERATION;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_COVERAGE_VECTOR
{
    //
    // Type of operation to perform.
    //

    HV_COVERAGE_OPERATION   CoverageOp;

    //
    // Number of pages supplied in GpaPageNumbers
    //

    UINT32                  PageCount;

    //
    // Array of GPAs to receive coverage data.
    //

    HV_GPA_PAGE_NUMBER      GpaPageNumbers[HV_MAX_COVERAGE_PAGES];

} HV_INPUT_GET_COVERAGE_VECTOR, *PHV_INPUT_GET_COVERAGE_VECTOR;

typedef struct HV_CALL_ATTRIBUTES _HV_IMAGE_COVERAGE_INFO
{
        //
        // Size of coverage section for this image without coverage vector.
        //

        UINT32 HeaderSize;

        //
        // Size of coverage section for this image with coverage vector.
        //

        UINT32 Size;

        //
        // Image Name that is filled up on a CovInfo request.
        //

        WCHAR ImageName[HV_IMAGE_NAME_MAX_LENGTH];
} HV_IMAGE_COVERAGE_INFO, *PHV_IMAGE_COVERAGE_INFO;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_COVERAGE_VECTOR
{

    HV_IMAGE_COVERAGE_INFO CoverageInfo;

} HV_OUTPUT_GET_COVERAGE_VECTOR, *PHV_OUTPUT_GET_COVERAGE_VECTOR;

typedef union _HV_SPA_PAGE_LIST_FLAGS
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 Deposited   : 1;
        UINT64 RsvdZ       : 63;
    };

} HV_SPA_PAGE_LIST_FLAGS, *PHV_SPA_PAGE_LIST_FLAGS;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_SPA_PAGE_LIST
{
    HV_SPA_PAGE_NUMBER StartPage;
    HV_SPA_PAGE_NUMBER EndPage;
    HV_SPA_PAGE_LIST_FLAGS Flags;

} HV_INPUT_GET_SPA_PAGE_LIST, *PHV_INPUT_GET_SPA_PAGE_LIST;

#define HV_SPA_PAGE_RANGE_LIST_SIZE 510

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_SPA_PAGE_LIST
{
    HV_SPA_PAGE_NUMBER NextPageToScan;
    UINT16 RangeCount;
    UINT16 Rsvd[3];
    HV_SPA_PAGE_RANGE RangeList[HV_SPA_PAGE_RANGE_LIST_SIZE];

} HV_OUTPUT_GET_SPA_PAGE_LIST, *PHV_OUTPUT_GET_SPA_PAGE_LIST;

//
// Code security hypercall structure definitions.
//

#if defined(XBOX_SYSTEMOS)

typedef union HV_CALL_ATTRIBUTES _HV_INPUT_VERIFY_SIGNATURE_PAGE {
    UINT64 AsUINT64[2];
    struct {
        HV_GPA SignatureGpa;
        HV_GPA VerifyGpa;
    };

    struct {
        UINT64 Index : 10;
        UINT64 IgnoreVerify : 1;
        UINT64 Catalog : 1;
        UINT64 SignatureGfn : 52;
        UINT64 Rsvd0 : 12;
        UINT64 VerifyGfn : 52;
    };

} HV_INPUT_VERIFY_SIGNATURE_PAGE, *PHV_INPUT_VERIFY_SIGNATURE_PAGE;

typedef union _HV_MAP_CODE_PAGE_INFO {
    UINT64 AsUINT64[3];
    struct {
        HV_GPA SignatureGpa;
        HV_GPA SourceGpa;
        HV_GPA MapGpa;
    };

    struct {
        UINT64 Index : 7;
        UINT64 Rsvd0 : 2;
        UINT64 Writeable : 1;
        UINT64 MapData : 1;
        UINT64 IgnoreVerify : 1;
        UINT64 SignatureGfn : 52;
        UINT64 Rsvd1 : 12;
        UINT64 SrcGfn : 52;
        UINT64 Rsvd2 : 11;
        UINT64 User : 1;
        UINT64 MapGfn : 52;
    };

} HV_MAP_CODE_PAGE_INFO, *PHV_MAP_CODE_PAGE_INFO;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_MAP_CODE_PAGE {
        HV_PARTITION_ID Id;
        HV_MAP_CODE_PAGE_INFO MapList[];
} HV_INPUT_MAP_CODE_PAGE, *PHV_INPUT_MAP_CODE_PAGE;

typedef union HV_CALL_ATTRIBUTES _HV_INPUT_UNMAP_CODE_PAGE {
    UINT64 AsUINT64;
    HV_GPA CodeGpa;
    struct {
        UINT64 CopyBack : 1;
        UINT64 NoFlush : 1;
        UINT64 Rsvd0 : 10;
        UINT64 CodeGfn : 52;
    };

} HV_INPUT_UNMAP_CODE_PAGE, *PHV_INPUT_UNMAP_CODE_PAGE;

typedef union HV_CALL_ATTRIBUTES _HV_INPUT_COPY_CODE_PAGE {
    UINT64 AsUINT64[3];
    struct {
        HV_GPA SourceGpa;
        HV_GPA TargetGpa;
        HV_GPA MapGpa;
    };

    struct {
        UINT64 Rsvd0 : 10;
        UINT64 NoFlush : 1;
        UINT64 Delete : 1;
        UINT64 SrcGfn : 52;
        UINT64 Rsvd1 : 12;
        UINT64 DstGfn : 52;
        UINT64 Rsvd2 : 12;
        UINT64 MapGfn : 52;
    };

} HV_INPUT_COPY_CODE_PAGE, *PHV_INPUT_COPY_CODE_PAGE;

typedef union HV_CALL_ATTRIBUTES _HV_INPUT_MAKE_CODE_PAGE_WRITEABLE {
    UINT64 AsUINT64;
    HV_GPA CodeGpa;
    struct {
        UINT64 Rsvd0 : 12;
        UINT64 CodeGfn : 52;
    };

} HV_INPUT_MAKE_CODE_PAGE_WRITEABLE, *PHV_INPUT_MAKE_CODE_PAGE_WRITEABLE;

typedef union HV_CALL_ATTRIBUTES _HV_INPUT_MAKE_DATA_PAGE_READONLY {
    UINT64 AsUINT64;
    HV_GPA DataGpa;
    struct {
        UINT64 Rsvd0 : 11;
        UINT64 Lock : 1;
        UINT64 DataGfn : 52;
    };

} HV_INPUT_MAKE_DATA_PAGE_READONLY, *PHV_INPUT_MAKE_DATA_PAGE_READONLY;

typedef union HV_CALL_ATTRIBUTES _HV_INPUT_MAKE_DATA_PAGE_WRITEABLE {
    UINT64 AsUINT64;
    HV_GPA DataGpa;
    struct {
        UINT64 Rsvd0 : 12;
        UINT64 DataGfn : 52;
    };

} HV_INPUT_MAKE_DATA_PAGE_WRITEABLE, *PHV_INPUT_MAKE_DATA_PAGE_WRITEABLE;

//
// Read code bytes.
//

typedef union _HV_READ_CODE_BYTES_INFO {
    UINT64 AsUINT64;
    struct {
        UINT64 Offset : 12;
        UINT64 CodeGfn : 52;
    };

} HV_READ_CODE_BYTES_INFO, *PHV_READ_CODE_BYTES_INFO;

typedef union HV_CALL_ATTRIBUTES _HV_INPUT_READ_CODE_BYTES {
    HV_READ_CODE_BYTES_INFO CodeList[2];
} HV_INPUT_READ_CODE_BYTES, *PHV_INPUT_READ_CODE_BYTES;

typedef struct _HV_OUTPUT_READ_CODE_BYTES {
    UINT32 Count;
    UINT8 Buffer[16];
} HV_OUTPUT_READ_CODE_BYTES, *PHV_OUTPUT_READ_CODE_BYTES;

//
// Get/set debugger data information.
//

typedef enum _HV_DEBUGGER_DATA_INFO_VERSION {
    HV_DEBUGGER_DATA_INFO_V1 = 1,
} HV_DEBUGGER_DATA_INFO_VERSION, *PHV_DEBUGGER_DATA_INFO_VERSION;

typedef struct HV_CALL_ATTRIBUTES _HV_DEBUGGER_DATA_INFO {
    HV_DEBUGGER_DATA_INFO_VERSION Version;
    UINT64 KdVersionBlockPtr;
    UINT64 KdDebuggerDataBlockPtr;
    UINT64 KdpDataBlockEncodedPtr;
    UINT64 KiWaitAlways;
    UINT64 KiWaitNever;
} HV_DEBUGGER_DATA_INFO, *PHV_DEBUGGER_DATA_INFO;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SET_DEBUGGER_DATA_INFO {
    HV_DEBUGGER_DATA_INFO DebuggerDataInfo;
} HV_INPUT_SET_DEBUGGER_DATA_INFO, *PHV_INPUT_SET_DEBUGGER_DATA_INFO;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_DEBUGGER_DATA_INFO {
    HV_PARTITION_ID PartitionId;
} HV_INPUT_GET_DEBUGGER_DATA_INFO, *PHV_INPUT_GET_DEBUGGER_DATA_INFO;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_DEBUGGER_DATA_INFO {
    HV_DEBUGGER_DATA_INFO DebuggerDataInfo;
} HV_OUTPUT_GET_DEBUGGER_DATA_INFO, *PHV_OUTPUT_GET_DEBUGGER_DATA_INFO;

#endif

//
//
// MSR that returns if we are sharing a debug device.
//

#define HV_X64_MSR_DEBUG_DEVICE_OPTIONS           HvSyntheticMsrDebugDeviceOptions
#define HV_DEBUG_SHARED_SERIAL_DEVICE             0x1
#define HV_DEBUG_SHARED_FW_DEVICE                 0x2
#define HV_DEBUG_SHARED_NET_DEVICE                0x4
#define HV_DEBUG_SYNTH_DEBUG_DEVICE               0x8

//
// Define synthetic debugging interface
//

#define HV_X64_MSR_SYNTH_DEBUG_CONTROL            HvSyntheticMsrSynthDebugControl
#define HV_X64_MSR_SYNTH_DEBUG_STATUS             HvSyntheticMsrSynthDebugStatus
#define HV_X64_MSR_SYNTH_DEBUG_SEND_BUFFER        HvSyntheticMsrSynthDebugSendBuffer
#define HV_X64_MSR_SYNTH_DEBUG_RECEIVE_BUFFER     HvSyntheticMsrSynthDebugReceiveBuffer
#define HV_X64_MSR_SYNTH_DEBUG_PENDING_BUFFER     HvSyntheticMsrSynthDebugPendingBuffer

#define HV_SYNTH_DEBUG_CONTROL_SEND 0x1
#define HV_SYNTH_DEBUG_CONTROL_RECEIVE 0x2

#define HV_SYNTH_DEBUG_STATUS_SEND_SUCCESS 0x1
#define HV_SYNTH_DEBUG_STATUS_SEND_FAILURE 0x2
#define HV_SYNTH_DEBUG_STATUS_PACKET_AVAILABLE 0x4
#define HV_SYNTH_DEBUG_STATUS_SNAPSHOT_APPLIED 0x8

#define HV_SYNTH_DEBUG_PACKET_SIZE_MASK 0xFFFF0000
#define HV_SYNTH_DEBUG_PACKET_SIZE_SHIFT 16

#define HV_SYNTH_DEBUG_GET_PACKET_SIZE(Value) \
        (((Value) & HV_SYNTH_DEBUG_PACKET_SIZE_MASK) >> HV_SYNTH_DEBUG_PACKET_SIZE_SHIFT)

typedef union _HV_X64_MSR_SYNTH_DEBUG_BUFFER_CONTENTS
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 ReservedZ0       : 12;
        UINT64 GpaPageNumber    : 40;
        UINT64 ReservedZ1       : 11;
        UINT64 Disabled         : 1;
    };
} HV_X64_MSR_SYNTH_DEBUG_BUFFER_CONTENTS, *PHV_X64_MSR_SYNTH_DEBUG_BUFFER_CONTENTS;

//
// MSR to retrieve the below 1MB page that is currently used by the
// hypervisor. This is reused when we disable the old hypervisor and
// launch the new hypervisor.
//

#define HV_X64_MSR_BELOW_1MB_PAGE                 HvSyntheticMsrBelow1MbPage

//
// Routine exposed by hvloader.lib to setup the hypervisor.
//
#if 0
INT32
HvlSetupHypervisor(
    __in UINT64 Below1MbPage,
    __out PHV_MINI_LOADER_BLOCK *MiniLoaderBlock
    );
#endif


//
// Map Guest Physical Address (GPA) related definitions.
//

//
// HV Map GPA (Guest Physical Address) Flags
//
// Definitions of flags to describe the access a partition has to a GPA page.
// (used with HV_MAP_GPA_FLAGS).
//

//
// The first byte is reserved for permissions.
//
#define HV_MAP_GPA_PERMISSIONS_NONE     0x0
#define HV_MAP_GPA_READABLE             0x1
#define HV_MAP_GPA_WRITABLE             0x2
#define HV_MAP_GPA_KERNEL_EXECUTABLE    0x4
#define HV_MAP_GPA_USER_EXECUTABLE      0x8
#define HV_MAP_GPA_EXECUTABLE           0xC
#define HV_MAP_GPA_PERMISSIONS_MASK     0xF

#define HV_MAP_GPA_ADJUSTABLE           0x10
#define HV_MAP_GPA_PERMISSIONS_MASK_WITH_ADJUST 0x1F

#define HV_MAP_GPA_PERMISSION_BITS      5
C_ASSERT(HV_MAP_GPA_PERMISSIONS_MASK_WITH_ADJUST == ((1 << HV_MAP_GPA_PERMISSION_BITS) - 1));

//
// The second byte contains flags that can be set for mapping information.
//
#define HV_MAP_GPA_ONES                 0x100
#define HV_MAP_GPA_NOT_PRESENT          HV_MAP_GPA_ONES
#define HV_MAP_GPA_ZEROED               0x200
#define HV_MAP_GPA_NO_ACCESS            0x400

#define HV_MAP_GPA_SPECIAL_MASK         (HV_MAP_GPA_ONES | \
                                         HV_MAP_GPA_ZEROED | \
                                         HV_MAP_GPA_NO_ACCESS)

#define HV_MAP_GPA_NOT_ACCESSED         0x800
#define HV_MAP_GPA_ADJUSTABLE_SPECIFIED 0x1000
#define HV_MAP_GPA_LARGE_PAGE           0x2000

//
// Prevent guest memory accesses from allocating cache lines.
//
#define HV_MAP_GPA_NOT_CACHED          0x4000

//
// The set of all flags that may be specified.
//
#define HV_MAP_GPA_FLAGS_MASK           0x7F1F

//
// The set of flags that can be used with ModifyVtlProtectionMask.
//

#define HV_MAP_GPA_VTL_ALLOW_MMIO           0x8000

#define HV_MAP_GPA_VTL_FLAGS_MASK \
    (HV_MAP_GPA_PERMISSIONS_MASK_WITH_ADJUST | HV_MAP_GPA_ADJUSTABLE_SPECIFIED | \
     HV_MAP_GPA_VTL_ALLOW_MMIO)

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
// Maximum number of pages that can be specified in a single
// HvCallMapGpaPages hypercall.
//
#define HV_MAP_GPA_MAX_PAGE_COUNT     \
    ((HV_PAGE_SIZE - sizeof(HV_INPUT_MAP_GPA_PAGES)) / \
     sizeof(HV_GPA_PAGE_NUMBER))

//
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
// Maximum number of pages that can be specified in a single
// HvCallMapSparseGpaPages hypercall.
//
#define HV_MAP_SPARSE_GPA_MAX_PAGE_COUNT    \
    ((HV_PAGE_SIZE - sizeof(HV_INPUT_MAP_SPARSE_GPA_PAGES)) / \
     sizeof(HV_GPA_MAPPING))

#if !defined(XBOX_SYSTEMOS)

//
// Facilities to query and report supported cpu management interface versions.
//

#define HV_X64_MSR_CPU_MANAGEMENT_VER   (HvSyntheticMsrCpuMgmtVersion)

typedef union _HV_CPU_MANAGEMENT_VERSION_REGISTER
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 VersionNumber:8;
        UINT64 RsvdZ:54;
        UINT64 Available:1;
        UINT64 Locked:1;
    };

} HV_CPU_MANAGEMENT_VERSION_REGISTER, *PHV_CPU_MANAGEMENT_VERSION_REGISTER;

//
// Cpu Management interface version.
//

#define HV_CPU_MANAGEMENT_INTERFACE_VERSION 1

#endif

//
// SVM support.
//
typedef UINT32 HV_PASID, *PHV_PASID;
typedef UINT32 HV_PASID_SPACE_ID, *PHV_PASID_SPACE_ID;

typedef union _HV_FULL_PASID
{
    struct
    {
        HV_PASID Pasid;
        HV_PASID_SPACE_ID PasidSpaceId;
    };

    UINT64 AsUINT64;

} HV_FULL_PASID, *PHV_FULL_PASID;

#define HV_PASID_INVALID            ((HV_PASID)-1)
#define HV_PASID_SPACE_ID_INVALID   ((HV_PASID_SPACE_ID)-1)
#define HV_FULL_PASID_INVALID       ((UINT64)-1)

typedef union _HV_DEVICE_ADDRESS_SPACE
{
    struct
    {
        UINT64 Present : 1;
        UINT64 Reserved : 11;
        UINT64 PageTableRootPfn : 52;
    };

    UINT64 AsUINT64;

} HV_DEVICE_ADDRESS_SPACE, *PHV_DEVICE_ADDRESS_SPACE;

typedef UINT64 HV_LOGICAL_DEVICE_ID, *PHV_LOGICAL_DEVICE_ID;

typedef struct _HV_DEVICE_PAGE_REQUEST
{
    HV_LOGICAL_DEVICE_ID DeviceId;
    UINT32 Pasid : 20;
    UINT32 Virtual : 1;
    UINT32 GroupIndex : 9;
    UINT32 LastInGroup : 1;
    UINT32 StreamRequest : 1;

    UINT32 Reserved0;

    UINT64 Read : 1;
    UINT64 Write : 1;
    UINT64 Execute : 1;
    UINT64 Privileged : 1;
    UINT64 Reserved1 : 8;
    UINT64 PageNumber : 52;

    UINT64 Reserved2;

    union
    {
        struct
        {
            UINT64 Reserved3 : 61;
            UINT64 InvalidRequest : 1;
            UINT64 ValidateGpa : 1;
            UINT64 SendResponse : 1;
        };

        struct
        {
            UINT64 : 63;
            UINT64 Overflow : 1;
        };
    };

} HV_DEVICE_PAGE_REQUEST, *PHV_DEVICE_PAGE_REQUEST;

typedef UINT32 HV_DEVICE_PR_QUEUE_ID, *PHV_DEVICE_PR_QUEUE_ID;

typedef struct _HV_DEVICE_PRQ_HEADER
{
    UINT32 Head;
    UINT32 Tail;
    UINT8 Pending;
    UINT8 Stalled;
    UINT8 Error;

} HV_DEVICE_PRQ_HEADER, *PHV_DEVICE_PRQ_HEADER;

#define HV_DEVICE_PRQ_MIN_SIZE (HV_PAGE_SIZE / sizeof(HV_DEVICE_PAGE_REQUEST))

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_CREATE_PASID_SPACE
{
    HV_PASID_SPACE_ID PasidSpaceId;
    UINT32 PasidCount;

} HV_INPUT_CREATE_PASID_SPACE, *PHV_INPUT_CREATE_PASID_SPACE;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_DELETE_PASID_SPACE
{
    HV_PASID_SPACE_ID PasidSpaceId;
    UINT32 Reserved;

} HV_INPUT_DELETE_PASID_SPACE, *PHV_INPUT_DELETE_PASID_SPACE;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SET_PASID_ADDRESS_SPACE
{
    HV_FULL_PASID FullPasid;
    HV_DEVICE_ADDRESS_SPACE AddressSpace;

} HV_INPUT_SET_PASID_ADDRESS_SPACE, *PHV_INPUT_SET_PASID_ADDRESS_SPACE;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_FLUSH_PASID_ADDRESS_SPACE
{
    HV_FULL_PASID FullPasid;
    UINT32 Flags;
    UINT32 Reserved;

} HV_INPUT_FLUSH_PASID_ADDRESS_SPACE, *PHV_INPUT_FLUSH_PASID_ADDRESS_SPACE;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_FLUSH_PASID_ADDRESS_LIST
{
    HV_FULL_PASID FullPasid;
    UINT32 Flags;
    UINT32 VaCount;
    HV_GVA_RANGE_EXTENDED VaList[];

} HV_INPUT_FLUSH_PASID_ADDRESS_LIST, *PHV_INPUT_FLUSH_PASID_ADDRESS_LIST;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_ATTACH_PASID_SPACE
{
    HV_LOGICAL_DEVICE_ID LogicalDeviceId;
    HV_PASID_SPACE_ID PasidSpaceId;
    HV_DEVICE_PR_QUEUE_ID PrQueueId;

} HV_INPUT_ATTACH_PASID_SPACE, *PHV_INPUT_ATTACH_PASID_SPACE;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_DETACH_PASID_SPACE
{
    HV_LOGICAL_DEVICE_ID LogicalDeviceId;

} HV_INPUT_DETACH_PASID_SPACE, *PHV_INPUT_DETACH_PASID_SPACE;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_ENABLE_PASID
{
    HV_LOGICAL_DEVICE_ID LogicalDeviceId;
    HV_PASID Pasid;
    UINT32 Reserved;

} HV_INPUT_ENABLE_PASID, *PHV_INPUT_ENABLE_PASID;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_DISABLE_PASID
{
    HV_LOGICAL_DEVICE_ID LogicalDeviceId;
    HV_PASID Pasid;
    UINT32 Reserved;

} HV_INPUT_DISABLE_PASID, *PHV_INPUT_DISABLE_PASID;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_CREATE_DEVICE_PR_QUEUE
{
    HV_DEVICE_PR_QUEUE_ID QueueId;
    UINT32 Size;
    HV_GPA_PAGE_NUMBER BaseGpaPage;
    HV_INTERRUPT_VECTOR InterruptVector;
    HV_VP_INDEX InterruptVpIndex;
    UINT32 Flags;
    UINT32 Reserved;

} HV_INPUT_CREATE_DEVICE_PR_QUEUE, *PHV_INPUT_CREATE_DEVICE_PR_QUEUE;

#define HV_CREATE_DEVICE_PRQ_INTERRUPT_MASKED 1

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_DELETE_DEVICE_PR_QUEUE
{
    HV_DEVICE_PR_QUEUE_ID QueueId;
    UINT32 Reserved;

} HV_INPUT_DELETE_DEVICE_PR_QUEUE, *PHV_INPUT_DELETE_DEVICE_PR_QUEUE;

typedef enum _HV_DEVICE_PRQ_PROPERTY
{
    HvDevicePrqPropertyStalled = 0,
    HvDevicePrqPropertyInterruptMasked = 1

} HV_DEVICE_PRQ_PROPERTY, *PHV_DEVICE_PRQ_PROPERTY;

typedef struct HV_CALL_ATTRIBUTES _HV_DEVICE_PRQ_PROPERTY_INTERRUPT_MASKED
{
    BOOLEAN InterruptMasked;
    UINT8 Reserved0;
    UINT16 Reserved1;
    UINT32 Reserved2;

} HV_DEVICE_PRQ_PROPERTY_INTERRUPT_MASKED, *PHV_DEVICE_PRQ_PROPERTY_INTERRUPT_MASKED;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SET_DEVICE_PRQ_PROPERTY_HEADER
{
    HV_DEVICE_PR_QUEUE_ID QueueId;
    HV_DEVICE_PRQ_PROPERTY Property;

} HV_INPUT_SET_DEVICE_PRQ_PROPERTY_HEADER, *PHV_INPUT_SET_DEVICE_PRQ_PROPERTY_HEADER;

//
// Root-only definitions (mostly for SVM).
//
typedef enum _HV_PHYSICAL_DEVICE_PROPERTY
{
    HvPhysicalDevicePropertyCapabilities = 0,
    HvPhysicalDevicePropertyEnabled = 1,
    HvPhysicalDevicePropertyFaultReporting = 2,

} HV_PHYSICAL_DEVICE_PROPERTY, *PHV_PHYSICAL_DEVICE_PROPERTY;

typedef struct HV_CALL_ATTRIBUTES _HV_PHYSICAL_DEVICE_PROPERTY_CAPABILITIES
{
    struct
    {
        UINT32 RootSvm : 1;
        UINT32 PciExecute : 1;
        UINT32 NoExecute : 1;
        UINT32 Reserved0 : 28;
        UINT32 OverflowPossible : 1;
    };

    UINT32 PasidCount;
    UINT32 RootSvmIommuIndex;
    UINT32 Reserved1;

} HV_PHYSICAL_DEVICE_PROPERTY_CAPABILITIES, *PHV_PHYSICAL_DEVICE_PROPERTY_CAPABILITIES;

typedef struct HV_CALL_ATTRIBUTES _HV_PHYSICAL_DEVICE_PROPERTY_ENABLED
{
    BOOLEAN PasidEnabled;
    UINT8 Reserved0;
    UINT16 Reserved1;
    UINT32 Reserved2;

} HV_PHYSICAL_DEVICE_PROPERTY_ENABLED, *PHV_PHYSICAL_DEVICE_PROPERTY_ENABLED;

typedef struct HV_CALL_ATTRIBUTES _HV_PHYSICAL_DEVICE_PROPERTY_FAULT_REPORTING
{
    BOOLEAN FaultReportingEnabled;
    UINT8 Reserved0;
    UINT16 Reserved1;
    UINT32 Reserved2;

} HV_PHYSICAL_DEVICE_PROPERTY_FAULT_REPORTING, *PHV_PHYSICAL_DEVICE_PROPERTY_FAULT_REPORTING;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_PHYSICAL_DEVICE_PROPERTY
{
    HV_DEVICE_ID PhysicalDeviceId;
    HV_PHYSICAL_DEVICE_PROPERTY Property;
    UINT32 Reserved;

} HV_INPUT_GET_PHYSICAL_DEVICE_PROPERTY, *PHV_INPUT_GET_PHYSICAL_DEVICE_PROPERTY;

typedef union HV_CALL_ATTRIBUTES _HV_OUTPUT_GET_PHYSICAL_DEVICE_PROPERTY
{
    HV_PHYSICAL_DEVICE_PROPERTY_CAPABILITIES Capabilities;
    HV_PHYSICAL_DEVICE_PROPERTY_ENABLED Enabled;
    HV_PHYSICAL_DEVICE_PROPERTY_FAULT_REPORTING FaultReporting;

} HV_OUTPUT_GET_PHYSICAL_DEVICE_PROPERTY, *PHV_OUTPUT_GET_PHYSICAL_DEVICE_PROPERTY;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SET_PHYSICAL_DEVICE_PROPERTY_HEADER
{
    HV_DEVICE_ID PhysicalDeviceId;
    HV_PHYSICAL_DEVICE_PROPERTY Property;
    UINT32 Reserved;

} HV_INPUT_SET_PHYSICAL_DEVICE_PROPERTY_HEADER, *PHV_INPUT_SET_PHYSICAL_DEVICE_PROPERTY_HEADER;

typedef union _HV_DEVICE_PCI_CAPABILITIES
{
    struct
    {
        UINT32 MaxPasidWidth : 5;
        UINT32 InvalidateQueueDepth : 5;
        UINT32 GlobalInvalidate : 1;
        UINT32 Reserved : 21;
    };

    UINT32 AsUINT32;

} HV_DEVICE_PCI_CAPABILITIES, *PHV_DEVICE_PCI_CAPABILITIES;

typedef union _HV_ATTACH_DEVICE_FLAGS
{
    struct
    {
        UINT32 LogicalId : 1;
        UINT32 PasidSupported : 1;
        UINT32 PasidEnabled : 1;
        UINT32 SharedInterruptsRoot : 1;
        UINT32 VirtualFunction : 1;
        UINT32 SharedInterruptsChild : 1;
        UINT32 VirtualDevice : 1;
        UINT32 LogicalInterrupts : 1;
        UINT32 Reserved : 24;
    };

    UINT32 AsUINT32;

} HV_ATTACH_DEVICE_FLAGS, *PHV_ATTACH_DEVICE_FLAGS;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_ATTACH_DEVICE
{
    HV_PARTITION_ID PartitionId;
    HV_DEVICE_ID DeviceId;
    HV_ATTACH_DEVICE_FLAGS Flags;
    HV_LOGICAL_DEVICE_ID LogicalDeviceId;
    HV_DEVICE_PCI_CAPABILITIES PciCapabilities;
    HV_PCI_RID PhysicalFunctionRid;
    UINT16 Reserved;

} HV_INPUT_ATTACH_DEVICE, *PHV_INPUT_ATTACH_DEVICE;


typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_DETACH_DEVICE
{
    HV_PARTITION_ID PartitionId;
    UINT64 DeviceId;

} HV_INPUT_DETACH_DEVICE, *PHV_INPUT_DETACH_DEVICE;

#define HV_DEVICE_DOMAIN_ID_TYPE_S2             0
#define HV_DEVICE_DOMAIN_ID_TYPE_S1             1
#define HV_DEVICE_DOMAIN_ID_TYPE_SOC            2


typedef union _HV_DEVICE_DOMAIN_ID
{
    UINT64 AsUINT64;
    struct
    {
        UINT32 Type:4;
        UINT32 Reserved:28;
        UINT32 Id;
    };
} HV_DEVICE_DOMAIN_ID, *PHV_DEVICE_DOMAIN_ID;

C_ASSERT(sizeof(HV_DEVICE_DOMAIN_ID) == sizeof(UINT64));

#define IS_HV_DEVICE_DOMAIN_EQUAL(_DevId_, _Type_, _Id_) \
    ((_DevId_).Type == (_Type_) && \
     (_DevId_).Id == (_Id_))

#define HV_DEVICE_DOMAIN_ID_S2_DEFAULT 0
#define HV_DEVICE_DOMAIN_ID_S2_NULL 0xFFFFFFFFULL

typedef struct _HV_INPUT_DEVICE_DOMAIN
{
    HV_PARTITION_ID PartitionId;
    HV_INPUT_VTL OwnerVtl;
    HV_DEVICE_DOMAIN_ID DomainId;

} HV_INPUT_DEVICE_DOMAIN, *PHV_INPUT_DEVICE_DOMAIN;

typedef struct _HV_INPUT_CREATE_DEVICE_DOMAIN
{
    HV_INPUT_DEVICE_DOMAIN DeviceDomain;

} HV_INPUT_CREATE_DEVICE_DOMAIN, *PHV_INPUT_CREATE_DEVICE_DOMAIN;

typedef struct _HV_INPUT_DELETE_DEVICE_DOMAIN
{
    HV_INPUT_DEVICE_DOMAIN DeviceDomain;

} HV_INPUT_DELETE_DEVICE_DOMAIN, *PHV_INPUT_DELETE_DEVICE_DOMAIN;

typedef struct _HV_INPUT_ATTACH_DEVICE_DOMAIN
{
    HV_INPUT_DEVICE_DOMAIN DeviceDomain;
    HV_DEVICE_ID DeviceId;

} HV_INPUT_ATTACH_DEVICE_DOMAIN, *PHV_INPUT_ATTACH_DEVICE_DOMAIN;

typedef struct _HV_INPUT_DETACH_DEVICE_DOMAIN
{
    HV_PARTITION_ID PartitionId;
    HV_DEVICE_ID DeviceId;

} HV_INPUT_DETACH_DEVICE_DOMAIN, *PHV_INPUT_DETACH_DEVICE_DOMAIN;

typedef struct _HV_INPUT_MAP_DEVICE_GPA_PAGES
{
    HV_INPUT_DEVICE_DOMAIN DeviceDomain;
    HV_INPUT_VTL TargetVtl;
    HV_MAP_GPA_FLAGS MapFlags;
    HV_DEVICE_VA TargetDeviceVaBase;
    HV_CALL_ATTRIBUTES HV_GPA_PAGE_NUMBER GpaPageList[];

} HV_INPUT_MAP_DEVICE_GPA_PAGES, *PHV_INPUT_MAP_DEVICE_GPA_PAGES;

typedef struct _HV_INPUT_UNMAP_DEVICE_GPA_PAGES
{
    HV_INPUT_DEVICE_DOMAIN DeviceDomain;
    HV_DEVICE_VA TargetDeviceVaBase;

} HV_INPUT_UNMAP_DEVICE_GPA_PAGES, *PHV_INPUT_UNMAP_DEVICE_GPA_PAGES;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_QUERY_DEVICE_DOMAIN
{
    HV_INPUT_DEVICE_DOMAIN DeviceDomain;
    HV_DEVICE_VA TargetDeviceVaList[];

} HV_INPUT_QUERY_DEVICE_DOMAIN, *PHV_INPUT_QUERY_DEVICE_DOMAIN;

typedef struct HV_CALL_ATTRIBUTES _HV_DEVICE_VA_MAPPING
{
    HV_DEVICE_VA TargetDeviceVa;
    HV_GPA_PAGE_NUMBER GpaPageNumber;

} HV_DEVICE_VA_MAPPING, *PHV_DEVICE_VA_MAPPING;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_MAP_SPARSE_DEVICE_GPA_PAGES
{
    HV_INPUT_DEVICE_DOMAIN DeviceDomain;
    HV_INPUT_VTL TargetVtl;
    HV_MAP_GPA_FLAGS MapFlags;
    HV_DEVICE_VA_MAPPING PageList[];

} HV_INPUT_MAP_SPARSE_DEVICE_GPA_PAGES, *PHV_INPUT_MAP_SPARSE_DEVICE_GPA_PAGES;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_UNMAP_SPARSE_DEVICE_GPA_PAGES
{
    HV_INPUT_DEVICE_DOMAIN DeviceDomain;
    HV_DEVICE_VA TargetDeviceVaList[];

} HV_INPUT_UNMAP_SPARSE_DEVICE_GPA_PAGES, *PHV_INPUT_UNMAP_SPARSE_DEVICE_GPA_PAGES;

typedef struct _HV_DEVICE_DOMAIN_SETTINGS
{
    struct
    {
        //
        // Enable translations. If not enabled, all transaction bypass S1
        // translations.
        //
        UINT64 TranslationEnabled : 1;

        //
        // Enable coherent translation table walks.
        //
        UINT64 CoherentTableWalks : 1;

        UINT64 Reserved : 61;
    } Flags;

    //
    // Address of translation table 0.
    //
    HV_GPA PageTableRoot0;

    //
    // Address of translation table 1.
    //
    HV_GPA PageTableRoot1;

    //
    // Size of the lower VA subrange (2^(64-T0Sz)).
    //
    UINT8 InputSize0;

    //
    // Size of the upper VA subrange (2^(64-T1Sz)).
    //
    UINT8 InputSize1;

    //
    // Memory attribute indirection registers.
    //
    UINT32 Mair0;
    UINT32 Mair1;

    //
    // Address space identifier.
    //
    UINT16 Asid;

} HV_DEVICE_DOMAIN_SETTINGS, *PHV_DEVICE_DOMAIN_SETTINGS;

typedef struct _HV_INPUT_CONFIGURE_DEVICE_DOMAIN
{
    HV_INPUT_DEVICE_DOMAIN DeviceDomain;

    HV_DEVICE_DOMAIN_SETTINGS Settings;

} HV_INPUT_CONFIGURE_DEVICE_DOMAIN, *PHV_INPUT_CONFIGURE_DEVICE_DOMAIN;

typedef struct _HV_INPUT_FLUSH_DEVICE_DOMAIN
{
    HV_INPUT_DEVICE_DOMAIN DeviceDomain;

    struct
    {
        //
        // Flush global TLB entries.
        //
        UINT32 Global : 1;

        UINT32 Reserved : 31;

    } Flags;

} HV_INPUT_FLUSH_DEVICE_DOMAIN, *PHV_INPUT_FLUSH_DEVICE_DOMAIN;

//
// Definitions for the HvCallAddPhysicalMemory hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_ADD_PHYSICAL_MEMORY
{
    HV_SPA_PAGE_NUMBER StartPage;
    UINT64 PageCount;

} HV_INPUT_ADD_PHYSICAL_MEMORY, *PHV_INPUT_ADD_PHYSICAL_MEMORY;

typedef struct HV_CALL_ATTRIBUTES _HV_OUTPUT_ADD_PHYSICAL_MEMORY
{
    UINT64 PagesProcessed;

} HV_OUTPUT_ADD_PHYSICAL_MEMORY, *PHV_OUTPUT_ADD_PHYSICAL_MEMORY;

//
// Definitions for the HvCallPrecommitGpaPages hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_PRECOMMIT_GPA_PAGES
{
    //
    // Supplies the partition ID of the partition that this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies the base guest physical page number where the GPA space will be
    // precommitted.
    //

    HV_GPA_PAGE_NUMBER TargetGpaBase;

} HV_INPUT_PRECOMMIT_GPA_PAGES, *PHV_INPUT_PRECOMMIT_GPA_PAGES;

//
// Definitions for the HvCallUncommitGpaPages hypercall.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_UNCOMMIT_GPA_PAGES
{
    //
    // Supplies the partition ID of the partition that this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies the base guest physical page number where the GPA space will be
    // uncommitted.
    //

    HV_GPA_PAGE_NUMBER TargetGpaBase;

} HV_INPUT_UNCOMMIT_GPA_PAGES, *PHV_INPUT_UNCOMMIT_GPA_PAGES;

#if defined(_AMD64_)

//
// Definitions for the HvCallGetGpaPagesAccessState and
// HvCallGetSparseGpaPagesAccessState hypercalls.
//

typedef union _HV_GPA_PAGE_ACCESS_STATE_FLAGS
{
    UINT64 AsUINT64;

    struct
    {
        //
        // Clear the accessed flag. This enables access tracking for the page.
        // Access tracking can be disabled by setting the accessed flag.
        //
        UINT64 ClearAccessed : 1;

        //
        // Set the accessed flag. This disables access tracking (and rebuilds
        // large pages).
        //
        UINT64 SetAccessed : 1;

        //
        // Clear the dirty flag. This enables dirty tracking for the page.
        // Dirty tracking can be disabled by setting the dirty flag.
        //
        UINT64 ClearDirty : 1;

        //
        // Set the dirty flag. This disables dirty tracking (and rebuilds
        // large pages).
        //
        UINT64 SetDirty : 1;

        UINT64 Reserved : 60;
    };
} HV_GPA_PAGE_ACCESS_STATE_FLAGS, *PHV_GPA_PAGE_ACCESS_STATE_FLAGS;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_GPA_PAGES_ACCESS_STATE
{
    //
    // Supplies the partition ID of the partition that this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies flags for the operation.
    //

    HV_GPA_PAGE_ACCESS_STATE_FLAGS Flags;

    //
    // Supplies the base guest physical page number for the operation.
    //

    HV_GPA_PAGE_NUMBER TargetGpaBase;

} HV_INPUT_GET_GPA_PAGES_ACCESS_STATE, *PHV_INPUT_GET_GPA_PAGES_ACCESS_STATE;

typedef union _HV_GPA_PAGE_ACCESS_STATE
{
    UINT8 AsUINT8;

    struct
    {
        UINT8 Accessed : 1;
        UINT8 Dirty : 1;
    };

} HV_GPA_PAGE_ACCESS_STATE, *PHV_GPA_PAGE_ACCESS_STATE;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_SPARSE_GPA_PAGES_ACCESS_STATE
{
    //
    // Supplies the partition ID of the partition that this request is for.
    //

    HV_PARTITION_ID TargetPartitionId;

    //
    // Supplies flags for the operation.
    //

    HV_GPA_PAGE_ACCESS_STATE_FLAGS Flags;

    //
    // Supplies an array of guest physical page numbers for the operation.
    //

    HV_CALL_ATTRIBUTES HV_GPA_PAGE_NUMBER GpaPageList[];

} HV_INPUT_GET_SPARSE_GPA_PAGES_ACCESS_STATE, *PHV_INPUT_GET_SPARSE_GPA_PAGES_ACCESS_STATE;

#endif

#if defined(_AMD64_)

//
// ARM64HV_WORKITEM - We should be able to support this for ARM64, as long as
// we hook it up via the system register mechanism.
//

typedef union _HV_MSR_WATCHDOG_CONFIG_CONTENTS
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 Enable   : 1;
        UINT64 AutoEnable : 1;
        UINT64 RsvdZ    : 62;
    };

} HV_MSR_WATCHDOG_CONFIG_CONTENTS, *PHV_MSR_WATCHDOG_CONFIG_CONTENTS;

#define HV_X64_MSR_SWATCHDOG_CONFIG  HvSyntheticMsrSWatchdogConfig
#define HV_X64_MSR_SWATCHDOG_COUNT   HvSyntheticMsrSWatchdogCount
#define HV_X64_MSR_SWATCHDOG_STATUS  HvSyntheticMsrSWatchdogStatus

#endif

//
// Definitions for the HvCallSwitchAliasMap hypercall.
//
#define HV_ALIAS_MAP_DEFAULT    0
#define HV_ALIAS_MAP_ALIAS      1

//
// Definitions for HvCallUpdateMicrocode.
//
#if defined (_AMD64_)

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_UPDATE_MICROCODE
{
    HV_GVA MicrocodePayloadGva;
    UINT32 MicrocodePayloadSize;
    UINT32 RsvdZ;

} HV_INPUT_UPDATE_MICROCODE, *PHV_INPUT_UPDATE_MICROCODE;

#endif

#if defined(_AMD64_)

//
// Type definitions for hot-patch hypercalls.
//
typedef enum _HV_IMAGE_QUERY_TYPE
{
    ImageQueryTypeModuleCount,
    ImageQueryTypeModuleTraits,
    ImageQueryTypeSlotVaMappings,
    ImageQueryTypeMax
} HV_IMAGE_QUERY_TYPE, *PHV_IMAGE_QUERY_TYPE;

//
// Define the number of hot-patchable modules. This is currently limited due to VA
// space and to avoid over-engg.
// Currently assumes that hv?x64.exe is the first module.
//
#define HV_HOTPATCH_MAXIMUM_MODULE_COUNT                (2)
#define HV_HOTPATCH_MAXIMUM_MODULE_INDEX                (HV_HOTPATCH_MAXIMUM_MODULE_COUNT - 1)

//
// Define the number of patch slots for every hot-patchable image. 
// Internally, Slot#0 aka primary slot refers to base image.
//
#define HV_HOTPATCH_MAXIMUM_SECONDARY_SLOT_COUNT        (2)
#define HV_HOTPATCH_TOTAL_SLOT_COUNT                    (HV_HOTPATCH_MAXIMUM_SECONDARY_SLOT_COUNT + 1)
#define HV_HOTPATCH_MAXIMUM_SLOT_INDEX                  (HV_HOTPATCH_TOTAL_SLOT_COUNT - 1)

//
// Define the number of characters in the name of a module.
//
#define HV_MODULE_NAME_LENGTH                           (8)

typedef struct _HV_SLOT_TRAITS
{
    //
    // Base address of this region.
    //
    HV_GVA StartVa;

    //
    // Number of bytes.
    //
    UINT32 Size;

    //
    // True for an image's base image slot.
    //
    UINT32 IsPrimarySlot: 1;

    //
    // True if a slot is active. Primary slot is always active.
    //
    UINT32 IsSlotActive : 1;

    UINT32 ReservedZ0   : 30;

    //
    // Checksum and timestamp of the image in the slot.
    // The slot must be active, for these values to be valid.
    //
    UINT32 Checksum;
    UINT32 Timestamp;

} HV_SLOT_TRAITS, *PHV_SLOT_TRAITS;

typedef struct _HV_MODULE_TRAITS
{
    //
    // Null terminated name of the module.
    // RSG_TODO: Confirm it is null terminated.
    //
    WCHAR ModuleName[HV_MODULE_NAME_LENGTH];

    HV_SLOT_TRAITS SlotTraits[HV_HOTPATCH_TOTAL_SLOT_COUNT];

} HV_MODULE_TRAITS, *PHV_MODULE_TRAITS;

typedef union _HV_GPA_PAGE_PERMISSION
{
    UINT64 AsUINT64;
    
    struct
    {
        UINT64 Valid        : 1;
        UINT64 Writable     : 1;
        UINT64 Executable   : 1;
        UINT64 ReservedZ0   : 9;
        UINT64 GpaPage      : 40;
        UINT64 ReservedZ1   : 12;
    };
    
} HV_GPA_PAGE_PERMISSION, *PHV_GPA_PAGE_PERMISSION;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_QUERY_IMAGE_INFO
{
    HV_IMAGE_QUERY_TYPE QueryType;

    UINT32 ReservedZ0;

    union
    {
        //
        // Identify the specific module for ImageQueryTypeModuleInfo
        //
        UINT64 ModuleIndex;

        //
        // Identify the module,slot tuple and retrieve VA-PA mappings in the specified range.
        //
        struct
        {
            UINT64 ModuleIndex  : 4;
            UINT64 SlotIndex    : 2;
            UINT64 PageCount    : 16;
            UINT64 ReservedZ0   : 42;
            HV_GVA StartVa;
        } Mapping;
    };

} HV_INPUT_QUERY_IMAGE_INFO, *PHV_INPUT_QUERY_IMAGE_INFO;


#define HV_MAXIMUM_SLOT_VA_MAPPING_COUNT                (512)
typedef union HV_CALL_ATTRIBUTES _HV_OUTPUT_QUERY_IMAGE_INFO
{
    //
    // Returns the number of modules for ImageQueryTypeModuleCount
    //
    UINT32 ModuleCount;

    //
    // Returns the module layout information for ImageQueryTypeModuleInfo.
    //
    HV_MODULE_TRAITS ModuleTraits;

    //
    // Returns the HVA-SPA translations and permissions of an image, slot tuple for
    // ImageQueryTypeSlotVaMappings.
    //
    HV_GPA_PAGE_PERMISSION GpaPagePerm[HV_MAXIMUM_SLOT_VA_MAPPING_COUNT]; //PFN+Perms.

} HV_OUTPUT_QUERY_IMAGE_INFO, *PHV_OUTPUT_QUERY_IMAGE_INFO;

typedef union _HV_VA_PERMISSION
{
    UINT64 AsUINT64;

    struct
    {
        UINT64 Valid            : 1;
        UINT64 Writable         : 1;
        UINT64 Executable       : 1;
        UINT64 ReservedZ0       : 9;
        UINT64 VirtualPageFrame : 52;
    };

} HV_VA_PERMISSION, *PHV_VA_PERMISSION;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_MAP_IMAGE_PAGES
{
    //
    // Identify the module index for this operation.
    //
    UINT64 ModuleIndex  : 4;

    //
    // Identify a specific patch slot in this module.
    //
    UINT64 SlotIndex    : 2;

    UINT64 ReservedZ0   : 58;

    //
    // Mappings for pages beginning at StartVa.
    //
    HV_VA_PERMISSION VirtualPageFramePerm[];
} HV_INPUT_MAP_IMAGE_PAGES, *PHV_INPUT_MAP_IMAGE_PAGES;

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_COMMIT_PATCH
{
    //
    // Identify the module index for this operation.
    //
    UINT64 ModuleIndex      : 4;

    //
    // Identify a specific patch slot in this module.
    //
    UINT64 SlotIndex        : 2;

    //
    // Indicates a valid argument for patch routine.
    //
    UINT64 UseTargetPatch   : 1;

    UINT64 ReservedZ0       : 57;

    //
    // Patch routine VA, in the target slot.
    //
    HV_GVA PatchRoutineVa;

    //
    // Supplies the pages backing the input image.
    //
    HV_MEMORY_DESCRIPTOR Descriptor;

} HV_INPUT_COMMIT_PATCH, *PHV_INPUT_COMMIT_PATCH;

#endif

#endif // _HVHDK_MINI
