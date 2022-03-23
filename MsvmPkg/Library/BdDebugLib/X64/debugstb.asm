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
; PVOID
; DebugService2(
;     __in_opt PVOID Param1,
;     __in_opt PVOID Param2,
;     __in ULONG Service
;     )
;
; Routine Description:
;
;    This function calls the kernel debugger to execute a command.
;
; Arguments:
;
;    Param1 (rcx) - Supplies the first parameter to the KD fault handler
;
;    Param2 (rdx) - Supplies the second parameter to the KD fault handler
;
;    Service (r8d) - Supplies the service code of the debugger request.
;
; Return Value:
;
;    None.
;
;--

LEAF_ENTRY DebugService2, _TEXT$00

    ;
    ; If the debugger is present call the debugger to execute the command.
    ; If no debugger is present we should avoid calling the debugger and
    ; not let it trap.
    ;

    mov     eax, r8d                ; set debug service type
    int     2dh                     ; call debug service
    int     3                       ; required - do not remove

NoDebuggerPresent:

    ret                             ; return

LEAF_END DebugService2, _TEXT$00

;++
;
; VOID
; GenerateDumpAndFailFast(
;    __in    UINT64 ErrorCode,
;    __in    UINT64 Param1,
;    __in    UINT64 Param2,
;    __in    UINT64 Param3,
;    __in    UINT64 Param4
;    );

;
; Routine Description:
;
;   Called when a fatal error is detected and the system cannot continue.
;   It places the four parameters in the respective registers prior to the crash.
;   The Hyper-V worker process logs the VP state which includes these registers.
;
;
; Arguments:
;
;   This function assumes x64 calling conventions and the
;   four arguments are expected in rcx, rdx, r8, and r9 respectively.
;
;   ErrorCode     Bugcheck error code
;   Param1        Bugcheck code specific parameter.
;   Param2        Bugcheck code specific parameter.
;   Param3        Bugcheck code specific parameter.
;   Param4        Bugcheck code specific parameter.
;
; Return Value:
;
;    None.
;
;--

LEAF_ENTRY      GenerateDumpAndFailFast, _TEXT$00

                int     12H

LEAF_END        GenerateDumpAndFailFast, _TEXT$00

end

