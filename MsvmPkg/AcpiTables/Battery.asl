/*++

Copyright (c) Microsoft Corporation

Module Name:

    Battery.asl

Abstract:

    A table (VBAT) that contains the declarations of the virtual battery device.

--*/

#define MSVMPKG_MMIO_DEVICE_BATTERY 0xfed3f000

DefinitionBlock (
    "vbat.aml",
    "OEM4",
    0x02,           // DSDT Compliance revision.
    "MSFTVM",       // OEM ID (6 byte string)
    "VBAT",         // OEM table ID  (8 byte string)
    0x01            // OEM version of DSDT table (4 byte Integer)
    )
{

    //
    // MMIO region for Virtual Battery
    //
    OperationRegion(BATM, SystemMemory, MSVMPKG_MMIO_DEVICE_BATTERY, 0x1000)
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

    Scope(\_SB)
    {
        //
        // Virtual Battery
        //
        Device(BAT1)
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
                Name (BST, Package () {
                    0x0, // Battery State
                    0x0, // Battery Present Rate (discharge rate)
                    0x0, // Battery Remaining Capacity
                    0x1388  // Battery Present Voltage - 5 volts
                })

                Store(BSTE, Index(BST, 0))
                Store(BRAT, Index(BST, 1))
                Store(BCAP, Index(BST, 2))

                //
                // Check if battery isn't present
                //
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

        //
        // Virtual AC Adapter
        //
        Device(AC1)
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
    }

#if defined (MDE_CPU_X64)
    //
    // General Purpose Event Scope - must be used in X64 due to Linux only
    // supporting interrupt-signaled events in ACPI Hardware Reduced platforms.
    //
    Scope(\_GPE)
    {
        //
        // Method for notifying external changes to the control
        // method battery:
        //      E  - This event is edge triggered
        //      09 - Use bit 9 in the General Purpose Event register described
        //           in the FADT
        Method(_E09)
        {
#elif defined (MDE_CPU_AARCH64)
    //
    // ACPI 6.1 Interrupt Signaled Events - Required on ACPI Hardware reduced platforms.
    // TODO-cho: Figure out what interrupt this will use on ARM64.
    //
    Device(\_SB.GED1)
    {
        Name(_HID,"ACPI0013")
        Name(_CRS, ResourceTemplate()
            {
                Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) {10}
            }
        )

        Method(_EVT, 1)
        {
#else
    #error Undefined Architecture
#endif
            //
            // Read battery notify type into local
            //
            Store(BNST, Local0)

            //
            // Break status into different bits
            // Local1 is 0x80 status at bit 0
            // Local2 is 0x81 status at bit 1
            //
            And(Local0, 0x1, Local1)
            And(Local0, 0x2, Local2)

            //
            // For Windows and Linux, sending a notify of 0x81 forces a recheck
            // of both _BIX and _BST. Thus we can let Notify 0x81 take priority
            // if we see both bits set.
            //
            If (LEqual(Local2, 0x2))
            {
                //
                // Note that since 0x81 takes priority, force a recheck of AC
                // _PSR as well.
                //
                Notify(\_SB.BAT1, 0x81)
                Notify(\_SB.AC1, 0x80)
            }
            ElseIf(LEqual(Local1, 0x1))
            {
                //
                // Notify OSPM that both battery _BST and AC _PSR have changed
                //
                Notify(\_SB.BAT1, 0x80)
                Notify(\_SB.AC1, 0x80)
            }

            //
            // Clear whatever bits we read as set by writing to the clear
            // location
            //
            Store(Local0, BNCL)
        }
    }
}