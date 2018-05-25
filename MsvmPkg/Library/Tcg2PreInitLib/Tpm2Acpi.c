/** @file -- Tpm2Acpi.c
This file contains routines to initialize and install TPM2 ACPI table.
Specific to Hyper-V vDevice.

Copyright (c) 2018, Microsoft Corporation

All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**/

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


/*++

    This routine initialize and install TPM2 ACPI table.

Arguments:

    NONE

Return Value:

    EFI_STATUS

--*/
EFI_STATUS
EFIAPI
IntallTpm2AcpiTable (
    VOID
    )
{
    EFI_STATUS                  Status = EFI_SUCCESS;
    UINTN                       TableHandle;
    EFI_ACPI_TABLE_PROTOCOL     *AcpiTableProtocol;

    // TODO: Register a callback for when this protocol is available, if EFI_NOT_FOUND.
    Status = gBS->LocateProtocol(&gEfiAcpiTableProtocolGuid, NULL, (VOID**) &AcpiTableProtocol);
    if (EFI_ERROR(Status))
    {
        goto Cleanup;
    }

    Tpm2InitializeAcpiTable();

    Status = AcpiTableProtocol->InstallAcpiTable(AcpiTableProtocol,
                                                 &mTpm20AcpiTable,
                                                 sizeof(mTpm20AcpiTable),
                                                 &TableHandle);
    if (EFI_ERROR(Status))
    {
        goto Cleanup;
    }

    Status = EFI_SUCCESS;

Cleanup:

    return Status;
}
