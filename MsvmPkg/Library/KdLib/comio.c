/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    comio.c

Abstract:

    This module implements the I/O comunications for the portable kernel
    debugger.

Author:

    David N. Cutler 27-July-1990

--*/

// ------------------------------------------------------------------- Includes

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>

#include "cp.h"

UINT32 KdComPacketsReceived = 0;
UINT32 KdComPacketsSent = 0;
CP_PORT CpPort;

// -------------------------------------------------------------------- Defines

// ----------------------------------------------------------------- Prototypes

UINT32
EfiKdpGetByte (
    __out_bcount(sizeof(UINT8)) PUINT8 Input
    );

UINT32
EfiKdpComputeChecksum (
    __in_bcount(Length) PUINT8 Buffer,
    __in UINT32 Length
    );

UINT32
EfiKdpPollByte (
    __out_ecount(1) PUINT8 Input
    );

VOID
EfiKdpPutByte (
    __in UINT8 Output
    );

USHORT
EfiKdpReceivePacketLeader (
    __in UINT32 PacketType,
    __out PUINT32 PacketLeader
    );

UINT32
EfiKdpReceiveString (
    __out_bcount(Length) PCHAR Destination,
    __in UINT32 Length
    );

VOID
EfiKdpSendControlPacket(
    __in USHORT PacketType,
    __in UINT32 PacketId
    );

VOID
EfiKdpSendString (
    __in_bcount(Length) PCHAR Source,
    __in UINT32 Length
    );

// ------------------------------------------------------------------ Functions

VOID
EfiKdComCloseDebuggerDevice (
    VOID
    )
/*++

Routine Description:

    This routine closes the debugger device.

Arguments:

    None.  State about the debugger device to close is kept globally.

Return Value:

    None.

--*/
{
    return;
}


NTSTATUS
EfiKdComConfigureDebuggerDevice (
    __in PEFI_KD_CONNECTION_PARAMETERS Parameters
    )
/*++

Routine Description:

    This routine configures the debugger device described by the given
    parameters.

Arguments:

    Parameters - Supplies information about the type of debugger device and the
        settings to be used.

Return Value:

    NT Status code.

--*/
{
    NTSTATUS status;
    CP_PORT_ADDRESS portAddress;

    ASSERT(Parameters->Type == EfiKdSerial);

    status = STATUS_SUCCESS;
    ZeroMem(&CpPort, sizeof(CP_PORT));
    portAddress.Type = CpPortTypeIoPort;
    portAddress.IoPort = (UINT16)Parameters->SerialPort.AlternatePortNumber;
    CpPortInit(&CpPort,
               &portAddress,
               Parameters->SerialPort.BaudRate,
               CpMcIgnoreRings);

    return status;
}


NTSTATUS
EfiKdComGetConnectionParameters (
    __out PEFI_KD_CONNECTION_PARAMETERS Parameters
    )
/*++

Routine Deescriptoin:

    This routine returns the parameters to be used to configure the debugging
    device, along with the type of debugging device to be used.

Arguments:

    Parameters - Supplies a pointer to a variable that receives the connection
        parameters.

Return Value:

    STATUS_SUCCESS if the parameters are retrieved successfully.

    STATUS_INVALID_PARAMETER if the application options do not contain valid
        connection parameter data.

--*/
{

    UINT64 baudRate;

    //
    // Initialize the connection parameters for the default serial debugger device.
    //

    baudRate = EFI_KD_115200;
    Parameters->Type = EfiKdSerial;
    Parameters->SerialPort.AlternatePortNumber = COM1_PORT;
    Parameters->SerialPort.BaudRate = baudRate;

    return STATUS_SUCCESS;
}


UINT32
EfiKdComReceivePacket (
    __in UINT32 PacketType,
    __out_opt PSTRING MessageHeader,
    __out_opt PSTRING MessageData,
    __out_opt PUINT32 DataLength
    )
