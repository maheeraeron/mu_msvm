     title  "Trap Processing"
;++
;
; Copyright (c) 2005  Microsoft Corporation
;
; Module Name:
;
;   trap.asm
;
; Abstract:
;
;   This module implements the code necessary to field and process AMD64
;   trap conditions.
;
; Author:
;
;   Jamie Schwartz (jamschw) 15-Jun-2004 - Pruned ntos\ke\trap.asm to the
;       trap handlers required for the Boot Debugger.
;
; Environment:
;
;   Boot
;
;--

        extern  EfiDispatchException
        extern  EfiBugCheck

%define blank     0
%define Blank     0
%define ErrorCode 1
%define Virtual   2
%define NoPop     3
%define Volatile  4
%define Rbp       5

;
; TODO: Define special macros to align trap entry points on cache line boundaries.
;
; N.B. This will only work if all functions in this module are declared with
;      these macros.
;

%macro BL_TRAP_ENTRY 1

        align   16
        global  %1
        %1:

%endmacro


%macro BL_TRAP_END 1
%endmacro

;
; Define trap frame generation macro.
;
;   This macro generates a trap frame.
;
; Arguments:
;
;   ErrorCode - If non-blank, then an error code is on the stack.
;
; Return value:
;
;   If ErrorCode is non-blank, then the value of the error code is returned
;   in eax.
;
; Note: Trap and interrupt frames are exempt from the "first instruction must
;       be two bytes" rule.
;

%macro BL_GENERATE_TRAP_FRAME_INTERNAL 1

; %1 is ErrorCode, true or false or a number (Virtual)

%if %1 != Blank

        push_frame  blank               ; mark machine frame without error code
        alloc_stack 8                   ; allocate dummy error code

%else

        push_frame code                 ; mark machine frame with error code

%endif

;
; After pushing rbp onto the stack, The high low 56 bytes of the trap
; frame have been saved (rbp, ErrorCode, rip, cs, eflags, etc).
; Adjust the stack pointer by the remaining portion of the trap
; frame.
;

        push_reg rbp                    ; save nonvolatile register
        alloc_stack (KTRAP_FRAME_LENGTH - (7 * 8)) ; allocate fixed frame

;
; rbp = rsp + 80h .  Tr offsets are all based on this stack address.
;

        set_frame rbp, 128              ; set frame pointer

        END_PROLOGUE

        mov     byte ptr TrExceptionActive[rbp], 1 ; set exception active

        BL_SAVE_TRAP_STATE <>           ; save trap state

%if %1 != Blank

        mov     eax, TrErrorCode[rbp]   ; return error code

%if %1 == Virtual

        mov     rcx, cr2                ; return virtual address

%endif

%endif

%endmacro

%macro BL_GENERATE_TRAP_FRAME 0
    BL_GENERATE_TRAP_FRAME_INTERNAL Blank
%endmacro

%macro BL_GENERATE_TRAP_FRAME_Virtual 0
    BL_GENERATE_TRAP_FRAME_INTERNAL Virtual
%endmacro

%macro BL_GENERATE_TRAP_FRAME_ErrorCode 0
    BL_GENERATE_TRAP_FRAME_INTERNAL ErrorCode
%endmacro

;
; Define save trap state macro.
;
;   This macro saves the volatile state, and if necessary, saves the user
;   debug state and loads the kernel debug state.
;
; Arguments:
;
;   None.
;
; Implicit arguments:
;
;   rbp - Supplies the address of the trap frame.
;

%macro BL_SAVE_TRAP_STATE 1

        mov     TrRax[rbp], rax         ; save volatile integer registers
        mov     TrRcx[rbp], rcx         ;
        mov     TrRdx[rbp], rdx         ;
        mov     TrR8[rbp], r8           ;
        mov     TrR9[rbp], r9           ;
        mov     TrR10[rbp], r10         ;
        mov     TrR11[rbp], r11         ;
        cld                             ; clear direction flag

