/*++

Copyright (c) Microsoft Corporation

Module Name:

    SecIso.c

Abstract:

    Routines to support hardware isolation of the SEC driver.

--*/

#include <Library/UefiCpuLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <EfiNt.h>
#include <hvgdk_mini.h>
#include <BiosInterface.h>
#include "SecP.h"

#define SetGhcbField16(Ghcb, Field, Value) \
    (*(PUINT16)((PUCHAR)(Ghcb) + (Field)) = (Value))
#define SetGhcbField32(Ghcb, Field, Value) \
    (*(PUINT32)((PUCHAR)(Ghcb) + (Field)) = (Value))
#define SetGhcbField64(Ghcb, Field, Value) \
    (*(PUINT64)((PUCHAR)(Ghcb) + (Field)) = (Value))
#define GetGhcbField64(Ghcb, Field) \
    (*(PUINT64)((PUCHAR)(Ghcb) + (Field)))

#define GHCB_INFO_MAP_GHCB              6

#define GHCB_EXITCODE_MSR               0x7C

#define GHCB_FIELD64_RAX                0x1F8
#define GHCB_FIELD64_RBX                0x318
#define GHCB_FIELD64_RCX                0x308
#define GHCB_FIELD64_RDX                0x310
#define GHCB_FIELD64_EXITCODE           0x390
#define GHCB_FIELD64_EXITINFO1          0x398
#define GHCB_FIELD16_VERSION            0xFFA
#define GHCB_FIELD32_FORMAT             0xFFC

PVOID Ghcb;
UINT64 TscMultiplier;
UINT64 TscDivisor;
HV_PSP_CPUID_PAGE *CpuidPage;
SEC_CPUID_INFO CpuidInfo;
SEC_CPUID_INFO ExtendedCpuidInfo;

UINT64
SecReadMsrWithGhcb(
    _In_ UINT64 MsrNumber
    )
{
    UINT64 msrValue;

    //
    // Initialize the GHCB page to indicate a request to set the specified
    // MSR.
    //

    SetGhcbField64(Ghcb, GHCB_FIELD64_EXITCODE, GHCB_EXITCODE_MSR);
    SetGhcbField64(Ghcb, GHCB_FIELD64_EXITINFO1, 0);
    SetGhcbField64(Ghcb, GHCB_FIELD64_RCX, MsrNumber);
    SetGhcbField32(Ghcb, GHCB_FIELD32_FORMAT, 0);
    SetGhcbField16(Ghcb, GHCB_FIELD16_VERSION, 1);

    SecVmgexit();

    msrValue = (UINT32)GetGhcbField64(Ghcb, GHCB_FIELD64_RAX);
    msrValue |= GetGhcbField64(Ghcb, GHCB_FIELD64_RDX) << 32;

    return msrValue;
}

VOID
SecInitializeReferenceTime (
    _In_ UINT32 ClockFrequency,
    _In_ UINT32 TscNumerator,
    _In_ UINT32 TscDenominator
    )
{
    //
    // The TSC frequency is (clock * numerator) / (denominator).
    // From a given TSC value, the reference time in 100ns units will be
    // (TSC / TscFrequency) * (100ns-frequency).  This is equivalent to
    // TSC * (denominator * 100ns-frequency) / (clock * numerator).  Since all
    // of these components are 32-bit values, they can be multiplied in pairs
    // to produce a 64-bit multiplier and divisor for a 64-bit MulDiv to
    // calculate reference time from TSC.
    //

    TscMultiplier = (UINT64)TscDenominator * 10000000;
    TscDivisor = ClockFrequency * (UINT64)TscNumerator;
    if (TscDivisor == 0)
    {
        TscDivisor = 1;
    }
}

