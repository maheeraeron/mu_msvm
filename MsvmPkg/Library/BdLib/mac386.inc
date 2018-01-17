;++
;
; Copyright (c) 1989  Microsoft Corporation
;
; Module Name:
;
;    mac386.inc - 386 machine specific assembler macros
;
; Abstract:
;
;   This module contains 386 machine specific (assembler) macros
;   applicable to code outside the kernel.  Note that
;   ACQUIRE_SPINLOCK_DIRECT assumes the PCR is handy, so it won't
;   work in user mode (with debugging turned on.)
;
; Author:
;
;   Bryan Willman (bryanwi)  1 Aug 90
;

NT_INST = 0

;
; IceCAP macros
;

ifdef _CAPKERN

;
; Define kernel icecap macros for tracing assembly routines.
;

        extern  __CAP_Start_Profiling_ASM:proc
        extern  __CAP_Start_Tail_Profiling_ASM:proc
        extern  __CAP_End_Profiling_ASM:proc
        extern  __CAP_SystemService:proc
        extern  __CAP_Trap:proc

;
; CAPCALL - Perform an IceCAP instrumented function call
;
; Arguments:
;
;   Callee - A function label.
;
;   NoRet - One possible value:
;
;       NoRet - Set if the function does not return to the caller
;
; Return value:
;
;   Returns the value of the function specified by label.
;

CAPCALL macro Callee, NoRet

ifnb <NoRet>

        push    Callee                    ; push argument
        call    __CAP_Start_Tail_Profiling_ASM ; record profiling information
        call    Callee                    ; perform the actual function call

else

        push    Callee                    ; push argument
        call    __CAP_Start_Profiling_ASM ; record profiling information
        call    Callee                    ; perform the actual function call
        call    __CAP_End_Profiling_ASM   ; record profiling information

endif

        endm

;
; STDCAPCALLCALL - Make the actual stdCall.
;
; Arguments:
;
;   Callee - A function label.
;
;   Bytes - Number of bytes of arguments
;
;   NoRet - If set, the function does not return to the caller.
;
; Return value:
;
;   Returns the value of the function specified by label.
;

STDCAPCALLCALL macro Callee, Bytes, NoRet

ifdef   __imp_&Callee&@&Bytes

        push    dword ptr [__imp_&Callee&@&Bytes] ; push argument

ifnb <NoRet>

        call    __CAP_Start_Tail_Profiling_ASM    ; record profiling information
        call    dword ptr [__imp_&Callee&@&Bytes] ; perform the actual function call

else

        call    __CAP_Start_Profiling_ASM         ; record profiling information
        call    dword ptr [__imp_&Callee&@&Bytes] ; perform the actual function call
        call    __CAP_End_Profiling_ASM           ; record profiling information

endif

else
        push    Callee&@&Bytes                    ; push argument

ifnb <NoRet>

        call    __CAP_Start_Tail_Profiling_ASM    ; record profiling information
        call    Callee&@&Bytes                    ; perform the actual function call

else

        call    __CAP_Start_Profiling_ASM         ; record profiling information
        call    Callee&@&Bytes                    ; perform the actual function call
        call    __CAP_End_Profiling_ASM           ; record profiling information

endif

endif

        endm

;
; STDCAPCALL - Perform an IceCAP instrumented stdCall call
;
; Arguments:
;
;   Callee - A function label.
;
;   ArgList - Arguments of the function to call
;
;   NoRet - If set, the function does not return to the caller.
;
; Return value:
;
;   Returns the value of the function specified by label.
;

STDCAPCALL macro Callee, ArgList, NoRet
        Local   Bytes

        RevPush <ArgList>, Bytes
        Bytes = Bytes*4

        STDCAPCALLCALL Callee, %(Bytes), NoRet

        endm

;
; FSTCAPCALL - Perform an IceCAP instrumented fstCall call
;
; Arguments:
;
;   Callee - A function label.
;
;   ArgList - Arguments of the function to call
;
;   NoRet - If set, the function does not return to the caller.
;
; Return value:
;
;   Returns the value of the function specified by label.
;

FSTCAPCALL macro Callee, ArgList, NoRet
        Local   Bytes

        RevPush <ArgList>, Bytes
        Bytes = Bytes*4

        if Bytes eq 0
            STDCAPCALLCALL &@&Callee,%cFCall&@&Callee,NoRet
        else
            STDCAPCALLCALL &@&Callee,%(Bytes+8),NoRet
        endif
        endm

