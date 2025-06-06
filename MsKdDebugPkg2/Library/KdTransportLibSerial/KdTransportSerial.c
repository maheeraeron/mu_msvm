/**@file KdTransportSerial.c

This file contains support for connecting to the kernel debugger via serial.

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

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/SerialPortLib.h>
#include <Library/BaseMemoryLib.h>
#include <KdTypes.h>
#include <KdTransportSerial.h>

UINT32   mKdComPacketsReceived = 0;
UINT32   mKdComPacketsSent     = 0;
UINT32   mKdNextPacketIdToSend = INITIAL_PACKET_ID;
UINT32   mKdPacketIdExpected   = INITIAL_PACKET_ID;
UINT32   mKdControlCPending    = FALSE;
BOOLEAN  mKdDebuggerConnected  = TRUE;

VOID
KdSerialSendControlPacket (
  IN UINT16  PacketType,
  IN UINT32  PacketId
  );

UINT32
KdSerialRead (
  UINT8   *Byte,
  UINT32  Timeout
  )
{
  UINT32  Status;

  Status = KDSERIAL_NODATA;
  while (Timeout != 0) {
    Timeout -= 1;
    if (SerialPortPoll () != FALSE) {
      SerialPortRead (Byte, 1);
      Status = KDSERIAL_SUCCESS;
      break;
    }
  }

  return Status;
}

UINT32
KdSerialWrite (
  UINT8  Byte
  )
{
  SerialPortWrite (&Byte, 1);
  return KDSERIAL_SUCCESS;
}

UINT32
KdSerialComputeChecksum (
  IN UINT8   *Buffer,
  IN UINT32  Length
  )
{
  UINT32  Checksum;

  Checksum = 0;
  while (Length > 0) {
    Checksum = Checksum + (UINT32)*Buffer++;
    Length--;
  }

  return Checksum;
}

/**
  This routine performs initialization for the KdTransportLib.

  @param  None.

  @retval EFI_SUCCESS           The transport library successfully initialized.

**/
EFI_STATUS
KdTransportLibInitialize (
  VOID
  )
{
  UINT8  Byte;

  //
  // Drain any data that has been sent to us. Any data present is of
  // no use because we have not attempted to sync with the host yet.
  //

  while (KdSerialRead (&Byte, 1) != KDSERIAL_NODATA) {
  }

  return EFI_SUCCESS;
}

UINT32
KdSerialReceivePacketLeader (
  UINT32  PacketType,
  UINT32  *PacketLeader
  )
{
  BOOLEAN  BreakinDetected;
  UINT32   Index;
  UINT8    Input;
  UINT8    PreviousByte;
  UINT32   Status;

  BreakinDetected = FALSE;
  PreviousByte    = 0;

  //
  // NOTE - With all the interrupts being off, it is very hard
  // to implement the actual timeout code. (Maybe, by reading the CMOS.)
  // Here we use a loop count to wait about 3 seconds.  The CpGetByte
  // will return with error code = CP_GET_NODATA if it cannot find data
  // byte within 1 second. Kernel debugger's timeout period is 5 seconds.
  //

  Index = 0;
  do {
    Status = KdSerialRead (&Input, KDSERIAL_MAX_TIMEOUT);
    if (Status == KDSERIAL_NODATA) {
      if (BreakinDetected != FALSE) {
        mKdControlCPending = TRUE;
        Status             = KD_PACKET_RESEND;
        goto Cleanup;
      } else {
        Status = KD_PACKET_TIMEOUT;
        goto Cleanup;
      }
    } else if (Status == KDSERIAL_ERROR) {
      Index = 0;
      continue;
    } else {
      if ((Input == PACKET_LEADER_BYTE) ||
          (Input == CONTROL_PACKET_LEADER_BYTE))
      {
        if (Index == 0) {
          PreviousByte = Input;
          Index++;
        } else if (Input == PreviousByte) {
          Index++;
        } else {
          PreviousByte = Input;
          Index        = 1;
        }
      } else {
        //
        // If we detect breakin character, we need to verify it
        // validity.  (It is possible that we missed a packet leader
        // and the breakin character is simply a data byte in the
        // packet.)
        // Since kernel debugger send out breakin character ONLY
        // when it is waiting for State Change packet.  The breakin
        // character should not be followed by any other character
        // except packet leader byte.
        //
        if (Input == BREAKIN_PACKET_BYTE) {
          BreakinDetected = TRUE;
        } else {
          //
          // The following statement is ABSOLUTELY necessary.
          //
          BreakinDetected = FALSE;
        }

        Index = 0;
      }
    }
  } while (Index < 4);

  if (BreakinDetected != FALSE) {
    mKdControlCPending = TRUE;
  }

  //
  // Return the packet leader and FALSE to indicate no resend is needed.
  //
  if (Input == PACKET_LEADER_BYTE) {
    *PacketLeader = PACKET_LEADER;
  } else {
    *PacketLeader = CONTROL_PACKET_LEADER;
  }

  Status = KD_PACKET_RECEIVED;

Cleanup:
  return Status;
}

