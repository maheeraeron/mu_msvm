/*++

Copyright (c) Microsoft Corporation

Module Name:

    Apic.c

Abstract:

    This module is responsible for runtime initialization of the APIC APCI
    table.

Author:

    Rich Yampell (richyam) 11-Jul-2012

--*/

#include <PiDxe.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include "AcpiPlatform.h"

EFI_STATUS
ApicInitializeTable(
    __inout EFI_ACPI_DESCRIPTION_HEADER* Table
    )
/*++

Routine Description:

    Initializes the Apic table, using the BiosDevice.

Arguments:

    Table - The Apic Table, expressed as an EFI_ACPI_DESCRIPTION_HEADER*.

Return Value:

    EFI_SUCCESS

--*/
{
    EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_STRUCTURE* entry;
    INT32 highestLocalApicId;
    UINT8 index;
    UINT32 processorCount;
    VM_APIC_TABLE* table;

    table = (VM_APIC_TABLE *)Table;
    processorCount = PcdGet32(PcdProcessorCount);

    ASSERT(processorCount > 0);

    //
    // Initialize the local APIC entries.
    //

    highestLocalApicId = 0;
    for (index = 0; index < MAX_PROCESSORS; index++)
    {

        entry = &table->LocalApicTable[index];
        entry->Type = EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC;
        entry->Length = sizeof(*entry);
        entry->AcpiProcessorId = index + 1;
        entry->ApicId = index;
        entry->Flags = 0;
        if (index < processorCount)
        {
            entry->ApicId = index;
            entry->Flags = EFI_ACPI_3_0_LOCAL_APIC_ENABLED;
        }
    }

    //
    // Update the I/O APIC ID.
    //

    table->IoApic.IoApicId = (UINT8)processorCount;
    return EFI_SUCCESS;
}

