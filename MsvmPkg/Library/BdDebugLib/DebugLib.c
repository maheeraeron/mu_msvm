/*++

Copyright (c) 2012  Microsoft Corporation

Module Name:

    DebugLib.c

Abstract:

    This module implements the UEFI DebugLib library for the UEFI Microsoft Boot Debugger.

Author:

    Shreyas Srivatsan (shreyas) 01-Aug-2012

--*/

// ------------------------------------------------------------------- Includes

#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CrashLib.h>
#include <Library/DebugPrintErrorLevelLib.h>
#include <Library/BdDebugLib.h>

#include <EfiNt.h>

// -------------------------------------------------------------------- Defines

#define DEBUG_PRINT_MAX_SIZE 1024

// ------------------------------------------------------------------ Globals

UINT32 BdDebugPrintComponentMask = 0;
BD_DEBUG_TABLE *gBdDebugTable = NULL;

// ------------------------------------------------------------------ Functions

RETURN_STATUS
EFIAPI
BdDebugLibConstructor (
    VOID
    )
{
    //
    // BdDebugLib depends on the debug trap handlers being installed. This operation is
    // complete by the time the constructor is called.
    //
    // Note that a NULL return value indicates that the debugger is not
    // available.
    //
    gBdDebugTable = DebugService2(0, 0, BREAKPOINT_GET_TABLE);
    return 0;
}


VOID
EFIAPI
DebugPrint(
    __in UINTN ErrorLevel,
    __in CONST CHAR8 *String,
    ...
    )
/*++

Routine Description:

    This function Prints a debug message to the debug output device if the specified error
    level is enabled.

    If any bit in ErrorLevel is also set in DebugPrintErrorLevelLib function
    GetDebugPrintErrorLevel (), then print the message specified by Format and the  associated
    variable argument list to the debug output device.

    If Format is NULL, then ASSERT().

Arguments:

    ErrorLevel - Supplies the error level of the debug message.

    Format - Format string for the debug message to print.

    ... - Variable argument list whose contents are accessed based on the format string
           specified by Format.

Return value:

    None.

--*/
{
    CHAR8 buffer[DEBUG_PRINT_MAX_SIZE];
    VA_LIST marker;

    //
    // If Format is NULL, then ASSERT().
    //

    ASSERT(String != NULL);

    //
    // If there is no debugger or this trace is masked, then return
    // immediately.
    //

    if (gBdDebugTable == NULL ||
        *gBdDebugTable->DebuggerNotPresent ||
        ((BdDebugPrintComponentMask | *gBdDebugTable->DebuggerMask) & ErrorLevel) == 0)
    {
        return;
    }

    //
    //
    // Convert the DEBUG() message to an ASCII String.
    //

    VA_START(marker, String);
    AsciiVSPrint(buffer, sizeof(buffer), String, marker);
    VA_END (marker);

    DebugPrintString(buffer, sizeof(buffer));
}


VOID
EFIAPI
DebugAssert (
  __in CONST CHAR8 *FileName,
  __in UINTN LineNumber,
  __in CONST CHAR8 *Description
  )
/*++

Routine Description:

    Breaks into the debugger with assertion status.

Arguments:

    FileName - The pointer to the name of the source file that generated the assert condition.

    LineNumber - The line number in the source file that generated the assert condition

    Description - The pointer to the description of the assert condition.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(FileName);
    UNREFERENCED_PARAMETER(LineNumber);
    UNREFERENCED_PARAMETER(Description);

    if (gBdDebugTable != NULL &&
        *gBdDebugTable->DebuggerNotPresent == FALSE)
    {
#if defined(_AMD64_)
        __int2c();
#elif defined(_ARM64_)
        __break(0xf001);
#endif
    }
}


VOID *
EFIAPI
DebugClearMemory (
    __out VOID  *Buffer,
    __in UINTN  Length
    )
/*++

Routine Description:

    Fills a target buffer with PcdDebugClearMemoryValue, and returns the target buffer.

    This function fills Length bytes of Buffer with the value specified by
    PcdDebugClearMemoryValue, and returns Buffer.

    If Buffer is NULL, then ASSERT().
    If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().

Arguments:

    Buffer - Supplies The pointer to the target buffer to be filled with
        PcdDebugClearMemoryValue.

    Length - Supplies the number of bytes in Buffer to fill with
        PcdDebugClearMemoryValue.

Return Value:

    Cleared buffer.

--*/
{
    UNREFERENCED_PARAMETER(Length);

    return Buffer;
}


