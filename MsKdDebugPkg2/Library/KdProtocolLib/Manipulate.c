/**@file Manipulate.c

This file contains the support functions for processing manipulate packets.

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
#include <KdTypes.h>
#include <Library/KdTransportLib.h>
#include <KdProtocol.h>

UINT32             mNtBuildNumber = 9600 | 0xc0000000;
extern LIST_ENTRY  mModuleList;

/**
  This function is called in response of a write breakpoint state
  manipulation message.  Its function is to write a breakpoint
  and return a handle to the breakpoint.

  @param  m                 Supplies the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  Context           Supplies the current context.

**/
VOID
KdProtocolWriteBreakpoint (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  )
{
  DBGKD_WRITE_BREAKPOINT64  *a = &m->u.WriteBreakPoint;
  KD_STRING                 messageHeader;

  a->BreakPointHandle = KdProtocolAddBreakpoint (a->BreakPointAddress);
  if (a->BreakPointHandle != 0) {
    m->ReturnStatus = STATUS_SUCCESSFUL;
  } else {
    m->ReturnStatus = STATUS_UNSUCCESSFUL;
  }

  //
  // Send reply packet.
  //

  messageHeader.Length = sizeof (*m);
  messageHeader.Buffer = (UINT8 *)m;
  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &messageHeader,
    NULL
    );

  return;
}

/**
  This function is called in response of a restore breakpoint state
  manipulation message.  Its function is to restore a breakpoint
  using the specified handle.

  @param  m                 Supplies the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  Context           Supplies the current context.

**/
VOID
KdProtocolRestoreBreakpoint (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  )
{
  DBGKD_RESTORE_BREAKPOINT  *a = &m->u.RestoreBreakPoint;
  KD_STRING                 messageHeader;

  if (KdProtocolDeleteBreakpoint (a->BreakPointHandle)) {
    m->ReturnStatus = STATUS_SUCCESSFUL;
  } else {
    m->ReturnStatus = STATUS_UNSUCCESSFUL;
  }

  //
  // Send reply packet.
  //

  messageHeader.Length = sizeof (*m);
  messageHeader.Buffer = (UINT8 *)m;
  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &messageHeader,
    NULL
    );
}

/**
  This function is called in response of a write breakpoint state 'ex'
  manipulation message.  Its function is to clear breakpoints, write
  new breakpoints, and continue the target system.  The clearing of
  breakpoints is conditional based on the presence of breakpoint handles.
  The setting of breakpoints is conditional based on the presence of
  valid, non-zero, addresses.  The continuing of the target system
  is conditional based on a non-zero continue status.

  This api allows a debugger to clear breakpoints, add new breakpoint,
  and continue the target system all in one api packet.  This reduces the
  amount of traffic across the wire and greatly improves source stepping.

  @param  m                 Supplies the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  Context           Supplies the current context.

**/
UINT32
KdProtocolWriteBreakPointEx (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  )
{
  DBGKD_BREAKPOINTEX        *a = &m->u.BreakPointEx;
  DBGKD_WRITE_BREAKPOINT64  *b;
  UINT32                    i;
  KD_STRING                 messageHeader;
  DBGKD_WRITE_BREAKPOINT64  bpBuf[BREAKPOINT_TABLE_SIZE];
  UINT32                    breakPointCount;

  messageHeader.Length = sizeof (*m);
  messageHeader.Buffer = (UINT8 *)m;

  //
  // verify that the packet size is correct
  //

  if (AdditionalData->Length != a->BreakPointCount * sizeof (DBGKD_WRITE_BREAKPOINT64)) {
    m->ReturnStatus = STATUS_UNSUCCESSFUL;
    KdTransportSendPacket (
      PACKET_TYPE_KD_STATE_MANIPULATE,
      &messageHeader,
      AdditionalData
      );

    return STATUS_UNSUCCESSFUL;
  }

  breakPointCount = a->BreakPointCount;
  if (breakPointCount > BREAKPOINT_TABLE_SIZE) {
    breakPointCount = BREAKPOINT_TABLE_SIZE;
  }

  i = KdProtocolMoveMemory (
        (UINT8 *)bpBuf,
        (UINT8 *)AdditionalData->Buffer,
        breakPointCount * sizeof (DBGKD_WRITE_BREAKPOINT64)
        );

  //
  // assume success
  //

  m->ReturnStatus = STATUS_SUCCESSFUL;

  //
  // loop thru the breakpoint handles passed in from the debugger and
  // clear any breakpoint that has a non-zero handle
  //

  b = bpBuf;
  for (i = 0; i < breakPointCount; i += 1, b += 1) {
    if (b->BreakPointHandle) {
      if (!KdProtocolDeleteBreakpoint (b->BreakPointHandle)) {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
      }

      b->BreakPointHandle = 0;
    }
  }

  //
  // loop thru the breakpoint addesses passed in from the debugger and
  // add any new breakpoints that have a non-zero address
  //

  b = bpBuf;
  for (i = 0; i < breakPointCount; i += 1, b += 1) {
    if (b->BreakPointAddress) {
      b->BreakPointHandle = KdProtocolAddBreakpoint (b->BreakPointAddress);
      if (!b->BreakPointHandle) {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
      }
    }
  }

  //
  // send back our response
  //

  KdProtocolMoveMemory (
    (UINT8 *)AdditionalData->Buffer,
    (UINT8 *)bpBuf,
    breakPointCount * sizeof (DBGKD_WRITE_BREAKPOINT64)
    );

  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &messageHeader,
    AdditionalData
    );

  //
  // return the caller's continue status value.  if this is a non-zero
  // value the system is continued using this value as the continuestatus.
  //

  return a->ContinueStatus;
}

