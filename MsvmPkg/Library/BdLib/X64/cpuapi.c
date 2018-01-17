/*++

Copyright (c) 2004  Microsoft Corporation

Module Name:

    cpuapi.c

Abstract:

    This module implements CPU specific remote debug APIs.

Author:

    Jamie Schwartz (jamschw) 15-Jun-2004 - Based on the x86 implementation in
        boot\environ\lib\Bd\i386\cpuapi.c.

Environment:

    Boot

--*/

#include <Library/DebugLib.h>
#include "Bd.h"

#define END_OF_CONTROL_SPACE ((PCHAR)(sizeof(KPROCESSOR_STATE)))

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
    // Special registers for the amd64.
    //

    WaitStateChange->ControlReport.Dr6 =
        BdPrcb->ProcessorState.SpecialRegisters.KernelDr6;

    WaitStateChange->ControlReport.Dr7 =
        BdPrcb->ProcessorState.SpecialRegisters.KernelDr7;

    WaitStateChange->ControlReport.SegCs = (USHORT)(ContextRecord->SegCs);
    WaitStateChange->ControlReport.SegDs = (USHORT)(ContextRecord->SegDs);
    WaitStateChange->ControlReport.SegEs = (USHORT)(ContextRecord->SegEs);
    WaitStateChange->ControlReport.SegFs = (USHORT)(ContextRecord->SegFs);
    WaitStateChange->ControlReport.EFlags = ContextRecord->EFlags;
    WaitStateChange->ControlReport.ReportFlags = X86_REPORT_INCLUDES_SEGS;
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

    if (NT_SUCCESS(ManipulateState->u.Continue2.ContinueStatus) != FALSE) 
    {
        //
        // Set or clear the TF flag in the EFLAGS field of the context record.
        //

        if (ManipulateState->u.Continue2.ControlSet.TraceFlag != FALSE) 
        {
            ContextRecord->EFlags |= EFLAGS_TF_MASK;
        } 
        else 
        {
            ContextRecord->EFlags &= ~EFLAGS_TF_MASK;
        }

        //
        // Clear DR6 and set the specified DR7 value for each of the processors.
        //

        BdPrcb->ProcessorState.SpecialRegisters.KernelDr6 = 0;
        BdPrcb->ProcessorState.SpecialRegisters.KernelDr7 =
                              ManipulateState->u.Continue2.ControlSet.Dr7;
    }

    return;
}


VOID
BdSetStateChange (
    __in PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    __in PEXCEPTION_RECORD ExceptionRecord,
    __in PCONTEXT ContextRecord
    )
