      TITLE   SecEntry.asm
;------------------------------------------------------------------------------
;*
;*   Copyright (c) 2006 - 2009, Intel Corporation. All rights reserved.<BR>
;*   This program and the accompanying materials
;*   are licensed and made available under the terms and conditions of the BSD License
;*   which accompanies this distribution.  The full text of the license may be found at
;*   http://opensource.org/licenses/bsd-license.php
;*
;*   THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
;*   WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
;*
;*    CpuAsm.asm
;*
;*   Abstract:
;*
;------------------------------------------------------------------------------

#include <Base.h>

.code

EXTERN SecCoreStartupWithStack:PROC
EXTERN SecProcessVirtualCommunicationException:PROC

;
; SecCore Entry Point
;
; Processor is in flat protected mode
;
; @param[in]  RAX   Initial value of the EAX register (BIST: Built-in Self Test)
; @param[in]  DI    'BP': boot-strap processor, or 'AP': application processor
; @param[in]  RBP   Pointer to the start of the Boot Firmware Volume
; @param[in]  R8, R9, R10, R11  Hypervisor isolation configuration CPUID leaf
; @param[in]  R12   Pointer to UEFI IGVM config header if required
;
; @return     None  This routine does not return
;
_ModuleEntryPoint PROC PUBLIC

    ;
    ; Load temporary stack top at very low memory.  The C code
    ; can reload to a better address.
    ;
    mov     rsp, BASE_512KB
    nop

    ;
    ; Setup parameters and call SecCoreStartupWithStack
    ;   rcx: BootFirmwareVolumePtr
    ;   rdx: TopOfCurrentStack
    ;   r8:  IsolationConfiguration
    ;   r9:  IgvmConfigHeader
    ;
    mov     rcx, rbp
    mov     rdx, rsp
    sub     rsp, 30h
    mov     20h[rsp], r8d
    mov     24h[rsp], r9d
    mov     28h[rsp], r10d
    mov     2ch[rsp], r11d
    lea     r8, 20h[rsp]
    mov     r9, r12
    call    SecCoreStartupWithStack

    ;
    ; If SecCoreStartupWithStack returns, then startup has failed.  Invoke a
    ; fatal exception.
    ;
    int     3

_ModuleEntryPoint ENDP

;
; #VC exception handler
;

TrapFrame   struct      ; keep in sync with SecP.h
            P1Home      dq ?
            P2Home      dq ?
            P3Home      dq ?
            P4Home      dq ?
            SavedXmm0   oword ?
            SavedXmm1   oword ?
            SavedXmm2   oword ?
            SavedXmm3   oword ?
            SavedXmm4   oword ?
            SavedXmm5   oword ?
            SavedRax    dq ?
            SavedRcx    dq ?
            SavedRdx    dq ?
            SavedRbx    dq ?
            SavedR8     dq ?
            SavedR9     dq ?
            SavedR10    dq ?
            SavedR11    dq ?
            TrErrorCode dq ?
TrapFrame   ends

BEGIN_TRAP_HANDLER macro ErrorCode

ifb <ErrorCode>

            sub     rsp, (sizeof TrapFrame) ; allocate trap frame storage

else

            sub     rsp, TrapFrame.TrErrorCode ; allocate trap frame storage

endif

            mov     TrapFrame.SavedRax[rsp], rax ; save volatile registers
            mov     TrapFrame.SavedRcx[rsp], rcx
            mov     TrapFrame.SavedRdx[rsp], rdx
            mov     TrapFrame.SavedR8[rsp], r8
            mov     TrapFrame.SavedR9[rsp], r9
            mov     TrapFrame.SavedR10[rsp], r10
            mov     TrapFrame.SavedR11[rsp], r11
            movdqa  TrapFrame.SavedXmm0[rsp], xmm0
            movdqa  TrapFrame.SavedXmm1[rsp], xmm1
            movdqa  TrapFrame.SavedXmm2[rsp], xmm2
            movdqa  TrapFrame.SavedXmm3[rsp], xmm3
            movdqa  TrapFrame.SavedXmm4[rsp], xmm4
            movdqa  TrapFrame.SavedXmm5[rsp], xmm5
            mov     TrapFrame.SavedRbx[rsp], rbx ; non-volatile but required by trap handler

            endm

END_TRAP_HANDLER macro

            movdqa  xmm0, TrapFrame.SavedXmm0[rsp] ; restore volatile registers
            movdqa  xmm1, TrapFrame.SavedXmm1[rsp]
            movdqa  xmm2, TrapFrame.SavedXmm2[rsp]
            movdqa  xmm3, TrapFrame.SavedXmm3[rsp]
            movdqa  xmm4, TrapFrame.SavedXmm4[rsp]
            movdqa  xmm5, TrapFrame.SavedXmm5[rsp]
            mov     r11, TrapFrame.SavedR11[rsp]
            mov     r10, TrapFrame.SavedR10[rsp]
            mov     r9, TrapFrame.SavedR9[rsp]
            mov     r8, TrapFrame.SavedR8[rsp]
            mov     rbx, TrapFrame.SavedRbx[rsp]
            mov     rdx, TrapFrame.SavedRdx[rsp]
            mov     rcx, TrapFrame.SavedRcx[rsp]
            mov     rax, TrapFrame.SavedRax[rsp]
            add     rsp, (sizeof TrapFrame) ; deallocate trap frame
            iretq

            endm

SecVirtualCommunicationExceptionHandler PROC PUBLIC

            BEGIN_TRAP_HANDLER <ErrorCode>

            mov     rcx, rsp                ; load address of trap frame
            call    SecProcessVirtualCommunicationException ; attempt to handle
            test    al, al                  ; check return value
            jnz     @f                      ; if nz, successful
            int     3                       ; force unrecoverable exception
@@:

            END_TRAP_HANDLER

SecVirtualCommunicationExceptionHandler ENDP

;
; SecVmgexit
;
; Executes the VMGEXIT instruction
;

SecVmgexit PROC PUBLIC

            db      0f3h                ; VMGEXIT prefix
            vmmcall
            ret

SecVmgexit ENDP

;
; MulDiv64
;
; Multiply two 64-bit numbers and divide by a third.
;
; @param[in] RCX  Value
; @param[in] RDX  Multiplier
; @param[in] R8   Divisor
;
; @return         Result
;

MulDiv64 PROC PUBLIC

            mov     rax, rdx            ; move multiplier to correct register
            mul     rcx                 ; multiply into RDX:RAX
            div     r8                  ; divide RDX:RAX by R8
            ret                         ; result is in rax

MulDiv64 ENDP

END
