/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    api.c

Abstract:

    This module implements the boot bebugger platform independent remote APIs.

Author:

    Mark Lucovsky (markl) 31-Aug-1990

Revision History:

--*/

// ------------------------------------------------------------------- Includes

#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>

#include "Bd.h"

// ------------------------------------------------------------------ Functions

VOID
BdGetVersion (
    __in PDBGKD_MANIPULATE_STATE64 m
    )
/*++

Routine Description:

    This function returns to the caller a general information packet
    that contains useful information to a debugger.  This packet is also
    used for a debugger to determine if the writebreakpointex and
    readbreakpointex apis are available.

Arguments:

    m - Supplies the state manipulation message.

Return Value:

    None.

--*/
{
    STRING messageHeader;

    messageHeader.Length = sizeof(*m);
    messageHeader.Buffer = (PCHAR)m;
    ZeroMem(&m->u.GetVersion64, sizeof(m->u.GetVersion64));

    //
    // the current build number
    //
    // - 4 - tells the debugger this is a "special" OS - the boot loader.
    // The boot loader has a lot of special cases associated with it, like
    // the lack of the DebuggerDataBlock, lack of ntoskrnl, etc ...
    //

    m->u.GetVersion64.MinorVersion = (INT16)NtBuildNumber;
    m->u.GetVersion64.MajorVersion = 0x400 |
                                     (INT16)((NtBuildNumber >> 28) & 0xFFFFFFF);

    //
    // Kd protocol version number.
    //

    m->u.GetVersion64.ProtocolVersion = DBGKD_64BIT_PROTOCOL_VERSION2;
    m->u.GetVersion64.KdSecondaryVersion = CURRENT_KD_SECONDARY_VERSION;
    m->u.GetVersion64.Flags = DBGKD_VERS_FLAG_PTR64;

#if defined(_AMD64_)

    m->u.GetVersion64.MachineType = IMAGE_FILE_MACHINE_AMD64;

#elif defined(_ARM64_)

    m->u.GetVersion64.MachineType = IMAGE_FILE_MACHINE_ARM64;

#else

#error( "unknown target machine" );

#endif

    m->u.GetVersion64.MaxPacketType = (PACKET_TYPE_KD_FILE_IO + 1) & 0xFF;
    m->u.GetVersion64.MaxStateChange = (DbgKdLoadSymbolsStateChange + 1) & 0xFF;
    m->u.GetVersion64.MaxManipulate = (DbgKdSetBusDataApi + 1) & 0xFF;

    //
    // Set the address of a "module list" containing the current module.
    // Use the current module base as the "kernel base" so that the debugger
    // does not complain about the lack of a kernel.
    //

    m->u.GetVersion64.PsLoadedModuleList = (UINT64)(UINT_PTR)&BdModuleList;
    m->u.GetVersion64.KernBase = (UINT64)(UINT_PTR)BdModuleDataTableEntry->DllBase;

    //m->u.GetVersion64.ThCallbackStack = 0;
    //m->u.GetVersion64.KiCallUserMode = 0;
    //m->u.GetVersion64.KeUserCallbackDispatcher = 0;
    //m->u.GetVersion64.NextCallback = 0;

    m->u.GetVersion64.DebuggerDataList = (UINTN)&BdDebuggerDataListHead;

    //
    // the usual stuff
    //

    m->ReturnStatus = STATUS_SUCCESS;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &messageHeader, NULL);
    return;
}


VOID
BdGetContext (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData,
    __in PCONTEXT Context
    )
