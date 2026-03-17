/**@file Exception.c

This file contains ARM64 exception related support functions.

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

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/Cpu.h>
#include <KdTypes.h>
#include <Library/KdProtocolLib.h>
#include <Exception.h>
#include <KdDxe.h>
#include <Library/WatchdogTimerLib.h>

EFI_CPU_ARCH_PROTOCOL  *mKdDxeCpu = NULL;

VOID
KdDxeAbortException (
  EFI_SYSTEM_CONTEXT_AARCH64  *Context,
  EXCEPTION_RECORD            *ExceptionRecord
  );

VOID
KdDxeTrap (
  EXCEPTION_RECORD            *ExceptionRecord,
  EFI_SYSTEM_CONTEXT_AARCH64  *Context
  );

/**
  This routine handles synchronous exceptions.

  @param  InterruptType     Supplies the type of the exception.
  @param  SystemContext     Supplies the system context at the time of the exception.

  N.B. For more information about ARM64 exception handling, download the ARMV8
       architecture reference manual from https://developer.arm.com/.

**/
VOID
EFIAPI
KdDxeExceptionHandler (
  EFI_EXCEPTION_TYPE  InterruptType,
  EFI_SYSTEM_CONTEXT  SystemContext
  )
{
  EFI_SYSTEM_CONTEXT_AARCH64  *Context;
  EXCEPTION_RECORD            ExceptionRecord;
  UINT64                      ExceptionType;
  UINT64                      Value;
  BOOLEAN                     WatchdogState;

  //
  // Suspend the watchdog while handling debug events
  // Even simple debug events, like symbol loading,
  // can wait in the debugger if there was a pending break-in.
  //
  WatchdogState = WatchdogSuspend ();

  Context = SystemContext.SystemContextAArch64;
  ZeroMem (&ExceptionRecord, sizeof (EXCEPTION_RECORD));

  //
  // Determine the incoming exception type from the upper 6 bits of the
  // ESR value stored in the TrapFrame. The default behavior (upon issuing
  // a break out of the switch statement) is to dispatch an exception
  // described in ExceptionRecord.
  //
  // If the exception is handled and should be returned from, a premature
  // return is issued instead of a break.
  //

  ExceptionType = (Context->ESR >> 26);

  switch (ExceptionType) {
    case 0x00:

      //
      // Unknown or Uncategorised Reason (illegal opcodes)
      //

      ExceptionRecord.ExceptionCode           = KI_EXCEPTION_INVALID_OP;
      ExceptionRecord.ExceptionAddress        = Context->ELR;
      ExceptionRecord.NumberParameters        = 1;
      ExceptionRecord.ExceptionInformation[0] = Context->SPSR;
      break;

    case 0x20:
    case 0x21:
    case 0x24:
    case 0x25:

      //
      // Instruction (20/21) or data (24/25) abort from either
      // the current EL (21/25) or a lower EL (20/24)
      //

      KdDxeAbortException (Context, &ExceptionRecord);
      break;

    case 0x22:

      //
      // PC alignment exception
      //

      ExceptionRecord.ExceptionCode    = STATUS_INSTRUCTION_MISALIGNMENT;
      ExceptionRecord.ExceptionAddress = Context->ELR;
      ExceptionRecord.NumberParameters = 0;
      break;

    case 0x26:

      //
      // Stack alignment exception
      //

      ExceptionRecord.ExceptionCode    = STATUS_DATATYPE_MISALIGNMENT;
      ExceptionRecord.ExceptionAddress = Context->ELR;
      ExceptionRecord.NumberParameters = 0;
      break;

    case 0x2c:

      //
      // Floating point exception from AArch64 mode
      //

      Value  = ReadStatusRegFpcr ();
      Value &= ~(FPCR_IDE | FPCR_IXE | FPCR_UFE | FPCR_OFE | FPCR_DZE | FPCR_IOE);
      WriteStatusRegFpcr (Value);
      break;

    case 0x2f:

      //
      // SError interrupt
      //

      // ARM64_WORKITEM: What to do here?
      ExceptionRecord.ExceptionCode    = STATUS_DATATYPE_MISALIGNMENT;
      ExceptionRecord.ExceptionAddress = Context->ELR;
      ExceptionRecord.NumberParameters = 0;
      break;

    case 0x30:
    case 0x31:

      //
      // Hardware breakpoint exception from current EL (31) or
      // from lower EL (30)
      //

      ExceptionRecord.ExceptionCode           = STATUS_BREAKPOINT;
      ExceptionRecord.ExceptionAddress        = Context->ELR;
      ExceptionRecord.NumberParameters        = 1;
      ExceptionRecord.ExceptionInformation[0] = BREAKPOINT_HW_BREAK;
      break;

    case 0x32:
    case 0x33:

      //
      // Single step exception from current EL (33) or
      // from lower EL (32)
      //

      Value  = ReadStatusRegMdscrEl1 ();
      Value &= ~ARM64_MDSCR_SS;
      WriteStatusRegMdscrEl1 (Value);

      ExceptionRecord.ExceptionCode    = STATUS_SINGLE_STEP;
      ExceptionRecord.ExceptionAddress = Context->ELR;
      ExceptionRecord.NumberParameters = 0;
      break;

    case 0x34:
    case 0x35:

      //
      // Hardware watchpoint exception from current EL (35) or
      // from lower EL (34)
      //

      ExceptionRecord.ExceptionCode           = STATUS_BREAKPOINT;
      ExceptionRecord.ExceptionAddress        = Context->ELR;
      ExceptionRecord.NumberParameters        = 2;
      ExceptionRecord.ExceptionInformation[0] = BREAKPOINT_HW_WATCH;
      ExceptionRecord.ExceptionInformation[1] = Context->FAR;
      break;

    case 0x3c:

      //
      // BRK instruction in AArch64 mode
      //

      ExceptionRecord.ExceptionCode           = STATUS_BREAKPOINT;
      ExceptionRecord.ExceptionAddress        = Context->ELR;
      ExceptionRecord.NumberParameters        = 2;
      ExceptionRecord.ExceptionInformation[0] = BREAKPOINT_BREAK;
      ExceptionRecord.ExceptionInformation[1] = Context->ESR & 0xffff;
      break;

    case 0x01:  // WFE/WFI traps
    case 0x03:  // MCR/MRC to invalid CP15 in AArch32 mode
    case 0x04:  // MCRR/MRRC to invalid CP15 in AArch32 mode
    case 0x05:  // MCR/MRC to invalid CP14 in AArch32 mode
    case 0x06:  // LDC/STC to invalid CP14 in AArch32 mode
    case 0x07:  // Invalid access to VFP/Advanced SIMD registers
    case 0x08:  // MRC to invalid CP10 in AArch32 mode
    case 0x0c:  // MCRR/MRRC to invalid CP14 in AArch32 mode
    case 0x0e:  // Illegal Instruction Set state (IL) bit
    case 0x11:  // SVC from AArch32 mode
    case 0x12:  // HVC from AArch32 mode
    case 0x13:  // SMC from AArch32 mode
    case 0x15:  // SVC from AArch64 mode (should be short-circuited)
    case 0x16:  // HVC from AArch64 mode
    case 0x17:  // SMC from AArch64 mode
    case 0x18:  // invalid MSR/MRS/SYS in AArch64 mode
    case 0x28:  // floating point exception from AArch32 mode
    case 0x38:  // BKPT from AArch32 mode
    case 0x3a:  // AArch32 mode vector catch
    default:

      //
      // Miscellaneous unhandled situations
      //
      CpuDeadLoop ();
  }

  //
  // Enable interrupts if they were previously enabled, then dispatch
  // the exception.
  //

  if ((Context->SPSR & DAIF_INT) == 0) {
    EnableInterrupts ();
  }

  KdDxeTrap (&ExceptionRecord, Context);

  //
  // Enable the single step registers before returning.
  //

  if (Context->SPSR & BIT21) {
    Value  = ReadStatusRegMdscrEl1 ();
    Value |= (ARM64_MDSCR_SS | ARM64_MDSCR_MDE | ARM64_MDSCR_KDE);
    WriteStatusRegMdscrEl1 (Value);
  }

  //
  // Resume the watchdog.
  //
  WatchdogResume (WatchdogState);

  return;
}

