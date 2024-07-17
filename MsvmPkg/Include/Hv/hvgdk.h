/*++

Copyright (c) Microsoft Corporation

Module Name:

    HvGdk.h

Abstract:

    Type definitions for the hypervisor guest interface.

Author:

    Hypervisor Engineering Team (hvet) 01-May-2005

--*/

#if !defined(_HVGDK_)
#define _HVGDK_

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

#include <Hv/HvGdk_mini.h>
#include <Hv/HvGdk_ext.h>


//
// Memory Types
//
// System physical addresses (SPAs) define the physical address space of the underlying
// hardware. There is only one system physical address space for the entire machine.
//
// Guest virtual addresses (GVAs) are used within the guest when it enables address
// translation and provides a valid guest page table.
//

typedef UINT64 HV_SPA, *PHV_SPA;
typedef UINT64 HV_GVA, *PHV_GVA;

#ifndef X64_PAGE_SIZE
#define X64_PAGE_SIZE 0x1000
#endif

typedef UINT64 HV_GPA_PAGE_NUMBER, *PHV_GPA_PAGE_NUMBER;
typedef UINT64 HV_GVA_PAGE_NUMBER, *PHV_GVA_PAGE_NUMBER;

typedef const HV_GPA_PAGE_NUMBER *PCHV_GPA_PAGE_NUMBER;
typedef const HV_GVA_PAGE_NUMBER *PCHV_GVA_PAGE_NUMBER;

//
// MessageId: HV_STATUS_SUCCESS
//
// MessageText:
//
// The specified hypercall succeeded
//
#define HV_STATUS_SUCCESS                ((HV_STATUS)0x0000)

//
// MessageId: HV_STATUS_INVALID_HYPERCALL_CODE
//
// MessageText:
//
// The hypervisor does not support the operation because the specified hypercall code is not supported.
//
#define HV_STATUS_INVALID_HYPERCALL_CODE ((HV_STATUS)0x0002)

//
// MessageId: HV_STATUS_INVALID_HYPERCALL_INPUT
//
// MessageText:
//
// The hypervisor does not support the operation because the encoding for the hypercall input register is not supported.
//
#define HV_STATUS_INVALID_HYPERCALL_INPUT ((HV_STATUS)0x0003)

//
// MessageId: HV_STATUS_INVALID_ALIGNMENT
//
// MessageText:
//
// The hypervisor could not perform the operation because a parameter has an invalid alignment.
//
#define HV_STATUS_INVALID_ALIGNMENT      ((HV_STATUS)0x0004)

//
// MessageId: HV_STATUS_INVALID_PARAMETER
//
// MessageText:
//
// The hypervisor could not perform the operation because an invalid parameter was specified.
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
// MessageId: HV_STATUS_INVALID_PARTITION_STATE
//
// MessageText:
//
// The hypervisor could not perform the operation because the partition is entering or in an invalid state.
//
#define HV_STATUS_INVALID_PARTITION_STATE ((HV_STATUS)0x0007)

//
// MessageId: HV_STATUS_OPERATION_DENIED
//
// MessageText:
//
// The operation is not allowed in the current state.
//
#define HV_STATUS_OPERATION_DENIED       ((HV_STATUS)0x0008)

//
// MessageId: HV_STATUS_UNKNOWN_PROPERTY
//
// MessageText:
//
// The hypervisor does not recognize the specified partition property.
//
#define HV_STATUS_UNKNOWN_PROPERTY       ((HV_STATUS)0x0009)

//
// MessageId: HV_STATUS_PROPERTY_VALUE_OUT_OF_RANGE
//
// MessageText:
//
// The specified value of a partition property is out of range or violates an invariant.
//
#define HV_STATUS_PROPERTY_VALUE_OUT_OF_RANGE ((HV_STATUS)0x000A)

//
// MessageId: HV_STATUS_INSUFFICIENT_MEMORY
//
// MessageText:
//
// There is not enough memory in the hypervisor pool to complete the operation.
//
#define HV_STATUS_INSUFFICIENT_MEMORY    ((HV_STATUS)0x000B)

