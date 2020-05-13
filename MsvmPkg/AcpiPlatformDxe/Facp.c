/*++

Copyright (c) Microsoft Corporation

Module Name:

    Facp.c

Abstract:

    This module is responsible for runtime initialization of the FACP acpi table.

Author:

    Larry Cleeton (lcleeton) 03-Feb-2015

--*/

#include <PiDxe.h>
#include "AcpiPlatform.h"
#include <IsolationTypes.h>
#include <Library/BaseMemoryLib.h>

#include <BiosInterface.h>

EFI_STATUS
FacpInitializeTable(
    __inout EFI_ACPI_DESCRIPTION_HEADER* Facp
    )
/*++

Routine Description:

    Initializes the Facp table.

Arguments:

    Facp - The FACP Table, expressed as an EFI_ACPI_DESCRIPTION_HEADER*.

Return Value:

    EFI_SUCCESS

--*/
{
    EFI_ACPI_6_2_FIXED_ACPI_DESCRIPTION_TABLE *facp = (EFI_ACPI_6_2_FIXED_ACPI_DESCRIPTION_TABLE *)Facp;

    //
    // Get configuration to determine if headless.
    //
    UINT32 consoleMode = PcdGet8(PcdConsoleMode);

    //
    // Set headless bit if console mode is not default (no video/kbd present)
    //
    if (consoleMode != ConfigLibConsoleModeDefault)
    {
        facp->Flags |= EFI_ACPI_6_2_HEADLESS;
    }

    //
    // Set the hypervisor vendor identity to MsHyperV
    //
    CopyMem(&facp->HypervisorVendorIdentity, "MsHyperV", 8);


    if (PcdGetBool(PcdLowPowerS0IdleEnabled))
    {
        //
        // Set EFI_ACPI_6_2_LOW_POWER_S0_IDLE_CAPABLE flag.
        // Pending investigation, EFI_ACPI_6_2_LOW_POWER_S0_IDLE_CAPABLE causes negative side-effects in a VM.
        //
        facp->Flags |= EFI_ACPI_6_2_LOW_POWER_S0_IDLE_CAPABLE;
    }

    //
    // Special case if battery is enabled
    //
    if (PcdGetBool(PcdVirtualBatteryEnabled))
    {
        //
        // Set the profile to Mobile
        //
        facp->PreferredPmProfile = EFI_ACPI_6_2_PM_PROFILE_MOBILE;
    }


#if defined(MDE_CPU_X64)
    // If this is a SNP VM, report it as hardware reduced. Zero out any of
    // filled in legacy structures.
    if (PcdGet32(PcdIsolationArchitecture) == UefiIsolationTypeSnp)
    {
        facp->Flags = EFI_ACPI_6_2_WBINVD |
             EFI_ACPI_6_2_PROC_C1 |
             EFI_ACPI_6_2_PWR_BUTTON |
             EFI_ACPI_6_2_SLP_BUTTON |
             EFI_ACPI_6_2_TMR_VAL_EXT |
             EFI_ACPI_6_2_HW_REDUCED_ACPI;

        // Zero out set fields between offsets 46 - 108
        ZeroMem(&facp->SciInt,
            FIELD_OFFSET(EFI_ACPI_6_2_FIXED_ACPI_DESCRIPTION_TABLE, IaPcBootArch) - FIELD_OFFSET(EFI_ACPI_6_2_FIXED_ACPI_DESCRIPTION_TABLE, SciInt));

        // Zero out set fields between offsets 148 - 232
        ZeroMem(&facp->XPm1aEvtBlk,
            FIELD_OFFSET(EFI_ACPI_6_2_FIXED_ACPI_DESCRIPTION_TABLE, HypervisorVendorIdentity) - FIELD_OFFSET(EFI_ACPI_6_2_FIXED_ACPI_DESCRIPTION_TABLE, XPm1aEvtBlk));
    }
#endif

    return EFI_SUCCESS;
}
