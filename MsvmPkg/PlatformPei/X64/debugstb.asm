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
;    This file contains architecture specific functions for crash handling.
;
; Author:
;
;    Arti Gupta (argu) 11-Feb-2021
;
;--

include macamd64.inc

;++
;
; VOID
; TripleFault(
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

LEAF_ENTRY      TripleFault, _TEXT$00

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

LEAF_END        TripleFault, _TEXT$00

end

