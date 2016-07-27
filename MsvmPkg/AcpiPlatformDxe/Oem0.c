/*++

Copyright (c) Microsoft Corporation

Module Name:

    Oem0.c

Abstract:

    This module is responsible for runtime initialization of the entropy
    table.

Author:

    Rich Yampell (richyam) 9-Jul-2012

--*/

#include <PiDxe.h>
#include <Library/BaseMemoryLib.h>
#include "AcpiPlatform.h"

//
// Entry point
//

EFI_STATUS
Oem0InitializeTable(
    __inout EFI_ACPI_DESCRIPTION_HEADER* Table
    )
/*++

Routine Description:

    Initializes the OEM0 table.

Arguments:

    Table - The Oem0 Table, expressed as an EFI_ACPI_DESCRIPTION_HEADER*.

Return Value:

    EFI_SUCCESS

--*/
{
    VM_ACPI_ENTROPY_TABLE *table;

    //
    // Copy the entropy data from the configuration.
    //
    table = (VM_ACPI_ENTROPY_TABLE *)Table;

    CopyMem(table->Data, GetEntropyData(), ConfigLibEntropyDataSize);
    
    return EFI_SUCCESS;
}

