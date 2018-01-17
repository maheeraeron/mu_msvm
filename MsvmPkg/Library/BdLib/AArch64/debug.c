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

#include "bd.h"

#include <Library/Debuglib.h>

#define NT_ASSERT ASSERT

BD_DEBUG_TABLE BdDebugTable =
{
    BdPollBreakIn,
    BdBreakPointWithStatus,
    &BdDebugPrintGlobalMask,
    &BdDebuggerNotPresent
};

//
// Copied from ki.h
//

#define KI_EXCEPTION_INTERNAL               0x10000000
#define KI_EXCEPTION_INVALID_OP             (KI_EXCEPTION_INTERNAL | 0x2)

//
// References to exception vector table
//

extern void (*BdExceptionVectors[])(VOID);

extern BOOL ArchpForceX18Recovery;

// ------------------------------------------------------------------ Functions

VOID
BdRestoreKframe (
    __inout PKTRAP_FRAME TrapFrame,
    __inout PKEXCEPTION_FRAME ExceptionFrame,
    __in PCONTEXT ContextRecord
    );

VOID
BdSaveKframe (
    __in PKTRAP_FRAME TrapFrame,
    __in PKEXCEPTION_FRAME ExceptionFrame,
    __inout PCONTEXT ContextRecord
    );


__inline
UINT32
GetCurrentEl(
    VOID
    )

/*++

Routine Description:

    This function returns the current Exception Level

Arguments:

    None.

Return Value:

    The current Exception Level (0-3).

--*/

{
    switch ((_ReadStatusReg(ARM64_CurrentEL) & CPSREL_MASK)) {
    case CPSREL_0: return 0;
    case CPSREL_1: return 1;
    case CPSREL_2: return 2;
    case CPSREL_3: return 3;
    }

    return 0;
}


static
VOID
BdArm64CleanInitDebug(
    VOID
    )
