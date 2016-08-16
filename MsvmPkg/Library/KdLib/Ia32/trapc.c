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
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>

#include "EfiKd.h"
#include "CrashDump.h"

// -------------------------------------------------------------------- Pragmas

#pragma warning(disable:4152)      // Function pointer to data pointer.

// ------------------------------------------------------------------ Functions

typedef union _KIDT_HANDLER_ADDRESS {
    struct {
        UINT16 OffsetLow;
        UINT16 OffsetMiddle;
        UINT32 OffsetHigh;
    };

    UINT64 Address;
} KIDT_HANDLER_ADDRESS, *PKIDT_HANDLER_ADDRESS;

#define Add2Ptr(P,I) ((PVOID)((PUCHAR)(P) + (I)))

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
    idt->Bits.Selector = Selector;
    idt->Bits.Reserved_0 = (Access & 0xFF);
    idt->Bits.GateType = (Access >> 8);

    // X64 specific structure field
#if defined(MDE_CPU_X64)
    idt->Bits.OffsetUpper = handlerAddress.OffsetHigh;
#endif
}


void TempUnhandledException() {
    DEBUG((DEBUG_INFO, "DDD Called Unhandled Exception handler\n"));
}

void EfiKdDivideError() {
    DEBUG((DEBUG_INFO, "DDD Called EfiKdDivideError \n"));
}


void EfiKdNmiInterrupt() {
    DEBUG((DEBUG_INFO, "DDD Called EfiKdNmiInterrupt \n"));
}


void EfiKdOverflowTrap() {
    DEBUG((DEBUG_INFO, "DDD Called EfiKdOverflowTrap \n"));
}


void EfiKdBoundFault() {
    DEBUG((DEBUG_INFO, "DDD Called EfiKdBoundFault \n"));
}


void EfiKdInvalidOpcodeFault() {
    DEBUG((DEBUG_INFO, "DDD Called EfiKdInvalidOpcodeFault \n"));
}


void EfiKdNpxNotAvailableFault() {
    DEBUG((DEBUG_INFO, "DDD Called EfiKdNpxNotAvailableFault \n"));
}


void EfiKdDoubleFault() {
    DEBUG((DEBUG_INFO, "DDD Called EfiKdDoubleFault \n"));
}


void EfiKdInvalidTss() {
    DEBUG((DEBUG_INFO, "DDD Called EfiKdInvalidTss \n"));
}


void EfiKdFloatingPointFault() {
    DEBUG((DEBUG_INFO, "DDD Called EfiKdFloatingPointFault \n"));
}


void EfiKdAlignmentFault() {
    DEBUG((DEBUG_INFO, "DDD Called EfiKdAlignmentFault \n"));
}


void EfiKdMachineCheckAbort() {
    DEBUG((DEBUG_INFO, "DDD Called EfiKdMachineCheckAbort \n"));
}


void EfiKdXmmException() {
    DEBUG((DEBUG_INFO, "DDD Called EfiKdXmmException \n"));
}

