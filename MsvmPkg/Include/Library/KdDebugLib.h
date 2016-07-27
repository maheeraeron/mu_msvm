/*++

Copyright (c) Microsoft Corporation

Module Name:

    KdDebugLib.h

Abstract:

    Definitions for functionality provided by the KD UEFI debug library.

Author:

    Shreyas Srivatsan (shreyas) - 2-Aug-2012

--*/

#pragma once

#include <EfiNt.h>

typedef struct _EFI_KD_DEBUG_TABLE
{
    UINT32
    (*PollBreakIn)(
        VOID
        );

    VOID
    (*BreakPointWithStatus)(
        __in UINT32 Status
        );

    volatile UINT32 *DebuggerMask;
    volatile UINT32 *DebuggerNotPresent;

} EFI_KD_DEBUG_TABLE;

VOID *
DebugService2(
    __in_opt VOID *Param1,
    __in_opt VOID *Param2,
    __in UINT32 Service
    );

#define BREAKPOINT_BREAK            0
#define BREAKPOINT_PRINT            1
#define BREAKPOINT_PROMPT           2
#define BREAKPOINT_LOAD_SYMBOLS     3
#define BREAKPOINT_UNLOAD_SYMBOLS   4
#define BREAKPOINT_COMMAND_STRING   5
#define BREAKPOINT_GET_TABLE        6

VOID
EFIAPI
DebugBreakPointWithStatus(
    __in UINT32 Status
    );

#define DBG_STATUS_CONTROL_C 1

VOID
EFIAPI
DebugPrintString(
    __in_ecount(Length) CHAR8 *String,
    __in UINTN Length
    );

VOID
EFIAPI
DebugPollDebugger(
    VOID
    );