/**
  This routine handles both instruction and data abort exceptions.

  @param  Context           Supplies a pointer to the trap frame.
  @param  ExceptionRecord   Supplies a pointer to an exception record
                            that will be filled in.

**/
VOID
KdDxeAbortException (
  EFI_SYSTEM_CONTEXT_AARCH64  *Context,
  EXCEPTION_RECORD            *ExceptionRecord
  )
{
  UINT64  FaultStatus;
  UINT32  FaultType;

  //
  // Enable interrupts if they were previously enabled.
  //

  if ((Context->SPSR & DAIF_INT) == 0) {
    EnableInterrupts ();
  }

  //
  // The fault type is provided by the low 6 bits of the ESR, as follows:
  //
  //      0x00-0x03   0000LL Address Size Fault
  //      0x04-0x07   0001LL Translation Fault
  //      0x08-0x0B   0010LL Access Flag Fault
  //      0x0C-0x0F   0011LL Permission Fault
  //      0x10        010000 Synchronous External Abort
  //      0x14-0x17   0101LL Synchronous External Abort on Translation Table walk
  //      0x18        011000 Memory access Synchronous Parity error
  //      0x1C-0x1F   0111LL Memory access Synchronous Parity error on Translation Table walk
  //      0x21        100001 Alignment Fault
  //      0x30        110000 TLB Conflict
  //      0x34        110100 IMPLEMENTATION DEFINED (Lockdown Abort)
  //      0x35        110101 IMPLEMENTATION DEFINED (unsupported exclusive)
  //      0x3A        111010 IMPLEMENTATION DEFINED (Coprocessor Aobrt)
  //      0x3D        111101 Section Domain Fault
  //      0x3E        111110 Page Domain Fault
  //

  switch (Context->ESR & 0x3f) {
    //
    // Address size, translation, access flag, and permission faults all
    // go through Mm for processing.
    //

    default:
    case 0x00:  case 0x01:  case 0x02:  case 0x03:
    case 0x04:  case 0x05:  case 0x06:  case 0x07:
    case 0x08:  case 0x09:  case 0x0a:  case 0x0b:
    case 0x0c:  case 0x0d:  case 0x0e:  case 0x0f:

      //
      // Determine whether this was an execution fault (implied if the fault
      // type was an Instruction Abort), and if not, whether it was a write
      // fault.
      //

      FaultType   = (UINT32)Context->ESR >> 26;
      FaultStatus = 0;
      if ((FaultType == 0x20) || (FaultType == 0x21)) {
        FaultStatus = SWFS_EXECUTE;
      } else if ((Context->ESR & 0x40) != 0) {
        FaultStatus = SWFS_WRITE;
      }

      //
      // Synthesize an error
      //

      ExceptionRecord->ExceptionCode           = STATUS_ACCESS_VIOLATION;
      ExceptionRecord->ExceptionAddress        = Context->ELR;
      ExceptionRecord->NumberParameters        = 2;
      ExceptionRecord->ExceptionInformation[0] = FaultStatus;
      ExceptionRecord->ExceptionInformation[1] = Context->FAR;
      break;

    //
    // Alignment faults get dispatched with their own code.
    //

    case 0x21:
      ExceptionRecord->ExceptionCode    = STATUS_DATATYPE_MISALIGNMENT;
      ExceptionRecord->ExceptionAddress = Context->FAR;
      ExceptionRecord->NumberParameters = 0;
      break;
  }
}