BOOLEAN
SecInitializeSnp (
    UEFI_IGVM_PARAMETER_INFO *ParameterInfo
    )
{
    UINT32 clockFrequency;
    SEC_CPUID_INFO *cpuidInfo;
    UINT64 ghcbAddress;
    UINT64 ghcbMsr;
    UINT32 index;
    UINT32 leafNumber;
    UINT32 leafType;
    UINT64 sharedGpaBoundary;

    //
    // Select a GHCB address as the first page before the parameter info block.
    //

    if (mIsolationConfiguration.SharedGpaBoundaryActive)
    {
        sharedGpaBoundary = (1UI64 << mIsolationConfiguration.SharedGpaBoundaryBits);
    }
    else
    {
        sharedGpaBoundary = 0;
    }

    ghcbAddress = (UINTN)ParameterInfo - EFI_PAGE_SIZE + sharedGpaBoundary;

    //
    // Check to see whether the hypervisor has configured a GHCB.  If not,
    // configure a GHCB page at the selected address.
    //

    ghcbMsr = AsmReadMsr64(MSR_GHCB);
    if (ghcbMsr == 0)
    {
        AsmWriteMsr64(MSR_GHCB, ghcbAddress | GHCB_INFO_MAP_GHCB);
        SecVmgexit();
        ghcbMsr = AsmReadMsr64(MSR_GHCB);
    }
    
    //
    // The selected GHCB is usable as long as it is either the selected
    // address or it is beyond the parameter block and above the shared GPA
    // boundary.
    //

    if ((ghcbMsr & 0xFFF) != 0)
    {
        return FALSE;
    }

    if (ghcbMsr != ghcbAddress)
    {
        ghcbAddress = (UINTN)ParameterInfo +
                      (ParameterInfo->ParameterPageCount * EFI_PAGE_SIZE) +
                      sharedGpaBoundary;

        if (ghcbMsr < ghcbAddress)
        {
            return FALSE;
        }

        ghcbAddress = ghcbMsr;
    }

    Ghcb = (PVOID)ghcbAddress;

    //
    // Capture the location of CPUID information.
    //

    CpuidPage = (HV_PSP_CPUID_PAGE *)((UINTN)ParameterInfo + ParameterInfo->CpuidPagesOffset * EFI_PAGE_SIZE);

    //
    // Capture the set of CPUID information that is present.
    //

    CpuidInfo.SupportedLeaves |= 1;
    ExtendedCpuidInfo.SupportedLeaves |= 1;

    for (index = 0; index < HV_PSP_CPUID_LEAF_COUNT_MAX; index += 1)
    {
        leafNumber = CpuidPage->CpuidLeafInfo[index].EaxIn & 0x0FFFFFFF;
        leafType = (CpuidPage->CpuidLeafInfo[index].EaxIn >> 28);
        if (leafType == 0)
        {
            cpuidInfo = &CpuidInfo;
        }
        else if (leafType == 8)
        {
            cpuidInfo = &ExtendedCpuidInfo;
        }
        else
        {
            cpuidInfo = NULL;
        }

        if ((cpuidInfo != NULL) && (leafNumber < 0x40))
        {
            if (leafNumber > cpuidInfo->MaximumLeafIndex)
            {
                cpuidInfo->MaximumLeafIndex = leafNumber;
            }
            cpuidInfo->SupportedLeaves |= (1UI64 << leafNumber);
        }
    }

    //
    // Capture the TSC frequency for scaling.
    //

    clockFrequency = (UINT32)SecReadMsrWithGhcb(HV_X64_MSR_TSC_FREQUENCY);

    SecInitializeReferenceTime(clockFrequency, 1, 1);

    return TRUE;
}

BOOLEAN
SecProcessVirtualMsrRead (
    _In_ PTRAP_FRAME TrapFrame
    )
{
    UINT64 referenceTime;

    //
    // Only handle requests to obtain reference time.
    //

    if (TrapFrame->Rcx != HV_X64_MSR_TIME_REF_COUNT)
    {
        return FALSE;
    }

    referenceTime = MulDiv64(AsmReadTsc(), TscMultiplier, TscDivisor);
    TrapFrame->Rax = (UINT32)referenceTime;
    TrapFrame->Rdx = referenceTime >> 32;

    return TRUE;
}