;
; CAPJMP  - Perform an IceCAP instrumented function tail jump.
;           Use this when a function does not return to the caller.
;
; Callee is a label.
;
; __CAP_Start_Tail_Profiling_ASM does not use PxHome locations.
;

CAPJMP macro Callee

        push    Callee                         ; push argument
        call    __CAP_Start_Tail_Profiling_ASM ; record profiling information
        jmp     Callee                         ; perform the actual jump
        endm

;
; CAPJNZ  - Perform an IceCAP instrumented conditional tail jump.
;           Use this when a function does not return to the caller.
;
; Callee is a label.
;
; __CAP_Start_Tail_Profiling_ASM does not use PxHome locations.
;

CAPJNZ macro Callee

        LOCAL   NotCalled

        jz      NotCalled                      ; If Z, the call is not made
        call    __CAP_Start_Tail_Profiling_ASM ; record profiling information
        jmp     Callee                         ; perform the actual jump

NotCalled:

        endm

;
; CAPJE   - Perform an IceCAP instrumented conditional tail jump.
;           Use this when a function does not return to the caller.
;
; Callee is a label.
;
; __CAP_Start_Tail_Profiling_ASM does not use PxHome locations.
;

CAPJE macro Callee

        LOCAL   NotCalled

        jne     NotCalled                      ; If ne, the call is not made
        call    __CAP_Start_Tail_Profiling_ASM ; record profiling information
        jmp     Callee                         ; perform the actual jump

NotCalled:

        endm

;
; CAPSYSTEMSERVICE - Log a system service record.
;
; Assumes the SystemService address is already loaded into ebx and
; that ebx and all argument registers will be preserved.
;

CAPLOGSYSTEMSERVICE macro ServiceAddress

        call    __CAP_SystemService     ; record profile information
        endm

;
; CAPLOGTRAP - Log a trap record.
;
; Assumes all registers will be preserved.
;

CAPLOGTRAP macro

        call    __CAP_Trap              ; record profile information
        endm

else

CAPCALL macro Callee, NoRet

        call Callee
        endm

STDCAPCALL macro Callee, ArgList, NoRet

        stdCall Callee, <ArgList>
        endm

FSTCAPCALL macro Callee, ArgList, NoRet

        fstCall Callee, <ArgList>
        endm

CAPJMP macro Callee

        jmp Callee
        endm

CAPJNZ macro Callee

        jnz Callee
        endm

CAPJE macro Callee

        je Callee
        endm

CAPLOGSYSTEMSERVICE macro ServiceAddress

        endm

CAPLOGTRAP macro

        endm

endif ; _CAPKERN

;++
;
;  YIELD
;
;  Macro Description:
;
;     This macro implements the yield instruction
;--

YIELD macro
ifndef NT_UP
    db  0f3h
    db  090h
endif
endm

ifdef NTOS_YIELD_MACRO

HVL_SPIN_COUNT_MASK     equ     _HvlLongSpinCountMask
HVL_ENLIGHTENMENTS_MASK equ     _HvlEnlightenments
HVL_LONG_SPIN_WAIT_CALL equ     _HvlNotifyLongSpinWait@4

else

HVL_SPIN_COUNT_MASK     equ     _HalpEnlightenment.HeSpinCountMask
HVL_ENLIGHTENMENTS_MASK equ     _HalpEnlightenment.HeEnlightenments
HVL_LONG_SPIN_WAIT_CALL equ     dword ptr _HalpEnlightenment.HeLongSpinWait

endif

;++
;
;  ENLIGHTENED_YIELD   CounterReg, LockAddress
;
;  Macro Description:
;
;     This macro performs an enlightened yield when running in a virtualized
;     environment.
;
;  Arguments:
;
;     CounterReg  - Volatile register used to track the enlightened spin count.
;
;     LockAddress - Optionally specifies the volatile register containing the
;       lock address to be saved. If this is omitted, all volatile registers
;       will be preserved across this macro.
;
;--

ENLIGHTENED_YIELD macro   CounterReg, LockAddress

local   a                       ; define a local label

