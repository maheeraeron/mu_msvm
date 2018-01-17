/*++

Copyright (c) 2005  Microsoft Corporation

Module Name:

    trap.c

Abstract:

    This module contains code to implement the target side of the boot debugger.

Author:

    Jamie Schwartz (jamschw) 10-Dec-2003 - Mostly a copy of the original
        implementation by DaveC.

Environment:

    Boot

--*/

#include <EfiNt.h>
#include "Bd.h"
#include <Library/ResetSystemLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/WatchdogTimerLib.h>
#include "CrashDump.h"

UINT32 BdTrapRoutine;

//
// Define forward referenced function prototypes.
//

BD_DEBUG_TABLE BdDebugTable =
{
    BdPollBreakIn,
    BdBreakPointWithStatus,
    &BdDebugPrintGlobalMask,
    &BdDebuggerNotPresent
};

NTSTATUS
BdArchInitialize (
    VOID
    )

/*++

Routine Description:

    This function carries out architecture specific boot debugger
    initialization for x86 systems.  This involves allocating the boot
    debugger PCR and hooking the relevant trap handlers into the IDT.

Arguments:

    None.

Return Value:

    STATUS_SUCCESS when initialization is successful.

    An allocation failure status is returned if the boot debugger PCR cannot
        be allocated.

--*/

{
    BdPrcb = &BdPcr.Prcb;

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

    BdInstallTrapVectors();

    //
    // With the trap handlers installed, the boot debugger is prepared to
    // initiate a debugger connection at any time.
    //

    BdArchBlockDebuggerOperation = FALSE;
    return STATUS_SUCCESS;
}

VOID
BdArchDestroy (
    VOID
    )

/*++

Routine Description:

    This function is responsible for freeing any resources allocated during
    BdArchInitialize.  For x86, this just involves freeing the memory
    allocated for the boot debugger PCR.

Arguments:

    None.

Return Value:

    None.

--*/

{

    BdArchBlockDebuggerOperation = TRUE;
    return;
}

