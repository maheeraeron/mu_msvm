;++
;
; Copyright (c) Microsoft Corporation
;
; Module Name:
;
;   HvlSnp.asm
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
; VOID
; _sev_vmgexit(
;     VOID
;     );
;
; Routine Description:
;
;   This routine performs an VMGEXIT.
;
; Arguments:
;
;   None.
;
; Return Value:
;
;   None.
;
;--*

        LEAF_ENTRY _sev_vmgexit, _TEXT$00

        db      0f2h                    ; VMGEXIT prefix
        vmmcall
        ret

        LEAF_END _sev_vmgexit, _TEXT$00

        end
