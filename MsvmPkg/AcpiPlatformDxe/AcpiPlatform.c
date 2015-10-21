/*++

Copyright (c) Microsoft Corporation

Module Name:

    AcpiPlatform.c

Abstract:

    This file contains routines to locate ACPI tables in the firmware volume,
    update them appropriately, and install them via the AcpiTable protocol.

Author:

    Rich Yampell (richyam) 9-Jul-2012

--*/

#include <PiDxe.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/FirmwareVolume2.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Guid/Acpi.h>
#include <AcpiPlatform.h>

typedef EFI_STATUS (*INIT_ROUTINE)(EFI_ACPI_DESCRIPTION_HEADER*);

typedef struct _INIT_TABLE_ENTRY
{
    UINT32 signature;
    INIT_ROUTINE initRoutine;
} INIT_TABLE_ENTRY;

//
// The list of tables that need to be updated at runtime. All other tables
// are installed without modification.
//

INIT_TABLE_ENTRY AcpiInitTable[] =
{
    { VM_ACPI_ENTROPY_TABLE_SIGNATURE, Oem0InitializeTable },
    { EFI_ACPI_3_0_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE, ApicInitializeTable },
    { EFI_ACPI_3_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE, DsdtInitializeTable },
    { EFI_ACPI_3_0_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_SIGNATURE, SpcrInitializeTable },
    { EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE, FacpInitializeTable },
};

#define NUM_TABLE_ENTRIES (sizeof(AcpiInitTable) / sizeof(INIT_TABLE_ENTRY))

EFI_STATUS
RuntimeInitializeTableIfNecessary(
    __inout EFI_ACPI_DESCRIPTION_HEADER* Table
    )
/*++

Routine Description:

    Performs any runtime initialization required by a given acpi table.

Arguments:

    Table - The acpi table which may require runtime initialization.

Return Value:

    An appropriate EFI_STATUS value.

--*/
{
    INIT_TABLE_ENTRY* entry;
    UINT32 index;

    for (index = 0; index < NUM_TABLE_ENTRIES; index++)
    {
        entry = &AcpiInitTable[index];
        if (entry->signature == Table->Signature)
        {
            return entry->initRoutine(Table);
        }
    }

    return EFI_SUCCESS;
}


EFI_STATUS
LocateFvInstanceWithTables(
    __out EFI_FIRMWARE_VOLUME2_PROTOCOL** Instance
    )
/*++

Routine Description:

    Locates the first instance of a protocol.  If the protocol requested is an
    FV protocol, then it will return the first FV that contains the ACPI table
    storage file.

Arguments:

    Instance - Return pointer to the first instance of the protocol

Return Value:

    An appropriate EFI_STATUS value.

--*/
{
    EFI_STATUS                      status;
    EFI_HANDLE*                     handleBuffer;
    UINTN                           numberOfHandles;
    EFI_FV_FILETYPE                 fileType;
    UINT32                          fvStatus;
    EFI_FV_FILE_ATTRIBUTES          attributes;
    UINTN                           size;
    UINTN                           index;
    EFI_FIRMWARE_VOLUME2_PROTOCOL*  fvInstance;

    fvStatus = 0;

    //
    // Locate protocol.
    //
    status = gBS->LocateHandleBuffer(ByProtocol,
                                     &gEfiFirmwareVolume2ProtocolGuid,
                                     NULL,
                                     &numberOfHandles,
                                     &handleBuffer);

    if (EFI_ERROR(status))
    {
        //
        // Defined errors at this time are not found and out of resources.
        //
        return status;
    }

    //
    // Looking for FV with ACPI storage file
    //

    status = EFI_NOT_FOUND;
    for (index = 0; index < numberOfHandles; index++)
    {
        //
        // Get the protocol on this handle
        // This should not fail because of LocatehandleBuffer
        //

        status = gBS->HandleProtocol(handleBuffer[index],
                                     &gEfiFirmwareVolume2ProtocolGuid,
                                     (VOID **)&fvInstance);

        ASSERT_EFI_ERROR(status);

        //
        // See if it has the ACPI storage file
        //

        status = fvInstance->ReadFile(fvInstance,
                                      (EFI_GUID *)PcdGetPtr(PcdAcpiTableStorageFile),
                                      NULL,
                                      &size,
                                      &fileType,
                                      &attributes,
                                      &fvStatus);

        if (!EFI_ERROR(status))
        {
            *Instance = fvInstance;
            break;
        }
        else if (status != EFI_NOT_FOUND)
        {
            break;
        }
    }

    //
    // Our exit status is determined by the success of the previous operations
    // If the protocol was found, Instance already points to it.
    //

    gBS->FreePool(handleBuffer);
    return status;
}