%endmacro


;
; Define restore trap state macro.
;
;   This macro restores the volatile state, and if necessary, restores the
;   user debug state, deallocats the trap frame, and exits the trap.
;
;   N.B. This macro must preserve eax in case it is not reloaded from the
;        trap frame.
;
; Arguments:
;
;   State - Determines what state is restored and what tests are made. Valid
;       values are:
;
;           Service - restore state for a service executed from user mode.
;           Kernel - restore state for a service executed from kernel mode.
;           Volatile - restore state for a trap or interrupt.
;
;   Disable - If blank, then disable interrupts.
;
; Implicit arguments:
;
;   rbp - Supplies the address of the trap frame.
;

%macro BL_RESTORE_TRAP_STATE 1

        mov     r11, TrR11[rbp]         ; restore volatile integer state
        mov     r10, TrR10[rbp]         ;
        mov     r9, TrR9[rbp]           ;
        mov     r8, TrR8[rbp]           ;
        mov     rdx, TrRdx[rbp]         ;
        mov     rcx, TrRcx[rbp]         ;
        mov     rax, TrRax[rbp]         ;
        mov     rsp, rbp                ; trim stack to frame offset
        mov     rbp, TrRbp[rbp]         ; restore RBP

;
; Deallocate stack.
;

        add     rsp, (KTRAP_FRAME_LENGTH - (5 * 8) - 128)
        iretq                           ;

%endmacro

;
; Define generate exception frame macro.
;
;   This macro allocates an exception frame and saves the nonvolatile state.
;
; Arguments:
;
;   Flag - If blank, then nonvolatile floating and integer registers are
;       saved. If nonblank and identical to "Rbp", then rbp is saved in
;       addition to the nonvolatile floating and integer registers. If
;       nonblank and identical to "NoFp", then only the nonvolatile integer
;       registers are saved. If nonblank and identical to "NoPop", then
;       allocate an exception record in addition to an exception frame.
;
; Implicit arguments:
;
;   The top of the stack is assumed to contain a return address.
;

%macro BL_GENERATE_EXCEPTION_FRAME 1 ; Flag

%if %1 == NoPop

        alloc_stack (EXCEPTION_RECORD_LENGTH + KEXCEPTION_FRAME_LENGTH - (1 * 8)) ; allocate frame

%else

        alloc_stack (KEXCEPTION_FRAME_LENGTH - (1 * 8)) ; allocate frame

%endif

        lea     rax, 100h[rsp]          ; set frame display pointer

;
; Save nonvolatile integer registers
;

%if %1 == Rbp

        mov     (ExRbp - 100h)[rax], rbp ; save nonvolatile integer register
        .savereg rbp, ExRbp             ;
        set_frame rbp, 0                ; set frame pointer

%endif

        mov     (ExRbx - 100h)[rax], rbx ;
        .savereg rbx, ExRbx             ;

        mov     (ExRdi - 100h)[rax], rdi ;
        .savereg rdi, ExRdi             ;

        mov     (ExRsi - 100h)[rax], rsi ;
        .savereg rsi, ExRsi             ;

        mov     (ExR12 - 100h)[rax], r12 ;
        .savereg r12, ExR12             ;

        mov     (ExR13 - 100h)[rax], r13 ;
        .savereg r13, ExR13             ;

        mov     (ExR14 - 100h)[rax], r14 ;
        .savereg r14, ExR14             ;

        mov     (ExR15 - 100h)[rax], r15 ;
        .savereg r15, ExR15             ;

        END_PROLOGUE

%endmacro

;
; Define restore exception state macro.
;
;   This macro restores the nonvolatile state.
;
; Arguments:
;
;   Flag - If blank, then nonvolatile floating and integer registers are
;       restored. If nonblank and identical to "Rbp", then rbp is restored
;       in addition to the nonvolatile floating and integer registers. If
;       nonblank and identical to "NoFp", then only the nonvolatile integer
;       registers are restored.
;
; Implicit arguments:
;
;   rsp - Supplies the address of the exception frame.
;

