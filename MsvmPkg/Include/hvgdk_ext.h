/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    hvgdk_ext.h

Abstract:

    Declarations for extended hypercalls and other features exposed by the
    virtualization stack.

--*/

#if !defined(_HVGDK_EXT_)
#define _HVGDK_EXT_

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

#if !defined(XBOX_SYSTEMOS)

//
// Extended hypercall codes.
//

typedef enum _HV_EXT_CALL
{
    //
    // Reserved Hypercall Code.
    //
    HvExtCallReserved                   = 0x8000,
    HvExtCallQueryCapabilities          = 0x8001,
    HvExtCallGetBootZeroedMemory        = 0x8002,
    HvExtCallMemoryHeatHint             = 0x8003,
    HvExtCallEpfSetup                   = 0x8004,
    HvExtCallSchedulerAssistSetup       = 0x8005,
    HvExtCallMax
} HV_EXT_CALL, *PHV_EXT_CALL;

//
// Extended hypercall parameter definitions.
//

//
// Output parameter for HvExtCallQueryCapabilities.
//

typedef union HV_CALL_ATTRIBUTES _HV_EXT_OUTPUT_QUERY_CAPABILITIES
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 GetBootZeroedMemory      : 1;
        UINT64 MemoryHotHint            : 1;
        UINT64 MemoryColdHint           : 1;
        UINT64 Epf                      : 1;
        UINT64 RootScheduler            : 1;
        UINT64 PowerSchedulerQos        : 1;
        UINT64 Reserved                 : 58;
    };
} HV_EXT_OUTPUT_QUERY_CAPABILITIES, *PHV_EXT_OUTPUT_QUERY_CAPABILITIES;

//
// Output parameter for HvExtCallGetBootZeroedMemory and associated types.
//

typedef struct _HV_EXT_OUTPUT_BOOT_ZEROED_MEMORY_RANGE
{
    UINT64 StartGpa;
    UINT64 PageCount;
} HV_EXT_OUTPUT_BOOT_ZEROED_MEMORY_RANGE, *PHV_EXT_OUTPUT_BOOT_ZEROED_MEMORY_RANGE;

//
// Because hypercall output is limited to a single page, there's an upper bound on
// the number of returned ranges as defined below.
//
#define HV_EXT_BOOT_ZEROED_MEMORY_MAX_RANGE_COUNT \
    (((HV_PAGE_SIZE) - sizeof(UINT64)) \
        / sizeof(HV_EXT_OUTPUT_BOOT_ZEROED_MEMORY_RANGE))

typedef struct HV_CALL_ATTRIBUTES _HV_EXT_OUTPUT_BOOT_ZEROED_MEMORY
{
    UINT64 RangeCount;
    HV_EXT_OUTPUT_BOOT_ZEROED_MEMORY_RANGE ZeroedRanges[HV_EXT_BOOT_ZEROED_MEMORY_MAX_RANGE_COUNT];
} HV_EXT_OUTPUT_BOOT_ZEROED_MEMORY, *PHV_EXT_OUTPUT_BOOT_ZEROED_MEMORY;

//
// Definitions for HvExtCallMemoryHeatHint.
//

typedef enum _HV_EXT_MEMORY_HEAT_HINT_TYPE
{
    HvExtMemoryHeatHintTypeCold = 0,
    HvExtMemoryHeatHintTypeHot = 1,
    HvExtMemoryHeatHintTypeMax
} HV_EXT_MEMORY_HEAT_HINT_TYPE;

typedef struct HV_CALL_ATTRIBUTES _HV_EXT_INPUT_MEMORY_HEAT_HINT
{
    UINT64 Heat : 2; // HV_EXT_MEMORY_HEAT_HINT_TYPE
    UINT64 Reserved : 62;
    HV_GPA_PAGE_RANGE Ranges[];
} HV_EXT_INPUT_MEMORY_HEAT_HINT, *PHV_EXT_INPUT_MEMORY_HEAT_HINT;

#define HV_EXT_MEMORY_HEAT_HINT_MAX_RANGE_COUNT \
    ((HV_PAGE_SIZE - FIELD_OFFSET(HV_EXT_INPUT_MEMORY_HEAT_HINT, Ranges)) / \
        sizeof(HV_GPA_PAGE_RANGE))

//
// EPF shared definitions.
//

#define HV_EXT_EPF_VERSION 1

//
// Definitions for HvExtCallEpfSetup.
//

//
// The EPF delivery mode. Each value represents a known method for determining
// whether a stage 2 fault can be converted into an EPF fault, and how the
// synchronous EPF fault should be sent to the faulting VP.
//
typedef enum _HV_EXT_EPF_MODE
{
    HvExtEpfModeMin = 0,

    HvExtEpfModeNt = HvExtEpfModeMin,  // IF and (IRQL < DISPATCH_LEVEL)

    HvExtEpfModeMax
} HV_EXT_EPF_MODE, *PHV_EXT_EPF_MODE;

typedef struct HV_CALL_ATTRIBUTES _HV_EXT_INPUT_EPF_SETUP
{
    //
    // The version requested by the guest.
    //
    UINT32 Version;

    //
    // The delivery mode requested by the guest.
    //
    HV_EXT_EPF_MODE Mode;

    //
    // CompletionQueueGpaPage and CompletionQueuePageCount specify the location
    // of the EPF completion queue in GPA space. (StartGpaPage is a GPA page
    // index.)
    //
    UINT64 CompletionQueueGpaPage;
    UINT64 CompletionQueuePageCount;
} HV_EXT_INPUT_EPF_SETUP, *PHV_EXT_INPUT_EPF_SETUP;

