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
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
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

#define SNP_SUCCESS             0
#define SNP_FAIL_INPUT          1
#define SNP_FAIL_SIZEMISMATCH   6

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

#define GHCB_INFO_PAGE_STATE_CHANGE     0x014
#define GHCB_INFO_PAGE_STATE_UPDATED    0x015

#define GHCB_DATA_PAGE_STATE_PRIVATE    0x001
#define GHCB_DATA_PAGE_STATE_SHARED     0x002


#define TDX_SUCCESS                 0
#define TDX_PAGE_SIZE_INVALID       0x8000000000000002ULL


UINT64
_tdx_tdg_mem_page_accept(
    _In_ HV_GPA Gpa,
    _In_ UINT32 PageSize
    );

UINT64
_tdx_vmcall_map_gpa(
    _In_ UINT64 Gpa,
    _In_ UINT64 Size,
    _Out_opt_ PUINT64 FailedGpa
    );


VOID
EfiUpdatePageRangeAcceptanceSnp(
    _In_ HV_GPA_PAGE_NUMBER StartingPageNumber,
    _In_ UINT64 PageCount,
    _In_ BOOLEAN Accept
    )
/*++

Routine Description:

    This routine updates hardware page acceptance state on an SNP platform
    that runs with no paravisor.

Arguments:

    StartingPageNumber - Supplies the starting GPA page number of the range to
                         change.

    PageCount - Supplies the number of pages to change.

    Accept - Supplies TRUE if the pages are to be accepted, or FALSE if the
             pages are to be unaccepted.

Return Value:

    None.

--*/
{
    UINT64 errorCode;

    while (PageCount != 0)
    {
        //
        // Attempt to validate a 2 MB page if possible.
        //

        if (((StartingPageNumber & (SIZE_2MB - 1)) == 0) &&
            (PageCount >= SIZE_2MB))
        {
            if (_sev_pvalidate(
                (PVOID)(StartingPageNumber * EFI_PAGE_SIZE),
                1,
                Accept,
                &errorCode) != 0)
            {
                errorCode = SNP_FAIL_INPUT;
            }

            if (errorCode == SNP_SUCCESS)
            {
                StartingPageNumber += SIZE_2MB / EFI_PAGE_SIZE;
                PageCount -= SIZE_2MB / EFI_PAGE_SIZE;
                continue;
            }
            else if (errorCode != SNP_FAIL_SIZEMISMATCH)
            {
                //
                // TODO-19259739: Have a better way of reporting UEFI errors.
                //
                continue;
            }
        }

        if (_sev_pvalidate(
            (PVOID)(StartingPageNumber * EFI_PAGE_SIZE),
            0,
            Accept,
            &errorCode) != 0)
        {
            errorCode = SNP_FAIL_INPUT;
        }

        if (errorCode != SNP_SUCCESS)
        {
            //
            // TODO-19259739: Have a better way of reporting UEFI errors.
            //
            continue;
        }

        StartingPageNumber += 1;
        PageCount -= 1;
    }
}


VOID
EfiUpdatePageRangeAcceptanceTdx(
    _In_ HV_GPA_PAGE_NUMBER StartingPageNumber,
    _In_ UINT64 PageCount
    )
/*++

Routine Description:

    This routine updates hardware page acceptance state on a TDX platform that
    runs with no paravisor.

Arguments:

    StartingPageNumber - Supplies the starting GPA page number of the range to
                         change.

    PageCount - Supplies the number of pages to change.

Return Value:

    None.

--*/
{
    UINT64 errorCode;

    while (PageCount != 0)
    {
        //
        // Attempt to validate a 2 MB page if possible.
        //

        if (((StartingPageNumber & (SIZE_2MB - 1)) == 0) &&
            (PageCount >= SIZE_2MB))
        {
            errorCode = _tdx_tdg_mem_page_accept(
                StartingPageNumber * HV_PAGE_SIZE,
                1);
            if (errorCode == TDX_SUCCESS)
            {
                StartingPageNumber += SIZE_2MB / HV_PAGE_SIZE;
                PageCount -= SIZE_2MB / HV_PAGE_SIZE;
                continue;
            }
            else if (errorCode != TDX_PAGE_SIZE_INVALID)
            {
                //
                // TODO-19259739: Have a better way of reporting UEFI errors.
                //
                continue;
            }
        }

        errorCode = _tdx_tdg_mem_page_accept(
            StartingPageNumber * HV_PAGE_SIZE,
            0);
        if (errorCode != TDX_SUCCESS)
        {
            //
            // TODO-19259739: Have a better way of reporting UEFI errors.
            //
            continue;
        }

        StartingPageNumber += 1;
        PageCount -= 1;
    }
}


VOID
EfiUpdatePageRangeAcceptance(
    _In_ UINT32 IsolationType,
    _In_ HV_GPA_PAGE_NUMBER StartingPageNumber,
    _In_ UINT64 PageCount,
    _In_ BOOLEAN Accept
    )
