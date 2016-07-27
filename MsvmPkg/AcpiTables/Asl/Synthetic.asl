/*++

Copyright (c) Microsoft Corporation

Module Name:

    Synthetic.asl

Abstract:

    An ASL file defining synthetic devices.

--*/

Scope(\_SB)
{
    //
    // ACPI module device that holds the VMBus device. A module device is
    // useful here because Windows automatically provides a resource arbiter
    // for module devices that have resources. This allows VMBus's child devices
    // (including virtual PCI and its children) to claim memory resources
    // without VMBus having to provide its own arbiter implementation.
    //
    // Without this, Windows looks for memory space on any ISA bus it can find,
    // which means that the PCI bus arbiter gets used. However, since the PCI
    // bus is optional on UEFI Hyper-V VMs, this is not an option.
    //
    // An alternative would be to write an arbiter implementation for VMBus,
    // but this would prevent inbox Windows 7 and Windows 8 VMBus drivers from
    // claiming memory space, which would prevent synthetic video and SR-IOV
    // devices from working.
    //

    Device(VMOD)
    {
        Name(_HID, "ACPI0004")
        Name(_UID, 0)
        Name(_CRS,
            ResourceTemplate()
            {
                // MMIO above 4GB
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                //  Granularity Min Max Translation Range (Length = Max-Min+1)
                    0,          0,  0,  0,          0,,,
                    MEM7)

                //
                // MMIO space below 4GB.
                //

                DWORDMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                // Granularity Min Max Translation Range (Length = Max-Min+1)
                   0,          0,  0,  0,          0,,,
                   MEM6)   // Name declaration for this descriptor

            }
        )

        CreateDwordField(_CRS, MEM6._MIN, MIN6)  // Min
        CreateDwordField(_CRS, MEM6._MAX, MAX6)  // Max
        CreateDwordField(_CRS, MEM6._LEN, LEN6)  // Memory length

        CreateQwordField(_CRS, MEM7._MIN, MIN7)  // Min
        CreateQwordField(_CRS, MEM7._MAX, MAX7)  // Max
        CreateQwordField(_CRS, MEM7._LEN, LEN7)  // Memory length

        //
        // Initialize _CRS.
        //

        Method(_INI, 0)
        {
            //
            // Update the resource descriptor with the low MMIO space gap.
            //

            Store(MG2B, MIN6)
            Store(MG2L, LEN6)
            Store(MG2L, Local0)
            Add(MIN6, Decrement(Local0), MAX6)

            //
            // Get the base and total length of the high MMIO gap in bytes.
            //

            ShiftLeft (HMIB, 20, Local1)
            ShiftLeft (HMIL, 20, Local2)

            Store(Local1, MIN7)
            Store(Local2, LEN7)
            Store(Local2, Local0)
            Add(MIN7, Decrement(Local0), MAX7)
        }

        //
        // VMBus device.
        //

        Device(VMBS)
        {
            Name(STA, 0xF)
            Name(_HID, "VMBus")
            Name(_UID, 0)
            Name(_DDN, "VMBUS")
            Method(_DIS, 0) { And(STA, 0xD, STA) }
            Method(_PS0, 0) { Or(STA, 0xF, STA) }
            Method(_STA, 0)
            {
                return(STA)
            }

            Name(_PS3, 0)
            Name(_CRS,

                //
                // Include an interrupt resource so that Linux VMs can get IDT
                // entries.
                //
                // N.B. All Windows VMs that support UEFI also support
                // getting IDT entries via other mechanisms, so this is not
                // necessary for Windows.
                //

                ResourceTemplate()
                {
                    IRQ(Edge,ActiveHigh,Exclusive) {5}
                }
            )
        }

        Device(APIC)
        {
            Name(_HID, EISAID("PNP0003"))
            Name(_CRS,
                ResourceTemplate()
                {
                    // I/O APIC
                    Memory32Fixed (ReadWrite, 0xfec00000, 0x1000)
                    // Local APIC
                    Memory32Fixed (ReadWrite, 0xfee00000, 0x1000)
                })
        }
    }

    //
    // Generation counter device.
    //

    Device(GENC)
    {
        Name(_CID, "VM_Gen_Counter")
        Name(_HID, "Hyper_V_Gen_Counter_V1")
        Name(_UID, 0)
        Name(_DDN, "VM_Gen_Counter")
        Method(ADDR, 0)
        {
            Name(LPKG, Package(){0, 0})
            Store(GCAL, Index(LPKG, 0))
            Store(GCAH, Index(LPKG, 1))
            Return(LPKG)
        }
    }
}

//
// General-purpose event scope.
//

Scope(\_GPE)
{
    //
    // The following code will cause the ACPI driver to arm "GPE input
    // pin 0" as an edge-triggered signal, to be dealt with whenever
    // an ACPI interrupt occurs.  When that input is found to be in the
    // pending state, as can be done from the worker process, the code
    // below will run within the ACPI interpreter.  This will allow
    // a driver on the GENC device (see above) to get a callback routine
    // invoked in response to the "Notify" statement below.  0x80 is
    // the first of the "vendor defined" codes for devices not in
    // any defined category.
    //

    Method(_E00)
    {
        Notify(\_SB.GENC, 0x80)
    }
}

