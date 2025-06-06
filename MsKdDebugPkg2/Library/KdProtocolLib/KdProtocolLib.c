/**@file KdProtocolLib.c

This file contains the entry points into KdProtocolLib.

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

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <KdTypes.h>
#include <Library/KdTransportLib.h>
#include <KdProtocol.h>

#define NT_SUCCESS(Status)  ((Status) >= 0)

#define KD_MESSAGE_BUFFER_SIZE  4096
UINT8  mKdMessageBuffer[KD_MESSAGE_BUFFER_SIZE];

EFI_LOCK  mKdProtocolLock;

/**
  This routine performs any necessary initialization for the KdProtcolLib.

  @param  None.

  @retval EFI_SUCCESS           The protocol library successfully initialized.
  @retval EFI_OUT_OF_RESOURCES  The protocol library was unable to allocate
                                sufficient memory.

**/
EFI_STATUS
KdProtocolLibInitialize (
  VOID
  )
{
  EfiInitializeLock (&mKdProtocolLock, TPL_NOTIFY);
  return EFI_SUCCESS;
}

/**
  This function sends a packet and waits for a continue message. BreakIns
  received while waiting will always cause a resend of the packet originally
  sent out. While waiting state manipulate messages will be serviced.

  A resend always resends the original event sent to the debugger, not the
  last response to some debugger command.

  @param  OutPacketType     The type of packet to send.
  @param  OutMessageHeader  Pointer to a string that describes the message information.
  @param  OutMessageData    Pointer to a string that describes the optional message data.
  @param  Context           Exception context.

  @retval ContinueSuccess  The debugger requested to continue.
  @retval ContinueError    The debugger failed to continue.

**/
KCONTINUE_STATUS
KdProtocolSendWaitContinue (
  UINT32              OutPacketType,
  KD_STRING           *OutMessageHeader,
  KD_STRING           *OutMessageData,
  EFI_SYSTEM_CONTEXT  *Context
  )
{
  UINT32                    Length;
  KD_STRING                 MessageData;
  KD_STRING                 MessageHeader;
  DBGKD_MANIPULATE_STATE64  ManipulateState;
  UINT32                    Status;

  //
  // Loop servicing state manipulation message until a continue message
  // is received.
  //

  ZeroMem (&ManipulateState, sizeof (DBGKD_MANIPULATE_STATE64));
  MessageHeader.MaximumLength = sizeof (DBGKD_MANIPULATE_STATE64);
  MessageHeader.Buffer        = (UINT8 *)&ManipulateState;
  MessageData.MaximumLength   = KD_MESSAGE_BUFFER_SIZE;
  MessageData.Buffer          = &mKdMessageBuffer[0];

  //
  // Send event notification packet to debugger on host. Come back here
  // any time we see a breakin sequence.
  //

ResendPacket:

  KdTransportSendPacket (OutPacketType, OutMessageHeader, OutMessageData);

  if (!KdTransportIsDebuggerConnected ()) {
    return ContinueSuccess;
  }

  while (TRUE) {
    //
    // Wait for State Manipulate Packet without timeout.
    //

    do {
      Status = KdTransportReceivePacket (
                 PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 &MessageData,
                 &Length
                 );

      if (Status == KD_PACKET_RESEND) {
        goto ResendPacket;
      }
    } while (Status == KD_PACKET_TIMEOUT);

    //
    // Switch on the return message API number.
    //

    switch (ManipulateState.ApiNumber) {
      case DbgKdQueryMemoryApi:
        KdProtocolQueryMemory (&ManipulateState, &MessageData, Context);
        break;

      case DbgKdReadVirtualMemoryApi:
        KdProtocolReadVirtualMemory (&ManipulateState, &MessageData, Context);
        break;

      case DbgKdWriteVirtualMemoryApi:
        KdProtocolWriteVirtualMemory (&ManipulateState, &MessageData, Context);
        break;

      case DbgKdReadPhysicalMemoryApi:
        KdProtocolReadPhysicalMemory (&ManipulateState, &MessageData, Context);
        break;

      case DbgKdWritePhysicalMemoryApi:
        KdProtocolWritePhysicalMemory (&ManipulateState, &MessageData, Context);
        break;

      case DbgKdGetContextApi:
        KdProtocolGetContext (&ManipulateState, &MessageData, Context);
        break;

      case DbgKdSetContextApi:
        KdProtocolSetContext (&ManipulateState, &MessageData, Context);
        break;

      case DbgKdWriteBreakPointApi:
        KdProtocolWriteBreakpoint (&ManipulateState, &MessageData, Context);
        break;

      case DbgKdRestoreBreakPointApi:
        KdProtocolRestoreBreakpoint (&ManipulateState, &MessageData, Context);
        break;

      case DbgKdReadControlSpaceApi:
        KdProtocolReadControlSpace (&ManipulateState, &MessageData, Context);
        break;

      case DbgKdWriteControlSpaceApi:
        KdProtocolWriteControlSpace (&ManipulateState, &MessageData, Context);
        break;

      case DbgKdReadIoSpaceApi:
        KdProtocolReadIoSpace (&ManipulateState, &MessageData, Context);
        break;

      case DbgKdWriteIoSpaceApi:
        KdProtocolWriteIoSpace (&ManipulateState, &MessageData, Context);
        break;

      case DbgKdContinueApi:
        if (NT_SUCCESS (ManipulateState.u.Continue.ContinueStatus) != FALSE) {
          return ContinueSuccess;
        } else {
          return ContinueError;
        }

        break;

      case DbgKdContinueApi2:
        if (NT_SUCCESS (ManipulateState.u.Continue2.ContinueStatus) != FALSE) {
          KdProtocolGetStateChange (&ManipulateState, Context);
          return ContinueSuccess;
        } else {
          return ContinueError;
        }

        break;

      case DbgKdRebootApi:
        gRT->ResetSystem (EfiResetWarm, EFI_SUCCESS, 0, NULL);
        break;

      case DbgKdGetVersionApi:
        KdProtocolGetVersion (&ManipulateState);
        break;

      case DbgKdWriteBreakPointExApi:
        Status = KdProtocolWriteBreakPointEx (
                   &ManipulateState,
                   &MessageData,
                   Context
                   );

        if (Status) {
          ManipulateState.ApiNumber                 = DbgKdContinueApi;
          ManipulateState.u.Continue.ContinueStatus = Status;
          return ContinueError;
        }

        break;

      case DbgKdRestoreBreakPointExApi:
        KdProtocolRestoreBreakPointEx (&ManipulateState, &MessageData, Context);
        break;

      case DbgKdReadMachineSpecificRegister:
        KdProtocolReadMachineSpecificRegister (
          &ManipulateState,
          &MessageData,
          Context
          );

        break;

      case DbgKdWriteMachineSpecificRegister:
        KdProtocolWriteMachineSpecificRegister (
          &ManipulateState,
          &MessageData,
          Context
          );

        break;

      //
      // Prevent spurious PACKET_TYPE_KD_STATE_MANIPULATE packets with a
      // ReturnStatus of STATUS_UNSUCCESSFUL from being sent for these APIs
      // since no response is expected.  Sending the spurious response can
      // cause the host and target to get out of sync.
      //

      case DbgKdSetSpecialCallApi:
      case DbgKdClearSpecialCallsApi:
      case DbgKdSetInternalBreakPointApi:
      case DbgKdClearAllInternalBreakpointsApi:
        break;

      //
      // Invalid message.
      //

      default:
        MessageData.Length           = 0;
        ManipulateState.ReturnStatus = STATUS_UNSUCCESSFUL;
        KdTransportSendPacket (
          PACKET_TYPE_KD_STATE_MANIPULATE,
          &MessageHeader,
          &MessageData
          );

        break;
    }
  }
}