//
// MessageId: HV_STATUS_PARTITION_TOO_DEEP
//
// MessageText:
//
// The maximum partition depth has been exceeded for the partition hierarchy.
//
#define HV_STATUS_PARTITION_TOO_DEEP     ((HV_STATUS)0x000C)

//
// MessageId: HV_STATUS_INVALID_PARTITION_ID
//
// MessageText:
//
// A partition with the specified partition Id does not exist.
//
#define HV_STATUS_INVALID_PARTITION_ID   ((HV_STATUS)0x000D)

//
// MessageId: HV_STATUS_INVALID_VP_INDEX
//
// MessageText:
//
// The hypervisor could not perform the operation because the specified VP index is invalid.
//
#define HV_STATUS_INVALID_VP_INDEX       ((HV_STATUS)0x000E)

//
// MessageId: HV_STATUS_NOT_FOUND
//
// MessageText:
//
// The iteration is complete; no addition items in the iteration could be found.
//
#define HV_STATUS_NOT_FOUND              ((HV_STATUS)0x0010)

//
// MessageId: HV_STATUS_INVALID_PORT_ID
//
// MessageText:
//
// The hypervisor could not perform the operation because the specified port identifier is invalid.
//
#define HV_STATUS_INVALID_PORT_ID        ((HV_STATUS)0x0011)

//
// MessageId: HV_STATUS_INVALID_CONNECTION_ID
//
// MessageText:
//
// The hypervisor could not perform the operation because the specified connection identifier is invalid.
//
#define HV_STATUS_INVALID_CONNECTION_ID  ((HV_STATUS)0x0012)

//
// MessageId: HV_STATUS_INSUFFICIENT_BUFFERS
//
// MessageText:
//
// You did not supply enough message buffers to send a message.
//
#define HV_STATUS_INSUFFICIENT_BUFFERS   ((HV_STATUS)0x0013)

//
// MessageId: HV_STATUS_NOT_ACKNOWLEDGED
//
// MessageText:
//
// The previous virtual interrupt has not been acknowledged.
//
#define HV_STATUS_NOT_ACKNOWLEDGED       ((HV_STATUS)0x0014)

//
// MessageId: HV_STATUS_INVALID_VP_STATE
//
// MessageText:
//
// A virtual processor is not in the correct state for the performance of the indicated operation.
//
#define HV_STATUS_INVALID_VP_STATE       ((HV_STATUS)0x0015)

//
// MessageId: HV_STATUS_ACKNOWLEDGED
//
// MessageText:
//
// The previous virtual interrupt has already been acknowledged.
//
#define HV_STATUS_ACKNOWLEDGED           ((HV_STATUS)0x0016)

//
// MessageId: HV_STATUS_INVALID_SAVE_RESTORE_STATE
//
// MessageText:
//
// The indicated partition is not in a valid state for saving or restoring.
//
#define HV_STATUS_INVALID_SAVE_RESTORE_STATE ((HV_STATUS)0x0017)

//
// MessageId: HV_STATUS_INVALID_SYNIC_STATE
//
// MessageText:
//
// The hypervisor could not complete the operation because a required feature of the synthetic interrupt controller (SynIC) was disabled.
//
#define HV_STATUS_INVALID_SYNIC_STATE    ((HV_STATUS)0x0018)

//
// MessageId: HV_STATUS_OBJECT_IN_USE
//
// MessageText:
//
// The hypervisor could not perform the operation because the object or value was either already in use or being used for a purpose that would not permit completing the operation.
//
#define HV_STATUS_OBJECT_IN_USE          ((HV_STATUS)0x0019)

//
// MessageId: HV_STATUS_INVALID_PROXIMITY_DOMAIN_INFO
//
// MessageText:
//
// The proximity domain information is invalid.
//
#define HV_STATUS_INVALID_PROXIMITY_DOMAIN_INFO ((HV_STATUS)0x001A)

//
// MessageId: HV_STATUS_NO_DATA
//
// MessageText:
//
// An attempt to retrieve debugging data failed because none was available.
//
#define HV_STATUS_NO_DATA                ((HV_STATUS)0x001B)

