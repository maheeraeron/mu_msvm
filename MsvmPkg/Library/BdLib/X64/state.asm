        title  "Processor State Save Restore"
;++
;
; Copyright (c) 2005  Microsoft Corporation
;
; Module Name:
;
;   state.asm
;
; Abstract:
;
;   This module implements routines to save and restore processor control
;   state.
;
; Author:
;
;   Jamie Schwartz (jamschw) 15-Jun-2004 - Stole the required routines from
;       ntos\ke\amd64\procstat.asm.
;
; Environment:
;
;   Boot
;
;--

include ksamd64.inc



        subttl  "Save Processor Control State"
;++
;
; KiSaveProcessorControlState (
;     PKPROCESSOR_STATE ProcessorState
;     );
;
; Routine Description:
;
;   This routine saves the control state of the current processor.
;
; Arguments:
;
;   ProcessorState (rcx) - Supplies a pointer to a processor state structure.
;
; Return Value:
;
;    None.
;
;--

        LEAF_ENTRY KiSaveProcessorControlState, _TEXT$00

        mov     rax, cr0                ; save processor control state
        mov     PsCr0[rcx], rax         ;
        mov     rax, cr2                ;
        mov     PsCr2[rcx], rax         ;
        mov     rax, cr3                ;
        mov     PsCr3[rcx], rax         ;
        mov     rax, cr4                ;
        mov     PsCr4[rcx], rax         ;
        mov     rax, cr8                ;
        mov     PsCr8[rcx], rax         ;

        sgdt    fword ptr PsGdtr[rcx]   ; save GDTR
        sidt    fword ptr PsIdtr[rcx]   ; save IDTR

        str     word ptr PsTr[rcx]      ; save TR
        sldt    word ptr PsLdtr[rcx]    ; save LDTR

;
; Save debug control state.
;

        mov     rax, dr0                ; save debug registers
        mov     rdx, dr1                ;
        mov     PsKernelDr0[rcx], rax   ;
        mov     PsKernelDr1[rcx], rdx   ;
        mov     rax, dr2                ;
        mov     rdx, dr3                ;
        mov     PsKernelDr2[rcx], rax   ;
        mov     PsKernelDr3[rcx], rdx   ;
        mov     rax, dr6                ;
        mov     rdx, dr7                ;
        mov     PsKernelDr6[rcx], rax   ;
        mov     PsKernelDr7[rcx], rdx   ;
        xor     eax, eax                ;
        mov     dr7, rax                ;

        ret                             ; return

        LEAF_END KiSaveProcessorControlState, _TEXT$00


        subttl  "Restore Processor Control State"
;++
;
; KiRestoreProcessorControlState (
;     VOID
;     );
;
; Routine Description:
;
;   This routine restores the control state of the current processor.
;
; Arguments:
;
;   ProcessorState (rcx) - Supplies a pointer to a processor state structure.
;
; Return Value:
;
;   None.
;
;--

        LEAF_ENTRY KiRestoreProcessorControlState, _TEXT$00

        mov     rax, PsCr0[rcx]         ; restore processor control registers
        mov     cr0, rax                ;
        mov     rax, PsCr3[rcx]         ;
        mov     cr3, rax                ;
        mov     rax, PsCr4[rcx]         ;
        mov     cr4, rax                ;
        mov     rax, PsCr8[rcx]         ;
        mov     cr8, rax                ;

        lgdt    fword ptr PsGdtr[rcx]   ; restore GDTR
        lidt    fword ptr PsIdtr[rcx]   ; restore IDTR

        xor     eax, eax                ; load a NULL selector into the ldt
        lldt    ax                      ;

;
; Restore debug control state.
;

        xor     edx, edx                ; restore debug registers
        mov     dr7, rdx                ;
        mov     rax, PsKernelDr0[rcx]   ;
        mov     rdx, PsKernelDr1[rcx]   ;
        mov     dr0, rax                ;
        mov     dr1, rdx                ;
        mov     rax, PsKernelDr2[rcx]   ;
        mov     rdx, PsKernelDr3[rcx]   ;
        mov     dr2, rax                ;
        mov     dr3, rdx                ;
        mov     rdx, PsKernelDr7[rcx]   ;
        xor     eax, eax                ;
        mov     dr6, rax                ;
        mov     dr7, rdx                ;

        ret                             ; return

        LEAF_END KiRestoreProcessorControlState, _TEXT$00

        subttl  "Capture Context"