%macro BL_RESTORE_EXCEPTION_STATE 1 ; Flag

        lea     rcx, 100h[rsp]          ; set frame display pointer
        mov     rbx, (ExRbx - 100h)[rcx] ; restore nonvolatile integer registers
        mov     rdi, (ExRdi - 100h)[rcx] ;
        mov     rsi, (ExRsi - 100h)[rcx] ;
        mov     r12, (ExR12 - 100h)[rcx] ;
        mov     r13, (ExR13 - 100h)[rcx] ;
        mov     r14, (ExR14 - 100h)[rcx] ;
        mov     r15, (ExR15 - 100h)[rcx] ;

%if %1 != NoPop

%if %1 == Rbp

        mov     rbp, (ExRbp - 100h)[rcx]  ; restore nonvolatile integer register

%endif

        add     rsp, KEXCEPTION_FRAME_LENGTH - (1 * 8) ; deallocate frame

%endif

%endmacro


        subttl "Unhandled Exception"
;++
; Routine Description:
;
;   This routine is entered on an unexpected trap. We will try to break in
;   to the debugger if its present, otherwise we will restart the system.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--
        BL_TRAP_ENTRY BdUnhandledException

        BL_GENERATE_TRAP_FRAME              ; generate trap frame

        mov     ecx, STATUS_NONCONTINUABLE_EXCEPTION    ; set exception code.
        mov     r8, TrRip[rbp]                          ; set exception address
        xor     edx, edx                                ; set number of parameters
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdUnhandledException


        subttl "Divide Error Fault"
;++
; Routine Description:
;
;   This routine is entered as the result of an attempted division by zero
;   or the result of an attempted division does not fit in the destination
;   operand (i.e., the largest negative number divided by minus one).
;
;   N.B. The two possible conditions that can cause this exception are not
;        separated and the exception is reported as a divide by zero.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        BL_TRAP_ENTRY BdDivideError

        BL_GENERATE_TRAP_FRAME              ; generate trap frame

        mov     ecx, STATUS_INTEGER_DIVIDE_BY_ZERO  ; set exception code.
        mov     r8, TrRip[rbp]                      ; set exception address
        xor     edx, edx                            ; set number of parameters
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdDivideError


        subttl  "Debug Trap Or Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a debug trap or fault. The
;   following conditions cause entry to this routine:
;
;   1. Instruction fetch breakpoint fault.
;   2. Data read or write breakpoint trap.
;   3. I/O read or write breakpoint trap.
;   4. General detect condition fault (in-circuit emulator).
;   5. Single step trap (TF set).
;   6. Task switch trap (not possible on this system).
;   7. Execution of an int 1 instruction.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        BL_TRAP_ENTRY BdDebugTrapOrFault

        BL_GENERATE_TRAP_FRAME              ; generate trap frame

        and     dword ptr TrEflags[rbp], ~ EFLAGS_TF_MASK ; clear TF Flag.

        mov     ecx, STATUS_SINGLE_STEP     ; set exception code.
        mov     r8, TrRip[rbp]              ; set exception address
        xor     edx, edx                    ; set number of parameters
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdDebugTrapOrFault


        subttl "Nonmaskable Interrupt"
;++
;
; Routine Description:
;
;   This routine is entered as a result of a nonmaskable interrupt. We just
;   ignore them here.
;
; Arguments:
;
;   The standard machine frame is pushed by hardware on the kernel stack.
;   There is no error code for this interrupt.
;
; Disposition:
;
;   The standard trap frame is pushed on the stack and the IPI handler is
;   called with the trap frame as the first argument.
;--

        BL_TRAP_ENTRY BdNmiInterrupt

        BL_GENERATE_TRAP_FRAME              ; generate trap frame

        xor     r10,r10
        mov     TrP5[rbp], r10              ; clear parameter 5
        xor     r9, r9                      ; clear other bugcheck parameters
        xor     r8, r8                      ;
        xor     edx, edx                    ;
        mov     ecx, NMI_HARDWARE_FAILURE   ; Bugcheck code
        call    EfiBugCheck

        BL_TRAP_END BdNmiInterrupt


        subttl  "Breakpoint Trap"
