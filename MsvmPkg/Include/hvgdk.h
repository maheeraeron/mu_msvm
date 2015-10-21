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

#include <HvGdk_mini.h>


//
// Memory Types
//
//
// Guest virtual addresses (GVAs) are used within the guest when it enables address
// translation and provides a valid guest page table.
//

typedef UINT64 HV_GVA, *PHV_GVA;

#ifndef X64_PAGE_SIZE
#define X64_PAGE_SIZE 0x1000
#endif

typedef UINT64 HV_GVA_PAGE_NUMBER, *PHV_GVA_PAGE_NUMBER;

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
// The hypervisor could not perform the operation beacuse a parameter has an invalid alignment.
//
#define HV_STATUS_INVALID_ALIGNMENT      ((HV_STATUS)0x0004)

//
// MessageId: HV_STATUS_INVALID_PARAMETER
//
// MessageText:
//
// The hypervisor could not perform the operation beacuse an invalid parameter was specified.
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
// MessageId: HV_STATUS_PROCESSOR_FEATURE_SSE3_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (SSE3).
//
#define HV_STATUS_PROCESSOR_FEATURE_SSE3_NOT_SUPPORTED ((HV_STATUS)0x0020)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_LAHFSAHF_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (LAHFSAHF).
//
#define HV_STATUS_PROCESSOR_FEATURE_LAHFSAHF_NOT_SUPPORTED ((HV_STATUS)0x0021)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_SSSE3_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (SSSE3).
//
#define HV_STATUS_PROCESSOR_FEATURE_SSSE3_NOT_SUPPORTED ((HV_STATUS)0x0022)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_SSE4_1_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (SSE4.1).
//
#define HV_STATUS_PROCESSOR_FEATURE_SSE4_1_NOT_SUPPORTED ((HV_STATUS)0x0023)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_SSE4_2_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (SSE4.2).
//
#define HV_STATUS_PROCESSOR_FEATURE_SSE4_2_NOT_SUPPORTED ((HV_STATUS)0x0024)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_SSE4A_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (SSE4a).
//
#define HV_STATUS_PROCESSOR_FEATURE_SSE4A_NOT_SUPPORTED ((HV_STATUS)0x0025)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_XOP_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (XOP).
//
#define HV_STATUS_PROCESSOR_FEATURE_XOP_NOT_SUPPORTED ((HV_STATUS)0x0026)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_POPCNT_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (POPCNT).
//
#define HV_STATUS_PROCESSOR_FEATURE_POPCNT_NOT_SUPPORTED ((HV_STATUS)0x0027)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_CMPXCHG16B_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (CMPXCHG16B).
//
#define HV_STATUS_PROCESSOR_FEATURE_CMPXCHG16B_NOT_SUPPORTED ((HV_STATUS)0x0028)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_ALTMOVCR8_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (ALTMOVCR8).
//
#define HV_STATUS_PROCESSOR_FEATURE_ALTMOVCR8_NOT_SUPPORTED ((HV_STATUS)0x0029)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_LZCNT_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (LZCNT).
//
#define HV_STATUS_PROCESSOR_FEATURE_LZCNT_NOT_SUPPORTED ((HV_STATUS)0x002A)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_MISALIGNED_SSE_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (misaligned SSE).
//
#define HV_STATUS_PROCESSOR_FEATURE_MISALIGNED_SSE_NOT_SUPPORTED ((HV_STATUS)0x002B)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_MMX_EXT_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (MMX EXT).
//
#define HV_STATUS_PROCESSOR_FEATURE_MMX_EXT_NOT_SUPPORTED ((HV_STATUS)0x002C)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_3DNOW_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (3DNow!).
//
#define HV_STATUS_PROCESSOR_FEATURE_3DNOW_NOT_SUPPORTED ((HV_STATUS)0x002D)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_EXTENDED_3DNOW_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (extended 3DNow!).
//
#define HV_STATUS_PROCESSOR_FEATURE_EXTENDED_3DNOW_NOT_SUPPORTED ((HV_STATUS)0x002E)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_PAGE_1GB_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (1GB pages).
//
#define HV_STATUS_PROCESSOR_FEATURE_PAGE_1GB_NOT_SUPPORTED ((HV_STATUS)0x002F)

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
// MessageId: HV_STATUS_PROCESSOR_FEATURE_XSAVE_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (XSAVE).
//
#define HV_STATUS_PROCESSOR_FEATURE_XSAVE_NOT_SUPPORTED ((HV_STATUS)0x0031)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_XSAVE_XSAVEOPT_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (XSAVEOPT).
//
#define HV_STATUS_PROCESSOR_FEATURE_XSAVEOPT_NOT_SUPPORTED ((HV_STATUS)0x0032)

