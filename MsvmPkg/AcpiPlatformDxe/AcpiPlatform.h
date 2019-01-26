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

#if defined (MDE_CPU_AARCH64)
EFI_STATUS
GicInitializeTable(
    __inout EFI_ACPI_DESCRIPTION_HEADER* Table
    );
#endif

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
