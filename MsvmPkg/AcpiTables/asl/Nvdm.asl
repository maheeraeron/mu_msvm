/*++

Copyright (c) Microsoft Corporation

Module Name:

    Nvdm.asl

Abstract:

    A table (Nvdm) that contains the declarations of the NVDIMM root device
    and any NVDIMM child devices.

--*/
Scope(\_SB)
{

    //
    // NVDIMM Root Device
    //

    Device(NVDR)
    {

        //
        // Operation Region for Method I/O buffer between the 
        // ACPI NVDIMM devices and the vPMEM vdev on the Host.
        // This address gets updated by the EFI firmware during
        // ACPI table initialization.
        //
        OperationRegion(NVDB, SystemMemory, NVDA, 4096)
        Field(NVDB, AnyAcc, NoLock, WriteAsZeros)
        {
            MBUF,4096 // Raw buffer that can be returned to callers
        }
        
        Name (_HID, "ACPI0012")
        Name (_STA, 0xF)

        //
        // This mutex protects the above NVDB OperationRegion. 
        // It should be used as follows.
        // 1. Acquire NMTX
        // 2. Store method arguments in MBUF.
        // 3. Signal the vdev via the NVIO MMIO page.
        // 4. Read the return values from MBUF into scratch space.
        // 5. Release NMTX.
        //
        Mutex(NMTX, 0)

        //
        // Operation Region used for MMIO to signal to the
        // host vdev that a method is being called.
        // We split up the MMIO region into offsets to indicate
        // to the host which method is being called on which
        // device.
        //
        OperationRegion(NVIO, SystemMemory, 0xfed3d000, 4096)
        Field(NVIO, DWordAcc, NoLock, WriteAsZeros)
        {
            RDSM,32, // Root  _DSM
            CDSM,32, // Child _DSM
            CLSI,32, // Child _LSI
            CLSR,32, // Child _LSR
        }


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
                                    Name (RBUF, Buffer(4096) {})
                                    CopyObject (MBUF, RBUF)

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
                                    Name (RBUF, Buffer(4096) {})
                                    CopyObject (MBUF, RBUF)

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
            Name (RBUF, Buffer(4096) {})
            CopyObject (MBUF, RBUF)

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
            Name (RBUF, Buffer(4096) {})
            CopyObject (MBUF, RBUF)

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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
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

            Function (LSI, {BuffObj})
            {
                Return (LSIM(_ADR))
            }

            Function (LSR, {BuffObj}, {IntObj, IntObj})
            {
                Return (LSRM(Arg0, Arg1, _ADR))
            }
        }
    }
}