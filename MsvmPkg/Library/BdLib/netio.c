/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    netio.c

Abstract:

    This module implements the network transport layer for the kernel debugger.

--*/

#include <EfiNt.h>
#include <kdnetinterface.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <hvgdk_mini.h>

#include "Bd.h"

EFI_KDNET_HOB *KdnetHob;
KD_CONTEXT BdContext = {MAXIMUM_RETRIES, FALSE};

PHYSICAL_ADDRESS
BdNetGetPhysicalAddress (
    _In_ PVOID Va
    )
/*++

Routine Description:

    This routine is used internally by the Net debug transport library that is
    shared between the boot and kernel environments.  It provides address
    translation of a virtual address to a physical address.

Arguments:

    VirtualAddress - Supplies the virtual address to translate.

Return Value:

    The physical address corresponding to the given virtual address.

--*/
{
    return (ULONG_PTR)Va;
}

VOID
BdNetStallExecutionProcessor (
    ULONG Microseconds
    )

/*++

Routine Description:

    Stall for the specified number of microseconds.  This is done using the
    hypervisor reference timer to avoid having to determine the frequency of
    the local timestamp counter.

Arguments:

    Microseconds - Supplies the number of Microseconds to stall.

Return Value:

    None.

--*/

{
    ULONG64 StartTime;
    ULONG64 WaitTime;

    //
    // Stall for the specified number of microseconds.
    //

    WaitTime = 10;
    WaitTime *= Microseconds;
    StartTime = AsmReadMsr64(HV_X64_MSR_TIME_REF_COUNT);
    do {
    } while (AsmReadMsr64(HV_X64_MSR_TIME_REF_COUNT) - StartTime < WaitTime);

    return;
}

VOID
BdNetSetDebuggerNotPresent (
    __in BOOLEAN NotPresent
    )

/*++

Routine Description:

    This function is used to set the BdDebuggerNotPresent boot environment
    boolean.

Arguments:

    NotPresent - Supplies a boolean indicating whether the debugger is present
        or not.

Return Value:

    None.

--*/

{

    BdDebuggerNotPresent = (LOGICAL)NotPresent;
    return;
}

NTSTATUS
BdNetConfigureDebuggerDevice (
    __in PBD_CONNECTION_PARAMETERS Parameters
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
    KDNET_IMPORTS imports;

    //
    // Capture the debugger HOB and initialize the transport.
    //

    KdnetHob = Parameters->Net.TransportHob;

    ZeroMem(&imports, sizeof(KDNET_IMPORTS));
    imports.GetPhysicalAddress = BdNetGetPhysicalAddress;
    imports.StallExecutionProcessor = BdNetStallExecutionProcessor;
    imports.SetDebuggerNotPresent = BdNetSetDebuggerNotPresent;

    KdnetHob->InitializeLibrary(&imports);

    return KdnetHob->InitializeDebugging();
}


VOID
BdNetSendPacket (
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
    KdnetHob->SendPacket(PacketType, MessageHeader, MessageData, &BdContext);
}


UINT32
BdNetReceivePacket (
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

    N.B. If caller is BdrintString, the parameter PacketType is
         PACKET_TYPE_KD_ACKNOWLEDGE.  In this case, this routine will return
         right after the ack packet is received.

Arguments:

    PacketType - Supplies the type of packet that is expected.

    MessageHeader - Supplies a pointer to a string descriptor for the input
        message.

    MessageData - Supplies a pointer to a string descriptor for the input data.

    DataLength - Supplies pointer to UINT32 to receive length of recv. data.

Return Value:

    Bd_PACKET_RESEND - if resend is required.

    Bd_PAKCET_TIMEOUT - if timeout.

    Bd_PACKET_RECEIVED - if packet received.

--*/
{
    ULONG ReturnCode;

    BdContext.KdpControlCPending = (BOOLEAN)BdControlCPending;
    ReturnCode = KdnetHob->ReceivePacket(PacketType,
                                         MessageHeader,
                                         MessageData,
                                         DataLength,
                                         &BdContext);

    BdControlCPending = BdContext.KdpControlCPending;
    return ReturnCode;
}


ULONG64
BdNetSentReceivedPacketCount (
    VOID
    )

/*++

Routine Description:

    Return the number of KD packets sent and received over the Net transport.

Arguments:

    None.

Return Value:

    This routine returns the total number of successfully sent KD packets in
    the top 32 bits, and the total number of successfully received KD packets in
    the bottom 32 bits.

--*/

{

    ULONG64 PacketCount;

    PacketCount = ((ULONG64)KdnetHob->GetSentPacketCount() << 32) |
                   KdnetHob->GetReceivedPacketCount();

    return PacketCount;
}
