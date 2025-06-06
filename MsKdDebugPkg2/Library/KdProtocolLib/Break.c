/**@file Break.c

This file contains support for software breakpoints.

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
#include <Protocol/Cpu.h>
#include <KdTypes.h>
#include <KdProtocol.h>

#define BREAKPOINT_TABLE_SIZE  32       // max number supported by kernel

//
// Flags:
//

#define BREAKPOINT_IN_USE         0x1
#define BREAKPOINT_NEEDS_WRITE    0x2
#define BREAKPOINT_SUSPENDED      0x4
#define BREAKPOINT_NEEDS_REPLACE  0x8

typedef struct _BREAKPOINT_ENTRY {
  UINT64             Address;
  BREAKPOINT_TYPE    Content;
  UINT32             Flags;
} BREAKPOINT_ENTRY;

BREAKPOINT_ENTRY  mBreakpointTable[BREAKPOINT_TABLE_SIZE];
BREAKPOINT_TYPE   mBreakpointInstruction = BREAKPOINT_VALUE;

UINT32
KdProtocolWriteContent (
  IN UINT32  Index
  );

/**

  This routine adds an entry to the breakpoint table and returns a handle
  to the breakpoint table entry.

  @param[in] Address  Supplies the address where to set the breakpoint.

  @retval  A value of zero is returned if the specified address is already in
           the breakpoint table, there are no free entries in the breakpoint
           table, the specified address is not correctly aligned, or the
           specified address is not valid. Otherwise, the index of the assigned
           breakpoint table entry plus one is returned as the function value.

**/

extern EFI_CPU_ARCH_PROTOCOL  *mKdDxeCpu;

UINT32
KdProtocolAddBreakpoint (
  IN UINT64  Address
  )
{
  BOOLEAN          Accessible;
  BREAKPOINT_TYPE  Content;
  UINT32           Handle;
  UINT32           Index;
  EFI_STATUS       Status;

  //
  // Get the instruction to be replaced. If the instruction cannot be read,
  // then mark the breakpoint as not accessible.
  //
  if (KdProtocolMoveMemory (
        (UINT8 *)&Content,
        (UINT8 *)Address,
        sizeof (BREAKPOINT_TYPE)
        ) !=
      sizeof (BREAKPOINT_TYPE))
  {
    Accessible = FALSE;
  } else {
    Accessible = TRUE;
  }

  //
  // If the specified address is not write accessible, then return zero.
  //
  if ((Accessible != FALSE) &&
      (KdProtocolWriteCheck ((UINT8 *)Address) == NULL))
  {
    Handle = 0;
    goto Cleanup;
  }

  //
  // Search the breakpoint table to see if this address already appears.
  //

  for (Index = 0; Index < BREAKPOINT_TABLE_SIZE; Index += 1) {
    if (mBreakpointTable[Index].Address == Address) {
      break;
    }
  }

  //
  // If the address doesn't already appear, use the first empty slot.
  //

  if (Index == BREAKPOINT_TABLE_SIZE) {
    for (Index = 0; Index < BREAKPOINT_TABLE_SIZE; Index += 1) {
      if (mBreakpointTable[Index].Flags == 0) {
        break;
      }
    }
  }

  //
  // If a free entry was found, then write breakpoint and return the handle
  // value plus one. Otherwise, return zero.
  //
  if (Index == BREAKPOINT_TABLE_SIZE) {
    Handle = 0;
    goto Cleanup;
  }

  //
  // Make the associated code page writable, so the instruction can be patched.
  //
  // TODO-cho-19462446: Rethink how page protections are handled on memory
  // accesses.
  //
  // For example AARCH64 has a bit in SCTLR_EL1 WXN which forces all pages that
  // are writeable to also be no execute, even if that protection isn't set in
  // the pagetable entry. Currently, no platforms set this bit in UEFI EL1, but
  // future AARCH64 platforms may.
  //
  Status = mKdDxeCpu->SetMemoryAttributes (
                        mKdDxeCpu,
                        (UINT64)Address & 0xFFFFFFFFFFFFF000,
                        0x1000,
                        EFI_MEMORY_WB
                        );

  if (EFI_ERROR (Status)) {
    CpuDeadLoop ();
  }

  if (Accessible != FALSE) {
    mBreakpointTable[Index].Address = Address;
    mBreakpointTable[Index].Content = Content;
    mBreakpointTable[Index].Flags   = BREAKPOINT_IN_USE;

    KdProtocolMoveMemory (
      (UINT8 *)Address,
      (UINT8 *)&mBreakpointInstruction,
      sizeof (BREAKPOINT_TYPE)
      );

    KdProtocolInvalidateInstructionCache (Address);
  } else {
    mBreakpointTable[Index].Address = Address;
    mBreakpointTable[Index].Flags   = (BREAKPOINT_IN_USE | BREAKPOINT_NEEDS_WRITE);
  }

  Handle = Index + 1;

Cleanup:
  return Handle;
}