//
// MessageId: HV_STATUS_INSUFFICIENT_BUFFER
//
// MessageText:
//
// The specified buffer was too small to contain all of the requested data.
//
#define HV_STATUS_INSUFFICIENT_BUFFER    ((HV_STATUS)0x0033)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_XSAVE_AVX_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (AVX).
//
#define HV_STATUS_PROCESSOR_FEATURE_XSAVE_AVX_NOT_SUPPORTED ((HV_STATUS)0x0034)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_XSAVE_FEATURE_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported XSAVE processor
// feature
//
#define HV_STATUS_PROCESSOR_FEATURE_XSAVE_FEATURE_NOT_SUPPORTED ((HV_STATUS)0x0035)

//
// MessageId: HV_STATUS_PROCESSOR_XSAVE_SAVE_AREA_INCOMPATIBLE
//
// MessageText:
//
// The supplied restore state is incompatible with the processor's XSAVE save
// layout.
//
#define HV_STATUS_PROCESSOR_XSAVE_SAVE_AREA_INCOMPATIBLE ((HV_STATUS)0x0036)

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
// MessageId: HV_STATUS_PROCESSOR_FEATURE_AES_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (AES).
//
#define HV_STATUS_PROCESSOR_FEATURE_AES_NOT_SUPPORTED ((HV_STATUS)0x0039)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_PCLMULQDQ_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (PCLMULQDQ).
//
#define HV_STATUS_PROCESSOR_FEATURE_PCLMULQDQ_NOT_SUPPORTED ((HV_STATUS)0x003A)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_INCOMPATIBLE_XSAVE_FEATURES
//
// MessageText:
//
// The supplied restore state enables incompatible XSAVE features.
// (Enabling AVX without XSAVE/enabling XSAVEOPT without XSAVE)
//
#define HV_STATUS_PROCESSOR_FEATURE_INCOMPATIBLE_XSAVE_FEATURES ((HV_STATUS)0x003B)

//
// MessageId: HV_STATUS_CPUID_PROCESSOR_PHY_ADDR_LIMIT_EXCEEDED
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
// MessageId: HV_STATUS_PROCESSOR_FEATURE_PCID_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (PCID).
//
#define HV_STATUS_PROCESSOR_FEATURE_PCID_NOT_SUPPORTED ((HV_STATUS)0x0040)

