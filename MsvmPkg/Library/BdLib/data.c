/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    data.c

Abstract:

    This module contains global data for the boot debugger.

Author:

    David N. Cutler (davec) 27-Nov-1996

--*/

#include "Bd.h"

//
// Define boot debugger data.
//
// Breakpoint instruction.
//

BD_BREAKPOINT_TYPE BdBreakpointInstruction;

//
// Break point table.
//

BREAKPOINT_ENTRY BdBreakpointTable[BREAKPOINT_TABLE_SIZE] = {0};

//
// Control C pressed and control C pending.
//

LOGICAL BdControlCPending = FALSE;
LOGICAL BdControlCPressed = FALSE;

//
// The trap handlers that fundamentally enable the boot debugger are managed
// by architecture specific code.  Supply a global toggle used by the
// architecture specific code to indicate whether these fundamental handlers
// are in place.  The presence or absence of the handlers determines whether
// debugger operations can be carried out successfully.
//
// N.B. All updates to this flag are applied by architecture specific code.
//

LOGICAL BdArchBlockDebuggerOperation = TRUE;

//
// Debugger not present.
//

volatile UINT32 BdDebuggerNotPresent = FALSE;

//
// Message buffer.
//
// N.B. The message buffer size is guaranteed to be 0 mod 8.
//

UINT64 BdMessageBuffer[BD_MESSAGE_BUFFER_SIZE / 8];

//
// Next packet id to send and next packet id to expect.
//

UINT32 BdPacketIdExpected;
UINT32 BdNextPacketIdToSend;

//
// Processor control block used to saved processor state.
//

PKPRCB BdPrcb;

//
// Processor control region structure.
//

KPCR BdPcr;

//
// Number of retries and the retry count.
//

UINT32 BdNumberRetries = 5;
UINT32 BdRetryCount = 5;

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

UINT32 BdTransportMaxPacketSize;

//
// DebugPrint filter to OR with the per-component mask.
//

volatile UINT32 BdDebugPrintGlobalMask;

//
//  Indicates if the debugger has been fully initialized.
//
BOOLEAN BdSubsystemInitialized = FALSE;

//
// Indicates whether or not the debugger subsystem
// can be initialized.  This is controlled by the
// BIOS_CONFIG_FLAGS_ENABLE_DEBUGGER flag which can be set
// in the bios_flags in the VMs configuration file.
//
BOOLEAN BdSubsystemEnabled = FALSE;

//
// Used to track unloaded UEFI modules.
//
//
ULONG BdLastUnloadedModule = 0;
EFI_UNLOADED_MODULE BdUnloadedModules[BD_MAX_UNLOADED_MODULES];


