/*++

Copyright (c) Microsoft Corporation

Module Name:

    AcpiPlatform.h

Abstract:

    This file contains declarations and definitions used globally in the
    MsvmAcpiPlatformDxe driver.

--*/

#pragma once

#include <IndustryStandard/Acpi.h>

#include <EfiNt.h>
#include <AcpiTables.h>
#include <Library/ConfigLib.h>

EFI_STATUS
Oem0InitializeTable(
    __inout EFI_ACPI_DESCRIPTION_HEADER* Table
    );

EFI_STATUS
ApicInitializeTable(
    __inout EFI_ACPI_DESCRIPTION_HEADER* Table
    );

EFI_STATUS
DsdtInitializeTable(
    __inout EFI_ACPI_DESCRIPTION_HEADER* Dsdt
    );

EFI_STATUS
SpcrInitializeTable(
    __inout EFI_ACPI_DESCRIPTION_HEADER* Spcr
    );

EFI_STATUS
FacpInitializeTable(
    __inout EFI_ACPI_DESCRIPTION_HEADER* Facp
    );
