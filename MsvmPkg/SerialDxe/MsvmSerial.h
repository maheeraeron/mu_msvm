/*++

Copyright (c) Microsoft Corporation

Module Name:

    MsvmSerial.h

Abstract:

    Provides the protocol definition for MSVM_SERIAL_BUS_PROTOCOL.

    This is a tag protocol used privately in the SerialDxe driver.

Author:

    Larry Cleeton (lcleeton) - 08-Oct-2014

--*/

#pragma once

#define MSVM_SERIAL_BUS_PROTOCOL_GUID \
    {0x316fa37e, 0x63e5, 0x4179, {0xba, 0xb9, 0x0f, 0x66, 0x5a, 0x4c, 0x06, 0x15}}

GLOBAL_REMOVE_IF_UNREFERENCED EFI_GUID gMsvmSerialBusProtocolGuid = MSVM_SERIAL_BUS_PROTOCOL_GUID;