/*++

Routine Description:

    This function is called in response of a get context state
    manipulation message.  Its function is to return the current
    context.

Arguments:

    m - Supplies the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/
{
    STRING messageHeader;

    m->ReturnStatus = STATUS_SUCCESS;

    ASSERT(AdditionalData->MaximumLength == BD_MESSAGE_BUFFER_SIZE);
    __analysis_assume(AdditionalData->MaximumLength == BD_MESSAGE_BUFFER_SIZE);

    AdditionalData->Length = sizeof(CONTEXT);
    BdCopyMemory(AdditionalData->Buffer, (PCHAR)Context, sizeof(CONTEXT));
    BdpContextSent = TRUE;

    //
    // Send reply packet.
    //

    messageHeader.Length = sizeof(*m);
    messageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                    &messageHeader,
                    AdditionalData);

    return;
}


VOID
BdGetContextEx (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData,
    __in PCONTEXT Context
    )

/*++

Routine Description:

    This function is called in response to an extended get context state
    manipulation message.  Its function is to return a portion of the current
    context.  This allows the context to be queried in chunks that are smaller
    than the debugger transport maximum packet size.

Arguments:

    m - Supplies the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/

{

    ULONG ByteCount;
    ULONG ContextLength;
    STRING MessageHeader;
    ULONG Offset;

    ASSERT(AdditionalData->Length == 0);
    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    Offset = m->u.GetContextEx.Offset;
    ByteCount = m->u.GetContextEx.ByteCount;

    ContextLength = sizeof(CONTEXT);

    //
    // Return just the portion that was requested.
    //

    m->u.GetContextEx.BytesCopied = 0;

    //
    // Calculate how much can be copied.
    //

    if (Offset >= ContextLength) {
        Offset = ContextLength;
    }

    if (ByteCount > (ContextLength - Offset)) {
        ByteCount = (ContextLength - Offset);
    }

    //
    // Move the requested portion of the context to the buffer.
    //

    BdCopyMemory(AdditionalData->Buffer, (PCHAR)Context + Offset, ByteCount);

    //
    // Return the length of the context and how much was copied and its
    // location.
    //

    m->u.GetContextEx.Offset = Offset;
    m->u.GetContextEx.ByteCount = ContextLength;
    m->u.GetContextEx.BytesCopied = ByteCount;

    //
    // Track when a complete context has been sent.  Specifically exclude
    // cases when the Offset was equal to or larger than the Context and
    // nothing was copied.
    //

    if ((Offset < ContextLength) && ((Offset + ByteCount) == ContextLength)) {
        BdpContextSent = TRUE;
    }

    //
    // Set the length of the returned buffer.
    //

    AdditionalData->Length = (USHORT)ByteCount;

    //
    // Send the response.
    //

    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 AdditionalData);

    return;
}


VOID
BdSetContext (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __out PCONTEXT Context
    )
/*++

Routine Description:

    This function is called in response of a set context state
    manipulation message.  Its function is set the current
    context.

Arguments:

    m - Supplies the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/
{
    STRING messageHeader;

    messageHeader.Length = sizeof(*m);
    messageHeader.Buffer = (PCHAR)m;
    if (BdpContextSent == FALSE) {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        goto BdSetContextEnd;
    }

    m->ReturnStatus = STATUS_SUCCESS;
    BdCopyMemory((PCHAR)Context, AdditionalData->Buffer, sizeof(CONTEXT));

    //
    // Send reply packet.
    //

BdSetContextEnd:
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                    &messageHeader,
                    NULL);
}


VOID
BdSetContextEx (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __out PCONTEXT Context
    )

/*++

Routine Description:

    This function is called in response to an extended set context state
    manipulation message.    Its function is to set a portion of the current
    context.  This allows the context to be set using chunks that are smaller
    than the debugger transport maximum packet size.

Arguments:

    m - Supplies the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/

{

    ULONG ByteCount;
    ULONG ContextLength;
    STRING MessageHeader;
    ULONG Offset;
    static UCHAR DECLSPEC_ALIGN(8) CachedData[BD_MESSAGE_BUFFER_SIZE];

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    Offset = m->u.SetContextEx.Offset;
    ByteCount = m->u.SetContextEx.ByteCount;
    ContextLength = m->u.SetContextEx.BytesCopied;

    ASSERT(ContextLength >= sizeof(CONTEXT));

    if (BdpContextSent == FALSE) {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        goto BdSetContextExEnd;
    }

    if ((ContextLength > BD_MESSAGE_BUFFER_SIZE) ||
        (Offset >= ContextLength) || (ByteCount == 0) ||
        (((ULONG64)Offset + ByteCount) > ContextLength)) {

        m->ReturnStatus = STATUS_INVALID_PARAMETER;
        goto BdSetContextExEnd;
    }

    //
    // Move the supplied portion of the Context to its location in CachedData.
    //

    BdCopyMemory((PCHAR)&CachedData[Offset],
                 AdditionalData->Buffer,
                 ByteCount);

    //
    // If the complete Context has been captured into CachedData, then set the
    // Context into the processor.
    //

    if (((ULONG64)Offset + ByteCount) == ContextLength) {

        //
        // Copy context.
        //

        BdCopyMemory((PCHAR)Context, (PCHAR)CachedData, sizeof(CONTEXT));
    }

    m->ReturnStatus = STATUS_SUCCESS;
    m->u.SetContextEx.BytesCopied = ByteCount;

BdSetContextExEnd:
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL);

    return;
}


VOID
BdReadVirtualMemory (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData,
    __in PCONTEXT Context
    )
/*++

Routine Description:

    This function is called in response to a read virtual memory 32-bit
    state manipulation message. Its function is to read virtual memory
    and return.

Arguments:

    m - Supplies a pointer to the state manipulation message.

    AdditionalData - Supplies a pointer to a descriptor for the data to read.

    Context - Supplies a pointer to the current context.

Return Value:

    None.

--*/
{
    UINT32 length;
    STRING messageHeader;

    UNREFERENCED_PARAMETER(Context);

    ASSERT(AdditionalData->MaximumLength == BD_MESSAGE_BUFFER_SIZE);
    __analysis_assume(AdditionalData->MaximumLength == BD_MESSAGE_BUFFER_SIZE);

    //
    // Trim the transfer count to fit in a single message.
    //

    length = min(m->u.ReadMemory.TransferCount,
                 PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64));

    //
    // Move the data to the destination buffer.
    //

    AdditionalData->Length = (UINT16)BdMoveMemory(
        (PCHAR)AdditionalData->Buffer,
        (PCHAR)(UINT_PTR)m->u.ReadMemory.TargetBaseAddress,
        length);

    //
    // If all the data is read, then return a success status. Otherwise,
    // return an unsuccessful status.
    //

    m->ReturnStatus = STATUS_SUCCESS;
    if (length != AdditionalData->Length) {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }

    //
    // Set the actual number of bytes read, initialize the message header,
    // and send the reply packet to the host debugger.
    //

    m->u.ReadMemory.ActualBytesRead = AdditionalData->Length;
    messageHeader.Length = sizeof(DBGKD_MANIPULATE_STATE64);
    messageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                    &messageHeader,
                    AdditionalData);
    return;
}


