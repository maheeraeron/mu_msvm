/*++

Copyright (c) Microsoft Corporation

Module Name:

    Madt.c

Abstract:

    This module is responsible for runtime initialization of the MADT table.

Author:

    Chris Oo (cho) 17-Apr-2017

--*/

#include <PiDxe.h>
#include <Library/DebugLib.h>
#if defined(MDE_CPU_AARCH64)
#include <Library/ArmGicLib.h>
#endif
#include "AcpiPlatform.h"

#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)

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



#elif defined(MDE_CPU_AARCH64)

//
// Size of each GICD for GICv3
//
#define REDIST_MAPPING_SIZE_V3 (ARM_GICR_CTLR_FRAME_SIZE + ARM_GICR_SGI_PPI_FRAME_SIZE)

EFI_STATUS
GicInitializeTable(
    _Inout_ EFI_ACPI_DESCRIPTION_HEADER* Table
    )
/*++

Routine Description:

    Initializes the Madt table, using the BiosDevice.

Arguments:

    Table - The Madt Table, expressed as an EFI_ACPI_DESCRIPTION_HEADER*.

Return Value:

    EFI_SUCCESS

--*/
{
    EFI_ACPI_6_1_GIC_STRUCTURE* gicc;
    UINT8 index, gicVersion;
    UINT32 processorCount;
    UINT64* processorMPIDRValues = NULL;
    VM_MADT_TABLE* table;
    EFI_STATUS status = EFI_SUCCESS;
    ARM_GIC_ARCH_REVISION gicVersionEnum;

    table = (VM_MADT_TABLE *)Table;
    processorCount = PcdGet32(PcdProcessorCount);

    ASSERT(processorCount > 0);

    //
    // Determine GIC version to expose
    //
    gicVersionEnum = ArmGicGetSupportedArchRevision();
    if (gicVersionEnum == ARM_GIC_ARCH_REVISION_2)
    {
        DEBUG((DEBUG_VERBOSE, ">>> Exposing GICv2\n"));
        //
        // GICv2 only supports 8 VPs
        //
        ASSERT(processorCount <= 8);
        gicVersion = EFI_ACPI_6_1_GIC_V2;
    }
    else if (gicVersionEnum == ARM_GIC_ARCH_REVISION_3)
    {
        DEBUG((DEBUG_VERBOSE, ">>> Exposing GICv3\n"));
        gicVersion = EFI_ACPI_6_1_GIC_V3;
    }
    else
    {
        DEBUG((DEBUG_VERBOSE, ">>> Unexpected GIC version detected\n"));
        gicVersion = 0;
        ASSERT(FALSE);
        status = EFI_INVALID_PARAMETER;
        goto Cleanup;
    }

    table->GICD.GicVersion = gicVersion;

    //
    // Set the overall size of the table based on number of processors.
    //
    table->Header.Header.Length = sizeof(EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER) +
        sizeof(EFI_ACPI_6_1_GIC_DISTRIBUTOR_STRUCTURE) +
        sizeof(EFI_ACPI_6_1_GIC_STRUCTURE) * processorCount;

    //
    // Get MPIDR values from the config blob parsed in PEI.
    //
    processorMPIDRValues = (UINT64*)PcdGet64(PcdProcessorMPIDRValuesPtr);

    //
    // Initialize GICC entries, only ones that are actually exposed.
    //

    for (index = 0; index < processorCount; index++)
    {
        gicc = &table->GICC[index];

        memset(gicc, 0, sizeof(EFI_ACPI_6_1_GIC_STRUCTURE));

        gicc->Type             = EFI_ACPI_6_1_GIC;
        gicc->Length           = sizeof(EFI_ACPI_6_1_GIC_STRUCTURE);

        gicc->AcpiProcessorUid = index + 1; // Processor UIDs start at 1
        gicc->Flags            = EFI_ACPI_6_1_GIC_ENABLED;

        //
        // ACPI requires for ARMv8 that bits 24 thru 31 and bits 40 thru 63
        // must be zero. However, the ARM spec requires that bit 31 is set, so
        // we need to make sure to mask off any bits from the MPIDR values we
        // got from the Bios VDEV.
        //
        gicc->MPIDR = processorMPIDRValues[index] & 0xFF00FFFFFFULL;

        if (gicVersion == EFI_ACPI_6_1_GIC_V2)
        {
            gicc->CPUInterfaceNumber = index;
        }
        else if (gicVersion == EFI_ACPI_6_1_GIC_V3)
        {
            gicc->GICRBaseAddress = FixedPcdGet64(PcdGicRedistributorsBase) +
                REDIST_MAPPING_SIZE_V3 * index;
        }
        else
        {
            //
            // Unsupported GIC version
            //
            status = EFI_INVALID_PARAMETER;
            ASSERT(FALSE);
            goto Cleanup;
        }
    }


Cleanup:

    return status;
}

#endif

