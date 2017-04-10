/*++

Copyright (c) Microsoft Corporation
Copyright (c) 1985-2001, American Megatrends, Inc.

Module Name:

    AmlUpd.asl

Abstract:

    An ASL file defining the OperationRegion "BIOS" used to transfer parameters
    from UEFI to the ACPI ASL code.

--*/

//
// The following line defines a table in system memory that can be consulted
// elsewhere in the ASL code. This table contains parameters, most of which
// are computed at runtime by the UEFI code in DsdtAllocateAmlData(). The
// address of this table is dynamic as well and is updated during ACPI table
// initialialization.
//
// N.B. This line must not be changed, since the compiled form of this statement
// is searched for by DsdtInitializeTable() and updated with the address of the
// actual allocated table in runtime memory.
//

OperationRegion(BIOS, SystemMemory, 0xffffff00, 0xff)

//
// This structure must match DSDT_AML_DATA, defined in Dsdt.c.
//

Field(BIOS, ByteAcc, NoLock, Preserve)
{
    MG2B,32,        // Base of first MMIO hole in bytes.
    MG2L,32,        // Length of first MMIO hole in bytes.
    HMIB,32,        // Base of the second MMIO hole in MB.
    HMIL,32,        // Length of the second MMIO hole in MB.
    GCAL,32,        // Lower 32 bits of address of Generation counter
    GCAH,32,        // Upper 32 bits of address of Generation counter
    PCNT,32,        // Processor count
    NVDA,32,        // NVDIMM Method buffer address
    SCFG,8,         // Serial controller enabled/disabled
    TCFG,8,         // TPM enabled/disabled
    PCFG,8,         // OEMP table load enabled/disabled
    HCFG,8,         // Hibernation enabled/disabled
    NCFG,8,         // PMEM (NVDIMMs) enabled/disabled
}