;++
;
; Routine Description:
;
;   This routine is entered as the result of the execution of an int 3
;   instruction.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        BL_TRAP_ENTRY BdBreakpointTrap

        BL_GENERATE_TRAP_FRAME          ; generate trap frame

        dec     qword ptr TrRip[rbp]    ; Adjust RIP to reflect address of
                                        ; int 03 instruction.

        mov     ecx, STATUS_BREAKPOINT  ; set exception code
        mov     edx, 1                  ; set number of parameters
        mov     r8, TrRip[rbp]          ; set exception address
        mov     r9d, BREAKPOINT_BREAK   ; set parameter 1 value
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdBreakpointTrap


        subttl "Overflow Trap"
;++
; Routine Description:
;
;   This routine is entered as the result of the execution of an into
;   instruction when the OF flag is set.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        BL_TRAP_ENTRY BdOverflowTrap

        BL_GENERATE_TRAP_FRAME              ; generate trap frame

        mov     ecx, STATUS_INTEGER_OVERFLOW    ; set exception code.
        mov     r8, TrRip[rbp]                  ; set exception address
        xor     edx, edx                        ; set number of parameters
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdOverflowTrap


        subttl "Bound Fault"
;++
; Routine Description:
;
;   This routine is entered as the result of the execution of a bound
;   instruction and when the bound range is exceeded.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        BL_TRAP_ENTRY BdBoundFault

        BL_GENERATE_TRAP_FRAME                      ; generate a trap frame, align stack.

        mov     ecx, STATUS_ARRAY_BOUNDS_EXCEEDED   ; set exception code.
        mov     r8, TrRip[rbp]                      ; set exception address
        xor     edx, edx                            ; set number of parameters
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdBoundFault


        subttl "Invalid Opcode Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of the execution of an invalid
;   instruction.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        BL_TRAP_ENTRY BdInvalidOpcodeFault

        BL_GENERATE_TRAP_FRAME              ; generate trap frame

        mov     r10, rcx                        ; set parameter 2 to fault address.
        mov     ecx, STATUS_ILLEGAL_INSTRUCTION ; set exception code.
        mov     r8, TrRip[rbp]                  ; set exception address
        mov     edx, 2                          ; set number of parameters
        mov     r9d, TrErrorCode[rbp]           ; set parameter 1 to error code
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdInvalidOpcodeFault


        subttl  "NPX Not Available Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of the numeric coprocessor not
;   being available for one of the following conditions:
;
;   1. A floating point instruction was executed and EM is set in CR0 -
;       this condition should never happen since EM will never be set.
;
;   2. A floating point instruction was executed and the TS flag is set
;       in CR0 - this condition should never happen since TS will never
;       be set.
;
;   3. A WAIT of FWAIT instruction was executed and the MP and TS flags
;       are set in CR0 - this condition should never occur since neither
;       TS nor MP will ever be set.
;
;   N.B. The NPX state should always be available.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack and bug check
;   is called.
;
;--

        BL_TRAP_ENTRY BdNpxNotAvailableFault

        BL_GENERATE_TRAP_FRAME              ; generate trap frame

        mov     r10, TrRip[rbp]             ; set parameter 5 to exception address
        mov     ExP5[rsp], r10              ;
        mov     r9, cr4                     ; set parameter 4 to control register 4
        mov     r8, cr0                     ; set parameter 3 to control register 0
        mov     edx, EXCEPTION_NPX_NOT_AVAILABLE ; set unexpected trap number
        mov     ecx, UNEXPECTED_KERNEL_MODE_TRAP ; Bugcheck code
        call    EfiBugCheck

        BL_TRAP_END BdNpxNotAvailableFault


        subttl  "Double Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of the generation of a second