typedef struct HV_CALL_ATTRIBUTES _HV_EXT_OUTPUT_EPF_SETUP
{
    //
    // The number of pages actually being used by the host for the completion
    // queue.
    //
    UINT64 CompletionQueuePageCount;
} HV_EXT_OUTPUT_EPF_SETUP, *PHV_EXT_OUTPUT_EPF_SETUP;

//
// Shared circular queue for receiving EPF completion notifications.
//
typedef struct _HV_EXT_EPF_COMPLETION_QUEUE
{
    UINT64 ReadCursor;
    UINT64 WriteCursor;
    UINT64 Queue[1];
} HV_EXT_EPF_COMPLETION_QUEUE, *PHV_EXT_EPF_COMPLETION_QUEUE;

//
// Virtualization fault definitions.
//

typedef enum _HV_EXT_VIRTUALIZATION_FAULT_CODE
{
    HvExtVirtualizationFaultNone = 0,
    HvExtVirtualizationFaultEpf = 1,

    HvExtVirtualizationFaultMax
} HV_EXT_VIRTUALIZATION_FAULT_CODE, *PHV_EXT_VIRTUALIZATION_FAULT_CODE;

//
// Input parameter for HvExtCallSetupSchedulerAssist.
//
// The GPA is a location of the scheduler assist data.
//

typedef enum _HV_EXT_SCHEDULER_ASSIST_MODE
{
    HvExtSchedulerAssistModeNone = 0,
    HvExtSchedulerAssistModeNt = 1,

    HvExtSchedulerAssistModeMax
} HV_EXT_SCHEDULER_ASSIST_MODE, *PHV_EXT_SCHEDULER_ASSIST_MODE;

//
//  N.B. Version 1 was used in RS3 and only matching version is
//       supported.  Barcelona by default should have the same version
//       as the host and the underneath HV.
//

#define HV_EXT_SCHEDULER_ASSIST_DATA_VERSION 2

typedef struct HV_CALL_ATTRIBUTES _HV_EXT_INPUT_SETUP_SCHEDULER_ASSIST
{
    UINT32 Version;
    UINT32 Mode;
    UINT64 SchedulerAssistDataGpa;
} HV_EXT_INPUT_SETUP_SCHEDULER_ASSIST, *PHV_EXT_INPUT_SETUP_SCHEDULER_ASSIST;

typedef enum _HV_EXT_SCHEDULER_ASSIST_ACTION
{
    HvExtSchedulerAssistSetDataPage = 0,
    HvExtSchedulerAssistUpdateBamQos = 1,
    HvExtSchedulerAssistUpdatePriority = 2,
    HvExtSchedulerAssistRemoveSystemWorkBoost = 3,

    HvExtSchedulerAssistActionMax
} HV_EXT_SCHEDULER_ASSIST_ACTION, *PHV_EXT_SCHEDULER_ASSIST_ACTION;

//
// Contents of the SchedulerAssist data.
//
// Fields written by guest and read by the root:
// GuestPriority
// BamQosLevel
// TargetVpIndex
// SchedulerAssistAction
// DoNotPreempt
//
// Fields written by the root and read by the guest:
// RootPriority
// RootInReadyQueue
//
// Field written or read by both guest and root's
// current running VP:
// SystemWork
//

typedef struct _HV_EXT_SCHEDULER_ASSIST_DATA
{
    INT32 GuestPriority;
    INT32 RootPriority;
    UINT32 BamQosLevel;
    UINT32 TargetVpIndex;
    UINT32 SchedulerAssistAction;
    UINT8 DoNotPreempt;
    UINT8 SystemWork;
    UINT8 RootInReadyQueue;

} HV_EXT_SCHEDULER_ASSIST_DATA, *PHV_EXT_SCHEDULER_ASSIST_DATA;

//
//  Helper macros to read from or write to the scheduler assist
//  shared memory.
//
//  N.B. Both host and guest can update certain fields within the shared
//       memory at anytime.
//       The root integrated scheduler on the host will locally copy
//       the field content and probe it before applying the scheduling
//       parameters on the associated VPbacking thread.
//

#define HvExtSchedulerAssistReadULong(SchedulerAssistData, Field) \
    ReadULongNoFence((PULONG)&(SchedulerAssistData)->Field)

#define HvExtSchedulerAssistWriteULong(SchedulerAssistData, Field, Value) \
    WriteULongNoFence((PULONG)&(SchedulerAssistData)->Field, (ULONG)Value)

#define HvExtSchedulerAssistReadUChar(SchedulerAssistData, Field) \
    ReadUCharNoFence((PUCHAR)&(SchedulerAssistData)->Field)

#define HvExtSchedulerAssistWriteUChar(SchedulerAssistData, Field, Value) \
    WriteUCharNoFence((PUCHAR)&(SchedulerAssistData)->Field, (UCHAR)Value)

#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4200) /* nonstandard extension used : zero-sized array in struct/union */
#pragma warning(default:4214) /* nonstandard extension used : bit field types other then int */
#pragma warning(default:4324) /* structure was padded due to __declspec(align()) */
#endif

#endif //_HVGDK_EXT_
