/*++

Copyright (c) Microsoft Corporation

Module Name:

    HvHypercallLibP.h

Abstract:

    Private include for the hypercall support routine library.

--*/

#pragma once

#include <Base.h>

#include <Library/HvHypercallLib.h>

#if defined(MDE_CPU_X64)

VOID
HvHypercallpSetMsrWithGhcb(
    _In_ HV_HYPERCALL_CONTEXT *Context,
    _In_ UINT64 MsrNumber,
    _In_ UINT64 RegisterValue
    );

HV_STATUS
HvHypercallpIssueGhcbHypercall(
    _In_ HV_HYPERCALL_CONTEXT *Context,
    _In_ HV_CALL_CODE CallCode,
    _In_opt_ VOID *InputPage,
    _In_ UINT32 CountOfElements,
    _Out_opt_ PUINT32 ElementsProcessed
    );

VOID
_tdx_vmcall_wrmsr(
    _In_ UINT32 MsrIndex,
    _In_ UINT64 MsrValue
    );

UINT64
_tdx_vmcall_rdmsr(
    _In_ UINT32 MsrIndex
    );

/// Functions that enable and disable interrupts, that are implemented based
/// on the environment the library is built for.

VOID
HvHypercallpDisableInterrupts(
    VOID
    );

VOID
HvHypercallpEnableInterrupts(
    VOID
    );

#endif