VOID
BdWriteVirtualMemory (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    )
/*++

Routine Description:

    This function is called in response of a write virtual memory 32-bit
    state manipulation message. Its function is to write virtual memory
    and return.

Arguments:

    m - Supplies a pointer to the state manipulation message.

    AdditionalData - Supplies a pointer to a descriptor for the data to write.

    Context - Supplies a pointer to the current context.

Return Value:

    None.

--*/
{
    UINT32 length;
    STRING messageHeader;

    UNREFERENCED_PARAMETER(Context);

    //
    // Move the data to the destination buffer.
    //

    length = BdMoveMemory((PCHAR)(UINT_PTR)m->u.WriteMemory.TargetBaseAddress,
                             (PCHAR)AdditionalData->Buffer,
                             AdditionalData->Length);

    //
    // If all the data is written, then return a success status. Otherwise,
    // return an unsuccessful status.
    //

    m->ReturnStatus = STATUS_SUCCESS;
    if (length != AdditionalData->Length) {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }

    //
    // Set the actual number of bytes written, initialize the message header,
    // and send the reply packet to the host debugger.
    //

    m->u.WriteMemory.ActualBytesWritten = length;
    messageHeader.Length = sizeof(DBGKD_MANIPULATE_STATE64);
    messageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                    &messageHeader,
                    NULL);

    return;
}


