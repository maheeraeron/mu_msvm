/*++

    Copyright (c) Microsoft Corporation

Module Name:

    Dsdt.asl

Abstract:

    ACPI DSDT table source

--*/

// Establish local define for architecture

#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)

#define _DSDT_INTEL_

#elif defined(MDE_CPU_AARCH64)

#define _DSDT_ARM_

#else

#error Unsupported Architecture!

#endif

// Define DSDT

DefinitionBlock (
    "Dsdt.aml",
    "DSDT",
    0x02,       // DSDT Compliance revision.
                // A Revision field value greater than or equal to 2 signifies that integers
                // declared within the Definition Block are to be evaluated as 64-bit values
    "MSFTVM",   // OEM ID (6 byte string)
    "DSDT01",   // OEM table ID  (8 byte string)
    0x01        // OEM version of DSDT table (4 byte Integer)
    )
{
    // The following operation region provides for a block of system memory
    // that can be referenced by this ASL code. The memory block contains
    // configuration parameters that are filled in at runtime by the UEFI C code in:
    // MsvmPkg/AcpiPlatformDxe/Dsdt.c DsdtAllocateAmlData. This definition of the
    // OperationRegion must match the signature in the above C code. As well
    // the definition of the fields must match the DSDT_AML_DATA struct in the above
    // C code.

    OperationRegion(BIOS, SystemMemory, 0xffffff00, 0xff)
    Field(BIOS, ByteAcc, NoLock, Preserve)
    {
        MG2B,32,        // Base of first MMIO region in bytes.
        MG2L,32,        // Length of first MMIO region in bytes.
        HMIB,32,        // Base of the second MMIO region in MB.
        HMIL,32,        // Length of the second MMIO region in MB.
        GCAL,32,        // Lower 32 bit address of Generation counter
        GCAH,32,        // Upper 32 bit address of Generation counter
        PCNT,32,        // Processor count
        NVDA,32,        // NVDIMM Method buffer address
        SCFG,8,         // Serial controllers enabled/disabled
        TCFG,8,         // TPM enabled/disabled
        PCFG,8,         // OEMP table load enabled/disabled
        HCFG,8,         // Hibernation enabled/disabled
        NCFG,8,         // PMEM (NVDIMMs) enabled/disabled
        BCFG,8,         // Virtual Battery enabled/disabled
        SGXE,8,         // SGX Memory enabled/disabled
        NCNT,16,        // NVDIMM count
    }

    // Supported machine sleep states =========================================

#if defined(_DSDT_INTEL_)

    // Define the S0 running state. This package is not used, but it is required
    // to exist by the specification.

    Name(\_S0, Package(2){0, 0})

    // Define the S5 powered off state. The first package value is the value to
    // write to PM1A_CNT.SLP_TYPE to cause the machine to power off. The second
    // value is for PM1B_CNT.SLP_TYPE, which is not supported by the Hyper-V
    // PM device.

    Name(\_S5, Package(2){0, 0})

    // Define the S4 hibernate state only if configured.

    If(LGreater(HCFG, 0))
    {
        Name(\_S4, Package(2){1, 0})
    }

#endif

    // VMOD ==================================================================

    // ACPI module device that holds the VMBus device. A module device is
    // useful here because Windows automatically provides a resource arbiter
    // for module devices that have resources. This allows VMBus's child devices
    // (including virtual PCI and its children) to claim memory resources
    // without VMBus having to provide its own arbiter implementation.
    // Without this, Windows looks for memory space on any ISA bus it can find,
    // which means that the PCI bus arbiter gets used. However, since the PCI
    // bus is typically not present in UEFI Hyper-V VMs, this is not an option.
    // An alternative would be to write an arbiter implementation for VMBus
    // but this would prevent inbox Windows 7 and Windows 8 VMBus drivers from
    // claiming memory space which would prevent synthetic video and SR-IOV
    // devices from working.

    Device(\_SB.VMOD)
    {
        Name(_HID, "ACPI0004")
        Name(_UID, 0)
        Name(_CRS,
            ResourceTemplate()
            {
                // MMIO space below 4GB.
                // TODO-cho: Technically this is now a lie on AARCH64 since we have a
                // tiny 1 page gap for the bios device. Should we be instead publishing
                // 3 MMIO regions then? Or is it okay since we have an ACPI device that
                // claims the bios device region?
                DWORDMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                // Granularity Min Max Translation Range (Length = Max-Min+1)
                   0,          0,  0,  0,          0,,,
                   MEM6)   // Name declaration for this descriptor
                // MMIO above 4GB
                QWORDMemory( ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                //  Granularity Min Max Translation Range (Length = Max-Min+1)
                    0,          0,  0,  0,          0,,,
                    MEM7)
            }
        )

        CreateDwordField(_CRS, MEM6._MIN, MIN6)  // Min
        CreateDwordField(_CRS, MEM6._MAX, MAX6)  // Max
        CreateDwordField(_CRS, MEM6._LEN, LEN6)  // Memory length

        CreateQwordField(_CRS, MEM7._MIN, MIN7)  // Min
        CreateQwordField(_CRS, MEM7._MAX, MAX7)  // Max
        CreateQwordField(_CRS, MEM7._LEN, LEN7)  // Memory length

        Method(_INI, 0)
        {
            // Update the DWORDMemory resource descriptor with the low MMIO region.
            Store(MG2B, MIN6)
            Store(MG2L, LEN6)
            Store(MG2L, Local0)
            Add(MIN6, Decrement(Local0), MAX6)

            // Update the QWORDMemory resource descriptor with the high MMIO region.
            ShiftLeft (HMIB, 20, Local1)
            ShiftLeft (HMIL, 20, Local2)
            Store(Local1, MIN7)
            Store(Local2, LEN7)
            Store(Local2, Local0)
            Add(MIN7, Decrement(Local0), MAX7)
        }
    }

    // BIOS Registers =========================================================

    Scope(\_SB)
    {

#if defined(_DSDT_INTEL_)

        OperationRegion(BIOB, SystemIO, FixedPcdGet32(PcdBiosBaseAddress), 0x8)

#elif defined(_DSDT_ARM_)

        OperationRegion(BIOB, SystemMemory, FixedPcdGet32(PcdBiosBaseAddress), 0x1000)

#endif

        Field (BIOB, DWordAcc, NoLock, Preserve)
        {
            BADR, 32,      // address
            BDAT, 32,      // data
        }
    }

    // APIC ===================================================================

#if defined(_DSDT_INTEL_)

    Device(\_SB.VMOD.APIC)
    {
        Name(_HID, EISAID("PNP0003"))
        Name(_CRS,
            ResourceTemplate()
            {
                // Local APIC - hard coded architectural address
                Memory32Fixed(ReadWrite, 0xfee00000, 0x1000)
                // I/O APIC - hard coded architectural address
                Memory32Fixed(ReadWrite, 0xfec00000, 0x1000)
            })
    }

#endif

    // Serial Ports =======================================================

    If(LGreater(SCFG, 0))
    {

#if defined(_DSDT_INTEL_)

        // COM1 (SerialControllerDevice.cpp).
        Device(\_SB.UAR1)
        {
            Name(_HID, EISAID("PNP0501")) // 16550A-compatible COM port
            Name(_DDN, "COM1")
            Name(_UID, 1)
            Name(_CRS, ResourceTemplate()
            {
                IO(Decode16, FixedPcdGet32(PcdCom1RegisterBase), FixedPcdGet32(PcdCom1RegisterBase), 1, 8)
                Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive)
                    {FixedPcdGet8(PcdCom1Vector)}
            })
        }

        // COM2 (SerialControllerDevice.cpp).
        Device(\_SB.UAR2)
        {
            Name(_HID, EISAID("PNP0501")) // 16550A-compatible COM port
            Name(_DDN, "COM2")
            Name(_UID, 2)
            Name(_CRS, ResourceTemplate()
            {
                IO(Decode16, FixedPcdGet32(PcdCom2RegisterBase), FixedPcdGet32(PcdCom2RegisterBase), 1, 8)
                Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive)
                    {FixedPcdGet8(PcdCom2Vector)}
            })
        }

#elif defined(_DSDT_ARM_)

        // COM1 (PL011Device.cpp)
        Device(\_SB.VMOD.UAR1)
        {
            Name(_HID, "ARMH0011") // ARM SBSA PL011 UART
            Name(_DDN, "COM1")
            Name(_UID, 1)
            Name(_CRS, ResourceTemplate()
            {
                Memory32Fixed(ReadWrite, FixedPcdGet32(PcdCom1RegisterBase), 0x1000)
                Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive)
                    {FixedPcdGet8(PcdCom1Vector)}
            })
        }

        // COM2 (PL011Device.cpp)
        Device(\_SB.VMOD.UAR2)
        {
            Name(_HID, "ARMH0011") // ARM SBSA PL011 UART
            Name(_DDN, "COM2")
            Name(_UID, 2)
            Name(_CRS, ResourceTemplate()
            {
                Memory32Fixed(ReadWrite, FixedPcdGet32(PcdCom2RegisterBase), 0x1000)
                Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive)
                    {FixedPcdGet8(PcdCom2Vector)}
            })
        }

#endif

    }

    // VMBus ==================================================================

    Device(\_SB.VMOD.VMBS)
    {
        Name(STA, 0xF)
        Name(_ADR, 0x00)
        Name(_DDN, "VMBUS")
        Name(_HID, "VMBus")
        Name(_UID, 0)
        Method(_DIS, 0) { And(STA, 0xD, STA) }
        Method(_PS0, 0) { Or(STA, 0xF, STA) }
        Method(_STA, 0)
        {
            return(STA)
        }

        Name(_PS3, 0)

        // TODO-cho: SPIs are not available to the guest on AARCH64, which is what
        // PcdVmbusVector is currently defined as. Supposedly it should use a PPI,
        // but those are strange because they're reserved for hypervisor devices.
        //
        // Windows doesn't boot when VmBus is given an SPI, since it's unable to
        // allocate any since none exist in guests. Thus, leave it out on AARCH64
        // for now.
        //
        // Linux may need this field if it's not hardcoded, unsure.
        //
        // Additionally, no Interrupt-Signaled event devices currently work either,
        // due to SPIs not being available to guests.
        Name(_CRS,

            // Include an interrupt resource so that Linux VMs can get IDT
            // entries.
            //
            // N.B. All Windows VMs that support UEFI also support
            // getting IDT entries via other mechanisms, so this is not
            // necessary for Windows.

            ResourceTemplate()
            {

#if defined(_DSDT_INTEL_)

                // Older Linux kernels like RHEL/CentOS don't seem to be able to
                // parse the new Extended Interrupt Descriptor resource type (see ACPI Section 6.4.3.6),
                // so we instead use the old legacy IRQ description which
                // becomes the short form of Interrupt Descriptor (ACPI Section 5.4.2.1)
                // which only supports legacy PIC devices to describe up to 15
                // interrupts. VMBUS is interrupt 5 on X64, so this is okay.
                IRQ(Edge,ActiveHigh,Exclusive)
                    {FixedPcdGet8(PcdVmbusVector)}

#else
                // Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive)
                //    {FixedPcdGet8(PcdVmbusVector)}
                //
                // TODO-cho: Include a dummy resource so this device has a _CRS
                // until interrupts are figured out.
                VendorShort() { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }

#endif
            }
        )
    }

    // TPM ====================================================================