/**
  This routine is called whenever a exception is dispatched and the EFI debugger
  is active.

  @param  ExceptionRecord   Supplies a pointer to an exception record that
                            describes the exception.
  @param  Context           Supplies a pointer to a trap frame that describes
                            the trap.

**/
VOID
KdDxeTrap (
  EXCEPTION_RECORD            *ExceptionRecord,
  EFI_SYSTEM_CONTEXT_AARCH64  *Context
  )
{
  BOOLEAN             BoolRet;
  EFI_SYSTEM_CONTEXT  Temp;
  KD_STRING           Input;
  UINT64              OldElr;
  UINT32              Opcode;
  KD_STRING           Output;
  BOOLEAN             UnloadSymbols;

  //
  // Invalid instructions are post-processed in the loader and
  // kernel to set the right exception code and information.
  //

  if (ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) {
    Opcode = Context->ESR & 0xffff;
    switch (Opcode) {
      case ARM64_BREAKPOINT:
      case ARM64_DEBUG_SERVICE:
        ExceptionRecord->ExceptionCode    = STATUS_BREAKPOINT;
        ExceptionRecord->NumberParameters = 1;
        if (Opcode == ARM64_BREAKPOINT) {
          ExceptionRecord->ExceptionInformation[0] = BREAKPOINT_BREAK;
        } else {
          ExceptionRecord->ExceptionInformation[0] = Context->X16;
          Context->ELR                            += 4;
        }

        break;

      case ARM64_ASSERT:
        ExceptionRecord->ExceptionCode    = STATUS_ASSERTION_FAILURE;
        ExceptionRecord->NumberParameters = 0;
        ExceptionRecord->ExceptionAddress = Context->ELR;
        break;

      case ARM64_FASTFAIL:
        ExceptionRecord->ExceptionCode           = STATUS_STACK_BUFFER_OVERRUN;
        ExceptionRecord->NumberParameters        = 1;
        ExceptionRecord->ExceptionInformation[0] = Context->X0;
        break;

      default:
        ExceptionRecord->ExceptionCode    = STATUS_ILLEGAL_INSTRUCTION;
        ExceptionRecord->ExceptionAddress = Context->ELR;
    }
  }

  //
  // Print, Prompt, Load symbols, Unload symbols, are all special
  // cases of STATUS_BREAKPOINT.
  //

  UnloadSymbols = FALSE;
  if ((ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) &&
      (ExceptionRecord->ExceptionInformation[0] != BREAKPOINT_BREAK))
  {
    OldElr = Context->ELR;

    //
    // Switch on the breakpoint code.
    //

    switch (ExceptionRecord->ExceptionInformation[0]) {
      //
      // Print a debug string.
      //
      // Arguments:
      //
      //     r0 - Supplies a pointer to an output string buffer.
      //     r1 - Supplies the length of the output string buffer.
      //     r2 - Supplies the Id of the calling component.
      //     r3 - Supplies the output filter level.
      //

      case BREAKPOINT_PRINT:
        Output.Buffer = (UINT8 *)Context->X0;
        Output.Length = (UINT16)Context->X1;
        if (mKdDxeInitialized != FALSE) {
          BoolRet = (BOOLEAN)KdProtocolPrintString (&Output);
          if (BoolRet == FALSE) {
            Context->X0 = STATUS_SUCCESSFUL;
          } else {
            Context->X0 = STATUS_BREAKPOINT;
          }
        } else {
          Context->X0 = STATUS_DEVICE_NOT_CONNECTED;
        }

        Context->ELR += 4;
        goto BdTrapEnd;

      //
      // Print a debug prompt string, then input a string.
      //
      // Arguments:
      //
      //     r0 - Supplies a pointer to an output string buffer.
      //     r1 - Supplies the length of the output string buffer.
      //     r2 - Supplies a pointer to an input string buffer.
      //     r3 - Supplies the length of the input string bufffer.
      //

      case BREAKPOINT_PROMPT:

        Output.Buffer = (UINT8 *)Context->X0;
        Output.Length = (UINT16)Context->X1;
        Input.Buffer  = (UINT8 *)Context->X2;
        Input.Length  = (UINT16)Context->X3;

        //
        // Continue to prompt until no breakin is seen.
        //

        do {
          BoolRet = (BOOLEAN)KdProtocolPromptString (&Output, &Input);
        } while (BoolRet != FALSE);

        Context->X0   = Input.Length;
        Context->ELR += 4;
        goto BdTrapEnd;

      //
      // Load the symbolic information for an image.
      //
      // Arguments:
      //
      //    r0 - Supplies a pointer to a filename string descriptor.
      //    r1 - Supplies the base address of the image.
      //

      case BREAKPOINT_UNLOAD_SYMBOLS:
        UnloadSymbols = TRUE;

      //
      // Fall through
      //

      case BREAKPOINT_LOAD_SYMBOLS:

        if (mKdDxeInitialized != FALSE) {
          Temp.SystemContextAArch64 = Context;
          KdProtocolReportLoadSymbolsStateChange (
            (KD_STRING *)Context->X0,
            (KD_SYMBOLS_INFO *)Context->X1,
            UnloadSymbols,
            &Temp
            );
        }

        //
        // If the kernel debugger did not update the PC (ELR), then
        // increment past the breakpoint instruction.
        //

        if (Context->ELR == OldElr) {
          Context->ELR += 4;
        }

        goto BdTrapEnd;

      //
      // Unknown command.
      //

      default:
        break;
    }

    if (Context->ELR == OldElr) {
      Context->ELR += 4;
    }
  } else {
    //
    // Report state change to the kernel debugger.
    //

    Temp.SystemContextAArch64 = Context;
    KdProtocolReportExceptionStateChange (
      ExceptionRecord,
      &Temp,
      TRUE
      );

    goto BdTrapEnd;
  }

BdTrapEnd:
  return;
}

