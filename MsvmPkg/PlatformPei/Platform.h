/** @file
  Definitions relating to the Hyper-V "Platform" PEI Module.

  Copyright (c) Microsoft Corporation.
  Licensed under the BSD-2-Clause-Patent license.

**/

#pragma once

#include <Library/HvHypercallLib.h>

#define CRITICAL_INITIALIZATION_FAILURE 0x13D
#define KERNEL_SECURITY_CHECK_FAILURE 0x139
#define FAST_FAIL_UNEXPECTED_HOST_BEHAVIOR 58

VOID
TripleFault(
    UINTN   Rax,
    UINTN   Rbx,
    UINTN   Rcx,
    UINTN   Rdx
);

#if defined(MDE_CPU_AARCH64)
#define PEI_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(Info1) \
    { ASSERT(FALSE); CpuDeadLoop(); }
#define PEI_FAIL_FAST_IF_FAILED(Status, ErrorCode, Info1) \
    do \
    { \
        if (EFI_ERROR(Status)) \
        { \
            ASSERT(FALSE); \
            CpuDeadLoop(); \
        } \
    } while(0)
#elif defined(MDE_CPU_X64)
#define PEI_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(Info1) \
    do \
    { \
        DEBUG((DEBUG_ERROR, "Fatal error at %a (%d)\n", __FUNCTION__, __LINE__)); \
        TripleFault(KERNEL_SECURITY_CHECK_FAILURE, FAST_FAIL_UNEXPECTED_HOST_BEHAVIOR, __LINE__, Info1); \
    } while (0)

#define PEI_FAIL_FAST_IF_FAILED(Status, ErrorCode, Info1) \
    do \
    { \
        UINTN _LocalStatus_; \
        _LocalStatus_ = Status; \
        if (EFI_ERROR(_LocalStatus_)) \
        { \
            DEBUG((DEBUG_ERROR, "Fatal error at %a (%d)\n", __FUNCTION__, __LINE__)); \
            TripleFault(ErrorCode, _LocalStatus_, __LINE__, Info1); \
        } \
    } while(0)
#else
#error Unsupported Architecture
#endif

#if defined(MDE_CPU_X64)

//
// On X64, the config blob starts after the end of the firmware, and after
// the 6 pages for pagetables, 1 page for GDT entries, and 2 free RW pages.
//

#define MISC_PAGE_COUNT_PAGE_TABLES 6
#define MISC_PAGE_COUNT_GDT_ENTRIES 1
#define MISC_PAGE_COUNT_FREE_RW     2

#define MISC_PAGE_COUNT_TOTAL ( \
    MISC_PAGE_COUNT_PAGE_TABLES + \
    MISC_PAGE_COUNT_GDT_ENTRIES + \
    MISC_PAGE_COUNT_FREE_RW)

#define MISC_PAGE_OFFSET_FREE_RW ( \
    MISC_PAGE_COUNT_PAGE_TABLES + \
    MISC_PAGE_COUNT_GDT_ENTRIES)

#endif

typedef struct _PLATFORM_INIT_CONTEXT
{
    struct _UEFI_CONFIG_HEADER *StartOfConfigBlob;
    HV_HYPERCALL_CONTEXT HvHypercallContext;
    UINT8 PhysicalAddressWidth;

#if defined (MDE_CPU_X64)

    struct _HV_PAGES *HvPages;

#endif
} PLATFORM_INIT_CONTEXT, *PPLATFORM_INIT_CONTEXT;