LOGICAL
BdDispatchDebugException (
    __in PEXCEPTION_RECORD ExceptionRecord,
    __in PKEXCEPTION_FRAME ExceptionFrame,
    __in PKTRAP_FRAME TrapFrame,
    __in PCONTEXT ContextRecord
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

    A value of TRUE is returned if the exception is handled. Otherwise a
    value of FALSE is returned.

--*/

{
    STRING Input;
    ULONG OldEip;
    STRING Output;
    LOGICAL ReturnValue = FALSE;
    PKD_SYMBOLS_INFO SymbolInfo;
    LOGICAL UnloadSymbols = TRUE;

    UNREFERENCED_PARAMETER(ExceptionFrame);

    //
    // Print, prompt, load symbols, and unload symbols are all special cases
    // of STATUS_BREAKPOINT.
    //

    switch (ExceptionRecord->ExceptionInformation[0])
    {
	//
	// Print:
	//
	// ExceptionInformation[1] is a PSTRING which describes the string
	// to print.
	//

    case BREAKPOINT_PRINT:
	Output.Buffer = (PCHAR)ExceptionRecord->ExceptionInformation[1];
	Output.Length = (USHORT)ExceptionRecord->ExceptionInformation[2];
	if (BdDebuggerNotPresent == FALSE) {
            if (BdPrintString(&Output))
            {
                TrapFrame->Eax = (ULONG)(STATUS_BREAKPOINT);
            }
            else
            {
                TrapFrame->Eax = STATUS_SUCCESS;
            }
	}
        else
        {
            TrapFrame->Eax = (ULONG)STATUS_DEVICE_NOT_CONNECTED;
	}

	TrapFrame->Eip += 1;
	ReturnValue = TRUE;
	break;

        //
        // Prompt: Print a debug prompt string, then input a string.
        //
        //  ExceptionInformation[1] is a PSTRING which describes the prompt
        //      string,
        //
        //  ExceptionInformation[2] is a PSTRING that describes the return
        //      string.
        //

    case BREAKPOINT_PROMPT:
    {
        Output.Buffer = (PCHAR)ExceptionRecord->ExceptionInformation[1];
        Output.Length = (USHORT)ExceptionRecord->ExceptionInformation[2];
        Input.Buffer = (PCHAR)TrapFrame->Ebx;
        Input.MaximumLength = (USHORT)TrapFrame->Edi;

        //
        // Prompt and keep prompting until no breakin seen.
        //

        do {
        } while (BdPromptString(&Output, &Input) != FALSE);

        TrapFrame->Eax = Input.Length;
        TrapFrame->Eip += 1;
        ReturnValue = TRUE;
        break;
    }
    //
    // Unload symbols:
    //
    //  ExceptionInformation[1] is file name of a module.
    //  ExceptionInformaiton[2] is the base of the dll.
    //

    case BREAKPOINT_UNLOAD_SYMBOLS:
        UnloadSymbols = TRUE;

        //
        // Fall through to load symbols case.
        //

    case BREAKPOINT_LOAD_SYMBOLS:
        BdSaveKframe(TrapFrame, ExceptionFrame, ContextRecord);
        OldEip = ContextRecord->Eip;
        SymbolInfo = (PKD_SYMBOLS_INFO)ExceptionRecord->ExceptionInformation[2];
        if (BdDebuggerNotPresent == FALSE)
        {
            BdReportLoadSymbolsStateChange((PSTRING)ExceptionRecord->ExceptionInformation[1],
                                              SymbolInfo,
                                              UnloadSymbols,
                                              ContextRecord);
        }

        //
        // If the kernel debugger did not update EIP, then increment
        // past the breakpoint instruction.
        //

        if (ContextRecord->Eip == OldEip)
        {
            ContextRecord->Eip += 1;
        }

        BdRestoreKframe(TrapFrame, ExceptionFrame, ContextRecord);
        ReturnValue = TRUE;
        break;

        // Return the KD debugger for faster debug access
    case BREAKPOINT_GET_TABLE:
        TrapFrame->Eax = (ULONG)&BdDebugTable;
        TrapFrame->Eip += 1;
        ReturnValue = TRUE;
        break;

        //
        //  Unknown command
        //

    default:
        ReturnValue = FALSE;
        break;
    }
    return ReturnValue;
}

LOGICAL
BdDispatchException (
    __in PEXCEPTION_RECORD ExceptionRecord,
    __in PKEXCEPTION_FRAME ExceptionFrame,
    __in PKTRAP_FRAME TrapFrame
    )

/*++

Routine Description:

    This routine is called whenever an exception is dispatched and the boot
    debugger is active.

Arguments:

    ExceptionRecord - Supplies a pointer to an exception record that
        describes the exception.

    ExceptionFrame - Supplies a pointer to an exception frame (NULL).

    TrapFrame - Supplies a pointer to a trap frame that describes the
        trap.

Return Value:

    A value of TRUE is returned if the exception is handled. Otherwise a
    value of FALSE is returned.

--*/

{
    PCONTEXT ContextRecord;
    LOGICAL Handled = FALSE;
    BOOLEAN FirstChance = TRUE;
    BOOLEAN watchdogState = FALSE;

    //
    // Defer to the stub handler if the debugger is not initialized
    //

    if (!BdSubsystemInitialized)
    {
        return BdDispatchStub(ExceptionRecord, ExceptionFrame, TrapFrame);
    }

    //
    // Set address of context record and set context flags.
    //

    ContextRecord = &BdPrcb->ProcessorState.ContextFrame;
    ContextRecord->ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

    //
    // Suspend the watchdog while handling debug events
    // Even simple debug events, like symbol loading,
    // can wait in the debugger if there wasa a pending break-in
    //
    watchdogState = WatchdogSuspend();

    //
    // Dispatch based on exception code
    // This will determine what action is needed for exception,
    // if it is continuable and first vs. second chance
    //

    switch (ExceptionRecord->ExceptionCode)
    {
    case STATUS_BREAKPOINT:
        Handled = BdDispatchDebugException(ExceptionRecord, ExceptionFrame, TrapFrame, ContextRecord);
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
        BdSaveKframe(TrapFrame, ExceptionFrame, ContextRecord);
        Handled = BdReportExceptionStateChange(ExceptionRecord, ContextRecord, FirstChance);
        BdRestoreKframe(TrapFrame, ExceptionFrame, ContextRecord);
        BdControlCPressed = FALSE;
    }

    WatchdogResume(watchdogState);
    return Handled;
}

LOGICAL
BdDispatchStub (
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
            TrapFrame->Eax = 0LL;
            __fallthrough;

        case BREAKPOINT_BREAK:
        case BREAKPOINT_PRINT:
        case BREAKPOINT_PROMPT:
        case BREAKPOINT_LOAD_SYMBOLS:
        case BREAKPOINT_UNLOAD_SYMBOLS:
        case BREAKPOINT_COMMAND_STRING:
            TrapFrame->Eip += 1;
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

VOID
BdArchStart (
    VOID
    )

/*++

Routine Description:

    This function performs any architecture specific actions required before a
    debugger connection attempt can take place.  There are no such actions on
    x86 systems since all trap handlers can be safely installed persistently.

Arguments:

    None.

Return Value:

    None.

--*/

{
#if defined(MDE_CPU_IA32)
    DEBUG((DEBUG_INFO, "DDD BdArchStart\n"));
    BdTrapRoutine = (UINT32)BdDispatchException;

#endif
    return;
}

VOID
BdArchStop (
    VOID
    )

/*++

Routine Description:

    This function performs any architecture specific actions required before
    an active debugger connection can be disconnected.  There are no such
    actions on x86 systems since all trap handlers can be safely installed
    persistently.

Arguments:

    None.

Return Value:

    None.

--*/

{

    return;
}