/**
  This routine initializes the KdDxe exception handling support on AARM64.

  @retval EFI_SUCCESS       On success.
  @retval EFI_STATUS        On failure.

**/
EFI_STATUS
KdDxeExceptionInitialize (
  )
{
  EFI_STATUS  Status;
  UINT64      Value;

  //
  // Find the CPU architectural protocol and register our exception handler.
  //

  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **)&mKdDxeCpu);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Locate(gEfiCpuArchProtocolGuid) failed, Status = (%r).\n", __func__, Status));
    ASSERT_EFI_ERROR (Status);
    goto Cleanup;
  }

  Status = mKdDxeCpu->RegisterInterruptHandler (
                        mKdDxeCpu,
                        EXCEPT_AARCH64_SYNCHRONOUS_EXCEPTIONS,
                        KdDxeExceptionHandler
                        );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: RegisterInterruptHandler failed, Status = (%r).\n", __func__, Status));
    ASSERT_EFI_ERROR (Status);
    mKdDxeCpu = NULL;
    goto Cleanup;
  }

  //
  // Enable debugging.
  //

  WriteStatusRegOslarEl1 (0);
  Value  = ReadStatusRegMdscrEl1 ();
  Value |= (ARM64_MDSCR_MDE | ARM64_MDSCR_KDE | ARM64_MDSCR_TDCC);
  WriteStatusRegMdscrEl1 (Value);

Cleanup:

  return Status;
}

/**
  This routine removes the KdDxe exception handling support.

**/
VOID
KdDxeExceptionDestroy (
  )
{
  EFI_STATUS  Status;

  if (mKdDxeCpu != NULL) {
    Status = mKdDxeCpu->RegisterInterruptHandler (
                          mKdDxeCpu,
                          EXCEPT_AARCH64_SYNCHRONOUS_EXCEPTIONS,
                          NULL
                          );

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: RegisterInterruptHandler failed, Status = (%r).\n", __func__, Status));
      ASSERT_EFI_ERROR (Status);
    }
  }
}
