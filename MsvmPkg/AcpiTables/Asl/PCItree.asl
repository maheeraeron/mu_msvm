/*++

Copyright (c) Microsoft Corporation

Module Name:

    PCItree.asl

Abstract:

    An ASL file defining the PCI root bus and associated interrupt link
    devices.

--*/

Scope(\_SB)
{
    //
    // Root PCI bus device.
    //

    Device(PCI0)
    {
        Name(_HID, EISAID("PNP0A03"))    // PnP ID for PCI Bus
        Name(_ADR, 0x00000000)    // Root PCI bus is always of address 0
        Name(_BBN, 0x0000)    // Bus number, optional for the Root PCI Bus
        Name(_UID, 0x0000)    // Unique Bus ID, optional

        //
        // PCI routing table. IRQ 11 is hard-coded in IsaBusDevice.cpp and
        // PciBusDevice.cpp.
        //

        Name(_PRT,
            Package()
            {
                //
                // Slot 1: VGA.
                //

                Package(){ 0x0008FFFF, 0, 0, 11 },

                //
                // Slot 3: Ethernet (multi-function).
                //

                Package(){ 0x000AFFFF, 0, 0, 11 },
            }
        )

        //
        // Bus resources. These define the MMIO space and I/O port space that
        // are available to child devices of the PCI bus, as well as resources
        // that are used by the PCI host bridge itself.
        //

        Name(_CRS, ResourceTemplate()
        {
            //
            // Bus Number resources
            //

            WORDBusNumber(ResourceProducer, MinFixed, MaxFixed, PosDecode,
            // Granularity     Min     Max     Translation     Range (Length = Max-Min+1)
                0x00,   0x00,   0xff,   0x00,           0x0100)

            //
            // I/O ports used by the bus device.
            //

            IO(Decode16, 0xCF8, 0xCF8, 1, 8)

            //
            // I/O ports for the IDE controller.
            //

            WORDIO(ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
            // Granularity     Min     Max     Translation     Range (Length = Max-Min+1)
                0x00,   0x01F0,   0x01F7, 0x00,           0x0008)

            WORDIO(ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
            // Granularity     Min     Max     Translation     Range (Length = Max-Min+1)
                0x00,   0x0170,   0x0177, 0x00,           0x0008)

            WORDIO(ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
            // Granularity     Min     Max     Translation     Range (Length = Max-Min+1)
                0x00,   0x03F6,   0x03F6, 0x00,           0x0001)

            WORDIO(ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
            // Granularity     Min     Max     Translation     Range (Length = Max-Min+1)
                0x00,   0x0376,   0x0376, 0x00,           0x0001)

            //
            // I/O ports passed through to the child devices.
            //

            WORDIO(ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
            // Granularity     Min     Max     Translation     Range (Length = Max-Min+1)
                0x00,   0x0D00, 0xffff, 0x00,           0xf300)

            //
            // MMIO space for the VGA device.
            //

            DWORDMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
            // Granularity     Min     Max             Translation     Range (Length = Max-Min+1)
                0x00,   0x0a0000,0x0bffff,0x00,         0x020000)

            //
            // MMIO space below 4GB.
            //

            DWORDMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
            // Granularity     Min     Max     Translation     Range (Length = Max-Min+1)
                0x00,   0x00000,0x00000,0x00,           0x00000,,,
                MEM6)   // Name declaration for this descriptor
        })

        CreateDwordField(_CRS, MEM6._MIN, MIN6)  // Min
        CreateDwordField(_CRS, MEM6._MAX, MAX6)  // Max
        CreateDwordField(_CRS, MEM6._LEN, LEN6)  // Memory length

        Method(_INI, 0)
        {
            //
            // Update the resource descriptor with the first MMIO space gap.
            //

            Store(MG2B, MIN6)
            Store(MG2L, LEN6)
            Store(MG2L, Local0)
            Add(MIN6, Decrement(Local0), MAX6)
        }
    }
}