#if defined(_DSDT_INTEL_)

    If(LGreater(TCFG, 0))
    {
        Device(\_SB.VMOD.TPM2)
        {
            Name(_ADR, 0x00)
            Name(_HID, "VTPM0101")
            Name(_CID, "MSFT0101")
            Name(_UID, 0x01)
            Name(_DDN, "Microsoft Virtual TPM 2.0")
            Name(_STR, Unicode ("Microsoft Virtual TPM 2.0"))
            Name(_CRS, ResourceTemplate () {
                Memory32Fixed(ReadWrite, 0xfed40000, 0x1000)   // TODO: Use PCDs
            })
            Method (_STA, 0, NotSerialized)
            {
                Return (0x0F)
            }

            // Operational region for TPM general IO port access
            // TODO: Need MMIO for ARM
            OperationRegion (GIO, SystemIO, 0x1040, 8)
            Field (GIO, DWordAcc, NoLock, Preserve)
            {
                CTLP, 32,      // 32-bit control port
                DATP, 32,      // 32-bit data port
            }

            // TCG Physical Presence Interface
            Method (TPPI, 2, Serialized)
            {
                Name(PKG2, Package(){0, 0})
                Name(PKG3, Package(){0, 0, 0})

                // Switch by function index
                Switch (ToInteger(Arg0))
                {
                    Case (0)
                    {
                        // Func 0 - Standard query, supports function 1-8
                        Return (Buffer () {0xFF, 0x01})
                    }
                    Case (1)
                    {
                        // Func 1 - Get Physical Presence Interface Version
                        Return ("1.3")
                    }
                    Case (2)
                    {
                        // Func 2 - Submit TPM Operation Request to Pre-OS Environment (Deprecated, Not implemented)
                        Return (3)
                    }
                    Case (3)
                    {
                        // Func 3 - Get Pending TPM Operation Requested By the OS
                        // Process the request in vDev. IO command is identical to Function Id
                        Store (3, CTLP)
                        Store (0, Index (PKG2, 0))
                        Store (DATP, Index (PKG2, 1))
                        Return (PKG2)
                    }
                    Case (4)
                    {
                        // Func 4 - Get Platform-Specific Action to Transition to Pre-OS Environment
                        // Return reboot.
                        Return (2)
                    }
                    Case (5)
                    {
                        // Func 5 - Return TPM Operation Response to OS Environment
                        // Process the request in vDev. IO command is identical to Function Id
                        // Get operation value
                        Store (5, CTLP)
                        Store (DATP, Index (PKG3, 1))
                        // Get operation result
                        Store (6, CTLP)
                        Store (DATP, Index (PKG3, 2))
                        // Set succeed
                        Store (0, Index (PKG3, 0))
                        Return (PKG3)
                    }
                    Case (6)
                    {
                        // Func 6 - Submit preferred user language (Not implemented)
                        Return (3)
                    }
                    Case (7)
                    {
                        // Func 7 - Submit TPM Operation Request to Pre-OS Environment 2
                        // Process the request in vDev. IO command is identical to Function Id
                        Store (7, CTLP)
                        Store (DerefOf (Index (Arg1, 0)), DATP)
                        Return (DATP)
                    }
                    Case (8)
                    {
                        // Func 8 - Get User Confirmation Status for Operation
                        // Process in vDev. IO command is identical to Function Id
                        Store (8, CTLP)
                        Store (DerefOf (Index (Arg1, 0)), DATP)
                        Return (DATP)
                    }
                }
                Return (1)
            }

            Method (TMCI, 2, Serialized)
            {
                // Switch by function index
                Switch (ToInteger (Arg0))
                {
                    Case (0)
                    {
                        // Standard query, supports function 1-1
                        Return (Buffer () {0x03})
                    }
                    Case (1)
                    {
                        Store (0x31, BADR) // 0x32 is BiosConfigMorSetVariable
                        Store (DerefOf (Index (Arg1, 0)), BDAT)
                        return (BDAT)
                    }
                }
                Return (1)
            }

            Method (_DSM, 4, Serialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                // TCG Physical Presence Interface
                If (LEqual(Arg0, ToUUID ("3dddfaa6-361b-4eb4-a424-8d10089d1653")))
                {
                    Return (TPPI(Arg2, Arg3))
                }

                // TCG Memory Clear Interface
                If (LEqual(Arg0, ToUUID ("376054ed-cc13-4675-901c-4756d7f2d45d")))
                {
                    Return (TMCI (Arg2, Arg3))
                }

                // If not one of the function identifiers we recognize, then return a buffer
                // with bit 0 set to 0 indicating no functions supported.
                Return (Buffer () {0})
            }
        }
    }

#endif

    // SGX ====================================================================

    // The Enclave Page Cache aka SGX memory device. This is intentionally not
    // Intel spec compliant in that it doesn't have any memory regions described
    // in the _CRS. Existence of this device will trigger a guest kernel to load
    // a device driver. That device driver will use other mechanisms (cpuid) to
    // discover the SGX memory regions.

#if defined (_DSDT_INTEL_)

    If(LGreater(SGXE, 0))
    {
        Device(\_SB.EPC)
        {
            Name(_HID, EISAID("INT0E0C"))
            Name(_STR, Unicode ("Enclave Page Cache 1.0"))
            Name(_CRS, ResourceTemplate()
            {
                // This is dummy data to make the _CRS not empty.
                VendorShort() { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
            })
            Method(_STA, 0x0)
            {
                Return(0xF)
            }
        }
    }

#endif

    // Generation Counter =====================================================

    Device(\_SB.GENC)
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

#if defined(_DSDT_INTEL_)


    // GPE method for generation counter
    Scope(\_GPE)
    {
        // Method for notifying external changes to the generation counter:
        //      E  - This event is edge triggered
        //      00 - Use bit 0 in the General Purpose Event register described
        //           in the FADT
        Method(_E00)
        {
            Notify(\_SB.GENC, 0x80)
        }
    }

#elif defined(_DSDT_ARM_)

    // Interrupt signalled event for generation counter
    Device (\_SB.GED1)
    {
        Name(_HID, "ACPI0013")
        Name(_UID, 1)
        Name(_CRS,
            ResourceTemplate()
            {
                Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive)
                    {FixedPcdGet8(PcdGenCountEventVector)}
            }
        )
        Method(_EVT, 1)
        {
            Notify(\_SB.GENC, 0x80) // counter value changed event
        }
    }
#endif

    // RealTime Clock (RTC) ===================================================

#if defined(_DSDT_INTEL_)
    Device(\_SB.RTC0)
    {
        Name(_HID, EISAID("PNP0B00")) // AT real-time clock
        Name(_UID, 0)
        Name(_CRS, ResourceTemplate()
        {
            IO(Decode16, FixedPcdGet32(PcdRtcRegisterBase), FixedPcdGet32(PcdRtcRegisterBase), 0, 0x2)
            Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) {FixedPcdGet8(PcdRtcVector)}
        })
    }
