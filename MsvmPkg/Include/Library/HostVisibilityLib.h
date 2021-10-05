/*++

Copyright (c) Microsoft Corporation

Module Name:

    HostVisibilityLib.h

Abstract:

    Definitions for functionality provided by host visibility change package.
    These routines will perform the correct platform-specific sequences when
    hardware isolation is in effect with no paravisor present.

Author:

    Jon Lange (jlange) 14-Sep-2020

--*/

#pragma once

VOID
EfiUpdatePageRangeAcceptance(
    _In_ UINT32 IsolationType,
    _In_ HV_GPA_PAGE_NUMBER StartingPageNumber,
    _In_ UINT64 PageCount,
    _In_ BOOLEAN Accept
    );

EFI_STATUS
EfiMakePageRangeHostVisible(
    _In_ UINT32 IsolationType,
    _In_ HV_GPA SharedBoundaryGpa,
    _In_ HV_GPA_PAGE_NUMBER StartingPageNumber,
    _In_ UINT64 PageCount,
    _Out_opt_ PUINT64 PagesProcessed
    );

EFI_STATUS
EfiMakePageRangeHostNotVisible(
    _In_ UINT32 IsolationType,
    _In_ HV_GPA SharedBoundaryGpa,
    _In_ HV_GPA_PAGE_NUMBER PageNumber,
    _In_ UINT64 PageCount,
    _Out_opt_ PUINT64 PagesProcessed
    );