/**
  This function is called in response of a restore breakpoint state 'ex'
  manipulation message.  Its function is to clear a list of breakpoints.

  @param  m                 Supplies the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  Context           Supplies the current context.

**/
VOID
KdProtocolRestoreBreakPointEx (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  )
{
  DBGKD_BREAKPOINTEX        *a = &m->u.BreakPointEx;
  DBGKD_RESTORE_BREAKPOINT  *b;
  KD_STRING                 messageHeader;
  UINT32                    i;
  DBGKD_RESTORE_BREAKPOINT  bpBuf[BREAKPOINT_TABLE_SIZE];
  UINT32                    breakPointCount;

  messageHeader.Length = sizeof (*m);
  messageHeader.Buffer = (UINT8 *)m;

  //
  // verify that the packet size is correct
  //

  if (AdditionalData->Length !=
      a->BreakPointCount * sizeof (DBGKD_RESTORE_BREAKPOINT))
  {
    m->ReturnStatus = STATUS_UNSUCCESSFUL;
    KdTransportSendPacket (
      PACKET_TYPE_KD_STATE_MANIPULATE,
      &messageHeader,
      AdditionalData
      );

    return;
  }

  breakPointCount = a->BreakPointCount;
  if (breakPointCount > BREAKPOINT_TABLE_SIZE) {
    breakPointCount = BREAKPOINT_TABLE_SIZE;
  }

  KdProtocolMoveMemory (
    (UINT8 *)bpBuf,
    (UINT8 *)AdditionalData->Buffer,
    breakPointCount * sizeof (DBGKD_RESTORE_BREAKPOINT)
    );

  //
  // Assume success
  //

  m->ReturnStatus = STATUS_SUCCESSFUL;

  //
  // Loop through the breakpoint handles passed in from the debugger and
  // clear any breakpoint that has a non-zero handle
  //

  b = bpBuf;
  for (i = 0; i < breakPointCount; i += 1, b += 1) {
    if (!KdProtocolDeleteBreakpoint (b->BreakPointHandle)) {
      m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }
  }

  //
  // send back our response
  //

  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &messageHeader,
    AdditionalData
    );

  return;
}

