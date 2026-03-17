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

#include "Bd.h"
#include <Hv/HvGuestCpuid.h>
#include <Library/ResetSystemLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/WatchdogTimerLib.h>
#include <IsolationTypes.h>
#include "CrashDump.h"

BD_DEBUG_TABLE BdDebugTable =
{
    BdPollBreakIn,
    BdBreakPointWithStatus,
    &BdDebugPrintGlobalMask,
    &BdDebuggerNotPresent
};

BOOLEAN mWatchdogActive;

NTSTATUS
BdArchInitialize (
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
    BOOLEAN hardwareIsolated = FALSE;

    BdSerialPrint(">>> %a\n", __func__);
    //
    // Allocate the boot debugger PCR.
    //

    BdPrcb = &BdPcr.Prcb;

    //
    // Determine whether the watchdog is active.  It is always active except
    // on platforms that enable hardware isolation with no paravisor.
    //

#if defined(MDE_CPU_X64)

    //
    // Since boot services have not yet been initialized, PCD values are not
    // accessible, and the determination of isolation status must be done
    // through CPUID.
    //

    {
        HV_CPUID_RESULT cpuidResult;

        AsmCpuid(HvCpuIdFunctionVersionAndFeatures,
                 &cpuidResult.Eax,
                 &cpuidResult.Ebx,
                 &cpuidResult.Ecx,
                 &cpuidResult.Edx);

        if (cpuidResult.VersionAndFeatures.HypervisorPresent)
        {
            AsmCpuid(HvCpuIdFunctionHvInterface,
                     &cpuidResult.Eax,
                     &cpuidResult.Ebx,
                     &cpuidResult.Ecx,
                     &cpuidResult.Edx);

            if (cpuidResult.HvInterface.Interface == HvMicrosoftHypervisorInterface)
            {
                AsmCpuid(HvCpuIdFunctionMsHvFeatures,
                         &cpuidResult.Eax,
                         &cpuidResult.Ebx,
                         &cpuidResult.Ecx,
                         &cpuidResult.Edx);

                if (cpuidResult.MsHvFeatures.PartitionPrivileges.Isolation)
                {
                    AsmCpuid(HvCpuidFunctionMsHvIsolationConfiguration,
                             &cpuidResult.Eax,
                             &cpuidResult.Ebx,
                             &cpuidResult.Ecx,
                             &cpuidResult.Edx);

                    if ((cpuidResult.MsHvIsolationConfiguration.IsolationType >= HV_PARTITION_ISOLATION_TYPE_SNP) &&
                        !cpuidResult.MsHvIsolationConfiguration.ParavisorPresent)
                    {
                        hardwareIsolated = TRUE;
                    }
                }
            }
        }
    }

#endif

    if (!hardwareIsolated)
    {
        mWatchdogActive = TRUE;
    }

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
    BdSerialPrint("<<< %a\n", __func__);
    return STATUS_SUCCESS;
}


VOID
BdArchDestroy (
    VOID
    )
/*++

Routine Description:

    This function is responsible for freeing any resources allocated during
    BdArchInitialize.  For amd64, this just involves freeing the memory
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


VOID
BdArchStart (
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

    if (BdDebuggerInitialized() != FALSE)
    {
        BdInstallTrapVectors();
    }

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
BdDispatchDebugException(
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
        if (BdDebuggerNotPresent == FALSE)
        {
            BoolRet = BdPrintString(&Output);

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
                BoolRet = BdPromptString(&Output, &Input);
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
        BdSaveKframe(TrapFrame, ExceptionFrame, ContextRecord);
        OldRip = ContextRecord->Rip;
        if (BdDebuggerNotPresent == FALSE)
        {
            BdReportLoadSymbolsStateChange((PSTRING)TrapFrame->Rcx,
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

        BdRestoreKframe(TrapFrame, ExceptionFrame, ContextRecord);
        Handled = TRUE;
        break;

        //
        // Return the KD debugger table for faster debugger access.
        //

    case BREAKPOINT_GET_TABLE:
        TrapFrame->Rax = (UINT64)&BdDebugTable;
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
BdDispatchException (
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
    // can wait in the debugger if there was a pending break-in
    //
    if (mWatchdogActive)
    {
        watchdogState = WatchdogSuspend();
    }

    //
    // Dispatch based on exception code
    // This will determine what action is needed for exception,
    // if it is continuable and first vs. second chance.
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

    if (mWatchdogActive)
    {
        WatchdogResume(watchdogState);
    }

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
            // NULL indicates no debugger to DebugLib.
            //
            TrapFrame->Rax = 0; // fallthrough
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