//
// MessageId: HV_STATUS_INVALID_LP_INDEX
//
// MessageText:
//
// The hypervisor could not perform the operation because the specified LP index is invalid.
//
#define HV_STATUS_INVALID_LP_INDEX ((HV_STATUS)0x0041)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_FMA4_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (FMA4).
//
#define HV_STATUS_PROCESSOR_FEATURE_FMA4_NOT_SUPPORTED ((HV_STATUS)0x0042)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_F16C_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (F16C).
//
#define HV_STATUS_PROCESSOR_FEATURE_F16C_NOT_SUPPORTED ((HV_STATUS)0x0043)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_RDRAND_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (RDRAND).
//
#define HV_STATUS_PROCESSOR_FEATURE_RDRAND_NOT_SUPPORTED ((HV_STATUS)0x0044)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_RDWRFSGS_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (Read/Write FS/GS).
//
#define HV_STATUS_PROCESSOR_FEATURE_RDWRFSGS_NOT_SUPPORTED ((HV_STATUS)0x0045)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_SMEP_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (SMEP).
//
#define HV_STATUS_PROCESSOR_FEATURE_SMEP_NOT_SUPPORTED ((HV_STATUS)0x0046)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_ENHANCED_FAST_STRING_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (Enhanced Fast String).
//
#define HV_STATUS_PROCESSOR_FEATURE_ENHANCED_FAST_STRING_NOT_SUPPORTED ((HV_STATUS)0x0047)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_MOVBE_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (MovBe Instruction).
//
#define HV_STATUS_PROCESSOR_FEATURE_MOVBE_NOT_SUPPORTED ((HV_STATUS)0x0048)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_BMI1_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (Bmi1).
//
#define HV_STATUS_PROCESSOR_FEATURE_BMI1_NOT_SUPPORTED ((HV_STATUS)0x0049)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_BMI2_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (Bmi2).
//
#define HV_STATUS_PROCESSOR_FEATURE_BMI2_NOT_SUPPORTED ((HV_STATUS)0x004A)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_HLE_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (Hle).
//
#define HV_STATUS_PROCESSOR_FEATURE_HLE_NOT_SUPPORTED ((HV_STATUS)0x004B)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_RTM_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (Rtm).
//
#define HV_STATUS_PROCESSOR_FEATURE_RTM_NOT_SUPPORTED ((HV_STATUS)0x004C)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_XSAVE_FMA_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (Fma).
//
#define HV_STATUS_PROCESSOR_FEATURE_XSAVE_FMA_NOT_SUPPORTED ((HV_STATUS)0x004D)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_XSAVE_AVX2_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (Avx2).
//
#define HV_STATUS_PROCESSOR_FEATURE_XSAVE_AVX2_NOT_SUPPORTED ((HV_STATUS)0x004E)

//
// MessageId: HV_STATUS_PROCESSOR_FEATURE_NPIEP1_NOT_SUPPORTED
//
// MessageText:
//
// The supplied restore state requires an unsupported processor
// processor feature (NPIEP1).
//
#define HV_STATUS_PROCESSOR_FEATURE_NPIEP1_NOT_SUPPORTED ((HV_STATUS)0x004F)

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
// This structure is used to transfer crashdump information between the
// Hypervisor and the HvBoot.sys driver in the root Windows instance at the
// time of a Hypervisor BugCheck.  It is allocated by winload during the
// Hypervisor launch process, and its SPA is handed in to the Hypervisor via
// the loader block. The structure must be kept in sync with hvgdk_root.h.
//

#define HV_CRASHDUMP_AREA_VERSION   4
#define HV_IMAGE_NAME_MAX_LENGTH    32

typedef struct _HV_CRASHDUMP_AREA_V1
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
    // Loaded Module Information
    //

    UINT64 HypervisorBase;
    UINT32 SizeOfImage;
    UINT16 ImageNameLength;
    WCHAR ImageName[HV_IMAGE_NAME_MAX_LENGTH];

    //
    // Bugcheck error code fields
    //

    UINT64 BugCheckData[5];
    void  *BugCheckErrorReturnAddress;

    //
    // The root of the page table needed to lookup virtual addresses
    // and the debugger data block. The debugger data block contains
    // all the information necc. for the debugger to interpret the
    // dump file. Of particular interest within it is the prcb address
    // that contain the processor state.
    //

    UINT64 PageTableBase;
    UINT64 PfnDataBase;
    UINT64 DebuggerDataBlock;
    UINT32 NumberProcessors;

    //
    // Context of the crashing thread
    //

    UINT32 ProcessorStateOffset;
    UINT32 ProcessorStateSize;

    //
    // The stack of crashing thread.
    //

    UINT32 CrashStackSize;
    UINT32 CrashStackOffset;
    UINT64 CrashStackAddress;

} HV_CRASHDUMP_AREA_V1, *PHV_CRASHDUMP_AREA_V1;


typedef struct _HV_CRASHDUMP_PROCESSOR_STATE
{
    //
    // Context of the crashing thread.
    //

    UINT32 RegisterStateOffset;
    UINT32 RegisterStateSize;

    //
    // The stack of the crashing thread.
    //

    UINT32 CrashStackSize;
    UINT32 CrashStackOffset;
    UINT64 CrashStackAddress;

    //
    // Platform specific data.
    //

    UINT32 PlatformStateSize;
    UINT32 PlatformStateOffset;

} HV_CRASHDUMP_PROCESSOR_STATE, *PHV_CRASHDUMP_PROCESSOR_STATE;