EFI_STATUS
AcpiInstallSratTable(
    EFI_ACPI_TABLE_PROTOCOL *AcpiTable
    )
/*++

Routine Description:

    Retrieves the SRAT table from the worker process and installs it.

Arguments:

    AcpiTable - A pointer to the ACPI table protocol.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;
    EFI_ACPI_DESCRIPTION_HEADER *table;
    UINTN tableHandle;

    //
    // Get pointer to the SRAT.
    //
    table = (EFI_ACPI_DESCRIPTION_HEADER *)GetSrat();
    ASSERT(table->Length == GetSratSize());

    //
    // Install it into the published tables.
    //
    status = AcpiTable->InstallAcpiTable(AcpiTable,
                                         table,
                                         table->Length,
                                         &tableHandle);

    return status;
}


EFI_STATUS
EFIAPI
AcpiPlatformInitializeAcpiTables(
    __in EFI_HANDLE        ImageHandle,
    __in EFI_SYSTEM_TABLE* SystemTable
    )
/*++

Routine Description:

    Entry point of the ACPI platform driver.

Arguments:

    ImageHandle - Driver Image Handle.

    SystemTable - EFI System Table.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_ACPI_TABLE_PROTOCOL*        acpiTable;
    EFI_FIRMWARE_VOLUME2_PROTOCOL*  fwVol;
    INTN                            instance;
    EFI_ACPI_DESCRIPTION_HEADER*    currentTable;
    UINT32                          fvStatus;
    UINTN                           size;
    EFI_STATUS                      status;
    UINTN                           tableHandle;

    //
    // Find the AcpiTable protocol.
    //
    status = gBS->LocateProtocol(&gEfiAcpiTableProtocolGuid, NULL, (VOID**) &acpiTable);

    ASSERT_EFI_ERROR(status);

    //
    // Locate the firmware volume protocol.
    //

    status = LocateFvInstanceWithTables(&fwVol);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    //
    // Read tables from the storage file.
    //

    for (instance = 0; ; instance += 1)
    {
        currentTable = NULL;
        status = fwVol->ReadSection(fwVol,
                                    (EFI_GUID*) PcdGetPtr(PcdAcpiTableStorageFile),
                                    EFI_SECTION_RAW,
                                    instance,
                                    (VOID **)&currentTable,
                                    &size,
                                    &fvStatus);

        if (EFI_ERROR(status))
        {
            if (status == EFI_NOT_FOUND)
            {
                break;
            }

            goto Cleanup;
        }

        //
        // Add the table.
        //

        ASSERT(size >= currentTable->Length);

        status = RuntimeInitializeTableIfNecessary(currentTable);

        if (EFI_ERROR(status))
        {
            //
            // If the table init routine returned EFI_UNSUPPORTED then
            // don't install this table and continue normally.
            //
            if (status == EFI_UNSUPPORTED)
            {
                status = EFI_SUCCESS;
            }
        }
        else
        {
            //
            // Install the table.
            //
            status = acpiTable->InstallAcpiTable(acpiTable,
                                                 currentTable,
                                                 currentTable->Length,
                                                 &tableHandle);
        }

        //
        // Free memory allocated by ReadSection.
        //

        gBS->FreePool(currentTable);

        if (EFI_ERROR(status))
        {
            goto Cleanup;
        }
    }

    //
    // Add the SRAT table.
    //

    status = AcpiInstallSratTable(acpiTable);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    status = EFI_SUCCESS;

Cleanup:
    return status;
}



