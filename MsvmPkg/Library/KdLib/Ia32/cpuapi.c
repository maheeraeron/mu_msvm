/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    cpuapi.c

Abstract:

    This module implements CPU specific remote debug APIs.

Author:

    Mark Lucovsky (markl) 04-Sep-1990

Revision History:

--*/

#include <Library/DebugLib.h>
#include "EfiKd.h"

//
// Define end of control space.
//

#define END_OF_CONTROL_SPACE (sizeof(KPROCESSOR_STATE))

#define Add2Ptr(P,I) ((PVOID)((PUCHAR)(P) + (I)))

VOID
EfiKdSetContextState (
    __in PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    __in PCONTEXT ContextRecord
    )

/*++

Routine Description:

    The function fills in the processor-specific portions of
    the wait state change message record.

Arguments:

    WaitStateChange - Supplies a pointer to record to fill in.

    ContextRecord - Supplies a pointer to a context record.

Return Value:

    None.

--*/

{
    //
    // Special registers for the x86.
    //

    //
    // TODO
    // I commented out the Dr7 assignment because it breaks stepping with the debugger.
    // I'm not sure that the code is still correct, and I will probably break in some cases.
    //


    WaitStateChange->ControlReport.Dr6 =
        EfiKdPrcb->ProcessorState.SpecialRegisters.KernelDr6;
//    WaitStateChange->ControlReport.Dr7 =
//        EfiKdPrcb->ProcessorState.SpecialRegisters.KernelDr7;
    WaitStateChange->ControlReport.SegCs = (USHORT)(ContextRecord->SegCs);
    WaitStateChange->ControlReport.SegDs = (USHORT)(ContextRecord->SegDs);
    WaitStateChange->ControlReport.SegEs = (USHORT)(ContextRecord->SegEs);
    WaitStateChange->ControlReport.SegFs = (USHORT)(ContextRecord->SegFs);
    WaitStateChange->ControlReport.EFlags = ContextRecord->EFlags;
    WaitStateChange->ControlReport.ReportFlags = X86_REPORT_INCLUDES_SEGS;
    return;
}

VOID
EfiKdGetStateChange (
    __in PDBGKD_MANIPULATE_STATE64 ManipulateState,
    __in PCONTEXT ContextRecord
    )

/*++

Routine Description:

    The function extracts continuation control data from a manipulate state
    message.

Arguments:

    ManipulateState - Supplies a pointer to the manipulate state packet.

    ContextRecord - Supplies a pointer to a context record.

Return Value:

    None.

--*/

{

    //
    // If the continuation status is success, then set control space value.
    //

    if (NT_SUCCESS(ManipulateState->u.Continue2.ContinueStatus) != FALSE) {

        //
        // Set trace flag.
        //

        if (ManipulateState->u.Continue2.ControlSet.TraceFlag == TRUE) {
            ContextRecord->EFlags |= 0x100L;

        } else {
            ContextRecord->EFlags &= ~0x100L;

        }

        //
        // Set debug registers in processor control block.
        //

        EfiKdPrcb->ProcessorState.SpecialRegisters.KernelDr6 = 0L;
        EfiKdPrcb->ProcessorState.SpecialRegisters.KernelDr7 =
            ManipulateState->u.Continue2.ControlSet.Dr7;
    }
}

VOID
EfiKdSetStateChange (
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

    UNREFERENCED_PARAMETER(ExceptionRecord);

    EfiKdSetContextState(WaitStateChange, ContextRecord);
}

VOID
EfiKdReadControlSpace (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData,
    __in PCONTEXT Context
    )

/*++

Routine Description:

    This function reads control space.

Arguments:

    m - Supplies a pointer to the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/

{

    PDBGKD_READ_MEMORY64 a = &m->u.ReadMemory;
    ULONG Length;
    STRING MessageHeader;

    UNREFERENCED_PARAMETER(Context);

    ASSERT(AdditionalData->MaximumLength == EFI_KD_MESSAGE_BUFFER_SIZE);
    __analysis_assume(AdditionalData->MaximumLength == EFI_KD_MESSAGE_BUFFER_SIZE);

    //
    // If the specified control registers are within control space, then
    // read the specified space and return a success status. Otherwise,
    // return an unsuccessful status.
    //

    Length = min(a->TransferCount,
                 PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64));

    // MARK_SAFE_ADDITION(a->TargetBaseAddress, Length);
    if (((UINTN)a->TargetBaseAddress + Length) <= END_OF_CONTROL_SPACE) {
        EfiKdCopyMemory(AdditionalData->Buffer,
                     Add2Ptr(&EfiKdPrcb->ProcessorState, a->TargetBaseAddress),
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
    EfiKdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 AdditionalData);

    return;
}

VOID
EfiKdWriteControlSpace (
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
    ULONG Length;
    STRING MessageHeader;

    UNREFERENCED_PARAMETER(Context);

    //
    // If the specified control registers are within control space, then
    // write the specified space and return a success status. Otherwise,
    // return an unsuccessful status.
    //

    Length = min(a->TransferCount, AdditionalData->Length);
    // MARK_SAFE_ADDITION(a->TargetBaseAddress, Length);
    if (((UINTN)a->TargetBaseAddress + Length) <= END_OF_CONTROL_SPACE) {
        EfiKdCopyMemory(Add2Ptr(&EfiKdPrcb->ProcessorState, a->TargetBaseAddress),
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
    EfiKdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL);

    return;
}

VOID
EfiKdReadIoSpace (
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

    // PDBGKD_READ_WRITE_IO64 a = &m->u.ReadWriteIo;
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
            a->DataValue = BL_READ_PORT_UCHAR((PUCHAR)(UINTN)a->IoAddress);
            break;

        case 2:
            if (((ULONG)a->IoAddress & 1) != 0) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                a->DataValue = BL_READ_PORT_USHORT((PUSHORT)(UINTN)a->IoAddress);
            }

            break;

        case 4:
            if (((ULONG)a->IoAddress & 3) != 0) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                a->DataValue = BL_READ_PORT_ULONG((PULONG)(UINTN)a->IoAddress);
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
    EfiKdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL);

    return;
}

VOID
EfiKdWriteIoSpace (
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

    // PDBGKD_READ_WRITE_IO64 a = &m->u.ReadWriteIo;
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
            BL_WRITE_PORT_UCHAR((PUCHAR)(UINTN)a->IoAddress,
                                (UCHAR)a->DataValue);
            break;

        case 2:
            if (((ULONG)a->IoAddress & 1) != 0) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                BL_WRITE_PORT_USHORT((PUSHORT)(UINTN)a->IoAddress,
                                     (USHORT)a->DataValue);
            }

            break;

        case 4:
            if (((ULONG)a->IoAddress & 3) != 0) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                BL_WRITE_PORT_ULONG((PULONG)(UINTN)a->IoAddress,
                                    a->DataValue);
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
    EfiKdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL);

    return;
}

