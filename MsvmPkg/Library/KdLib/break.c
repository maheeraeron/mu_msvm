/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    break.c

Abstract:

    This module implements machine dependent functions to add and delete
    breakpoints from the kernel debugger breakpoint table.

Author:

    David N. Cutler 2-Aug-1990

Revision History:

--*/


#include "EfiKd.h"

LOGICAL BreakpointsSuspended = FALSE;

LOGICAL
EfiKdLowWriteContent (
    __in UINT32 Index
    );


UINT32
EfiKdAddBreakpoint (
    __in UINT64 Address
    )
/*++

Routine Description:

    This routine adds an entry to the breakpoint table and returns a handle
    to the breakpoint table entry.

Arguments:

    Address - Supplies the address where to set the breakpoint.

Return Value:

    A value of zero is returned if the specified address is already in the
    breakpoint table, there are no free entries in the breakpoint table, the
    specified address is not correctly aligned, or the specified address is
    not valid. Otherwise, the index of the assigned breakpoint table entry
    plus one is returned as the function value.

--*/
{
    BOOLEAN accessible;
    EFI_KD_BREAKPOINT_TYPE content;
    UINT32 index;

    //DPRINT(("KD: Setting breakpoint at 0x%08x\n", Address));
    //
    // If the specified address is not properly aligned, then return zero.
    //

    if (((UINT_PTR)Address & EFI_KD_BREAKPOINT_ALIGN) != 0) 
    {
        return 0;
    }

    //
    // Get the instruction to be replaced. If the instruction cannot be read,
    // then mark the breakpoint as not accessible.
    //

    if (EfiKdMoveMemory((PCHAR)&content,
                        (PCHAR)(UINT_PTR)Address,
                        sizeof(EFI_KD_BREAKPOINT_TYPE) ) != sizeof(EFI_KD_BREAKPOINT_TYPE)) 
    {
        accessible = FALSE;
    } 
    else 
    {
        accessible = TRUE;
    }

    //
    // If the specified address is not write accessible, then return zero.
    //

    if (accessible && EfiKdWriteCheck((PVOID)(UINT_PTR)Address) == NULL) 
    {
        return 0;
    }

    //
    // Search the breakpoint table for a free entry and check if the specified
    // address is already in the breakpoint table.
    //

    for (index = 0; index < BREAKPOINT_TABLE_SIZE; index += 1) 
    {
        if (EfiKdBreakpointTable[index].Flags == 0) 
        {
            break;
        }
    }

    //
    // If a free entry was found, then write breakpoint and return the handle
    // value plus one. Otherwise, return zero.
    //

    if (index == BREAKPOINT_TABLE_SIZE) 
    {
        return 0;
    }

    if (accessible) {
        EfiKdBreakpointTable[index].Address = Address;
        EfiKdBreakpointTable[index].Content = content;
        EfiKdBreakpointTable[index].Flags = EFI_KD_BREAKPOINT_IN_USE;
        if (EfiKdMoveMemory((PCHAR)(UINT_PTR)Address,
                            (PCHAR)&EfiKdBreakpointInstruction,
                            sizeof(EFI_KD_BREAKPOINT_TYPE)) != sizeof(EFI_KD_BREAKPOINT_TYPE)) 
        {
        }
    } 
    else 
    {
        EfiKdBreakpointTable[index].Address = Address;
        EfiKdBreakpointTable[index].Flags = EFI_KD_BREAKPOINT_IN_USE | EFI_KD_BREAKPOINT_NEEDS_WRITE;
    }

    return index + 1;
}


LOGICAL
EfiKdLowWriteContent (
    __in UINT32 Index
    )
/*++

Routine Description:

    This routine attempts to replace the code that a breakpoint is
    written over.  This routine, EfiKdAddBreakpoint,
    EfiKdLowRestoreBreakpoint and KdSetOwedBreakpoints are responsible
    for getting data written as requested.  Callers should not
    examine or use EfiKdOweBreakpoints, and they should not set the
    NEEDS_WRITE or NEEDS_REPLACE flags.

    Callers must still look at the return value from this function,
    however: if it returns FALSE, the breakpoint record must not be
    reused until KdSetOwedBreakpoints has finished with it.

Arguments:

    index - Supplies the index of the breakpoint table entry
        which is to be deleted.

Return Value:

    Returns TRUE if the breakpoint was removed, FALSE if it was deferred.

--*/
{
    //
    // Do the contents need to be replaced at all?
    //

    if (EfiKdBreakpointTable[Index].Flags & EFI_KD_BREAKPOINT_NEEDS_WRITE) 
    {
        //
        // The breakpoint was never written out.  Clear the flag
        // and we are done.
        //

        EfiKdBreakpointTable[Index].Flags &= ~EFI_KD_BREAKPOINT_NEEDS_WRITE;
        return TRUE;
    }

    if (EfiKdMoveMemory((PCHAR)(UINT_PTR)EfiKdBreakpointTable[Index].Address,
                        (PCHAR)&EfiKdBreakpointTable[Index].Content,
                        sizeof(EFI_KD_BREAKPOINT_TYPE) ) != sizeof(EFI_KD_BREAKPOINT_TYPE)) 
    {
        EfiKdBreakpointTable[Index].Flags |= EFI_KD_BREAKPOINT_NEEDS_REPLACE;
        return FALSE;
    } 
    else 
    {
        return TRUE;
    }
}


LOGICAL
EfiKdDeleteBreakpoint (
    __in UINT32 Handle
    )
