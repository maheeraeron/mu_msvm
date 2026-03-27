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

    int3                            ; break into debugger
    ret                             ; return

LEAF_END BdBreakPointWithStatus, _TEXT$00

end