;   exception while another exception is being generated. A switch to the
;   panic stack occurs before the exception frame is pushed on the stack.
;
;   N.B. This routine executes on the panic stack.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the new stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack and bug check
;   is called.
;
;--

        BL_TRAP_ENTRY BdDoubleFault

        BL_GENERATE_TRAP_FRAME              ; generate trap frame

        ;
        ; There is a strong chance that double fault occured due to page fault 
        ; in stack area. In this case CR2 will point to the faulting stack address.
        ; Save the faulting address in the trap frame.
        ;

        mov     rax, cr2                            ; set rax to faulting address
        mov     TrFaultAddress[rbp], rax            ; save the faulting address
        mov     ecx, STATUS_ACCESS_VIOLATION ; set exception code.
        mov     r8, TrRip[rbp]             ; set exception address
        mov     edx, 2                     ; set number of parameters
        mov     r9d, TrErrorCode[rbp]      ; set parameter 1 to error code
        and     r9d, 0ffffh                ;
        xor     r10, r10                   ; set parameter 2 value
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdDoubleFault


        subttl  "Invalid TSS Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a hardware or software failure
;   since there is no task switching in 64-bit mode and 32-bit code does not
;   have any task state segments.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the new stack.
;   The segment selector index for the segment descriptor that caused the
;   violation is pushed as the error code.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack and bug check
;   is called.
;
;--

        BL_TRAP_ENTRY BdInvalidTss

        BL_GENERATE_TRAP_FRAME_ErrorCode ; generate trap frame

        mov     r10, TrRip[rbp]         ; set parameter 5 to exception address
        mov     TrP5[rbp], r10
        mov     r9d, TrErrorCode[rbp]   ; set parameter 4 to selector index
        mov     r8, cr0                 ; set parameter 3 to control register 0
        mov     edx, EXCEPTION_INVALID_TSS ; set unexpected trap number
        mov     ecx, UNEXPECTED_KERNEL_MODE_TRAP ; set bugcheck code
        call    EfiBugCheck

        BL_TRAP_END BdInvalidTss



        subttl  "General Protection Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a general protection violation.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   The segment selector index for the segment descriptor that caused the
;   exception, the IDT vector number for the descriptor that caused the
;   exception, or zero is pushed as the error code.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        BL_TRAP_ENTRY BdGeneralProtectionFault

        BL_GENERATE_TRAP_FRAME_ErrorCode   ; generate trap frame

        mov     ecx, STATUS_ACCESS_VIOLATION ; set exception code.
        mov     r8, TrRip[rbp]             ; set exception address
        mov     edx, 2                     ; set number of parameters
        mov     r9d, TrErrorCode[rbp]      ; set parameter 1 to error code
        and     r9d, 0ffffh                ;
        xor     r10, r10                   ; set parameter 2 value
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdGeneralProtectionFault


        subttl  "Page Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a page fault which can occur
;   because of the following reasons:
;
;   1. The referenced page is not present.
;
;   2. The referenced page does not allow the requested access.
;
; Arguments:
;
;   A standard exception frame is pushed by hardware on the kernel stack.
;   A special format error code is pushed which specifies the cause of the
;   page fault as not present, read/write access denied, from user/kernel
;   mode, and attempting to set reserved bits.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack and memory
;   management is called to resolve the page fault. If memory management
;   successfully resolves the page fault, then working set information is
;   recorded, owed breakpoints are inserted, and execution is continued.
;   If memory management cannot resolve the page fault and the fault
;   address is the pop SLIST code, then the execution of the pop SLIST
;   code is continued at the resumption address. Otherwise, if the page
;   fault occurred at an IRQL greater than APC_LEVEL, then the system is
;   shut down via a call to bug check. Otherwise, an appropriate exception
;   is raised.
;
;--

        BL_TRAP_ENTRY BdPageFault

        BL_GENERATE_TRAP_FRAME_Virtual  ; generate trap frame

