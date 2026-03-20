/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    cpuapi.c

Abstract:

    This module implements CPU specific remote debug APIs.

Author:

    Aaron Giles (aarongi) 11-Jan-2013

--*/

#include "bd.h"

#define END_OF_CONTROL_SPACE (sizeof(KPROCESSOR_STATE))

VOID
BdSetContextState (
    __in PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    __in PCONTEXT ContextRecord
    )

/*++

Routine Description:

    The function fills in the processor-specific portions of the wait state
    change message record.

Arguments:

    WaitStateChange - Supplies a pointer to record to fill in.

    ContextRecord - Supplies a pointer to a context record.

Return Value:

    None.

--*/

{

    //
    // Copy special registers for the ARM64
    //

    WaitStateChange->ControlReport.Cpsr = ContextRecord->Cpsr;

    return;
}

VOID
BdGetStateChange (
    __in PDBGKD_MANIPULATE_STATE64 ManipulateState,
    __in PCONTEXT ContextRecord
    )

/*++

Routine Description:

    Extract continuation control data from manipulate state message.

Arguments:

    ManipulateState - Supplies a pointer to the manipulate state packet.

    ContextRecord - Supplies a pointer to a context record.

Return Value:

    None.

--*/

{

    //
    // If the status of the manipulate state message was successful, then
    // extract the continuation control information.
    //

    if (NT_SUCCESS(ManipulateState->u.Continue2.ContinueStatus) != FALSE) {

        //
        // Set or clear the TF flag in the EFLAGS field of the context record.
        //

        if (ManipulateState->u.Continue2.ControlSet.TraceFlag != FALSE) {
            ContextRecord->Cpsr |= CPSR_SS;

        } else {
            ContextRecord->Cpsr &= ~CPSR_SS;

        }

    }

    return;
}

VOID
BdSetStateChange (
    __in PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    __in PCONTEXT ContextRecord
    )
/*++

Routine Description:

    Fill in the wait state change message record.

Arguments:

    WaitStateChange - Supplies pointer to record to fill in

    ContextRecord - Supplies a pointer to a context record.

Return Value:

    None.

--*/
{
    BdSetContextState(WaitStateChange, ContextRecord);
}

VOID
BdReadControlSpace (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData
    )
/*++

Routine Description:

    This function reads implementation specific system data for the specified
    processor.

Arguments:

    m - Supplies a pointer to the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

Return Value:

    None.

--*/
{
    PDBGKD_READ_MEMORY64 a = &m->u.ReadMemory;
    ULONG Length;
    STRING MessageHeader;

    //ASSERT(AdditionalData->MaximumLength == BD_MESSAGE_BUFFER_SIZE);
    __analysis_assume(AdditionalData->MaximumLength == BD_MESSAGE_BUFFER_SIZE);

    //
    // If the specified control registers are within control space, then
    // read the specified space and return a success status. Otherwise,
    // return an unsuccessful status.
    //

    Length = MIN(a->TransferCount,
                 PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64));

    if (((UINTN)a->TargetBaseAddress + Length) <= END_OF_CONTROL_SPACE) {
        BdCopyMemory(AdditionalData->Buffer,
                     Add2Ptr(&BdPrcb->ProcessorState, a->TargetBaseAddress),
                     Length);

        m->ReturnStatus = STATUS_SUCCESS;
        a->ActualBytesRead = Length;
        AdditionalData->Length = (USHORT)Length;

    } else {

        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        a->ActualBytesRead = 0;
        AdditionalData->Length = 0;
    }

    //
    // Send reply packet.
    //

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 AdditionalData);
}

VOID
BdWriteControlSpace (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData
    )
/*++

Routine Description:

    This function writes control space.

Arguments:

    m - Supplies a pointer to the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

Return Value:

    None.

--*/
{
    PDBGKD_WRITE_MEMORY64 a = &m->u.WriteMemory;
    ULONG Length;
    STRING MessageHeader;

    //
    // If the specified control registers are within control space, then
    // write the specified space and return a success status. Otherwise,
    // return an unsuccessful status.
    //

    Length = MIN(a->TransferCount, AdditionalData->Length);
    if (((UINTN)a->TargetBaseAddress + Length) <= END_OF_CONTROL_SPACE) {
        BdCopyMemory(Add2Ptr(&BdPrcb->ProcessorState, a->TargetBaseAddress),
                     AdditionalData->Buffer,
                     Length);

        m->ReturnStatus = STATUS_SUCCESS;
        a->ActualBytesWritten = Length;

    } else {

        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        a->ActualBytesWritten = 0;
    }

    //
    // Send reply message.
    //

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL);
}

VOID
BdReadIoSpace (
    __in PDBGKD_MANIPULATE_STATE64 m
    )
/*++

Routine Description:

    This function reads I/O space.

Arguments:

    m - Supplies a pointer to the state manipulation message.

Return Value:

    None.

--*/
{
    STRING MessageHeader;

    m->ReturnStatus = STATUS_INVALID_PARAMETER;

    //
    // Send reply packet.
    //

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL);
}

VOID
BdWriteIoSpace (
    __in PDBGKD_MANIPULATE_STATE64 m
    )
/*++

Routine Description:

    This function wrties I/O space.

Arguments:

    m - Supplies a pointer to the state manipulation message.

Return Value:

    None.

--*/
{
    STRING MessageHeader;

    //
    // Case on data size and check alignment.
    //

    m->ReturnStatus = STATUS_INVALID_PARAMETER;

    //
    // Send reply packet.
    //

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL);
}
