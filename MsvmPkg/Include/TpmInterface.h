/*++

Copyright (c) Microsoft Corporation

Module Name:

    TpmInterface.h

Abstract:

    This file contains types and constants shared between
    the VTpmDevice device and the UEFI firmware.

Author:

    Jingbo Wu (jingbowu) 10/2/2013

--*/

#pragma once

//
// TPM register base address.
// See TPM interface Spec (TIS)
// For system software, the TPM has a 64 bit address of 0x0000_0000_FED4_xxxx.
// The south bridge will route the entire address range
// from 0xFED4_0000 through 0xFED4_4FFF to the TPM over SPI.
// Virtual TPM control space uses one page of MMIO range.
//
#define TPM_BASE_ADDRESS            0xfed40000

//
// VTPM configuration ports. Each port is 8-bit long. Accessing 32 bits
// requires 4 ports.
//
// Use a pair of I/O ports to establish TPM 2.0 Command-Response Interface memory.
// It is expected that UEFI TPM driver sets up the GPAs for the communication buffer.
// (TODO: Use MMIO address. Use static well-known memory addresses. One candidate address is
// the one right below APIC.)
//
enum
{
    TpmControlPort         = 0x1040,   // 4 ports for 32-bit access
    TpmDataPort            = 0x1044,   // 4 ports for 32-bit access
};

//
// I/O port command defintiions
//
enum
{
    //
    // It can be used for engine vs. guest version negotiation.
    // Not used.
    //
    TpmIoVersion                = 0,

    //
    // Map command-response interface buffer.
    //
    TpmIoMapSharedMemory        = 1,

    //
    // Query host if map is succeeded.
    //
    TpmIoEstablished            = 2,

    //
    // Get pending TPM operation requested by the OS.
    // PPI function Id 3.
    //
    TpmIoPPIGetPendingOperation = 3, // do not change

    //
    // Get TPM Operation Response to OS.
    // PPI function Id 5
    //
    TpmIoPPIGetLastOperation     = 5,    // do not change
    TpmIoPPIGetLastResult        = 6,    // do not change

    //
    // Set TPM operation requested by the OS.
    // PPI function Id 7
    //
    TpmIoPPISetOperation         = 7,    // do not change

    //
    // Get user confirmation status for operation. Used in PPI over ACPI.
    // PPI function Id 8
    //
    TpmIoPPIGetUserConfirmation  = 8,    // do not change
};
