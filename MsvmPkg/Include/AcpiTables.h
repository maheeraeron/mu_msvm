/*++

Copyright (c) Microsoft Corporation

Module Name:

    AcpiTables.h

Abstract:

    This file contains declarations and definitions shared between the
    ACPI table ASLC files and the ACPI platform DXE driver.

--*/

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/SerialPortConsoleRedirectionTable.h>
#include <BiosInterface.h>

#define STANDARD_HEADER(sig, length, rev)                   \
    {                                                       \
        sig,                                                \
        length,                                             \
        rev,                                                \
        0,                                                  \
        "VRTUAL",                                           \
        SIGNATURE_64('M','I','C','R','O','S','F','T'),      \
        1,                                                  \
        SIGNATURE_32('M','S','F','T'),                      \
        1                                                   \
    },

#define MAX_PROCESSORS 240

#pragma pack(push, 1)

//
// MADT information is architecture dependent.
//
#if defined (MDE_CPU_X64)

typedef struct _VM_APIC_TABLE
{
    EFI_ACPI_3_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER Header;
    EFI_ACPI_3_0_IO_APIC_STRUCTURE IoApic;
    EFI_ACPI_3_0_LOCAL_APIC_NMI_STRUCTURE LocalApicNmi;
    EFI_ACPI_3_0_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE Override1;
    EFI_ACPI_3_0_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE Override2;
    EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_STRUCTURE LocalApicTable[MAX_PROCESSORS];
} VM_APIC_TABLE;

#elif defined (MDE_CPU_AARCH64)

typedef struct _VM_MADT_TABLE
{
    EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER Header;
    EFI_ACPI_6_1_GIC_DISTRIBUTOR_STRUCTURE GICD;
    EFI_ACPI_6_1_GIC_STRUCTURE GICC[MAX_PROCESSORS]; // GICV2 only supports 8 CPUs, so technically MAX_PROCESSORS is a lie in that case
} VM_MADT_TABLE;

#else
    #error Unsupported Architecture
#endif

typedef struct _VM_ACPI_ENTROPY_TABLE
{
    EFI_ACPI_DESCRIPTION_HEADER Header;
    UINT8 Data[BiosInterfaceEntropyTableSize];
} VM_ACPI_ENTROPY_TABLE;

#define VM_ACPI_ENTROPY_TABLE_SIGNATURE SIGNATURE_32('O','E','M','0')

#pragma pack(pop)