;++
;
; VOID
; EfiCaptureContext (
;     __out PCONTEXT ContextRecord
;     )
;
; Routine Description:
;
;   This function captures the context of the caller in the specified
;   context record.
;
;   N.B. The stored value of registers rcx and rsp will be a side effect of
;        having made this call. All other registers will be stored as they
;        were when the call to this function was made.
;
; Arguments:
;
;    ContextRecord (rcx) - Supplies a pointer to a context record.
;
; Return Value:
;
;    None.
;
;--

CcFrame struct
        EFlags  dd ?                    ; saved processor flags
        Fill    dd ?                    ; fill
CcFrame ends


        NESTED_ENTRY EfiCaptureContext, _TEXT$00

        rex_push_eflags                 ; save processor flags

        END_PROLOGUE

        mov     CxRax[rcx], rax         ; save volatile integer registers
        mov     CxRcx[rcx], rcx         ;
        mov     CxRdx[rcx], rdx         ;
        mov     CxR8[rcx], r8           ;
        mov     CxR9[rcx], r9           ;
        mov     CxR10[rcx], r10         ;
        mov     CxR11[rcx], r11         ;

        movaps  CxXmm0[rcx], xmm0       ; save volatile sse registers
        movaps  CxXmm1[rcx], xmm1       ;
        movaps  CxXmm2[rcx], xmm2       ;
        movaps  CxXmm3[rcx], xmm3       ;
        movaps  CxXmm4[rcx], xmm4       ;
        movaps  CxXmm5[rcx], xmm5       ;

        mov     CxSegCs[rcx], cs        ; save segment registers
        mov     CxSegDs[rcx], ds        ;
        mov     CxSegEs[rcx], es        ;
        mov     CxSegSs[rcx], ss        ;
        mov     CxSegFs[rcx], fs        ;
        mov     CxSegGs[rcx], gs        ;

        mov     CxRbx[rcx], rbx         ; save non-volatile integer registers
        mov     CxRbp[rcx], rbp         ;
        mov     CxRsi[rcx], rsi         ;
        mov     CxRdi[rcx], rdi         ;
        mov     CxR12[rcx], r12         ;
        mov     CxR13[rcx], r13         ;
        mov     CxR14[rcx], r14         ;
        mov     CxR15[rcx], r15         ;

        fnstcw  CxFltSave+LfControlWord[rcx] ; save x87 ControlWord
        mov     dword ptr CxFltSave+LfStatusWord[rcx], 0 ; reset x87 StatusWord and TagWord

        movaps  CxXmm6[rcx], xmm6       ; save non-volatile sse registers
        movaps  CxXmm7[rcx], xmm7       ;
        movaps  CxXmm8[rcx], xmm8       ;
        movaps  CxXmm9[rcx], xmm9       ;
        movaps  CxXmm10[rcx], xmm10     ;
        movaps  CxXmm11[rcx], xmm11     ;
        movaps  CxXmm12[rcx], xmm12     ;
        movaps  CxXmm13[rcx], xmm13     ;
        movaps  CxXmm14[rcx], xmm14     ;
        movaps  CxXmm15[rcx], xmm15     ;
        stmxcsr CxFltSave+LfMxCsr[rcx]  ; save sse control state

        stmxcsr CxMxCsr[rcx]            ; save sse control state

        lea     rax, (sizeof CcFrame) + 8[rsp] ; get previous stack address
        mov     CxRsp[rcx], rax         ;

        mov     rax, (sizeof CcFrame)[rsp] ; set return address
        mov     CxRip[rcx], rax         ;

        mov     eax, Ccframe.EFlags[rsp] ; set processor flags
        mov     CxEFlags[rcx], eax      ;

        mov     dword ptr CxContextFlags[rcx], CONTEXT_FULL or CONTEXT_SEGMENTS ; set context flags

        add     rsp, sizeof CcFrame     ; deallocate stack frame

        BEGIN_EPILOGUE

        ret                             ; return

        NESTED_END EfiCaptureContext, _TEXT$00


    end