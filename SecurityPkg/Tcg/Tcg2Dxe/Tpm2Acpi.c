/*++

Copyright (c) Microsoft Corporation

Module Name:

    Tpm2Acpi.c

Abstract:

    This file contains routines to initialize and install TPM2 ACPI table.

Author:

    Jingbowu (jingbowu) 9-20-2013

--*/

#include <Protocol/AcpiTable.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/IoLib.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/Tpm2Acpi.h>
#include <TpmInterface.h>

EFI_TPM2_ACPI_TABLE mTpm20AcpiTable = {0};

VOID
Tpm2InitializeAcpiTable()
/*++

Routine Description:

    This routine fill in the TPM20 ACPI table entries.

Arguments:

    None

Return Value:

    EFI_STATUS.

--*/
{
    ZeroMem(&mTpm20AcpiTable, sizeof(EFI_TPM2_ACPI_TABLE));

    mTpm20AcpiTable.Header.Signature = 0x324D5054;          // 'TPM2'
    mTpm20AcpiTable.Header.Length = sizeof(EFI_TPM2_ACPI_TABLE);
    mTpm20AcpiTable.Header.Revision = 3;
    CopyMem(mTpm20AcpiTable.Header.OemId, PcdGetPtr(PcdAcpiDefaultOemId), sizeof(mTpm20AcpiTable.Header.OemId));
    mTpm20AcpiTable.Header.OemTableId = 0x202020204D505456; // 'VTPM    '
    mTpm20AcpiTable.Header.OemRevision = 0x1;
    mTpm20AcpiTable.Header.CreatorId = 0x5446534D;          // 'MSFT'
    mTpm20AcpiTable.Header.CreatorRevision = 0x00000001;

    mTpm20AcpiTable.StartMethod = EFI_TPM2_ACPI_TABLE_START_METHOD_COMMAND_RESPONSE_BUFFER_INTERFACE;

    mTpm20AcpiTable.AddressOfControlArea = (UINT64)FixedPcdGet32(PcdTpmBaseAddress);

    mTpm20AcpiTable.Header.Checksum = CalculateCheckSum8((UINT8*)&mTpm20AcpiTable, sizeof(EFI_TPM2_ACPI_TABLE));
}


EFI_STATUS
IntallTpm2AcpiTable(
    IN EFI_ACPI_TABLE_PROTOCOL* AcpiTable
    )
/*++

    This routine initialize and install TPM2 ACPI table.

Arguments:

    AcpiTable - A pointer to the ACPI table protocol.

Return Value:

    EFI_STATUS

--*/
{
    EFI_STATUS  status = EFI_SUCCESS;
    UINTN       tableHandle;

    Tpm2InitializeAcpiTable();

    status = AcpiTable->InstallAcpiTable(AcpiTable,
                                         &mTpm20AcpiTable,
                                         sizeof(mTpm20AcpiTable),
                                         &tableHandle);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    status = EFI_SUCCESS;

Cleanup:

    return status;
}
