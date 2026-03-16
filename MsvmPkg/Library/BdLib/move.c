/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    move.c

Abstract:

    This module contains code to implement the portable kernel debugger
    memory mover.

Author:

    Mark Lucovsky (markl) 31-Aug-1990

Revision History:

--*/
#include <PiDxe.h>
#include <Guid/DebugImageInfoTable.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include "Bd.h"

// Toplevel volatile is historical and helps avoid
// the compiler transforming this to memcpy.

ULONG
BdMoveMemory (
    void volatile * volatile VoidDestination,
    void volatile const * volatile VoidSource,
    UINT32 Length
    )
/*++

Routine Description:

    This routine moves data to or from the message buffer and returns the
    actual length of the information that was moved. As data is moved, checks
    are made to ensure that the data is resident in memory and a page fault
    will not occur. If a page fault would occur, then the move is truncated.

Arguments:

    Destination  - Supplies a pointer to destination of the move operation.

    Source - Supplies a pointer to the source of the move operation.

    Length - Supplies the length of the move operation.

Return Value:

    The actual length of the move is returned as the fucntion value.

--*/
{
    PVOID BaseDestination = (PVOID)VoidDestination;
    UINT32 BytesMoved = 0;

    // Toplevel volatile is historical and helps avoid
    // the compiler transforming this to memcpy.
    UINT_PTR volatile Destination = (UINT_PTR)VoidDestination;
    UINT_PTR volatile Source = (UINT_PTR)VoidSource;

    //
    // If the length is greater than the size of the message buffer, then
    // reduce the length to the size of the message buffer.
    //

    if (Length > BD_MESSAGE_BUFFER_SIZE)
    {
        Length = BD_MESSAGE_BUFFER_SIZE;
    }

    //
    // Move the source information to the destination address.
    //
    UINT32 ActualLength = Length;

    // Copy bytes until aligned.

    while (Length > 0 && ((Source | Destination) & 3))
    {
        //
        // Only perform the move operation if it will not generate a page
        // fault.
        //
        if (!BdWriteCheck(Destination) || !BdReadCheck(Source))
        {
            break;
        }

        *(UINT8 * volatile)Destination = *(UINT8 * const volatile)Source;
        Destination += 1;
        Source += 1;
        Length -= 1;
    }

    // Copy ints while aligned.

    while (Length > 3 && 0 == ((Source | Destination) & 3))
    {
        //
        // Only perform the move operation if it will not generate a page
        // fault.
        //
        if (!BdWriteCheck(Destination) || !BdReadCheck(Source))
        {
            break;
        }

        *(UINT32 * volatile)Destination = *(UINT32 * const volatile)Source;
        Destination += 4;
        Source += 4;
        Length -= 4;
    }

    // Copy tail of bytes.

    while (Length > 0)
    {
        //
        // Only perform the move operation if it will not generate a page
        // fault.
        //
        if (!BdWriteCheck(Destination) || !BdReadCheck(Source))
        {
            break;
        }

        *(UINT8 * volatile)Destination = *(UINT8 * const volatile)Source;
        Destination += 1;
        Source += 1;
        Length -= 1;
    }

    //
    // Sweep the altered range out of the instruction cache.  This ensures
    // that future instruction fetches will never execute stale copies of code
    // patched using this routine.
    //

    BytesMoved = ActualLength - Length;

    BlArchSweepIcacheRange(BaseDestination, BytesMoved);

    return BytesMoved;
}

// Toplevel volatile is historical and helps avoid
// the compiler transforming this to memcpy.

VOID
BdCopyMemory (
    void volatile * volatile VoidDestination,
    void volatile const * volatile VoidSource,
    UINT32 Length
    )
/*++

Routine Description:

    This routine duplicates the function pf RtlCopyMemory, but is private
    to the debugger. This allows breakpoints and watch points to be set
    RtlMoveMemory itself without risk of recursive debugger entry and the
    accompanying hang.

    N.B. Unlike BdMoveMemory, this routine does NOT check for accessability
      and may fault! Use it ONLY in the debugger and ONLY where you could
      use RtlMoveMemory.

Arguments:

    Destination  - Supplies a pointer to destination of the move operation.

    Source - Supplies a pointer to the source of the move operation.

    Length - Supplies the length of the move operation.

Return Value:

    None.

--*/
{
    UINT8 volatile * volatile Destination = VoidDestination;
    UINT8 volatile const * volatile Source = VoidSource;

    while (Length > 0)
    {
        *Destination = *Source;
        Destination += 1;
        Source += 1;
        Length -= 1;
    }
}