/**
  This function is called in response of a set context state
  manipulation message.  Its function is set the current
  context.

  @param  m                 Supplies the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  Context           Supplies the current context.

**/
VOID
KdProtocolSetContext (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  )
{
  KD_STRING  messageHeader;

  m->ReturnStatus = STATUS_SUCCESSFUL;
  // KdProtocolCopyMemory((UINT8 *)Context, AdditionalData->Buffer, sizeof(EFI_SYSTEM_CONTEXT));

  KdProtocolKdContextToContext ((KD_CONTEXT *)AdditionalData->Buffer, Context);

  //
  // Send reply packet.
  //

  messageHeader.Length = sizeof (*m);
  messageHeader.Buffer = (UINT8 *)m;
  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &messageHeader,
    NULL
    );
}

/**
  This function is called in response to a get context state
  manipulation message.  Its function is to return the current
  context.

  @param  m                 Supplies the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  Context           Supplies the current context.

**/
VOID
KdProtocolGetContext (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  )
{
  KD_CONTEXT  KdContext;
  KD_STRING   messageHeader;

  m->ReturnStatus = STATUS_SUCCESSFUL;
  KdProtocolContextToKdContext (Context, &KdContext);
  AdditionalData->Length = sizeof (KD_CONTEXT);
  KdProtocolCopyMemory (AdditionalData->Buffer, (UINT8 *)&KdContext, sizeof (KD_CONTEXT));

  //
  // Send reply packet.
  //

  messageHeader.Length = sizeof (*m);
  messageHeader.Buffer = (UINT8 *)m;
  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &messageHeader,
    AdditionalData
    );

  return;
}

/**
  This function is called in response to a query memory manipulation message.
  Its function is to query the memory type and return.

  @param  m                 Supplies the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  Context           Supplies the current context.

**/
VOID
KdProtocolQueryMemory (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  )
{
  KD_STRING  messageHeader;

  if (m->u.QueryMemory.AddressSpace == DBGKD_QUERY_MEMORY_VIRTUAL) {
    //
    // We do not support the notion of user-mode, so this will
    // always be kernel memory.
    //

    m->u.QueryMemory.AddressSpace = DBGKD_QUERY_MEMORY_KERNEL;

    //
    // Always return the most permissive flags.
    //

    m->u.QueryMemory.Flags =
      DBGKD_QUERY_MEMORY_READ |
      DBGKD_QUERY_MEMORY_WRITE |
      DBGKD_QUERY_MEMORY_EXECUTE;

    m->ReturnStatus = STATUS_SUCCESSFUL;
  } else {
    m->ReturnStatus = STATUS_UNSUCCESSFUL;
  }

  //
  // Send reply packet.
  //

  messageHeader.Length = sizeof (*m);
  messageHeader.Buffer = (UINT8 *)m;
  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &messageHeader,
    AdditionalData
    );

  return;
}

/**
  This function is called in response to a read virtual memory 32-bit
  state manipulation message. Its function is to read virtual memory
  and return.

  @param  m                 Supplies the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  Context           Supplies the current context.

**/
VOID
KdProtocolReadVirtualMemory (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  )
{
  DBGKD_READ_MEMORY64  *a = &m->u.ReadMemory;
  UINT8                *destination;
  UINT32               length;
  KD_STRING            messageHeader;
  UINT8                *source;

  //
  // Trim transfer count to fit in a single message.
  //

  length = MIN (
             a->TransferCount,
             PACKET_MAX_SIZE - sizeof (DBGKD_MANIPULATE_STATE64)
             );

  source      = (UINT8 *)a->TargetBaseAddress;
  destination = (UINT8 *)AdditionalData->Buffer;

  AdditionalData->Length = (UINT16)KdProtocolMoveMemory (
                                     destination,
                                     source,
                                     (UINT16)length
                                     );

  if (length == AdditionalData->Length) {
    m->ReturnStatus = STATUS_SUCCESSFUL;
  } else {
    m->ReturnStatus = STATUS_UNSUCCESSFUL;
  }

  a->ActualBytesRead = AdditionalData->Length;

  //
  // Send reply packet.
  //

  messageHeader.Length = sizeof (*m);
  messageHeader.Buffer = (UINT8 *)m;
  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &messageHeader,
    AdditionalData
    );

  return;
}

