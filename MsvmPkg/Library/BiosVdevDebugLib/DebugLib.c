/*++

Copyright (c) 2017  Microsoft Corporation

Module Name:

    DebugLib.c

Abstract:

    This module implements the UEFI debug library interface.
    It sends the strings to the Hyper-V BiosDevice via an intercept.

--*/

#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugPrintErrorLevelLib.h>
#include <EfiNt.h>
#include <BiosInterface.h>


// -------------------------------------------------------------------- Defines

#define DEBUG_PRINT_MAX_SIZE 1024

// ------------------------------------------------------------------ Globals

static CHAR8 gBuffer[DEBUG_PRINT_MAX_SIZE];

// ------------------------------------------------------------------ Functions


VOID
EFIAPI
DebugPrintString(
    __in_ecount(Length) CHAR8 *String,
    __in UINTN Length
    );

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
    VA_LIST marker;

    ASSERT(String != NULL);

    VA_START(marker, String);
    AsciiVSPrint(gBuffer, sizeof(gBuffer), String, marker);
    VA_END (marker);

    DebugPrintString(gBuffer, sizeof(gBuffer));
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

    Normally this breaks into the debugger with assertion status.
    In this particular library implementation it just outputs an "assert" message.

Arguments:

    FileName - The pointer to the name of the source file that generated the assert condition.

    LineNumber - The line number in the source file that generated the assert condition

    Description - The pointer to the description of the assert condition.

Return Value:

    None.

--*/
{
    UINTN length = AsciiSPrint(gBuffer,
                               DEBUG_PRINT_MAX_SIZE,
                               "**ASSERT** FILE: %s LINE: %ull DESC: %s\n",
                               FileName,
                               LineNumber,
                               Description);

    DebugPrintString(gBuffer, length+1); // include null

    return;
}


VOID *
EFIAPI
DebugClearMemory (
    __out VOID  *Buffer,
    __in UINTN  Length
    )
/*++

Routine Description:

    Normally this function in an implementation of this library fills a target buffer
    with PcdDebugClearMemoryValue, and returns the target buffer.

    This function does *nothing* and returns the Buffer.

    It should not be called since DebugClearMemoryEnabled below returns FALSE.

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
    return TRUE;
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
    return TRUE;
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

    Issues a debug print command to the debugger. In this particular instance
    it sends the formatted string over to the worker process and it will
    output the string to an attached debugger.

Arguments:

    String - A pointer to the string to write.

    Length - The maximum length of the string, in bytes.

Return Value:

    None.

--*/
{
    static UINTN * const pBiosAddressRegister = (UINTN*)BiosAddressRegister;
    static UINTN * const pBiosDataRegister = (UINTN*)BiosDataRegister;

    //
    // Copy string into our intercept buffer and ensure it is null terminated.
    //
    CopyMem(gBuffer, String, MIN(1024, Length));
    gBuffer[DEBUG_PRINT_MAX_SIZE-1] = 0;

    //
    // Intercept the Bios VDev with the correct codepoint and buffer GPA.
    //
    *pBiosAddressRegister = (UINT32)BiosDebugOutputString;
    *pBiosDataRegister = (UINT32)(UINTN)gBuffer;
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

    Nothing implemented in this instance as it is an output-only DebugLib.

Arguments:

    None.

Return Value:

    None.

--*/
{
}
