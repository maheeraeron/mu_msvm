;++
;
; Copyright (c) Microsoft Corporation
;
; Module Name:
;
;   Snp.asm
;
; Abstract:
;
;   Asm implementations of SNP instructions that will become compiler
;   intrinsics.
;
; Author:
;
;   Jon Lange (jlange) 15-Oct-2019
;
;--

include macamd64.inc

;*++
;
; UINT64
; _sev_pvalidate(
;     _In_ PVOID Address,
;     _In_ UINT32 PageSize,
;     _In_ UINT32 Validate,
;     _Out_ PUINT64 ErrorCode
;     );
;
; Routine Description:
;
;   This routine executes the PVALIDATE instruction.
;
; Arguments:
;
;   Address (rcx) - Supplies the linear address argument (RAX).
;
;   PageSize (edx) - Supplies the page size argument (RCX).
;
;   Validate (r8d) - Supplies the validate argument (RDX).
;
;   ErrorCode (r9) - Supplies a pointer to a variable that will receive the
;                    error code following the instruction (RAX).
;
; Return Value:
;
;   Value of EFLAGS.CF following the instruction.
;
;--*

        LEAF_ENTRY _sev_pvalidate, _TEXT$00

        mov     rax, rcx                ; move arguments to correct registers.
        mov     ecx, edx
        mov     edx, r8d
        xor     r8d, r8d                ; load zero
        db      0f2h                    ; PVALIDATE instruction (F2 0F 01 FF)
        db      00fh
        db      001h
        db      0ffh

        jnc     @f                      ; if NC, operation completed as expected
        inc     r8d                     ; indicate presence of carry flag
@@:
        mov     [r9], rax               ; save error code
        mov     eax, r8d                ; return value of EFLAGS.CF
        ret

        LEAF_END _sev_pvalidate, _TEXT$00

;*++
;
; UINT64
; SpecialGhcbCall(
;     _In_ UINT64 GhcbValue
;     );
;
; Routine Description:
;
;   This routine temporarily updates the GHCB MSR to a specified value and
;   executes VMGEXIT.
;
; Arguments:
;
;   GhcbValue (rcx) - Supplies the GHCB MSR value to use for the call.
;
; Return Value:
;
;   Value of GHCB MSR following the exit.
;
;--*

        NESTED_ENTRY SpecialGhcbCall, _TEXT$00

        rex_push_eflags                 ; save current eflags

        END_PROLOGUE

        cli                             ; disable interrupts to prevent conflicting GHCB MSR use

        mov     r8, rcx                 ; copy input parameter
        mov     ecx, 0C0010130h         ; load GHCB MSR index
        rdmsr                           ; capture current GHCB value
        mov     r9d, eax                ; copy low 32 bits
        mov     r10d, edx               ; copy high 32 bits
        mov     eax, r8d                ; copy low 32 bits of new GHCB value
        mov     rdx, r8                 ; copy full 64-bit value
        shr     rdx, 32                 ; capture high 32 bits of GHCB value
        wrmsr                           ; update GHCB value

        db      0f3h                    ; rep prefix for VMGEXIT
        vmmcall                         ;

        rdmsr                           ; read current GHCB value
        shl     rdx, 32                 ; shift high 32 bits
        or      rax, rdx                ; combine with low 32 bits
        mov     r8, rax                 ; save 64-bit output value
        mov     eax, r9d                ; copy low 32 bits of previous value
        mov     edx, r10d               ; copy high 32 bits of previous value
        wrmsr                           ; restore previous GHCB value

        mov     rax, r8                 ; copy output value to return register

        popfq                           ; restore saved eflags

        BEGIN_EPILOGUE

        ret

        NESTED_END SpecialGhcbCall, _TEXT$00
        
        end
