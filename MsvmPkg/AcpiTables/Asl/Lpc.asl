/*++

Copyright (c) Microsoft Corporation

Module Name:

    Lpc.asl

Abstract:

    An ASL file defining the LPC devices and other motherboard resources.

--*/

Scope(_SB)
{
    //
    // Real time clock (RTC) (RTCDevice.cpp).
    //

    Device(RTC0)
    {
        Name(_HID, EISAID("PNP0B00")) // AT real-time clock
        Name(_UID, 0)
        Name (_CRS, ResourceTemplate()
        {
            IO(Decode16, 0x70, 0x70, 0, 0x2)
            IRQNoFlags(){8}
        })
    }
}

