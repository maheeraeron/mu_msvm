      TITLE   IvtAsm.asm:
;++
;
; Copyright (c) 2013  Microsoft Corporation
;
; Module Name:
;
;   IvtAsm.asm
;
; Abstract:
;
;   This module implements functions to support the Interrupt Vector Table.
;
;--

#include <Base.h>

#ifdef MDE_CPU_IA32
    .686
    .model  flat,C
#endif
    .code

;------------------------------------------------------------------------------
;  Generic IDT Vector Handlers for the Host. They are all the same so they
;  will compress really well.
;
;  By knowing the return address for Vector 00 you can can calculate the
;  vector number by looking at the call CommonInterruptEntry return address.
;  (return address - (AsmIdtVector00 + 5))/8 == IDT index
;
;------------------------------------------------------------------------------

EXTRN CommonInterruptEntry:PROC

ALIGN   8

PUBLIC	AsmIdtVector00

AsmIdtVector00 LABEL BYTE
REPEAT  256
    call    CommonInterruptEntry
    dw      ($ - AsmIdtVector00 - 5) / 8 ; vector number
    nop
ENDM

END

