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

ULONG
BdMoveMemory (
    __out_bcount_part(Length, return) volatile PCHAR Destination,
    __in_bcount(Length) volatile PCHAR Source,
    __in UINT32 Length
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
    PVOID Address1;
    PVOID Address2;
    UINT32 ActualLength;
    PVOID BaseDestination;
    UINT32 BytesMoved;

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

    ActualLength = Length;
    BaseDestination = Destination;
    while (((UINT_PTR)Source & 3) && (Length > 0))
    {
        //
        // Only perform the move operation if it will not generate a page
        // fault.
        //

        Address1 = BdWriteCheck((PVOID)Destination);
        Address2 = BdReadCheck((PVOID)Source);
        if ((Address1 == NULL) || (Address2 == NULL))
        {
            break;
        }

        *(PCHAR)Destination = *(PCHAR)Source;
        Destination += 1;
        Source += 1;
        Length -= 1;
    }

    while (Length > 3)
    {
        //
        // Only perform the move operation if it will not generate a page
        // fault.
        //

        Address1 = BdWriteCheck((PVOID)Destination);
        Address2 = BdReadCheck((PVOID)Source);
        if ((Address1 == NULL) || (Address2 == NULL))
        {
            break;
        }

        *(UINT32 UNALIGNED *)Destination = *(PUINT32)Source;
        Destination += 4;
        Source += 4;
        Length -= 4;
    }

    while (Length > 0)
    {
        //
        // Only perform the move operation if it will not generate a page
        // fault.
        //

        Address1 = BdWriteCheck((PVOID)Destination);
        Address2 = BdReadCheck((PVOID)Source);
        if ((Address1 == NULL) || (Address2 == NULL))
        {
            break;
        }

        *(PCHAR)Destination = *(PCHAR)Source;
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
#if defined(MDE_CPU_AARCH64)
    BlArchSweepIcacheRange(BaseDestination, (SIZE_T)BytesMoved);
#endif
    return BytesMoved;
}


VOID
BdCopyMemory (
    __out_bcount(Length) volatile PCHAR Destination,
    __in_bcount(Length) volatile PCHAR Source,
    __in UINT32 Length
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
    while (Length > 0)
    {
        *Destination = *Source;
        Destination += 1;
        Source += 1;
        Length -= 1;
    }

    return;
}
