/*++

Copyright (c) Microsoft Corporation

Module Name:

    Dsdt.asl

Abstract:

    The top-level DSDT table. Establishes the table and includes additional
    files containing the majority of the configuration.

Author:

    Rich Yampell (richyam) 18-Jul-2012

--*/

DefinitionBlock (
        "Dsdt.aml",
        "DSDT",
        0x02,           // DSDT Compliance revision.
                        // A Revision field value greater than or equal to 2 signifies that integers
                        // declared within the Definition Block are to be evaluated as 64-bit values
        "MSFTVM",       // OEM ID (6 byte string)
        "DSDT01",       // OEM table ID  (8 byte string)
        0x01            // OEM version of DSDT table (4 byte Integer)
        )

{
    Scope(_SB)
    {
        Method(_INI)
        {
            //
            // Load the COM port UARTs if configured.
            //
            If(LGreater(SCFG, 0))
            {
                LoadTable("OEM1", "MSFTVM", "UARTS",,,)
            }

            //
            // Load the test hook table if configured.
            //
            If(LGreater(PCFG, 0))
            {
                LoadTable("OEMP", "MSFTVM", "SPCI",,,)
            }

            //
            // Load the TPM device if configured.
            //
            If(LGreater(TCFG, 0))
            {
                LoadTable("OEM2", "MSFTVM", "VTPM2",,,)
            }

            //
            // Load the virtual battery if configured.
            //
            If(LGreater(BCFG, 0))
            {
                LoadTable("OEM4", "MSFTVM", "VBAT")
            }
        }
    }

    Include("Asl/AmlUpd.asl")

    // The Enclave Page Cache aka SGX memory device. This device is conditionally 
    // created if SGX memory is present. This is not Intel spec compliant in that
    // it doesn't have any memory regions described in the _CRS. Existence of this
    // device will trigger a guest kernel to load a device driver. That device 
    // driver will use other mechanisms to discover the SGX memory regions.
    
    If(LGreater(SGXE, 0))
    {
        Scope(_SB)
        {
            Device(EPC)
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
    }

    
    Include("Asl/Processor.asl")
    Include("Asl/Synthetic.asl")
    Include("Asl/Lpc.asl")
    Include("Asl/Sleep.asl")


    //
    // Load the NVDIMMs if configured.
    //
    If(LGreater(NCFG, 0))
    {
        Include("Asl/Nvdm.asl")
    }
}

