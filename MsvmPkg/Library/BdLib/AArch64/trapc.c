/*++

Copyright (c) 2013  Microsoft Corporation

Module Name:

    trapc.c

Abstract:

    This module implements those portions of system exception handling that
    can be written in C.

Author:

    Aaron Giles

Environment:

    Kernel mode only.

--*/

#include "bd.h"

#define KI_EXCEPTION_INVALID_OP 0x10000002

#define KiCreateExceptionFrame(Frame) \
    (Frame)->X19 = __getCallerReg(19); \
    (Frame)->X20 = __getCallerReg(20); \
    (Frame)->X21 = __getCallerReg(21); \
    (Frame)->X22 = __getCallerReg(22); \
    (Frame)->X23 = __getCallerReg(23); \
    (Frame)->X24 = __getCallerReg(24); \
    (Frame)->X25 = __getCallerReg(25); \
    (Frame)->X26 = __getCallerReg(26); \
    (Frame)->X27 = __getCallerReg(27); \
    (Frame)->X28 = __getCallerReg(28); \
    (Frame)->Fp = __getCallerReg(29); \
    (Frame)->Return = (ULONG_PTR) _ReturnAddress(); \

VOID
BdAbortException (
    __inout PKTRAP_FRAME TrapFrame,
    __out PEXCEPTION_RECORD ExceptionRecord
    );

