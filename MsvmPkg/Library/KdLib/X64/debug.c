/*++

Copyright (c) 2005  Microsoft Corporation

Module Name:

    kdtrap.c

Abstract:

    This module contains code to implement the target side of the portable
    kernel debugger.

Author:

    Jamie Schwartz (jamschw) 15-Jun-2004 - Most of the file's contents are a
        copy from the kernel's code in ntos\kd64\kdtrap.c.

Environment:

    Boot

--*/

#include <EfiNt.h>
#include "EfiKd.h"
#include <Library/ResetSystemLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/WatchdogTimerLib.h>
#include "CrashDump.h"

EFI_KD_DEBUG_TABLE EfiKdDebugTable =
{
    EfiKdPollBreakIn,
    EfiKdBreakPointWithStatus,
    &EfiKdDebugPrintGlobalMask,
    &EfiKdDebuggerNotPresent
};

NTSTATUS
EfiKdArchInitialize (
    VOID
    )
/*++

Routine Description:

    This function carries out architecture specific boot debugger
    initialization for amd64 systems.  This involves allocating the boot
    debugger PCR and hooking the relevant trap handlers into the IDT.

Arguments:

    None.

Return Value:

    STATUS_SUCCESS when initialization is successful.

    An allocation failure status is returned if the boot debugger PCR cannot
        be allocated.

--*/
{
    //
    // Allocate the boot debugger PCR.
    //

    EfiKdPrcb = &EfiKdPcr.Prcb;

    //
    // Install the boot debugger trap handlers.  Each boot application has a
    // private IDT, implying that only this application is affected when the
    // handlers are installed.  This allows trap handlers safely reside in the
    // IDT for the life of this boot application (even across debugger
    // connection start and stop operations) after being installed once during
    // library initialization.  Restricted to this private IDT, these handlers
    // can have no impact on other applications that may be started without
    // debugging enabled.
    //

    EfiKdInstallTrapVectors();

    //
    // With the trap handlers installed, the boot debugger is prepared to
    // initiate a debugger connection at any time.
    //

    EfiKdArchBlockDebuggerOperation = FALSE;
    return STATUS_SUCCESS;
}


VOID
EfiKdArchDestroy (
    VOID
    )
/*++

Routine Description:

    This function is responsible for freeing any resources allocated during
    EfiKdArchInitialize.  For amd64, this just involves freeing the memory
    allocated for the boot debugger PCR.

Arguments:

    None.

Return Value:

    None.

--*/
{
    EfiKdArchBlockDebuggerOperation = TRUE;
    return;
}


VOID
EfiKdArchStart (
    VOID
    )
/*++

Routine Description:

    This routine reinstalls the current boot application's trap handlers
    before the debugger starts. This is required as the trap handlers may have
    been overwritten (by some other application) after the debugger was stopped.

    N.B. These actions are not required on x64 systems once per-application
         GDT/IDT is implemented across all firmwares. With separate IDTs,
         all trap handlers can be safely installed persistently.

Arguments:

    None.

Return Value:

    None.

--*/
{
    //
    // If the EFI debugger is enabled, then EFI debugger's trap handlers
    // should be installed. Otherwise, default library trap handlers should be
    // installed.
    //

    if (EfiKdDebuggerInitialized() != FALSE)
    {
        EfiKdInstallTrapVectors();
    }

    return;
}

VOID
EfiKdArchStop (
    VOID
    )
/*++

Routine Description:

    This function performs any architecture specific actions required before
    an active debugger connection can be disconnected.  There are no such
    actions on x64 systems since all trap handlers can be safely installed
    persistently.

Arguments:

    None.

Return Value:

    None.

--*/
{
    return;
}

