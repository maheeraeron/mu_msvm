/*++

Copyright (c) Microsoft Corporation

Module Name:

    Nvdm.asl

Abstract:

    A table (Nvdm) that contains the declarations of the NVDIMM root device
    and any NVDIMM child devices.

--*/


//
// General Purpose Event Scope
//
Scope(\_GPE)
{
    //
    // Method for notifying external changes to the control
    // method battery:
    //      E  - This event is edge triggered
    //      0A - Use bit 10 (in hex!) in the General Purpose Event register described
    //           in the FADT
    Method(_E0A)
    {
        // Read the Event registers.
        Store(\_SB.NVDR.NEV0, Local0)
        Store(\_SB.NVDR.NEV1, Local1)
        Store(\_SB.NVDR.NEV2, Local2)
        Store(\_SB.NVDR.NEV3, Local3)
        Store(\_SB.NVDR.NEV4, Local4)

        //
        // Go through each event register to see what events were signalled.
        // For NEV0-3, each bit corresponds to a 0x81 event on a different NVDIMM
        // child device.
        // For NEV4, bit 0 corresponds to 0x80 (NFIT Update Notification), and
        // bit 1 corresponds to 0x81 (Unconsumed Uncorrectable Memory Error Detected),
        // both on the NVDIMM root device.
        //
        if (LNotEqual(Local0, 0))
        {
            if (LNotEqual( And(Local0, 0x00000001), 0))
            {
                Notify (\_SB.NVDR.N000, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00000002), 0))
            {
                Notify (\_SB.NVDR.N001, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00000004), 0))
            {
                Notify (\_SB.NVDR.N002, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00000008), 0))
            {
                Notify (\_SB.NVDR.N003, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00000010), 0))
            {
                Notify (\_SB.NVDR.N004, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00000020), 0))
            {
                Notify (\_SB.NVDR.N005, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00000040), 0))
            {
                Notify (\_SB.NVDR.N006, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00000080), 0))
            {
                Notify (\_SB.NVDR.N007, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00000100), 0))
            {
                Notify (\_SB.NVDR.N008, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00000200), 0))
            {
                Notify (\_SB.NVDR.N009, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00000400), 0))
            {
                Notify (\_SB.NVDR.N010, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00000800), 0))
            {
                Notify (\_SB.NVDR.N011, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00001000), 0))
            {
                Notify (\_SB.NVDR.N012, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00002000), 0))
            {
                Notify (\_SB.NVDR.N013, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00004000), 0))
            {
                Notify (\_SB.NVDR.N014, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00008000), 0))
            {
                Notify (\_SB.NVDR.N015, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00010000), 0))
            {
                Notify (\_SB.NVDR.N016, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00020000), 0))
            {
                Notify (\_SB.NVDR.N017, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00040000), 0))
            {
                Notify (\_SB.NVDR.N018, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00080000), 0))
            {
                Notify (\_SB.NVDR.N019, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00100000), 0))
            {
                Notify (\_SB.NVDR.N020, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00200000), 0))
            {
                Notify (\_SB.NVDR.N021, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00400000), 0))
            {
                Notify (\_SB.NVDR.N022, 0x81)
            }

            if (LNotEqual( And(Local0, 0x00800000), 0))
            {
                Notify (\_SB.NVDR.N023, 0x81)
            }

            if (LNotEqual( And(Local0, 0x01000000), 0))
            {
                Notify (\_SB.NVDR.N024, 0x81)
            }

            if (LNotEqual( And(Local0, 0x02000000), 0))
            {
                Notify (\_SB.NVDR.N025, 0x81)
            }

            if (LNotEqual( And(Local0, 0x04000000), 0))
            {
                Notify (\_SB.NVDR.N026, 0x81)
            }

            if (LNotEqual( And(Local0, 0x08000000), 0))
            {
                Notify (\_SB.NVDR.N027, 0x81)
            }

            if (LNotEqual( And(Local0, 0x10000000), 0))
            {
                Notify (\_SB.NVDR.N028, 0x81)
            }

            if (LNotEqual( And(Local0, 0x20000000), 0))
            {
                Notify (\_SB.NVDR.N029, 0x81)
            }

            if (LNotEqual( And(Local0, 0x40000000), 0))
            {
                Notify (\_SB.NVDR.N030, 0x81)
            }

            if (LNotEqual( And(Local0, 0x80000000), 0))
            {
                Notify (\_SB.NVDR.N031, 0x81)
            }

            //
            // Clear the event register.
            //
            Store (Local0, \_SB.NVDR.NEV0)
        }

        if (LNotEqual(Local1, 0))
        {
            if (LNotEqual( And(Local1, 0x00000001), 0))
            {
                Notify (\_SB.NVDR.N032, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00000002), 0))
            {
                Notify (\_SB.NVDR.N033, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00000004), 0))
            {
                Notify (\_SB.NVDR.N034, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00000008), 0))
            {
                Notify (\_SB.NVDR.N035, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00000010), 0))
            {
                Notify (\_SB.NVDR.N036, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00000020), 0))
            {
                Notify (\_SB.NVDR.N037, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00000040), 0))
            {
                Notify (\_SB.NVDR.N038, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00000080), 0))
            {
                Notify (\_SB.NVDR.N039, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00000100), 0))
            {
                Notify (\_SB.NVDR.N040, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00000200), 0))
            {
                Notify (\_SB.NVDR.N041, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00000400), 0))
            {
                Notify (\_SB.NVDR.N042, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00000800), 0))
            {
                Notify (\_SB.NVDR.N043, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00001000), 0))
            {
                Notify (\_SB.NVDR.N044, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00002000), 0))
            {
                Notify (\_SB.NVDR.N045, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00004000), 0))
            {
                Notify (\_SB.NVDR.N046, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00008000), 0))
            {
                Notify (\_SB.NVDR.N047, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00010000), 0))
            {
                Notify (\_SB.NVDR.N048, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00020000), 0))
            {
                Notify (\_SB.NVDR.N049, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00040000), 0))
            {
                Notify (\_SB.NVDR.N050, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00080000), 0))
            {
                Notify (\_SB.NVDR.N051, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00100000), 0))
            {
                Notify (\_SB.NVDR.N052, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00200000), 0))
            {
                Notify (\_SB.NVDR.N053, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00400000), 0))
            {
                Notify (\_SB.NVDR.N054, 0x81)
            }

            if (LNotEqual( And(Local1, 0x00800000), 0))
            {
                Notify (\_SB.NVDR.N055, 0x81)
            }

            if (LNotEqual( And(Local1, 0x01000000), 0))
            {
                Notify (\_SB.NVDR.N056, 0x81)
            }

            if (LNotEqual( And(Local1, 0x02000000), 0))
            {
                Notify (\_SB.NVDR.N057, 0x81)
            }

            if (LNotEqual( And(Local1, 0x04000000), 0))
            {
                Notify (\_SB.NVDR.N058, 0x81)
            }

            if (LNotEqual( And(Local1, 0x08000000), 0))
            {
                Notify (\_SB.NVDR.N059, 0x81)
            }

            if (LNotEqual( And(Local1, 0x10000000), 0))
            {
                Notify (\_SB.NVDR.N060, 0x81)
            }

            if (LNotEqual( And(Local1, 0x20000000), 0))
            {
                Notify (\_SB.NVDR.N061, 0x81)
            }

            if (LNotEqual( And(Local1, 0x40000000), 0))
            {
                Notify (\_SB.NVDR.N062, 0x81)
            }

            if (LNotEqual( And(Local1, 0x80000000), 0))
            {
                Notify (\_SB.NVDR.N063, 0x81)
            }

            //
            // Clear the event register.
            //
            Store (Local1, \_SB.NVDR.NEV1)
        }

        if (LNotEqual(Local2, 0))
        {
            if (LNotEqual( And(Local2, 0x00000001), 0))
            {
                Notify (\_SB.NVDR.N064, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00000002), 0))
            {
                Notify (\_SB.NVDR.N065, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00000004), 0))
            {
                Notify (\_SB.NVDR.N066, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00000008), 0))
            {
                Notify (\_SB.NVDR.N067, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00000010), 0))
            {
                Notify (\_SB.NVDR.N068, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00000020), 0))
            {
                Notify (\_SB.NVDR.N069, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00000040), 0))
            {
                Notify (\_SB.NVDR.N070, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00000080), 0))
            {
                Notify (\_SB.NVDR.N071, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00000100), 0))
            {
                Notify (\_SB.NVDR.N072, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00000200), 0))
            {
                Notify (\_SB.NVDR.N073, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00000400), 0))
            {
                Notify (\_SB.NVDR.N074, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00000800), 0))
            {
                Notify (\_SB.NVDR.N075, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00001000), 0))
            {
                Notify (\_SB.NVDR.N076, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00002000), 0))
            {
                Notify (\_SB.NVDR.N077, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00004000), 0))
            {
                Notify (\_SB.NVDR.N078, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00008000), 0))
            {
                Notify (\_SB.NVDR.N079, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00010000), 0))
            {
                Notify (\_SB.NVDR.N080, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00020000), 0))
            {
                Notify (\_SB.NVDR.N081, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00040000), 0))
            {
                Notify (\_SB.NVDR.N082, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00080000), 0))
            {
                Notify (\_SB.NVDR.N083, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00100000), 0))
            {
                Notify (\_SB.NVDR.N084, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00200000), 0))
            {
                Notify (\_SB.NVDR.N085, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00400000), 0))
            {
                Notify (\_SB.NVDR.N086, 0x81)
            }

            if (LNotEqual( And(Local2, 0x00800000), 0))
            {
                Notify (\_SB.NVDR.N087, 0x81)
            }

            if (LNotEqual( And(Local2, 0x01000000), 0))
            {
                Notify (\_SB.NVDR.N088, 0x81)
            }

            if (LNotEqual( And(Local2, 0x02000000), 0))
            {
                Notify (\_SB.NVDR.N089, 0x81)
            }

            if (LNotEqual( And(Local2, 0x04000000), 0))
            {
                Notify (\_SB.NVDR.N090, 0x81)
            }

            if (LNotEqual( And(Local2, 0x08000000), 0))
            {
                Notify (\_SB.NVDR.N091, 0x81)
            }

            if (LNotEqual( And(Local2, 0x10000000), 0))
            {
                Notify (\_SB.NVDR.N092, 0x81)
            }

            if (LNotEqual( And(Local2, 0x20000000), 0))
            {
                Notify (\_SB.NVDR.N093, 0x81)
            }

            if (LNotEqual( And(Local2, 0x40000000), 0))
            {
                Notify (\_SB.NVDR.N094, 0x81)
            }

            if (LNotEqual( And(Local2, 0x80000000), 0))
            {
                Notify (\_SB.NVDR.N095, 0x81)
            }

            //
            // Clear the event register.
            //
            Store (Local2, \_SB.NVDR.NEV2)
        }

        if (LNotEqual(Local3, 0))
        {
            if (LNotEqual( And(Local3, 0x00000001), 0))
            {
                Notify (\_SB.NVDR.N096, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00000002), 0))
            {
                Notify (\_SB.NVDR.N097, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00000004), 0))
            {
                Notify (\_SB.NVDR.N098, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00000008), 0))
            {
                Notify (\_SB.NVDR.N099, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00000010), 0))
            {
                Notify (\_SB.NVDR.N100, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00000020), 0))
            {
                Notify (\_SB.NVDR.N101, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00000040), 0))
            {
                Notify (\_SB.NVDR.N102, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00000080), 0))
            {
                Notify (\_SB.NVDR.N103, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00000100), 0))
            {
                Notify (\_SB.NVDR.N104, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00000200), 0))
            {
                Notify (\_SB.NVDR.N105, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00000400), 0))
            {
                Notify (\_SB.NVDR.N106, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00000800), 0))
            {
                Notify (\_SB.NVDR.N107, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00001000), 0))
            {
                Notify (\_SB.NVDR.N108, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00002000), 0))
            {
                Notify (\_SB.NVDR.N109, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00004000), 0))
            {
                Notify (\_SB.NVDR.N110, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00008000), 0))
            {
                Notify (\_SB.NVDR.N111, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00010000), 0))
            {
                Notify (\_SB.NVDR.N112, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00020000), 0))
            {
                Notify (\_SB.NVDR.N113, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00040000), 0))
            {
                Notify (\_SB.NVDR.N114, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00080000), 0))
            {
                Notify (\_SB.NVDR.N115, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00100000), 0))
            {
                Notify (\_SB.NVDR.N116, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00200000), 0))
            {
                Notify (\_SB.NVDR.N117, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00400000), 0))
            {
                Notify (\_SB.NVDR.N118, 0x81)
            }

            if (LNotEqual( And(Local3, 0x00800000), 0))
            {
                Notify (\_SB.NVDR.N119, 0x81)
            }

            if (LNotEqual( And(Local3, 0x01000000), 0))
            {
                Notify (\_SB.NVDR.N120, 0x81)
            }

            if (LNotEqual( And(Local3, 0x02000000), 0))
            {
                Notify (\_SB.NVDR.N121, 0x81)
            }

            if (LNotEqual( And(Local3, 0x04000000), 0))
            {
                Notify (\_SB.NVDR.N122, 0x81)
            }

            if (LNotEqual( And(Local3, 0x08000000), 0))
            {
                Notify (\_SB.NVDR.N123, 0x81)
            }

            if (LNotEqual( And(Local3, 0x10000000), 0))
            {
                Notify (\_SB.NVDR.N124, 0x81)
            }

            if (LNotEqual( And(Local3, 0x20000000), 0))
            {
                Notify (\_SB.NVDR.N125, 0x81)
            }

            if (LNotEqual( And(Local3, 0x40000000), 0))
            {
                Notify (\_SB.NVDR.N126, 0x81)
            }

            if (LNotEqual( And(Local3, 0x80000000), 0))
            {
                Notify (\_SB.NVDR.N127, 0x81)
            }

            //
            // Clear the event register.
            //
            Store (Local3, \_SB.NVDR.NEV3)
        }

        if (LNotEqual(Local4, 0))
        {
            if (LNotEqual(And(Local4, 0x1), 0))
            {
                Notify (\_SB.NVDR, 0x80)
            }

            if (LNotEqual(And(Local4, 0x2), 0))
            {
                Notify (\_SB.NVDR, 0x81)
            }

            //
            // Clear the event register.
            //
            Store (Local4, \_SB.NVDR.NEV4)
        }
    }
}