/*++

Routine Description:

    This routine receives a packet from the host machine that is running
    the kernel debugger UI.  This routine is ALWAYS called after packet being
    sent by caller.  It first waits for ACK packet for the packet sent and
    then waits for the packet desired.

    N.B. If caller is EfiKdrintString, the parameter PacketType is
         PACKET_TYPE_KD_ACKNOWLEDGE.  In this case, this routine will return
         right after the ack packet is received.

Arguments:

    PacketType - Supplies the type of packet that is excepted.

    MessageHeader - Supplies a pointer to a string descriptor for the input
        message.

    MessageData - Supplies a pointer to a string descriptor for the input data.

    DataLength - Supplies pointer to UINT32 to receive length of recv. data.

Return Value:

    EfiKd_PACKET_RESEND - if resend is required.

    EfiKd_PAKCET_TIMEOUT - if timeout.

    EfiKd_PACKET_RECEIVED - if packet received.

--*/
{
    UINT32 checksum;
    UINT8 input;
    UINT32 messageLength;
    KD_PACKET packetHeader;
    UINT32 returnCode;
    UINT32 status;

    ZeroMem(&packetHeader, sizeof(KD_PACKET));

    //
    // A breakin packet is a single byte that is requested frequently.  To avoid
    // spending significant time waiting for this packet, look for this single
    // byte and exit early if this packet type is being requested.  Do not
    // increment KdComPacketsReceived for a breakin byte, since there is no
    // validation of a full packet performed.  We only want to require the
    // disconnect message to be sent when full packets were successfully sent
    // or received.
    //

    if (PacketType == PACKET_TYPE_KD_POLL_BREAKIN)
    {
        status = EfiKdpPollByte(&input);
        if ((status == CP_GET_SUCCESS) &&
            (input == BREAKIN_PACKET_BYTE))
        {
            return EFI_KD_PACKET_RECEIVED;
        }
        else
        {
            return EFI_KD_PACKET_TIMEOUT;
        }
    }

WaitForPacketLeader:

    //
    // Read Packet Leader
    //

    returnCode = EfiKdpReceivePacketLeader(PacketType, &packetHeader.PacketLeader);

    //
    // If we can successfully read packet leader, it has high possibility that
    // kernel debugger is alive.  So reset count.
    //

    if (returnCode != EFI_KD_PACKET_TIMEOUT)
    {
        EfiKdNumberRetries = EfiKdRetryCount;
    }

    if (returnCode != EFI_KD_PACKET_RECEIVED)
    {
        return returnCode;
    }

    //
    // Read packet type.
    //

    returnCode = EfiKdpReceiveString((PCHAR)&packetHeader.PacketType,
                                     sizeof(packetHeader.PacketType));

    if (returnCode == CP_GET_NODATA)
    {
        return EFI_KD_PACKET_TIMEOUT;
    }
    else if (returnCode == CP_GET_ERROR)
    {
        if (packetHeader.PacketLeader == CONTROL_PACKET_LEADER)
        {
            //
            // If read error and it is for a control packet, simply
            // preptend that we have not seen this packet.  Hopefully
            // we will receive the packet we desire which automatically acks
            // the packet we just sent.
            //

            goto WaitForPacketLeader;
        }
        else
        {
            //
            // if read error while reading data packet, we have to ask
            // kernel debugger to resend us the packet.
            //

            goto SendResendPacket;
        }
    }

    //
    // if the packet we received is a resend request, we return true and
    // let caller resend the packet.
    //

    if ((packetHeader.PacketLeader == CONTROL_PACKET_LEADER) &&
        (packetHeader.PacketType == PACKET_TYPE_KD_RESEND))
    {
        return EFI_KD_PACKET_RESEND;
    }

    //
    // Read data length.
    //

    returnCode = EfiKdpReceiveString((PCHAR)&packetHeader.ByteCount,
                                     sizeof(packetHeader.ByteCount));

    if (returnCode == CP_GET_NODATA)
    {
        return EFI_KD_PACKET_TIMEOUT;
    }
    else if (returnCode == CP_GET_ERROR)
    {
        if (packetHeader.PacketLeader == CONTROL_PACKET_LEADER)
        {
            goto WaitForPacketLeader;
        }
        else
        {
            goto SendResendPacket;
        }
    }

    //
    // Read Packet Id.
    //

    returnCode = EfiKdpReceiveString((PCHAR)&packetHeader.PacketId,
                                     sizeof(packetHeader.PacketId));

    if (returnCode == CP_GET_NODATA)
    {
        return EFI_KD_PACKET_TIMEOUT;
    }
    else if (returnCode == CP_GET_ERROR)
    {
        if (packetHeader.PacketLeader == CONTROL_PACKET_LEADER)
        {
            goto WaitForPacketLeader;
        }
        else
        {
            goto SendResendPacket;
        }
    }

    //
    // Read packet checksum.
    //

    returnCode = EfiKdpReceiveString((PCHAR)&packetHeader.Checksum,
                                     sizeof(packetHeader.Checksum));

    if (returnCode == CP_GET_NODATA)
    {
        return EFI_KD_PACKET_TIMEOUT;
    }
    else if (returnCode == CP_GET_ERROR)
    {
        if (packetHeader.PacketLeader == CONTROL_PACKET_LEADER)
        {
            goto WaitForPacketLeader;
        }
        else
        {
            goto SendResendPacket;
        }
    }

    //
    // A complete packet header is received.  Check its validity and
    // perform appropriate action depending on packet type.
    //

    if (packetHeader.PacketLeader == CONTROL_PACKET_LEADER)
    {
        if (packetHeader.PacketType == PACKET_TYPE_KD_ACKNOWLEDGE)
        {
            //
            // If we received an expected ACK packet and we are not
            // waiting for any new packet, update outgoing packet id
            // and return.  If we are NOT waiting for ACK packet
            // we will keep on waiting.  If the ACK packet
            // is not for the packet we send, ignore it and keep on waiting.
            //

            if (packetHeader.PacketId !=
                (EfiKdNextPacketIdToSend & ~SYNC_PACKET_ID))
            {
                goto WaitForPacketLeader;
            }
            else if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE)
            {
                EfiKdNextPacketIdToSend ^= 1;
                return EFI_KD_PACKET_RECEIVED;
            }
            else
            {
                goto WaitForPacketLeader;
            }
        }
        else if (packetHeader.PacketType == PACKET_TYPE_KD_RESET)
        {
            //
            // if we received Reset packet, reset the packet control variables
            // and resend earlier packet.
            //

            EfiKdNextPacketIdToSend = INITIAL_PACKET_ID;
            EfiKdPacketIdExpected = INITIAL_PACKET_ID;
            EfiKdpSendControlPacket(PACKET_TYPE_KD_RESET, 0L);
            return EFI_KD_PACKET_RESEND;
        }
        else if (packetHeader.PacketType == PACKET_TYPE_KD_RESEND)
        {
            return EFI_KD_PACKET_RESEND;
        }
        else
        {
            //
            // Invalid packet header, ignore it.
            //

            goto WaitForPacketLeader;
        }

    //
    // The packet header is for data packet (not control packet).
    //

    }
    else if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE)
    {
        //
        // if we are waiting for ACK packet ONLY
        // and we receive a data packet header, check if the packet id
        // is what we expected.  If yes, assume the acknowledge is lost (but
        // sent), ask sender to resend and return with PACKET_RECEIVED.
        //

        if (packetHeader.PacketId == EfiKdPacketIdExpected)
        {
            EfiKdpSendControlPacket(PACKET_TYPE_KD_RESEND, 0L);
            EfiKdNextPacketIdToSend ^= 1;
            return EFI_KD_PACKET_RECEIVED;

        }
        else
        {
            EfiKdpSendControlPacket(PACKET_TYPE_KD_ACKNOWLEDGE,
                                    packetHeader.PacketId);
            goto WaitForPacketLeader;
        }
    }

    //
    // we are waiting for data packet and we received the packet header
    // for data packet. Perform the following checkings to make sure
    // it is the packet we are waiting for.
    //
    // Check ByteCount received is valid
    //

    messageLength = MessageHeader->MaximumLength;
    if ((packetHeader.ByteCount > (USHORT)PACKET_MAX_SIZE) ||
        (packetHeader.ByteCount < (USHORT)messageLength))
    {
        goto SendResendPacket;
    }

    *DataLength = packetHeader.ByteCount - messageLength;

    //
    // Read the message header.
    //

    returnCode = EfiKdpReceiveString(MessageHeader->Buffer, messageLength);
    if (returnCode != CP_GET_SUCCESS)
    {
        goto SendResendPacket;
    }

    MessageHeader->Length = (USHORT)messageLength;

    //
    // Read the message data.
    //

    returnCode = EfiKdpReceiveString(MessageData->Buffer, *DataLength);
    if (returnCode != CP_GET_SUCCESS)
    {
        goto SendResendPacket;
    }

    MessageData->Length = (USHORT)*DataLength;

    //
    // Read packet trailing byte
    //

    returnCode = EfiKdpGetByte(&input);
    if (returnCode != CP_GET_SUCCESS || input != PACKET_TRAILING_BYTE)
    {
        goto SendResendPacket;
    }

    //
    // Check PacketType is what we are waiting for.
    //

    if (PacketType != packetHeader.PacketType)
    {
        EfiKdpSendControlPacket(PACKET_TYPE_KD_ACKNOWLEDGE, packetHeader.PacketId);
        goto WaitForPacketLeader;
    }

    //
    // Check PacketId is valid.
    //

    if ((packetHeader.PacketId == INITIAL_PACKET_ID) ||
        (packetHeader.PacketId == (INITIAL_PACKET_ID ^ 1)))
    {
        if (packetHeader.PacketId != EfiKdPacketIdExpected)
        {
            EfiKdpSendControlPacket(PACKET_TYPE_KD_ACKNOWLEDGE,
                                    packetHeader.PacketId);

            goto WaitForPacketLeader;
        }
    }
    else
    {
        goto SendResendPacket;
    }

    //
    // Check checksum is valid.
    //

    checksum = EfiKdpComputeChecksum((PUINT8)MessageHeader->Buffer,
                                     MessageHeader->Length);

    checksum += EfiKdpComputeChecksum((PUINT8)MessageData->Buffer,
                                      MessageData->Length);

    if (checksum != packetHeader.Checksum)
    {
        goto SendResendPacket;
    }

    //
    // Send Acknowledge byte and the Id of the packet received.
    // Then, update the ExpectId for next incoming packet.
    //

    EfiKdpSendControlPacket(PACKET_TYPE_KD_ACKNOWLEDGE, packetHeader.PacketId);

    //
    // We have successfully received the packet so update the
    // packet control variables and return sucess.
    //

    KdComPacketsReceived += 1;
    EfiKdPacketIdExpected ^= 1;
    return EFI_KD_PACKET_RECEIVED;