LOGICAL
BdTrap (
    __in PEXCEPTION_RECORD ExceptionRecord,
    __in PKEXCEPTION_FRAME ExceptionFrame,
    __in PKTRAP_FRAME TrapFrame
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

VOID
BdSynchronousException (
    __inout PKTRAP_FRAME TrapFrame
    )

/*++

Routine Description:

    This routine handles a synchronous exception.

Arguments:

    TrapFrame - Supplies a pointer to the trap frame.

Return Value:

    None.

--*/

{

    KEXCEPTION_FRAME ExceptionFrame;
    EXCEPTION_RECORD ExceptionRecord;
    ULONG64 ExceptionType;
    ULONG64 Value;

    //
    // Determine the incoming exception type from the upper 6 bits of the
    // ESR value stored in the TrapFrame. The default behavior (upon issuing
    // a break out of the switch statement) is to dispatch an exception
    // described in ExceptionRecord.
    //
    // If the exception is handled and should be returned from, a premature
    // return is issued instead of a break.
    //

    ExceptionType = (TrapFrame->Esr >> 26);
    switch (ExceptionType) {

    case 0x00:

        //
        // Unknown or Uncategorized Reason (illegal opcodes)
        //

        ExceptionRecord.ExceptionCode = KI_EXCEPTION_INVALID_OP;
        ExceptionRecord.ExceptionAddress = (PVOID) TrapFrame->Pc;
        ExceptionRecord.NumberParameters = 1;
        ExceptionRecord.ExceptionInformation[0] = TrapFrame->Spsr;
        break;

    case 0x20:
    case 0x21:
    case 0x24:
    case 0x25:

        //
        // Instruction (20/21) or data (24/25) abort from either
        // the current EL (21/25) or a lower EL (20/24)
        //

        BdAbortException(TrapFrame, &ExceptionRecord);
        break;

    case 0x22:

        //
        // PC alignment exception
        //

        ExceptionRecord.ExceptionCode = STATUS_INSTRUCTION_MISALIGNMENT;
        ExceptionRecord.ExceptionAddress = (PVOID) TrapFrame->Pc;
        ExceptionRecord.NumberParameters = 0;
        break;

    case 0x26:

        //
        // Stack alignment exception
        //

        ExceptionRecord.ExceptionCode = STATUS_DATATYPE_MISALIGNMENT;
        ExceptionRecord.ExceptionAddress = (PVOID) TrapFrame->Pc;
        ExceptionRecord.NumberParameters = 0;
        break;

    case 0x2c:

        //
        // Floating point exception from AArch64 mode
        //

        Value = _ReadStatusReg(ARM64_FPCR);
        Value &= ~(FPCR_IDE | FPCR_IXE | FPCR_UFE | FPCR_OFE | FPCR_DZE | FPCR_IOE);
        _WriteStatusReg(ARM64_FPCR, Value);
        return;

    case 0x2f:

        //
        // SError interrupt
        //

// ARM64_WORKITEM: What to do here?
        ExceptionRecord.ExceptionCode = STATUS_DATATYPE_MISALIGNMENT;
        ExceptionRecord.ExceptionAddress = (PVOID) TrapFrame->Pc;
        ExceptionRecord.NumberParameters = 0;
        break;

    case 0x30:
    case 0x31:

        //
        // Hardware breakpoint exception from current EL (31) or
        // from lower EL (30)
        //

        ExceptionRecord.ExceptionCode = STATUS_BREAKPOINT;
        ExceptionRecord.ExceptionAddress = (PVOID) TrapFrame->Pc;
        ExceptionRecord.NumberParameters = 1;
        ExceptionRecord.ExceptionInformation[0] = BREAKPOINT_HW_BREAK;
        break;

    case 0x32:
    case 0x33:

        //
        // Single step exception from current EL (33) or
        // from lower EL (32)
        //

        Value = _ReadStatusReg(ARM64_MDSCR_EL1);
        Value &= ~ARM64_MDSCR_SS;
        _WriteStatusReg(ARM64_MDSCR_EL1, Value);

        ExceptionRecord.ExceptionCode = STATUS_SINGLE_STEP;
        ExceptionRecord.ExceptionAddress = (PVOID) TrapFrame->Pc;
        ExceptionRecord.NumberParameters = 0;
        break;

    case 0x34:
    case 0x35:

        //
        // Hardware watchpoint exception from current EL (35) or
        // from lower EL (34)
        //

        ExceptionRecord.ExceptionCode = STATUS_BREAKPOINT;
        ExceptionRecord.ExceptionAddress = (PVOID) TrapFrame->Pc;
        ExceptionRecord.NumberParameters = 2;
        ExceptionRecord.ExceptionInformation[0] = BREAKPOINT_HW_WATCH;
        if (GetCurrentEl() == 1) {
            ExceptionRecord.ExceptionInformation[1] =
                _ReadStatusReg(ARM64_FAR_EL1);

        } else {
            ExceptionRecord.ExceptionInformation[1] =
                _ReadStatusReg(ARM64_FAR_EL2);

        }

        break;

    case 0x3c:

        //
        // BRK instruction in AArch64 mode
        //

        ExceptionRecord.ExceptionCode = STATUS_BREAKPOINT;
        ExceptionRecord.ExceptionAddress = (PVOID) TrapFrame->Pc;
        ExceptionRecord.NumberParameters = 2;
        ExceptionRecord.ExceptionInformation[0] = BREAKPOINT_BREAK;
        ExceptionRecord.ExceptionInformation[1] = TrapFrame->Esr & 0xffff;
        break;

    case 0x01:  // WFE/WFI traps
    case 0x03:	// MCR/MRC to invalid CP15 in AArch32 mode
    case 0x04:	// MCRR/MRRC to invalid CP15 in AArch32 mode
    case 0x05:	// MCR/MRC to invalid CP14 in AArch32 mode
    case 0x06:	// LDC/STC to invalid CP14 in AArch32 mode
    case 0x07:  // Invalid access to VFP/Advanced SIMD registers
    case 0x08:	// MRC to invalid CP10 in AArch32 mode
    case 0x0c:	// MCRR/MRRC to invalid CP14 in AArch32 mode
    case 0x0e:	// Illegal Instruction Set state (IL) bit
    case 0x11:	// SVC from AArch32 mode
    case 0x12:	// HVC from AArch32 mode
    case 0x13:	// SMC from AArch32 mode
    case 0x15:	// SVC from AArch64 mode (should be short-circuited)
    case 0x16:	// HVC from AArch64 mode
    case 0x17:	// SMC from AArch64 mode
    case 0x18:	// invalid MSR/MRS/SYS in AArch64 mode
    case 0x28:  // floating point exception from AArch32 mode
    case 0x38:  // BKPT from AArch32 mode
    case 0x3a:  // AArch32 mode vector catch
    default:

        //
        // Miscellaneous unhandled situations
        //

        while (1) { }
        break;
    }

    //
    // Enable interrupts if they were previously enabled, then dispatch
    // the exception.
    //

    if ((TrapFrame->Spsr & DAIF_INT) == 0) {
        _enable();
    }

    KiCreateExceptionFrame(&ExceptionFrame);
    BdTrap(&ExceptionRecord,
           &ExceptionFrame,
           TrapFrame);
}

VOID
BdAbortException (
    __inout PKTRAP_FRAME TrapFrame,
    __out PEXCEPTION_RECORD ExceptionRecord
    )

/*++

Routine Description:

    This routine handles both instruction and data abort exceptions.

Arguments:

    TrapFrame - Supplies a pointer to the trap frame.

    ExceptionRecord - Supplies a pointer to an exception record
        that must be filled out if this function returns FALSE.

Return Value:

    TRUE if the exception was handled; FALSE if an exception
    needs to be dispatched.

--*/

{

    ULONG_PTR FaultAddress;
    ULONG_PTR FaultStatus;
    ULONG FaultType;

    //
    // Enable interrupts if they were previously enabled.
    //

    if ((TrapFrame->Spsr & DAIF_INT) == 0) {
        _enable();
    }

    //
    // The fault type is provided by the low 6 bits of the ESR, as follows:
    //
    //      0x00-0x03   0000LL Address Size Fault
    //      0x04-0x07   0001LL Translation Fault
    //      0x08-0x0B   0010LL Access Flag Fault
    //      0x0C-0x0F   0011LL Permission Fault
    //      0x10        010000 Synchronous External Abort
    //      0x14-0x17   0101LL Synchronous External Abort on Translation Table walk
    //      0x18        011000 Memory access Synchronous Parity error
    //      0x1C-0x1F   0111LL Memory access Synchronous Parity error on Translation Table walk
    //      0x21        100001 Alignment Fault
    //      0x30        110000 TLB Conflict
    //      0x34        110100 IMPLEMENTATION DEFINED (Lockdown Abort)
    //      0x35        110101 IMPLEMENTATION DEFINED (unsupported exclusive)
    //      0x3A        111010 IMPLEMENTATION DEFINED (Coprocessor Aobrt)
    //      0x3D        111101 Section Domain Fault
    //      0x3E        111110 Page Domain Fault
    //


    if (GetCurrentEl() == 1) {
        FaultAddress = _ReadStatusReg(ARM64_FAR_EL1);

    } else {
        FaultAddress = _ReadStatusReg(ARM64_FAR_EL2);
    }


    switch (TrapFrame->Esr & 0x3f)
    {

    //
    // Address size, translation, access flag, and permission faults all
    // go through Mm for processing.
    //

    default:
    case 0x00:  case 0x01:  case 0x02:  case 0x03:
    case 0x04:  case 0x05:  case 0x06:  case 0x07:
    case 0x08:  case 0x09:  case 0x0a:  case 0x0b:
    case 0x0c:  case 0x0d:  case 0x0e:  case 0x0f:

        //
        // Determine whether this was an execution fault (implied if the fault
        // type was an Instruction Abort), and if not, whether it was a write
        // fault.
        //

        FaultType = TrapFrame->Esr >> 26;
        FaultStatus = 0;
        if (FaultType == 0x20 || FaultType == 0x21) {
            FaultStatus = SWFS_EXECUTE;
        } else if ((TrapFrame->Esr & 0x40) != 0) {
            FaultStatus = SWFS_WRITE;
        }

        //
        // Synthesize an error
        //

        ExceptionRecord->ExceptionCode = STATUS_ACCESS_VIOLATION;
        ExceptionRecord->ExceptionAddress = (PVOID) TrapFrame->Pc;
        ExceptionRecord->NumberParameters = 2;
        ExceptionRecord->ExceptionInformation[0] = FaultStatus;
        ExceptionRecord->ExceptionInformation[1] = FaultAddress;
        break;

    //
    // Alignment faults get dispatched with their own code.
    //

    case 0x21:
        ExceptionRecord->ExceptionCode = STATUS_DATATYPE_MISALIGNMENT;
        ExceptionRecord->ExceptionAddress = (PVOID) FaultAddress;
        ExceptionRecord->NumberParameters = 0;
        break;

    }
}