#endif

    // Battery ================================================================

    If(LGreater(BCFG, 0))
    {

        // MMIO region for Virtual Battery
        OperationRegion(BATM, SystemMemory, FixedPcdGet32(PcdBatteryBase), 0x1000)
        Field(BATM, DWordAcc, NoLock, WriteAsZeros)
        {
            BSTA,32, // Battery Status
            BSTE,32, // Battery State
            BRAT,32, // Battery Present Rate
            BCAP,32, // Battery Remaining Capacity
            ACPS,32, // AC Adapter PSR Status
            BNST,32, // Battery Notify Status
            BNCL,32  // Battery Notify Clear
        }

        Device(\_SB.VMOD.BAT1)
        {
            Name(BIX, Package () {
                0, // Revision - 0
                0x0, // Power unit - 0 is mWh
                5000, // Design Capacity - 5000 mWh
                5000, // Last Full Charge Capacity - 5000 mWh
                0x00000001, // Battery Technology - Secondary (Rechargeable)
                0x1388, // Design Voltage - 5 Volts
                500, // Design capacity of warning - 10%
                250, // Design capacity of low - 5%
                0xFFFFFFFF, // Cycle count - Unknown
                10000, // Measurement Accuracy - 100%
                0xFFFFFFFF, // Max Sampling Time - Unknown
                0xFFFFFFFF, // Min Sampling Time - Unknown
                1000, // Max Averaging Internval - 1000ms
                100, // Min Averaging
                10, // Battery Capacity Granularity 1
                10, // Battery Capacity Granularity 2
                "Microsoft Hyper-V Virtual Battery",
                "Virtual",
                "Virtual Battery",
                ""
            })

            Name(BIXE, Package () {
                0, // Revision - 0
                0x0, // Power unit - 0 is mWh
                0xFFFFFFFF, // Design Capacity - Unknown
                0xFFFFFFFF, // Last Full Charge Capacity - Unknown
                0x00000001, // Battery Technology - Secondary (Rechargeable)
                0xFFFFFFFF, // Design Voltage - Unknown
                0, // Design capacity of warning - 10%
                0, // Design capacity of low - 5%
                0xFFFFFFFF, // Cycle count - Unknown
                10000, // Measurement Accuracy - 100%
                0xFFFFFFFF, // Max Sampling Time - Unknown
                0xFFFFFFFF, // Min Sampling Time - Unknown
                1000, // Max Averaging Internval - 1000ms
                100, // Min Averaging
                10, // Battery Capacity Granularity 1
                10, // Battery Capacity Granularity 2
                "",
                "",
                "",
                ""
            })

            Name(_CID, "Virtual Battery")
            Name(_HID, "PNP0C0A")
            Method(_BIX, 0)
            {
                // Check if battery is present
                If (LEqual(BSTA, 0x1F))
                {
                    // If so return BIX with battery static capacity info
                    Return(BIX)
                }
                Else
                {
                    // Otherwise return battery empty BIX
                    Return(BIXE)
                }
            }

            Method(_BST, 0)
            {
                Name(BST, Package () {
                    0x0, // Battery State
                    0x0, // Battery Present Rate (discharge rate)
                    0x0, // Battery Remaining Capacity
                    0x1388  // Battery Present Voltage - 5 volts
                })

                Store(BSTE, Index(BST, 0))
                Store(BRAT, Index(BST, 1))
                Store(BCAP, Index(BST, 2))

                // Check if battery isn't present
                If (LEqual(BSTA, 0xF))
                {
                    // If so, report unknown voltage
                    Store(0xFFFFFFFF, Index(BST, 3))
                }

                Return(BST)
            }

            Method(_STA, 0)
            {
                Return(BSTA)
            }

            Name(_PCL, Package() {
                \_SB // All nodes under SB are powered by this device
            })
        }

        Device(\_SB.VMOD.AC1)
        {
            Name(_HID, "ACPI0003")
            Name(_CID, "Virtual AC Adapter")

            Name(_PCL, Package () {
                \_SB // All nodes under SB are powered by this device
            })

            Method(_PSR, 0)
            {
                Return(ACPS)
            }
        }

#if defined (_DSDT_INTEL_)

        // GPE method for battery
        Scope(\_GPE)
        {
            // Method for notifying external changes to the battery device:
            //      E  - This event is edge triggered
            //      09 - Use bit 9 in the General Purpose Event register described
            //           in the FADT
            Method(_E09)
            {

#elif defined (_DSDT_ARM_)

        // Interrupt signalled event device for battery
        Device(\_SB.GED2)
        {
            Name(_HID,"ACPI0013")
            Name(_UID, 2)
            Name(_CRS, ResourceTemplate()
            {
                Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive)
                    {FixedPcdGet32(PcdBatteryEventVector)}
            })
            Method(_EVT, 1)
            {

#endif

                // Read battery notify type into local
                Store(BNST, Local0)

                // Break status into different bits
                // Local1 is 0x80 status at bit 0
                // Local2 is 0x81 status at bit 1
                And(Local0, 0x1, Local1)
                And(Local0, 0x2, Local2)

                // For Windows and Linux, sending a notify of 0x81 forces a recheck
                // of both _BIX and _BST. Thus we can let Notify 0x81 take priority
                // if we see both bits set.
                If (LEqual(Local2, 0x2))
                {
                    // Note that since 0x81 takes priority, force a recheck of AC
                    // _PSR as well.
                    Notify(\_SB.VMOD.BAT1, 0x81)
                    Notify(\_SB.VMOD.AC1, 0x80)
                }
                ElseIf(LEqual(Local1, 0x1))
                {
                    // Notify OSPM that both battery _BST and AC _PSR have changed
                    Notify(\_SB.VMOD.BAT1, 0x80)
                    Notify(\_SB.VMOD.AC1, 0x80)
                }

                // Clear whatever bits we read as set by writing to the clear
                // location
                Store(Local0, BNCL)
            }
        }
    }

    // NVDIMM root and child devices

    If(LGreater(NCFG, 0))
    {
        Device(\_SB.NVDR)
        {
            // 4K Operation Region used for MMIO to signal to the host vdev that
            // a method is being called. We split up the MMIO region into offsets
            // to indicate to the host which method is being called on which device.
            OperationRegion(NVIO, SystemMemory, FixedPcdGet32(PcdPmemRegisterBase), 4096)
            Field(NVIO, DWordAcc, NoLock, WriteAsZeros)
            {
                RDSM,32, // Root  _DSM
                CDSM,32, // Child _DSM
                CLSI,32, // Child _LSI
                CLSR,32, // Child _LSR
                NEV0,32, // NEV# is a NVDIMM Event Register.
                NEV1,32, // NEV0 - NEV3 are NFIT Health Event Notifications
                NEV2,32, //                 (0x81 on NVDIMM Child Device)
                NEV3,32, //
                NEV4,32, // NVDIMM Root Device notifications.
            }

            // 4K Operation Region for Method I/O buffer between the ACPI NVDIMM
            // devices and the vPMEM vdev on the Host. The actual address (NVDA)
            // comes from the BIOS operation region that gets updated from by the
            // UEFI firmware during ACPI table initialization.
            OperationRegion(NVDB, SystemMemory, NVDA, 4096)
            Field(NVDB, AnyAcc, NoLock, WriteAsZeros)
            {
                MBUF,32736, // Raw buffer that can be returned to callers (4k bytes - 4).
                MBFL,32,    // Size of the data that has been returned.
            }

            Name (_HID, "ACPI0012")
            Name (_STA, 0xF)

            // This mutex protects the above NVDB OperationRegion.
            // It should be used as follows.
            // 1. Acquire NMTX
            // 2. Store method arguments in MBUF.
            // 3. Signal the vdev via the NVIO MMIO page.
            // 4. Read the return values from MBUF into scratch space (of length MBFL).
            // 5. Release NMTX.
            Mutex(NMTX, 0)


            // _DSM Device Specific Method
            // Arg0: UUID Unique function identifier
            // Arg1: Integer Revision Level
            // Arg2: Integer Function Index (0 = Return Supported Functions)
            // Arg3: Package Parameters
            Method (_DSM, 4, Serialized, 0, {IntObj,BuffObj}, )
            {
                Switch (ToBuffer(Arg0))
                {
                    Case (ToUUID("2f10e7a4-9e91-11e4-89d3-123b93f75cba"))
                    {
                        Switch (ToInteger(Arg1))
                        {
                            Case (1)
                            {
                                Switch (ToInteger(Arg2))
                                {
                                    // Function 0: Return supported functions.
                                    Case (0)
                                    {
                                        // Read from the MMIO page to get the supported functions.
                                        Return (RDSM)
                                    }
                                    // For all other functions, call into the vdev.
                                    Default
                                    {
                                        Acquire (NMTX, 0xFFFF)

                                        // Copy the arguments into the method I/O buffer.
                                        Store (DeRefOf(Index(Arg3,0)), MBUF)

                                        // Write the function index to the MMIO Page
                                        // to signal the vdev.
                                        Store (Arg2, RDSM)

                                        // Copy the contents of the method I/O Buffer.
                                        Name (RBUF, Buffer(MBFL) {})
                                        Multiply (MBFL, 8, Local0)
                                        Store (MBUF, RBUF)
                                        CreateField (RBUF, 0, Local0, RBFF)

                                        Release (NMTX)
                                        Return (RBFF)
                                    }
                                }
                            }
                            Default
                            {
                                // Return a buffer with bit 0 set to 0 indicating no functions supported
                                // if we don't recognize the revision level.
                                Return (Buffer(){0})
                            }
                        }
                    }
                }

                // Return a buffer with bit 0 set to 0 indicating no functions supported
                // if we don't recognize the UUID.
                Return (Buffer(){0})
            }

            // CDSF - Generic Method for Child _DSMs.
            // Arg0: UUID Unique function identifier
            // Arg1: Integer Revision Level
            // Arg2: Integer Function Index (0 = Return Supported Functions)
            // Arg3: Package Parameters
            // Arg4: Integer Device Index
            Method (CDSF, 5, Serialized, 0, {IntObj,BuffObj})
            {
                Switch (ToBuffer(Arg0))
                {
                    Case (ToUUID("5746C5F2-A9A2-4264-AD0E-E4DDC9E09E80"))
                    {
                        Switch (ToInteger(Arg1))
                        {
                            Case (1)
                            {
                                Switch (ToInteger(Arg2))
                                {
                                    // Function 0: Return supported functions.
                                    Case (0)
                                    {
                                        // Read from the MMIO page to get the supported functions.
                                        Return (CDSM)
                                    }
                                    // For all other functions, call into the vdev.
                                    Default
                                    {
                                        // We need to pack the function index and device index into a DWORD.
                                        Name (INDX, Buffer(4) {})
                                        CreateField (INDX, 0, 16, FIND) // Space for Function Index
                                        CreateField(INDX, 16, 16, DIND) // Space for Device Index
                                        Store (Arg2, FIND)
                                        Store (Arg4, DIND)

                                        Acquire (NMTX, 0xFFFF)

                                        // Copy the arguments into the method I/O buffer.
                                        Store (DeRefOf(Index(Arg3,0)), MBUF)

                                        // Write the function and device indices
                                        // to the MMIO Page to signal the vdev.
                                        Store (INDX, CDSM)

                                        // Copy the contents of the method I/O Buffer.
                                        Name (RBUF, Buffer(MBFL) {})
                                        Multiply (MBFL, 8, Local0)
                                        Store (MBUF, RBUF)
                                        CreateField (RBUF, 0, Local0, RBFF)

                                        Release (NMTX)
                                        Return (RBFF)
                                    }
                                }
                            }
                            Default
                            {
                                // Return a buffer with bit 0 set to 0 indicating no functions supported
                                // if we don't recognize the revision level.
                                Return (Buffer(){0})
                            }
                        }
                    }
                }

                // Return a buffer with bit 0 set to 0 indicating no functions supported
                // if we don't recognize the UUID.
                Return (Buffer(){0})
            }

            // LSIM - Generic Method for Child _LSIs.
            //
            // Arg0: Integer Device Index
            Method (LSIM, 1, Serialized, 0, {PkgObj})
            {
                Acquire (NMTX, 0xFFFF)

                // Write the device index
                // to the MMIO Page to signal the vdev.
                Store (Arg0, CLSI)

                // Copy the contents of the method I/O Buffer.
                Name (RBUF, Buffer(MBFL) {})
                Store (MBUF, RBUF)

                Release (NMTX)

                CreateDWordField (RBUF, 0, DWD0)
                CreateDWordField (RBUF, 4, DWD1)
                CreateDWordField (RBUF, 8, DWD2)
                Name (PKGI, Package(3) {0, 0, 0})
                Store (DWD0, Index(PKGI, 0))
                Store (DWD1, Index(PKGI, 1))
                Store (DWD2, Index(PKGI, 2))
                Return (PKGI)
            }

            // LSRM - Generic Method for Child _LSRs.
            // Arg0: Integer(DWORD) Byte Offset.
            // Arg1: Integer(DWORD) Tranfer Byte Length.
            // Arg2: Integer Device Index.
            Method (LSRM, 3, Serialized, 0, {PkgObj})
            {
                // Pack up the arguments.
                // Make INPT the same size as MBUF, so ASL does not do source
                // and destination type conversion, to guarantee that the resulting
                // store copies INPT byte for byte into MBUF.
                Name (INPT, Buffer(0xffc) {})
                CreateField (INPT, 0, 32, BTOF) // Space for Byte Offset
                CreateField (INPT, 32, 32, TFLT) // Space for Transfer Length
                Store (Arg0, BTOF)
                Store (Arg1, TFLT)

                Acquire (NMTX, 0xFFFF)

                // Copy the arguments.
                Store (INPT, MBUF)

                // Write the device index
                // to the MMIO Page to signal the vdev.
                Store (Arg2, CLSR)

                // Copy the contents of the method I/O Buffer.
                Name (RBUF, Buffer(MBFL) {})
                Multiply (MBFL, 8, Local0)
                Store (MBUF, RBUF)

                Release (NMTX)

                CreateDWordField (RBUF, 0, DWD0)
                Store (Subtract(Local0, 32), Local1) // size of the data buffer in bits
                CreateField (RBUF, 32, Local1, LBLD) // label data buffer
                Name (PKGR, Package(2) {0, Buffer(){0}})
                Store (DWD0, Index (PKGR, 0))
                Store (LBLD, Index (PKGR, 1))
                Return (PKGR)
            }
        }

        // NVDIMM Child Devices

        If(LLessEqual(1, NCNT))
        {
            Device(\_SB.NVDR.N000)
            {
                Name(_ADR, 0)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(2, NCNT))
        {
            Device(\_SB.NVDR.N001)
            {
                Name(_ADR, 1)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(3, NCNT))
        {
            Device(\_SB.NVDR.N002)
            {
                Name(_ADR, 2)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(4, NCNT))
        {
            Device(\_SB.NVDR.N003)
            {
                Name(_ADR, 3)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(5, NCNT))
        {
            Device(\_SB.NVDR.N004)
            {
                Name(_ADR, 4)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(6, NCNT))
        {
            Device(\_SB.NVDR.N005)
            {
                Name(_ADR, 5)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(7, NCNT))
        {
            Device(\_SB.NVDR.N006)
            {
                Name(_ADR, 6)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(8, NCNT))
        {
            Device(\_SB.NVDR.N007)
            {
                Name(_ADR, 7)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(9, NCNT))
        {
            Device(\_SB.NVDR.N008)
            {
                Name(_ADR, 8)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(10, NCNT))
        {
            Device(\_SB.NVDR.N009)
            {
                Name(_ADR, 9)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(11, NCNT))
        {
            Device(\_SB.NVDR.N010)
            {
                Name(_ADR, 10)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(12, NCNT))
        {
            Device(\_SB.NVDR.N011)
            {
                Name(_ADR, 11)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(13, NCNT))
        {
            Device(\_SB.NVDR.N012)
            {
                Name(_ADR, 12)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(14, NCNT))
        {
            Device(\_SB.NVDR.N013)
            {
                Name(_ADR, 13)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(15, NCNT))
        {
            Device(\_SB.NVDR.N014)
            {
                Name(_ADR, 14)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(16, NCNT))
        {
            Device(\_SB.NVDR.N015)
            {
                Name(_ADR, 15)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(17, NCNT))
        {
            Device(\_SB.NVDR.N016)
            {
                Name(_ADR, 16)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(18, NCNT))
        {
            Device(\_SB.NVDR.N017)
            {
                Name(_ADR, 17)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(19, NCNT))
        {
            Device(\_SB.NVDR.N018)
            {
                Name(_ADR, 18)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(20, NCNT))
        {
            Device(\_SB.NVDR.N019)
            {
                Name(_ADR, 19)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(21, NCNT))
        {
            Device(\_SB.NVDR.N020)
            {
                Name(_ADR, 20)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(22, NCNT))
        {
            Device(\_SB.NVDR.N021)
            {
                Name(_ADR, 21)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(23, NCNT))
        {
            Device(\_SB.NVDR.N022)
            {
                Name(_ADR, 22)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(24, NCNT))
        {
            Device(\_SB.NVDR.N023)
            {
                Name(_ADR, 23)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(25, NCNT))
        {
            Device(\_SB.NVDR.N024)
            {
                Name(_ADR, 24)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(26, NCNT))
        {
            Device(\_SB.NVDR.N025)
            {
                Name(_ADR, 25)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(27, NCNT))
        {
            Device(\_SB.NVDR.N026)
            {
                Name(_ADR, 26)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(28, NCNT))
        {
            Device(\_SB.NVDR.N027)
            {
                Name(_ADR, 27)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(29, NCNT))
        {
            Device(\_SB.NVDR.N028)
            {
                Name(_ADR, 28)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(30, NCNT))
        {
            Device(\_SB.NVDR.N029)
            {
                Name(_ADR, 29)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(31, NCNT))
        {
            Device(\_SB.NVDR.N030)
            {
                Name(_ADR, 30)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(32, NCNT))
        {
            Device(\_SB.NVDR.N031)
            {
                Name(_ADR, 31)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(33, NCNT))
        {
            Device(\_SB.NVDR.N032)
            {
                Name(_ADR, 32)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(34, NCNT))
        {
            Device(\_SB.NVDR.N033)
            {
                Name(_ADR, 33)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(35, NCNT))
        {
            Device(\_SB.NVDR.N034)
            {
                Name(_ADR, 34)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(36, NCNT))
        {
            Device(\_SB.NVDR.N035)
            {
                Name(_ADR, 35)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(37, NCNT))
        {
            Device(\_SB.NVDR.N036)
            {
                Name(_ADR, 36)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(38, NCNT))
        {
            Device(\_SB.NVDR.N037)
            {
                Name(_ADR, 37)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(39, NCNT))
        {
            Device(\_SB.NVDR.N038)
            {
                Name(_ADR, 38)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(40, NCNT))
        {
            Device(\_SB.NVDR.N039)
            {
                Name(_ADR, 39)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(41, NCNT))
        {
            Device(\_SB.NVDR.N040)
            {
                Name(_ADR, 40)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(42, NCNT))
        {
            Device(\_SB.NVDR.N041)
            {
                Name(_ADR, 41)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(43, NCNT))
        {
            Device(\_SB.NVDR.N042)
            {
                Name(_ADR, 42)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(44, NCNT))
        {
            Device(\_SB.NVDR.N043)
            {
                Name(_ADR, 43)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(45, NCNT))
        {
            Device(\_SB.NVDR.N044)
            {
                Name(_ADR, 44)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(46, NCNT))
        {
            Device(\_SB.NVDR.N045)
            {
                Name(_ADR, 45)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(47, NCNT))
        {
            Device(\_SB.NVDR.N046)
            {
                Name(_ADR, 46)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(48, NCNT))
        {
            Device(\_SB.NVDR.N047)
            {
                Name(_ADR, 47)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(49, NCNT))
        {
            Device(\_SB.NVDR.N048)
            {
                Name(_ADR, 48)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(50, NCNT))
        {
            Device(\_SB.NVDR.N049)
            {
                Name(_ADR, 49)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(51, NCNT))
        {
            Device(\_SB.NVDR.N050)
            {
                Name(_ADR, 50)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(52, NCNT))
        {
            Device(\_SB.NVDR.N051)
            {
                Name(_ADR, 51)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(53, NCNT))
        {
            Device(\_SB.NVDR.N052)
            {
                Name(_ADR, 52)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(54, NCNT))
        {
            Device(\_SB.NVDR.N053)
            {
                Name(_ADR, 53)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(55, NCNT))
        {
            Device(\_SB.NVDR.N054)
            {
                Name(_ADR, 54)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(56, NCNT))
        {
            Device(\_SB.NVDR.N055)
            {
                Name(_ADR, 55)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(57, NCNT))
        {
            Device(\_SB.NVDR.N056)
            {
                Name(_ADR, 56)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(58, NCNT))
        {
            Device(\_SB.NVDR.N057)
            {
                Name(_ADR, 57)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(59, NCNT))
        {
            Device(\_SB.NVDR.N058)
            {
                Name(_ADR, 58)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(60, NCNT))
        {
            Device(\_SB.NVDR.N059)
            {
                Name(_ADR, 59)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(61, NCNT))
        {
            Device(\_SB.NVDR.N060)
            {
                Name(_ADR, 60)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(62, NCNT))
        {
            Device(\_SB.NVDR.N061)
            {
                Name(_ADR, 61)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(63, NCNT))
        {
            Device(\_SB.NVDR.N062)
            {
                Name(_ADR, 62)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(64, NCNT))
        {
            Device(\_SB.NVDR.N063)
            {
                Name(_ADR, 63)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(65, NCNT))
        {
            Device(\_SB.NVDR.N064)
            {
                Name(_ADR, 64)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(66, NCNT))
        {
            Device(\_SB.NVDR.N065)
            {
                Name(_ADR, 65)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(67, NCNT))
        {
            Device(\_SB.NVDR.N066)
            {
                Name(_ADR, 66)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(68, NCNT))
        {
            Device(\_SB.NVDR.N067)
            {
                Name(_ADR, 67)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(69, NCNT))
        {
            Device(\_SB.NVDR.N068)
            {
                Name(_ADR, 68)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(70, NCNT))
        {
            Device(\_SB.NVDR.N069)
            {
                Name(_ADR, 69)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(71, NCNT))
        {
            Device(\_SB.NVDR.N070)
            {
                Name(_ADR, 70)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(72, NCNT))
        {
            Device(\_SB.NVDR.N071)
            {
                Name(_ADR, 71)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(73, NCNT))
        {
            Device(\_SB.NVDR.N072)
            {
                Name(_ADR, 72)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(74, NCNT))
        {
            Device(\_SB.NVDR.N073)
            {
                Name(_ADR, 73)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(75, NCNT))
        {
            Device(\_SB.NVDR.N074)
            {
                Name(_ADR, 74)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(76, NCNT))
        {
            Device(\_SB.NVDR.N075)
            {
                Name(_ADR, 75)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(77, NCNT))
        {
            Device(\_SB.NVDR.N076)
            {
                Name(_ADR, 76)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(78, NCNT))
        {
            Device(\_SB.NVDR.N077)
            {
                Name(_ADR, 77)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(79, NCNT))
        {
            Device(\_SB.NVDR.N078)
            {
                Name(_ADR, 78)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(80, NCNT))
        {
            Device(\_SB.NVDR.N079)
            {
                Name(_ADR, 79)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(81, NCNT))
        {
            Device(\_SB.NVDR.N080)
            {
                Name(_ADR, 80)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(82, NCNT))
        {
            Device(\_SB.NVDR.N081)
            {
                Name(_ADR, 81)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(83, NCNT))
        {
            Device(\_SB.NVDR.N082)
            {
                Name(_ADR, 82)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(84, NCNT))
        {
            Device(\_SB.NVDR.N083)
            {
                Name(_ADR, 83)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(85, NCNT))
        {
            Device(\_SB.NVDR.N084)
            {
                Name(_ADR, 84)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(86, NCNT))
        {
            Device(\_SB.NVDR.N085)
            {
                Name(_ADR, 85)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(87, NCNT))
        {
            Device(\_SB.NVDR.N086)
            {
                Name(_ADR, 86)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(88, NCNT))
        {
            Device(\_SB.NVDR.N087)
            {
                Name(_ADR, 87)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(89, NCNT))
        {
            Device(\_SB.NVDR.N088)
            {
                Name(_ADR, 88)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(90, NCNT))
        {
            Device(\_SB.NVDR.N089)
            {
                Name(_ADR, 89)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(91, NCNT))
        {
            Device(\_SB.NVDR.N090)
            {
                Name(_ADR, 90)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(92, NCNT))
        {
            Device(\_SB.NVDR.N091)
            {
                Name(_ADR, 91)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(93, NCNT))
        {
            Device(\_SB.NVDR.N092)
            {
                Name(_ADR, 92)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(94, NCNT))
        {
            Device(\_SB.NVDR.N093)
            {
                Name(_ADR, 93)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(95, NCNT))
        {
            Device(\_SB.NVDR.N094)
            {
                Name(_ADR, 94)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(96, NCNT))
        {
            Device(\_SB.NVDR.N095)
            {
                Name(_ADR, 95)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(97, NCNT))
        {
            Device(\_SB.NVDR.N096)
            {
                Name(_ADR, 96)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(98, NCNT))
        {
            Device(\_SB.NVDR.N097)
            {
                Name(_ADR, 97)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(99, NCNT))
        {
            Device(\_SB.NVDR.N098)
            {
                Name(_ADR, 98)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(100, NCNT))
        {
            Device(\_SB.NVDR.N099)
            {
                Name(_ADR, 99)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(101, NCNT))
        {
            Device(\_SB.NVDR.N100)
            {
                Name(_ADR, 100)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(102, NCNT))
        {
            Device(\_SB.NVDR.N101)
            {
                Name(_ADR, 101)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(103, NCNT))
        {
            Device(\_SB.NVDR.N102)
            {
                Name(_ADR, 102)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(104, NCNT))
        {
            Device(\_SB.NVDR.N103)
            {
                Name(_ADR, 103)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(105, NCNT))
        {
            Device(\_SB.NVDR.N104)
            {
                Name(_ADR, 104)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(106, NCNT))
        {
            Device(\_SB.NVDR.N105)
            {
                Name(_ADR, 105)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(107, NCNT))
        {
            Device(\_SB.NVDR.N106)
            {
                Name(_ADR, 106)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(108, NCNT))
        {
            Device(\_SB.NVDR.N107)
            {
                Name(_ADR, 107)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(109, NCNT))
        {
            Device(\_SB.NVDR.N108)
            {
                Name(_ADR, 108)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(110, NCNT))
        {
            Device(\_SB.NVDR.N109)
            {
                Name(_ADR, 109)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(111, NCNT))
        {
            Device(\_SB.NVDR.N110)
            {
                Name(_ADR, 110)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(112, NCNT))
        {
            Device(\_SB.NVDR.N111)
            {
                Name(_ADR, 111)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(113, NCNT))
        {
            Device(\_SB.NVDR.N112)
            {
                Name(_ADR, 112)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(114, NCNT))
        {
            Device(\_SB.NVDR.N113)
            {
                Name(_ADR, 113)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(115, NCNT))
        {
            Device(\_SB.NVDR.N114)
            {
                Name(_ADR, 114)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(116, NCNT))
        {
            Device(\_SB.NVDR.N115)
            {
                Name(_ADR, 115)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(117, NCNT))
        {
            Device(\_SB.NVDR.N116)
            {
                Name(_ADR, 116)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(118, NCNT))
        {
            Device(\_SB.NVDR.N117)
            {
                Name(_ADR, 117)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(119, NCNT))
        {
            Device(\_SB.NVDR.N118)
            {
                Name(_ADR, 118)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(120, NCNT))
        {
            Device(\_SB.NVDR.N119)
            {
                Name(_ADR, 119)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(121, NCNT))
        {
            Device(\_SB.NVDR.N120)
            {
                Name(_ADR, 120)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(122, NCNT))
        {
            Device(\_SB.NVDR.N121)
            {
                Name(_ADR, 121)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(123, NCNT))
        {
            Device(\_SB.NVDR.N122)
            {
                Name(_ADR, 122)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(124, NCNT))
        {
            Device(\_SB.NVDR.N123)
            {
                Name(_ADR, 123)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(125, NCNT))
        {
            Device(\_SB.NVDR.N124)
            {
                Name(_ADR, 124)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(126, NCNT))
        {
            Device(\_SB.NVDR.N125)
            {
                Name(_ADR, 125)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(127, NCNT))
        {
            Device(\_SB.NVDR.N126)
            {
                Name(_ADR, 126)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }
        If(LLessEqual(128, NCNT))
        {
            Device(\_SB.NVDR.N127)
            {
                Name(_ADR, 127)
                Method(_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
                    { Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR)) }
                Function (_LSI, {PkgObj}) { Return (LSIM(_ADR)) }
                Function (_LSR, {PkgObj}, {IntObj, IntObj}) { Return (LSRM(Arg0, Arg1, _ADR)) }
            }
        }


#if defined (_DSDT_INTEL_)

        // GPE method for PMEM
        Scope(\_GPE)
        {
            // Method for notifying external changes to the battery device:
            //      E  - This event is edge triggered
            //      0A - Use bit A in the General Purpose Event register described
            //           in the FADT
            Method(_E0A)
            {

#elif defined (_DSDT_ARM_)

        // Interrupt signalled event device for PMEM
        Device(\_SB.GED3)
        {
            Name(_HID,"ACPI0013")
            Name(_UID, 3)
            Name(_CRS, ResourceTemplate()
            {
                Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive)
                    {FixedPcdGet8(PcdPmemEventVector)}
            })
            Method(_EVT, 1)
            {

#endif
                // Read the Event registers.
                Store(\_SB.NVDR.NEV0, Local0)
                Store(\_SB.NVDR.NEV1, Local1)
                Store(\_SB.NVDR.NEV2, Local2)
                Store(\_SB.NVDR.NEV3, Local3)
                Store(\_SB.NVDR.NEV4, Local4)

                // Go through each event register to see what events were signalled.
                // For NEV0-3, each bit corresponds to a 0x81 event on a different NVDIMM
                // child device.
                // For NEV4, bit 0 corresponds to 0x80 (NFIT Update Notification), and
                // bit 1 corresponds to 0x81 (Unconsumed Uncorrectable Memory Error Detected),
                // both on the NVDIMM root device.
                if (LNotEqual(Local0, 0))
                {
                    if (LNotEqual( And(Local0, 0x00000001), 0)) { Notify (\_SB.NVDR.N000, 0x81) }
                    if (LNotEqual( And(Local0, 0x00000002), 0)) { Notify (\_SB.NVDR.N001, 0x81) }
                    if (LNotEqual( And(Local0, 0x00000004), 0)) { Notify (\_SB.NVDR.N002, 0x81) }
                    if (LNotEqual( And(Local0, 0x00000008), 0)) { Notify (\_SB.NVDR.N003, 0x81) }
                    if (LNotEqual( And(Local0, 0x00000010), 0)) { Notify (\_SB.NVDR.N004, 0x81) }
                    if (LNotEqual( And(Local0, 0x00000020), 0)) { Notify (\_SB.NVDR.N005, 0x81) }
                    if (LNotEqual( And(Local0, 0x00000040), 0)) { Notify (\_SB.NVDR.N006, 0x81) }
                    if (LNotEqual( And(Local0, 0x00000080), 0)) { Notify (\_SB.NVDR.N007, 0x81) }
                    if (LNotEqual( And(Local0, 0x00000100), 0)) { Notify (\_SB.NVDR.N008, 0x81) }
                    if (LNotEqual( And(Local0, 0x00000200), 0)) { Notify (\_SB.NVDR.N009, 0x81) }
                    if (LNotEqual( And(Local0, 0x00000400), 0)) { Notify (\_SB.NVDR.N010, 0x81) }
                    if (LNotEqual( And(Local0, 0x00000800), 0)) { Notify (\_SB.NVDR.N011, 0x81) }
                    if (LNotEqual( And(Local0, 0x00001000), 0)) { Notify (\_SB.NVDR.N012, 0x81) }
                    if (LNotEqual( And(Local0, 0x00002000), 0)) { Notify (\_SB.NVDR.N013, 0x81) }
                    if (LNotEqual( And(Local0, 0x00004000), 0)) { Notify (\_SB.NVDR.N014, 0x81) }
                    if (LNotEqual( And(Local0, 0x00008000), 0)) { Notify (\_SB.NVDR.N015, 0x81) }
                    if (LNotEqual( And(Local0, 0x00010000), 0)) { Notify (\_SB.NVDR.N016, 0x81) }
                    if (LNotEqual( And(Local0, 0x00020000), 0)) { Notify (\_SB.NVDR.N017, 0x81) }
                    if (LNotEqual( And(Local0, 0x00040000), 0)) { Notify (\_SB.NVDR.N018, 0x81) }
                    if (LNotEqual( And(Local0, 0x00080000), 0)) { Notify (\_SB.NVDR.N019, 0x81) }
                    if (LNotEqual( And(Local0, 0x00100000), 0)) { Notify (\_SB.NVDR.N020, 0x81) }
                    if (LNotEqual( And(Local0, 0x00200000), 0)) { Notify (\_SB.NVDR.N021, 0x81) }
                    if (LNotEqual( And(Local0, 0x00400000), 0)) { Notify (\_SB.NVDR.N022, 0x81) }
                    if (LNotEqual( And(Local0, 0x00800000), 0)) { Notify (\_SB.NVDR.N023, 0x81) }
                    if (LNotEqual( And(Local0, 0x01000000), 0)) { Notify (\_SB.NVDR.N024, 0x81) }
                    if (LNotEqual( And(Local0, 0x02000000), 0)) { Notify (\_SB.NVDR.N025, 0x81) }
                    if (LNotEqual( And(Local0, 0x04000000), 0)) { Notify (\_SB.NVDR.N026, 0x81) }
                    if (LNotEqual( And(Local0, 0x08000000), 0)) { Notify (\_SB.NVDR.N027, 0x81) }
                    if (LNotEqual( And(Local0, 0x10000000), 0)) { Notify (\_SB.NVDR.N028, 0x81) }
                    if (LNotEqual( And(Local0, 0x20000000), 0)) { Notify (\_SB.NVDR.N029, 0x81) }
                    if (LNotEqual( And(Local0, 0x40000000), 0)) { Notify (\_SB.NVDR.N030, 0x81) }
                    if (LNotEqual( And(Local0, 0x80000000), 0)) { Notify (\_SB.NVDR.N031, 0x81) }
                    // Clear the event register.
                    Store (Local0, \_SB.NVDR.NEV0)
                }
                if (LNotEqual(Local1, 0))
                {
                    if (LNotEqual( And(Local1, 0x00000001), 0)) { Notify (\_SB.NVDR.N032, 0x81) }
                    if (LNotEqual( And(Local1, 0x00000002), 0)) { Notify (\_SB.NVDR.N033, 0x81) }
                    if (LNotEqual( And(Local1, 0x00000004), 0)) { Notify (\_SB.NVDR.N034, 0x81) }
                    if (LNotEqual( And(Local1, 0x00000008), 0)) { Notify (\_SB.NVDR.N035, 0x81) }
                    if (LNotEqual( And(Local1, 0x00000010), 0)) { Notify (\_SB.NVDR.N036, 0x81) }
                    if (LNotEqual( And(Local1, 0x00000020), 0)) { Notify (\_SB.NVDR.N037, 0x81) }
                    if (LNotEqual( And(Local1, 0x00000040), 0)) { Notify (\_SB.NVDR.N038, 0x81) }
                    if (LNotEqual( And(Local1, 0x00000080), 0)) { Notify (\_SB.NVDR.N039, 0x81) }
                    if (LNotEqual( And(Local1, 0x00000100), 0)) { Notify (\_SB.NVDR.N040, 0x81) }
                    if (LNotEqual( And(Local1, 0x00000200), 0)) { Notify (\_SB.NVDR.N041, 0x81) }
                    if (LNotEqual( And(Local1, 0x00000400), 0)) { Notify (\_SB.NVDR.N042, 0x81) }
                    if (LNotEqual( And(Local1, 0x00000800), 0)) { Notify (\_SB.NVDR.N043, 0x81) }
                    if (LNotEqual( And(Local1, 0x00001000), 0)) { Notify (\_SB.NVDR.N044, 0x81) }
                    if (LNotEqual( And(Local1, 0x00002000), 0)) { Notify (\_SB.NVDR.N045, 0x81) }
                    if (LNotEqual( And(Local1, 0x00004000), 0)) { Notify (\_SB.NVDR.N046, 0x81) }
                    if (LNotEqual( And(Local1, 0x00008000), 0)) { Notify (\_SB.NVDR.N047, 0x81) }
                    if (LNotEqual( And(Local1, 0x00010000), 0)) { Notify (\_SB.NVDR.N048, 0x81) }
                    if (LNotEqual( And(Local1, 0x00020000), 0)) { Notify (\_SB.NVDR.N049, 0x81) }
                    if (LNotEqual( And(Local1, 0x00040000), 0)) { Notify (\_SB.NVDR.N050, 0x81) }
                    if (LNotEqual( And(Local1, 0x00080000), 0)) { Notify (\_SB.NVDR.N051, 0x81) }
                    if (LNotEqual( And(Local1, 0x00100000), 0)) { Notify (\_SB.NVDR.N052, 0x81) }
                    if (LNotEqual( And(Local1, 0x00200000), 0)) { Notify (\_SB.NVDR.N053, 0x81) }
                    if (LNotEqual( And(Local1, 0x00400000), 0)) { Notify (\_SB.NVDR.N054, 0x81) }
                    if (LNotEqual( And(Local1, 0x00800000), 0)) { Notify (\_SB.NVDR.N055, 0x81) }
                    if (LNotEqual( And(Local1, 0x01000000), 0)) { Notify (\_SB.NVDR.N056, 0x81) }
                    if (LNotEqual( And(Local1, 0x02000000), 0)) { Notify (\_SB.NVDR.N057, 0x81) }
                    if (LNotEqual( And(Local1, 0x04000000), 0)) { Notify (\_SB.NVDR.N058, 0x81) }
                    if (LNotEqual( And(Local1, 0x08000000), 0)) { Notify (\_SB.NVDR.N059, 0x81) }
                    if (LNotEqual( And(Local1, 0x10000000), 0)) { Notify (\_SB.NVDR.N060, 0x81) }
                    if (LNotEqual( And(Local1, 0x20000000), 0)) { Notify (\_SB.NVDR.N061, 0x81) }
                    if (LNotEqual( And(Local1, 0x40000000), 0)) { Notify (\_SB.NVDR.N062, 0x81) }
                    if (LNotEqual( And(Local1, 0x80000000), 0)) { Notify (\_SB.NVDR.N063, 0x81) }
                    // Clear the event register.
                    Store (Local1, \_SB.NVDR.NEV1)
                }
                if (LNotEqual(Local2, 0))
                {
                    if (LNotEqual( And(Local2, 0x00000001), 0)) { Notify (\_SB.NVDR.N064, 0x81) }
                    if (LNotEqual( And(Local2, 0x00000002), 0)) { Notify (\_SB.NVDR.N065, 0x81) }
                    if (LNotEqual( And(Local2, 0x00000004), 0)) { Notify (\_SB.NVDR.N066, 0x81) }
                    if (LNotEqual( And(Local2, 0x00000008), 0)) { Notify (\_SB.NVDR.N067, 0x81) }
                    if (LNotEqual( And(Local2, 0x00000010), 0)) { Notify (\_SB.NVDR.N068, 0x81) }
                    if (LNotEqual( And(Local2, 0x00000020), 0)) { Notify (\_SB.NVDR.N069, 0x81) }
                    if (LNotEqual( And(Local2, 0x00000040), 0)) { Notify (\_SB.NVDR.N070, 0x81) }
                    if (LNotEqual( And(Local2, 0x00000080), 0)) { Notify (\_SB.NVDR.N071, 0x81) }
                    if (LNotEqual( And(Local2, 0x00000100), 0)) { Notify (\_SB.NVDR.N072, 0x81) }
                    if (LNotEqual( And(Local2, 0x00000200), 0)) { Notify (\_SB.NVDR.N073, 0x81) }
                    if (LNotEqual( And(Local2, 0x00000400), 0)) { Notify (\_SB.NVDR.N074, 0x81) }
                    if (LNotEqual( And(Local2, 0x00000800), 0)) { Notify (\_SB.NVDR.N075, 0x81) }
                    if (LNotEqual( And(Local2, 0x00001000), 0)) { Notify (\_SB.NVDR.N076, 0x81) }
                    if (LNotEqual( And(Local2, 0x00002000), 0)) { Notify (\_SB.NVDR.N077, 0x81) }
                    if (LNotEqual( And(Local2, 0x00004000), 0)) { Notify (\_SB.NVDR.N078, 0x81) }
                    if (LNotEqual( And(Local2, 0x00008000), 0)) { Notify (\_SB.NVDR.N079, 0x81) }
                    if (LNotEqual( And(Local2, 0x00010000), 0)) { Notify (\_SB.NVDR.N080, 0x81) }
                    if (LNotEqual( And(Local2, 0x00020000), 0)) { Notify (\_SB.NVDR.N081, 0x81) }
                    if (LNotEqual( And(Local2, 0x00040000), 0)) { Notify (\_SB.NVDR.N082, 0x81) }
                    if (LNotEqual( And(Local2, 0x00080000), 0)) { Notify (\_SB.NVDR.N083, 0x81) }
                    if (LNotEqual( And(Local2, 0x00100000), 0)) { Notify (\_SB.NVDR.N084, 0x81) }
                    if (LNotEqual( And(Local2, 0x00200000), 0)) { Notify (\_SB.NVDR.N085, 0x81) }
                    if (LNotEqual( And(Local2, 0x00400000), 0)) { Notify (\_SB.NVDR.N086, 0x81) }
                    if (LNotEqual( And(Local2, 0x00800000), 0)) { Notify (\_SB.NVDR.N087, 0x81) }
                    if (LNotEqual( And(Local2, 0x01000000), 0)) { Notify (\_SB.NVDR.N088, 0x81) }
                    if (LNotEqual( And(Local2, 0x02000000), 0)) { Notify (\_SB.NVDR.N089, 0x81) }
                    if (LNotEqual( And(Local2, 0x04000000), 0)) { Notify (\_SB.NVDR.N090, 0x81) }
                    if (LNotEqual( And(Local2, 0x08000000), 0)) { Notify (\_SB.NVDR.N091, 0x81) }
                    if (LNotEqual( And(Local2, 0x10000000), 0)) { Notify (\_SB.NVDR.N092, 0x81) }
                    if (LNotEqual( And(Local2, 0x20000000), 0)) { Notify (\_SB.NVDR.N093, 0x81) }
                    if (LNotEqual( And(Local2, 0x40000000), 0)) { Notify (\_SB.NVDR.N094, 0x81) }
                    if (LNotEqual( And(Local2, 0x80000000), 0)) { Notify (\_SB.NVDR.N095, 0x81) }
                    // Clear the event register.
                    Store (Local2, \_SB.NVDR.NEV2)
                }
                if (LNotEqual(Local3, 0))
                {
                    if (LNotEqual( And(Local3, 0x00000001), 0)) { Notify (\_SB.NVDR.N096, 0x81) }
                    if (LNotEqual( And(Local3, 0x00000002), 0)) { Notify (\_SB.NVDR.N097, 0x81) }
                    if (LNotEqual( And(Local3, 0x00000004), 0)) { Notify (\_SB.NVDR.N098, 0x81) }
                    if (LNotEqual( And(Local3, 0x00000008), 0)) { Notify (\_SB.NVDR.N099, 0x81) }
                    if (LNotEqual( And(Local3, 0x00000010), 0)) { Notify (\_SB.NVDR.N100, 0x81) }
                    if (LNotEqual( And(Local3, 0x00000020), 0)) { Notify (\_SB.NVDR.N101, 0x81) }
                    if (LNotEqual( And(Local3, 0x00000040), 0)) { Notify (\_SB.NVDR.N102, 0x81) }
                    if (LNotEqual( And(Local3, 0x00000080), 0)) { Notify (\_SB.NVDR.N103, 0x81) }
                    if (LNotEqual( And(Local3, 0x00000100), 0)) { Notify (\_SB.NVDR.N104, 0x81) }
                    if (LNotEqual( And(Local3, 0x00000200), 0)) { Notify (\_SB.NVDR.N105, 0x81) }
                    if (LNotEqual( And(Local3, 0x00000400), 0)) { Notify (\_SB.NVDR.N106, 0x81) }
                    if (LNotEqual( And(Local3, 0x00000800), 0)) { Notify (\_SB.NVDR.N107, 0x81) }
                    if (LNotEqual( And(Local3, 0x00001000), 0)) { Notify (\_SB.NVDR.N108, 0x81) }
                    if (LNotEqual( And(Local3, 0x00002000), 0)) { Notify (\_SB.NVDR.N109, 0x81) }
                    if (LNotEqual( And(Local3, 0x00004000), 0)) { Notify (\_SB.NVDR.N110, 0x81) }
                    if (LNotEqual( And(Local3, 0x00008000), 0)) { Notify (\_SB.NVDR.N111, 0x81) }
                    if (LNotEqual( And(Local3, 0x00010000), 0)) { Notify (\_SB.NVDR.N112, 0x81) }
                    if (LNotEqual( And(Local3, 0x00020000), 0)) { Notify (\_SB.NVDR.N113, 0x81) }
                    if (LNotEqual( And(Local3, 0x00040000), 0)) { Notify (\_SB.NVDR.N114, 0x81) }
                    if (LNotEqual( And(Local3, 0x00080000), 0)) { Notify (\_SB.NVDR.N115, 0x81) }
                    if (LNotEqual( And(Local3, 0x00100000), 0)) { Notify (\_SB.NVDR.N116, 0x81) }
                    if (LNotEqual( And(Local3, 0x00200000), 0)) { Notify (\_SB.NVDR.N117, 0x81) }
                    if (LNotEqual( And(Local3, 0x00400000), 0)) { Notify (\_SB.NVDR.N118, 0x81) }
                    if (LNotEqual( And(Local3, 0x00800000), 0)) { Notify (\_SB.NVDR.N119, 0x81) }
                    if (LNotEqual( And(Local3, 0x01000000), 0)) { Notify (\_SB.NVDR.N120, 0x81) }
                    if (LNotEqual( And(Local3, 0x02000000), 0)) { Notify (\_SB.NVDR.N121, 0x81) }
                    if (LNotEqual( And(Local3, 0x04000000), 0)) { Notify (\_SB.NVDR.N122, 0x81) }
                    if (LNotEqual( And(Local3, 0x08000000), 0)) { Notify (\_SB.NVDR.N123, 0x81) }
                    if (LNotEqual( And(Local3, 0x10000000), 0)) { Notify (\_SB.NVDR.N124, 0x81) }
                    if (LNotEqual( And(Local3, 0x20000000), 0)) { Notify (\_SB.NVDR.N125, 0x81) }
                    if (LNotEqual( And(Local3, 0x40000000), 0)) { Notify (\_SB.NVDR.N126, 0x81) }
                    if (LNotEqual( And(Local3, 0x80000000), 0)) { Notify (\_SB.NVDR.N127, 0x81) }
                    // Clear the event register.
                    Store (Local3, \_SB.NVDR.NEV3)
                }
                if (LNotEqual(Local4, 0))
                {
                    if (LNotEqual(And(Local4, 0x1), 0)) { Notify (\_SB.NVDR, 0x80) }
                    if (LNotEqual(And(Local4, 0x2), 0)) { Notify (\_SB.NVDR, 0x81) }
                    // Clear the event register.
                    Store (Local4, \_SB.NVDR.NEV4)
                }
            }
        }
    }

    // Processor devices ======================================================

    If(LLessEqual(1, PCNT))
    {
        Device(P001) { Name(_HID, "ACPI0007") Name(_UID, 1) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(2, PCNT))
    {
        Device(P002) { Name(_HID, "ACPI0007") Name(_UID, 2) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(3, PCNT))
    {
        Device(P003) { Name(_HID, "ACPI0007") Name(_UID, 3) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(4, PCNT))
    {
        Device(P004) { Name(_HID, "ACPI0007") Name(_UID, 4) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(5, PCNT))
    {
        Device(P005) { Name(_HID, "ACPI0007") Name(_UID, 5) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(6, PCNT))
    {
        Device(P006) { Name(_HID, "ACPI0007") Name(_UID, 6) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(7, PCNT))
    {
        Device(P007) { Name(_HID, "ACPI0007") Name(_UID, 7) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(8, PCNT))
    {
        Device(P008) { Name(_HID, "ACPI0007") Name(_UID, 8) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(9, PCNT))
    {
        Device(P009) { Name(_HID, "ACPI0007") Name(_UID, 9) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(10, PCNT))
    {
        Device(P010) { Name(_HID, "ACPI0007") Name(_UID, 10) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(11, PCNT))
    {
        Device(P011) { Name(_HID, "ACPI0007") Name(_UID, 11) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(12, PCNT))
    {
        Device(P012) { Name(_HID, "ACPI0007") Name(_UID, 12) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(13, PCNT))
    {
        Device(P013) { Name(_HID, "ACPI0007") Name(_UID, 13) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(14, PCNT))
    {
        Device(P014) { Name(_HID, "ACPI0007") Name(_UID, 14) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(15, PCNT))
    {
        Device(P015) { Name(_HID, "ACPI0007") Name(_UID, 15) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(16, PCNT))
    {
        Device(P016) { Name(_HID, "ACPI0007") Name(_UID, 16) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(17, PCNT))
    {
        Device(P017) { Name(_HID, "ACPI0007") Name(_UID, 17) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(18, PCNT))
    {
        Device(P018) { Name(_HID, "ACPI0007") Name(_UID, 18) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(19, PCNT))
    {
        Device(P019) { Name(_HID, "ACPI0007") Name(_UID, 19) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(20, PCNT))
    {
        Device(P020) { Name(_HID, "ACPI0007") Name(_UID, 20) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(21, PCNT))
    {
        Device(P021) { Name(_HID, "ACPI0007") Name(_UID, 21) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(22, PCNT))
    {
        Device(P022) { Name(_HID, "ACPI0007") Name(_UID, 22) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(23, PCNT))
    {
        Device(P023) { Name(_HID, "ACPI0007") Name(_UID, 23) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(24, PCNT))
    {
        Device(P024) { Name(_HID, "ACPI0007") Name(_UID, 24) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(25, PCNT))
    {
        Device(P025) { Name(_HID, "ACPI0007") Name(_UID, 25) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(26, PCNT))
    {
        Device(P026) { Name(_HID, "ACPI0007") Name(_UID, 26) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(27, PCNT))
    {
        Device(P027) { Name(_HID, "ACPI0007") Name(_UID, 27) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(28, PCNT))
    {
        Device(P028) { Name(_HID, "ACPI0007") Name(_UID, 28) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(29, PCNT))
    {
        Device(P029) { Name(_HID, "ACPI0007") Name(_UID, 29) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(30, PCNT))
    {
        Device(P030) { Name(_HID, "ACPI0007") Name(_UID, 30) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(31, PCNT))
    {
        Device(P031) { Name(_HID, "ACPI0007") Name(_UID, 31) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(32, PCNT))
    {
        Device(P032) { Name(_HID, "ACPI0007") Name(_UID, 32) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(33, PCNT))
    {
        Device(P033) { Name(_HID, "ACPI0007") Name(_UID, 33) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(34, PCNT))
    {
        Device(P034) { Name(_HID, "ACPI0007") Name(_UID, 34) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(35, PCNT))
    {
        Device(P035) { Name(_HID, "ACPI0007") Name(_UID, 35) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(36, PCNT))
    {
        Device(P036) { Name(_HID, "ACPI0007") Name(_UID, 36) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(37, PCNT))
    {
        Device(P037) { Name(_HID, "ACPI0007") Name(_UID, 37) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(38, PCNT))
    {
        Device(P038) { Name(_HID, "ACPI0007") Name(_UID, 38) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(39, PCNT))
    {
        Device(P039) { Name(_HID, "ACPI0007") Name(_UID, 39) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(40, PCNT))
    {
        Device(P040) { Name(_HID, "ACPI0007") Name(_UID, 40) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(41, PCNT))
    {
        Device(P041) { Name(_HID, "ACPI0007") Name(_UID, 41) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(42, PCNT))
    {
        Device(P042) { Name(_HID, "ACPI0007") Name(_UID, 42) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(43, PCNT))
    {
        Device(P043) { Name(_HID, "ACPI0007") Name(_UID, 43) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(44, PCNT))
    {
        Device(P044) { Name(_HID, "ACPI0007") Name(_UID, 44) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(45, PCNT))
    {
        Device(P045) { Name(_HID, "ACPI0007") Name(_UID, 45) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(46, PCNT))
    {
        Device(P046) { Name(_HID, "ACPI0007") Name(_UID, 46) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(47, PCNT))
    {
        Device(P047) { Name(_HID, "ACPI0007") Name(_UID, 47) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(48, PCNT))
    {
        Device(P048) { Name(_HID, "ACPI0007") Name(_UID, 48) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(49, PCNT))
    {
        Device(P049) { Name(_HID, "ACPI0007") Name(_UID, 49) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(50, PCNT))
    {
        Device(P050) { Name(_HID, "ACPI0007") Name(_UID, 50) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(51, PCNT))
    {
        Device(P051) { Name(_HID, "ACPI0007") Name(_UID, 51) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(52, PCNT))
    {
        Device(P052) { Name(_HID, "ACPI0007") Name(_UID, 52) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(53, PCNT))
    {
        Device(P053) { Name(_HID, "ACPI0007") Name(_UID, 53) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(54, PCNT))
    {
        Device(P054) { Name(_HID, "ACPI0007") Name(_UID, 54) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(55, PCNT))
    {
        Device(P055) { Name(_HID, "ACPI0007") Name(_UID, 55) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(56, PCNT))
    {
        Device(P056) { Name(_HID, "ACPI0007") Name(_UID, 56) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(57, PCNT))
    {
        Device(P057) { Name(_HID, "ACPI0007") Name(_UID, 57) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(58, PCNT))
    {
        Device(P058) { Name(_HID, "ACPI0007") Name(_UID, 58) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(59, PCNT))
    {
        Device(P059) { Name(_HID, "ACPI0007") Name(_UID, 59) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(60, PCNT))
    {
        Device(P060) { Name(_HID, "ACPI0007") Name(_UID, 60) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(61, PCNT))
    {
        Device(P061) { Name(_HID, "ACPI0007") Name(_UID, 61) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(62, PCNT))
    {
        Device(P062) { Name(_HID, "ACPI0007") Name(_UID, 62) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(63, PCNT))
    {
        Device(P063) { Name(_HID, "ACPI0007") Name(_UID, 63) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(64, PCNT))
    {
        Device(P064) { Name(_HID, "ACPI0007") Name(_UID, 64) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(65, PCNT))
    {
        Device(P065) { Name(_HID, "ACPI0007") Name(_UID, 65) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(66, PCNT))
    {
        Device(P066) { Name(_HID, "ACPI0007") Name(_UID, 66) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(67, PCNT))
    {
        Device(P067) { Name(_HID, "ACPI0007") Name(_UID, 67) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(68, PCNT))
    {
        Device(P068) { Name(_HID, "ACPI0007") Name(_UID, 68) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(69, PCNT))
    {
        Device(P069) { Name(_HID, "ACPI0007") Name(_UID, 69) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(70, PCNT))
    {
        Device(P070) { Name(_HID, "ACPI0007") Name(_UID, 70) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(71, PCNT))
    {
        Device(P071) { Name(_HID, "ACPI0007") Name(_UID, 71) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(72, PCNT))
    {
        Device(P072) { Name(_HID, "ACPI0007") Name(_UID, 72) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(73, PCNT))
    {
        Device(P073) { Name(_HID, "ACPI0007") Name(_UID, 73) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(74, PCNT))
    {
        Device(P074) { Name(_HID, "ACPI0007") Name(_UID, 74) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(75, PCNT))
    {
        Device(P075) { Name(_HID, "ACPI0007") Name(_UID, 75) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(76, PCNT))
    {
        Device(P076) { Name(_HID, "ACPI0007") Name(_UID, 76) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(77, PCNT))
    {
        Device(P077) { Name(_HID, "ACPI0007") Name(_UID, 77) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(78, PCNT))
    {
        Device(P078) { Name(_HID, "ACPI0007") Name(_UID, 78) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(79, PCNT))
    {
        Device(P079) { Name(_HID, "ACPI0007") Name(_UID, 79) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(80, PCNT))
    {
        Device(P080) { Name(_HID, "ACPI0007") Name(_UID, 80) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(81, PCNT))
    {
        Device(P081) { Name(_HID, "ACPI0007") Name(_UID, 81) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(82, PCNT))
    {
        Device(P082) { Name(_HID, "ACPI0007") Name(_UID, 82) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(83, PCNT))
    {
        Device(P083) { Name(_HID, "ACPI0007") Name(_UID, 83) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(84, PCNT))
    {
        Device(P084) { Name(_HID, "ACPI0007") Name(_UID, 84) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(85, PCNT))
    {
        Device(P085) { Name(_HID, "ACPI0007") Name(_UID, 85) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(86, PCNT))
    {
        Device(P086) { Name(_HID, "ACPI0007") Name(_UID, 86) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(87, PCNT))
    {
        Device(P087) { Name(_HID, "ACPI0007") Name(_UID, 87) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(88, PCNT))
    {
        Device(P088) { Name(_HID, "ACPI0007") Name(_UID, 88) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(89, PCNT))
    {
        Device(P089) { Name(_HID, "ACPI0007") Name(_UID, 89) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(90, PCNT))
    {
        Device(P090) { Name(_HID, "ACPI0007") Name(_UID, 90) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(91, PCNT))
    {
        Device(P091) { Name(_HID, "ACPI0007") Name(_UID, 91) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(92, PCNT))
    {
        Device(P092) { Name(_HID, "ACPI0007") Name(_UID, 92) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(93, PCNT))
    {
        Device(P093) { Name(_HID, "ACPI0007") Name(_UID, 93) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(94, PCNT))
    {
        Device(P094) { Name(_HID, "ACPI0007") Name(_UID, 94) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(95, PCNT))
    {
        Device(P095) { Name(_HID, "ACPI0007") Name(_UID, 95) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(96, PCNT))
    {
        Device(P096) { Name(_HID, "ACPI0007") Name(_UID, 96) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(97, PCNT))
    {
        Device(P097) { Name(_HID, "ACPI0007") Name(_UID, 97) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(98, PCNT))
    {
        Device(P098) { Name(_HID, "ACPI0007") Name(_UID, 98) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(99, PCNT))
    {
        Device(P099) { Name(_HID, "ACPI0007") Name(_UID, 99) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(100, PCNT))
    {
        Device(P100) { Name(_HID, "ACPI0007") Name(_UID, 100) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(101, PCNT))
    {
        Device(P101) { Name(_HID, "ACPI0007") Name(_UID, 101) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(102, PCNT))
    {
        Device(P102) { Name(_HID, "ACPI0007") Name(_UID, 102) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(103, PCNT))
    {
        Device(P103) { Name(_HID, "ACPI0007") Name(_UID, 103) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(104, PCNT))
    {
        Device(P104) { Name(_HID, "ACPI0007") Name(_UID, 104) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(105, PCNT))
    {
        Device(P105) { Name(_HID, "ACPI0007") Name(_UID, 105) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(106, PCNT))
    {
        Device(P106) { Name(_HID, "ACPI0007") Name(_UID, 106) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(107, PCNT))
    {
        Device(P107) { Name(_HID, "ACPI0007") Name(_UID, 107) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(108, PCNT))
    {
        Device(P108) { Name(_HID, "ACPI0007") Name(_UID, 108) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(109, PCNT))
    {
        Device(P109) { Name(_HID, "ACPI0007") Name(_UID, 109) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(110, PCNT))
    {
        Device(P110) { Name(_HID, "ACPI0007") Name(_UID, 110) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(111, PCNT))
    {
        Device(P111) { Name(_HID, "ACPI0007") Name(_UID, 111) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(112, PCNT))
    {
        Device(P112) { Name(_HID, "ACPI0007") Name(_UID, 112) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(113, PCNT))
    {
        Device(P113) { Name(_HID, "ACPI0007") Name(_UID, 113) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(114, PCNT))
    {
        Device(P114) { Name(_HID, "ACPI0007") Name(_UID, 114) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(115, PCNT))
    {
        Device(P115) { Name(_HID, "ACPI0007") Name(_UID, 115) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(116, PCNT))
    {
        Device(P116) { Name(_HID, "ACPI0007") Name(_UID, 116) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(117, PCNT))
    {
        Device(P117) { Name(_HID, "ACPI0007") Name(_UID, 117) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(118, PCNT))
    {
        Device(P118) { Name(_HID, "ACPI0007") Name(_UID, 118) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(119, PCNT))
    {
        Device(P119) { Name(_HID, "ACPI0007") Name(_UID, 119) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(120, PCNT))
    {
        Device(P120) { Name(_HID, "ACPI0007") Name(_UID, 120) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(121, PCNT))
    {
        Device(P121) { Name(_HID, "ACPI0007") Name(_UID, 121) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(122, PCNT))
    {
        Device(P122) { Name(_HID, "ACPI0007") Name(_UID, 122) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(123, PCNT))
    {
        Device(P123) { Name(_HID, "ACPI0007") Name(_UID, 123) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(124, PCNT))
    {
        Device(P124) { Name(_HID, "ACPI0007") Name(_UID, 124) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(125, PCNT))
    {
        Device(P125) { Name(_HID, "ACPI0007") Name(_UID, 125) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(126, PCNT))
    {
        Device(P126) { Name(_HID, "ACPI0007") Name(_UID, 126) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(127, PCNT))
    {
        Device(P127) { Name(_HID, "ACPI0007") Name(_UID, 127) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(128, PCNT))
    {
        Device(P128) { Name(_HID, "ACPI0007") Name(_UID, 128) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(129, PCNT))
    {
        Device(P129) { Name(_HID, "ACPI0007") Name(_UID, 129) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(130, PCNT))
    {
        Device(P130) { Name(_HID, "ACPI0007") Name(_UID, 130) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(131, PCNT))
    {
        Device(P131) { Name(_HID, "ACPI0007") Name(_UID, 131) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(132, PCNT))
    {
        Device(P132) { Name(_HID, "ACPI0007") Name(_UID, 132) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(133, PCNT))
    {
        Device(P133) { Name(_HID, "ACPI0007") Name(_UID, 133) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(134, PCNT))
    {
        Device(P134) { Name(_HID, "ACPI0007") Name(_UID, 134) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(135, PCNT))
    {
        Device(P135) { Name(_HID, "ACPI0007") Name(_UID, 135) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(136, PCNT))
    {
        Device(P136) { Name(_HID, "ACPI0007") Name(_UID, 136) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(137, PCNT))
    {
        Device(P137) { Name(_HID, "ACPI0007") Name(_UID, 137) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(138, PCNT))
    {
        Device(P138) { Name(_HID, "ACPI0007") Name(_UID, 138) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(139, PCNT))
    {
        Device(P139) { Name(_HID, "ACPI0007") Name(_UID, 139) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(140, PCNT))
    {
        Device(P140) { Name(_HID, "ACPI0007") Name(_UID, 140) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(141, PCNT))
    {
        Device(P141) { Name(_HID, "ACPI0007") Name(_UID, 141) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(142, PCNT))
    {
        Device(P142) { Name(_HID, "ACPI0007") Name(_UID, 142) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(143, PCNT))
    {
        Device(P143) { Name(_HID, "ACPI0007") Name(_UID, 143) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(144, PCNT))
    {
        Device(P144) { Name(_HID, "ACPI0007") Name(_UID, 144) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(145, PCNT))
    {
        Device(P145) { Name(_HID, "ACPI0007") Name(_UID, 145) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(146, PCNT))
    {
        Device(P146) { Name(_HID, "ACPI0007") Name(_UID, 146) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(147, PCNT))
    {
        Device(P147) { Name(_HID, "ACPI0007") Name(_UID, 147) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(148, PCNT))
    {
        Device(P148) { Name(_HID, "ACPI0007") Name(_UID, 148) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(149, PCNT))
    {
        Device(P149) { Name(_HID, "ACPI0007") Name(_UID, 149) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(150, PCNT))
    {
        Device(P150) { Name(_HID, "ACPI0007") Name(_UID, 150) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(151, PCNT))
    {
        Device(P151) { Name(_HID, "ACPI0007") Name(_UID, 151) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(152, PCNT))
    {
        Device(P152) { Name(_HID, "ACPI0007") Name(_UID, 152) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(153, PCNT))
    {
        Device(P153) { Name(_HID, "ACPI0007") Name(_UID, 153) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(154, PCNT))
    {
        Device(P154) { Name(_HID, "ACPI0007") Name(_UID, 154) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(155, PCNT))
    {
        Device(P155) { Name(_HID, "ACPI0007") Name(_UID, 155) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(156, PCNT))
    {
        Device(P156) { Name(_HID, "ACPI0007") Name(_UID, 156) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(157, PCNT))
    {
        Device(P157) { Name(_HID, "ACPI0007") Name(_UID, 157) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(158, PCNT))
    {
        Device(P158) { Name(_HID, "ACPI0007") Name(_UID, 158) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(159, PCNT))
    {
        Device(P159) { Name(_HID, "ACPI0007") Name(_UID, 159) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(160, PCNT))
    {
        Device(P160) { Name(_HID, "ACPI0007") Name(_UID, 160) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(161, PCNT))
    {
        Device(P161) { Name(_HID, "ACPI0007") Name(_UID, 161) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(162, PCNT))
    {
        Device(P162) { Name(_HID, "ACPI0007") Name(_UID, 162) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(163, PCNT))
    {
        Device(P163) { Name(_HID, "ACPI0007") Name(_UID, 163) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(164, PCNT))
    {
        Device(P164) { Name(_HID, "ACPI0007") Name(_UID, 164) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(165, PCNT))
    {
        Device(P165) { Name(_HID, "ACPI0007") Name(_UID, 165) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(166, PCNT))
    {
        Device(P166) { Name(_HID, "ACPI0007") Name(_UID, 166) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(167, PCNT))
    {
        Device(P167) { Name(_HID, "ACPI0007") Name(_UID, 167) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(168, PCNT))
    {
        Device(P168) { Name(_HID, "ACPI0007") Name(_UID, 168) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(169, PCNT))
    {
        Device(P169) { Name(_HID, "ACPI0007") Name(_UID, 169) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(170, PCNT))
    {
        Device(P170) { Name(_HID, "ACPI0007") Name(_UID, 170) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(171, PCNT))
    {
        Device(P171) { Name(_HID, "ACPI0007") Name(_UID, 171) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(172, PCNT))
    {
        Device(P172) { Name(_HID, "ACPI0007") Name(_UID, 172) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(173, PCNT))
    {
        Device(P173) { Name(_HID, "ACPI0007") Name(_UID, 173) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(174, PCNT))
    {
        Device(P174) { Name(_HID, "ACPI0007") Name(_UID, 174) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(175, PCNT))
    {
        Device(P175) { Name(_HID, "ACPI0007") Name(_UID, 175) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(176, PCNT))
    {
        Device(P176) { Name(_HID, "ACPI0007") Name(_UID, 176) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(177, PCNT))
    {
        Device(P177) { Name(_HID, "ACPI0007") Name(_UID, 177) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(178, PCNT))
    {
        Device(P178) { Name(_HID, "ACPI0007") Name(_UID, 178) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(179, PCNT))
    {
        Device(P179) { Name(_HID, "ACPI0007") Name(_UID, 179) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(180, PCNT))
    {
        Device(P180) { Name(_HID, "ACPI0007") Name(_UID, 180) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(181, PCNT))
    {
        Device(P181) { Name(_HID, "ACPI0007") Name(_UID, 181) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(182, PCNT))
    {
        Device(P182) { Name(_HID, "ACPI0007") Name(_UID, 182) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(183, PCNT))
    {
        Device(P183) { Name(_HID, "ACPI0007") Name(_UID, 183) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(184, PCNT))
    {
        Device(P184) { Name(_HID, "ACPI0007") Name(_UID, 184) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(185, PCNT))
    {
        Device(P185) { Name(_HID, "ACPI0007") Name(_UID, 185) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(186, PCNT))
    {
        Device(P186) { Name(_HID, "ACPI0007") Name(_UID, 186) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(187, PCNT))
    {
        Device(P187) { Name(_HID, "ACPI0007") Name(_UID, 187) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(188, PCNT))
    {
        Device(P188) { Name(_HID, "ACPI0007") Name(_UID, 188) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(189, PCNT))
    {
        Device(P189) { Name(_HID, "ACPI0007") Name(_UID, 189) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(190, PCNT))
    {
        Device(P190) { Name(_HID, "ACPI0007") Name(_UID, 190) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(191, PCNT))
    {
        Device(P191) { Name(_HID, "ACPI0007") Name(_UID, 191) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(192, PCNT))
    {
        Device(P192) { Name(_HID, "ACPI0007") Name(_UID, 192) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(193, PCNT))
    {
        Device(P193) { Name(_HID, "ACPI0007") Name(_UID, 193) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(194, PCNT))
    {
        Device(P194) { Name(_HID, "ACPI0007") Name(_UID, 194) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(195, PCNT))
    {
        Device(P195) { Name(_HID, "ACPI0007") Name(_UID, 195) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(196, PCNT))
    {
        Device(P196) { Name(_HID, "ACPI0007") Name(_UID, 196) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(197, PCNT))
    {
        Device(P197) { Name(_HID, "ACPI0007") Name(_UID, 197) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(198, PCNT))
    {
        Device(P198) { Name(_HID, "ACPI0007") Name(_UID, 198) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(199, PCNT))
    {
        Device(P199) { Name(_HID, "ACPI0007") Name(_UID, 199) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(200, PCNT))
    {
        Device(P200) { Name(_HID, "ACPI0007") Name(_UID, 200) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(201, PCNT))
    {
        Device(P201) { Name(_HID, "ACPI0007") Name(_UID, 201) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(202, PCNT))
    {
        Device(P202) { Name(_HID, "ACPI0007") Name(_UID, 202) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(203, PCNT))
    {
        Device(P203) { Name(_HID, "ACPI0007") Name(_UID, 203) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(204, PCNT))
    {
        Device(P204) { Name(_HID, "ACPI0007") Name(_UID, 204) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(205, PCNT))
    {
        Device(P205) { Name(_HID, "ACPI0007") Name(_UID, 205) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(206, PCNT))
    {
        Device(P206) { Name(_HID, "ACPI0007") Name(_UID, 206) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(207, PCNT))
    {
        Device(P207) { Name(_HID, "ACPI0007") Name(_UID, 207) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(208, PCNT))
    {
        Device(P208) { Name(_HID, "ACPI0007") Name(_UID, 208) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(209, PCNT))
    {
        Device(P209) { Name(_HID, "ACPI0007") Name(_UID, 209) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(210, PCNT))
    {
        Device(P210) { Name(_HID, "ACPI0007") Name(_UID, 210) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(211, PCNT))
    {
        Device(P211) { Name(_HID, "ACPI0007") Name(_UID, 211) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(212, PCNT))
    {
        Device(P212) { Name(_HID, "ACPI0007") Name(_UID, 212) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(213, PCNT))
    {
        Device(P213) { Name(_HID, "ACPI0007") Name(_UID, 213) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(214, PCNT))
    {
        Device(P214) { Name(_HID, "ACPI0007") Name(_UID, 214) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(215, PCNT))
    {
        Device(P215) { Name(_HID, "ACPI0007") Name(_UID, 215) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(216, PCNT))
    {
        Device(P216) { Name(_HID, "ACPI0007") Name(_UID, 216) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(217, PCNT))
    {
        Device(P217) { Name(_HID, "ACPI0007") Name(_UID, 217) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(218, PCNT))
    {
        Device(P218) { Name(_HID, "ACPI0007") Name(_UID, 218) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(219, PCNT))
    {
        Device(P219) { Name(_HID, "ACPI0007") Name(_UID, 219) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(220, PCNT))
    {
        Device(P220) { Name(_HID, "ACPI0007") Name(_UID, 220) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(221, PCNT))
    {
        Device(P221) { Name(_HID, "ACPI0007") Name(_UID, 221) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(222, PCNT))
    {
        Device(P222) { Name(_HID, "ACPI0007") Name(_UID, 222) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(223, PCNT))
    {
        Device(P223) { Name(_HID, "ACPI0007") Name(_UID, 223) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(224, PCNT))
    {
        Device(P224) { Name(_HID, "ACPI0007") Name(_UID, 224) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(225, PCNT))
    {
        Device(P225) { Name(_HID, "ACPI0007") Name(_UID, 225) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(226, PCNT))
    {
        Device(P226) { Name(_HID, "ACPI0007") Name(_UID, 226) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(227, PCNT))
    {
        Device(P227) { Name(_HID, "ACPI0007") Name(_UID, 227) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(228, PCNT))
    {
        Device(P228) { Name(_HID, "ACPI0007") Name(_UID, 228) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(229, PCNT))
    {
        Device(P229) { Name(_HID, "ACPI0007") Name(_UID, 229) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(230, PCNT))
    {
        Device(P230) { Name(_HID, "ACPI0007") Name(_UID, 230) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(231, PCNT))
    {
        Device(P231) { Name(_HID, "ACPI0007") Name(_UID, 231) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(232, PCNT))
    {
        Device(P232) { Name(_HID, "ACPI0007") Name(_UID, 232) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(233, PCNT))
    {
        Device(P233) { Name(_HID, "ACPI0007") Name(_UID, 233) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(234, PCNT))
    {
        Device(P234) { Name(_HID, "ACPI0007") Name(_UID, 234) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(235, PCNT))
    {
        Device(P235) { Name(_HID, "ACPI0007") Name(_UID, 235) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(236, PCNT))
    {
        Device(P236) { Name(_HID, "ACPI0007") Name(_UID, 236) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(237, PCNT))
    {
        Device(P237) { Name(_HID, "ACPI0007") Name(_UID, 237) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(238, PCNT))
    {
        Device(P238) { Name(_HID, "ACPI0007") Name(_UID, 238) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(239, PCNT))
    {
        Device(P239) { Name(_HID, "ACPI0007") Name(_UID, 239) Method(_STA, 0) { Return(0xF) } }
    }
    If(LLessEqual(240, PCNT))
    {
        Device(P240) { Name(_HID, "ACPI0007") Name(_UID, 240) Method(_STA, 0) { Return(0xF) } }
    }
}