.errb <CounterReg>

        inc     CounterReg                      ; increment the counter
        test    HVL_SPIN_COUNT_MASK, CounterReg ; test if long spin wait
        jnz     short a                         ; max count not hit, yield
.errnz (HV_KE_USE_HYPERCALL_FOR_LONG_SPIN_WAIT AND 0FFFFFF00h)
        test    byte ptr HVL_ENLIGHTENMENTS_MASK, HV_KE_USE_HYPERCALL_FOR_LONG_SPIN_WAIT
        jz      short a                         ; long spin not enlightened, yield

ifnb <LockAddress>

        push    LockAddress                     ; save volatile registers
        push    CounterReg                      ; (includes the counter)

else

        push    eax                             ; save all volatile registers
        push    ecx                             ;
        push    edx                             ;

endif

        push    CounterReg                      ; push argument

        CAPCALL HVL_LONG_SPIN_WAIT_CALL         ; issue the hypercall

ifnb <LockAddress>

        pop     CounterReg                      ; restore the volatile registers
        pop     LockAddress                     ;

else

        pop     edx                             ; restore all volatile registers
        pop     ecx                             ;
        pop     eax                             ;

endif

a:      YIELD                                   ; yield to other SMT processors

endm

if NT_INST
else

;++
;
;  ACQUIRE_SPINLOCK LockAddress, SpinLabel, NoChecking, SaveReg
;
;  Macro Description:
;
;     This macro acquires a kernel spin lock.
;
;     N.B. This macro assumes that the current IRQL is set properly.
;        It neither raises nor lowers IRQL.
;
;  Arguments:
;
;     (KSPIN_LOCK) LockAddress - address of SpinLock value
;     SpinLabel   - if acquire spinlock fail, the label to perform the
;                   spin checking.  It could be simply a "label" or
;                   "short label" which means the label is within 128
;                   bytes in distant.
;     NoChecking - Not blank, if no debugging code should be generated.
;     SaveReg     - register to store the spin count.
;--

ACQUIRE_SPINLOCK macro   LockAddress, SpinLabel, NoChecking, SaveReg

local   a                       ; define a local label

.errb <LockAddress>
.errb <SpinLabel>

ifndef NT_UP

;
; Attempt to assert the lock
;

        lock bts dword ptr [LockAddress], 0     ; test and set the spinlock
        jc      SpinLabel               ; spinlock owned, go SpinLabe

ifnb <SaveReg>

;
; Increase the spinlock acquisition count
;

        inc     dword ptr PCR[PcPrcbData+PbSpinLockAcquireCount]

        cmp     SaveReg, 0                      ; check if collision
        jz      a

;
;    Increase the spinlock contention count and spin count
;

        inc     dword ptr PCR[PcPrcbData+PbSpinLockContentionCount]
        add     dword ptr PCR[PcPrcbData+PbSpinLockSpinCount], SaveReg

a:

endif ; SaveReg


if    DBG
ifb   <NoChecking>
        push    edi                     ; save edi
        mov     edi,fs:PcPrcb
        mov     edi, [edi].PbCurrentThread
        or      edi, 1                  ; spinlock owned
        mov     [LockAddress], edi      ; remember current thread
        pop     edi                     ; restore edi
endif ; NoChecking
endif ; DBG
endif ; NT_UP

endm

;++
;
;  SPIN_ON_SPINLOCK   LockAddress, AcquireLabel
;
;  Macro Description:
;
;     This macro spins on a kernel spin lock.
;
;     N.B. This macro assumes that the current IRQL is set properly.
;        It neither raises nor lowers IRQL.
;
;  Arguments:
;
;     (KSPIN_LOCK) LockAddress - address of a SpinLock value
;
;     SpinLabel    - if the test on cleared spinlock sucess, the label
;                    to assert the spin lock.  It could be simply a
;                    "label" or  "short label" which means the label is
;                    within 128 bytes in distance.
;
;     NoChecking - Not blank, if no debugging code should be generated.
;--

SPIN_ON_SPINLOCK macro   LockAddress, AcquireLabel, NoChecking, PollDebugger, NoTimeout, EnlightenReg, SaveReg

local   a,b                             ; define a local label

.errb <LockAddress>
.errb <AcquireLabel>

ifnb <EnlightenReg>

ifnb <SaveReg>

        push    EnlightenReg                    ; save register

endif ; SaveReg

        xor     EnlightenReg, EnlightenReg      ; initialize counter