//
// MessageId: HV_STATUS_INACTIVE
//
// MessageText:
//
// The physical connection being used for debuggging has not recorded any receive activity since the last operation.
//
#define HV_STATUS_INACTIVE               ((HV_STATUS)0x001C)

//
// MessageId: HV_STATUS_NO_RESOURCES
//
// MessageText:
//
// There are not enough resources to complete the operation.
//
#define HV_STATUS_NO_RESOURCES           ((HV_STATUS)0x001D)

//
// MessageId: HV_STATUS_FEATURE_UNAVAILABLE
//
// MessageText:
//
// A hypervisor feature is not available to the user.
//
#define HV_STATUS_FEATURE_UNAVAILABLE    ((HV_STATUS)0x001E)

//
// MessageId: HV_STATUS_PARTIAL_PACKET
//
// MessageText:
//
// The debug packet returned is only a partial packet due to an io error.
//
#define HV_STATUS_PARTIAL_PACKET         ((HV_STATUS)0x001F)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor feature.
//
#define HV_STATUS_PROCESSOR_FEATURE_NOT_SUPPORTED ((HV_STATUS)0x0020)

//
// MessageId: HV_STATUS_PROCESSOR_CACHE_LINE_FLUSH_SIZE_INCOMPATIBLE
//
// MessageText:
//
// The supplied restore state requires requires a processor with a different
// cache line flush size.
//
#define HV_STATUS_PROCESSOR_CACHE_LINE_FLUSH_SIZE_INCOMPATIBLE ((HV_STATUS)0x0030)

//
// MessageId: HV_STATUS_INSUFFICIENT_BUFFER
//
// MessageText:
//
// The specified buffer was too small to contain all of the requested data.
//
#define HV_STATUS_INSUFFICIENT_BUFFER    ((HV_STATUS)0x0033)

//
// MessageId: HV_STATUS_INCOMPATIBLE_PROCESSOR
//
// MessageText:
//
// The supplied restore state is for an incompatible processor
// vendor.
//
#define HV_STATUS_INCOMPATIBLE_PROCESSOR ((HV_STATUS)0x0037)

//
// MessageId: HV_STATUS_INSUFFICIENT_DEVICE_DOMAINS
//
// MessageText:
//
// The maximum number of domains supported by the platform I/O remapping
// hardware is currently in use. No domains are available to assign this device
// to this partition.
//
#define HV_STATUS_INSUFFICIENT_DEVICE_DOMAINS ((HV_STATUS)0x0038)

//
// MessageId: HV_STATUS_CPUID_FEATURE_VALIDATION_ERROR
//
// MessageText:
//
// Generic logical processor CPUID feature set validation error.
//
#define HV_STATUS_CPUID_FEATURE_VALIDATION_ERROR ((HV_STATUS)0x003C)

//
// MessageId: HV_STATUS_CPUID_XSAVE_FEATURE_VALIDATION_ERROR
//
// MessageText:
//
// CPUID XSAVE feature validation error.
//
#define HV_STATUS_CPUID_XSAVE_FEATURE_VALIDATION_ERROR ((HV_STATUS)0x003D)

//
// MessageId: HV_STATUS_PROCESSOR_STARTUP_TIMEOUT
//
// MessageText:
//
// Processor startup timed out.
//
#define HV_STATUS_PROCESSOR_STARTUP_TIMEOUT ((HV_STATUS)0x003E)

//
// MessageId: HV_STATUS_SMX_ENABLED
//
// MessageText:
//
// SMX enabled by the BIOS.
//
#define HV_STATUS_SMX_ENABLED ((HV_STATUS)0x003F)

//
// MessageId: HV_STATUS_INVALID_LP_INDEX
//
// MessageText:
//
// The hypervisor could not perform the operation because the specified LP index is invalid.
//
#define HV_STATUS_INVALID_LP_INDEX ((HV_STATUS)0x0041)

//
// MessageId: HV_STATUS_INVALID_REGISTER_VALUE
//
// MessageText:
//
// The supplied register value is invalid.
//
#define HV_STATUS_INVALID_REGISTER_VALUE ((HV_STATUS)0x0050)

