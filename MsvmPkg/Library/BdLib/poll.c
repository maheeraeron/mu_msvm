/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    poll.c

Abstract:

    This module contains code to poll for debugger breakin.

Author:

    David N. Cutler (davec) 27-Nov-96

Revision History:

--*/

#include "Bd.h"

UINT32
BdPollBreakIn (
    VOID
    )
/*++

Routine Description:

    This function checks to determine if a breakin packet is pending.
    If a packet is present.

    A packet is present if:

    There is a valid character which matches BREAK_CHAR.

Return Value:

    A function value of TRUE is returned if a breakin packet is present.
    Otherwise, a value of FALSE is returned.

--*/
{
    LOGICAL BreakIn;
    ULONG Status;

    //
    // If the debugger is initialized, check if a breakin by the kernel
    // debugger is pending.  Always query for the breakin packet so that the
    // debugger present state can be updated by the transport.
    //

    BreakIn = FALSE;
    if (BdDebuggerInitialized() != FALSE)
    {
        Status = BdReceivePacket(PACKET_TYPE_KD_POLL_BREAKIN,
                                    NULL,
                                    NULL,
                                    NULL);

        if (Status == BD_PACKET_RECEIVED)
        {
            BreakIn = TRUE;
            BdControlCPressed = TRUE;
        }

        if (BdControlCPending != FALSE)
        {
            BdControlCPressed = TRUE;
            BreakIn = TRUE;
            BdControlCPending = FALSE;
        }
    }

    return BreakIn;
}
