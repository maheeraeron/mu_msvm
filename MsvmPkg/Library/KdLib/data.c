/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    data.c

Abstract:

    This module contains global data for the boot debugger.

Author:

    David N. Cutler (davec) 27-Nov-1996

--*/

#include "EfiKd.h"

//
// Define boot debugger data.
//
// Breakpoint instruction.
//

EFI_KD_BREAKPOINT_TYPE EfiKdBreakpointInstruction;

//
// Break point table.
//

BREAKPOINT_ENTRY EfiKdBreakpointTable[BREAKPOINT_TABLE_SIZE] = {0};

//
// Control C pressed and control C pending.
//

LOGICAL EfiKdControlCPending = FALSE;
LOGICAL EfiKdControlCPressed = FALSE;

//
// The trap handlers that fundamentally enable the boot debugger are managed
// by architecture specific code.  Supply a global toggle used by the
// architecture specific code to indicate whether these fundamental handlers
// are in place.  The presence or absence of the handlers determines whether
// debugger operations can be carried out successfully.
//
// N.B. All updates to this flag are applied by architecture specific code.
//

LOGICAL EfiKdArchBlockDebuggerOperation = TRUE;

//
// Debugger not present.
//

volatile LOGICAL EfiKdDebuggerNotPresent = FALSE;

//
// Message buffer.
//
// N.B. The message buffer size is guaranteed to be 0 mod 8.
//

UINT64 EfiKdMessageBuffer[EFI_KD_MESSAGE_BUFFER_SIZE / 8];

//
// Next packet id to send and next packet id to expect.
//

UINT32 EfiKdPacketIdExpected;
UINT32 EfiKdNextPacketIdToSend;

//
// Processor control block used to saved processor state.
//

PKPRCB EfiKdPrcb;

//
// Processor control region structure.
//

KPCR EfiKdPcr;

//
// Number of retries and the retry count.
//

UINT32 EfiKdNumberRetries = 5;
UINT32 EfiKdRetryCount = 5;

//
// NT build number.
//

#if DBG

const UINT32 NtBuildNumber = VER_PRODUCTBUILD | 0xc0000000;

#else

const UINT32 NtBuildNumber = VER_PRODUCTBUILD | 0xf0000000;

#endif

//
// Maximum KD transport packet size.
//

UINT32 KdTransportMaxPacketSize;

//
// DebugPrint filter to OR with the per-component mask.
//

UINT32 EfiKdDebugPrintGlobalMask;

//
//  Indicates if the debugger has been fully initialized.
//
BOOLEAN EfiKdSubsystemInitialized = FALSE;

//
// Indicates whether or not the debugger subsystem
// can be initialized.  This is controlled by the 
// BIOS_CONFIG_FLAGS_ENABLE_DEBUGGER flag which can be set
// in the bios_flags in the VMs configuration file.
//
BOOLEAN EfiKdSubsystemEnabled = FALSE;

//
// Used to track unloaded UEFI modules.
//
//
ULONG EfiKdLastUnloadedModule = 0;
EFI_UNLOADED_MODULE EfiKdUnloadedModules[EFI_KD_MAX_UNLOADED_MODULES];