/*++

Routine Description:

    This routine updates hardware page acceptance state on an SNP platform
    that runs with no paravisor.

Arguments:

    IsolationType - Supplies the isolation type of the current platform.

    StartingPageNumber - Supplies the starting GPA page number of the range to
                         change.

    PageCount - Supplies the number of pages to change.

    Accept - Supplies TRUE if the pages are to be accepted, or FALSE if the
             pages are to be unaccepted.

Return Value:

    None.

--*/
{
    if (IsolationType == UefiIsolationTypeTdx)
    {
        //
        // No action is required when acceptance is being revoked.
        //

        if (Accept)
        {
            EfiUpdatePageRangeAcceptanceTdx(StartingPageNumber, PageCount);
        }
    }
    else
    {
        ASSERT(IsolationType == UefiIsolationTypeSnp);
        EfiUpdatePageRangeAcceptanceSnp(StartingPageNumber, PageCount, Accept);
    }
}


EFI_STATUS
EfiMakePageRangeHostVisibleSnp(
    _In_ HV_GPA_PAGE_NUMBER StartingPageNumber,
    _In_ UINT64 PageCount,
    _Out_opt_ PUINT64 PagesProcessed
    )
/*++

Routine Description:

    This routine makes a range of pages visible to the host on an SNP platform
    that runs with no paravisor.

Arguments:

    StartingPageNumber - Supplies the starting GPA page number of the range to
                         make visible.

    PageCount - Supplies the number of pages to make visible.

    PagesProcessed - Supplies an optional pointer to a page that should
                     receive the number of pages that were successfully
                     processed.

Return Value:

    EFI_STATUS.

--*/
{
    UINT64 errorCode;
    GHCB_MSR ghcbMsr;

    if (ARGUMENT_PRESENT(PagesProcessed))
    {
        *PagesProcessed = 0;
    }

    while (PageCount != 0)
    {
        //
        // Ensure this page is no longer a valid private address.
        //

        if (_sev_pvalidate((PVOID)(
            StartingPageNumber * EFI_PAGE_SIZE),
            0,
            0,
            &errorCode))
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
        ghcbMsr.GpaPageNumber = StartingPageNumber;
        ghcbMsr.ExtraData = GHCB_DATA_PAGE_STATE_SHARED;

        ghcbMsr.AsUINT64 = SpecialGhcbCall(ghcbMsr.AsUINT64);

        if (ghcbMsr.AsUINT64 != GHCB_INFO_PAGE_STATE_UPDATED)
        {
            //
            // Restore this page to an accepted state since the visibility was not
            // modified.
            //

            while ((_sev_pvalidate((PVOID)(StartingPageNumber * EFI_PAGE_SIZE), 0, TRUE, &errorCode)) ||
                   (errorCode != 0))
            {
                //
                // TODO-19259739: Have a better way of reporting UEFI errors.
                //
                ;
            }

            return EFI_SECURITY_VIOLATION;
        }

        if (ARGUMENT_PRESENT(PagesProcessed))
        {
            *PagesProcessed += 1;
        }

        StartingPageNumber += 1;
        PageCount -= 1;
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EfiChangePageRangeHostVisibilityTdx(
    _In_ HV_GPA SharedBoundaryGpa,
    _In_ HV_GPA StartingGpa,
    _In_ UINT64 PageCount,
    _Out_opt_ PUINT64 PagesProcessed
    )
/*++

Routine Description:

    This routine changes the host visibility of a range of pages on a TDX
    platform.

Arguments:

    SharedBoundaryGpa - Supplies the shared boundary GPA for the current
                        platform.

    StartingGpa - Supplies the starting GPA of the range to make visible.

    PageCount - Supplies the number of pages to make visible.

    PagesProcessed - Supplies an optional pointer to a page that should
                     receive the number of pages that were successfully
                     processed.

Return Value:

    EFI_STATUS.

--*/
{
    HV_GPA failedGpa;
    UINT64 pagesProcessed;
    EFI_STATUS status;

    //
    // Request a page conversion via the MapPage GHCI call.
    //

    if (_tdx_vmcall_map_gpa(
        StartingGpa,
        PageCount * HV_PAGE_SIZE,
        &failedGpa) != 0)
    {
        //
        // If the count of pages processed is not reasonable, then proceed as
        // if the call failed entirely.
        //

        if (ARGUMENT_PRESENT(PagesProcessed))
        {
            pagesProcessed = (failedGpa - StartingGpa) / HV_PAGE_SIZE;
            if (pagesProcessed >= PageCount)
            {
                pagesProcessed = 0;
            }
        }

        status = EFI_SECURITY_VIOLATION;
    }
    else
    {
        status = EFI_SUCCESS;
        pagesProcessed = PageCount;
    }

    //
    // If pages are being made private, then reaccept any pages that were
    // successfully processed.
    //

    if ((pagesProcessed != 0) && (StartingGpa < SharedBoundaryGpa))
    {
        EfiUpdatePageRangeAcceptanceTdx(
            StartingGpa / HV_PAGE_SIZE,
            pagesProcessed);
    }

    if (ARGUMENT_PRESENT(PagesProcessed))
    {
        *PagesProcessed = PageCount;
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EfiMakePageRangeHostVisible(
    _In_ UINT32 IsolationType,
    _In_ UINT64 SharedBoundaryGpa,
    _In_ HV_GPA_PAGE_NUMBER StartingPageNumber,
    _In_ UINT64 PageCount,
    _Out_opt_ PUINT64 PagesProcessed
    )
/*++

Routine Description:

    This routine makes a page visible to the host on a hardware isolated
    platform that runs with no paravisor.

Arguments:

    IsolationType - Supplies the isolation type of the current platform.

    SharedBoundaryGpa - Supplies the shared boundary GPA for the current
                        platform.

    StartingPageNumber - Supplies the starting GPA page number of the range to
                         make visible.

    PageCount - Supplies the number of pages to make visible.

    PagesProcessed - Supplies an optional pointer to a page that should
                     receive the number of pages that were successfully
                     processed.

Return Value:

    EFI_STATUS.

--*/
{
    switch (IsolationType)
    {
    case UefiIsolationTypeSnp:
        return EfiMakePageRangeHostVisibleSnp(
            StartingPageNumber,
            PageCount,
            PagesProcessed);

    case UefiIsolationTypeTdx:
        return EfiChangePageRangeHostVisibilityTdx(
            SharedBoundaryGpa,
            SharedBoundaryGpa + (StartingPageNumber * HV_PAGE_SIZE),
            PageCount,
            PagesProcessed);
    }

    return EFI_INVALID_PARAMETER;
}


EFI_STATUS
EfiMakePageRangeHostNotVisibleSnp(
    _In_ HV_GPA_PAGE_NUMBER StartingPageNumber,
    _In_ UINT64 PageCount,
    _Out_opt_ PUINT64 PagesProcessed
    )
/*++

Routine Description:

    This routine makes a range of pages private to the guest (not visible to
    the host) on an SNP platform that runs with no paravisor.

Arguments:

    StartingPageNumber - Supplies the starting GPA page number of the range to
                         make private.

    PageCount - Supplies the number of pages to make private.

    PagesProcessed - Supplies an optional pointer to a page that should
                     receive the number of pages that were successfully
                     processed.

Return Value:

    EFI_STATUS.

--*/
{
    UINT64 errorCode;
    GHCB_MSR ghcbMsr;

    if (ARGUMENT_PRESENT(PagesProcessed))
    {
        *PagesProcessed = 0;
    }

    while (PageCount != 0)
    {
        //
        // Request a page conversion via the GHCB register protocol.
        //

        ghcbMsr.AsUINT64 = 0;
        ghcbMsr.GhcbInfo = GHCB_INFO_PAGE_STATE_CHANGE;
        ghcbMsr.GpaPageNumber = StartingPageNumber;
        ghcbMsr.ExtraData = GHCB_DATA_PAGE_STATE_SHARED;

        ghcbMsr.AsUINT64 = SpecialGhcbCall(ghcbMsr.AsUINT64);

        if (ghcbMsr.AsUINT64 != GHCB_INFO_PAGE_STATE_UPDATED)
        {
            return EFI_SECURITY_VIOLATION;
        }

        //
        // Validate this page to make it accessible again.
        //

        if (_sev_pvalidate((PVOID)(
            StartingPageNumber * EFI_PAGE_SIZE),
            0,
            1,
            &errorCode))
        {
            return EFI_SECURITY_VIOLATION;
        }

        if (errorCode != 0)
        {
            return EFI_SECURITY_VIOLATION;
        }

        if (ARGUMENT_PRESENT(PagesProcessed))
        {
            *PagesProcessed += 1;
        }

        StartingPageNumber += 1;
        PageCount -= 1;
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EfiMakePageRangeHostNotVisible(
    _In_ UINT32 IsolationType,
    _In_ UINT64 SharedBoundaryGpa,
    _In_ HV_GPA_PAGE_NUMBER StartingPageNumber,
    _In_ UINT64 PageCount,
    _Out_opt_ PUINT64 PagesProcessed
    )
/*++

Routine Description:

    This routine makes a page private to the guest (not visible to the host)
    on a hardware isolated platform that runs with no paravisor.

Arguments:

Arguments:

    IsolationType - Supplies the isolation type of the current platform.

    SharedBoundaryGpa - Supplies the shared boundary GPA for the current
                        platform.

    StartingPageNumber - Supplies the starting GPA page number of the range to
                         make not visible.

    PageCount - Supplies the number of pages to make not visible.

    PagesProcessed - Supplies an optional pointer to a page that should
                     receive the number of pages that were successfully
                     processed.

Return Value:

    EFI_STATUS.

--*/
{
    switch (IsolationType)
    {
    case UefiIsolationTypeSnp:
        return EfiMakePageRangeHostNotVisibleSnp(
            StartingPageNumber,
            PageCount,
            PagesProcessed);

    case UefiIsolationTypeTdx:
        return EfiChangePageRangeHostVisibilityTdx(
            SharedBoundaryGpa,
            StartingPageNumber * HV_PAGE_SIZE,
            PageCount,
            PagesProcessed);
    }

    return EFI_INVALID_PARAMETER;
}
