/**@file Cpuapi.c

This file contains support routines to handle CPU related manipulate requests
from the kernel debugger.

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
#include <Library/DebugLib.h>
#include <Library/ArmLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <KdTypes.h>
#include <Library/KdTransportLib.h>
#include <Library/BaseMemoryLib.h>
#include <KdProtocol.h>

typedef enum _DEBUG_CONTROL_SPACE_ITEM {
  DEBUG_CONTROL_SPACE_PCR,
  DEBUG_CONTROL_SPACE_PRCB,
  DEBUG_CONTROL_SPACE_KSPECIAL,
  DEBUG_CONTROL_SPACE_THREAD,
  DEBUG_CONTROL_SPACE_MAXIMUM
} DEBUG_CONTROL_SPACE_ITEM;

/**
  The function returns the ARM64 machine type.

  @retval IMAGE_FILE_MACHINE_ARM64

**/
UINT16
KdProtocolGetMachineType (
  )
{
  return IMAGE_FILE_MACHINE_ARM64;
}

/**
  The function invalidates the instruction cache for a given address.

  @param  Address      Supplies the address to be invalidated.

**/
VOID
KdProtocolInvalidateInstructionCache (
  UINT64  Address
  )
{
  //
  // Use the ArmCacheMaintenanceLib to flush the cache. Instructions are always
  // 4 bytes on AARCH64.
  //
  // TODO-cho: Technically, this does a more encompassing flush as it does a
  // DSB SY vs DSB ISH (Full system vs Inner shareable). However, it's not
  // clear if there is a performance difference as it's probably
  // chip implementation dependent.
  //
  InvalidateInstructionCacheRange ((VOID *)Address, 4);

  return;
}

/**
  The function fills in the processor-specific portions of the wait state
  change message record.

  @param  WaitStateChange    Supplies a pointer to record to fill in.
  @param  Context            Supplies a pointer to a context record.

**/
VOID
KdProtocolSetContextState (
  DBGKD_ANY_WAIT_STATE_CHANGE  *WaitStateChange,
  EFI_SYSTEM_CONTEXT           *SystemContext
  )
{
  EFI_SYSTEM_CONTEXT_AARCH64  *Context;

  Context = SystemContext->SystemContextAArch64;

  //
  // Copy special registers for ARM64
  //

  WaitStateChange->ControlReport.Cpsr = (UINT32)Context->SPSR;
  return;
}

/**
  Extract continuation control data from manipulate state message.

  @param  m         Supplies a pointer to the manipulate state packet.
  @param  Context   Supplies a pointer to a context record.

**/
VOID
KdProtocolGetStateChange (
  DBGKD_MANIPULATE_STATE64  *m,
  EFI_SYSTEM_CONTEXT        *SystemContext
  )
{
  EFI_SYSTEM_CONTEXT_AARCH64  *Context;

  Context = SystemContext->SystemContextAArch64;

  //
  // If the status of the manipulate state message was successful, then
  // extract the continuation control information.
  //

  if (m->u.Continue2.ContinueStatus >= 0) {
    //
    // Set or clear the SS flag in the CPSR of the context record.
    //

    if (m->u.Continue2.ControlSet.TraceFlag != FALSE) {
      Context->SPSR |= BIT21;
    } else {
      Context->SPSR &= ~BIT21;
    }
  }

  return;
}

/**
  Fill in the wait state change message record.

  @param  WaitStateChange    Supplies pointer to record to fill in.
  @param  ExceptionRecord    Supplies a pointer to an exception record.
  @param  Context            Supplies a pointer to a context record.

**/
VOID
KdProtocolSetStateChange (
  DBGKD_ANY_WAIT_STATE_CHANGE  *WaitStateChange,
  EXCEPTION_RECORD             *ExceptionRecord,
  EFI_SYSTEM_CONTEXT           *Context
  )
{
  KdProtocolSetContextState (WaitStateChange, Context);
  return;
}

