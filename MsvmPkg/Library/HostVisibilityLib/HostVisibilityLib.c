/*++

Copyright (c) Microsoft Corporation

Module Name:

    HostVisibilityLib.c

Abstract:

    This file implements routines to update host visibility of memory.  These
    routines will perform the correct platform-specific sequences when
    hardware isolation is in effect with no paravisor present.

--*/

#include <EfiNt.h>
#include <Uefi/UefiBaseType.h>
#include <hvgdk_mini.h>
#include <IsolationTypes.h>

UINT64
_sev_pvalidate(
    _In_ PVOID Address,
    _In_ UINT32 PageSize,
    _In_ UINT32 Validate,
    _Out_ PUINT64 ErrorCode
    );

UINT64
SpecialGhcbCall(
    _In_ UINT64 GhcbValue
    );

typedef union _GHCB_MSR
{
    UINT64 AsUINT64;
    struct
    {
        UINT64 GhcbLow : 32;
        UINT64 GhcbHigh : 32;
    };

    struct
    {
        UINT64 GhcbInfo : 12;
        UINT64 GpaPageNumber : 40;
        UINT64 ExtraData : 12;
    };

} GHCB_MSR;

#define GHCB_INFO_PAGE_STATE_CHANGE     0x012
#define GHCB_INFO_PAGE_STATE_UPDATED    0x013

#define GHCB_DATA_PAGE_STATE_PRIVATE    0x001
#define GHCB_DATA_PAGE_STATE_SHARED     0x002


EFI_STATUS
EfiMakePageHostVisibleSnp(
    _In_ HV_GPA_PAGE_NUMBER PageNumber
    )
/*++

Routine Description:

    This routine makes a page visible to the host on an SNP platform that runs
    with no paravisor.

Arguments:

    PageNumber - Supplies the GPA page number of the page to make visible.

Return Value:

    EFI_STATUS.

--*/
{
    UINT64 errorCode;
    GHCB_MSR ghcbMsr;

    //
    // Ensure this page is no longer a valid private address.
    //

    if (_sev_pvalidate((PVOID)(PageNumber * EFI_PAGE_SIZE), 0, 0, &errorCode))
    {
        return EFI_SECURITY_VIOLATION;
    }

    if (errorCode != 0)
    {
        return EFI_SECURITY_VIOLATION;
    }

    //
    // Request a page conversion via the GHCB register protocol.
    //

    ghcbMsr.AsUINT64 = 0;
    ghcbMsr.GhcbInfo = GHCB_INFO_PAGE_STATE_CHANGE;
    ghcbMsr.GpaPageNumber = PageNumber;
    ghcbMsr.ExtraData = GHCB_DATA_PAGE_STATE_SHARED;

    ghcbMsr.AsUINT64 = SpecialGhcbCall(ghcbMsr.AsUINT64);

    if (ghcbMsr.AsUINT64 != GHCB_INFO_PAGE_STATE_UPDATED)
    {
        return EFI_SECURITY_VIOLATION;
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EfiMakePageHostVisible(
    _In_ UINT32 IsolationType,
    _In_ HV_GPA_PAGE_NUMBER PageNumber
    )
/*++

Routine Description:

    This routine makes a page visible to the host on a hardware isolated
    platform that runs with no paravisor.

Arguments:

    IsolationType - Supplies the isolation type of the current platform.

    PageNumber - Supplies the GPA page number of the page to make visible.

Return Value:

    EFI_STATUS.

--*/
{
    if (IsolationType == UefiIsolationTypeSnp)
    {
        return EfiMakePageHostVisibleSnp(PageNumber);
    }

    return EFI_INVALID_PARAMETER;
}


EFI_STATUS
EfiMakePageHostNotVisibleSnp(
    _In_ HV_GPA_PAGE_NUMBER PageNumber
    )
/*++

Routine Description:

    This routine makes a page private to the guest (not visible to the host)
    on an SNP platform that runs with no paravisor.

Arguments:

    IsolationType - Supplies the isolation type of the current platform.

    PageNumber - Supplies the GPA page number of the page to make private.

Return Value:

    EFI_STATUS.

--*/
{
    UINT64 errorCode;
    GHCB_MSR ghcbMsr;

    //
    // Request a page conversion via the GHCB register protocol.
    //

    ghcbMsr.AsUINT64 = 0;
    ghcbMsr.GhcbInfo = GHCB_INFO_PAGE_STATE_CHANGE;
    ghcbMsr.GpaPageNumber = PageNumber;
    ghcbMsr.ExtraData = GHCB_DATA_PAGE_STATE_SHARED;

    ghcbMsr.AsUINT64 = SpecialGhcbCall(ghcbMsr.AsUINT64);

    if (ghcbMsr.AsUINT64 != GHCB_INFO_PAGE_STATE_UPDATED)
    {
        return EFI_SECURITY_VIOLATION;
    }

    //
    // Validate this page to make it accessible again.
    //

    if (_sev_pvalidate((PVOID)(PageNumber * EFI_PAGE_SIZE), 0, 1, &errorCode))
    {
        return EFI_SECURITY_VIOLATION;
    }

    if (errorCode != 0)
    {
        return EFI_SECURITY_VIOLATION;
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EfiMakePageHostNotVisible(
    _In_ UINT32 IsolationType,
    _In_ HV_GPA_PAGE_NUMBER PageNumber
    )
/*++

Routine Description:

    This routine makes a page private to the guest (not visible to the host)
    on a hardware isolated platform that runs with no paravisor.

Arguments:

    IsolationType - Supplies the isolation type of the current platform.

    PageNumber - Supplies the GPA page number of the page to make private.

Return Value:

    EFI_STATUS.

--*/
{
    if (IsolationType == UefiIsolationTypeSnp)
    {
        return EfiMakePageHostNotVisibleSnp(PageNumber);
    }

    return EFI_INVALID_PARAMETER;
}