//
// MessageId: HV_STATUS_INVALID_VTL_STATE
//
// MessageText:
//
// The supplied virtual trust level is not in the correct state to perform the requested operation.
//
#define HV_STATUS_INVALID_VTL_STATE ((HV_STATUS)0x0051)

//
// MessageId: HV_STATUS_NX_NOT_DETECTED
//
// MessageText:
//
// NX not detected on the machine.
//
#define HV_STATUS_NX_NOT_DETECTED ((HV_STATUS)(0x0055))

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
// MessageId: HV_STATUS_KEY_ALREADY_EXISTS
//
// MessageText:
//
// The entry cannot be added as another entry with the same key already exists.
//
#define HV_STATUS_KEY_ALREADY_EXISTS     ((HV_STATUS)0x0065)

//
// MessageId: HV_STATUS_DEVICE_ALREADY_IN_DOMAIN
//
// MessageText:
//
// The device is already attached to the device domain.
//
#define HV_STATUS_DEVICE_ALREADY_IN_DOMAIN     ((HV_STATUS)0x0066)

//
// MessageId: HV_STATUS_INVALID_CPU_GROUP_ID
//
// MessageText:
//
// A CPU group with the specified CPU group Id does not exist.
//
#define HV_STATUS_INVALID_CPU_GROUP_ID ((HV_STATUS)0x006F)

//
// MessageId: HV_STATUS_INVALID_CPU_GROUP_STATE
//
// MessageText:
//
// The hypervisor could not perform the operation because the CPU group is entering or in an invalid state.
//
#define HV_STATUS_INVALID_CPU_GROUP_STATE ((HV_STATUS)0x0070)

//
// MessageId: HV_STATUS_OPERATION_FAILED
//
// MessageText:
//
// The requested operation failed.
//
#define HV_STATUS_OPERATION_FAILED       ((HV_STATUS)0x0071)

//
// MessageId: HV_STATUS_NOT_ALLOWED_WITH_NESTED_VIRT_ACTIVE
//
// MessageText:
//
// The requested operation is not allowed due to one or more virtual processors
// having nested virtualization active.
//
#define HV_STATUS_NOT_ALLOWED_WITH_NESTED_VIRT_ACTIVE  ((HV_STATUS)0x0072)

//
// MessageId: HV_STATUS_INSUFFICIENT_ROOT_MEMORY
//
// MessageText:
//
// There is not enough memory in the root partition's pool to complete the operation.
//
#define HV_STATUS_INSUFFICIENT_ROOT_MEMORY ((HV_STATUS)0x0073)

typedef union _HV_PICO100_DURATION
{
    UINT64 AsUINT64;
} HV_PICO100_DURATION;

//
// Definition of the VP exit reason structure
//

typedef union _HV_X64_MSR_VP_EXIT_REASON_CONTENTS
{
    UINT64 AsUINT64;
    struct
    {
        UINT32 AdditionalReason;
        UINT8  Reason;
        UINT8  ReservedZ[3];
    };
} HV_X64_MSR_VP_EXIT_REASON_CONTENTS, *PHV_X64_MSR_VP_EXIT_REASON_CONTENTS;

//
// Declare structure used to control the VP exit interrupts
//

typedef union _HV_X64_MSR_VP_EXIT_INTERRUPT_CONTROL_CONTENTS
{
    UINT64 AsUINT64;
    struct
    {
        UINT8 Vector;
        UINT8 SampleRate;
        UINT8 ReservedZ[6];
    };
} HV_X64_MSR_VP_EXIT_INTERRUPT_CONTROL_CONTENTS, *PHV_X64_MSR_VP_EXIT_INTERRUPT_CONTROL_CONTENTS;

//
// VP Exit Tracing Event Types. These masks are ORed together to
// control the set of events that is enabled.
//

#define HV_TR_VP_EXIT_NONE                        0x0000000000000000

