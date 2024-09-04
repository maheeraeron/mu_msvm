;++
;
; Copyright (c) Microsoft Corporation
;
; Module Name:
;
;    debugstb.asm
;
; Abstract:
;
;    This file contains architecture specific functions for debugging the
;    hypervisor.
;
; Author:
;
;    Joy Ganguly (sganguly) 23-Jan-2004
;
;--

include macamd64.inc

;++
;
; VOID
; BdBreakPointWithStatus(
;     __in ULONG Status
;     )
;
; Routine Description:
;
;    This function executes a breakpoint instruction. Useful for entering
;    the debugger under program control. This breakpoint will always go to
;    the kernel debugger if one is installed, otherwise it will go to the
;    debug subsystem. This function is identical to DbgBreakPoint, except
;    that it takes an argument which the debugger can see.
;
;    Note: The debugger checks the address of the breakpoint instruction
;    against the address KdpBreakWithStatusInstruction.  If it matches,
;    we have a breakpoint with status. A breakpoint is normally issued
;    with the break_debug_stop macro which generates two instructions.
;    We can't use the macro here because of the "label on the breakpoint"
;    requirement.
;
; Arguments:
;
;    Status (ecx) - Supplies the break point status code.
;
; Return Value:
;
;    None.
;
;--

LEAF_ENTRY BdBreakPointWithStatus, _TEXT$00

    int     3                       ; break into debugger
    ret                             ; return

LEAF_END BdBreakPointWithStatus, _TEXT$00

;++
;
; VOID
; BdTripleFault(
;    __in    UINT64 Rax,
;    __in    UINT64 Rbx,
;    __in    UINT64 Rcx,
;    __in    UINT64 Rdx
;    );

;
; Routine Description:
;
;   This function causes a triple fault.
;   It places the four parameters in the respective registers prior to the triple fault.
;   The Hyper-V worker process logs the VP state which includes these registers.
;
;
; Arguments:
;
;   This function assumes x64 calling conventions and the
;   four arguments are expected in rcx, rdx, r8, and r9 respectively.
;
;   Rax  - Value to place in rax register.
;   Rbx  - Value to place in rbx register.
;   Rcx  - Value to place in rcx register.
;   Rdx  - Value to place in rdx register.
;
; Return Value:
;
;    None.
;
;--

LEAF_ENTRY      BdTripleFault, _TEXT$00

                mov     rax, rcx
                mov     rbx, rdx
                mov     rcx, r8
                mov     rdx, r9

                push    0
                push    0
                lidt    fword ptr [rsp]   ; SET EMPTY IDT
;
; Generate #UD using UD2 instruction
;
EternalUD:
                db      0FH, 0Bh        ; #UD -> #DF -> TRIPLE FAULT

                jmp     EternalUD

LEAF_END        BdTripleFault, _TEXT$00

end