UINT32
KdSerialReceiveString (
  UINT8   *Destination,
  UINT32  Length
  )
{
  UINT8   Input;
  UINT32  Status;

  //
  // Read bytes until either a error is encountered or the entire string
  // has been read.
  //
  while (Length > 0) {
    Status = KdSerialRead (&Input, KDSERIAL_MAX_TIMEOUT);
    if (Status != KDSERIAL_SUCCESS) {
      goto Cleanup;
    } else {
      *Destination++ = Input;
      Length        -= 1;
    }
  }

  Status = KDSERIAL_SUCCESS;

Cleanup:
  return Status;
}

/**
  This routine receives a packet from the kernel debugger.

  @param  PacketType           The type of packet to receive.
  @param  MessageHeader        Pointer to the packet message header.
  @param  MessageData          Pointer to the packet message data.
  @param  DataLength           Length of the message data.

  @retval KD_PACKET_RECEIVED   Packet was received.
  @retval KD_PACKET_TIMEOUT    Packet time-out occurred.
  @retval KD_PACKET_RESEND     Packet resend was requested.
**/
UINT32
KdTransportReceivePacket (
  UINT32     PacketType,
  KD_STRING  *MessageHeader OPTIONAL,
  KD_STRING  *MessageData OPTIONAL,
  UINT32     *DataLength OPTIONAL
  )
{
  UINT8      Byte;
  UINT32     Checksum;
  UINT32     MessageLength;
  KD_PACKET  PacketHeader;
  UINT32     Status;

  // Consider raising TPL to block the poll from timer interrupt.  Old code used
  // KdContext->BlockPoll.

  //
  // A breakin packet is a single byte that is requested frequently.  To avoid
  // spending significant time waiting for this packet, look for this single
  // byte and exit early if this packet type is being requested.  Do not
  // increment KdComPacketsReceived for a breakin byte, since there is no
  // validation of a full packet performed.  We only want to require the
  // disconnect message to be sent when full packets were successfully sent
  // or received.
  //
  if (PacketType == PACKET_TYPE_KD_POLL_BREAKIN) {
    //
    // Requesting the presence of a BREAKIN_PACKET_BYTE.  The debugger
    // sends in a single character 'b' to indicate a break request.  The
    // problem with a physical serial port is the buffer.  The debugger
    // is also trying to connect by sending a PACKET_TYPE_KD_RESET packet,
    // which is 30 some bytes.  Only reading one character from the serial
    // port every second will never see the 'b'.  So, purge the buffer
    // looking for the 'b'.

    do {
      Status = KdSerialRead (&Byte, KDSERIAL_MIN_TIMEOUT);
    } while (!mKdDebuggerConnected &&
             (Status == KDSERIAL_SUCCESS) &&
             (Byte != BREAKIN_PACKET_BYTE));

    if ((Status == KDSERIAL_SUCCESS) && (Byte == BREAKIN_PACKET_BYTE)) {
      Status = KD_PACKET_RECEIVED;
    } else {
      Status = KD_PACKET_TIMEOUT;
    }

    goto Cleanup;
  }

  ZeroMem (&PacketHeader, sizeof (PacketHeader));

WaitForPacketLeader:

  //
  // Read Packet Leader
  //
  Status = KdSerialReceivePacketLeader (PacketType, &PacketHeader.PacketLeader);
  if (Status != KD_PACKET_RECEIVED) {
    goto Cleanup;
  }

  //
  // Read packet type.
  //
  Status = KdSerialReceiveString (
             (UINT8 *)&PacketHeader.PacketType,
             sizeof (PacketHeader.PacketType)
             );

  if (Status != KD_PACKET_RECEIVED) {
    goto Cleanup;
  }

  //
  // If the packet we received is a resend request, we inform the caller to
  // resend the packet.
  //
  if ((PacketHeader.PacketLeader == CONTROL_PACKET_LEADER) &&
      (PacketHeader.PacketType == PACKET_TYPE_KD_RESEND))
  {
    Status = KD_PACKET_RESEND;
    goto Cleanup;
  }

  //
  // Read data length.
  //
  Status = KdSerialReceiveString (
             (UINT8 *)&PacketHeader.ByteCount,
             sizeof (PacketHeader.ByteCount)
             );

  if (Status != KD_PACKET_RECEIVED) {
    goto Cleanup;
  }

  //
  // Read Packet Id.
  //
  Status = KdSerialReceiveString (
             (UINT8 *)&PacketHeader.PacketId,
             sizeof (PacketHeader.PacketId)
             );

  if (Status != KD_PACKET_RECEIVED) {
    goto Cleanup;
  }

  //
  // Read packet checksum.
  //
  Status = KdSerialReceiveString (
             (UINT8 *)&PacketHeader.Checksum,
             sizeof (PacketHeader.Checksum)
             );

  if (Status != KD_PACKET_RECEIVED) {
    goto Cleanup;
  }

  //
  // A complete packet header is received.  Check its validity and
  // perform appropriate action depending on packet type.
  //
  if (PacketHeader.PacketLeader == CONTROL_PACKET_LEADER) {
    if (PacketHeader.PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {
      //
      // If we received an expected ACK packet and we are not
      // waiting for any new packet, update outgoing packet id
      // and return.  If we are NOT waiting for ACK packet
      // we will keep on waiting.  If the ACK packet
      // is not for the packet we send, ignore it and keep on waiting.
      //
      if (PacketHeader.PacketId != (mKdNextPacketIdToSend & ~SYNC_PACKET_ID)) {
        goto WaitForPacketLeader;
      } else if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {
        mKdNextPacketIdToSend ^= 1;
        Status                 = KD_PACKET_RECEIVED;
        goto Cleanup;
      } else {
        goto WaitForPacketLeader;
      }
    } else if (PacketHeader.PacketType == PACKET_TYPE_KD_RESET) {
      //
      // Reset the packet control variables and resend earlier packet.
      //
      mKdNextPacketIdToSend = INITIAL_PACKET_ID;
      mKdPacketIdExpected   = INITIAL_PACKET_ID;
      KdSerialSendControlPacket (PACKET_TYPE_KD_RESET, 0L);
      Status = KD_PACKET_RESEND;
      goto Cleanup;
    } else if (PacketHeader.PacketType == PACKET_TYPE_KD_RESEND) {
      Status = KD_PACKET_RESEND;
      goto Cleanup;
    } else {
      //
      // Invalid packet header, ignore it.
      //
      goto WaitForPacketLeader;
    }

    //
    // The packet header is for data packet (not control packet).
    //
  } else if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {
    //
    // If we are waiting for ACK packet ONLY and we receive a data packet
    // header, check if the packet id is what we expected.  If yes, assume the
    // acknowledge is lost (but sent), ask sender to resend and return with
    // PACKET_RECEIVED.
    //
    if (PacketHeader.PacketId == mKdPacketIdExpected) {
      KdSerialSendControlPacket (PACKET_TYPE_KD_RESEND, 0L);
      mKdNextPacketIdToSend ^= 1;
      Status                 = KD_PACKET_RECEIVED;
      goto Cleanup;
    } else {
      KdSerialSendControlPacket (
        PACKET_TYPE_KD_ACKNOWLEDGE,
        PacketHeader.PacketId
        );

      goto WaitForPacketLeader;
    }
  }

  //
  // We are waiting for data packet and we received the correct packet header.
  // Ensure the ByteCount received is valid.
  //
  MessageLength = MessageHeader->MaximumLength;
  if ((PacketHeader.ByteCount > (UINT16)PACKET_MAX_SIZE) ||
      (PacketHeader.ByteCount < (UINT16)MessageLength))
  {
    goto SendResendPacket;
  }

  *DataLength = PacketHeader.ByteCount - MessageLength;

  //
  // Read the message header.
  //
  Status = KdSerialReceiveString (MessageHeader->Buffer, MessageLength);
  if (Status != KD_PACKET_RECEIVED) {
    goto SendResendPacket;
  }

  MessageHeader->Length = (UINT16)MessageLength;

  //
  // Read the message data.
  //
  Status = KdSerialReceiveString (MessageData->Buffer, *DataLength);
  if (Status != KD_PACKET_RECEIVED) {
    goto SendResendPacket;
  }

  MessageData->Length = (UINT16)*DataLength;

  //
  // Read packet trailing byte
  //
  Status = KdSerialRead (&Byte, KDSERIAL_MAX_TIMEOUT);
  if ((Status != KDSERIAL_SUCCESS) || (Byte != PACKET_TRAILING_BYTE)) {
    goto SendResendPacket;
  }

  //
  // Check PacketType is what we are waiting for.
  //
  if (PacketType != PacketHeader.PacketType) {
    KdSerialSendControlPacket (PACKET_TYPE_KD_ACKNOWLEDGE, PacketHeader.PacketId);
    goto WaitForPacketLeader;
  }

  //
  // Check PacketId is valid.
  //
  if ((PacketHeader.PacketId == INITIAL_PACKET_ID) ||
      (PacketHeader.PacketId == (INITIAL_PACKET_ID ^ 1)))
  {
    if (PacketHeader.PacketId != mKdPacketIdExpected) {
      KdSerialSendControlPacket (
        PACKET_TYPE_KD_ACKNOWLEDGE,
        PacketHeader.PacketId
        );

      goto WaitForPacketLeader;
    }
  } else {
    goto SendResendPacket;
  }

  //
  // Check checksum is valid.
  //
  Checksum = KdSerialComputeChecksum (
               (UINT8 *)MessageHeader->Buffer,
               MessageHeader->Length
               );

  Checksum += KdSerialComputeChecksum (
                (UINT8 *)MessageData->Buffer,
                MessageData->Length
                );

  if (Checksum != PacketHeader.Checksum) {
    goto SendResendPacket;
  }

  //
  // Send Acknowledge byte and the Id of the packet received.
  // Then, update the ExpectId for next incoming packet.
  //
  KdSerialSendControlPacket (PACKET_TYPE_KD_ACKNOWLEDGE, PacketHeader.PacketId);

  //
  // We have successfully received the packet so update the
  // packet control variables and return success.
  //
  mKdComPacketsReceived += 1;
  mKdPacketIdExpected   ^= 1;
  Status                 = KD_PACKET_RECEIVED;
  goto Cleanup;

SendResendPacket:
  KdSerialSendControlPacket (PACKET_TYPE_KD_RESEND, 0L);
  goto WaitForPacketLeader;

Cleanup:

  return Status;
}

VOID
KdSerialSendString (
  UINT8   *Source,
  UINT32  Length
  )
{
  UINT8  Output;

  //
  // Write the message string to the debugger port.
  //
  while (Length > 0) {
    Output = *Source++;
    KdSerialWrite (Output);
    Length -= 1;
  }

  return;
}

VOID
KdSerialSendControlPacket (
  UINT16  PacketType,
  UINT32  PacketId
  )
{
  KD_PACKET  PacketHeader;

  //
  // Initialize and send the packet header.
  //
  PacketHeader.PacketLeader = CONTROL_PACKET_LEADER;
  PacketHeader.PacketId     = PacketId;
  PacketHeader.ByteCount    = 0;
  PacketHeader.Checksum     = 0;
  PacketHeader.PacketType   = PacketType;
  KdSerialSendString ((UINT8 *)&PacketHeader, sizeof (PacketHeader));
  return;
}

/**
  This routine sends a packet to the kernel debugger.

  @param  PacketType           The type of packet to send.
  @param  MessageHeader        Pointer to the packet message header.
  @param  MessageData          Pointer to the packet message data.

  @retval None.

**/
VOID
KdTransportSendPacket (
  UINT32     PacketType,
  KD_STRING  *MessageHeader,
  KD_STRING  *MessageData OPTIONAL
  )
{
  UINT32     MessageDataLength;
  KD_PACKET  PacketHeader;
  UINT32     RetryCount;
  UINT32     ReturnCode;

  if (MessageData != NULL) {
    MessageDataLength     = MessageData->Length;
    PacketHeader.Checksum = KdSerialComputeChecksum (
                              (UINT8 *)MessageData->Buffer,
                              MessageData->Length
                              );
  } else {
    MessageDataLength     = 0;
    PacketHeader.Checksum = 0;
  }

  PacketHeader.Checksum += KdSerialComputeChecksum (
                             (UINT8 *)MessageHeader->Buffer,
                             MessageHeader->Length
                             );

  //
  // Initialize and send the packet header.
  //
  PacketHeader.PacketLeader = PACKET_LEADER;
  PacketHeader.ByteCount    = (UINT16)(MessageHeader->Length + MessageDataLength);
  PacketHeader.PacketType   = (UINT16)PacketType;
  RetryCount                = KDSERIAL_MAX_RETRY;

  do {
    //
    // Debug print packets should not retry indefinitely.  If one times out, clear
    // the mKdDebuggerConnected flag, which will prevent future attempts to print
    // until a state change packet completes successfully.
    //
    if (RetryCount == 0) {
      mKdDebuggerConnected = FALSE;
      return;
    }

    //
    // Setting PacketId has to be in the do loop in case Packet Id was
    // reset.
    //
    PacketHeader.PacketId = mKdNextPacketIdToSend;
    KdSerialSendString ((UINT8 *)&PacketHeader, sizeof (KD_PACKET));

    //
    // Output message header.
    //
    KdSerialSendString (MessageHeader->Buffer, MessageHeader->Length);

    //
    // Output message data.
    //
    if (MessageDataLength != 0) {
      KdSerialSendString (MessageData->Buffer, MessageData->Length);
    }

    //
    // Output a packet trailing byte
    //
    KdSerialWrite (PACKET_TRAILING_BYTE);

    //
    // Wait for the Ack Packet
    //
    ReturnCode = KdTransportReceivePacket (
                   PACKET_TYPE_KD_ACKNOWLEDGE,
                   NULL,
                   NULL,
                   NULL
                   );

    if (ReturnCode == KD_PACKET_TIMEOUT) {
      RetryCount -= 1;
    }
  } while (ReturnCode != KD_PACKET_RECEIVED);

  //
  // The debugger must have been connected to reach here.  Set the connected
  // flag, which will resume prints through the debugger.
  //
  mKdDebuggerConnected = TRUE;
  mKdComPacketsSent   += 1;

  //
  // Reset Sync bit in packet id.  The packet we sent may have Sync bit set
  //
  mKdNextPacketIdToSend &= ~SYNC_PACKET_ID;
  return;
}

/**
  This routine determines if the transport library believes a debugger is
  connected.

  @retval TRUE      A debugger is connected.
  @retval FALSE     A debugger is not connected.

**/
BOOLEAN
KdTransportIsDebuggerConnected (
  )
{
  return mKdDebuggerConnected;
}