SendResendPacket:
    EfiKdpSendControlPacket(PACKET_TYPE_KD_RESEND, 0L);
    goto WaitForPacketLeader;
}


VOID
EfiKdComSendPacket (
    __in UINT32 PacketType,
    __in PSTRING MessageHeader,
    __in_opt PSTRING MessageData
    )
/*++

Routine Description:

    This routine sends a packet to the host machine that is running the
    kernel debugger and waits for an ACK.

Arguments:

    PacketType - Supplies the type of packet to send.

    MessageHeader - Supplies a pointer to a string descriptor that describes
        the message information.

    MessageData - Supplies a pointer to a string descriptor that describes
        the optional message data.

Return Value:

    None.

--*/
{
    PDBGKD_DEBUG_IO debugIo;
    PDBGKD_FILE_IO fileIo;
    UINT32 messageDataLength;
    KD_PACKET packetHeader;
    UINT32 returnCode;

    if (ARGUMENT_PRESENT(MessageData))
    {
        messageDataLength = MessageData->Length;
        packetHeader.Checksum = EfiKdpComputeChecksum((PUINT8)MessageData->Buffer,
                                                   MessageData->Length);
    }
    else
    {
        messageDataLength = 0;
        packetHeader.Checksum = 0;
    }

    packetHeader.Checksum += EfiKdpComputeChecksum((PUINT8)MessageHeader->Buffer,
                                                   MessageHeader->Length);

    //
    // Initialize and send the packet header.
    //

    packetHeader.PacketLeader = PACKET_LEADER;
    packetHeader.ByteCount = (USHORT)(MessageHeader->Length + messageDataLength);
    packetHeader.PacketType = (USHORT)PacketType;
    EfiKdNumberRetries = EfiKdRetryCount;

    do
    {
        if (EfiKdNumberRetries == 0)
        {
            //
            // If the packet is not for reporting exception, we give up
            // and declare debugger not present.  Note that we require state
            // change packets to be sent in the boot applications.  This
            // ensures that the unload symbols message that gets sent when the
            // application shuts down, gets through to the host debugger.  That
            // ensures the debugger properly disconnects from the current
            // application and prepares to reconnect cleanly to the next code
            // that has debugging enabled.
            //

            if (PacketType == PACKET_TYPE_KD_DEBUG_IO)
            {
                debugIo = (PDBGKD_DEBUG_IO)MessageHeader->Buffer;
                if (debugIo->ApiNumber == DbgKdPrintStringApi)
                {
                    EfiKdDebuggerNotPresent = TRUE;
                    EfiKdNextPacketIdToSend = INITIAL_PACKET_ID | SYNC_PACKET_ID;
                    EfiKdPacketIdExpected = INITIAL_PACKET_ID;
                    return;
                }
            }
            else if (PacketType == PACKET_TYPE_KD_FILE_IO)
            {
                fileIo = (PDBGKD_FILE_IO)MessageHeader->Buffer;
                if (fileIo->ApiNumber == DbgKdCreateFileApi)
                {
                    EfiKdDebuggerNotPresent = TRUE;
                    EfiKdNextPacketIdToSend = INITIAL_PACKET_ID | SYNC_PACKET_ID;
                    EfiKdPacketIdExpected = INITIAL_PACKET_ID;
                    return;
                }
            }
        }

        //
        // Setting PacketId has to be in the do loop in case Packet Id was
        // reset.
        //

        packetHeader.PacketId = EfiKdNextPacketIdToSend;
        EfiKdpSendString((PCHAR)&packetHeader, sizeof(KD_PACKET));

        //
        // Output message header.
        //

        EfiKdpSendString(MessageHeader->Buffer, MessageHeader->Length);

        //
        // Output message data.
        //
        if (messageDataLength != 0)
        {
            EfiKdpSendString(MessageData->Buffer, MessageData->Length);
        }

        //
        // Output a packet trailing byte
        //

        EfiKdpPutByte(PACKET_TRAILING_BYTE);

        //
        // Wait for the Ack Packet
        //

        returnCode = EfiKdReceivePacket(PACKET_TYPE_KD_ACKNOWLEDGE,
                                        NULL,
                                        NULL,
                                        NULL);

        if (returnCode == EFI_KD_PACKET_TIMEOUT)
        {
            EfiKdNumberRetries -= 1;
        }
    } while (returnCode != EFI_KD_PACKET_RECEIVED);

    KdComPacketsSent += 1;

    //
    // Reset Sync bit in packet id.  The packet we sent may have Sync bit set
    //

    EfiKdNextPacketIdToSend &= ~SYNC_PACKET_ID;

    //
    // Since we are able to talk to debugger, the retrycount is set to
    // maximum value.
    //

    EfiKdRetryCount = MAXIMUM_RETRIES;

    return;
}


