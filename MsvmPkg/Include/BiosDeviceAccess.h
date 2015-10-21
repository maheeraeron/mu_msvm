/*++

Copyright (c) Microsoft Corporation

Module Name:

    BiosDeviceAccess.h

Abstract:

    Provides common functions for reading and writing the Hyper-V BIOS device.

Author:

    Kris Harper (kharp) 10-Oct-2013

--*/

#pragma once


__inline
VOID
WriteBiosDevice(
    _In_    UINT32                  Register,
    _In_    UINT32                  Value
    )
/*++

Routine Description:

    Issues a command to the Hyper-V BiosDevice.

Arguments:

    Register - The specific command to be issued.

    Value - The data value associated with Command.

Return Value:

    None.

--*/
{
    IoWrite32(BiosAddressPort, Register);
    IoWrite32(BiosDataPort, Value);
}


__inline
UINT32
ReadBiosDevice(
    _In_    UINT32                  Register
    )
/*++

Routine Description:

    Reads a value from the Hyper-V BiosDevice.

Arguments:

    Register - The BIOS device register to read from.

Return Value:

    The data value read from the register.

--*/
{
    IoWrite32(BiosAddressPort, Register);
    return IoRead32(BiosDataPort);
}