#define HV_TR_VP_EXIT_HYPERCALL                   0x0000000000000001
#define HV_TR_VP_EXIT_GUEST_EXCEPTION             0x0000000000000002
#define HV_TR_VP_EXIT_MSR_READ                    0x0000000000000004
#define HV_TR_VP_EXIT_MSR_WRITE                   0x0000000000000008
#define HV_TR_VP_EXIT_CR_READ                     0x0000000000000010
#define HV_TR_VP_EXIT_CR_WRITE                    0x0000000000000020
#define HV_TR_VP_EXIT_HLT_INSTRUCTION             0x0000000000000040
#define HV_TR_VP_EXIT_MWAIT_INSTRUCTION           0x0000000000000080
#define HV_TR_VP_EXIT_CPUID_INSTRUCTION           0x0000000000000100
#define HV_TR_VP_EXIT_IO_PORT_READ                0x0000000000000200
#define HV_TR_VP_EXIT_IO_PORT_WRITE               0x0000000000000400
#define HV_TR_VP_EXIT_EMULATED_INSTRUCTION        0x0000000000000800
#define HV_TR_VP_EXIT_INVLPG_INSTRUCTION          0x0000000000001000
#define HV_TR_VP_EXIT_IRET_INSTRUCTION            0x0000000000002000
#define HV_TR_VP_EXIT_TASK_SWITCH                 0x0000000000004000
#define HV_TR_VP_EXIT_INVD_INSTRUCTION            0x0000000000008000
#define HV_TR_VP_EXIT_DR_ACCESS                   0x0000000000010000
#define HV_TR_VP_EXIT_FERR_FREEZE                 0x0000000000020000
#define HV_TR_VP_EXIT_MEMORY_INTERCEPT            0x0000000000040000
#define HV_TR_VP_EXIT_REFLECTED_EXCEPTION         0x0000000000080000
#define HV_TR_VP_EXIT_APIC_EOI                    0x0000000000100000
#define HV_TR_VP_EXIT_APIC_WRITE                  0x0000000000200000
#define HV_TR_VP_EXIT_APIC_ACCESS                 0x0000000000400000
#define HV_TR_VP_EXIT_NESTED_PAGE_FAULT           0x0000000000800000
#define HV_TR_VP_EXIT_PAUSE_LOOP_EXIT             0x0000000001000000
#define HV_TR_VP_EXIT_CONTEXT_SWITCH              0x0000000002000000

#define HV_TR_VP_EXIT_VAILD_BITS                  0x0000000003FFFFFF

#define HV_TR_VP_EXIT_ALL                         0xFFFFFFFFFFFFFFFF

//
// Declare the MSRs used to control VP exit tracing and get the
// reasons associated with the event interrupts
//

#define HV_TR_VP_EXIT_INTERRUPTS_DISABLED_VECTOR 0

#define HV_X64_MSR_VP_EXIT_REASON (0x40000074)
#define HV_X64_MSR_VP_EXIT_INTERRUPT_CONTROL (0x40000075)
#define HV_X64_MSR_VP_EXIT_TRACE_EVENTS_CONTROL (0x40000076)



#if defined(_ARM64_)

#define HV_INTERRUPT_STATE_REGISTER HV_ARM64_INTERRUPT_STATE_REGISTER
#define PHV_INTERRUPT_STATE_REGISTER PHV_ARM64_INTERRUPT_STATE_REGISTER

#define HV_PENDING_INTERRUPTION_TYPE HV_ARM64_PENDING_INTERRUPTION_TYPE
#define PHV_PENDING_INTERRUPTION_TYPE PHV_ARM64_PENDING_INTERRUPTION_TYPE

#define HV_PENDING_INTERRUPTION_REGISTER HV_ARM64_PENDING_INTERRUPTION_REGISTER
#define PHV_PENDING_INTERRUPTION_REGISTER PHV_ARM64_PENDING_INTERRUPTION_REGISTER

#define HV_PENDING_EVENT  HV_ARM64_PENDING_EVENT
#define PHV_PENDING_EVENT  PHV_ARM64_PENDING_EVENT

#define HV_PENDING_EVENT_TYPE HV_ARM64_PENDING_EVENT_TYPE

#define HvPendingEventException HvArm64PendingEventException
#define HvPendingEventHypercallOutput HvArm64PendingEventHypercallOutput

#else

#define HV_INTERRUPT_STATE_REGISTER HV_X64_INTERRUPT_STATE_REGISTER
#define PHV_INTERRUPT_STATE_REGISTER PHV_X64_INTERRUPT_STATE_REGISTER

