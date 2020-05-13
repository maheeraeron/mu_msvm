/*++

Copyright (c) Microsoft Corporation

Module Name:

    Hv.c

Abstract:

    Hypervisor interactions during PEI.

--*/

#include <PiPei.h>
#include <Platform.h>
#include <Hv.h>
#include <IsolationTypes.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HvHypercallLib.h>
#include <Library/MemoryAllocationLib.h>


typedef struct _HV_PAGES
{
#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)

    UCHAR HypercallPage[EFI_PAGE_SIZE];

#endif

    UCHAR HypercallInputPage[EFI_PAGE_SIZE];
} HV_PAGES, *PHV_PAGES;

#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)

C_ASSERT(sizeof(HV_PAGES) <= MISC_PAGE_COUNT_FREE_RW * EFI_PAGE_SIZE);

#endif


static
BOOLEAN
HvpConfigureIsolation(
    VOID
    )
/*++

Routine Description:

    Determines whether UEFI is running in an isolated VM.

Arguments:

    None.

Return Value:

    Whether UEFI is running in an isolated VM.

--*/
{
#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)

    HV_CPUID_RESULT cpuidResult;

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionVersionAndFeatures);
    if (!cpuidResult.VersionAndFeatures.HypervisorPresent)
    {
        return FALSE;
    }

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionHvInterface);
    if (cpuidResult.HvInterface.Interface != HvMicrosoftHypervisorInterface)
    {
        return FALSE;
    }

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionMsHvFeatures);
    if (!cpuidResult.MsHvFeatures.PartitionPrivileges.Isolation)
    {
        return FALSE;
    }

    __cpuid(cpuidResult.AsUINT32, HvCpuidFunctionMsHvIsolationConfiguration);
    switch (cpuidResult.MsHvIsolationConfiguration.IsolationType)
    {
    case HV_PARTITION_ISOLATION_TYPE_VBS:
        PcdSet32(PcdIsolationArchitecture, UefiIsolationTypeVbs);
        break;
    case HV_PARTITION_ISOLATION_TYPE_SNP:
        PcdSet32(PcdIsolationArchitecture, UefiIsolationTypeSnp);
        break;
    case HV_PARTITION_ISOLATION_TYPE_NONE:
        return FALSE;
    default:
        ASSERT(FALSE);
        return FALSE;
    }
    
    if (cpuidResult.MsHvIsolationConfiguration.ParavisorPresent)
    {
        PcdSetBool(PcdIsolationParavisorPresent, TRUE);
    }
    if (cpuidResult.MsHvIsolationConfiguration.SharedGpaBoundaryActive)
    {
        PcdSet64(PcdIsolationSharedGpaBoundary, 1UI64 << cpuidResult.MsHvIsolationConfiguration.SharedGpaBoundaryBits);
    }

    return TRUE;

#else

    return FALSE;

#endif
}


BOOLEAN
HvInitialize(
    VOID
    )
/*++

Routine Description:

    Initializes the hypervisor interaction module.

Arguments:

    None.

Return Value:

    TRUE if a hypervisor connection is required, otherwise FALSE.

--*/
{
    return HvpConfigureIsolation();
}


EFI_STATUS
HvConnectToHypervisor(
    _Inout_ PPLATFORM_INIT_CONTEXT Context
    )
/*++

Routine Description:

    Initializes a connection to the hypervisor.

Arguments:

    Context - The platform init context.

Return Value:

    EFI status.

--*/
{
#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)

    Context->HvPages = (PHV_PAGES)(
        (UINTN)Context->StartOfConfigBlob - (MISC_PAGE_COUNT_TOTAL * EFI_PAGE_SIZE) +
        (MISC_PAGE_OFFSET_FREE_RW * EFI_PAGE_SIZE));

    HvHypercallConnect(Context->HvPages->HypercallPage, NULL, &Context->HvHypercallContext);

#else

    HvHypercallConnect(&Context->HvHypercallContext);

#endif

    return EFI_SUCCESS;
}


VOID
HvDisconnectFromHypervisor(
    _Inout_ PPLATFORM_INIT_CONTEXT Context
    )
/*++

Routine Description:

    Tears down a connection to the hypervisor.

Arguments:

    Context - The platform init context.

Return Value:

    None.

--*/
{
    HvHypercallDisconnect(&Context->HvHypercallContext);

#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)

    Context->HvPages = NULL;

#endif
}


HV_STATUS
HvAcceptGpaPages(
    _Inout_ PPLATFORM_INIT_CONTEXT Context,
    _In_ HV_ACCEPT_MEMORY_TYPE MemoryType,
    _In_ HV_MAP_GPA_FLAGS HostVisibility,
    _In_ HV_GPA_PAGE_NUMBER GpaPageBase,
    _In_ UINT64 PageCount,
    _Out_ UINT64* PageCountProcessed
    )
/*++

Routine Description:

    Accepts a range of GPA pages.

Arguments:

    MemoryType - The expected memory type.

    HostVisibility - The expected host visibility. Only HV_MAP_GPA_READABLE and
        HV_MAP_GPA_WRITABLE are valid in the access mask.

    GpaPageBase - Supplies the address of the first target GPA to accept. The
        remaining pages will be modified sequentially from this GPA.

    PageCount - The number of pages to modify.

    PageCountProcessed - Receives the number of pages successfully modified.

Return Value:

    HV_STATUS.

--*/
{
#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)

    HV_STATUS hvStatus;
    PHV_INPUT_ACCEPT_GPA_PAGES input;
    UINT64 pageCountRemaining;
    UINT32 pageCountPerCall;
    UINT32 pageCountProcessedInCall;

    DEBUG((DEBUG_VERBOSE, ">>> %a: MemoryType %u HostVisibility 0x%x GpaPageBase 0x%I64x PageCount %I64u\n", __FUNCTION__,
        MemoryType, HostVisibility, GpaPageBase, PageCount));

    *PageCountProcessed = 0;

    input = (PHV_INPUT_ACCEPT_GPA_PAGES)Context->HvPages->HypercallInputPage;
    ZeroMem(input, sizeof(*input));
    input->TargetPartitionId = HV_PARTITION_ID_SELF;
    input->MemoryType = MemoryType;
    input->HostVisibility = HostVisibility;
    input->GpaPageBase = GpaPageBase;

    pageCountRemaining = PageCount;
    pageCountPerCall = (UINT32)MIN(pageCountRemaining, (UINT64)HV_X64_MAX_HYPERCALL_ELEMENTS);

    while (pageCountRemaining != 0)
    {
        if (pageCountPerCall > pageCountRemaining)
        {
            pageCountPerCall = (UINT32)pageCountRemaining;
        }

        hvStatus = HvHypercallIssue(&Context->HvHypercallContext,
                                    HvCallAcceptGpaPages,
                                    FALSE,
                                    pageCountPerCall,
                                    (UINTN)input,
                                    0,
                                    &pageCountProcessedInCall);

        input->GpaPageBase += pageCountProcessedInCall;
        *PageCountProcessed += pageCountProcessedInCall;
        pageCountRemaining -= pageCountProcessedInCall;

        if (hvStatus != HV_STATUS_SUCCESS)
        {
            goto Exit;
        }
    }

    hvStatus = HV_STATUS_SUCCESS;

Exit:

    DEBUG((DEBUG_VERBOSE, "<<< %a: %r\n", __FUNCTION__, hvStatus));
    return hvStatus;

#else

    DEBUG((DEBUG_VERBOSE, ">>> %a: Not implemented\n", __FUNCTION__));

    return HV_STATUS_OPERATION_FAILED;

#endif
}