/**
  This function is called in response to a read physical memory
  state manipulation message. Its function is to read physical memory
  and return.

  @param  m                 Supplies the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  Context           Supplies the current context.

**/
VOID
KdProtocolReadPhysicalMemory (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  )
{
  DBGKD_READ_MEMORY64  *a = &m->u.ReadMemory;
  UINT16               bytesLeft;
  UINT8                *destination;
  UINT32               length;
  KD_STRING            messageHeader;
  UINT16               numberBytes;
  UINT64               source;
  UINT8                *virtualAddress;

  //
  // Trim transfer count to fit in a single message.
  //

  length = MIN (
             a->TransferCount,
             PACKET_MAX_SIZE - sizeof (DBGKD_MANIPULATE_STATE64)
             );

  //
  // Since the EfiKdTranslatePhysicalAddress only maps in one physical
  // page at a time, we need to break the memory move up into smaller
  // moves which don't cross page boundaries.  There are two cases we
  // need to deal with.  The area to be moved may start and end on the
  // same page, or it may start and end on different pages (with an
  // arbitrary number of pages in between)
  //

  source      = a->TargetBaseAddress;
  destination = (UINT8 *)AdditionalData->Buffer;
  bytesLeft   = (UINT16)length;
  if (PAGE_ALIGN (source) == PAGE_ALIGN (source + bytesLeft)) {
    //
    // Memory move starts and ends on the same page.
    //

    virtualAddress = KdProtocolTranslatePhysicalAddress (source);
    if (virtualAddress == NULL) {
      AdditionalData->Length = 0;
    } else {
      AdditionalData->Length = (UINT16)KdProtocolMoveMemory (
                                         destination,
                                         virtualAddress,
                                         bytesLeft
                                         );

      bytesLeft = bytesLeft - AdditionalData->Length;
      KdProtocolUnmapVirtualAddress (virtualAddress);
    }
  } else {
    //
    // Memory move spans page boundaries
    //

    virtualAddress = KdProtocolTranslatePhysicalAddress (source);
    if (virtualAddress == NULL) {
      AdditionalData->Length = 0;
    } else {
      numberBytes            = (UINT16)(EFI_PAGE_SIZE - BYTE_OFFSET (virtualAddress));
      AdditionalData->Length = (UINT16)KdProtocolMoveMemory (
                                         destination,
                                         virtualAddress,
                                         numberBytes
                                         );

      KdProtocolUnmapVirtualAddress (virtualAddress);
      source      += numberBytes;
      destination += numberBytes;
      bytesLeft    = bytesLeft - numberBytes;
      while (bytesLeft > 0) {
        //
        // Transfer a full page or the last bit,
        // whichever is smaller.
        //

        virtualAddress = KdProtocolTranslatePhysicalAddress (source);
        if (virtualAddress == NULL) {
          break;
        } else {
          numberBytes            = (UINT16)((EFI_PAGE_SIZE < bytesLeft) ? EFI_PAGE_SIZE : bytesLeft);
          AdditionalData->Length = AdditionalData->Length +
                                   (UINT16)KdProtocolMoveMemory (
                                             destination,
                                             virtualAddress,
                                             numberBytes
                                             );

          KdProtocolUnmapVirtualAddress (virtualAddress);
          source      += numberBytes;
          destination += numberBytes;
          bytesLeft    = bytesLeft - numberBytes;
        }
      }
    }
  }

  if (length == AdditionalData->Length) {
    m->ReturnStatus = STATUS_SUCCESSFUL;
  } else {
    m->ReturnStatus = STATUS_UNSUCCESSFUL;
  }

  a->ActualBytesRead = AdditionalData->Length;

  //
  // Send reply packet.
  //

  messageHeader.Length = sizeof (*m);
  messageHeader.Buffer = (UINT8 *)m;
  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &messageHeader,
    AdditionalData
    );

  return;
}