VOID
KdProtocolSetCommonState (
  UINT32                       NewState,
  EFI_SYSTEM_CONTEXT           *Context,
  DBGKD_ANY_WAIT_STATE_CHANGE  *WaitStateChange
  )
{
  UINT16  InstrCount;
  UINT8   *InstrStream;
  UINT8   *PcMemory;

  WaitStateChange->NewState         = NewState;
  WaitStateChange->ProcessorLevel   = 0;
  WaitStateChange->Processor        = 0;
  WaitStateChange->NumberProcessors = 1;
  WaitStateChange->Thread           = 0;
  PcMemory                          = (UINT8 *)(UINTN)CONTEXT_TO_PROGRAM_COUNTER (Context);
  WaitStateChange->ProgramCounter   = (UINT64)PcMemory;

  ZeroMem (
    &WaitStateChange->AnyControlReport,
    sizeof (WaitStateChange->AnyControlReport)
    );

  //
  // Copy instruction stream immediately following location of event.
  //

  InstrStream                                     = (UINT8 *)WaitStateChange->ControlReport.InstructionStream;
  InstrCount                                      = (UINT16)KdProtocolMoveMemory (InstrStream, PcMemory, DBGKD_MAXSTREAM);
  WaitStateChange->ControlReport.InstructionCount = InstrCount;

  //
  // Clear breakpoints in copied area.
  // If there were any breakpoints cleared, recopy the instruction area
  // without them.
  //

  if (KdProtocolDeleteBreakpointRange (
        (UINT64)PcMemory,
        (UINT64)PcMemory + InstrCount - 1
        ))
  {
    KdProtocolMoveMemory (InstrStream, PcMemory, InstrCount);
  }
}