/*++

Routine Description:

    This routine deletes an entry from the breakpoint table.

Arguments:

    Handle - Supplies the index plus one of the breakpoint table entry
        which is to be deleted.

Return Value:

    A value of FALSE is returned if the specified handle is not a valid
    value or the breakpoint cannot be deleted because the old instruction
    cannot be replaced. Otherwise, a value of TRUE is returned.

--*/
{
    UINT32 index;

    index = Handle - 1;

    //
    // If the specified handle is not valid, then return FALSE.
    //

    if ((Handle == 0) || (Handle > BREAKPOINT_TABLE_SIZE)) 
    {
        return FALSE;
    }

    //
    // If the specified breakpoint table entry is not valid, then return FALSE.
    //

    if (EfiKdBreakpointTable[index].Flags == 0) 
    {
        return FALSE;
    }

    //
    // If the breakpoint is already suspended, just delete it from the table.
    //

    if (EfiKdBreakpointTable[index].Flags & EFI_KD_BREAKPOINT_SUSPENDED) 
    {
        if ( !(EfiKdBreakpointTable[index].Flags & EFI_KD_BREAKPOINT_NEEDS_REPLACE) ) 
        {
            EfiKdBreakpointTable[index].Flags = 0;
            return TRUE;
        }
    }

    //
    // Replace the instruction contents.
    //

    if (EfiKdLowWriteContent(index)) 
    {
        //
        // Delete breakpoint table entry
        //

        EfiKdBreakpointTable[index].Flags = 0;
    }

    return TRUE;
}


LOGICAL
EfiKdDeleteBreakpointRange (
    __in UINT64 Lower,
    __in UINT64 Upper
    )
/*++

Routine Description:

    This routine deletes all breakpoints falling in a given range
    from the breakpoint table.

Arguments:

    Lower - inclusive lower address of range from which to remove BPs.

    Upper - include upper address of range from which to remove BPs.

Return Value:

    TRUE if any breakpoints removed, FALSE otherwise.

--*/
{
    UINT32 index;
    BOOLEAN returnStatus;

    returnStatus = FALSE;

    //
    // Examine each entry in the table in turn
    //

    for (index = 0; index < BREAKPOINT_TABLE_SIZE; index++) 
    {
        if ((EfiKdBreakpointTable[index].Flags & EFI_KD_BREAKPOINT_IN_USE) &&
            (((PVOID)(UINT_PTR)EfiKdBreakpointTable[index].Address >= 
              (PVOID)(UINT_PTR)Lower) &&
             ((PVOID)(UINT_PTR)EfiKdBreakpointTable[index].Address <= 
              (PVOID)(UINT_PTR)Upper))) 
        {

            //
            // Breakpoint is in use and falls in range, clear it.
            //

            returnStatus = (BOOLEAN)(returnStatus || EfiKdDeleteBreakpoint(index+1));
        }
    }

    return returnStatus;
}


VOID
EfiKdSuspendBreakpoint (
    UINT32 Handle
    )
{
    UINT32 index;

    index = Handle - 1;

    if ((EfiKdBreakpointTable[index].Flags & EFI_KD_BREAKPOINT_IN_USE) &&
        !(EfiKdBreakpointTable[index].Flags & EFI_KD_BREAKPOINT_SUSPENDED) ) 
    {
        EfiKdBreakpointTable[index].Flags |= EFI_KD_BREAKPOINT_SUSPENDED;
        EfiKdLowWriteContent(index);
    }

    return;
}


VOID
EfiKdSuspendAllBreakpoints (
    VOID
    )
{
    UINT32 handle;

    BreakpointsSuspended = TRUE;

    for (handle = 1; handle <= BREAKPOINT_TABLE_SIZE; handle++) 
    {
        EfiKdSuspendBreakpoint(handle);
    }

    return;
}


LOGICAL
EfiKdLowRestoreBreakpoint (
    __in UINT32 Index
    )
/*++

Routine Description:

    This routine attempts to write a breakpoint instruction.
    The old contents must have already been stored in the
    breakpoint record.

Arguments:

    index - Supplies the index of the breakpoint table entry
        which is to be written.

Return Value:

    Returns TRUE if the breakpoint was written, FALSE if it was
    not and has been marked for writing later.

--*/
{
    //
    // Does the breakpoint need to be written at all?
    //

    if (EfiKdBreakpointTable[Index].Flags & EFI_KD_BREAKPOINT_NEEDS_REPLACE) 
    {

        //
        // The breakpoint was never removed.  Clear the flag
        // and we are done.
        //

        EfiKdBreakpointTable[Index].Flags &= ~EFI_KD_BREAKPOINT_NEEDS_REPLACE;
        return TRUE;
    }

    //
    // Replace the instruction contents.
    //

    EfiKdMoveMemory((PCHAR)(UINT_PTR)EfiKdBreakpointTable[Index].Address,
                    (PCHAR)&EfiKdBreakpointInstruction,
                     sizeof(EFI_KD_BREAKPOINT_TYPE));
    return TRUE;
}


VOID
EfiKdRestoreAllBreakpoints (
    VOID
    )
{
    UINT32 index;

    BreakpointsSuspended = FALSE;

    for ( index = 0; index < BREAKPOINT_TABLE_SIZE; index++ ) 
    {
        if ((EfiKdBreakpointTable[index].Flags & EFI_KD_BREAKPOINT_IN_USE) &&
            (EfiKdBreakpointTable[index].Flags & EFI_KD_BREAKPOINT_SUSPENDED) ) 
        {
            EfiKdBreakpointTable[index].Flags &= ~EFI_KD_BREAKPOINT_SUSPENDED;
            EfiKdLowRestoreBreakpoint(index);
        }
    }

    return;
}