UINT64
EfiKdComSentReceivedPacketCount(
    VOID
    )
/*++

Routine Description:

    Return the number of KD packets sent and received over the serial transport.

Arguments:

    None.

Return Value:

    This routine returns the total number of successfully sent KD packets in
    the top 32 bits, and the total number of successfully received KD packets in
    the bottom 32 bits.

--*/
{
    UINT64 packetCount;

    packetCount = ((UINT64)KdComPacketsSent << 32) | KdComPacketsReceived;
    return packetCount;
}

// --------------------------------------------------------- Internal Functions

UINT32
EfiKdpComputeChecksum (
    __in_bcount(Length) PUINT8 Buffer,
    __in UINT32 Length
    )
/*++

Routine Description:

    This routine computes the checksum of the specified buffer.

Arguments:

    Buffer - Supplies a pointer to the buffer.

    Length - Supplies the length of the buffer.

Return Value:

    A UINT32 is return as the checksum for the input string.

--*/
{
    UINT32 checksum;

    checksum = 0;
    while (Length > 0)
    {
        checksum = checksum + (UINT32)*Buffer++;
        Length--;
    }

    return checksum;
}


UINT32
EfiKdpGetByte (
    __out_bcount(sizeof(UINT8)) PUINT8 Input
    )
/*++

Routine Description:

    Reads one byte from a serial port.

    The old loader library defined the routine BlPortGetByte to get a single
    byte from a serial port.  This routine provides the same functionality
    and minimizes the changes required for the boot debugger library.

Arguments:

    Input - Returns the data byte.

Return Value:

    CP_GET_SUCCESS is returned if a byte is successfully read from the
        kernel debugger line.

    CP_GET_ERROR is returned if error encountered during reading.
    CP_GET_NODATA is returned if timeout.

--*/
{
    return (UINT32)CpPortRead(&CpPort, Input, TRUE);
}