/*++

Routine Description:

    Fill in the wait state change message record.

Arguments:

    WaitStateChange - Supplies pointer to record to fill in

    ExceptionRecord - Supplies a pointer to an exception record.

    ContextRecord - Supplies a pointer to a context record.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER( ExceptionRecord);

    BdSetContextState(WaitStateChange, ContextRecord);
    return;
}

VOID
BdReadControlSpace (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData,
    __in PCONTEXT Context
    )
/*++

Routine Description:

    This function reads implementation specific system data for the specified
    processor.

Arguments:

    m - Supplies a pointer to the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/
{
    PDBGKD_READ_MEMORY64 a = &m->u.ReadMemory;
    UINT32 Length;
    STRING MessageHeader;

    UNREFERENCED_PARAMETER(Context);

    ASSERT(AdditionalData->MaximumLength == BD_MESSAGE_BUFFER_SIZE);
    __analysis_assume(AdditionalData->MaximumLength == BD_MESSAGE_BUFFER_SIZE);

    //
    // If the specified control registers are within control space, then
    // read the specified space and return a success status. Otherwise,
    // return an unsuccessful status.
    //

    Length = min(a->TransferCount,
                 PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64));

    ASSERT(sizeof(PVOID) == sizeof(UINT_PTR));

    //
    // Case on address to determine what part of Control space is being read.
    //

    switch ((UINT_PTR)a->TargetBaseAddress) 
    {
        //
        // Return the pcr address for the current processor.
        //

    case DEBUG_CONTROL_SPACE_PCR:
        *(PKPCR *)(AdditionalData->Buffer) =
            (PKPCR)CONTAINING_RECORD(BdPrcb, KPCR, Prcb);

        AdditionalData->Length = sizeof( PKPCR);
        a->ActualBytesRead = AdditionalData->Length;
        m->ReturnStatus = STATUS_SUCCESS;
        break;

        //
        // Return the prcb address for the current processor.
        //

    case DEBUG_CONTROL_SPACE_PRCB:
        *(PKPRCB *)(AdditionalData->Buffer) = BdPrcb;
        AdditionalData->Length = sizeof( PKPRCB);
        a->ActualBytesRead = AdditionalData->Length;
        m->ReturnStatus = STATUS_SUCCESS;
        break;

    case DEBUG_CONTROL_SPACE_KSPECIAL:
        Length = min(Length, sizeof(KSPECIAL_REGISTERS));
        BdMoveMemory (AdditionalData->Buffer,
                      (PVOID)&(BdPrcb->ProcessorState.SpecialRegisters),
                      Length);

        AdditionalData->Length = (USHORT)Length;
        a->ActualBytesRead = AdditionalData->Length;
        m->ReturnStatus = STATUS_SUCCESS;
        break;

    default:
        AdditionalData->Length = 0;
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        a->ActualBytesRead = 0;
        break;
    }

    //
    // Send reply packet.
    //

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                    &MessageHeader,
                    AdditionalData);

    return;
}

VOID
BdWriteControlSpace (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    )
/*++

Routine Description:

    This function writes control space.

Arguments:

    m - Supplies a pointer to the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/
{
    PDBGKD_WRITE_MEMORY64 a = &m->u.WriteMemory;
    UINT32 Length;
    STRING MessageHeader;

    UNREFERENCED_PARAMETER(Context);

    Length = min(a->TransferCount, AdditionalData->Length);

    //
    // If the specified control registers are within control space, then
    // write the specified space and return a success status. Otherwise,
    // return an unsuccessful status.
    //

    switch ( (UINT_PTR)a->TargetBaseAddress ) 
    {
    case DEBUG_CONTROL_SPACE_KSPECIAL:
        Length = min(Length, sizeof(KSPECIAL_REGISTERS));
        BdMoveMemory((PVOID)&(BdPrcb->ProcessorState.SpecialRegisters),
                         AdditionalData->Buffer,
                         Length);

        AdditionalData->Length = (USHORT)Length;
        a->ActualBytesWritten = AdditionalData->Length;
        m->ReturnStatus = STATUS_SUCCESS;
        break;

    default:
        AdditionalData->Length = 0;
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        a->ActualBytesWritten = 0;
        break;
    }

    //
    // Send reply message.
    //

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                    &MessageHeader,
                    NULL);

    return;
}

VOID
BdReadIoSpace (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    )
/*++

Routine Description:

    This function reads I/O space.

Arguments:

    m - Supplies a pointer to the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/
{
//    PDBGKD_READ_WRITE_IO64 a = &m->u.ReadWriteIo;
    STRING MessageHeader;

    UNREFERENCED_PARAMETER(AdditionalData);
    UNREFERENCED_PARAMETER(Context);

    //
    // Case of data size and check alignment.
    //
#if NEED_TO_DO
    m->ReturnStatus = STATUS_SUCCESS;
    switch (a->DataSize) {
        case 1:
            a->DataValue = (UINT32)BL_READ_PORT_UCHAR((PUCHAR)a->IoAddress);
            break;

        case 2:
            if (((UINT32)a->IoAddress & 1) != 0) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                a->DataValue = (UINT32)BL_READ_PORT_USHORT((PUSHORT)a->IoAddress);
            }

            break;

        case 4:
            if (((UINT32)a->IoAddress & 3) != 0) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                a->DataValue = BL_READ_PORT_ULONG((PUINT32)a->IoAddress);
            }

            break;

        default:
            m->ReturnStatus = STATUS_INVALID_PARAMETER;
            break;
    }
#endif
    //
    // Send reply packet.
    //

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                    &MessageHeader,
                    NULL);

    return;
}

VOID
BdWriteIoSpace (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    )

/*++

Routine Description:

    This function wrties I/O space.

Arguments:

    m - Supplies a pointer to the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/

{

//    PDBGKD_READ_WRITE_IO64 a = &m->u.ReadWriteIo;
    STRING MessageHeader;

    UNREFERENCED_PARAMETER(AdditionalData);
    UNREFERENCED_PARAMETER(Context);

    //
    // Case on data size and check alignment.
    //
#if NEED_TO_DO
    m->ReturnStatus = STATUS_SUCCESS;
    switch (a->DataSize) {
        case 1:
            BL_WRITE_PORT_UCHAR((PUCHAR)a->IoAddress, (UCHAR)a->DataValue);
            break;

        case 2:
            if (((UINT32)a->IoAddress & 1) != 0) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                BL_WRITE_PORT_USHORT((PUSHORT)a->IoAddress, (USHORT)a->DataValue);
            }

            break;

        case 4:
            if (((UINT32)a->IoAddress & 3) != 0) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                BL_WRITE_PORT_ULONG((PUINT32)a->IoAddress, a->DataValue);
            }

            break;

        default:
            m->ReturnStatus = STATUS_INVALID_PARAMETER;
            break;
    }
#endif
    //
    // Send reply packet.
    //

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                    &MessageHeader,
                    NULL);

    return;
}
