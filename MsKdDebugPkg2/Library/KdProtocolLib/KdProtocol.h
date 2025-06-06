/**@file KdProtocol.h

This file contains internal definitions and prototypes related to KdProtocolLib.

Copyright (c) 2018, Microsoft Corporation

All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#ifndef __KDPROTOCOL_H__
#define __KDPROTOCOL_H__

#if defined (_MSC_EXTENSIONS)
  #pragma warning(disable: 4201)
#endif

//
// AMD64 Context Formats
//

//
// Define 128-bit 16-byte aligned xmm register type.
//
typedef struct _M128A {
  UINT64    Low;
  INT64     High;
} M128A;

typedef struct _KDESCRIPTOR {
  UINT16    Pad[3];
  UINT16    Limit;
  VOID      *Base;
} KDESCRIPTOR;

typedef struct _AMD64_SPECIAL_REGISTERS {
  UINT64         Cr0;
  UINT64         Cr2;
  UINT64         Cr3;
  UINT64         Cr4;
  UINT64         Dr0;
  UINT64         Dr1;
  UINT64         Dr2;
  UINT64         Dr3;
  UINT64         Dr6;
  UINT64         Dr7;
  KDESCRIPTOR    Gdtr;
  KDESCRIPTOR    Idtr;
  UINT16         Tr;
  UINT16         Ldtr;
  UINT32         MxCsr;
  UINT64         DebugControl;
  UINT64         LastBranchToRip;
  UINT64         LastBranchFromRip;
  UINT64         LastExceptionToRip;
  UINT64         LastExceptionFromRip;
  UINT64         Cr8;
  UINT64         MsrGsBase;
  UINT64         MsrGsSwap;
  UINT64         MsrStar;
  UINT64         MsrLStar;
  UINT64         MsrCStar;
  UINT64         MsrSyscallMask;
  UINT64         Xcr0;
} AMD64_SPECIAL_REGISTERS;

//
// Format of data for (F)XSAVE/(F)XRSTOR instruction
//
typedef struct _XMM_SAVE_AREA32 {
  UINT16    ControlWord;
  UINT16    StatusWord;
  UINT8     TagWord;
  UINT8     Reserved1;
  UINT16    ErrorOpcode;
  UINT32    ErrorOffset;
  UINT16    ErrorSelector;
  UINT16    Reserved2;
  UINT32    DataOffset;
  UINT16    DataSelector;
  UINT16    Reserved3;
  UINT32    MxCsr;
  UINT32    MxCsr_Mask;
  M128A     FloatRegisters[8];

 #if defined (_WIN64)

  M128A     XmmRegisters[16];
  UINT8     Reserved4[96];

 #else

  M128A     XmmRegisters[8];
  UINT8     Reserved4[220];

  //
  // Cr0NpxState is not a part of XSAVE/XRSTOR format. The OS is relying on
  // a fact that neither (FX)SAVE nor (F)XSTOR uses this area.
  //
  UINT32    Cr0NpxState;

 #endif
} XMM_SAVE_AREA32;

typedef struct _AMD64_CONTEXT {
  //
  // Register parameter home addresses.
  //
  // N.B. These fields are for convience - they could be used to extend the
  //      context record in the future.
  //

  UINT64    P1Home;
  UINT64    P2Home;
  UINT64    P3Home;
  UINT64    P4Home;
  UINT64    P5Home;
  UINT64    P6Home;

  //
  // Control flags.
  //

  UINT32    ContextFlags;
  UINT32    MxCsr;

  //
  // Segment Registers and processor flags.
  //

  UINT16    SegCs;
  UINT16    SegDs;
  UINT16    SegEs;
  UINT16    SegFs;
  UINT16    SegGs;
  UINT16    SegSs;
  UINT32    EFlags;

  //
  // Debug registers
  //

  UINT64    Dr0;
  UINT64    Dr1;
  UINT64    Dr2;
  UINT64    Dr3;
  UINT64    Dr6;
  UINT64    Dr7;

  //
  // Integer registers.
  //

  UINT64    Rax;
  UINT64    Rcx;
  UINT64    Rdx;
  UINT64    Rbx;
  UINT64    Rsp;
  UINT64    Rbp;
  UINT64    Rsi;
  UINT64    Rdi;
  UINT64    R8;
  UINT64    R9;
  UINT64    R10;
  UINT64    R11;
  UINT64    R12;
  UINT64    R13;
  UINT64    R14;
  UINT64    R15;

  //
  // Program counter.
  //

  UINT64    Rip;

  //
  // Floating point state.
  //

  union {
    XMM_SAVE_AREA32    FltSave;
    struct {
      M128A    Header[2];
      M128A    Legacy[8];
      M128A    Xmm0;
      M128A    Xmm1;
      M128A    Xmm2;
      M128A    Xmm3;
      M128A    Xmm4;
      M128A    Xmm5;
      M128A    Xmm6;
      M128A    Xmm7;
      M128A    Xmm8;
      M128A    Xmm9;
      M128A    Xmm10;
      M128A    Xmm11;
      M128A    Xmm12;
      M128A    Xmm13;
      M128A    Xmm14;
      M128A    Xmm15;
    };
  };

  //
  // Vector registers.
  //

  M128A     VectorRegister[26];
  UINT64    VectorControl;

  //
  // Special debug control registers.
  //

  UINT64    DebugControl;
  UINT64    LastBranchToRip;
  UINT64    LastBranchFromRip;
  UINT64    LastExceptionToRip;
  UINT64    LastExceptionFromRip;
} AMD64_CONTEXT;

//
// ARM Context Formats
//

#define ARM64_MAX_BREAKPOINTS  8
#define ARM64_MAX_WATCHPOINTS  2

typedef struct _ARM64_CONTEXT {
  //
  // Control flags.
  //

  UINT32    ContextFlags;

  //
  // Integer registers
  //

  UINT32    Cpsr;      // NZVF + DAIF + CurrentEL + SPSel
  union {
    struct {
      UINT64    X_0;
      UINT64    X_1;
      UINT64    X_2;
      UINT64    X_3;
      UINT64    X_4;
      UINT64    X_5;
      UINT64    X_6;
      UINT64    X_7;
      UINT64    X_8;
      UINT64    X_9;
      UINT64    X_10;
      UINT64    X_11;
      UINT64    X_12;
      UINT64    X_13;
      UINT64    X_14;
      UINT64    X_15;
      UINT64    X_16;
      UINT64    X_17;
      UINT64    X_18;
      UINT64    X_19;
      UINT64    X_20;
      UINT64    X_21;
      UINT64    X_22;
      UINT64    X_23;
      UINT64    X_24;
      UINT64    X_25;
      UINT64    X_26;
      UINT64    X_27;
      UINT64    X_28;
      UINT64    Fp;
      UINT64    Lr;
    };

    UINT64    X[31];
  };

  UINT64    Sp;
  UINT64    Pc;

  //
  // Floating Point/NEON Registers
  //

  UINT64    V0[2];
  UINT64    V1[2];
  UINT64    V2[2];
  UINT64    V3[2];
  UINT64    V4[2];
  UINT64    V5[2];
  UINT64    V6[2];
  UINT64    V7[2];
  UINT64    V8[2];
  UINT64    V9[2];
  UINT64    V10[2];
  UINT64    V11[2];
  UINT64    V12[2];
  UINT64    V13[2];
  UINT64    V14[2];
  UINT64    V15[2];
  UINT64    V16[2];
  UINT64    V17[2];
  UINT64    V18[2];
  UINT64    V19[2];
  UINT64    V20[2];
  UINT64    V21[2];
  UINT64    V22[2];
  UINT64    V23[2];
  UINT64    V24[2];
  UINT64    V25[2];
  UINT64    V26[2];
  UINT64    V27[2];
  UINT64    V28[2];
  UINT64    V29[2];
  UINT64    V30[2];
  UINT64    V31[2];

  UINT32    Fpcr;
  UINT32    Fpsr;

  //
  // Debug registers
  //

  UINT32    Bcr[ARM64_MAX_BREAKPOINTS];
  UINT64    Bvr[ARM64_MAX_BREAKPOINTS];
  UINT32    Wcr[ARM64_MAX_WATCHPOINTS];
  UINT64    Wvr[ARM64_MAX_WATCHPOINTS];
} ARM64_KD_CONTEXT;

typedef struct _ARM64_SPECIAL_REGISTERS {
  UINT64    Elr_El1;
  UINT32    Spsr_El1;
  UINT64    Tpidr_El0;
  UINT64    Tpidrro_El0;
  UINT64    Tpidr_El1;

  //
  // H/w [break/watch]point support.
  //

  UINT64    KernelBvr[ARM64_MAX_BREAKPOINTS];
  UINT32    KernelBcr[ARM64_MAX_BREAKPOINTS];
  UINT64    KernelWvr[ARM64_MAX_WATCHPOINTS];
  UINT32    KernelWcr[ARM64_MAX_WATCHPOINTS];
} ARM64_SPECIAL_REGISTERS;

#if defined (MDE_CPU_IA32)

#define CONTEXT_TO_PROGRAM_COUNTER(Context)  ((Context)->Eip)
typedef X86_NT5_CONTEXT KD_CONTEXT;

#define BREAKPOINT_TYPE   UINT8
#define BREAKPOINT_VALUE  0xcc

#elif defined (MDE_CPU_X64)

#define CONTEXT_TO_PROGRAM_COUNTER(Context)  ((Context)->SystemContextX64->Rip)
typedef AMD64_CONTEXT KD_CONTEXT;

#define CONTEXT_AMD64  0x00100000L

#define CONTEXT_CONTROL          (CONTEXT_AMD64 | 0x00000001L)
#define CONTEXT_INTEGER          (CONTEXT_AMD64 | 0x00000002L)
#define CONTEXT_SEGMENTS         (CONTEXT_AMD64 | 0x00000004L)
#define CONTEXT_FLOATING_POINT   (CONTEXT_AMD64 | 0x00000008L)
#define CONTEXT_DEBUG_REGISTERS  (CONTEXT_AMD64 | 0x00000010L)

#define CONTEXT_FULL  (CONTEXT_CONTROL | CONTEXT_INTEGER)

#define BREAKPOINT_TYPE   UINT8
#define BREAKPOINT_VALUE  0xcc

#elif defined (MDE_CPU_AARCH64)

#define CONTEXT_TO_PROGRAM_COUNTER(Context)  ((Context)->SystemContextAArch64->ELR) // BUGBUG:  Verify this.
typedef ARM64_KD_CONTEXT KD_CONTEXT;

#define CONTEXT_ARM64                  0x00400000L
#define CONTEXT_ARM64_CONTROL          (CONTEXT_ARM64 | 0x1L)
#define CONTEXT_ARM64_INTEGER          (CONTEXT_ARM64 | 0x2L)
#define CONTEXT_ARM64_FLOATING_POINT   (CONTEXT_ARM64 | 0x4L)
#define CONTEXT_ARM64_DEBUG_REGISTERS  (CONTEXT_ARM64 | 0x8L)
#define CONTEXT_ARM64_X18              (CONTEXT_ARM64 | 0x10L)

#define CONTEXT_FULL  (CONTEXT_ARM64_CONTROL | CONTEXT_ARM64_INTEGER | CONTEXT_ARM64_FLOATING_POINT)

#define BREAKPOINT_TYPE   UINT32
#define BREAKPOINT_VALUE  (0xd4200000 | (0xf000 << 5))

#else

  #error "Unsupported architecture"

#endif

typedef enum {
  ContinueError   = FALSE,
  ContinueSuccess = TRUE,
  ContinueProcessorReselected
} KCONTINUE_STATUS;

VOID
KdProtocolQueryMemory (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolReadVirtualMemory (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolWriteVirtualMemory (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolReadPhysicalMemory (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolWritePhysicalMemory (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolGetContext (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolSetContext (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolWriteBreakpoint (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolRestoreBreakpoint (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolReadControlSpace (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolWriteControlSpace (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolReadIoSpace (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolWriteIoSpace (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolGetStateChange (
  DBGKD_MANIPULATE_STATE64  *m,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolGetVersion (
  DBGKD_MANIPULATE_STATE64  *m
  );

UINT32
KdProtocolWriteBreakPointEx (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

UINT32
KdProtocolDeleteBreakpointRange (
  UINT64  Lower,
  UINT64  Upper
  );

VOID
KdProtocolRestoreBreakPointEx (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolReadMachineSpecificRegister (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

VOID
KdProtocolWriteMachineSpecificRegister (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  );

UINT8 *
KdProtocolWriteCheck (
  UINT8  *Address
  );

UINT32
KdProtocolMoveMemory (
  volatile UINT8  *Destination,
  volatile UINT8  *Source,
  UINT32          Length
  );

VOID
KdProtocolCopyMemory (
  volatile UINT8  *Destination,
  volatile UINT8  *Source,
  UINT32          Length
  );

VOID
KdProtocolSetStateChange (
  DBGKD_ANY_WAIT_STATE_CHANGE  *WaitStateChange,
  EXCEPTION_RECORD             *ExceptionRecord,
  EFI_SYSTEM_CONTEXT           *Context
  );

VOID
KdProtocolSetContextState (
  DBGKD_ANY_WAIT_STATE_CHANGE  *WaitStateChange,
  EFI_SYSTEM_CONTEXT           *Context
  );

UINT32
KdProtocolAddBreakpoint (
  IN UINT64  Address
  );

UINT32
KdProtocolDeleteBreakpoint (
  IN UINT32  Handle
  );

UINT32
KdProtocolDeleteBreakpointRange (
  IN UINT64  Lower,
  IN UINT64  Upper
  );

VOID
KdProtocolSuspendBreakpoint (
  IN UINT32  Handle
  );

VOID
KdProtocolSuspendAllBreakpoints (
  VOID
  );

UINT8 *
KdProtocolWriteCheck (
  UINT8  *Address
  );

UINT8 *
KdProtocolReadCheck (
  UINT8  *Address
  );

UINT8 *
KdProtocolTranslatePhysicalAddress (
  UINT64  Address
  );

VOID
KdProtocolUnmapVirtualAddress (
  UINT8  *Va
  );

VOID
KdProtocolContextToKdContext (
  EFI_SYSTEM_CONTEXT  *Context,
  KD_CONTEXT          *KdContext
  );

void
KdProtocolKdContextToContext (
  KD_CONTEXT          *KdContext,
  EFI_SYSTEM_CONTEXT  *SystemContext
  );

void
KdProtocolInvalidateInstructionCache (
  UINT64  Address
  );

UINT16
KdProtocolGetMachineType (
  );

#endif