BOOLEAN
EFIAPI
DebugAssertEnabled (
    VOID
    )
/*++

Routine Description:

    Returns true if assert macros are enabled.

Arguments:

    None.

Return Value:

    TRUE if assert macros are enabled, else false.

--*/
{
    if (gBdDebugTable == NULL ||
        *gBdDebugTable->DebuggerNotPresent)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


BOOLEAN
EFIAPI
DebugPrintEnabled (
    VOID
    )
/*++

Routine Description:

    Returns true if DEBUG() macros are enabled.

Arguments:

    None.

Return Value:

    TRUE if debug prints are enabled, else false.

--*/
{
    if (gBdDebugTable == NULL ||
        *gBdDebugTable->DebuggerNotPresent)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


BOOLEAN
EFIAPI
DebugCodeEnabled (
    VOID
    )
/*++

Routine Description:

    Returns true if DEBUG_CODE() macros are enabled.

Arguments:

    None.

Return Value:

    TRUE if DEBUG_CODE() macros are enabled, else false.

--*/
{
    return TRUE;
}


BOOLEAN
EFIAPI
DebugClearMemoryEnabled (
    VOID
    )
/*++

Routine Description:

    Returns true if DEBUG_CLEAR_MEMORY() macros are enabled.

Arguments:

    None.

Return Value:

    TRUE if DEBUG_CLEAR_MEMORY() macros are enabled, else false.

--*/
{
    return FALSE;
}


BOOLEAN
EFIAPI
DebugPrintLevelEnabled(
  IN  CONST UINTN        ErrorLevel
  )
/*++

Routine Description:

    Returns TRUE if any one of the bit is set both in ErrorLevel and PcdFixedDebugPrintErrorLevel.

    This function compares the bit mask of ErrorLevel and PcdFixedDebugPrintErrorLevel.

Arguments:

    ErrorLevel - the bit mask to test against the Pcd value.

Return Value:

    TRUE    Current ErrorLevel is supported.
    FALSE   Current ErrorLevel is not supported.

--*/
{
  return (BOOLEAN) ((ErrorLevel & PcdGet32(PcdDebugPrintErrorLevel)) != 0);
}


VOID
EFIAPI
DebugPrintString(
    __in_ecount(Length) CHAR8 *String,
    __in UINTN Length
    )
/*++

Routine Description:

    Issues a debug print command to the debugger.

Arguments:

    String - A pointer to the string to write.

    Length - The maximum length of the string, in bytes.

Return Value:

    None.

--*/
{
    NTSTATUS status;

    if (gBdDebugTable != NULL &&
        *gBdDebugTable->DebuggerNotPresent == FALSE)
    {
        status = (NTSTATUS)(UINTN)DebugService2(String,
                                                (PVOID)Length,
                                                BREAKPOINT_PRINT);

        if (status == STATUS_BREAKPOINT)
        {
            DebugBreakPointWithStatus(DBG_STATUS_CONTROL_C);
        }
    }
}


VOID
EFIAPI
DebugPollDebugger(
    VOID
    )
/*++

Routine Description:

    If a debugger connection is established, polls the transport device for
    user input (in the form of ctrl-c).  When present, this routine will
    initiate communication with the debugger host and force the breakin.

Arguments:

    None.

Return Value:

    None.

--*/
{
    if ((gBdDebugTable != NULL) &&
        (gBdDebugTable->PollBreakIn() != FALSE))
    {
        DebugBreakPointWithStatus(DBG_STATUS_CONTROL_C);
    }
}


VOID
EFIAPI
DebugBreakPointWithStatus(
    __in UINT32 Status
    )
/*++

Routine Description:

    Breaks into the debugger with the provided status code.

Arguments:

    Status - A status code.

Return Value:

    None.

--*/
{
    if (gBdDebugTable != NULL)
    {
        gBdDebugTable->BreakPointWithStatus(Status);
    }
}