LOGICAL
EfiKdDispatchDebugException(
    _In_ PEXCEPTION_RECORD ExceptionRecord,
    _In_ PKEXCEPTION_FRAME ExceptionFrame,
    _In_ PKTRAP_FRAME TrapFrame,
    _In_ PCONTEXT ContextRecord
    )
{
    STRING  Input;
    UINT64  OldRip;
    STRING  Output;
    LOGICAL BoolRet;
    LOGICAL Handled = FALSE;
    BOOLEAN UnloadSymbols;

    //
    // Print, Prompt, Load symbols, Unload symbols, are all special
    // cases of STATUS_BREAKPOINT.
    //

    UnloadSymbols = FALSE;

    switch (ExceptionRecord->ExceptionInformation[0])
    {
        //
        // Print a debug string.
        //
        // Arguments:
        //
        //     rcx - Supplies a pointer to an output string buffer.
        //     dx - Supplies the length of the output string buffer.
        //     r8d - Supplies the calling component's filter.
        //     r9d - Supplies the output filter level.
        //

    case BREAKPOINT_PRINT:
        Output.Buffer = (PCHAR)TrapFrame->Rcx;
        Output.Length = (USHORT)TrapFrame->Rdx;
        if (EfiKdDebuggerNotPresent == FALSE)
        {
            BoolRet = EfiKdPrintString(&Output);

            if (BoolRet == FALSE)
            {
                TrapFrame->Rax = (UINT64) STATUS_SUCCESS;
            }
            else
            {
                TrapFrame->Rax = (UINT64) STATUS_BREAKPOINT;
            }
        }
        else
        {
            TrapFrame->Rax = (UINT64) STATUS_DEVICE_NOT_CONNECTED;
        }

        TrapFrame->Rip += 1;
        Handled = TRUE;
        break;

        //
        // Print a debug prompt string, then input a string.
        //
        // Arguments:
        //
        //     rcx - Supplies a pointer to an output string buffer.
        //     dx - Supplies the length of the output string buffer.
        //     r8 - Supplies a pointer to an input string buffer.
        //     r9w - Supplies the length of the input string bufffer.
        //

    case BREAKPOINT_PROMPT:
        {
            Output.Buffer = (PCHAR)TrapFrame->Rcx;
            Output.Length = (USHORT)TrapFrame->Rdx;
            Input.Buffer = (PCHAR)TrapFrame->R8;
            Input.Length = (USHORT)TrapFrame->R9;

            //
            // Continue to prompt until no breakin is seen.
            //
            do
            {
                BoolRet = EfiKdPromptString(&Output, &Input);
            } while (BoolRet != FALSE);

            TrapFrame->Rax = Input.Length;
            TrapFrame->Rip += 1;
            Handled = TRUE;
            break;
        }
        //
        // Load the symbolic information for an image.
        //
        // Arguments:
        //
        //    rcx - Supplies a pointer to a filename string descriptor.
        //    rdx - Supplies the base address of the image.
        //

    case BREAKPOINT_UNLOAD_SYMBOLS:
        UnloadSymbols = TRUE;

        //
        // Fall through
        //

    case BREAKPOINT_LOAD_SYMBOLS:
        EfiKdSaveKframe(TrapFrame, ExceptionFrame, ContextRecord);
        OldRip = ContextRecord->Rip;
        if (EfiKdDebuggerNotPresent == FALSE)
        {
            EfiKdReportLoadSymbolsStateChange((PSTRING)TrapFrame->Rcx,
                                              (PKD_SYMBOLS_INFO)TrapFrame->Rdx,
                                              UnloadSymbols,
                                              ContextRecord);
        }

        //
        // If the kernel debugger did not update RIP, then increment
        // past the breakpoint instruction.
        //

        if (ContextRecord->Rip == OldRip)
        {
            ContextRecord->Rip += 1;
        }

        EfiKdRestoreKframe(TrapFrame, ExceptionFrame, ContextRecord);
        Handled = TRUE;
        break;

        //
        // Return the KD debugger table for faster debugger access.
        //

    case BREAKPOINT_GET_TABLE:
        TrapFrame->Rax = (UINT64)&EfiKdDebugTable;
        TrapFrame->Rip += 1;
        Handled = TRUE;
        break;

        //
        // Unknown command.
        //

    default:
        Handled = FALSE;
        break;
    }

    return Handled;
}


LOGICAL
EfiKdDispatchException (
    _In_    PEXCEPTION_RECORD   ExceptionRecord,
    _In_    PKEXCEPTION_FRAME   ExceptionFrame,
    _In_    PKTRAP_FRAME        TrapFrame
    )
