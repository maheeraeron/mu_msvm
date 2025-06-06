/**@file Print.c

This file contains support for printing strings to the kernel debugger.

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

#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/DebugAgentLib.h>
#include <KdTypes.h>
#include <Library/KdTransportLib.h>
#include <Library/KdProtocolLib.h>
#include "KdProtocol.h"
#include <Protocol/Cpu.h>

BOOLEAN
KdProtocolPrintString (
  KD_STRING  *Output
  )
{
  BOOLEAN         BreakIn;
  DBGKD_DEBUG_IO  DebugIo;
  UINT16          Length;
  KD_STRING       MessageData;
  KD_STRING       MessageHeader;

  //
  // If the debugger is not connected, there is no point in sending debug
  // prints.
  //
  if (KdTransportIsDebuggerConnected () == FALSE) {
    return FALSE;
  }

  Length = (UINT16)Output->Length;

  //
  // If the total message length is greater than the maximum packet size,
  // then truncate the output string.
  //
  if ((sizeof (DBGKD_DEBUG_IO) + Length) > PACKET_MAX_SIZE) {
    Length = PACKET_MAX_SIZE - sizeof (DBGKD_DEBUG_IO);
  }

  //
  // Construct the print string message and message descriptor.
  //
  DebugIo.ApiNumber                    = DbgKdPrintStringApi;
  DebugIo.ProcessorLevel               = 0;
  DebugIo.Processor                    = 0;
  DebugIo.u.PrintString.LengthOfString = Length;
  MessageHeader.Length                 = sizeof (DBGKD_DEBUG_IO);
  MessageHeader.Buffer                 = (UINT8 *)&DebugIo;

  //
  // Construct the print string data and data descriptor.
  //
  MessageData.Length = Length;
  MessageData.Buffer = Output->Buffer;

  //
  // Send packet to the kernel debugger on the host machine.
  //
  KdTransportSendPacket (
    PACKET_TYPE_KD_DEBUG_IO,
    &MessageHeader,
    &MessageData
    );

  BreakIn = KdProtocolPollBreakIn ();
  return BreakIn;
}

/**

  This routine prints a string, then reads a reply string.

  @param[in]      Output  Supplies a pointer to a string descriptor for the
                          output string.

  @param[in,out]  Input   Supplies a pointer to a string descriptor for the
                          input string. (Length stored/returned in
                          Input->Length).

  @retval  TRUE  A Breakin sequence was seen, caller should breakpoint and retry
  @retval  FALSE  No Breakin seen.

**/
UINT32
KdProtocolPromptString (
  KD_STRING  *Output,
  KD_STRING  *Input
  )
{
  DBGKD_DEBUG_IO  DebugIo;
  UINT32          Length;
  KD_STRING       MessageData;
  KD_STRING       MessageHeader;
  UINT32          Status;

  Length = Output->Length;

  //
  // If the total message length is greater than the maximum packet size,
  // then truncate the output string.
  //
  if ((sizeof (DBGKD_DEBUG_IO) + Length) > PACKET_MAX_SIZE) {
    Length = PACKET_MAX_SIZE - sizeof (DBGKD_DEBUG_IO);
  }

  //
  // Construct the prompt string message and message descriptor.
  //
  DebugIo.ApiNumber                        = DbgKdGetStringApi;
  DebugIo.ProcessorLevel                   = 0;
  DebugIo.Processor                        = 0;
  DebugIo.u.GetString.LengthOfPromptString = Length;
  DebugIo.u.GetString.LengthOfStringRead   = Input->MaximumLength;
  MessageHeader.Length                     = sizeof (DBGKD_DEBUG_IO);
  MessageHeader.Buffer                     = (UINT8 *)&DebugIo;

  //
  // Construct the prompt string data and data descriptor.
  //
  MessageData.Length = (UINT16)Length;
  MessageData.Buffer = Output->Buffer;

  //
  // Send packet to the debugger on the host machine.
  //
  KdTransportSendPacket (
    PACKET_TYPE_KD_DEBUG_IO,
    &MessageHeader,
    &MessageData
    );

  //
  // Receive packet from the kernel debugger on the host machine.
  //
  MessageHeader.MaximumLength = sizeof (DBGKD_DEBUG_IO);
  MessageData.MaximumLength   = Input->MaximumLength;
  MessageData.Buffer          = Input->Buffer;
  do {
    Status = KdTransportReceivePacket (
               PACKET_TYPE_KD_DEBUG_IO,
               &MessageHeader,
               &MessageData,
               &Length
               );

    if (Status == KD_PACKET_RESEND) {
      Status        = TRUE;
      Input->Length = 0;
      goto Cleanup;
    }
  } while (Status != KD_PACKET_RECEIVED);

  Input->Length = (UINT16)Length;
  Status        = FALSE;

Cleanup:
  return Status;
}
