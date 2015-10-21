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
                //
                // Due to some compatibility issues, each QWORDMemory can only
                // define up to 2GB of memory, so use 8 and update them in
                // a loop in _INI below, based on information retrieved from
                // the worker process.
                //

                //  Granularity     Min     Max     Translation     Range (Length = Max-Min+1)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0xAAAAAAAAAA, 0xAAAAAAAAAB, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x7C80000000, 0x7CFFFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x7D00000000, 0x7D7FFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x7D80000000, 0x7DFFFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x7E00000000, 0x7E7FFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x7E80000000, 0x7EFFFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x7F00000000, 0x7F7FFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x7F80000000, 0x7FFFFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8000000000, 0x807FFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8080000000, 0x80FFFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8100000000, 0x817FFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8180000000, 0x81FFFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8200000000, 0x827FFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8280000000, 0x82FFFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8300000000, 0x837FFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8380000000, 0x83FFFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8400000000, 0x847FFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8480000000, 0x84FFFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8500000000, 0x857FFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8580000000, 0x85FFFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8600000000, 0x867FFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8680000000, 0x86FFFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8700000000, 0x877FFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8780000000, 0x87FFFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8800000000, 0x887FFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8880000000, 0x88FFFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8900000000, 0x897FFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8980000000, 0x89FFFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8A00000000, 0x8A7FFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8A80000000, 0x8AFFFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0x8B00000000, 0x8B7FFFFFFF, 0x0, 0x0080000000)
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00,     0xAAAAAAAAAA, 0xAAAAAAAAAB, 0x0, 0x0080000000)

                //
                // MMIO space below 4GB.
                //

                DWORDMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                // Granularity     Min     Max     Translation     Range (Length = Max-Min+1)
                   0x00,         0x00000, 0x00000,   0x00,         0x00000,,,
                   MEM6)   // Name declaration for this descriptor

            }
        )

        CreateDwordField(_CRS, MEM6._MIN, MIN6)  // Min
        CreateDwordField(_CRS, MEM6._MAX, MAX6)  // Max
        CreateDwordField(_CRS, MEM6._LEN, LEN6)  // Memory length

        //
        // Initialize _CRS.
        //

        Method(_INI, 0)
        {
            //
            // Update the resource descriptor with the first MMIO space gap.
            //

            Store(MG2B, MIN6)
            Store(MG2L, LEN6)
            Store(MG2L, Local0)
            Add(MIN6, Decrement(Local0), MAX6)

            //
            // Offset of header of next descriptor.
            //

            Store (0, Local0)

            //
            // Get the base and total length of the hole in bytes.
            //

            ShiftLeft (HMIB, 20, Local1)
            ShiftLeft (HMIL, 20, Local2)

            //
            // Loop while there are still descriptors to process. There
            // are 32 descriptors, and each are 0x2e bytes.
            //

            While (LLess(Local0, 0x5C0))
            {
                //
                // Update the descriptor and store the descriptor's length.
                //

                Store (UPDD (Local0, Local1, Local2), Local3)

                //
                // Update the descriptor offset, base address, and remaining
                // length.
                //

                Add (Local0, 0x2e, Local0)
                Add (Local1, Local3, Local1)
                Subtract (Local2, Local3, Local2)
            }
        }

        //
        // Updates a single memory descriptor with an address and length.
        //
        // Arg0 - Descriptor offset within _CRS.
        // Arg1 - Base address of memory region.
        // Arg2 - Total remaining length.
        //
        // Returns the actual length of the memory region.
        //

        Method(UPDD, 3)
        {
            //
            // Create fields for the type, min, max, and length.
            //

            CreateByteField (_CRS, Arg0, RTYP)
            CreateQWordField (_CRS, Add (Arg0, 0xe), RMIN)
            CreateQWordField (_CRS, Add (Arg0, 0x16), RMAX)
            CreateQWordField (_CRS, Add (Arg0, 0x26), RLEN)

            If (LEqual (Arg2, 0))
            {
                //
                // There is no more memory space. Replace type 0x8a (QWORDMemory)
                // with 0x84 (VendorLong).
                //

                Store (0x84, RTYP)
                Return (0)
            }
            Else
            {
                //
                // Store the minimum.
                //

                Store (Arg1, RMIN)

                //
                // Update the range's length if the remainder is less than
                // the range's preinitialized length (which is 2GB).
                //

                If (LLess (Arg2, RLEN))
                {
                    Store (Arg2, RLEN)
                }

                //
                // Compute the range's maximum.
                //

                Decrement (Arg1)
                Add (Arg1, RLEN, RMAX)

                //
                // Return the range length.
                //

                Return (RLEN)
            }
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

