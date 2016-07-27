/*++

Copyright (c) Microsoft Corporation

Module Name:

    Tpm2.asl

Abstract:

    The primary control structures for a TPM 2.0 device are in the static
    TPM2 table.

--*/

DefinitionBlock (
    "tpm2.aml",
    "OEM2",         // Signature
    2,              // DSDT Compliance revision.
    "MSFTVM",       // OEM ID (6 byte string)
    "VTPM2",        // OEM table ID  (8 byte string) is the manufacturer model ID
    0x00000001      // OEM version of TPM2 table (4 byte Integer)
    )
{
    Scope (\_SB.VMOD)
    {
        Device(TPM2)
        {
            Name (_ADR, 0x00)
            Name (_HID, "VTPM0101")
            Name (_CID, "MSFT0101")
            Name (_UID, 0x01)
            Name (_DDN, "Microsoft Virtual TPM 2.0")
            Name (_STR, Unicode ("Microsoft Virtual TPM 2.0"))

            //
            // Return the resource consumed by TPM device
            //
            Name (_CRS, ResourceTemplate () {
                Memory32Fixed (ReadWrite, 0xfed40000, 0x1000)
            })

            Method (_STA, 0, NotSerialized)
            {
                Return (0x0F)
            }

            //
            // Operational region for TPM general IO port access
            //
            OperationRegion (GIO, SystemIO, 0x1040, 8)
            Field (GIO, DWordAcc, NoLock, Preserve)
            {
                CTLP, 32,      // 32-bit control port
                DATP, 32,      // 32-bit data port
            }

            //
            // BIOS control port and data port
            //
            OperationRegion (GIOB, SystemIO, 0x28, 8)
            Field (GIOB, DWordAcc, NoLock, Preserve)
            {
                CTLB, 32,      // 32-bit control port
                DATB, 32,      // 32-bit data port
            }

            //
            // TCG Physical Presence Interface
            //
            Method (TPPI, 2, Serialized)
            {
                Name(PKG2, Package(){0, 0})

                Name(PKG3, Package(){0, 0, 0})

                //
                // Switch by function index
                //
                Switch (ToInteger(Arg0))
                {
                    Case (0)
                    {
                        //
                        // Func 0 - Standard query, supports function 1-8
                        //
                        Return (Buffer () {0xFF, 0x01})
                    }

                    Case (1)
                    {
                        //
                        // Func 1 - Get Physical Presence Interface Version
                        //
                        Return ("1.3")
                    }

                    Case (2)
                    {
                        //
                        // Func 2 - Submit TPM Operation Request to Pre-OS Environment (Deprecated, Not implemented)
                        //
                        Return (3)
                    }

                    Case (3)
                    {
                        //
                        // Func 3 - Get Pending TPM Operation Requested By the OS
                        //

                        //
                        // Process the request in vDev. IO command is identical to Function Id
                        //
                        Store (3, CTLP)
                        Store (0, Index (PKG2, 0))
                        Store (DATP, Index (PKG2, 1))

                        Return (PKG2)
                    }

                    Case (4)
                    {
                        //
                        // Func 4 - Get Platform-Specific Action to Transition to Pre-OS Environment
                        // Return reboot.
                        //
                        Return (2)
                    }

                    Case (5)
                    {
                        //
                        // Func 5 - Return TPM Operation Response to OS Environment
                        //

                        //
                        // Process the request in vDev. IO command is identical to Function Id
                        //
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
                        //
                        // Func 6 - Submit preferred user language (Not implemented)
                        //
                        Return (3)
                    }

                    Case (7)
                    {
                        //
                        // Func 7 - Submit TPM Operation Request to Pre-OS Environment 2
                        //

                        //
                        // Process the request in vDev. IO command is identical to Function Id
                        //
                        Store (7, CTLP)
                        Store (DerefOf (Index (Arg1, 0)), DATP)

                        Return (DATP)
                    }

                    Case (8)
                    {
                        //
                        // Func 8 - Get User Confirmation Status for Operation
                        //

                        //
                        // Process in vDev. IO command is identical to Function Id
                        //
                        Store (8, CTLP)
                        Store (DerefOf (Index (Arg1, 0)), DATP)

                        Return (DATP)
                    }
                }

                Return (1)
            }


            Method (TMCI, 2, Serialized)
            {
                //
                // Switch by function index
                //
                Switch (ToInteger (Arg0))
                {
                    Case (0)
                    {
                        //
                        // Standard query, supports function 1-1
                        //
                        Return (Buffer () {0x03})
                    }

                    Case (1)
                    {
                        Store (0x31, CTLB)
                        Store (DerefOf (Index (Arg1, 0)), DATB)

                        return (DATB)
                    }
                }

                Return (1)
            }


            Method (_DSM, 4, Serialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
            {
                //
                // TCG Physical Presence Interface
                //
                If (LEqual(Arg0, ToUUID ("3dddfaa6-361b-4eb4-a424-8d10089d1653")))
                {
                    Return (TPPI(Arg2, Arg3))
                }

                //
                // TCG Memory Clear Interface
                //
                If (LEqual(Arg0, ToUUID ("376054ed-cc13-4675-901c-4756d7f2d45d")))
                {
                    Return (TMCI (Arg2, Arg3))
                }

                //
                // If not one of the function identifiers we recognize, then return a buffer
                // with bit 0 set to 0 indicating no functions supported.
                //
                Return (Buffer () {0})
            }
        }
    }
}