/**
  This function reads implementation specific system data for the specified
  processor.

  @param  m                 Supplies a pointer to the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  SystemContext     Supplies the current context.

**/
VOID
KdProtocolReadControlSpace (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *SystemContext
  )
{
  DBGKD_READ_MEMORY64         *a = &m->u.ReadMemory;
  EFI_SYSTEM_CONTEXT_AARCH64  *Context;
  KD_STRING                   MessageHeader;
  UINT32                      Length;
  ARM64_SPECIAL_REGISTERS     SpecialRegisters;

  //
  // If the specified control registers are within control space, then
  // read the specified space and return a success status. Otherwise,
  // return an unsuccessful status.
  //

  Length = MIN (
             a->TransferCount,
             PACKET_MAX_SIZE - sizeof (DBGKD_MANIPULATE_STATE64)
             );

  //
  // Case on address to determine what part of Control space is being read.
  //

  switch ((UINT64)a->TargetBaseAddress) {
    case DEBUG_CONTROL_SPACE_KSPECIAL:
      ZeroMem (&SpecialRegisters, sizeof (ARM64_SPECIAL_REGISTERS));
      Context                      = SystemContext->SystemContextAArch64;
      SpecialRegisters.Elr_El1     = Context->ELR;
      SpecialRegisters.Spsr_El1    = (UINT32)Context->SPSR;
      SpecialRegisters.Tpidr_El0   = 0;
      SpecialRegisters.Tpidrro_El0 = 0;
      SpecialRegisters.Tpidr_El1   = 0;

      CopyMem (
        (UINT8 *)AdditionalData->Buffer,
        &SpecialRegisters,
        sizeof (ARM64_SPECIAL_REGISTERS)
        );

      AdditionalData->Length = sizeof (ARM64_SPECIAL_REGISTERS);
      a->ActualBytesRead     = AdditionalData->Length;
      m->ReturnStatus        = STATUS_SUCCESSFUL;
      break;

    default:

      AdditionalData->Length = 0;
      m->ReturnStatus        = STATUS_UNSUCCESSFUL;
      a->ActualBytesRead     = 0;
      break;
  }

  //
  // Send reply packet.
  //

  MessageHeader.Length = sizeof (*m);
  MessageHeader.Buffer = (UINT8 *)m;
  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &MessageHeader,
    AdditionalData
    );

  return;
}

/**
  This function writes control space.

  @param  m                 Supplies a pointer to the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  SystemContext     Supplies the current context.

**/
VOID
KdProtocolWriteControlSpace (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *SystemContext
  )
{
  DBGKD_WRITE_MEMORY64        *a = &m->u.WriteMemory;
  EFI_SYSTEM_CONTEXT_AARCH64  *Context;
  KD_STRING                   MessageHeader;
  ARM64_SPECIAL_REGISTERS     *SpecialRegisters;

  //
  // If the specified control registers are within control space, then
  // write the specified space and return a success status. Otherwise,
  // return an unsuccessful status.
  //

  switch ((UINT64)a->TargetBaseAddress) {
    case DEBUG_CONTROL_SPACE_KSPECIAL:
      SpecialRegisters = (ARM64_SPECIAL_REGISTERS *)AdditionalData->Buffer;
      Context          = SystemContext->SystemContextAArch64;

      //
      // BUGBUG: When manually breaking in and hitting go, KD resets the PC
      // to the break instruction.  Commenting out the following ELR restore
      // for now, but this may have unintended consequences.
      //

      // Context->ELR = SpecialRegisters->Elr_El1;
      Context->SPSR = SpecialRegisters->Spsr_El1;

      a->ActualBytesWritten = AdditionalData->Length;
      m->ReturnStatus       = STATUS_SUCCESSFUL;
      break;

    default:
      DEBUG ((DEBUG_ERROR, "Default hit: %p.\n", (UINT64)a->TargetBaseAddress));
      a->ActualBytesWritten = 0;
      m->ReturnStatus       = STATUS_UNSUCCESSFUL;
      break;
  }

  //
  // Send reply message.
  //

  MessageHeader.Length = sizeof (*m);
  MessageHeader.Buffer = (UINT8 *)m;
  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &MessageHeader,
    NULL
    );

  return;
}

/**
  This function reads I/O space.

  @param  m                 Supplies a pointer to the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  SystemContext     Supplies the current context.

**/
VOID
KdProtocolReadIoSpace (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  )
{
  KD_STRING  MessageHeader;

  m->ReturnStatus      = STATUS_INVALID_PARAMETER;
  MessageHeader.Length = sizeof (*m);
  MessageHeader.Buffer = (UINT8 *)m;
  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &MessageHeader,
    NULL
    );

  return;
}

/**
  This function writes I/O space.

  @param  m                 Supplies a pointer to the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  SystemContext     Supplies the current context.

**/
VOID
KdProtocolWriteIoSpace (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  )
{
  KD_STRING  MessageHeader;

  m->ReturnStatus      = STATUS_INVALID_PARAMETER;
  MessageHeader.Length = sizeof (*m);
  MessageHeader.Buffer = (UINT8 *)m;
  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &MessageHeader,
    NULL
    );

  return;
}

/**
  This function reads an MSR.

  @param  m                 Supplies a pointer to the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  SystemContext     Supplies the current context.

**/
VOID
KdProtocolReadMachineSpecificRegister (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  )
{
  KD_STRING  MessageHeader;

  m->ReturnStatus      = STATUS_INVALID_PARAMETER;
  MessageHeader.Length = sizeof (*m);
  MessageHeader.Buffer = (UINT8 *)m;
  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &MessageHeader,
    NULL
    );

  return;
}

/**
  This function writes an MSR.

  @param  m                 Supplies a pointer to the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  SystemContext     Supplies the current context.

**/
VOID
KdProtocolWriteMachineSpecificRegister (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  )
{
  KD_STRING  MessageHeader;

  m->ReturnStatus      = STATUS_INVALID_PARAMETER;
  MessageHeader.Length = sizeof (*m);
  MessageHeader.Buffer = (UINT8 *)m;
  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &MessageHeader,
    NULL
    );

  return;
}
