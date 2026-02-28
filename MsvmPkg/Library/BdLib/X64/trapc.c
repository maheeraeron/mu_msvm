/*++

Copyright (c) 1990-2004  Microsoft Corporation

Module Name:

    trapc.c

Abstract:

    This module contains the code to setup the IDT for the boot debugger.

Author:

    Jamie Schwartz (jamschw) Oct. 2004

Environment:

    Boot

--*/

// ------------------------------------------------------------------- Includes

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include "Bd.h"
#include "CrashDump.h"

// -------------------------------------------------------------------- Pragmas

#pragma warning(disable:4152)      // Function pointer to data pointer.
#pragma warning(disable:4201)

// ------------------------------------------------------------------ Functions

typedef union _KIDT_HANDLER_ADDRESS {
    struct {
        UINT16 OffsetLow;
        UINT16 OffsetMiddle;
        UINT32 OffsetHigh;
    };

    UINT64 Address;
} KIDT_HANDLER_ADDRESS, *PKIDT_HANDLER_ADDRESS;

VOID
ArchSetIdtEntry (
    _In_ IA32_IDT_GATE_DESCRIPTOR *IdtBase,
    _In_ UINT32 Offset,
    _In_ PVOID InterruptHandler,
    _In_ UINT16 Access,
    _In_ UINT16 Selector
    )
/*++

Routine Description:

    Initializes the specified interrupt vector with the provided ISR.

Arguments:

    IdtBase - Pointer to the base of the Interrupt Descriptor Table.

    Offset - Offset in the IDT for the interrupt vector to hook.

    InterruptHandler - Address to the interrupt handler.

    Access - Access rights for the interrupt vector.

    Selector - Code selector for the interrupt handler.

Return Value:

    None.

--*/
{

    KIDT_HANDLER_ADDRESS handlerAddress;
    IA32_IDT_GATE_DESCRIPTOR *idt;

    idt = Add2Ptr(IdtBase, Offset);

    //
    // Use KIDT_HANDLER_ADDRESS structure to facilitate setting Offset
    // fields in IDT entry.
    //

    handlerAddress.Address = (UINTN)InterruptHandler;

    idt->Bits.OffsetLow = handlerAddress.OffsetLow;
    idt->Bits.OffsetHigh = handlerAddress.OffsetMiddle;
    idt->Bits.OffsetUpper = handlerAddress.OffsetHigh;
    idt->Bits.Selector = Selector;
    idt->Bits.Reserved_0 = (Access & 0xFF);
    idt->Bits.GateType = (Access >> 8);
}


VOID
BdPatchIdt (
    _Inout_bytecount_(IdtLength) PVOID Idt,
    _In_    UINT32      IdtLength,
    _In_    UINT16      CodeSegment
    )

/*++

Routine Description:

    This routine will install trap vectors in entries 0 through 0x2d inclusive
    in the given Interrupt Descriptor Table.

Arguments:

    Idt - Supplies a pointer to the IDT to patch.

    IdtLength - Supplies the size of the IDT in bytes.

    CodeSegment - Supplies a pointer to the code segment to use when patching
        the IDT.

Return Value:

    None.

--*/