;
; No backing store for the page table in the boot environment.  Just
; raise an AV.
;

;
; The registers eax and rcx are loaded with the error code and virtual
; address of the fault respectively when the trap frame was generated.
;

        shr     eax, 1                  ; isolate load / store and i/d
        and     eax, 09h                ; indicators.
        mov     TrFaultIndicator[rbp], al ; save load/store indicator.
        mov     TrFaultAddress[rbp], rcx ; save fault address
        mov     r10, rcx                ; set parameter 2 to fault address.
        mov     ecx, STATUS_ACCESS_VIOLATION ; set exception code.
        mov     r8, TrRip[rbp]
        mov     edx, 2                  ; set number of parameters
        mov     r9d, TrErrorCode[rbp]   ; set parameter 1 to error code
        and     r9d, 2
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdPageFault


        subttl "Legacy Floating Point Trap"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a legacy floating point fault.
;   Floating point is not currently used in the hypervisor so this is not
;   fleshed out much beyond distinguishing the code
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack. If the previous
;   mode is user, then reason for the exception is determine, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code. Otherwise, bug check is called.
;
;--

        BL_TRAP_ENTRY BdFloatingPointFault

        BL_GENERATE_TRAP_FRAME              ; generate trap frame

        mov     ecx, STATUS_FLOAT_INVALID_OPERATION ; set exception code.
        mov     r8, TrRip[rbp]                      ; set exception address
        xor     edx, edx                            ; set number of parameters
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdFloatingPointFault


        subttl  "Alignment Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of an attempted access to unaligned
;   data.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   An error error code of zero is pushed on the stack.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--                  
        BL_TRAP_ENTRY BdAlignmentFault

        BL_GENERATE_TRAP_FRAME              ; generate trap frame

        mov     ecx, STATUS_DATATYPE_MISALIGNMENT   ; set exception code.
        mov     r8, TrRip[rbp]                      ; set exception address
        xor     edx, edx                            ; set number of parameters
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdAlignmentFault


        subttl "Machine Check Abort"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a machine check abort (or
;   execution of an int18 instruction)
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this abort.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   arguments are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        BL_TRAP_ENTRY BdMachineCheckAbort

        BL_GENERATE_TRAP_FRAME              ; generate trap frame

        test    ecx, ecx                        ; check if output argument present
        jnz     nonNullValue                    ; preserve the registers if present
        xor     r10,r10                         ; clear bugcheck parameters
        mov     TrP5[rbp],r10                   ; 
        xor     r9, r9                          ;
        xor     r8, r8                          ;
        xor     edx, edx                        ;
        mov     ecx, MACHINE_CHECK_EXCEPTION    ; Bugcheck code

nonNullValue:
        call    EfiBugCheck

        BL_TRAP_END BdMachineCheckAbort


        subttl "Xmm Exception"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a XMM floating point fault.
;   Note: this is not fleshed out much since floating point is uused in the
;   the hypervisor
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, mode is user,
;   then reason for the exception is determine, the exception parameters are
;   loaded into registers, and the exception is dispatched via common code.
;   If no reason can be determined for the exception, then bug check is called.
;
;--
        BL_TRAP_ENTRY BdXmmException

        BL_GENERATE_TRAP_FRAME              ; generate trap frame

        mov     ecx, STATUS_FLOAT_INVALID_OPERATION ; set exception code.
        mov     r8, TrRip[rbp]                      ; set exception address
        xor     edx, edx                            ; set number of parameters
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdXmmException



        subttl  "Fast Fail Trap"
