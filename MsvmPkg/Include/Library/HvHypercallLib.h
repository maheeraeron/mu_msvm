/*++

Copyright (c) Microsoft Corporation

Module Name:

    HvHypercallLib.h

Abstract:

    Low level hypercalls.

--*/

#pragma once

#include <EfiNt.h>
#include <hvhdk.h>
#include <hvgdk.h>

#if defined(MDE_CPU_AARCH64)

HV_HYPERCALL_OUTPUT
AsmHyperCall(
    _In_      HV_HYPERCALL_INPUT    InputControl,
    _In_opt_  UINT64                InputPhysicalAddress,
    _In_opt_  UINT64                OutputPhysicalAddress
    );

HV_STATUS
AsmGetVpRegister64(
     _In_  UINT32      RegisterIndex,
     _Out_ PUINT64     RegisterBuffer
    );

HV_STATUS
AsmSetVpRegister64(
     _In_  UINT32      RegisterIndex,
     _In_  UINT64      RegisterBuffer
    );

#endif

typedef struct _HV_HYPERCALL_CONTEXT
{
    BOOLEAN Connected;

#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)

    PVOID HypercallPage;

#endif
} HV_HYPERCALL_CONTEXT, *PHV_HYPERCALL_CONTEXT;

#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)

VOID
HvHypercallConnect(
    _In_ PVOID HypercallPage,
    _Out_ HV_HYPERCALL_CONTEXT *Context
    );

#elif defined(MDE_CPU_AARCH64)

VOID
HvHypercallConnect(
    _Out_ HV_HYPERCALL_CONTEXT *Context
    );

#endif

VOID
HvHypercallDisconnect(
    _Inout_ HV_HYPERCALL_CONTEXT *Context
    );

HV_STATUS
HvHypercallIssue(
    _In_ HV_HYPERCALL_CONTEXT *Context,
    _In_ HV_CALL_CODE CallCode,
    _In_ BOOLEAN Fast,
    _In_ UINT32 CountOfElements,
    _In_ UINT64 FirstRegister,
    _In_ UINT64 SecondRegister,
    _Out_opt_ UINT32 *ElementsProcessed
    );

UINT64
HvHypercallGetVpRegister64Self(
    _In_ HV_REGISTER_NAME RegisterName
    );

VOID
HvHypercallSetVpRegister64Self(
    _In_ HV_REGISTER_NAME RegisterName,
    _In_ UINT64 RegisterValue
    );
