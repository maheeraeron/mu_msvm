/*++

Copyright (c) Microsoft Corporation

Module Name:

    Sleep.asl

Abstract:

    Provides ASL definitions for the supported machine sleep states.

--*/

//
// Define the S0 running state. This package is not used, but it is required
// to exist by the specification.
//

Name(\_S0, Package(2){0, 0})

//
// If the VM config's bios_flags has bit 2 set, this triggers
// loading of a table that contains an _S4 object, indicating
// support for hibernation.
//

If(LGreater(HCFG, 0))
{
    //
    // Define the S4 hibernated state. The first package value is the value to
    // write to PM1A_CNT.SLP_TYPE to cause the machine to power off. The second
    // value is for PM1B_CNT.SLP_TYPE, which is not supported by the Hyper-V
    // PM device.
    //
    // The values below are actually the same as the ones for S5, and
    // when they are used, the VM will just be powered off.  If there ever
    // comes a time where the virtual motherboard should act differently
    // when the VM is being powered off in the course of hibernation than it
    // should in the course of a full S5-style shutdown, then these values
    // could be made unique, and then used to differentiate these states.
    //

    Name(\_S4, Package(2){0, 0})
}


//
// Define the S5 powered off state. The first package value is the value to
// write to PM1A_CNT.SLP_TYPE to cause the machine to power off. The second
// value is for PM1B_CNT.SLP_TYPE, which is not supported by the Hyper-V
// PM device.
//

Name(\_S5, Package(2){0, 0})

