/*++

Copyright (c) Microsoft Corporation

Module Name:

    oem1.asl

Abstract:

    A table (OEM1) that contains the declarations of the COM port UARTs.
    This table is condtionally loaded in the _INI method of the _SB scope.
    It is loaded if the COM ports are configured for the VM.

--*/

DefinitionBlock (
    "oem1.aml",
    "OEM1",
    0x02,           // DSDT Compliance revision.
    "MSFTVM",       // OEM ID (6 byte string)
    "UARTS",        // OEM table ID  (8 byte string)
    0x01            // OEM version of DSDT table (4 byte Integer)
    )
{
    Scope(_SB)
    {
        //
        // COM1 (SerialControllerDevice.cpp).
        //

        Device(UAR1)
        {
            Name(_HID, EISAID("PNP0501")) // 16550A-compatible COM port
            Name(_DDN, "COM1")
            Name(_UID, 1)
            Name(_CRS, ResourceTemplate()
            {
                IRQ(Edge,ActiveHigh) {4}
                IO(Decode16, 0x3f8, 0x3f8, 1, 8)
            })
        }

        //
        // COM2 (SerialControllerDevice.cpp).
        //

        Device(UAR2)
        {
            Name(_HID, EISAID("PNP0501")) // 16550A-compatible COM port
            Name(_DDN, "COM2")
            Name(_UID, 2)
            Name(_CRS, ResourceTemplate()
            {
                IRQ(Edge,ActiveHigh) {3}
                IO(Decode16, 0x2f8, 0x2f8, 1, 8)
            })
        }
    }
}