BOOLEAN
SecProcessVirtualCpuid (
    _In_ PTRAP_FRAME TrapFrame
    )
{
    SEC_CPUID_INFO *cpuidInfo;
    HV_CPUID_RESULT cpuidResult;
    UINT32 index;
    UINT32 leaf;
    UINT32 leafNumber;
    BOOLEAN matchEcx;

    ZeroMem(&cpuidResult, sizeof(HV_CPUID_RESULT));

    //
    // Only support architectural and hypervisor CPUID leaves.
    //

    matchEcx = FALSE;
    leaf = (UINT32)TrapFrame->Rax;
    switch (leaf >> 28)
    {
    case 0:

        //
        // Determine whether this CPUID leaf has any sub-leaves.
        //

        if ((0x890 & (1 << TrapFrame->Rax)) != 0)
        {
            matchEcx = TRUE;
        }

        cpuidInfo = &CpuidInfo;
        break;

    case 4:
        cpuidInfo = NULL;
        break;

    case 8:
        cpuidInfo = &ExtendedCpuidInfo;
        break;

    default:
        return FALSE;
    }

    if (cpuidInfo != NULL)
    {
        //
        // See if the requested leaf can be found in the table.  If not, then fail.
        //

        leafNumber = leaf & 0x0FFFFFFF;
        if ((cpuidInfo == NULL) ||
            (leafNumber > cpuidInfo->MaximumLeafIndex) ||
            ((cpuidInfo->SupportedLeaves & (1UI64 << leafNumber)) == 0))
        {
            return FALSE;
        }

        for (index = 0; index < HV_PSP_CPUID_LEAF_COUNT_MAX; index += 1)
        {
            if (leaf != CpuidPage->CpuidLeafInfo[index].EaxIn)
            {
                continue;
            }

            if (!matchEcx || (TrapFrame->Rcx == CpuidPage->CpuidLeafInfo[index].EcxIn))
            {
                cpuidResult.Eax = CpuidPage->CpuidLeafInfo[index].EaxOut;
                cpuidResult.Ebx = CpuidPage->CpuidLeafInfo[index].EbxOut;
                cpuidResult.Ecx = CpuidPage->CpuidLeafInfo[index].EcxOut;
                cpuidResult.Edx = CpuidPage->CpuidLeafInfo[index].EdxOut;
                break;
            }
        }
    }

    //
    // Customize output as required, including for hypervisor leaves.
    //

    switch (leaf)
    {
    case 0:
    case 0x80000000:

        //
        // These leaves are not normally present in the table, so the value
        // must be calculated here.
        //

        cpuidResult.Eax = cpuidInfo->MaximumLeafIndex | (leaf & 0x80000000);
        cpuidResult.Ebx = 'htuA';
        cpuidResult.Edx = 'itne';
        cpuidResult.Ecx = 'DMAc';
        break;

    case 1:

        //
        // Indicate the presence of a hypervisor.
        //

        cpuidResult.Ecx |= 0x80000000;
        break;

    case HvCpuIdFunctionHvVendorAndMaxFunction:
        cpuidResult.HvVendorAndMaxFunction.MaxFunction = HvCpuidFunctionMsHvIsolationConfiguration;
        CopyMem(cpuidResult.HvVendorAndMaxFunction.VendorName,
                "Microsoft Hv",
                sizeof(cpuidResult.HvVendorAndMaxFunction.VendorName));
        break;

    case HvCpuIdFunctionHvInterface:
        cpuidResult.HvInterface.Interface = '1#vH';
        break;

    case HvCpuIdFunctionMsHvFeatures:
        cpuidResult.MsHvFeatures.PartitionPrivileges.Isolation = 1;
        cpuidResult.MsHvFeatures.PartitionPrivileges.AccessPartitionReferenceCounter = 1;
        cpuidResult.MsHvFeatures.PartitionPrivileges.AccessSynicRegs = 1;
        cpuidResult.MsHvFeatures.PartitionPrivileges.AccessSyntheticTimerRegs = 1;
        cpuidResult.MsHvFeatures.PartitionPrivileges.AccessIntrCtrlRegs = 1;
        cpuidResult.MsHvFeatures.PartitionPrivileges.AccessHypercallMsrs = 1;
        cpuidResult.MsHvFeatures.PartitionPrivileges.AccessVpIndex = 1;
        cpuidResult.MsHvFeatures.DirectSyntheticTimers = 1;
        cpuidResult.MsHvFeatures.DebugRegsAvailable = 1;
        break;

    case HvCpuidFunctionMsHvIsolationConfiguration:
        cpuidResult.MsHvIsolationConfiguration = mIsolationConfiguration;
        break;

    default:

        //
        // Fail on any unhandled hypervisor leaves.
        //

        if (cpuidInfo == NULL)
        {
            return FALSE;
        }
    }

    TrapFrame->Rax = cpuidResult.Eax;
    TrapFrame->Rbx = cpuidResult.Ebx;
    TrapFrame->Rcx = cpuidResult.Ecx;
    TrapFrame->Rdx = cpuidResult.Edx;

    return TRUE;
}

BOOLEAN
SecProcessVirtualCommunicationException (
    _In_ PTRAP_FRAME TrapFrame
    )
{
    UINT32 InstructionLength;

    switch (TrapFrame->ErrorCode)
    {
    case VC_EXIT_CODE_MSR:

        //
        // Examine the instruction to determine whether it is a read or write.
        //

        if (*(PUINT8)(TrapFrame->Rip + 1) == 0x30)
        {
            //
            // WRMSR.
            //

            return FALSE;
        }

        if (!SecProcessVirtualMsrRead(TrapFrame))
        {
            return FALSE;
        }

        InstructionLength = 2;
        break;

    case VC_EXIT_CODE_CPUID:
        if (!SecProcessVirtualCpuid(TrapFrame))
        {
            return FALSE;
        }

        InstructionLength = 2;
        break;

    default:
        return FALSE;
    }

    TrapFrame->Rip += InstructionLength;

    return TRUE;
}