//
//  Size of code page to save during a crash.
//

#define CODE_CHUNK_SIZE     0x200

typedef struct _HV_CRASHDUMP_AREA_V2
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

} HV_CRASHDUMP_AREA_V2, *PHV_CRASHDUMP_AREA_V2;

typedef struct _HV_CRASHDUMP_AREA_V3
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

} HV_CRASHDUMP_AREA_V3, *PHV_CRASHDUMP_AREA_V3;

//
// External names used to manupulate registers
//

typedef enum _HV_REGISTER_NAME
{
    // Suspend Registers
    HvRegisterExplicitSuspend   = 0x00000000,
    HvRegisterInterceptSuspend  = 0x00000001,

    // Pending Interruption Register
    HvX64RegisterPendingInterruption    = 0x00010002,

    // Interrupt State register
    HvX64RegisterInterruptState         = 0x00010003,

    // User-Mode Registers
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

    // Floating Point and Vector Registers
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

    // Control Registers
    HvX64RegisterCr0                = 0x00040000,
    HvX64RegisterCr2                = 0x00040001,
    HvX64RegisterCr3                = 0x00040002,
    HvX64RegisterCr4                = 0x00040003,
    HvX64RegisterCr8                = 0x00040004,
    HvX64RegisterXfem               = 0x00040005,

    // Debug Registers
    HvX64RegisterDr0                = 0x00050000,
    HvX64RegisterDr1                = 0x00050001,
    HvX64RegisterDr2                = 0x00050002,
    HvX64RegisterDr3                = 0x00050003,
    HvX64RegisterDr6                = 0x00050004,
    HvX64RegisterDr7                = 0x00050005,

    // Segment Registers
    HvX64RegisterEs                 = 0x00060000,
    HvX64RegisterCs                 = 0x00060001,
    HvX64RegisterSs                 = 0x00060002,
    HvX64RegisterDs                 = 0x00060003,
    HvX64RegisterFs                 = 0x00060004,
    HvX64RegisterGs                 = 0x00060005,
    HvX64RegisterLdtr               = 0x00060006,
    HvX64RegisterTr                 = 0x00060007,

    // Table Registers
    HvX64RegisterIdtr               = 0x00070000,
    HvX64RegisterGdtr               = 0x00070001,

    // Virtualized MSRs
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
    // Cache control MSRs
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

    // Hypervisor-defined MSRs (Misc)
    HvX64RegisterVpRuntime           = 0x00090000,
    HvX64RegisterHypercall           = 0x00090001,
    HvX64RegisterGuestOsId           = 0x00090002,
    HvX64RegisterVpIndex             = 0x00090003,
    HvX64RegisterTimeRefCount        = 0x00090004,

    // Virtual APIC registers MSRs
    HvX64RegisterEoi                = 0x00090010,
    HvX64RegisterIcr                = 0x00090011,
    HvX64RegisterTpr                = 0x00090012,
    HvX64RegisterApicAssistPage     = 0x00090013,

    // Performance statistics MSRs
    HvX64RegisterStatsPartitionRetail  = 0x00090020,
    HvX64RegisterStatsPartitionInternal= 0x00090021,
    HvX64RegisterStatsVpRetail         = 0x00090022,
    HvX64RegisterStatsVpInternal       = 0x00090023,

    // Hypervisor-defined MSRs (Synic)
    HvX64RegisterSint0              = 0x000A0000,
    HvX64RegisterSint1              = 0x000A0001,
    HvX64RegisterSint2              = 0x000A0002,
    HvX64RegisterSint3              = 0x000A0003,
    HvX64RegisterSint4              = 0x000A0004,
    HvX64RegisterSint5              = 0x000A0005,
    HvX64RegisterSint6              = 0x000A0006,
    HvX64RegisterSint7              = 0x000A0007,
    HvX64RegisterSint8              = 0x000A0008,
    HvX64RegisterSint9              = 0x000A0009,
    HvX64RegisterSint10             = 0x000A000A,
    HvX64RegisterSint11             = 0x000A000B,
    HvX64RegisterSint12             = 0x000A000C,
    HvX64RegisterSint13             = 0x000A000D,
    HvX64RegisterSint14             = 0x000A000E,
    HvX64RegisterSint15             = 0x000A000F,
    HvX64RegisterScontrol           = 0x000A0010,
    HvX64RegisterSversion           = 0x000A0011,
    HvX64RegisterSifp               = 0x000A0012,
    HvX64RegisterSipp               = 0x000A0013,
    HvX64RegisterEom                = 0x000A0014,
    HvX64RegisterSirbp              = 0x000A0015,

    // Hypervisor-defined MSRs (Synthetic Timers)
    HvX64RegisterStimer0Config      = 0x000B0000,
    HvX64RegisterStimer0Count       = 0x000B0001,
    HvX64RegisterStimer1Config      = 0x000B0002,
    HvX64RegisterStimer1Count       = 0x000B0003,
    HvX64RegisterStimer2Config      = 0x000B0004,
    HvX64RegisterStimer2Count       = 0x000B0005,
    HvX64RegisterStimer3Config      = 0x000B0006,
    HvX64RegisterStimer3Count       = 0x000B0007,

    //
    // XSAVE/XRSTOR register names.
    //

    // XSAVE AFX extended state registers. YMM registers are 256-bit.
    // However, only 128-bit access is currently supported.
    // N.B. The lower 128-bits of YMM registers are overlyaid with
    // the cooresponding XMM register.
    HvX64RegisterYmm0Low             = 0x000C0000,
    HvX64RegisterYmm1Low             = 0x000C0001,
    HvX64RegisterYmm2Low             = 0x000C0002,
    HvX64RegisterYmm3Low             = 0x000C0003,
    HvX64RegisterYmm4Low             = 0x000C0004,
    HvX64RegisterYmm5Low             = 0x000C0005,
    HvX64RegisterYmm6Low             = 0x000C0006,
    HvX64RegisterYmm7Low             = 0x000C0007,
    HvX64RegisterYmm8Low             = 0x000C0008,
    HvX64RegisterYmm9Low             = 0x000C0009,
    HvX64RegisterYmm10Low            = 0x000C000A,
    HvX64RegisterYmm11Low            = 0x000C000B,
    HvX64RegisterYmm12Low            = 0x000C000C,
    HvX64RegisterYmm13Low            = 0x000C000D,
    HvX64RegisterYmm14Low            = 0x000C000E,
    HvX64RegisterYmm15Low            = 0x000C000F,
    HvX64RegisterYmm0High            = 0x000C0010,
    HvX64RegisterYmm1High            = 0x000C0011,
    HvX64RegisterYmm2High            = 0x000C0012,
    HvX64RegisterYmm3High            = 0x000C0013,
    HvX64RegisterYmm4High            = 0x000C0014,
    HvX64RegisterYmm5High            = 0x000C0015,
    HvX64RegisterYmm6High            = 0x000C0016,
    HvX64RegisterYmm7High            = 0x000C0017,
    HvX64RegisterYmm8High            = 0x000C0018,
    HvX64RegisterYmm9High            = 0x000C0019,
    HvX64RegisterYmm10High           = 0x000C001A,
    HvX64RegisterYmm11High           = 0x000C001B,
    HvX64RegisterYmm12High           = 0x000C001C,
    HvX64RegisterYmm13High           = 0x000C001D,
    HvX64RegisterYmm14High           = 0x000C001E,
    HvX64RegisterYmm15High           = 0x000C001F

} HV_REGISTER_NAME, *PHV_REGISTER_NAME;
typedef const HV_REGISTER_NAME *PCHV_REGISTER_NAME;

//
// Definiton of the HvCallGetVpRegister hypercall input structure.
// This call retrieves a Vp's register state.
//

typedef struct HV_CALL_ATTRIBUTES _HV_INPUT_GET_VP_REGISTERS
{
    HV_PARTITION_ID     PartitionId;
    HV_VP_INDEX         VpIndex;
    HV_CALL_ATTRIBUTES
    HV_REGISTER_NAME    Names[];
} HV_INPUT_GET_VP_REGISTERS, *PHV_INPUT_GET_VP_REGISTERS;

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
