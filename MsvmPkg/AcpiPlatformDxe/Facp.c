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
    EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE *facp;

    //
    // Get configuration to determine if headless.
    //
    UINT32 consoleMode = GetConsoleMode();

    //
    // Set headless bit if console mode is not default (no video/kbd present)
    //
    if (consoleMode != ConfigLibConsoleModeDefault)
    {
        facp = (EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE *)Facp;

        facp->Flags |= EFI_ACPI_3_0_HEADLESS;

    }

    return EFI_SUCCESS;
}
