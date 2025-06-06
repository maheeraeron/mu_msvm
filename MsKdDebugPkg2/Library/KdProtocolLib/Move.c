/**@file Move.c

This file contains implementations for copy memory functions.

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

#include <Guid/DebugImageInfoTable.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <KdTypes.h>
#include <KdProtocol.h>

/**
  This routine moves data to or from the message buffer and returns the
  actual length of the information that was moved. As data is moved, checks
  are made to ensure that the data is resident in memory and a page fault
  will not occur. If a page fault would occur, then the move is truncated.

  N.B. It's up to the caller to make sure to invalidate the memory moved from the
  instruction cache.

  TODO-cho: It's probably better to have this function invalidate the instruction cache,
  as any instruction patching done outside of the breakpoint code will not work.

  TODO-cho-19462446: This function should also make sure the source is writable,
  by setting PTE protection appropriately, either by modifying the PTE or by using
  a separate VA mapping.

  @param  Destination   Supplies a pointer to destination of the move operation.
  @param  Source        Supplies a pointer to the source of the move operation.
  @param  Length        Supplies the length of the move operation.

  @retval UINT32        The actual length of the move is returned as the function value.

**/
UINT32
KdProtocolMoveMemory (
  volatile UINT8  *Destination,
  volatile UINT8  *Source,
  UINT32          Length
  )
{
  UINT8   *Address1;
  UINT8   *Address2;
  UINT32  ActualLength;
  UINT8   *BaseDestination;
  UINT32  BytesMoved;

  //
  // Move the source information to the destination address.
  //

  ActualLength    = Length;
  BaseDestination = (UINT8 *)Destination;
  // If either is unaligned, then perform byte by byte move until both are aligned
  while ((((UINT64)Source & 3) || ((UINT64)Destination & 3)) && (Length > 0)) {
    //
    // Only perform the move operation if it will not generate a page
    // fault.
    //

    Address1 = KdProtocolWriteCheck ((UINT8 *)Destination);
    Address2 = KdProtocolReadCheck ((UINT8 *)Source);
    if ((Address1 == NULL) || (Address2 == NULL)) {
      break;
    }

    *(UINT8 *)Destination = *(UINT8 *)Source;
    Destination          += 1;
    Source               += 1;
    Length               -= 1;
  }

  while (Length > 3) {
    // Assumes both Dest and Source are UINT32 aligned, move by UINT32
    //
    // Only perform the move operation if it will not generate a page
    // fault.
    //

    Address1 = KdProtocolWriteCheck ((UINT8 *)Destination);
    Address2 = KdProtocolReadCheck ((UINT8 *)Source);
    if ((Address1 == NULL) || (Address2 == NULL)) {
      break;
    }

    *(UINT32 *)Destination = *(UINT32 *)Source;
    Destination           += 4;
    Source                += 4;
    Length                -= 4;
  }

  while (Length > 0) {
    // Any residual from UINT32 aligned move
    //
    // Only perform the move operation if it will not generate a page
    // fault.
    //

    Address1 = KdProtocolWriteCheck ((UINT8 *)Destination);
    Address2 = KdProtocolReadCheck ((UINT8 *)Source);
    if ((Address1 == NULL) || (Address2 == NULL)) {
      break;
    }

    *(UINT8 *)Destination = *(UINT8 *)Source;
    Destination          += 1;
    Source               += 1;
    Length               -= 1;
  }

  BytesMoved = ActualLength - Length;
  return BytesMoved;
}

/**
  This routine duplicates the function pf RtlCopyMemory, but is private
  to the debugger. This allows breakpoints and watch points to be set
  RtlMoveMemory itself without risk of recursive debugger entry and the
  accompanying hang.

  N.B. Unlike KdProtocolMoveMemory, this routine does NOT check for accessibility
       and may fault! Use it ONLY in the debugger and ONLY where you could
       use RtlMoveMemory.

  @param  Destination   Supplies a pointer to destination of the move operation.
  @param  Source        Supplies a pointer to the source of the move operation.
  @param  Length        Supplies the length of the move operation.

**/
VOID
KdProtocolCopyMemory (
  volatile UINT8  *Destination,
  volatile UINT8  *Source,
  UINT32          Length
  )
{
  while (Length > 0) {
    *Destination = *Source;
    Destination += 1;
    Source      += 1;
    Length      -= 1;
  }

  return;
}
