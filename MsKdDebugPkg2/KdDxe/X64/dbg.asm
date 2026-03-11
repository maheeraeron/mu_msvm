public DebugService2
public KdDxeKdBreakPointWithStatus

.code

; VOID
; DebugService2(
;     IN PVOID Param1,
;     IN PVOID Param2,
;     IN ULONG Service
;     )
;
; Routine Description:
;
;   Call debugger generically.
;
; Arguments:
;
;   Param1 (rcx) - first parameter
;
;   Param2 (rdx) - second parameter
;
;   Service (r8d) - what to do BREAKPOINT_LOAD_SYMBOLS etc.
;
; Return Value:
;
;   None.
;
;--

DebugService2:
    mov     eax, r8d                ; set debug service type
    int     2dh                     ; call debug service
    int     3                       ; required - do not remove
    ret

;++
;
; VOID
; KdDxeKdBreakPointWithStatus(
;     IN ULONG Status
;     )
;
; Routine Description:
;
;   This function executes a breakpoint instruction. Useful for entering
;   the debugger under program control. This breakpoint will always go to
;   the kernel debugger if one is installed, otherwise it will go to the
;   debug subsystem. This function is identical to DbgBreakPoint, except
;   that it takes an argument which the debugger can see.
;
;   Note: The debugger checks the address of the breakpoint instruction
;   against the address RtlpBreakWithStatusInstruction.  If it matches,
;   we have a breakpoint with status. A breakpoint is normally issued
;   with the break_debug_stop macro which generates two instructions.
;   We can't use the macro here because of the "label on the breakpoint"
;   requirement.
;
; Arguments:
;
;   Status (rcx) - Supplies the break point status code.
;
; Return Value:
;
;    None.
;
;--

KdDxeKdBreakPointWithStatus:
  int   3
  ret

END