/**
  This routine sends an exception state change packet to the kernel debugger and
  waits for a manipulate state return message.

  @param  ExceptionRecord   Pointer to an exception record.
  @param  Context           Pointer to a context record.
  @param  FirstChance       Indication of whether this exception should be
                            handled as a first or second chance exception.

  @retval TRUE              Exception handled successfully.
  @retval FALSE             Exception handling failed.

**/
BOOLEAN
KdProtocolReportExceptionStateChange (
  EXCEPTION_RECORD    *ExceptionRecord,
  EFI_SYSTEM_CONTEXT  *Context,
  BOOLEAN             FirstChance
  )
{
  KD_STRING                    MessageData;
  KD_STRING                    MessageHeader;
  KCONTINUE_STATUS             Status;
  DBGKD_ANY_WAIT_STATE_CHANGE  WaitStateChange;

  ZeroMem (&WaitStateChange, sizeof (WaitStateChange));

  //
  // Construct the wait state change message and message descriptor.
  //
  // N.B. All exceptions are reported as second chance exceptions since
  //      the boot environment does contain any exception handling
  //      system.
  //

  KdProtocolSetCommonState (DbgKdExceptionStateChange, Context, &WaitStateChange);
  KdProtocolMoveMemory (
    (UINT8 *)&WaitStateChange.u.Exception.ExceptionRecord,
    (UINT8 *)ExceptionRecord,
    sizeof (EXCEPTION_RECORD)
    );

  WaitStateChange.u.Exception.FirstChance = FirstChance;
  KdProtocolSetStateChange (&WaitStateChange, ExceptionRecord, Context);
  MessageHeader.Length = sizeof (WaitStateChange);
  MessageHeader.Buffer = (UINT8 *)&WaitStateChange;
  MessageData.Length   = 0;

  while (EfiAcquireLockOrFail (&mKdProtocolLock) != EFI_SUCCESS) {
  }

  //
  // Send packet to the kernel debugger on the host machine,
  // wait for answer.
  //

  Status = KdProtocolSendWaitContinue (
             PACKET_TYPE_KD_STATE_CHANGE64,
             &MessageHeader,
             &MessageData,
             Context
             );

  EfiReleaseLock (&mKdProtocolLock);

  return (BOOLEAN)Status;
}

