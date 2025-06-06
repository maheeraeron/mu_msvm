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
;   This function calls the kernel debugger to execute a command string.
;
; Arguments:
;
;   Param1 (x0) - Supplies the first parameter to the KD fault handler
;
;   Param2 (x1) - Supplies the second parameter to the KD fault handler
;
;   Service (x2) - Supplies a pointer to the command string.
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
;   Status (r0) - Supplies the break point status code.
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