#define HV_PENDING_INTERRUPTION_TYPE HV_X64_PENDING_INTERRUPTION_TYPE
#define PHV_PENDING_INTERRUPTION_TYPE PHV_X64_PENDING_INTERRUPTION_TYPE

#define HV_PENDING_INTERRUPTION_REGISTER HV_X64_PENDING_INTERRUPTION_REGISTER
#define PHV_PENDING_INTERRUPTION_REGISTER PHV_X64_PENDING_INTERRUPTION_REGISTER

#define HV_PENDING_EVENT  HV_X64_PENDING_EVENT
#define PHV_PENDING_EVENT  PHV_X64_PENDING_EVENT

#define HV_PENDING_EVENT_TYPE HV_X64_PENDING_EVENT_TYPE

#define HvPendingEventException HvX64PendingEventException
#define HvPendingEventHypercallOutput HvX64PendingEventHypercallOutput

#endif


typedef const HV_REGISTER_VALUE *PCHV_REGISTER_VALUE;

//
// Define intercept types.
//
typedef enum _HV_INTERCEPT_TYPE
{

#if defined(_AMD64_)

    //
    // Platform-specific intercept types.
    //
    HvInterceptTypeX64IoPort = 0x00000000,
    HvInterceptTypeX64Msr = 0x00000001,
    HvInterceptTypeX64Cpuid = 0x00000002,

#endif

    HvInterceptTypeException = 0x00000003,
    HvInterceptTypeRegister = 0x00000004,
    HvInterceptTypeMmio = 0x00000005,

#if defined (_AMD64_)

    HvInterceptTypeX64GlobalCpuid = 0x00000006,

#endif

    HvInterceptTypeMax,
    HvInterceptTypeInvalid = 0xFFFFFFFF,

} HV_INTERCEPT_TYPE, *PHV_INTERCEPT_TYPE;


#if !defined(_ARM64_)

//
// Define IO port type.
//
typedef UINT16 HV_X64_IO_PORT, *PHV_X64_IO_PORT;

#endif