;++
;
; Routine Description:
;
;   This routine is entered as the result of the execution of an int 29
;   instruction.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
;   FastFailCode (ecx) - Supplies the fast failure description code from the
;                        original requestor.  Legal values are drawn from the
;                        RTL_FAIL_FAST_* family of constants.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   arguments are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        BL_TRAP_ENTRY BdFastFailTrap

        BL_GENERATE_TRAP_FRAME          ; generate trap frame

        mov     r9, TrRcx[rbp]          ; set fast fail code
        mov     ecx, STATUS_STACK_BUFFER_OVERRUN  ; set exception code
        mov     r8, TrRip[rbp]          ; set exception address
        dec     r8                      ;
        dec     r8                      ;
        mov     edx, 1                  ; set number of parameters
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdFastFailTrap

        subttl  "Debug Assertion Trap"
;++
;
; Routine Description:
;
;   This routine is entered as the result of the execution of an int 2c
;   instruction.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   arguments are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        BL_TRAP_ENTRY BdAssertionFailureTrap

        BL_GENERATE_TRAP_FRAME          ; generate trap frame

        mov     ecx, STATUS_ASSERTION_FAILURE  ; set exception code
        mov     r8, TrRip[rbp]          ; set exception address
        dec     r8                      ;
        dec     r8                      ;
        xor     edx, edx                ; set number of parameters
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdAssertionFailureTrap


        subttl  "Debug Service Trap"
;++
;
; Routine Description:
;
;   This routine is entered as the result of the execution of an int 2d
;   instruction.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   arguments are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        BL_TRAP_ENTRY BdDebugServiceTrap

        BL_GENERATE_TRAP_FRAME          ; generate trap frame

        mov     ecx, STATUS_BREAKPOINT  ; set exception code
        mov     r8, TrRip[rbp]          ; set exception address
        mov     edx, 3                  ; set number of parameters
        mov     r9, TrRax[rbp]          ; set service name.
        mov     r10, TrRcx[rbp]         ; set first argument.
        mov     r11, TrRdx[rbp]         ; set second argument.
        call    EfiCommonExceptionDispatch

        BL_TRAP_END BdDebugServiceTrap

        subttl  "Common Exception Dispatch"
;++
;
; Routine Description:
;
;   This routine allocates an exception frame on stack, saves nonvolatile
;   machine state, and calls the system exception dispatcher.
;
;   N.B. It is the responsibility of the caller to initialize the exception
;        record.
;
; Arguments:
;
;   ecx - Supplies the exception code.
;
;   edx - Supplies the number of parameters.
;
;   r8 - Supplies the exception address.
;
;   r9 - r11 - Supply the exception  parameters.
;
;   rbp - Supplies a pointer to the trap frame.
;
;   rsp - Supplies a pointer to the exception frame.
;
; Return Value:
;
;    There is no return from this function.
;
;--

        BL_TRAP_ENTRY EfiCommonExceptionDispatch

        BL_GENERATE_EXCEPTION_FRAME NoPop ; generate exception frame

        lea     rax, (KEXCEPTION_FRAME_LENGTH - 8)[rsp] ; get exception record address
        mov     ErExceptionCode[rax], ecx ; set exception code
        xor     ecx, ecx                ;
        mov     dword ptr ErExceptionFlags[rax], ecx ; clear exception flags
        mov     ErExceptionRecord[rax], rcx ; clear exception record address
        mov     ErExceptionAddress[rax], r8 ; set exception address
        mov     ErNumberParameters[rax], edx ; set number of parameters
        mov     ErExceptionInformation[rax], r9 ; set exception parameters
        mov     ErExceptionInformation + 8[rax], r10 ;
        mov     ErExceptionInformation + 16[rax], r11 ;
        lea     r8, (-128)[rbp]         ; set trap frame address
        mov     rdx, rsp                ; set exception frame address
        mov     rcx, rax                ; set exception record address
        call    EfiDispatchException    ; dispatch exception

        BL_RESTORE_EXCEPTION_STATE NoPop ; restore exception state/deallocate

        BL_RESTORE_TRAP_STATE Volatile  ; restore trap state and exit

        BL_TRAP_END EfiCommonExceptionDispatch

        end
