/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    check.c

Abstract:

    This module implements the necessary memory check routines for the boot
    debugger.

Author:

    Jamie Schwartz (jamschw) June 2004

Environment:

    Boot

Revision History:

--*/

// ------------------------------------------------------------------- Includes

#include "Bd.h"

// ------------------------------------------------------------------ Functions

BOOL
BdReadCheck (
    UINT_PTR Address,
    size_t Size
    )
/*++

Routine Description:

    This routine determines if the specified address+size can be read.

Arguments:

    Address - Supplies the virtual address to check.

    Size - Number of bytes.

Return Value:

    if the address is valid and readable

--*/
{
    UINT_PTR const end = (Address + Size);
    UINT_PTR const limit = 0xFFFFFFFF;
    return Address < limit && end <= limit && end >= Address;
}

BOOL
BdWriteCheck (
    UINT_PTR Address,
    size_t Size
    )
/*++

Routine Description:

    This routine determines if the specified address+size can be written.

Arguments:

    Address - Supplies the virtual address to check.

    Size - Number of bytes.

Return Value:

    if the address is valid and writeable

--*/
{
    return BdReadCheck (Address, Size);
}

PVOID
BdTranslatePhysicalAddress (
    __in PHYSICAL_ADDRESS Address
    )
/*++

Routine Description:

    This routine returns the phyiscal address for a physical address
    which is valid (mapped).

    TODO: NOTE: I can't tell if this routine is suppose to map a physical
          address or return the physical address.  If so, this routine
          will need to be modified if we break 1:1 mapping.

Arguments:

    Address - Supplies the physical address to check.

Return Value:

    Returns NULL if the address is not valid or readable. Otherwise,
    returns the physical address of the corresponding virtual address.

--*/
{
    //
    // EFI environment is identity mapped.
    //

    return (PVOID)(UINTN)Address;
}
