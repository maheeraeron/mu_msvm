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

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionVersionAndFeatures);
    if (!cpuidResult.VersionAndFeatures.HypervisorPresent)
    {
        return;
    }

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionHvInterface);
    if (cpuidResult.HvInterface.Interface != HvMicrosoftHypervisorInterface)
    {
        return;
    }

    __cpuid(cpuidResult.AsUINT32, HvCpuIdFunctionMsHvFeatures);
    if (!cpuidResult.MsHvFeatures.PartitionPrivileges.Isolation)
    {
        return;
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
        return;
    default:
        ASSERT(FALSE);
        return;
    }
    
    if (cpuidResult.MsHvIsolationConfiguration.ParavisorPresent)
    {
        PcdSetBool(PcdIsolationParavisorPresent, TRUE);
    }
    if (cpuidResult.MsHvIsolationConfiguration.SharedGpaBoundaryActive)
    {
        PcdSet64(PcdIsolationSharedGpaBoundary, 1UI64 << cpuidResult.MsHvIsolationConfiguration.SharedGpaBoundaryBits);
    }

#endif
}
