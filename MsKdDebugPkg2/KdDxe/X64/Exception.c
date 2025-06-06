/**@file Exception.c

This file contains X64 exception related support functions.

Copyright (c) Microsoft Corporation

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

UINT32  mExceptionTypes[] = {
  EXCEPT_X64_DEBUG,
  EXCEPT_X64_BREAKPOINT,
  EXCEPT_X64_DOUBLE_FAULT,
  EXCEPT_X64_GP_FAULT,
  EXCEPT_X64_PAGE_FAULT,
  EXCEPT_X64_SEG_NOT_PRESENT,
  EXCEPT_NT_ASSERT,
  EXCEPT_NT_DEBUG_SERVICE,
  0xFFFF
};

VOID
KdDxeTrap (
  EXCEPTION_RECORD        *ExceptionRecord,
  EFI_SYSTEM_CONTEXT_X64  *Context
  );

/**
  This routine handles synchronous exceptions.

  @param  InterruptType     Supplies the type of the exception.
  @param  SystemContext     Supplies the system context at the time of the exception.

  N.B. For more information about X64 exception handling, download Intel's
       software developer manuals at
       https://software.intel.com/en-us/articles/intel-sdm.

**/
VOID
EFIAPI
KdDxeExceptionHandler (
  EFI_EXCEPTION_TYPE  InterruptType,
  EFI_SYSTEM_CONTEXT  SystemContext
  )
{
  EFI_SYSTEM_CONTEXT_X64  *Context;
  EXCEPTION_RECORD        ExceptionRecord;
  BOOLEAN                 WatchdogState;

  //
  // Suspend the watchdog while handling debug events
  // Even simple debug events, like symbol loading,
  // can wait in the debugger if there was a pending break-in.
  //
  WatchdogState = WatchdogSuspend ();

  Context = SystemContext.SystemContextX64;
  ZeroMem (&ExceptionRecord, sizeof (EXCEPTION_RECORD));

  switch (InterruptType) {
    case EXCEPT_X64_DEBUG:
      Context->Rflags                 &= (UINT64) ~BIT8;
      ExceptionRecord.ExceptionCode    = STATUS_SINGLE_STEP;
      ExceptionRecord.ExceptionAddress = Context->Rip;
      ExceptionRecord.NumberParameters = 0;
      break;

    case EXCEPT_X64_BREAKPOINT:
      ExceptionRecord.ExceptionCode = STATUS_BREAKPOINT;
      //
      // Decrement this as 0xCC op code is a trap with RIP pointing after the instruction.
      //
      Context->Rip                           -= 1;
      ExceptionRecord.ExceptionAddress        = Context->Rip;
      ExceptionRecord.NumberParameters        = 1;
      ExceptionRecord.ExceptionInformation[0] = BREAKPOINT_BREAK;
      break;

    case EXCEPT_X64_DOUBLE_FAULT:
    case EXCEPT_X64_SEG_NOT_PRESENT:
    case EXCEPT_X64_GP_FAULT:
    case EXCEPT_X64_PAGE_FAULT:
      ExceptionRecord.ExceptionCode           = STATUS_ACCESS_VIOLATION;
      ExceptionRecord.ExceptionAddress        = Context->Rip;
      ExceptionRecord.NumberParameters        = 2;
      ExceptionRecord.ExceptionInformation[0] = Context->ExceptionData & 0xFFFF;
      ExceptionRecord.ExceptionInformation[1] = InterruptType;
      break;

    case EXCEPT_NT_ASSERT:
      ExceptionRecord.ExceptionCode    = STATUS_ASSERTION_FAILURE;
      ExceptionRecord.ExceptionAddress = Context->Rip;
      ExceptionRecord.NumberParameters = 0;
      break;

    case EXCEPT_NT_DEBUG_SERVICE:
      ExceptionRecord.ExceptionCode           = STATUS_BREAKPOINT;
      ExceptionRecord.ExceptionAddress        = Context->Rip;
      ExceptionRecord.NumberParameters        = 3;
      ExceptionRecord.ExceptionInformation[0] = Context->Rax;
      ExceptionRecord.ExceptionInformation[1] = Context->Rcx;
      ExceptionRecord.ExceptionInformation[2] = Context->Rdx;
      break;

    default:

      //
      // Unhandled situations
      //

      CpuDeadLoop ();
  }

  KdDxeTrap (&ExceptionRecord, Context);

  //
  // Resume the watchdog.
  //
  WatchdogResume (WatchdogState);
  return;
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
  EXCEPTION_RECORD        *ExceptionRecord,
  EFI_SYSTEM_CONTEXT_X64  *Context
  )
{
  BOOLEAN             BoolRet;
  EFI_SYSTEM_CONTEXT  Temp;
  KD_STRING           Input;
  KD_STRING           Output;
  UINT64              OldRip;
  BOOLEAN             UnloadSymbols;

  //
  // Print, Prompt, Load symbols, Unload symbols, are all special
  // cases of STATUS_BREAKPOINT.
  //

  UnloadSymbols = FALSE;
  if ((ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) &&
      (ExceptionRecord->ExceptionInformation[0] != BREAKPOINT_BREAK))
  {
    //
    // Switch on the breakpoint code.
    //

    switch (ExceptionRecord->ExceptionInformation[0]) {
      //
      // Print a debug string.
      //
      // Arguments:
      //
      //     rcx - Supplies a pointer to an output string buffer.
      //     dx - Supplies the length of the output string buffer.
      //     r8d - Supplies the Id of the calling component.
      //     r9d - Supplies the output filter level.
      //

      case BREAKPOINT_PRINT:
        Output.Buffer = (UINT8 *)Context->Rcx;
        Output.Length = (UINT16)Context->Rdx;
        if (mKdDxeInitialized != FALSE) {
          BoolRet = (BOOLEAN)KdProtocolPrintString (&Output);
          if (BoolRet == FALSE) {
            Context->Rax = STATUS_SUCCESSFUL;
          } else {
            Context->Rax = STATUS_BREAKPOINT;
          }
        } else {
          Context->Rax = STATUS_DEVICE_NOT_CONNECTED;
        }

        Context->Rip += 1;
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

        Output.Buffer = (UINT8 *)Context->Rcx;
        Output.Length = (UINT16)Context->Rdx;
        Input.Buffer  = (UINT8 *)Context->R8;
        Input.Length  = (UINT16)Context->R9;

        //
        // Continue to prompt until no breakin is seen.
        //

        do {
          BoolRet = (BOOLEAN)KdProtocolPromptString (&Output, &Input);
        } while (BoolRet != FALSE);

        Context->Rax  = Input.Length;
        Context->Rip += 1;
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

        OldRip = Context->Rip;

        if (mKdDxeInitialized != FALSE) {
          Temp.SystemContextX64 = Context;
          KdProtocolReportLoadSymbolsStateChange (
            (KD_STRING *)Context->Rcx,
            (KD_SYMBOLS_INFO *)Context->Rdx,
            UnloadSymbols,
            &Temp
            );
        }

        //
        // If the kernel debugger did not update RIP, then increment
        // past the breakpoint instruction.
        //

        if (Context->Rip == OldRip) {
          Context->Rip += 1;
        }

        goto BdTrapEnd;

      //
      // Unknown command.
      //

      default:
        break;
    }
  } else {
    //
    // Report state change to the kernel debugger.
    //

    Temp.SystemContextX64 = Context;
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
  This routine initializes the KdDxe exception handling support on X64.

  @retval EFI_SUCCESS       On success.
  @retval EFI_STATUS        On failure.

**/
EFI_STATUS
KdDxeExceptionInitialize (
  )
{
  UINT8       i;
  EFI_STATUS  Status;

  //
  // Find the CPU architectural protocol and register our exception handler.
  //

  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **)&mKdDxeCpu);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, __FUNCTION__ ": Locate(gEfiCpuArchProtocolGuid) failed, Status = (%r).\n", Status));
    ASSERT_EFI_ERROR (Status);
    goto Cleanup;
  }

  for (i = 0; mExceptionTypes[i] != 0xFFFF; i += 1) {
    Status = mKdDxeCpu->RegisterInterruptHandler (
                          mKdDxeCpu,
                          mExceptionTypes[i],
                          KdDxeExceptionHandler
                          );

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, __FUNCTION__ ": RegisterInterruptHandler[%d] failed, Status = (%r).\n", mExceptionTypes[i], Status));
      ASSERT_EFI_ERROR (Status);
      mKdDxeCpu = NULL;
      goto Cleanup;
    }
  }

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
  UINT8       i;
  EFI_STATUS  Status;

  if (mKdDxeCpu != NULL) {
    for (i = 0; mExceptionTypes[i] != 0xFFFF; i += 1) {
      Status = mKdDxeCpu->RegisterInterruptHandler (
                            mKdDxeCpu,
                            mExceptionTypes[i],
                            NULL
                            );

      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, __FUNCTION__ ": RegisterInterruptHandler[%d] failed, Status = (%r).\n", mExceptionTypes[i], Status));
        ASSERT_EFI_ERROR (Status);
      }
    }
  }
}