UINT32
EfiKdpPollByte (
    __out_bcount(sizeof(UINT8)) PUINT8 Input
    )
/*++

Routine Description:

    Fetch a byte from the port and return it if one is available.

Arguments:

    BlFileId - The port to poll.

    Input - Returns the data byte.

Return Value:

    CP_GET_SUCCESS is returned if a byte is successfully read.
    CP_GET_ERROR is returned if error encountered during reading.
    CP_GET_NODATA is returned if timeout.

--*/
{
    return (UINT32)CpPortRead(&CpPort, Input, FALSE);
}


VOID
EfiKdpPutByte (
    __in UINT8 Output
    )
/*++

Routine Description:

    Writes one byte to a serial port.

    The old loader library defined the routine BlPortPutByte to write a single
    byte to a serial port.  This routine provides the same functionality
    and minimizes the changes required for the boot debugger library.

Arguments:

    DebuggerId - Device ID for the serial port to write.

    Output - Data byte to write to the serial port.

Return Value:

    None.

--*/
{
    CpPortWrite(&CpPort, Output, TRUE);
}


USHORT
EfiKdpReceivePacketLeader (
    __in UINT32 PacketType,
    __out PUINT32 PacketLeader
    )
/*++

Routine Description:

    This routine waits for a packet header leader.

Arguments:

    PacketType - Supplies the type of packet we are expecting.

    PacketLeader - Supplies a pointer to a UINT32 variable to receive
                   packet leader bytes.

Return Value:

    EfiKd_PACKET_RESEND - if resend is required.

    EfiKd_PAKCET_TIMEOUT - if timeout.

    EfiKd_PACKET_RECEIVED - if packet received.

--*/
{

    BOOLEAN breakinDetected;
    UINT32 index;
    UINT8 input;
    UINT8 previousByte;
    UINT32 returnCode;

    UNREFERENCED_PARAMETER(PacketType);

    breakinDetected = FALSE;
    previousByte = 0;

    //
    // NOTE - With all the interrupts being off, it is very hard
    // to implement the actual timeout code. (Maybe, by reading the CMOS.)
    // Here we use a loop count to wait about 3 seconds.  The CpGetByte
    // will return with error code = CP_GET_NODATA if it cannot find data
    // byte within 1 second. Kernel debugger's timeout period is 5 seconds.
    //

    index = 0;
    do
    {
        returnCode = EfiKdpGetByte(&input);
        if (returnCode == CP_GET_NODATA)
        {
            if (breakinDetected)
            {
                EfiKdControlCPending = TRUE;
                return EFI_KD_PACKET_RESEND;
            }
            else
            {
                return EFI_KD_PACKET_TIMEOUT;
            }
        }
        else if (returnCode == CP_GET_ERROR)
        {
            index = 0;
            continue;
        }
        else
        {                    // if (returnCode == CP_GET_SUCCESS)
            if ((input == PACKET_LEADER_BYTE) ||
                (input == CONTROL_PACKET_LEADER_BYTE))
            {
                if (index == 0)
                {
                    previousByte = input;
                    index++;
                }
                else if (input == previousByte)
                {
                    index++;
                }
                else
                {
                    previousByte = input;
                    index = 1;
                }
            }
            else
            {

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

                if (input == BREAKIN_PACKET_BYTE)
                {
                    breakinDetected = TRUE;
                }
                else
                {
                    //
                    // The following statement is ABSOLUTELY necessary.
                    //

                    breakinDetected = FALSE;
                }

                index = 0;
            }
        }

    } while (index < 4);

    if (breakinDetected)
    {
        EfiKdControlCPending = TRUE;
    }

    //
    // Return the packet leader and FALSE to indicate no resend is needed.
    //

    if (input == PACKET_LEADER_BYTE)
    {
        *PacketLeader = PACKET_LEADER;
    }
    else
    {
        *PacketLeader = CONTROL_PACKET_LEADER;
    }

    EfiKdDebuggerNotPresent = FALSE;
    return EFI_KD_PACKET_RECEIVED;
}