VOID
BdWriteBreakpoint (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    )
/*++

Routine Description:

    This function is called in response of a write breakpoint state
    manipulation message.  Its function is to write a breakpoint
    and return a handle to the breakpoint.

Arguments:

    m - Supplies the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/
{
    PDBGKD_WRITE_BREAKPOINT64 a = &m->u.WriteBreakPoint;
    STRING messageHeader;

    UNREFERENCED_PARAMETER(AdditionalData);
    UNREFERENCED_PARAMETER(Context);

    a->BreakPointHandle = BdAddBreakpoint(a->BreakPointAddress);
    if (a->BreakPointHandle != 0) {
        m->ReturnStatus = STATUS_SUCCESS;

    } else {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }

    //
    // Send reply packet.
    //

    messageHeader.Length = sizeof(*m);
    messageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                    &messageHeader,
                    NULL);

    return;
}


VOID
BdRestoreBreakpoint (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    )
/*++

Routine Description:

    This function is called in response of a restore breakpoint state
    manipulation message.  Its function is to restore a breakpoint
    using the specified handle.

Arguments:

    m - Supplies the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/
{
    PDBGKD_RESTORE_BREAKPOINT a = &m->u.RestoreBreakPoint;
    STRING messageHeader;

    UNREFERENCED_PARAMETER(AdditionalData);
    UNREFERENCED_PARAMETER(Context);

    if (BdDeleteBreakpoint(a->BreakPointHandle)) {
        m->ReturnStatus = STATUS_SUCCESS;

    } else {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }

    //
    // Send reply packet.
    //

    messageHeader.Length = sizeof(*m);
    messageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                    &messageHeader,
                    NULL);
}


VOID
BdReadPhysicalMemory (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData,
    __in PCONTEXT Context
    )
/*++

Routine Description:

    This function is called in response to a read physical memory
    state manipulation message. Its function is to read physical memory
    and return.

Arguments:

    m - Supplies the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/
{
    PDBGKD_READ_MEMORY64 a = &m->u.ReadMemory;
    UINT16 bytesLeft;
    PCHAR destination;
    UINT32 length;
    STRING messageHeader;
    UINT16 numberBytes;
    PHYSICAL_ADDRESS source;
    PVOID virtualAddress;

    UNREFERENCED_PARAMETER(Context);

    ASSERT(AdditionalData->MaximumLength == BD_MESSAGE_BUFFER_SIZE);
    __analysis_assume(AdditionalData->MaximumLength == BD_MESSAGE_BUFFER_SIZE);

    //
    // Trim transfer count to fit in a single message.
    //

    length = min(a->TransferCount,
                 PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64));

    //
    // Since the BdTranslatePhysicalAddress only maps in one physical
    // page at a time, we need to break the memory move up into smaller
    // moves which don't cross page boundaries.  There are two cases we
    // need to deal with.  The area to be moved may start and end on the
    // same page, or it may start and end on different pages (with an
    // arbitrary number of pages in between)
    //

    source = (UINT_PTR)a->TargetBaseAddress;
    destination = AdditionalData->Buffer;
    bytesLeft = (UINT16)length;
    if(PAGE_ALIGN((PUCHAR)(UINT_PTR)a->TargetBaseAddress) ==
       PAGE_ALIGN((PUCHAR)((UINT_PTR)a->TargetBaseAddress) + length))
    {
        //
        // Memory move starts and ends on the same page.
        //

        virtualAddress=BdTranslatePhysicalAddress(source);
        if (virtualAddress == NULL)
        {
            AdditionalData->Length = 0;
        }
        else
        {
            AdditionalData->Length = (UINT16)BdMoveMemory(destination,
                                                             virtualAddress,
                                                             bytesLeft);

            bytesLeft = bytesLeft - AdditionalData->Length;
            BdUnmapVirtualAddress(virtualAddress);
        }
    }
    else
    {
        //
        // Memory move spans page boundaries
        //

        virtualAddress=BdTranslatePhysicalAddress(source);
        if (virtualAddress == NULL)
        {
            AdditionalData->Length = 0;
        }
        else
        {
            numberBytes = (UINT16)(EFI_PAGE_SIZE - BYTE_OFFSET(virtualAddress));
            AdditionalData->Length = (UINT16)BdMoveMemory(destination,
                                                             virtualAddress,
                                                             numberBytes);

            BdUnmapVirtualAddress(virtualAddress);
            source += numberBytes;
            destination += numberBytes;
            bytesLeft = bytesLeft - numberBytes;
            while(bytesLeft > 0)
            {
                //
                // Transfer a full page or the last bit,
                // whichever is smaller.
                //

                virtualAddress = BdTranslatePhysicalAddress(source);
                if (virtualAddress == NULL)
                {
                    break;
                }
                else
                {
                    numberBytes = (UINT16) ((EFI_PAGE_SIZE < bytesLeft) ? EFI_PAGE_SIZE : bytesLeft);
                    AdditionalData->Length = AdditionalData->Length +
                        (UINT16)BdMoveMemory(destination,
                                                virtualAddress,
                                                numberBytes);

                    BdUnmapVirtualAddress(virtualAddress);
                    source += numberBytes;
                    destination += numberBytes;
                    bytesLeft = bytesLeft - numberBytes;
                }
            }
        }
    }

    if (length == AdditionalData->Length)
    {
        m->ReturnStatus = STATUS_SUCCESS;
    }
    else
    {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }

    a->ActualBytesRead = AdditionalData->Length;

    //
    // Send reply packet.
    //

    messageHeader.Length = sizeof(*m);
    messageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                    &messageHeader,
                    AdditionalData);

    return;
}


VOID
BdWritePhysicalMemory (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    )
/*++

Routine Description:

    This function is called in response to a write physical memory
    state manipulation message. Its function is to write physical memory
    and return.

Arguments:

    m - Supplies the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/
{
    PDBGKD_WRITE_MEMORY64 a = &m->u.WriteMemory;
    UINT16 bytesLeft;
    PHYSICAL_ADDRESS destination;
    UINT32 Length;
    STRING messageHeader;
    UINT16 numberBytes;
    PCHAR source;
    PVOID virtualAddress;

    UNREFERENCED_PARAMETER(Context);

    messageHeader.Length = sizeof(*m);
    messageHeader.Buffer = (PCHAR)m;

    //
    // Since the BdTranslatePhysicalAddress only maps in one physical
    // page at a time, we need to break the memory move up into smaller
    // moves which don't cross page boundaries.  There are two cases we
    // need to deal with.  The area to be moved may start and end on the
    // same page, or it may start and end on different pages (with an
    // arbitrary number of pages in between)
    //

    destination = (UINT_PTR)a->TargetBaseAddress;
    source = AdditionalData->Buffer;
    bytesLeft = (UINT16) a->TransferCount;
    if(PAGE_ALIGN(destination) ==
       PAGE_ALIGN(destination+bytesLeft))
    {
        //
        // Memory move starts and ends on the same page.
        //

        virtualAddress = BdTranslatePhysicalAddress(destination);
        if (virtualAddress == NULL)
        {
            Length = 0;
        }
        else
        {
            Length = (UINT16)BdMoveMemory(virtualAddress,
                                             source,
                                             bytesLeft);

            BdUnmapVirtualAddress(virtualAddress);
            bytesLeft = bytesLeft - (UINT16)Length;
        }
    }
    else
    {
        //
        // Memory move spans page boundaries
        //

        virtualAddress = BdTranslatePhysicalAddress(destination);
        if (virtualAddress == NULL)
        {
            Length = 0;
        }
        else
        {
            numberBytes = (UINT16) (EFI_PAGE_SIZE - BYTE_OFFSET(virtualAddress));
            Length = (UINT16)BdMoveMemory(virtualAddress,
                                             source,
                                             numberBytes);

            BdUnmapVirtualAddress(virtualAddress);
            source += numberBytes;
            destination += numberBytes;
            bytesLeft = bytesLeft - numberBytes;
            while(bytesLeft > 0)
            {
                //
                // Transfer a full page or the last bit, whichever is smaller.
                //

                virtualAddress = BdTranslatePhysicalAddress(destination);
                if (virtualAddress == NULL) {
                    break;
                }

                numberBytes = (UINT16) ((EFI_PAGE_SIZE < bytesLeft) ? EFI_PAGE_SIZE : bytesLeft);
                Length += (UINT16)BdMoveMemory(virtualAddress,
                                                  source,
                                                  numberBytes);

                BdUnmapVirtualAddress(virtualAddress);
                source += numberBytes;
                destination += numberBytes;
                bytesLeft = bytesLeft - numberBytes;
            }
        }
    }

    if (Length == AdditionalData->Length)
    {
        m->ReturnStatus = STATUS_SUCCESS;
    }
    else
    {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }

    a->ActualBytesWritten = Length;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                    &messageHeader,
                    NULL);

    return;
}


NTSTATUS
BdWriteBreakPointEx (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    )
/*++

Routine Description:

    This function is called in response of a write breakpoint state 'ex'
    manipulation message.  Its function is to clear breakpoints, write
    new breakpoints, and continue the target system.  The clearing of
    breakpoints is conditional based on the presence of breakpoint handles.
    The setting of breakpoints is conditional based on the presence of
    valid, non-zero, addresses.  The continueing of the target system
    is conditional based on a non-zero continuestatus.

    This api allows a debugger to clear breakpoints, add new breakpoint,
    and continue the target system all in one api packet.  This reduces the
    amount of traffic across the wire and greatly improves source stepping.


Arguments:

    m - Supplies the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/
{
    PDBGKD_BREAKPOINTEX a = &m->u.BreakPointEx;
    PDBGKD_WRITE_BREAKPOINT64 b;
    UINT32 i;
    STRING messageHeader;
    DBGKD_WRITE_BREAKPOINT64 bpBuf[BREAKPOINT_TABLE_SIZE];
    UINT32 breakPointCount;

    UNREFERENCED_PARAMETER(Context);

    messageHeader.Length = sizeof(*m);
    messageHeader.Buffer = (PCHAR)m;

    //
    // verify that the packet size is correct
    //

    if (AdditionalData->Length !=
                         a->BreakPointCount * sizeof(DBGKD_WRITE_BREAKPOINT64))
    {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                        &messageHeader,
                        AdditionalData);
        return m->ReturnStatus;
    }

    breakPointCount = a->BreakPointCount;
    if (breakPointCount > BREAKPOINT_TABLE_SIZE)
    {
        breakPointCount = BREAKPOINT_TABLE_SIZE;
    }

    i = BdMoveMemory((PCHAR)bpBuf,
                         AdditionalData->Buffer,
                         breakPointCount * sizeof(DBGKD_WRITE_BREAKPOINT64));

    _Analysis_assume_ (breakPointCount * sizeof(DBGKD_WRITE_BREAKPOINT64) == i);

    //
    // assume success
    //

    m->ReturnStatus = STATUS_SUCCESS;

    //
    // loop thru the breakpoint handles passed in from the debugger and
    // clear any breakpoint that has a non-zero handle
    //

    b = bpBuf;
    for (i = 0; i < breakPointCount; i += 1, b += 1)
    {
        if (b->BreakPointHandle)
        {
            if (!BdDeleteBreakpoint(b->BreakPointHandle))
            {
                m->ReturnStatus = STATUS_UNSUCCESSFUL;
            }

            b->BreakPointHandle = 0;
        }
    }

    //
    // loop thru the breakpoint addesses passed in from the debugger and
    // add any new breakpoints that have a non-zero address
    //

    b = bpBuf;
    for (i = 0; i < breakPointCount; i += 1, b += 1)
    {
        if (b->BreakPointAddress)
        {
            b->BreakPointHandle = BdAddBreakpoint( b->BreakPointAddress);
            if (!b->BreakPointHandle)
            {
                m->ReturnStatus = STATUS_UNSUCCESSFUL;
            }
        }
    }

    //
    // send back our response
    //

    BdMoveMemory(AdditionalData->Buffer,
                    (PCHAR)bpBuf,
                    breakPointCount * sizeof(DBGKD_WRITE_BREAKPOINT64));

    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                    &messageHeader,
                    AdditionalData);

    //
    // return the caller's continue status value.  if this is a non-zero
    // value the system is continued using this value as the continuestatus.
    //

    return a->ContinueStatus;
}


VOID
BdRestoreBreakPointEx (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    )
/*++

Routine Description:

    This function is called in response of a restore breakpoint state 'ex'
    manipulation message.  Its function is to clear a list of breakpoints.

Arguments:

    m - Supplies the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/
{
    PDBGKD_BREAKPOINTEX a = &m->u.BreakPointEx;
    PDBGKD_RESTORE_BREAKPOINT b;
    STRING messageHeader;
    UINT32 i;
    DBGKD_RESTORE_BREAKPOINT bpBuf[BREAKPOINT_TABLE_SIZE];
    UINT32 breakPointCount;

    UNREFERENCED_PARAMETER(Context);

    messageHeader.Length = sizeof(*m);
    messageHeader.Buffer = (PCHAR)m;

    //
    // verify that the packet size is correct
    //

    if (AdditionalData->Length !=
        a->BreakPointCount * sizeof(DBGKD_RESTORE_BREAKPOINT))
    {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                        &messageHeader,
                        AdditionalData);

        return;
    }

    breakPointCount = a->BreakPointCount;
    if (breakPointCount > BREAKPOINT_TABLE_SIZE)
    {
        breakPointCount = BREAKPOINT_TABLE_SIZE;
    }

    BdMoveMemory((PCHAR)bpBuf,
                     AdditionalData->Buffer,
                     breakPointCount * sizeof(DBGKD_RESTORE_BREAKPOINT));

    //
    // assume success
    //

    m->ReturnStatus = STATUS_SUCCESS;

    //
    // loop thru the breakpoint handles passed in from the debugger and
    // clear any breakpoint that has a non-zero handle
    //

    b = bpBuf;
    for (i = 0; i < breakPointCount; i += 1, b += 1)
    {
        if (!BdDeleteBreakpoint(b->BreakPointHandle))
        {
            m->ReturnStatus = STATUS_UNSUCCESSFUL;
        }
    }

    //
    // send back our response
    //

    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                    &messageHeader,
                    AdditionalData);

    return;
}