/**
  This function is called in response of a write virtual memory 32-bit
  state manipulation message. Its function is to write virtual memory
  and return.


  @param  m                 Supplies the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  Context           Supplies the current context.

**/
VOID
KdProtocolWriteVirtualMemory (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  )
{
  UINT32     length;
  KD_STRING  messageHeader;

  //
  // Move the data to the destination buffer.
  //

  length = KdProtocolMoveMemory (
             (UINT8 *)m->u.WriteMemory.TargetBaseAddress,
             (UINT8 *)AdditionalData->Buffer,
             AdditionalData->Length
             );

  //
  // If all the data is written, then return a success status. Otherwise,
  // return an unsuccessful status.
  //

  m->ReturnStatus = STATUS_SUCCESSFUL;
  if (length != AdditionalData->Length) {
    m->ReturnStatus = STATUS_UNSUCCESSFUL;
  }

  //
  // Set the actual number of bytes written, initialize the message header,
  // and send the reply packet to the host debugger.
  //

  m->u.WriteMemory.ActualBytesWritten = length;
  messageHeader.Length                = sizeof (DBGKD_MANIPULATE_STATE64);
  messageHeader.Buffer                = (UINT8 *)m;
  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &messageHeader,
    NULL
    );

  return;
}

/**
  This function is called in response to a write physical memory
  state manipulation message. Its function is to write physical memory
  and return.

  @param  m                 Supplies the state manipulation message.
  @param  AdditionalData    Supplies any additional data for the message.
  @param  Context           Supplies the current context.

**/
VOID
KdProtocolWritePhysicalMemory (
  DBGKD_MANIPULATE_STATE64  *m,
  KD_STRING                 *AdditionalData,
  EFI_SYSTEM_CONTEXT        *Context
  )
{
  DBGKD_WRITE_MEMORY64  *a = &m->u.WriteMemory;
  UINT16                bytesLeft;
  UINT64                destination;
  UINT32                Length;
  KD_STRING             messageHeader;
  UINT16                numberBytes;
  UINT8                 *source;
  UINT8                 *virtualAddress;

  messageHeader.Length = sizeof (*m);
  messageHeader.Buffer = (UINT8 *)m;

  //
  // Since the EfiKdTranslatePhysicalAddress only maps in one physical
  // page at a time, we need to break the memory move up into smaller
  // moves which don't cross page boundaries.  There are two cases we
  // need to deal with.  The area to be moved may start and end on the
  // same page, or it may start and end on different pages (with an
  // arbitrary number of pages in between)
  //

  destination = a->TargetBaseAddress;
  source      = (UINT8 *)AdditionalData->Buffer;
  bytesLeft   = (UINT16)a->TransferCount;
  if (PAGE_ALIGN (destination) == PAGE_ALIGN (destination + bytesLeft)) {
    //
    // Memory move starts and ends on the same page.
    //

    virtualAddress = KdProtocolTranslatePhysicalAddress (destination);
    if (virtualAddress == NULL) {
      Length = 0;
    } else {
      Length = (UINT16)KdProtocolMoveMemory (
                         virtualAddress,
                         source,
                         bytesLeft
                         );

      KdProtocolUnmapVirtualAddress (virtualAddress);
      bytesLeft = bytesLeft - (UINT16)Length;
    }
  } else {
    //
    // Memory move spans page boundaries
    //

    virtualAddress = KdProtocolTranslatePhysicalAddress (destination);
    if (virtualAddress == NULL) {
      Length = 0;
    } else {
      numberBytes = (UINT16)(EFI_PAGE_SIZE - BYTE_OFFSET (virtualAddress));
      Length      = (UINT16)KdProtocolMoveMemory (
                              virtualAddress,
                              source,
                              numberBytes
                              );

      KdProtocolUnmapVirtualAddress (virtualAddress);
      source      += numberBytes;
      destination += numberBytes;
      bytesLeft    = bytesLeft - numberBytes;
      while (bytesLeft > 0) {
        //
        // Transfer a full page or the last bit, whichever is smaller.
        //

        virtualAddress = KdProtocolTranslatePhysicalAddress (destination);
        if (virtualAddress == NULL) {
          break;
        }

        numberBytes = (UINT16)((EFI_PAGE_SIZE < bytesLeft) ? EFI_PAGE_SIZE : bytesLeft);
        Length     += (UINT16)KdProtocolMoveMemory (
                                virtualAddress,
                                source,
                                numberBytes
                                );

        KdProtocolUnmapVirtualAddress (virtualAddress);
        source      += numberBytes;
        destination += numberBytes;
        bytesLeft    = bytesLeft - numberBytes;
      }
    }
  }

  if (Length == AdditionalData->Length) {
    m->ReturnStatus = STATUS_SUCCESSFUL;
  } else {
    m->ReturnStatus = STATUS_UNSUCCESSFUL;
  }

  a->ActualBytesWritten = Length;
  KdTransportSendPacket (
    PACKET_TYPE_KD_STATE_MANIPULATE,
    &messageHeader,
    NULL
    );

  return;
}

