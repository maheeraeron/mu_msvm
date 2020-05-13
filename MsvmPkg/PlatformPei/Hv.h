/*++

Copyright (c) Microsoft Corporation

Module Name:

    Hv.h

Abstract:

    Hypervisor interactions during PEI.

--*/

#pragma once

#include <EfiNt.h>
#include <hvhdk.h>
#include <hvgdk.h>

BOOLEAN
HvInitialize(
    VOID
    );

EFI_STATUS
HvConnectToHypervisor(
    _Inout_ PPLATFORM_INIT_CONTEXT Context
    );

VOID
HvDisconnectFromHypervisor(
    _Inout_ PPLATFORM_INIT_CONTEXT Context
    );

HV_STATUS
HvAcceptGpaPages(
    _Inout_ PPLATFORM_INIT_CONTEXT Context,
    _In_ HV_ACCEPT_MEMORY_TYPE MemoryType,
    _In_ HV_MAP_GPA_FLAGS HostVisibility,
    _In_ HV_GPA_PAGE_NUMBER GpaPageBase,
    _In_ UINT64 PageCount,
    _Out_ UINT64* PageCountProcessed
    );