endif ; EnlightenReg

a:

ifnb <EnlightenReg>

        ENLIGHTENED_YIELD EnlightenReg

else

        YIELD

endif ; EnlightenReg

        test    dword ptr [LockAddress], 1 ; Was spinlock cleared?
        jnz     short a                    ; Spin

ifnb <EnlightenReg>

ifnb <SaveReg>

        pop     EnlightenReg               ; Restore register

endif ; SaveReg

endif ; EnlightenReg

        jmp     AcquireLabel               ; Try for the lock again

endm


;++
;
;  TEST_SPINLOCK   LockAddress, BusyLabel
;
;  Macro Description:
;
;     This macro tests a kernel spin lock to see if it's busy.
;     If it's not busy, ACQUIRE_SPINLOCK still needs to be called
;     to obtain the spinlock in a locked manner.
;
;  Arguments:
;
;     (KSPIN_LOCK) LockAddress - address of a SpinLock value


TEST_SPINLOCK macro   LockAddress, BusyLabel
        cmp     dword ptr [LockAddress], 0 ; spinlock clear?
        jnz     BusyLabel                  ; No, then busy
endm




;++
;
;  RELEASE_SPINLOCK  LockAddress
;
;  Macro Description:
;
;     This macro releases a kernel spin lock.
;
;     N.B. This macro assumes that the current IRQL is set properly.
;        It neither raises nor lowers IRQL.
;
;  Arguments:
;
;     (KSPIN_LOCK) LockAddress - Supplies an address to a spin lock value
;     NoChecking - Not blank, if no debugging code should be generated.
;--

RELEASE_SPINLOCK macro   LockAddress, NoChecking
local a
.errb <LockAddress>
ifndef NT_UP
if    DBG
ifb   <NoChecking>
EXTRNP  _KeBugCheckEx,5

        push    edi                     ; save edi
        mov     edi,fs:PcPrcb
        mov     edi,[edi].PbCurrentThread
        or      edi, 1                  ; assume current thread owns the lock
        cmp     edi, [LockAddress]      ; Does current thread own the lock?
        pop     edi                     ; restore edi
        jz      short a                 ; if z, yes, goto a and release lock
        STDCAPCALL _KeBugCheckEx,<SPIN_LOCK_NOT_OWNED,LockAddress,0,0,0>, <NoRet>
a:
endif
        lock and dword ptr [LockAddress], 0
else
        lock and byte ptr [LockAddress], 0

endif   ; DBG
endif   ; NT_UP
endm


endif
if NT_INST

;
; These are the instrumentation version of the above functions.
; internal use only
;

ACQUIRE_SPINLOCK macro   LockAddress, SpinLabel, NoChecking
EXTRNP  KiInst_AcquireSpinLock,0
ifidni  <&LockAddress>, <eax>
        STDCAPCALL KiInst_AcquireSpinLock
else
        push    eax
        mov     eax, LockAddress
        STDCAPCALL KiInst_AcquireSpinLock
        pop     eax
endif
        jc      SpinLabel
endm

SPIN_ON_SPINLOCK macro   LockAddress, AcquireLabel, NoChecking, PollDebugger
EXTRNP  KiInst_SpinOnSpinLock,0
ifidni  <&LockAddress>, <eax>
        STDCAPCALL KiInst_SpinOnSpinLock
else
        push    eax
        mov     eax, LockAddress
        STDCAPCALL KiInst_SpinOnSpinLock
        pop     eax
endif
        jmp     AcquireLabel
endm

TEST_SPINLOCK macro   LockAddress, BusyLabel
EXTRNP  KiInst_TestSpinLock,0
ifidni  <&LockAddress>, <eax>
        STDCAPCALL KiInst_TestSpinLock
else
        push    eax
        mov     eax, LockAddress
        STDCAPCALL KiInst_TestSpinLock
        pop     eax
endif
        jnc     AcquireLabel
endm

RELEASE_SPINLOCK macro   LockAddress, NoChecking
EXTRNP  KiInst_ReleaseSpinLock,0
ifidni  <&LockAddress>, <eax>
        STDCAPCALL KiInst_ReleaseSpinLock
else
        push    eax
        mov     eax, LockAddress
        STDCAPCALL KiInst_ReleaseSpinLock
        pop     eax
endif
endm

endif