/*++

Routine Description:

    This function initializes or reinitializes the debugger for use
    This can happen initially on init or after stop and start again.

Arguments:

    None.

Return Value:

    None.

--*/
{
    ULONG64 Value;
    ULONG Count;

    //
    // Trap handlers assume there is a PCR structure hanging off of x18.
    //

    __setReg(18, (ULONG_PTR)&BdPcr);
    if (GetCurrentEl() == 1) {
        _WriteStatusReg(ARM64_TPIDR_EL1, (ULONG_PTR)&BdPcr);

    } else {

        NT_ASSERT(GetCurrentEl() == 2);

        _WriteStatusReg(ARM64_TPIDR_EL2, (ULONG_PTR)&BdPcr);
    }

    //
    // Workaround for broken 8996 firmware that is unable to compliant with
    // AArch64 ABI to reserve x18.
    //
#if 0
    {

        PDEBUG_PORT_TABLE_V2 DebugPortTable;
        NTSTATUS Status;

        //
        // Force x18 recovery during context switch on QCOM 8996
        //

        Status = BlUtlGetAcpiTable(&DebugPortTable, DBG2_SIGNATURE);
        if (NT_SUCCESS(Status) &&
            (*(PULONG)(&DebugPortTable->Header.OEMID[0]) == 0x4d4f4351) &&
            (DebugPortTable->Header.OEMRevision == 0x8996)) {

            ArchpForceX18Recovery = TRUE;
        }
    }
#endif

    //
    // Install the boot debugger trap handlers.
    //

    BdInstallTrapVectors();

    //
    // Ensure debug exceptions are disabled during setup
    //

    Value = _ReadStatusReg(ARM64_DAIF);
    Value |= DAIF_DEBUG;
    _WriteStatusReg(ARM64_DAIF, Value);
    _InstructionSynchronizationBarrier();

    //
    // Unlock debug registers
    //

    Value = _ReadStatusReg(ARM64_OSLSR_EL1);
    if ((Value & ARM64_OSLSR_LOCKED) != 0) {
        _WriteStatusReg(ARM64_OSLAR_EL1, 0);
        _DataSynchronizationBarrier();
        Value = _ReadStatusReg(ARM64_OSLSR_EL1);

        NT_ASSERT((Value & ARM64_OSLSR_LOCKED) == 0);
    }

    //
    // Reset all breakpoint and watchpoint control registers to 0
    //

    Count = READ_ARM64_FEATURE(ARM64_ID_AA64DFR0_EL1, DFR0_BREAKPOINT_COUNT);
    switch (Count) {
    case 15:    _WriteStatusReg(ARM64_DBGBCR15_EL1, 0);
    case 14:    _WriteStatusReg(ARM64_DBGBCR14_EL1, 0);
    case 13:    _WriteStatusReg(ARM64_DBGBCR13_EL1, 0);
    case 12:    _WriteStatusReg(ARM64_DBGBCR12_EL1, 0);
    case 11:    _WriteStatusReg(ARM64_DBGBCR11_EL1, 0);
    case 10:    _WriteStatusReg(ARM64_DBGBCR10_EL1, 0);
    case 9:     _WriteStatusReg(ARM64_DBGBCR9_EL1, 0);
    case 8:     _WriteStatusReg(ARM64_DBGBCR8_EL1, 0);
    case 7:     _WriteStatusReg(ARM64_DBGBCR7_EL1, 0);
    case 6:     _WriteStatusReg(ARM64_DBGBCR6_EL1, 0);
    case 5:     _WriteStatusReg(ARM64_DBGBCR5_EL1, 0);
    case 4:     _WriteStatusReg(ARM64_DBGBCR4_EL1, 0);
    case 3:     _WriteStatusReg(ARM64_DBGBCR3_EL1, 0);
    case 2:     _WriteStatusReg(ARM64_DBGBCR2_EL1, 0);
    case 1:     _WriteStatusReg(ARM64_DBGBCR1_EL1, 0);
    case 0:     _WriteStatusReg(ARM64_DBGBCR0_EL1, 0);
    }
    _InstructionSynchronizationBarrier();

    Count = READ_ARM64_FEATURE(ARM64_ID_AA64DFR0_EL1, DFR0_WATCHPOINT_COUNT);
    switch (Count) {
    case 15:    _WriteStatusReg(ARM64_DBGWCR15_EL1, 0);
    case 14:    _WriteStatusReg(ARM64_DBGWCR14_EL1, 0);
    case 13:    _WriteStatusReg(ARM64_DBGWCR13_EL1, 0);
    case 12:    _WriteStatusReg(ARM64_DBGWCR12_EL1, 0);
    case 11:    _WriteStatusReg(ARM64_DBGWCR11_EL1, 0);
    case 10:    _WriteStatusReg(ARM64_DBGWCR10_EL1, 0);
    case 9:     _WriteStatusReg(ARM64_DBGWCR9_EL1, 0);
    case 8:     _WriteStatusReg(ARM64_DBGWCR8_EL1, 0);
    case 7:     _WriteStatusReg(ARM64_DBGWCR7_EL1, 0);
    case 6:     _WriteStatusReg(ARM64_DBGWCR6_EL1, 0);
    case 5:     _WriteStatusReg(ARM64_DBGWCR5_EL1, 0);
    case 4:     _WriteStatusReg(ARM64_DBGWCR4_EL1, 0);
    case 3:     _WriteStatusReg(ARM64_DBGWCR3_EL1, 0);
    case 2:     _WriteStatusReg(ARM64_DBGWCR2_EL1, 0);
    case 1:     _WriteStatusReg(ARM64_DBGWCR1_EL1, 0);
    case 0:     _WriteStatusReg(ARM64_DBGWCR0_EL1, 0);
    }
    _InstructionSynchronizationBarrier();

    //
    // Configure to allow debugging: enable kernel debugging
    // and disable user mode access to debug registers.
    //

    Value = _ReadStatusReg(ARM64_MDSCR_EL1);
    Value |= ARM64_MDSCR_MDE | ARM64_MDSCR_KDE | ARM64_MDSCR_TDCC;
    _WriteStatusReg(ARM64_MDSCR_EL1, Value);
    _InstructionSynchronizationBarrier();

    //
    // Enable debug exceptions again
    //

    Value = _ReadStatusReg(ARM64_DAIF);
    Value &= ~DAIF_DEBUG;
    _WriteStatusReg(ARM64_DAIF, Value);
    _InstructionSynchronizationBarrier();

    //
    // With the trap handlers installed, the boot debugger is prepared to
    // initiate a debugger connection at any time.
    //
}

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
    BdPrcb = &BdPcr.Prcb;

    BdArm64CleanInitDebug();

    BdArchBlockDebuggerOperation = FALSE;
    return STATUS_SUCCESS;
}

