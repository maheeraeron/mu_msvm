/*++

Copyright (c) Microsoft Corporation

Module Name:

    ConfigLib.c

Abstract:

    Library that provides version agnostic access to virtual machine configuration.

Author:

    Larry Cleeton (lcleeton) - 02-May-2014

--*/

#include <EfiNt.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <BiosInterface.h>
#include <BiosDeviceAccess.h>

UINT32
GetNfitSize(
    )
/*++

Routine Description:

    Returns the size of the NFIT.

Arguments:

    None

Return Value:

    The size of the NFIT.

--*/
{
    return ReadBiosDevice(BiosConfigNfitSize);
}

void
GetNfit(
    UINT64 Address
    )
/*++

Routine Description:

    Gets the NFIT.

Arguments:

    Address - the GPA at which to write the NFIT table

Return Value:

    None.

--*/
{
    ASSERT((UINT64) Address < 0xFFFFFFFFULL);
    WriteBiosDevice(BiosConfigNfitPopulate, (UINT32) Address);
}

void
SetVpmemACPIBuffer(
    UINT64 Address
    )
/*++

Routine Description:

    Sets the pointer to the VPMem ACPI Method Buffer.

Arguments:

    None

Return Value:

    None.

--*/
{
    ASSERT(Address < 0xFFFFFFFFULL);
    WriteBiosDevice(BiosConfigVpmemSetACPIBuffer, (UINT32) Address);
}

void
SetGenerationIdAddress(
    UINT64 Value
    )
/*++

Routine Description:

    Communicates the Generation ID memory location to the VDev.

Arguments:

    Value - the GPA of the Generation ID

Return Value:

    n/a

--*/
{
    IoWrite32(BiosAddressPort, BiosConfigGenerationIdPtrLow);
    IoWrite32(BiosDataPort, (UINT32)Value);
    IoWrite32(BiosAddressPort, BiosConfigGenerationIdPtrHigh);
    IoWrite32(BiosDataPort, (UINT32)(Value >> 32));
}