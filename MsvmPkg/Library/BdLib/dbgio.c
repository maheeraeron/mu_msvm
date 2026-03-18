/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    dbgio.c

Abstract:

    This module implements the boot debugger print and prompt functions.

Author:

    Mark Lucovsky (markl) 31-Aug-1990

--*/

// ------------------------------------------------------------------- Includes

#include "Bd.h"

// ------------------------------------------------------------------ Functions

LOGICAL
BdPrintString (
    __in PSTRING Output
    )
/*++

Routine Description:

    This routine prints a string.

Arguments:

    Output - Supplies a pointer to a string descriptor for the output string.

Return Value:

    TRUE if Control-C present in input buffer after print is done.
    FALSE otherwise.

--*/
{
    DBGKD_DEBUG_IO debugIo;
    UINT32 length;
    STRING messageData;
    STRING messageHeader;

    //
    // Move the output string to the message buffer.
    //

    length = (UINT32)BdMoveMemory((PCHAR)BdMessageBuffer,
                             (PCHAR)Output->Buffer,
                             Output->Length);

    //
    // If the total message length is greater than the maximum packet size,
    // then truncate the output string.
    //

    if ((sizeof(DBGKD_DEBUG_IO) + length) > PACKET_MAX_SIZE) 
    {
        length = PACKET_MAX_SIZE - sizeof(DBGKD_DEBUG_IO);
    }

    //
    // Construct the print string message and message descriptor.
    //

    debugIo.ApiNumber = DbgKdPrintStringApi;
    debugIo.ProcessorLevel = 0;
    debugIo.Processor = 0;
    debugIo.u.PrintString.LengthOfString = length;
    messageHeader.Length = sizeof(DBGKD_DEBUG_IO);
    messageHeader.Buffer = (PCHAR)&debugIo;

    //
    // Construct the print string data and data descriptor.
    //

    messageData.Length = (USHORT)length;
    messageData.Buffer = (PCHAR)(&BdMessageBuffer[0]);

    //
    // Send packet to the kernel debugger on the host machine.
    //

    BdSendPacket(PACKET_TYPE_KD_DEBUG_IO, &messageHeader, &messageData);
    return BdPollBreakIn();
}


LOGICAL
BdPromptString (
    __in PSTRING Output,
    __inout PSTRING Input
    )
/*++

Routine Description:

    This routine prints a string, then reads a reply string.

Arguments:

    Output - Supplies a pointer to a string descriptor for the output string.

    Input - Supplies a pointer to a string descriptor for the input string.
            (Length stored/returned in Input->Length)

Return Value:

    TRUE - A Breakin sequence was seen, caller should breakpoint and retry
    FALSE - No Breakin seen.

--*/
{
    UINT32 length;
    STRING messageData;
    STRING messageHeader;
    DBGKD_DEBUG_IO debugIo;
    UINT32 returnCode;

    //
    // Move the output string to the message buffer.
    //

    length = (UINT32)BdMoveMemory((PCHAR)BdMessageBuffer,
                             (PCHAR)Output->Buffer,
                             Output->Length);

    //
    // If the total message length is greater than the maximum packet size,
    // then truncate the output string.
    //

    if ((sizeof(DBGKD_DEBUG_IO) + length) > PACKET_MAX_SIZE) 
    {
        length = PACKET_MAX_SIZE - sizeof(DBGKD_DEBUG_IO);
    }

    //
    // Construct the prompt string message and message descriptor.
    //

    debugIo.ApiNumber = DbgKdGetStringApi;
    debugIo.ProcessorLevel = 0;
    debugIo.Processor = 0;
    debugIo.u.GetString.LengthOfPromptString = length;
    debugIo.u.GetString.LengthOfStringRead = Input->MaximumLength;
    messageHeader.Length = sizeof(DBGKD_DEBUG_IO);
    messageHeader.Buffer = (PCHAR)&debugIo;

    //
    // Construct the prompt string data and data descriptor.
    //

    messageData.Length = (USHORT)length;
    messageData.Buffer = (PCHAR)(&BdMessageBuffer[0]);

    //
    // Send packet to the kernel debugger on the host machine.
    //

    BdSendPacket(PACKET_TYPE_KD_DEBUG_IO, &messageHeader, &messageData);

    //
    // Receive packet from the kernel debugger on the host machine.
    //

    messageHeader.MaximumLength = sizeof(DBGKD_DEBUG_IO);
    messageData.MaximumLength = BD_MESSAGE_BUFFER_SIZE;
    do 
    {
        returnCode = BdReceivePacket(PACKET_TYPE_KD_DEBUG_IO,
                                        &messageHeader,
                                        &messageData,
                                        &length);

        if (returnCode == BD_PACKET_RESEND) 
        {
            return TRUE;
        }
    } while (returnCode != BD_PACKET_RECEIVED);

    length = MIN(length, Input->MaximumLength);
    Input->Length = (USHORT)BdMoveMemory((PCHAR)Input->Buffer,
                                            (PCHAR)BdMessageBuffer,
                                            length);

    return FALSE;
}