/**
  This routine determines if a breakin packet is pending.

  @param  None.

  @retval TRUE          Breakin packet detected.
  @retval FALSE         Breakin packet not detected.

**/
BOOLEAN
KdProtocolPollBreakIn (
  VOID
  )
{
  BOOLEAN     BreakIn;
  EFI_STATUS  Status;

  BreakIn = FALSE;
  Status  = EfiAcquireLockOrFail (&mKdProtocolLock);
  if (EFI_ERROR (Status)) {
    return BreakIn;
  }

  Status = KdTransportReceivePacket (
             PACKET_TYPE_KD_POLL_BREAKIN,
             NULL,
             NULL,
             NULL
             );

  EfiReleaseLock (&mKdProtocolLock);
  if (Status == KD_PACKET_RECEIVED) {
    BreakIn = TRUE;
  }

  return BreakIn;
}

/**
  This routine sends a load symbols state change packet to the kernel debugger and
  waits for a manipulate state message.

  @param      PathName       Supplies a pointer to the pathname of the image
                             whose symbols are to be loaded.
  @param      SymbolInfo     The symbol information for the image that was
                             loaded.
  @param      UnloadSymbols  TRUE if the symbols that were previous loaded for
                             the named image are to be unloaded from the
                             debugger
  @param      ContextRecord  Pointer to the current execution context record.
**/
VOID
KdProtocolReportLoadSymbolsStateChange (
  KD_STRING           *PathName,
  KD_SYMBOLS_INFO     *SymbolInfo,
  BOOLEAN             UnloadSymbols,
  EFI_SYSTEM_CONTEXT  *ContextRecord
  )
{
  KD_STRING                    *AdditionalData;
  KD_STRING                    MessageData;
  KD_STRING                    MessageHeader;
  KCONTINUE_STATUS             Status;
  DBGKD_ANY_WAIT_STATE_CHANGE  WaitStateChange;

  if (!KdTransportIsDebuggerConnected ()) {
    return;
  }

  //
  // Construct the wait state change message and message descriptor.
  //

  KdProtocolSetCommonState (
    DbgKdLoadSymbolsStateChange,
    ContextRecord,
    &WaitStateChange
    );
  KdProtocolSetContextState (&WaitStateChange, ContextRecord);

  WaitStateChange.u.LoadSymbols.UnloadSymbols = (BOOLEAN)UnloadSymbols;
  WaitStateChange.u.LoadSymbols.BaseOfDll     = (UINT64)SymbolInfo->BaseOfDll;
  WaitStateChange.u.LoadSymbols.ProcessId     = SymbolInfo->ProcessId;
  WaitStateChange.u.LoadSymbols.CheckSum      = SymbolInfo->CheckSum;
  WaitStateChange.u.LoadSymbols.SizeOfImage   = SymbolInfo->SizeOfImage;
  if (PathName != NULL) {
    WaitStateChange.u.LoadSymbols.PathNameLength =
      KdProtocolMoveMemory (
        (UINT8 *)mKdMessageBuffer,
        (UINT8 *)PathName->Buffer,
        PathName->Length
        ) + 1;

    MessageData.Buffer                       = (UINT8 *)(&mKdMessageBuffer[0]);
    MessageData.Length                       = (UINT16)WaitStateChange.u.LoadSymbols.PathNameLength;
    MessageData.Buffer[MessageData.Length-1] = '\0';
    AdditionalData                           = &MessageData;
  } else {
    WaitStateChange.u.LoadSymbols.PathNameLength = 0;
    AdditionalData                               = NULL;
  }

  MessageHeader.Length = sizeof (WaitStateChange);
  MessageHeader.Buffer = (UINT8 *)&WaitStateChange;

  //
  // Send packet to the kernel debugger on the host machine, wait
  // for the reply.
  //

  Status = KdProtocolSendWaitContinue (
             PACKET_TYPE_KD_STATE_CHANGE64,
             &MessageHeader,
             AdditionalData,
             ContextRecord
             );
}