VOID
EfiKdpSendControlPacket (
    __in USHORT PacketType,
    __in UINT32 PacketId
    )
/*++

Routine Description:

    This routine sends a control packet to the host machine that is running the
    kernel debugger and waits for an ACK.

Arguments:

    PacketType - Supplies the type of packet to send.

    PacketId - Supplies packet id, optionally.

Return Value:

    None.

--*/
{
    KD_PACKET packetHeader;

    //
    // Initialize and send the packet header.
    //

    packetHeader.PacketLeader = CONTROL_PACKET_LEADER;
    packetHeader.PacketId = PacketId;
    packetHeader.ByteCount = 0;
    packetHeader.Checksum = 0;
    packetHeader.PacketType = PacketType;

    EfiKdpSendString((PCHAR)&packetHeader, sizeof(KD_PACKET));
    return;
}


UINT32
EfiKdpReceiveString (
    __out_bcount(Length) PCHAR Destination,
    __in UINT32 Length
    )
/*++

Routine Description:

    This routine reads a string from the kernel debugger port.

Arguments:

    Destination - Supplies a pointer to the input string.

    Length - Supplies the length of the string to be read.

Return Value:

    CP_GET_SUCCESS is returned if string is successfully read from the
        kernel debugger line.

    CP_GET_ERROR is returned if error encountered during reading.

    CP_GET_NODATA is returned if timeout.

--*/
{
    UINT8 input;
    UINT32 returnCode;

    //
    // Read bytes until either a error is encountered or the entire string
    // has been read.
    //

    while (Length > 0)
    {
        returnCode = EfiKdpGetByte(&input);
        if (returnCode != CP_GET_SUCCESS)
        {
            return returnCode;
        }
        else
        {
            *Destination++ = input;
            Length -= 1;
        }
    }

    return CP_GET_SUCCESS;
}


VOID
EfiKdpSendString (
    __in_bcount(Length) PCHAR Source,
    __in UINT32 Length
    )
/*++

Routine Description:

    This routine writes a string to the kernel debugger port.

Arguments:

    Source - Supplies a pointer to the output string.

    Length - Supplies the length of the string to be written.

Return Value:

    None.

--*/
{
    UINT8 output;

    //
    // Write the message string to the debugger port.
    //

    while (Length > 0)
    {
        output = *Source++;
        EfiKdpPutByte(output);
        Length -= 1;
    }

    return;
}