/*++

Routine Description:

    This routine is called whenever a exception is dispatched and the boot
    debugger is active.

Arguments:

    ExceptionRecord - Supplies a pointer to an exception record that
        describes the exception.

    ExceptionFrame - Supplies a pointer to an exception frame (NULL).

    TrapFrame - Supplies a pointer to a trap frame that describes the
        trap.

Return Value:

    A value of TRUE is returned if the exception is Handled. Otherwise a
    value of FALSE is returned.

--*/
{
    PCONTEXT ContextRecord;
    LOGICAL Handled = FALSE;
    BOOLEAN FirstChance = TRUE;
    BOOLEAN watchdogState = FALSE;

    //
    // Defer to the stub handler if the debugger is not initialized.
    //
    if (!EfiKdSubsystemInitialized)
    {
        return EfiKdDispatchStub(ExceptionRecord, ExceptionFrame, TrapFrame);
    }

    //
    // Set address of context record and set context flags.
    //

    ContextRecord = &EfiKdPrcb->ProcessorState.ContextFrame;
    ContextRecord->ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

    //
    // Suspend the watchdog while handling debug events
    // Even simple debug events, like symbol loading,
    // can wait in the debugger if there was a pending break-in
    //
    watchdogState = WatchdogSuspend();

    //
    // Dispatch based on exception code
    // This will determine what action is needed for exception,
    // if it is continuable and first vs. second chance.
    //
    switch (ExceptionRecord->ExceptionCode)
    {
    case STATUS_BREAKPOINT:
        Handled = EfiKdDispatchDebugException(ExceptionRecord, ExceptionFrame, TrapFrame, ContextRecord);
        break;

    case STATUS_DATATYPE_MISALIGNMENT:
    case STATUS_ACCESS_VIOLATION:
    case STATUS_ILLEGAL_INSTRUCTION:
    case STATUS_NONCONTINUABLE_EXCEPTION:
    case STATUS_ARRAY_BOUNDS_EXCEEDED:
    case STATUS_FLOAT_INVALID_OPERATION:
    case STATUS_INTEGER_DIVIDE_BY_ZERO:
    case STATUS_INTEGER_OVERFLOW:
    case STATUS_STACK_BUFFER_OVERRUN:
        ExceptionRecord->ExceptionFlags = KD_EXCEPTION_NONCONTINUABLE;
        FirstChance = FALSE;
        break;

    default:
        // Some possible values that end up here
        // STATUS_ASSERTION_FAILURE
        // STATUS_SINGLE_STEP
        break;
    }

    //
    // Report unhandled exceptions to the debugger
    // and let it ultimately determine if the exception is
    // handled or not.
    //
    if (!Handled)
    {
        EfiKdSaveKframe(TrapFrame, ExceptionFrame, ContextRecord);
        
        Handled = EfiKdReportExceptionStateChange(ExceptionRecord, ContextRecord, FirstChance);
        EfiKdRestoreKframe(TrapFrame, ExceptionFrame, ContextRecord);
        EfiKdControlCPressed = FALSE;
    }

    WatchdogResume(watchdogState);
    return Handled;
}



LOGICAL
EfiKdDispatchStub (
    _In_    PEXCEPTION_RECORD   ExceptionRecord,
    _In_    PKEXCEPTION_FRAME   ExceptionFrame,
    _In_    PKTRAP_FRAME        TrapFrame
    )
/*++

Routine Description:

    This routine provides a kernel debugger stub routine to catch debug
    prints when the boot debugger is not active. It also serves to notify
    other UEFI drivers that debug services are not available.

Arguments:

    ExceptionRecord - Supplies a pointer to an exception record that
        describes the exception.

    ExceptionFrame - Supplies a pointer to an exception frame (NULL).

    TrapFrame - Supplies a pointer to a trap frame that describes the
        trap.

Return Value:

    A value of TRUE is returned if the exception is Handled. Otherwise a
    value of FALSE is returned.

--*/
{
    UINT_PTR BreakpointCode;
    LOGICAL Handled = FALSE;

    BreakpointCode = ExceptionRecord->ExceptionInformation[0];

    UNREFERENCED_PARAMETER(ExceptionFrame);

    //
    // If the breakpoint is a debug print or load/unload symbols, 
    // ignore it.
    // N.B.
    //   Assertions and Fail Fast exceptions are not Handled and
    //   will therefore bugcheck if a debugger is not attached.
    //

    if ((ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) &&
        (ExceptionRecord->NumberParameters > 0))
    {
        switch (BreakpointCode)
        {
        case BREAKPOINT_GET_TABLE:
            //
            // Make sure to return NULL to indicate to
            // the DebugLib that the debugger is not installed.
            //
            TrapFrame->Rax = 0LL;
            __fallthrough;

        case BREAKPOINT_BREAK:
        case BREAKPOINT_PRINT:
        case BREAKPOINT_PROMPT:
        case BREAKPOINT_LOAD_SYMBOLS:
        case BREAKPOINT_UNLOAD_SYMBOLS:
        case BREAKPOINT_COMMAND_STRING:
            TrapFrame->Rip += 1;
            Handled = TRUE;
        }

    }
    else if (ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP)
    {
        // Don't bugcheck on single step
        Handled = TRUE;
    }

    return Handled;
}
