/*++

Copyright (c) 1996-2003  Microsoft Corporation

Module Name:

    message.c

Abstract:

    This module implements the debugger state change message functions.

Author:

    Mark Lucovsky (markl) 31-Aug-1990

--*/

#include <PiDxe.h>

#include <Guid/DebugImageInfoTable.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/ResetSystemLib.h>

#include "EfiKd.h"

KCONTINUE_STATUS
EfiKdSendWaitContinue (
    __in UINT32 OutPacketType,
    __in PSTRING OutMessageHeader,
    __in_opt PSTRING OutMessageData,
    __inout PCONTEXT ContextRecord
    )
/*++

Routine Description:

    This function sends a packet and waits for a continue message. BreakIns
    received while waiting will always cause a resend of the packet originally
    sent out. While waiting state manipulate messages will be serviced.

    A resend always resends the original event sent to the debugger, not the
    last response to some debugger command.

Arguments:

    OutPacketType - Supplies the type of packet to send.

    OutMessageHeader - Supplies a pointer to a string descriptor that describes
        the message information.

    OutMessageData - Supplies a pointer to a string descriptor that describes
        the optional message data.

    ContextRecord - Exception context

Return Value:

    A value of TRUE is returned if the continue message indicates
    success, Otherwise, a value of FALSE is returned.

--*/
{
    UINT32 Length;
    STRING MessageData;
    STRING MessageHeader;
    DBGKD_MANIPULATE_STATE64 ManipulateState;
    UINT32 ReturnCode;
    NTSTATUS Status;

    //
    // Loop servicing state manipulation message until a continue message
    // is received.
    //

    ZeroMem(&ManipulateState, sizeof(DBGKD_MANIPULATE_STATE64));
    MessageHeader.MaximumLength = sizeof(DBGKD_MANIPULATE_STATE64);
    MessageHeader.Buffer = (PCHAR)&ManipulateState;
    MessageData.MaximumLength = EFI_KD_MESSAGE_BUFFER_SIZE;
    MessageData.Buffer = (PCHAR)(&EfiKdMessageBuffer[0]);

    //
    // Send event notification packet to debugger on host. Come back here
    // any time we see a breakin sequence.
    //

ResendPacket:

    EfiKdSendPacket(OutPacketType, OutMessageHeader, OutMessageData);

    //
    // After sending packet, if there is no response from debugger and the
    // packet is for reporting symbol (un)load, the debugger will be declared
    // to be not present. Note If the packet is for reporting exception, the
    // EfiKdSendPacket will never stop.
    //

    if (EfiKdDebuggerNotPresent != FALSE)
    {
        return ContinueSuccess;
    }

    while (TRUE)
    {
        //
        // Wait for State Manipulate Packet without timeout.
        //

        do
        {
            ReturnCode = EfiKdReceivePacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                                            &MessageHeader,
                                            &MessageData,
                                            &Length);
            if (ReturnCode == (UINT16)EFI_KD_PACKET_RESEND)
            {
                goto ResendPacket;
            }
        } while (ReturnCode == EFI_KD_PACKET_TIMEOUT);

        //
        // Switch on the return message API number.
        //

        //ConsolePrint(L"EfiKdSendWait: api number %x\r\n", ManipulateState.ApiNumber);
        switch (ManipulateState.ApiNumber)
        {
        case DbgKdReadVirtualMemoryApi:
            EfiKdReadVirtualMemory(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdWriteVirtualMemoryApi:
            EfiKdWriteVirtualMemory(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdReadPhysicalMemoryApi:
            EfiKdReadPhysicalMemory(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdWritePhysicalMemoryApi:
            EfiKdWritePhysicalMemory(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdGetContextApi:
            EfiKdGetContext(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdSetContextApi:
            EfiKdSetContext(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdWriteBreakPointApi:
            EfiKdWriteBreakpoint(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdRestoreBreakPointApi:
            EfiKdRestoreBreakpoint(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdReadControlSpaceApi:
            EfiKdReadControlSpace(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdWriteControlSpaceApi:
            EfiKdWriteControlSpace(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdReadIoSpaceApi:
            EfiKdReadIoSpace(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdWriteIoSpaceApi:
            EfiKdWriteIoSpace(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdContinueApi:
            if (NT_SUCCESS(ManipulateState.u.Continue.ContinueStatus) != FALSE)
            {
                return ContinueSuccess;
            }
            else
            {
                return ContinueError;
            }

            break;

        case DbgKdContinueApi2:
            if (NT_SUCCESS(ManipulateState.u.Continue2.ContinueStatus) != FALSE)
            {
                EfiKdGetStateChange(&ManipulateState, ContextRecord);
                return ContinueSuccess;
            }
            else
            {
                return ContinueError;
            }

            break;

        case DbgKdRebootApi:
            ResetWarm();
            break;

        case DbgKdGetVersionApi:
            EfiKdGetVersion(&ManipulateState);
            break;

        case DbgKdWriteBreakPointExApi:
            Status = EfiKdWriteBreakPointEx(&ManipulateState,
                                            &MessageData,
                                            ContextRecord);

            if (Status)
            {
                ManipulateState.ApiNumber = DbgKdContinueApi;
                ManipulateState.u.Continue.ContinueStatus = Status;
                return ContinueError;
            }

            break;

        case DbgKdRestoreBreakPointExApi:
            EfiKdRestoreBreakPointEx(&ManipulateState, &MessageData, ContextRecord);
            break;

        //
        // Prevent spurious PACKET_TYPE_KD_STATE_MANIPULATE packets with a
        // ReturnStatus of STATUS_UNSUCCESSFUL from being sent for these APIs
        // since no response is expected.  Sending the spurious response can
        // cause the host and target to get out of sync.
        //

        case DbgKdSetSpecialCallApi:
        case DbgKdClearSpecialCallsApi:
        case DbgKdSetInternalBreakPointApi:
        case DbgKdClearAllInternalBreakpointsApi:
            break;

        //
        // Invalid message.
        //

        default:
            MessageData.Length = 0;
            ManipulateState.ReturnStatus = STATUS_UNSUCCESSFUL;
            EfiKdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                         &MessageHeader,
                         &MessageData);

            break;
        }
    }
}


VOID
EfiKdpSetCommonState (
    __in UINT32 NewState,
    __in PCONTEXT ContextRecord,
    __out PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange
    )
{
    UINT16 InstrCount;
    PUINT8 InstrStream;
    PCHAR PcMemory;

    WaitStateChange->NewState = NewState;
    WaitStateChange->ProcessorLevel = 0;
    WaitStateChange->Processor = 0;
    WaitStateChange->NumberProcessors = 1;
    WaitStateChange->Thread = 0;
    PcMemory = (PCHAR)(UINTN)CONTEXT_TO_PROGRAM_COUNTER(ContextRecord);
    WaitStateChange->ProgramCounter = (UINT64)(LONG64)(LONG_PTR)PcMemory;

    ZeroMem(&WaitStateChange->AnyControlReport,
            sizeof(WaitStateChange->AnyControlReport));

    //
    // Copy instruction stream immediately following location of event.
    //

    InstrStream = WaitStateChange->ControlReport.InstructionStream;
    InstrCount =
        (UINT16)EfiKdMoveMemory((PCHAR)InstrStream, PcMemory, DBGKD_MAXSTREAM);
    WaitStateChange->ControlReport.InstructionCount = InstrCount;

    //
    // Clear breakpoints in copied area.
    // If there were any breakpoints cleared, recopy the instruction area
    // without them.
    //

    if (EfiKdDeleteBreakpointRange((UINT_PTR)PcMemory,
                                   (UINT_PTR)PcMemory + InstrCount - 1))
    {
        EfiKdMoveMemory((PCHAR)InstrStream, PcMemory, InstrCount);
    }
}


LOGICAL
EfiKdReportExceptionStateChange (
    __in PEXCEPTION_RECORD ExceptionRecord,
    __inout PCONTEXT ContextRecord,
    __in BOOLEAN FirstChance
    )
/*++

Routine Description:

    This routine sends an exception state change packet to the kernel
    debugger and waits for a manipulate state message.

Arguments:

    ExceptionRecord - Supplies a pointer to an exception record.

    ContextRecord - Supplies a pointer to a context record.

    FirstChance - Supplies an indication of whether this exception should as
        a first chance or second chance exception.

Return Value:

    A value of TRUE is returned if the exception is handled. Otherwise, a
    value of FALSE is returned.

--*/
{
    STRING MessageData;
    STRING MessageHeader;
    KCONTINUE_STATUS Status;
    DBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange;

    ZeroMem(&WaitStateChange, sizeof(WaitStateChange));

    do
    {
        //
        // Construct the wait state change message and message descriptor.
        //
        // N.B. All exceptions are reported as second chance exceptions since
        //      the boot environment does contain any exception handling
        //      system.
        //

        EfiKdpSetCommonState(DbgKdExceptionStateChange, ContextRecord,
                          &WaitStateChange);

        if (sizeof(EXCEPTION_RECORD) ==
            sizeof(WaitStateChange.u.Exception.ExceptionRecord))
        {
            EfiKdCopyMemory((PCHAR)&WaitStateChange.u.Exception.ExceptionRecord,
                            (PCHAR)ExceptionRecord,
                            sizeof(EXCEPTION_RECORD));
        }
        else
        {
            ExceptionRecord32To64((PEXCEPTION_RECORD32)ExceptionRecord,
                                  &WaitStateChange.u.Exception.ExceptionRecord);
        }

        WaitStateChange.u.Exception.FirstChance = FirstChance;

        EfiKdSetStateChange(&WaitStateChange, ExceptionRecord, ContextRecord);

        MessageHeader.Length = sizeof(WaitStateChange);
        MessageHeader.Buffer = (PCHAR)&WaitStateChange;
        MessageData.Length = 0;

        //
        // Send packet to the kernel debugger on the host machine,
        // wait for answer.
        //

        Status = EfiKdSendWaitContinue(PACKET_TYPE_KD_STATE_CHANGE64,
                                       &MessageHeader,
                                       &MessageData,
                                       ContextRecord);

    } while (Status == ContinueProcessorReselected) ;

    return (BOOLEAN) Status;
}


LOGICAL
EfiKdReportLoadSymbolsStateChange (
    __in PSTRING PathName,
    __in PKD_SYMBOLS_INFO SymbolInfo,
    __in LOGICAL UnloadSymbols,
    __inout PCONTEXT ContextRecord
    )
/*++

Routine Description:

    This routine sends a load symbols state change packet to the kernel
    debugger and waits for a manipulate state message.

Arguments:

    PathName - Supplies a pointer to the pathname of the image whose
        symbols are to be loaded.

    BaseOfDll - Supplies the base address where the image was loaded.

    ProcessId - Unique 32-bit identifier for process that is using
        the symbols.  -1 for system process.

    CheckSum - Unique 32-bit identifier from image header.

    UnloadSymbol - TRUE if the symbols that were previously loaded for
        the named image are to be unloaded from the debugger.

Return Value:

    A value of TRUE is returned if the exception is handled. Otherwise, a
    value of FALSE is returned.

--*/
{
    PSTRING AdditionalData;
    STRING MessageData;
    STRING MessageHeader;
    KCONTINUE_STATUS Status;
    DBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange;

    do {
        //
        // Construct the wait state change message and message descriptor.
        //

        EfiKdpSetCommonState(DbgKdLoadSymbolsStateChange, ContextRecord,
                             &WaitStateChange);
        EfiKdSetContextState(&WaitStateChange, ContextRecord);

        WaitStateChange.u.LoadSymbols.UnloadSymbols = (BOOLEAN)UnloadSymbols;
        WaitStateChange.u.LoadSymbols.BaseOfDll = (UINT64)SymbolInfo->BaseOfDll;
        WaitStateChange.u.LoadSymbols.ProcessId = SymbolInfo->ProcessId;
        WaitStateChange.u.LoadSymbols.CheckSum = SymbolInfo->CheckSum;
        WaitStateChange.u.LoadSymbols.SizeOfImage = SymbolInfo->SizeOfImage;
        if (ARGUMENT_PRESENT(PathName))
        {
            WaitStateChange.u.LoadSymbols.PathNameLength =
                EfiKdMoveMemory((PCHAR)EfiKdMessageBuffer,
                                (PCHAR)PathName->Buffer,
                                PathName->Length) + 1;

            MessageData.Buffer = (PCHAR)(&EfiKdMessageBuffer[0]);
            MessageData.Length = (UINT16)WaitStateChange.u.LoadSymbols.PathNameLength;
            MessageData.Buffer[MessageData.Length-1] = '\0';
            AdditionalData = &MessageData;
        }
        else
        {
            WaitStateChange.u.LoadSymbols.PathNameLength = 0;
            AdditionalData = NULL;
        }

        MessageHeader.Length = sizeof(WaitStateChange);
        MessageHeader.Buffer = (PCHAR)&WaitStateChange;

        //
        // Send packet to the kernel debugger on the host machine, wait
        // for the reply.
        //

        Status = EfiKdSendWaitContinue(PACKET_TYPE_KD_STATE_CHANGE64,
                                       &MessageHeader,
                                       AdditionalData,
                                       ContextRecord);
    } while (Status == ContinueProcessorReselected);

    return (BOOLEAN) Status;
}