/**

  This routine attempts to replace the code that a breakpoint is written over.
  This routine, KdProtocolAddBreakpoint, KdProtocolRestoreBreakpoint and
  KdSetBreakpoints are responsible for getting data written as requested.
  Callers should not examine or use Breakpoints, and they should not
  set the NEEDS_WRITE or NEEDS_REPLACE flags.

  Callers must still look at the return value from this function, however: if it
  returns FALSE, the breakpoint record must not be reused until
  KdSetOwedBreakpoints has finished with it.

  @param[in]  Index  Supplies the index of the breakpoint table entry which is
                     to be deleted.

  @retval Returns TRUE if the breakpoint was removed, FALSE if it was deferred.

**/
UINT32
KdProtocolWriteContent (
  IN UINT32  Index
  )
{
  UINT32      BytesMoved;
  UINT32      Result;
  EFI_STATUS  Status;

  //
  // If the breakpoint was never written out, just clear the flag.
  //
  if ((mBreakpointTable[Index].Flags & BREAKPOINT_NEEDS_WRITE) != 0) {
    mBreakpointTable[Index].Flags &= ~BREAKPOINT_NEEDS_WRITE;
    Result                         = TRUE;
    goto Cleanup;
  }

  //
  // Make the associated code page writable, so the instruction can be patched.
  //
  // TODO-cho-19462446: We should rethink about how protections are done.
  // Ideally, we should be restoring the page protection after patching, and
  // probably its better for that to be done in KdProtocolMoveMemory.
  //
  // Or, KdProtocolMoveMemory should be using what other debug stubs do, and use
  // a separate VA mapping in order to not modify exiting PTEs.
  //
  Status = mKdDxeCpu->SetMemoryAttributes (
                        mKdDxeCpu,
                        (UINT64)mBreakpointTable[Index].Address & 0xFFFFFFFFFFFFF000,
                        0x1000,
                        EFI_MEMORY_WB
                        );

  if (EFI_ERROR (Status)) {
    CpuDeadLoop ();
  }

  //
  // Write the context to the address.
  //
  BytesMoved =
    KdProtocolMoveMemory (
      (UINT8 *)mBreakpointTable[Index].Address,
      (UINT8 *)&mBreakpointTable[Index].Content,
      sizeof (BREAKPOINT_TYPE)
      );

  //
  // Tell the caller if the content was written to the address.
  //
  if (BytesMoved != sizeof (BREAKPOINT_TYPE)) {
    Result                         = FALSE;
    mBreakpointTable[Index].Flags |= BREAKPOINT_NEEDS_REPLACE;
  } else {
    KdProtocolInvalidateInstructionCache (mBreakpointTable[Index].Address);
    Result = TRUE;
  }

Cleanup:
  return Result;
}

/**

  This routine deletes an entry from the breakpoint table.

  @param[in] Handle  Supplies the index plus one of the breakpoint table entry
                     which is to be deleted.

  @retval A value of FALSE is returned if the specified handle is not a valid
          value or the breakpoint cannot be deleted because the old instruction
          cannot be replaced. Otherwise, a value of TRUE is returned.

**/
UINT32
KdProtocolDeleteBreakpoint (
  IN UINT32  Handle
  )
{
  UINT32  Index;
  UINT32  Result;

  //
  // If the specified handle is not valid, then return FALSE.
  //
  if ((Handle == 0) || (Handle > BREAKPOINT_TABLE_SIZE)) {
    Result = FALSE;
    goto Cleanup;
  }

  //
  // If the specified breakpoint table entry is not valid, then return FALSE.
  //

  Index = Handle - 1;
  if (mBreakpointTable[Index].Flags == 0) {
    Result = FALSE;
    goto Cleanup;
  }

  //
  // If the breakpoint is already suspended, just delete it from the table.
  //
  if ((mBreakpointTable[Index].Flags & BREAKPOINT_SUSPENDED) != 0) {
    if ((mBreakpointTable[Index].Flags & BREAKPOINT_NEEDS_REPLACE) == 0) {
      mBreakpointTable[Index].Flags = 0;
      Result                        = TRUE;
      goto Cleanup;
    }
  }

  //
  // Replace the instruction contents.
  //
  if (KdProtocolWriteContent (Index) != 0) {
    mBreakpointTable[Index].Flags = 0;
  }

  Result = TRUE;

Cleanup:
  return Result;
}

/**

  This routine deletes all breakpoints falling in a given range from the
  breakpoint table.

  @param[in] Lower   Supplies inclusive lower address of range from which to
                     remove BPs.

  @param[in] Upper   Supplies inclusive upper address of range from which to
                     remove BPs.

  @retval  TRUE if any breakpoints removed, FALSE otherwise.

**/
UINT32
KdProtocolDeleteBreakpointRange (
  IN UINT64  Lower,
  IN UINT64  Upper
  )
{
  UINT32  Index;
  UINT32  Status;

  Status = FALSE;

  //
  // Examine each entry in the table in turn.
  //
  for (Index = 0; Index < BREAKPOINT_TABLE_SIZE; Index += 1) {
    if (((mBreakpointTable[Index].Flags & BREAKPOINT_IN_USE) != 0) &&
        ((mBreakpointTable[Index].Address >= Lower) &&
         (mBreakpointTable[Index].Address <= Upper)))
    {
      Status = KdProtocolDeleteBreakpoint (Index + 1);
    }
  }

  return Status;
}

/**

  This routine suspends the supplied breakpoint.

  @param[in] Handle  Supplies the handle of the breakpoint to suspend.

**/
VOID
KdProtocolSuspendBreakpoint (
  IN UINT32  Handle
  )
{
  UINT32  Index;

  //
  // If the breakpoint is in use and it is not already suspended, suspend it
  // now.
  //
  Index = Handle - 1;
  if (((mBreakpointTable[Index].Flags & BREAKPOINT_IN_USE) != 0) &&
      ((mBreakpointTable[Index].Flags & BREAKPOINT_SUSPENDED) == 0))
  {
    mBreakpointTable[Index].Flags |= BREAKPOINT_SUSPENDED;
    KdProtocolWriteContent (Index);
  }

  return;
}

/**

  This routine suspends all breakpoint that are in use.

**/
VOID
KdProtocolSuspendAllBreakpoints (
  VOID
  )
{
  UINT32  Handle;

  for (Handle = 1; Handle <= BREAKPOINT_TABLE_SIZE; Handle += 1) {
    KdProtocolSuspendBreakpoint (Handle);
  }

  return;
}
