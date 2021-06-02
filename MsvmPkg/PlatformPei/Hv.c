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

#include <Library/DebugLib.h>
#include <Library/CrashDumpAgentLib.h>

#define HV 0x4856 // "HV"

VOID
HvDetectIsolation(
    VOID
    )
/*++

Routine Description:

    Determines whether UEFI is running in an isolated VM.

Arguments:

    None.

Return Value:

    None.

--*/
{
#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)

    HV_CPUID_RESULT cpuidResult;
    EFI_STATUS status = EFI_SUCCESS;

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionVersionAndFeatures);
    if (!cpuidResult.VersionAndFeatures.HypervisorPresent)
    {
        DEBUG((DEBUG_INFO, __FUNCTION__" - Hypervisor is not present \n"));
        return;
    }

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionHvInterface);
    if (cpuidResult.HvInterface.Interface != HvMicrosoftHypervisorInterface)
    {
        DEBUG((DEBUG_INFO, __FUNCTION__" - Hypervisor interface is not present \n"));
        return;
    }

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionMsHvFeatures);
    if (!cpuidResult.MsHvFeatures.PartitionPrivileges.Isolation)
    {
        DEBUG((DEBUG_INFO, __FUNCTION__" - Isolation is not present \n"));
        return;
    }

    __cpuid(cpuidResult.AsUINT32, HvCpuidFunctionMsHvIsolationConfiguration);
    switch (cpuidResult.MsHvIsolationConfiguration.IsolationType)
    {
    case HV_PARTITION_ISOLATION_TYPE_VBS:
        status = PcdSet32S(PcdIsolationArchitecture, UefiIsolationTypeVbs);
        break;
    case HV_PARTITION_ISOLATION_TYPE_SNP:
        status = PcdSet32S(PcdIsolationArchitecture, UefiIsolationTypeSnp);
        break;
    case HV_PARTITION_ISOLATION_TYPE_NONE:
        return;
    default:
        ASSERT(FALSE);
        return;
    }
    if (EFI_ERROR(status))
    {
        DEBUG((DEBUG_ERROR, "Failed to set the PCD PcdIsolationArchitecture::0x%x \n", status));
        PEI_FAIL_FAST_IF_FAILED(status, CRITICAL_INITIALIZATION_FAILURE, HV);
    }
    
    if (cpuidResult.MsHvIsolationConfiguration.ParavisorPresent)
    {
        status = PcdSetBoolS(PcdIsolationParavisorPresent, TRUE);
        if (EFI_ERROR(status))
        {
            DEBUG((DEBUG_ERROR, "Failed to set the PCD PcdIsolationParavisorPresent::0x%x \n", status));
            PEI_FAIL_FAST_IF_FAILED(status, CRITICAL_INITIALIZATION_FAILURE, HV);
        }
    }
    if (cpuidResult.MsHvIsolationConfiguration.SharedGpaBoundaryActive)
    {
        status = PcdSet64S(PcdIsolationSharedGpaBoundary, 1UI64 << cpuidResult.MsHvIsolationConfiguration.SharedGpaBoundaryBits);
        if (EFI_ERROR(status))
        {
            DEBUG((DEBUG_ERROR, "Failed to set the PCD PcdIsolationSharedGpaBoundary::0x%x \n", status));
            PEI_FAIL_FAST_IF_FAILED(status, CRITICAL_INITIALIZATION_FAILURE, HV);
        }
    }

#endif
    return;
}