VOID
BdArchDestroy (
    VOID
    )

/*++

Routine Description:

    This function is responsible for freeing any resources allocated
    during BdArchInitialize.  For ARM, this just involves freeing the
    memory allocated for the boot debugger PCR.

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

    This function performs any architecture specific actions required
    before a debugger connection attempt can take place.  On ARM,
    install the appropriate exception vectors.

Arguments:

    None.

Return Value:

    None.

--*/

{
    //
    // Redo the clean init
    //

    BdArm64CleanInitDebug();

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
BdTrap (
    __in PEXCEPTION_RECORD ExceptionRecord,
    __in PKEXCEPTION_FRAME ExceptionFrame,
    __in PKTRAP_FRAME TrapFrame
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

    LOGICAL BoolRet;
    PCONTEXT ContextRecord;
    STRING Input;
    ULONG Opcode;
    ULONG_PTR OldPc;
    STRING Output;
    BOOLEAN ReturnValue;
    BOOLEAN UnloadSymbols;

    //
    // Indicate that the boot debugger has been entered.  Specify that it is a
    // critical interface to prevent potentially time consuming library code
    // from executing while communicating with the debugger host.
    //

    // BlpCriticalInterfaceEnter(InterfaceBootDebugger);

    //
    // Invalid instructions are post-processed in the loader and
    // kernel to set the right exception code and information.
    //

    if (ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) {
        if (GetCurrentEl() == 1) {
            Opcode = _ReadStatusReg(ARM64_ESR_EL1) & 0xffff;

        } else {
            Opcode = _ReadStatusReg(ARM64_ESR_EL2) & 0xffff;
        }

        switch(Opcode) {
        case ARM64_BREAKPOINT:
        case ARM64_DEBUG_SERVICE:
            ExceptionRecord->ExceptionCode = STATUS_BREAKPOINT;
            ExceptionRecord->NumberParameters = 1;
            if (Opcode == ARM64_BREAKPOINT) {
                ExceptionRecord->ExceptionInformation[0] = BREAKPOINT_BREAK;
            } else {

                ExceptionRecord->ExceptionInformation[0] = TrapFrame->X[16];
                TrapFrame->Pc += 4;
            }
            break;

        case ARM64_ASSERT:
            ExceptionRecord->ExceptionCode = STATUS_ASSERTION_FAILURE;
            ExceptionRecord->NumberParameters = 0;
            ExceptionRecord->ExceptionAddress = (PVOID) TrapFrame->Pc;
            break;

        case ARM64_FASTFAIL:
            ExceptionRecord->ExceptionCode = STATUS_STACK_BUFFER_OVERRUN;
            ExceptionRecord->NumberParameters = 1;
            ExceptionRecord->ExceptionInformation[0] = TrapFrame->X[0];
            break;

        default:
            ExceptionRecord->ExceptionCode = STATUS_ILLEGAL_INSTRUCTION;
            ExceptionRecord->ExceptionAddress = (PVOID) TrapFrame->Pc;
        }
    }

    //
    // Set address of context record and set context flags.
    //

    ContextRecord = &BdPrcb->ProcessorState.ContextFrame;
    ContextRecord->ContextFlags = CONTEXT_FULL;

    //
    // Print, Prompt, Load symbols, Unload symbols, are all special
    // cases of STATUS_BREAKPOINT.
    //

    UnloadSymbols = FALSE;
    if ((ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) &&
        (ExceptionRecord->ExceptionInformation[0] != BREAKPOINT_BREAK)) {

        //
        // Switch on the breakpoint code.
        //

        switch (ExceptionRecord->ExceptionInformation[0]) {

            //
            // Print a debug string.
            //
            // Arguments:
            //
            //     r0 - Supplies a pointer to an output string buffer.
            //     r1 - Supplies the length of the output string buffer.
            //     r2 - Supplies the Id of the calling component.
            //     r3 - Supplies the output filter level.
            //

        case BREAKPOINT_PRINT:
            Output.Buffer = (PCHAR)TrapFrame->X[0];
            Output.Length = (USHORT)TrapFrame->X[1];
            if (BdDebuggerNotPresent == FALSE) {
                BoolRet = BdPrintString(&Output);
                if (BoolRet == FALSE) {
                    TrapFrame->X[0] = (ULONG) STATUS_SUCCESS;

                } else {
                    TrapFrame->X[0] = (ULONG) STATUS_BREAKPOINT;
                }

            } else {
                TrapFrame->X[0] = (ULONG) STATUS_DEVICE_NOT_CONNECTED;
            }

            TrapFrame->Pc += 4;
            ReturnValue = TRUE;
            goto BdTrapEnd;

            //
            // Print a debug prompt string, then input a string.
            //
            // Arguments:
            //
            //     r0 - Supplies a pointer to an output string buffer.
            //     r1 - Supplies the length of the output string buffer.
            //     r2 - Supplies a pointer to an input string buffer.
            //     r3 - Supplies the length of the input string bufffer.
            //

        case BREAKPOINT_PROMPT:

            Output.Buffer = (PCHAR)TrapFrame->X[0];
            Output.Length = (USHORT)TrapFrame->X[1];
            Input.Buffer = (PCHAR)TrapFrame->X[2];
            Input.Length = (USHORT)TrapFrame->X[3];

            //
            // Continue to prompt until no breakin is seen.
            //

            do {
                BoolRet = BdPromptString(&Output, &Input);
            } while (BoolRet != FALSE);

            TrapFrame->X[0] = Input.Length;
            TrapFrame->Pc += 4;
            ReturnValue = TRUE;
            goto BdTrapEnd;

            //
            // Load the symbolic information for an image.
            //
            // Arguments:
            //
            //    r0 - Supplies a pointer to a filename string descriptor.
            //    r1 - Supplies the base address of the image.
            //

        case BREAKPOINT_UNLOAD_SYMBOLS:
            UnloadSymbols = TRUE;

            //
            // Fall through
            //

        case BREAKPOINT_LOAD_SYMBOLS:
            BdSaveKframe(TrapFrame, ExceptionFrame, ContextRecord);
            OldPc = ContextRecord->Pc;
            if (BdDebuggerNotPresent == FALSE) {
                BdReportLoadSymbolsStateChange((PSTRING)TrapFrame->X[0],
                                               (PKD_SYMBOLS_INFO)TrapFrame->X[1],
                                               UnloadSymbols,
                                               ContextRecord);
            }

            //
            // If the kernel debugger did not update PC, then increment
            // past the breakpoint instruction.
            //

            if (ContextRecord->Pc == OldPc) {
                ContextRecord->Pc += 4;
            }

            BdRestoreKframe(TrapFrame, ExceptionFrame, ContextRecord);
            ReturnValue = TRUE;
            goto BdTrapEnd;

        case BREAKPOINT_GET_TABLE:
            TrapFrame->X[0] = (UINT64)&BdDebugTable;
            TrapFrame->Pc += 4;
            ReturnValue = TRUE;
            goto BdTrapEnd;

            //
            // Unknown command.
            //

        default:
            ReturnValue = FALSE;
            goto BdTrapEnd;
        }

    } else {

        //
        // Report state change to the kernel debugger.
        //

        BdSaveKframe(TrapFrame, ExceptionFrame, ContextRecord);
        BdReportExceptionStateChange(ExceptionRecord,
                                     &BdPrcb->ProcessorState.ContextFrame,
                                     TRUE);

        BdRestoreKframe(TrapFrame, ExceptionFrame, ContextRecord);
        BdControlCPressed = FALSE;
        ReturnValue = TRUE;
        goto BdTrapEnd;
    }

 BdTrapEnd:
    // BlpCriticalInterfaceExit(InterfaceBootDebugger);
    return ReturnValue;
}

LOGICAL
BdStub (
    __in PEXCEPTION_RECORD ExceptionRecord,
    __in PKEXCEPTION_FRAME ExceptionFrame,
    __in PKTRAP_FRAME TrapFrame
    )

/*++

Routine Description:

    This routine provides a kernel debugger stub routine to catch debug
    prints when the boot debugger is not active.

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

    ULONG_PTR BreakpointCode;

    BreakpointCode = ExceptionRecord->ExceptionInformation[0];

    UNREFERENCED_PARAMETER(ExceptionFrame);

    //
    // If the breakpoint is a debug print or load/unload symbols, then return
    // TRUE. Otherwise, return FALSE.
    //

    if ((ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) &&
        (ExceptionRecord->NumberParameters > 0) &&
        ((BreakpointCode == BREAKPOINT_LOAD_SYMBOLS) ||
         (BreakpointCode == BREAKPOINT_UNLOAD_SYMBOLS) ||
         (BreakpointCode == BREAKPOINT_PRINT))) {

        TrapFrame->Pc += 4;
        return TRUE;

    } else {
        return FALSE;
    }
}

VOID
BdRestoreKframe (
    __inout PKTRAP_FRAME TrapFrame,
    __inout PKEXCEPTION_FRAME ExceptionFrame,
    __in PCONTEXT ContextRecord
    )

/*++

Routine Description:

    This function copies processor state from a context record into the trap
    and exception frames used to load this state into the processor.
    Processor control state from the PRCB is restored directly into the
    processor.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame.

    ExceptionFrame - Suppliers a pointer to an exception frame.

    ContextRecord - Supplies a pointer to a context record.

Return Value:

    None.

--*/

{

    //
    // Copy information from context record to trap frame.
    //
    // Copy control information (SP, LR, PC, CSPR)
    //

    TrapFrame->Fp = ContextRecord->Fp;
    TrapFrame->Lr = ContextRecord->Lr;
    TrapFrame->Sp = ContextRecord->Sp;
    TrapFrame->Pc = ContextRecord->Pc;
    TrapFrame->Spsr = ContextRecord->Cpsr;

    //
    // Copy volatile integer register (X0-X17) contents into
    // the trap frame.
    //

    TrapFrame->X[0] = ContextRecord->X[0];
    TrapFrame->X[1] = ContextRecord->X[1];
    TrapFrame->X[2] = ContextRecord->X[2];
    TrapFrame->X[3] = ContextRecord->X[3];
    TrapFrame->X[4] = ContextRecord->X[4];
    TrapFrame->X[5] = ContextRecord->X[5];
    TrapFrame->X[6] = ContextRecord->X[6];
    TrapFrame->X[7] = ContextRecord->X[7];
    TrapFrame->X[8] = ContextRecord->X[8];
    TrapFrame->X[9] = ContextRecord->X[9];
    TrapFrame->X[10] = ContextRecord->X[10];
    TrapFrame->X[11] = ContextRecord->X[11];
    TrapFrame->X[12] = ContextRecord->X[12];
    TrapFrame->X[13] = ContextRecord->X[13];
    TrapFrame->X[14] = ContextRecord->X[14];
    TrapFrame->X[15] = ContextRecord->X[15];
    TrapFrame->X[16] = ContextRecord->X[16];
    TrapFrame->X[17] = ContextRecord->X[17];

    //
    // Copy non-volatile integer register (R4-R11) contents into the
    // exception frame.
    //

    ExceptionFrame->X19 = ContextRecord->X[19];
    ExceptionFrame->X20 = ContextRecord->X[20];
    ExceptionFrame->X21 = ContextRecord->X[21];
    ExceptionFrame->X22 = ContextRecord->X[22];
    ExceptionFrame->X23 = ContextRecord->X[23];
    ExceptionFrame->X24 = ContextRecord->X[24];
    ExceptionFrame->X25 = ContextRecord->X[25];
    ExceptionFrame->X26 = ContextRecord->X[26];
    ExceptionFrame->X27 = ContextRecord->X[27];
    ExceptionFrame->X28 = ContextRecord->X[28];

#ifdef NOTYET

    //
    // Restore processor control state.
    //

    KiRestoreProcessorControlState(&BdPrcb->ProcessorState);
#endif
    return;
}

VOID
BdSaveKframe (
    __in PKTRAP_FRAME TrapFrame,
    __in PKEXCEPTION_FRAME ExceptionFrame,
    __inout PCONTEXT ContextRecord
    )

/*++

Routine Description:

    This function copies processor state captured in the given trap and
    exception frames into the supplied context record.  Processor control
    state is captured directly from the processor and saved in the PRCB.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame.

    ExceptionFrame - Supplies a pointer to an exception frame.

    ContextRecord - Supplies a pointer to a context record.

Return Value:

    None.

--*/

{

    //
    // Copy information from trap frame to context record.
    //
    // Copy control information (SP, LR, PC, CSPR)
    //
    ContextRecord->Fp = TrapFrame->Fp;
    ContextRecord->Lr = TrapFrame->Lr;
    ContextRecord->Sp = TrapFrame->Sp;
    ContextRecord->Pc = TrapFrame->Pc;
    ContextRecord->Cpsr = TrapFrame->Spsr;

    //
    // Copy volatile integer register contents (R0-R3, R12) from
    // the trap frame.
    //

    ContextRecord->X[0] = TrapFrame->X[0];
    ContextRecord->X[1] = TrapFrame->X[1];
    ContextRecord->X[2] = TrapFrame->X[2];
    ContextRecord->X[3] = TrapFrame->X[3];
    ContextRecord->X[4] = TrapFrame->X[4];
    ContextRecord->X[5] = TrapFrame->X[5];
    ContextRecord->X[6] = TrapFrame->X[6];
    ContextRecord->X[7] = TrapFrame->X[7];
    ContextRecord->X[8] = TrapFrame->X[8];
    ContextRecord->X[9] = TrapFrame->X[9];
    ContextRecord->X[10] = TrapFrame->X[10];
    ContextRecord->X[11] = TrapFrame->X[11];
    ContextRecord->X[12] = TrapFrame->X[12];
    ContextRecord->X[13] = TrapFrame->X[13];
    ContextRecord->X[14] = TrapFrame->X[14];
    ContextRecord->X[15] = TrapFrame->X[15];
    ContextRecord->X[16] = TrapFrame->X[16];
    ContextRecord->X[17] = TrapFrame->X[17];

    //
    // Copy non-volatile integer register contents (R4-R11) from the
    // exception frame.
    //

    ContextRecord->X[19] = ExceptionFrame->X19;
    ContextRecord->X[20] = ExceptionFrame->X20;
    ContextRecord->X[21] = ExceptionFrame->X21;
    ContextRecord->X[22] = ExceptionFrame->X22;
    ContextRecord->X[23] = ExceptionFrame->X23;
    ContextRecord->X[24] = ExceptionFrame->X24;
    ContextRecord->X[25] = ExceptionFrame->X25;
    ContextRecord->X[26] = ExceptionFrame->X26;
    ContextRecord->X[27] = ExceptionFrame->X27;
    ContextRecord->X[28] = ExceptionFrame->X28;

    //
    // Save processor control state.
    //

    KiSaveProcessorControlState(&BdPrcb->ProcessorState);
    return;
}

VOID
BdInstallTrapVectors (
    VOID
    )

/*++

Routine Description:

    This routine installs the boot debugger vector table.

Arguments:

    None.

Return Value:

    None.

--*/

{
    //BlpArchInstallExceptionVectors(BdExceptionVectors);

    ULONG_PTR VectorAddress;

    //
    // ARM64 exceptions (including interrupts) result in an instruction
    // fetch from a vector table at a fixed physical or virtual
    // address (depending on whether translation is enabled).
    // Handlers are installed by copying a set of instructions and
    // vector table to the address.  This must be followed by ensuring
    // that the I/D caches are consistent.
    //

    //
    // Vector table must be 2048 byte aligned.
    //

    VectorAddress = (ULONG_PTR) BdExceptionVectors;
    NT_ASSERT( (VectorAddress & 0x7ff) == 0);

    if (GetCurrentEl() == 1) {
        _WriteStatusReg(ARM64_VBAR_EL1, VectorAddress);

    } else {

        NT_ASSERT(GetCurrentEl() == 2);

        _WriteStatusReg(ARM64_VBAR_EL2, VectorAddress);
    }

    _DataSynchronizationBarrier();
    _InstructionSynchronizationBarrier();
    return;
}