/**
  This function returns to the caller a general information packet
  that contains useful information to a debugger.  This packet is also
  used for a debugger to determine if the writebreakpointex and
  readbreakpointex apis are available.

  @param  m    Supplies the state manipulation message.

**/
VOID
KdProtocolGetVersion (
  DBGKD_MANIPULATE_STATE64  *m
  )
{
  KD_STRING  messageHeader;

  messageHeader.Length = sizeof (*m);
  messageHeader.Buffer = (UINT8 *)m;
  ZeroMem (&m->u.GetVersion64, sizeof (m->u.GetVersion64));

  //
  // the current build number
  //
  // - 4 - tells the debugger this is a "special" OS - the boot loader.
  // The boot loader has a lot of special cases associated with it, like
  // the lack of the DebuggerDataBlock, lack of ntoskrnl, etc ...
  //

  m->u.GetVersion64.MinorVersion =  (INT16)mNtBuildNumber;
  m->u.GetVersion64.MajorVersion = 0x400 |
                                   (INT16)((mNtBuildNumber >> 28) & 0xFFFFFFF);

  //
  // Kd protocol version number.
  //

  m->u.GetVersion64.ProtocolVersion    = DBGKD_64BIT_PROTOCOL_VERSION2;
  m->u.GetVersion64.KdSecondaryVersion = CURRENT_KD_SECONDARY_VERSION;
  m->u.GetVersion64.Flags              = DBGKD_VERS_FLAG_PTR64 | DBGKD_VERS_FLAG_DATA;
  m->u.GetVersion64.MachineType        = KdProtocolGetMachineType ();
  m->u.GetVersion64.MaxPacketType      = (PACKET_TYPE_KD_FILE_IO + 1) & 0xFF;
  m->u.GetVersion64.MaxStateChange     = (UINT8)(DbgKdMaximumStateChange - DbgKdMinimumStateChange);
  m->u.GetVersion64.MaxManipulate      = (UINT8)(DbgKdMaximumManipulate - DbgKdMinimumManipulate);

  //
  // Set the address of a "module list" containing the current module.
  // Use the current module base as the "kernel base" so that the debugger
  // does not complain about the lack of a kernel.
  //

  m->u.GetVersion64.PsLoadedModuleList = (UINT64)&mModuleList;
  m->u.GetVersion64.KernBase           = 0;
  m->u.GetVersion64.DebuggerDataList   = 0; // BUGBUG: (UINT64)(UINT8 *)&KdContext->EfiKdDebuggerDataListHead;

  //
  // the usual stuff
  //

  m->ReturnStatus = STATUS_SUCCESSFUL;

  KdTransportSendPacket (PACKET_TYPE_KD_STATE_MANIPULATE, &messageHeader, NULL);
  return;
}