//
// Define intercept parameters.
//
typedef union _HV_INTERCEPT_PARAMETERS
{
    //
    // HV_INTERCEPT_PARAMETERS is defined to be an 8-byte field.
    //
    UINT64 AsUINT64;


#if !defined(_ARM64_)

    //
    // HvInterceptTypeX64IoPort.
    //
    HV_X64_IO_PORT IoPort;

#endif

    //
    // HvInterceptTypeX64Cpuid.
    //
    UINT32 CpuidIndex;

    //
    // HvInterceptTypeException.
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
// Define connection identifier type.
//

typedef union _HV_CONNECTION_ID
{
    UINT32 AsUINT32;

    struct
    {
        UINT32 Id:24;
        UINT32 Reserved:8;
    };

} HV_CONNECTION_ID, *PHV_CONNECTION_ID;

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
        UINT32  TargetVtl               :  4;
        UINT32  Reserved1               : 15;
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
// Debug channel identifier
//
typedef UINT16 HV_DEBUG_CHANNEL_IDENTIFIER;

//
// Buffer counts for debugger support.
//

#define DBG_PACKETS_BUFFERED_MAX 160
#define DBG_PACKETS_BUFFERED_FOR_POST 2
#define DBG_PACKETS_BUFFERED_FOR_RETRIEVE 2
#define DBG_PACKETS_BUFFERED_FOR_POST_NET 2
#define DBG_PACKETS_BUFFERED_FOR_RETRIEVE_NET 160

#define DBG_POST_BUFFER_SIZE (DBG_PACKETS_BUFFERED_FOR_POST * HV_PAGE_SIZE)
#define DBG_RETRIEVE_BUFFER_SIZE (DBG_PACKETS_BUFFERED_FOR_RETRIEVE * HV_PAGE_SIZE)

#define DBG_POST_BUFFER_SIZE_HV (DBG_PACKETS_BUFFERED_FOR_POST_HV * HV_PAGE_SIZE)
#define DBG_RETRIEVE_BUFFER_SIZE_HV (DBG_PACKETS_BUFFERED_FOR_RETRIEVE_HV * HV_PAGE_SIZE)

#define DBG_INVALID_INDEX 0xBADBAD

//
// Mux Protocol Defines
//
#define HV_MUX_PACKET_LEADER            0x11223344

#define HV_MUX_PACKET_TYPE_DATA             0x0001
#define HV_MUX_PACKET_TYPE_BREAKIN          0x0002
#define HV_MUX_PACKET_TYPE_QUERY_CHANNELS   0x0003

#define HV_MUX_PACKET_TYPE_MAXIMUM          HV_MUX_PACKET_TYPE_QUERY_CHANNELS

#pragma pack(1)
typedef struct _HV_MUX_PACKET_HEADER
{
    UINT32 Leader;
    UINT16 Type;
    UINT16 Length;
    UINT32 CRC;
    HV_DEBUG_CHANNEL_IDENTIFIER Channel;
    UINT16 Reserved;

} HV_MUX_PACKET_HEADER, *PHV_MUX_PACKET_HEADER;

//
// Channel data returned in a HV_MUX_PACKET_TYPE_QUERY_CHANNELS
// respone. The channelIds arrays is variable length array
//

typedef struct
{
    UINT32 Count;
    HV_DEBUG_CHANNEL_IDENTIFIER ChannelIds[1];

} MUX_CHANNEL_DATA, *PMUX_CHANNEL_DATA;

#pragma pack()

//
// Debug Channel Id
//
#define HV_DEBUG_CHANNEL_ID_HYPERVISOR      0x00000000
#define HV_DEBUG_CHANNEL_ID_ROOT            0x00000001
#define HV_DEBUG_CHANNEL_ID_DEFAULT         0x0000BADA
#define HV_DEBUG_CHANNEL_ID_ASSIGN_START    0x00000002
#define HV_DEBUG_CHANNEL_ID_FW_MAX          0x0000003E

//
// Definition of the HvCallSwitchVirtualAddressSpace hypercall input
// structure.  This call switches the guest's virtual address space.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SWITCH_VIRTUAL_ADDRESS_SPACE
{
    HV_ADDRESS_SPACE_ID AddressSpace;
} HV_INPUT_SWITCH_VIRTUAL_ADDRESS_SPACE,
  *PHV_INPUT_SWITCH_VIRTUAL_ADDRESS_SPACE;

//
// Definition of the HvPostMessage hypercall input structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_POST_MESSAGE
{
    HV_CONNECTION_ID    ConnectionId;
    UINT32              Reserved;
    HV_MESSAGE_TYPE     MessageType;
    UINT32              PayloadSize;
    UINT64              Payload[HV_MESSAGE_PAYLOAD_QWORD_COUNT];
} HV_INPUT_POST_MESSAGE, *PHV_INPUT_POST_MESSAGE;


//
// Definition of the HvSignalEvent hypercall input structure.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_SIGNAL_EVENT
{
    HV_CONNECTION_ID ConnectionId;
    UINT16           FlagNumber;
    UINT16           RsvdZ;
} HV_INPUT_SIGNAL_EVENT, *PHV_INPUT_SIGNAL_EVENT;

//
// Definition of the HcpHvNotifyLongSpinWait hypercall input
// structure.  This call switches notifies the hypervisor of a long running
// spinlock acquisition failure.
//
typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_NOTIFY_LONG_SPINWAIT
{
    UINT64 InitialLongSpinWait;
} HV_INPUT_NOTIFY_LONG_SPINWAIT, *PHV_INPUT_NOTIFY_LONG_SPINWAIT;

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

    //
    // Supplies the flags to use for the unmapping.
    //

    HV_UNMAP_GPA_FLAGS UnmapFlags;

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
    // Reserved for future use - potentially for a address space ID.
    //

    UINT32 Rsvdz;

    //
    // Supplies an array of GPA page numbers to modify.
    //

    HV_CALL_ATTRIBUTES HV_GPA_PAGE_NUMBER GpaPageList[];

} HV_INPUT_MODIFY_SPARSE_GPA_PAGES, *PHV_INPUT_MODIFY_SPARSE_GPA_PAGES;

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

#endif //_HVGDK_