Scope(\_SB)
{

    //
    // NVDIMM Root Device
    //

    Device(NVDR)
    {
        //
        // Operation Region used for MMIO to signal to the
        // host vdev that a method is being called.
        // We split up the MMIO region into offsets to indicate
        // to the host which method is being called on which
        // device.
        //
        OperationRegion(NVIO, SystemMemory, FixedPcdGet32(PcdPmemBase), 4096)
        Field(NVIO, DWordAcc, NoLock, WriteAsZeros)
        {
            RDSM,32, // Root  _DSM
            CDSM,32, // Child _DSM
            CLSI,32, // Child _LSI
            CLSR,32, // Child _LSR
            NEV0,32, // NEV# is a NVDIMM Event Register.
            NEV1,32, // NEV0 - NEV3 correspond to NFIT Health Event Notification (0x81 on NVDIMM Child Device).
            NEV2,32, //
            NEV3,32, //
            NEV4,32, // Corresponds to NVDIMM Root Device notifications.
        }

        //
        // Operation Region for Method I/O buffer between the
        // ACPI NVDIMM devices and the vPMEM vdev on the Host.
        // This address gets updated by the EFI firmware during
        // ACPI table initialization.
        //
        OperationRegion(NVDB, SystemMemory, NVDA, 4096)
        Field(NVDB, AnyAcc, NoLock, WriteAsZeros)
        {
            MBUF,32736, // Raw buffer that can be returned to callers (4k bytes - 4).
            MBFL,32,   // Size of the data that has been returned.
        }

        Name (_HID, "ACPI0012")
        Name (_STA, 0xF)

        //
        // This mutex protects the above NVDB OperationRegion.
        // It should be used as follows.
        // 1. Acquire NMTX
        // 2. Store method arguments in MBUF.
        // 3. Signal the vdev via the NVIO MMIO page.
        // 4. Read the return values from MBUF into scratch space (of length MBFL).
        // 5. Release NMTX.
        //
        Mutex(NMTX, 0)


        // _DSM – Device Specific Method
        //
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
                                //
                                // Function 0: Return supported functions.
                                //
                                //
                                Case (0)
                                {
                                    //
                                    // Read from the MMIO page to get the supported functions.
                                    //
                                    Return (RDSM)
                                }

                                //
                                // For all other functions, call into the vdev.
                                //
                                Default
                                {
                                    Acquire (NMTX, 0xFFFF)

                                    //
                                    // Copy the arguments into the method I/O buffer.
                                    //
                                    Store (DeRefOf(Index(Arg3,0)), MBUF)

                                    //
                                    // Write the function index to the MMIO Page
                                    // to signal the vdev.
                                    //
                                    Store (Arg2, RDSM)

                                    //
                                    // Copy the contents of the method I/O Buffer.
                                    //
                                    Name (RBUF, Buffer(MBFL) {})
                                    Multiply (MBFL, 8, Local0)
                                    CreateField (RBUF, 0, Local0, RBFF)     
                                    Store (MBUF, RBFF)

                                    Release (NMTX)
                                    Return (RBUF)
                                }
                            }
                        }
                        Default
                        {
                            //
                            // Return a buffer with bit 0 set to 0 indicating no functions supported
                            // if we don't recognize the revision level.
                            //
                            Return (Buffer(){0})
                        }
                    }
                }
            }

            //
            // Return a buffer with bit 0 set to 0 indicating no functions supported
            // if we don't recognize the UUID.
            //
            Return (Buffer(){0})
        }

        // CDSF - Generic Method for Child _DSMs.
        //
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
                                //
                                // Function 0: Return supported functions.
                                //
                                //
                                Case (0)
                                {
                                    //
                                    // Read from the MMIO page to get the supported functions.
                                    //
                                    Return (CDSM)
                                }

                                //
                                // For all other functions, call into the vdev.
                                //
                                Default
                                {
                                    // We need to pack the function index and device index into a DWORD.
                                    Name (INDX, Buffer(4) {})
                                    CreateField (INDX, 0, 16, FIND) // Space for Function Index
                                    CreateField(INDX, 16, 16, DIND) // Space for Device Index
                                    Store (Arg2, FIND)
                                    Store (Arg4, DIND)

                                    Acquire (NMTX, 0xFFFF)

                                    //
                                    // Copy the arguments into the method I/O buffer.
                                    //
                                    Store (DeRefOf(Index(Arg3,0)), MBUF)

                                    //
                                    // Write the function and device indices
                                    // to the MMIO Page to signal the vdev.
                                    //
                                    Store (INDX, CDSM)

                                    //
                                    // Copy the contents of the method I/O Buffer.
                                    //
                                    Name (RBUF, Buffer(MBFL) {})
                                    Multiply (MBFL, 8, Local0)
                                    CreateField (RBUF, 0, Local0, RBFF)                             
                                    Store (MBUF, RBFF)

                                    Release (NMTX)
                                    Return (RBUF)
                                }
                            }
                        }
                        Default
                        {
                            //
                            // Return a buffer with bit 0 set to 0 indicating no functions supported
                            // if we don't recognize the revision level.
                            //
                            Return (Buffer(){0})
                        }
                    }
                }
            }

            //
            // Return a buffer with bit 0 set to 0 indicating no functions supported
            // if we don't recognize the UUID.
            //
            Return (Buffer(){0})
        }

        // LSIM - Generic Method for Child _LSIs.
        //
        // Arg0: Integer Device Index
        Method (LSIM, 1, Serialized, 0, {BuffObj})
        {
            Acquire (NMTX, 0xFFFF)

            //
            // Write the device index
            // to the MMIO Page to signal the vdev.
            //
            Store (Arg0, CLSI)

            //
            // Copy the contents of the method I/O Buffer.
            //
            Name (RBUF, Buffer(MBFL) {})
            Multiply (MBFL, 8, Local0)
            CreateField (RBUF, 0, Local0, RBFF)                                       
            Store (MBUF, RBFF)

            Release (NMTX)
            Return (RBUF)
        }

        // LSRM - Generic Method for Child _LSRs.
        //
        // Arg0: Integer(DWORD) Byte Offset.
        // Arg1: Integer(DWORD) Tranfer Byte Length.
        // Arg2: Integer Device Index.
        Method (LSRM, 3, Serialized, 0, {BuffObj})
        {
            //
            // Pack up the arguments.
            //
            Name (INPT, Buffer(4) {})
            CreateField (INPT, 0, 32, BTOF) // Space for Byte Offset
            CreateField (INPT, 32, 32, TFLT) // Space for Transfer Length
            Store (Arg0, BTOF)
            Store (Arg1, TFLT)

            Acquire (NMTX, 0xFFFF)

            //
            // Copy the arguments.
            //
            CopyObject (INPT, MBUF)

            //
            // Write the device index
            // to the MMIO Page to signal the vdev.
            //
            Store (Arg2, CLSR)

            //
            // Copy the contents of the method I/O Buffer.
            //
            Name (RBUF, Buffer(MBFL) {})
            Multiply (MBFL, 8, Local0)
            CreateField (RBUF, 0, Local0, RBFF)                                   
            Store (MBUF, RBFF)

            Release (NMTX)
            Return (RBUF)
        }

        //
        // NVDIMM Child Devices
        // These child devices are generated via script,
        // and each method calls into the generic methods above.
        //

        Device(N000)
        {
            Name (_ADR, 0)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N001)
        {
            Name (_ADR, 1)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N002)
        {
            Name (_ADR, 2)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N003)
        {
            Name (_ADR, 3)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N004)
        {
            Name (_ADR, 4)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N005)
        {
            Name (_ADR, 5)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N006)
        {
            Name (_ADR, 6)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N007)
        {
            Name (_ADR, 7)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N008)
        {
            Name (_ADR, 8)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N009)
        {
            Name (_ADR, 9)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N010)
        {
            Name (_ADR, 10)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N011)
        {
            Name (_ADR, 11)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N012)
        {
            Name (_ADR, 12)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N013)
        {
            Name (_ADR, 13)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N014)
        {
            Name (_ADR, 14)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N015)
        {
            Name (_ADR, 15)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N016)
        {
            Name (_ADR, 16)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N017)
        {
            Name (_ADR, 17)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N018)
        {
            Name (_ADR, 18)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N019)
        {
            Name (_ADR, 19)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N020)
        {
            Name (_ADR, 20)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N021)
        {
            Name (_ADR, 21)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N022)
        {
            Name (_ADR, 22)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N023)
        {
            Name (_ADR, 23)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N024)
        {
            Name (_ADR, 24)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N025)
        {
            Name (_ADR, 25)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N026)
        {
            Name (_ADR, 26)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N027)
        {
            Name (_ADR, 27)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N028)
        {
            Name (_ADR, 28)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N029)
        {
            Name (_ADR, 29)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N030)
        {
            Name (_ADR, 30)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N031)
        {
            Name (_ADR, 31)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N032)
        {
            Name (_ADR, 32)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N033)
        {
            Name (_ADR, 33)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N034)
        {
            Name (_ADR, 34)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N035)
        {
            Name (_ADR, 35)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N036)
        {
            Name (_ADR, 36)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N037)
        {
            Name (_ADR, 37)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N038)
        {
            Name (_ADR, 38)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N039)
        {
            Name (_ADR, 39)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N040)
        {
            Name (_ADR, 40)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N041)
        {
            Name (_ADR, 41)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N042)
        {
            Name (_ADR, 42)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N043)
        {
            Name (_ADR, 43)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N044)
        {
            Name (_ADR, 44)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N045)
        {
            Name (_ADR, 45)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N046)
        {
            Name (_ADR, 46)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N047)
        {
            Name (_ADR, 47)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N048)
        {
            Name (_ADR, 48)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N049)
        {
            Name (_ADR, 49)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N050)
        {
            Name (_ADR, 50)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N051)
        {
            Name (_ADR, 51)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N052)
        {
            Name (_ADR, 52)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N053)
        {
            Name (_ADR, 53)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N054)
        {
            Name (_ADR, 54)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N055)
        {
            Name (_ADR, 55)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N056)
        {
            Name (_ADR, 56)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N057)
        {
            Name (_ADR, 57)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N058)
        {
            Name (_ADR, 58)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N059)
        {
            Name (_ADR, 59)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N060)
        {
            Name (_ADR, 60)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N061)
        {
            Name (_ADR, 61)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N062)
        {
            Name (_ADR, 62)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N063)
        {
            Name (_ADR, 63)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N064)
        {
            Name (_ADR, 64)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N065)
        {
            Name (_ADR, 65)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N066)
        {
            Name (_ADR, 66)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N067)
        {
            Name (_ADR, 67)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N068)
        {
            Name (_ADR, 68)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N069)
        {
            Name (_ADR, 69)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N070)
        {
            Name (_ADR, 70)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N071)
        {
            Name (_ADR, 71)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N072)
        {
            Name (_ADR, 72)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N073)
        {
            Name (_ADR, 73)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N074)
        {
            Name (_ADR, 74)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N075)
        {
            Name (_ADR, 75)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N076)
        {
            Name (_ADR, 76)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N077)
        {
            Name (_ADR, 77)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N078)
        {
            Name (_ADR, 78)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N079)
        {
            Name (_ADR, 79)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N080)
        {
            Name (_ADR, 80)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N081)
        {
            Name (_ADR, 81)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N082)
        {
            Name (_ADR, 82)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N083)
        {
            Name (_ADR, 83)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N084)
        {
            Name (_ADR, 84)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N085)
        {
            Name (_ADR, 85)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N086)
        {
            Name (_ADR, 86)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N087)
        {
            Name (_ADR, 87)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N088)
        {
            Name (_ADR, 88)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N089)
        {
            Name (_ADR, 89)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N090)
        {
            Name (_ADR, 90)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N091)
        {
            Name (_ADR, 91)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N092)
        {
            Name (_ADR, 92)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N093)
        {
            Name (_ADR, 93)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N094)
        {
            Name (_ADR, 94)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N095)
        {
            Name (_ADR, 95)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N096)
        {
            Name (_ADR, 96)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N097)
        {
            Name (_ADR, 97)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N098)
        {
            Name (_ADR, 98)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N099)
        {
            Name (_ADR, 99)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N100)
        {
            Name (_ADR, 100)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N101)
        {
            Name (_ADR, 101)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N102)
        {
            Name (_ADR, 102)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N103)
        {
            Name (_ADR, 103)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N104)
        {
            Name (_ADR, 104)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N105)
        {
            Name (_ADR, 105)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N106)
        {
            Name (_ADR, 106)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N107)
        {
            Name (_ADR, 107)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N108)
        {
            Name (_ADR, 108)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N109)
        {
            Name (_ADR, 109)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N110)
        {
            Name (_ADR, 110)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N111)
        {
            Name (_ADR, 111)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N112)
        {
            Name (_ADR, 112)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N113)
        {
            Name (_ADR, 113)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N114)
        {
            Name (_ADR, 114)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N115)
        {
            Name (_ADR, 115)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N116)
        {
            Name (_ADR, 116)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N117)
        {
            Name (_ADR, 117)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N118)
        {
            Name (_ADR, 118)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N119)
        {
            Name (_ADR, 119)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N120)
        {
            Name (_ADR, 120)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N121)
        {
            Name (_ADR, 121)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N122)
        {
            Name (_ADR, 122)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N123)
        {
            Name (_ADR, 123)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N124)
        {
            Name (_ADR, 124)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N125)
        {
            Name (_ADR, 125)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N126)
        {
            Name (_ADR, 126)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }

        Device(N127)
        {
            Name (_ADR, 127)
            Method (_DSM, 4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                Return (CDSF(Arg0, Arg1, Arg2, Arg3, _ADR))
            }

            Function (_LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (_LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }
    }
}