{
    IA32_IDT_GATE_DESCRIPTOR *idtBase;
    UINT32 index;

    idtBase = (IA32_IDT_GATE_DESCRIPTOR*)Idt;

    if (IdtLength < (0x2d * sizeof(IA32_IDT_GATE_DESCRIPTOR)))
    {
        return;
    }

    //
    // Initialize the entries to the default unhandled exception handler.
    // Then insert specific exception handlers.
    //

    for (index = 0;
         index <= 0x2d;
         index += 1)
    {
        //
        // Leave isolation-related exception handlers alone if they are installed.
        //

        if ((idtBase[index].Bits.GateType == 0) ||
            ((index != 0x1D) && (index != 0x14)))
        {
            ArchSetIdtEntry(idtBase,
                            index * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                            BdUnhandledException,
                            0x8e00,
                            CodeSegment);
        }
    }

    //
    // Trap 00 : Divide by Zero fault.
    //

    ArchSetIdtEntry(idtBase,
                    0,
                    BdDivideError,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 01 : Instruction breakpoint, data address breakpoint, general
    //           detection fault, single step trap and TSS breakpoint.
    //

    ArchSetIdtEntry(idtBase,
                    0x01 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdDebugTrapOrFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 02 : NMI.
    //

    ArchSetIdtEntry(idtBase,
                    0x02 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdNmiInterrupt,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 03 : Breakpoint.
    //

    ArchSetIdtEntry(idtBase,
                    0x03 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdBreakpointTrap,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 04 : Overflow fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x04 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdOverflowTrap,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 05 : Bound fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x05 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdBoundFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 06 : Invalid opcode fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x06 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdInvalidOpcodeFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 07 : Device not available.
    //

    ArchSetIdtEntry(idtBase,
                    0x07 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdNpxNotAvailableFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 08 : Double fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x08 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdDoubleFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 0a : Invalid TSS fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x0A * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdInvalidTss,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 0d : General Protection Fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x0d * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdGeneralProtectionFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 0e : Page Fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x0e * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdPageFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 10 : Floating point fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x10 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdFloatingPointFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 11 : Alignment check fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x11 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdAlignmentFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 12 : Machine check abort.
    //

    ArchSetIdtEntry(idtBase,
                    0x12 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdMachineCheckAbort,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 13 : SIMD floating point fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x13 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdXmmException,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 29 : fast fail.
    //

    ArchSetIdtEntry(idtBase,
                    0x29 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdFastFailTrap,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 2c : assertion failure.
    //

    ArchSetIdtEntry(idtBase,
                    0x2c * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdAssertionFailureTrap,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 2d : Debug service.
    //

    ArchSetIdtEntry(idtBase,
                    0x2d * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    BdDebugServiceTrap,
                    0x8e00,
                    CodeSegment);

    return;
}

VOID
BdInstallTrapVectors (
    VOID
    )
/*++

Routine Description:

    This routine will hook the IDT for the boot debugger.

Arguments:

    None.

Return Value:

    None.

--*/
{
    UINT16 codeSegment;
    IA32_DESCRIPTOR idtRegister;

    //
    // Hook the interrupt descriptor table for the boot debugger.
    //

    AsmReadIdtr(&idtRegister);

    if (idtRegister.Limit < (0x2d * sizeof(IA32_IDT_GATE_DESCRIPTOR)))
    {
        //
        // We do not have enought IDT entries to setup the debuggers.
        // Skip installing trap vectors.
        //

        return;
    }

    //
    // All traps should execute in the context of the current boot
    // application. Hence initialize each IDT entry with the current
    // application's code segment selector value.
    //

    codeSegment = AsmReadCs();
    BdPatchIdt((PVOID)idtRegister.Base, idtRegister.Limit, codeSegment);
    AsmWriteIdtr(&idtRegister);

    return;
}


VOID
EfiDispatchException (
    _In_    PEXCEPTION_RECORD   ExceptionRecord,
    _In_    PKEXCEPTION_FRAME   ExceptionFrame,
    _In_    PKTRAP_FRAME        TrapFrame
    )
/*++

Routine Description:

    This routine is called whenever a exception is dispatched.

Arguments:

    ExceptionRecord - Supplies a pointer to an exception record that
        describes the exception.

    ExceptionFrame - Supplies a pointer to an exception frame (NULL).

    TrapFrame - Supplies a pointer to a trap frame that describes the
        trap.

Return Value:

    None.  This routine either handles the exception or bugchecks.

--*/
{
    //
    // See if the debugger will handle the exception first.
    // If unhandled, bugcheck.
    //
    if (!BdDispatchException(ExceptionRecord, ExceptionFrame, TrapFrame))
    {
        PCONTEXT ContextRecord = &BdPrcb->ProcessorState.ContextFrame;

        ContextRecord->ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
        BdSaveKframe(TrapFrame, ExceptionFrame, ContextRecord);

        EfiBugCheckWithContext(ContextRecord,
            ExceptionRecord,
            KMODE_EXCEPTION_NOT_HANDLED,
            ExceptionRecord->ExceptionCode,
            (UINTN)ExceptionRecord->ExceptionAddress,
            ExceptionRecord->ExceptionInformation[1],
            ExceptionRecord->ExceptionInformation[2]
            );

    }

}


VOID
BdRestoreKframe (
    _Inout_ PKTRAP_FRAME        TrapFrame,
    _Inout_ PKEXCEPTION_FRAME   ExceptionFrame,
    _In_    PCONTEXT            ContextRecord
    )
/*++

Routine Description:

    This function copies processor state from a context record into the trap
    and exception frames used to load this state into the processor.
    Processor control state from the PRCB is restored directly into the
    processor.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame.

    ExceptionFrame - Supplies a pointer to an exception frame.

    ContextRecord - Supplies a pointer to a context record.

Return Value:

    None.

--*/
{
    //
    // Copy information from context record to trap frame.
    //
    // Copy control information.
    //

    TrapFrame->Rbp = ContextRecord->Rbp;
    TrapFrame->Rip = ContextRecord->Rip;
    TrapFrame->SegCs = ContextRecord->SegCs;
    TrapFrame->SegSs = ContextRecord->SegSs;
    TrapFrame->EFlags = ContextRecord->EFlags;

    //
    // Copy volatile integer register contents into the trap frame.
    //

    TrapFrame->Rax = ContextRecord->Rax;
    TrapFrame->Rcx = ContextRecord->Rcx;
    TrapFrame->Rdx = ContextRecord->Rdx;
    TrapFrame->R8  = ContextRecord->R8;
    TrapFrame->R9  = ContextRecord->R9;
    TrapFrame->R10 = ContextRecord->R10;
    TrapFrame->R11 = ContextRecord->R11;

    //
    // Copy non-volatile integer register contents into the exception frame.
    //

    ExceptionFrame->Rbx = ContextRecord->Rbx;
    ExceptionFrame->Rsi = ContextRecord->Rsi;
    ExceptionFrame->Rdi = ContextRecord->Rdi;
    ExceptionFrame->R12 = ContextRecord->R12;
    ExceptionFrame->R13 = ContextRecord->R13;
    ExceptionFrame->R14 = ContextRecord->R14;
    ExceptionFrame->R15 = ContextRecord->R15;

    //
    // Restore processor control state.
    //

    KiRestoreProcessorControlState(&BdPrcb->ProcessorState);
    return;
}


VOID
BdSaveKframe (
    _In_    PKTRAP_FRAME        TrapFrame,
    _In_    PKEXCEPTION_FRAME   ExceptionFrame,
    _Inout_ PCONTEXT            ContextRecord
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
    // Copy control information.
    //

    ContextRecord->Rbp = TrapFrame->Rbp;
    ContextRecord->Rip = TrapFrame->Rip;
    ContextRecord->SegCs = TrapFrame->SegCs;
    ContextRecord->SegSs = TrapFrame->SegSs;
    ContextRecord->EFlags = TrapFrame->EFlags;
    ContextRecord->Rsp = TrapFrame->Rsp;

    //
    // Set segment registers.
    //

    ContextRecord->SegDs = BAGDT_DATA_SELECTOR;
    ContextRecord->SegEs = BAGDT_DATA_SELECTOR;
    ContextRecord->SegFs = BAGDT_DATA_SELECTOR;
    ContextRecord->SegGs = BAGDT_DATA_SELECTOR;

    //
    // Copy volatile integer register contents from the trap frame.
    //

    ContextRecord->Rax = TrapFrame->Rax;
    ContextRecord->Rcx = TrapFrame->Rcx;
    ContextRecord->Rdx = TrapFrame->Rdx;
    ContextRecord->R8 = TrapFrame->R8;
    ContextRecord->R9 = TrapFrame->R9;
    ContextRecord->R10 = TrapFrame->R10;
    ContextRecord->R11 = TrapFrame->R11;

    //
    // Copy non-volatile integer register contents from the exception frame.
    //

    ContextRecord->Rbx = ExceptionFrame->Rbx;
    ContextRecord->Rdi = ExceptionFrame->Rdi;
    ContextRecord->Rsi = ExceptionFrame->Rsi;
    ContextRecord->R12 = ExceptionFrame->R12;
    ContextRecord->R13 = ExceptionFrame->R13;
    ContextRecord->R14 = ExceptionFrame->R14;
    ContextRecord->R15 = ExceptionFrame->R15;

    //
    // Save processor control state.
    //

    KiSaveProcessorControlState(&BdPrcb->ProcessorState);
    return;
}

#pragma warning(default:4152)
#pragma warning(default:4201)