VOID
EfiKdPatchIdt (
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
        ArchSetIdtEntry(idtBase,
                        index * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                        // EfiKdUnhandledException,
                        TempUnhandledException,
                        0x8e00,
                        CodeSegment);
    }

    //
    // Trap 00 : Divide by Zero fault.
    //

    ArchSetIdtEntry(idtBase,
                    0,
                    EfiKdDivideError,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 01 : Instruction breakpoint, data address breakpoint, general
    //           detection fault, single step trap and TSS breakpoint.
    //

    ArchSetIdtEntry(idtBase,
                    0x01 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdDebugTrapOrFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 02 : NMI.
    //

    ArchSetIdtEntry(idtBase,
                    0x02 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdNmiInterrupt,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 03 : Breakpoint.
    //

    ArchSetIdtEntry(idtBase,
                    0x03 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdBreakpointTrap,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 04 : Overflow fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x04 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdOverflowTrap,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 05 : Bound fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x05 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdBoundFault,
                    0x8e00,
                    CodeSegment);
    //
    // Trap 06 : Invalid opcode fault.
    //


    ArchSetIdtEntry(idtBase,
                    0x06 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdInvalidOpcodeFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 07 : Device not available.
    //

    ArchSetIdtEntry(idtBase,
                    0x07 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdNpxNotAvailableFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 08 : Double fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x08 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdDoubleFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 0a : Invalid TSS fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x0A * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdInvalidTss,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 0d : General Protection Fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x0d * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdGeneralProtectionFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 0e : Page Fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x0e * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdPageFault,
                    0x8e00,
                    CodeSegment);


    //
    // Trap 10 : Floating point fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x10 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdFloatingPointFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 11 : Alignment check fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x11 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdAlignmentFault,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 12 : Machine check abort.
    //

    ArchSetIdtEntry(idtBase,
                    0x12 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdMachineCheckAbort,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 13 : SIMD floating point fault.
    //

    ArchSetIdtEntry(idtBase,
                    0x13 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdXmmException,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 29 : fast fail.
    //

    ArchSetIdtEntry(idtBase,
                    0x29 * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdFastFailTrap,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 2c : assertion failure.
    //

    ArchSetIdtEntry(idtBase,
                    0x2c * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdAssertionFailureTrap,
                    0x8e00,
                    CodeSegment);

    //
    // Trap 2d : Debug service.
    //

    ArchSetIdtEntry(idtBase,
                    0x2d * sizeof(IA32_IDT_GATE_DESCRIPTOR),
                    EfiKdDebugServiceTrap,
                    0x8e00,
                    CodeSegment);

    return;
}

VOID
EfiKdInstallTrapVectors (
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
    EfiKdPatchIdt((PVOID)idtRegister.Base, idtRegister.Limit, codeSegment);
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
    DEBUG((DEBUG_INFO, "DDD EfiDispatchException\n"));
    //
    // See if the debugger will handle the exception first.
    // If unhandled, bugcheck.
    //
    if (!EfiKdDispatchException(ExceptionRecord, ExceptionFrame, TrapFrame))
    {
        TripleFault(0, 0, 0, 0);

        // CrashDumpSupport will be added later
/*
        PCONTEXT ContextRecord = &EfiKdPrcb->ProcessorState.ContextFrame;

        ContextRecord->ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
        EfiKdSaveKframe(TrapFrame, ExceptionFrame, ContextRecord);

        EfiBugCheckWithContext(ContextRecord,
            ExceptionRecord,
            KMODE_EXCEPTION_NOT_HANDLED,
            ExceptionRecord->ExceptionCode,
            (UINTN)ExceptionRecord->ExceptionAddress,
            ExceptionRecord->ExceptionInformation[1],
            ExceptionRecord->ExceptionInformation[2]
            );
*/
    }
}

VOID
EfiKdRestoreKframe (
    __inout PKTRAP_FRAME TrapFrame,
     _In_    PKEXCEPTION_FRAME   ExceptionFrame,
    __in PCONTEXT ContextRecord
    )

/*++

Routine Description:

    This functions copie the processor state from a context record and
    the processor control block into the trap frame.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame.

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

    TrapFrame->Ebp = ContextRecord->Ebp;
    TrapFrame->Eip = ContextRecord->Eip;
    TrapFrame->SegCs = ContextRecord->SegCs;
    TrapFrame->EFlags = ContextRecord->EFlags;

    //
    // Copy segment register contents.
    //

    TrapFrame->SegDs = ContextRecord->SegDs;
    TrapFrame->SegEs = ContextRecord->SegEs;
    TrapFrame->SegFs = ContextRecord->SegFs;
    TrapFrame->SegGs = ContextRecord->SegGs;

    //
    // Copy integer registers contents.
    //

    TrapFrame->Edi = ContextRecord->Edi;
    TrapFrame->Esi = ContextRecord->Esi;
    TrapFrame->Ebx = ContextRecord->Ebx;
    TrapFrame->Ecx = ContextRecord->Ecx;
    TrapFrame->Edx = ContextRecord->Edx;
    TrapFrame->Eax = ContextRecord->Eax;

    //
    // Restore processor state.
    //

    KiRestoreProcessorControlState(&EfiKdPrcb->ProcessorState);
    return;
}

VOID
EfiKdSaveKframe (
    __in PKTRAP_FRAME TrapFrame,
     _In_    PKEXCEPTION_FRAME   ExceptionFrame,
    __inout PCONTEXT ContextRecord
    )

/*++

Routine Description:

    This functions copis the processor state from a trap frame and the
    processor control block into a context record.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame.

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

    ContextRecord->Ebp = TrapFrame->Ebp;
    ContextRecord->Eip = TrapFrame->Eip;
    ContextRecord->SegCs = TrapFrame->SegCs & SEGMENT_MASK;
    ContextRecord->EFlags = TrapFrame->EFlags;
    ContextRecord->Esp = TrapFrame->TempEsp;
    ContextRecord->SegSs = TrapFrame->TempSegCs;

    //
    // Copy segment register contents.
    //

    ContextRecord->SegDs = TrapFrame->SegDs & SEGMENT_MASK;
    ContextRecord->SegEs = TrapFrame->SegEs & SEGMENT_MASK;
    ContextRecord->SegFs = TrapFrame->SegFs & SEGMENT_MASK;
    ContextRecord->SegGs = TrapFrame->SegGs & SEGMENT_MASK;

    //
    // Copy the integer register contents.
    //

    ContextRecord->Eax = TrapFrame->Eax;
    ContextRecord->Ebx = TrapFrame->Ebx;
    ContextRecord->Ecx = TrapFrame->Ecx;
    ContextRecord->Edx = TrapFrame->Edx;
    ContextRecord->Edi = TrapFrame->Edi;
    ContextRecord->Esi = TrapFrame->Esi;

    //
    // Copy debug register contents.
    //

    ContextRecord->Dr0 = TrapFrame->Dr0;
    ContextRecord->Dr1 = TrapFrame->Dr1;
    ContextRecord->Dr2 = TrapFrame->Dr2;
    ContextRecord->Dr3 = TrapFrame->Dr3;
    ContextRecord->Dr6 = TrapFrame->Dr6;
    ContextRecord->Dr7 = TrapFrame->Dr7;

    //
    // Save processor control state.
    //

    KiSaveProcessorControlState(&EfiKdPrcb->ProcessorState);
    return;
}

