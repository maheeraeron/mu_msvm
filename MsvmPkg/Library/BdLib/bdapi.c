/*++

Copyright (c) 1990-2004  Microsoft Corporation

Module Name:

    BdApi.c

Abstract:

    This module implements the initializaion code for the boot debugger.

Author:

    Jamie Schwartz (jamschw) Dec. 2003
    Shreyas Srivatsan (shreyas) July 2012 - copied from boot debugging library.

Environment:

    Boot

--*/

// ------------------------------------------------------------------- Includes

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/BdDebugLib.h>
#include <Library/SerialPortLib.h>

#include "Bd.h"

// -------------------------------------------------------------------- Defines

#define CONNECTION_RETRIES 10

// -------------------------------------------------------------------- Globals

BOOLEAN BdConnectionActive = FALSE;
BOOLEAN BdDebuggerInitialConnection = FALSE;
LIST_ENTRY BdModuleList = { &BdModuleList, &BdModuleList };
UINT32 BdModuleListCount = 0;
PLDR_DATA_TABLE_ENTRY BdModuleDataTableEntry = NULL;
KDDEBUGGER_DATA64 BdDebuggerDataBlock;
LIST_ENTRY BdDebuggerDataListHead = { &BdDebuggerDataListHead, &BdDebuggerDataListHead };
PEFI_UNLOADED_MODULE BdUnloadedModulesPtr = BdUnloadedModules;

PBD_RECEIVE_PACKET BdReceivePacket;
PBD_SEND_PACKET BdSendPacket;

BD_CONNECTION_TYPE BdDebuggerType;
BD_CONNECTION_PARAMETERS BdParameters;

// ----------------------------------------------------------------- Prototypes

VOID
BdpCloseDebuggerDevice (
    VOID
    );

NTSTATUS
BdpConfigureDebuggerDevice (
    __in PBD_CONNECTION_PARAMETERS Parameters
    );

VOID
BdpDeviceStop (
    VOID
    );

NTSTATUS
BdpGetConnectionParameters (
    __out PBD_CONNECTION_PARAMETERS Parameters
    );

VOID
BdpFreeDataTableEntry (
    __in PLDR_DATA_TABLE_ENTRY TableEntry
    );

NTSTATUS
BdpModuleNameFromImage (
    __in_bcount(ImageSize) PVOID ImageBase,
    __in UINT32 ImageSize,
    __out PSTRING ModuleName
    );

UINT64
BdpSentReceivedPacketCount(
    VOID
    );

NTSTATUS
BdpReconfigureDebuggerDevice (
    VOID
    );

// ------------------------------------------------------------------ Functions

BOOLEAN
BdDebuggerEnabled (
    VOID
    )
/*++

Routine Description:

    This routine returns a BOOLEAN indicating the status of the debugger
    connection.

Arguments:

    None.

Return Value:

    TRUE if a debugger connection is established.
    FALSE otherwise.

--*/
{

    BOOLEAN debuggerEnabled;
    BOOLEAN debuggerInitialized;

    //
    // N.B. This routine does not have interface enter/exit calls to ensure
    //      that it can be called from any context, most notably from physical
    //      mode in a virtual mode application.  This is required because this
    //      routine is called during the execution of asserts.
    //

    debuggerInitialized = BdDebuggerInitialized();
    if ((debuggerInitialized != FALSE) && (BdDebuggerNotPresent == FALSE))
    {
        debuggerEnabled = TRUE;
    }
    else
    {
        debuggerEnabled = FALSE;
    }

    return debuggerEnabled;
}


VOID
BdNotifyShutdown(
    INTN       Reason
    )
/*++

Routine Description:

    Notifies the debugger that the debugee is stopping

Arguments:

    Reason      Shutdown reason, KD_REBOOT or KD_HIBERNATE

Return Value:

    None.

--*/
{
    KD_SYMBOLS_INFO symbolInfo = {0};

    ASSERT((Reason == KD_REBOOT) ||
           (Reason == KD_HIBERNATE));

    symbolInfo.BaseOfDll   = (VOID*)(UINTN)Reason;
    symbolInfo.CheckSum    = 0;
    symbolInfo.ProcessId   = 0;
    symbolInfo.SizeOfImage = 0;

    DebugService2(NULL, &symbolInfo, BREAKPOINT_UNLOAD_SYMBOLS);
}

VOID
BdStop (
    VOID
    )
/*++

Routine Description:

    This routine will unload the image symbols and temporarily stop debugging.

    It will not close the debugger port or reset any global state information.
    A call to BdStart will resume the debugger session.

Arguments:

    None.

Return Value:

    None.

--*/
{

    //
    // Unload the image symbols and perform any architecture specific actions
    // required as part of debugger suspension.
    //

    if ((BdConnectionActive != FALSE))
    {
        //
        // If the initial debugger connection succeeded, or if any packets were
        // successfully sent to or received from the host, then ensure that the
        // disconnect message is sent as well.
        //

        if ((BdDebuggerInitialConnection != FALSE) ||
            (BdpSentReceivedPacketCount() != 0))
        {
            BdDebuggerNotPresent = FALSE;
        }

        BdNotifyShutdown(KD_REBOOT);
        BdArchStop();
        BdpDeviceStop();
        BdConnectionActive = FALSE;
    }

    return;
}

VOID
BdStart (
    VOID
    )

/*++

Routine Description:

    This routine will initiate a debugger session.

    The "module loaded" message will be sent to the debugger on
    the first call to PeCoffLoaderRelocateImageExtraAction() in DxeMain().

Arguments:

    None.

Return Value:

    None.

--*/

{
    UINT32 retries;
    NTSTATUS status;

    //
    // Subsystem initialization must occur before any connection attempt can
    // succeed.  Furthermore, a connection attempt cannot occur when a
    // connection is already active.
    //

    if ((BdSubsystemInitialized == FALSE) || (BdConnectionActive != FALSE))
    {
        goto StartEnd;
    }

    //
    // Reconfigure the debugger device for usage.  This is required if another
    // application configured the device since it was first initialized.
    //

    status = BdpReconfigureDebuggerDevice();
    if (!NT_SUCCESS(status))
    {
        goto StartEnd;
    }

    //
    // Perform any architecture specific actions required as part of debugger
    // resumption.  Whenever the subsystem is initialized, this call is
    // guaranteed to prepare all trap handlers required for debugging.  Note
    // that this routine may change the state of the debugger operation block
    // flag.
    //

    BdArchStart();

    //
    // A connection attempt will be made.  Mark the debugger connection
    // active, it will be marked inactive when the connection is stopped.
    //

    BdConnectionActive = TRUE;

    //
    // Perform a debug print before loading the image symbols.
    //

    retries = CONNECTION_RETRIES;
    do
    {
        //
        // Note: Use DebugService2 directly since DebugPrintString uses the DebugLib
        // which has not been initialized yet.
        //
        BdDebuggerNotPresent = FALSE;
#define INIT_STRING "Microsoft UEFI Boot Debugger Initialized\n"
        DebugService2(INIT_STRING, (PVOID)sizeof(INIT_STRING), BREAKPOINT_PRINT);
        retries -= 1;
    } while ((BdDebuggerNotPresent != FALSE) && (retries > 0));

    //
    // Track initial connection success.
    //

    BdDebuggerInitialConnection = !BdDebuggerNotPresent;

StartEnd:
    return;
}


VOID
BdInitializeDebuggerDataBlock (
    VOID
    )
/*++

Routine Description:

    Initializes the KD debugger data block.

Arguments:

    None.

Return Value:

    None.

--*/
{
    ZeroMem(BdUnloadedModules, (sizeof(EFI_UNLOADED_MODULE) * BD_MAX_UNLOADED_MODULES));

    BdDebuggerDataBlock.Header.OwnerTag = 'GBDK';
    BdDebuggerDataBlock.Header.Size = sizeof(BdDebuggerDataBlock);
    BdDebuggerDataBlock.PsLoadedModuleList = (UINTN)&BdModuleList;
    BdDebuggerDataBlock.BreakpointWithStatus = (UINTN)BdBreakPointWithStatus;
    // kernel base will be filled in after debugger has initialized.
    BdDebuggerDataBlock.KernBase = 0;
    BdDebuggerDataBlock.SizePrcb = sizeof(KPRCB);
    BdDebuggerDataBlock.SizePcr  = sizeof(KPCR);
    //
    // MmUnloadedDrivers contains the address of a pointer to the base of
    // the unloaded modules array.
    //
    BdDebuggerDataBlock.MmUnloadedDrivers = (UINT64)&BdUnloadedModulesPtr;
    BdDebuggerDataBlock.MmLastUnloadedDriver = (UINT64)&BdLastUnloadedModule;
    InsertTailList(&BdDebuggerDataListHead,
                   &BdDebuggerDataBlock.Header.List);
}


NTSTATUS
BdInitialize (
    VOID
    )
/*++

Routine Description:

    Initializes the boot debugger for the executing boot application.
    This can be called multiple times.  The first call will perform complete
    initialization, while subsequent calls will reset the state and re-initialize
    the trap hooks only.

Arguments:

    None.

Return Value:

    STATUS_SUCCESS when successful, when the debugger is already initialized
        or if boot debugging is not requested.

    STATUS_NOT_FOUND if boot debugging was requested, but a debugger UART was
        not found.

--*/
{

    UINT32 index;
    NTSTATUS optionStatus;
    BOOLEAN portOpened;
    NTSTATUS status;

    status = STATUS_SUCCESS;
    portOpened = FALSE;

    BdSerialPrint(">>> %a\n", __FUNCTION__);

    //
    // Initialization that is always performed (this includes debugger reinitialization).
    // - Trap Handlers
    //      Minimal trap handlers are needed for the DebugLib (DebugPrint, DebugAssert, etc.)
    //      to function properly.
    //

    //
    // Initialize the communication channel by configuring the initial
    // packet request to be sent.
    //

    BdNextPacketIdToSend = INITIAL_PACKET_ID | SYNC_PACKET_ID;
    BdPacketIdExpected = INITIAL_PACKET_ID;

    status = BdArchInitialize();
    if (!NT_SUCCESS(status))
    {
        goto InitializeEnd;
    }

    //
    // One time only initialization.
    //
    if (BdSubsystemInitialized == FALSE)
    {
        BdInitializeDebuggerDataBlock();

        //
        // Stop after minimal initialization if the debugger is not enabled.
        //
        if (!BdSubsystemEnabled)
        {
            goto InitializeEnd;
        }

        //
        // Only initialize the boot debugger if specified in the application's
        // options.  Considered exit successful if there was no request to
        // initialize the debugger.
        //

        optionStatus = BdpGetConnectionParameters(&BdParameters);
        if (!NT_SUCCESS(optionStatus))
        {
            goto InitializeEnd;
        }

        //
        // Store the type of debugger being used and configure the debugger device
        // with the given parameters.
        //

        BdDebuggerType = BdParameters.Type;
        if (BdDebuggerType == BdSerial)
        {
            BdTransportMaxPacketSize = LEGACY_PACKET_MAX_SIZE;
            BdSendPacket = BdComSendPacket;
            BdReceivePacket = BdComReceivePacket;
        }

        //
        // Perform architecture specific boot debugger initialization.
        //

        status = BdpConfigureDebuggerDevice(&BdParameters);
        if (!NT_SUCCESS(status))
        {
            goto InitializeEnd;
        }

        portOpened = TRUE;

        //
        // The debugger port has been initialized.  Attempt to initialize
        // debugger settings.
        //

        BdBreakpointInstruction = BD_BREAKPOINT_VALUE;
        for (index = 0; index < BREAKPOINT_TABLE_SIZE; index += 1)
        {
            BdBreakpointTable[index].Flags = 0;
            BdBreakpointTable[index].Address = 0;
        }

        //
        // Get the initial DebugPrint mask from PCD.
        //

        BdDebugPrintGlobalMask = PcdGet32(PcdDebugPrintErrorLevel);

        //
        // Subsystem initialization is now complete.  Indicate that subsystem
        // resources are now allocated and can only be freed by calling the
        // subsystem destruction routine.
        //

        BdSubsystemInitialized = TRUE;

    }

    //
    // Common initialization.
    // This is also needed when we are reinitializing the debugger.
    //

    //
    // Attempt to initiate the debugger session.
    //

    BdStart();

InitializeEnd:
    if (!NT_SUCCESS(status))
    {
        if (portOpened != FALSE)
        {
            BdpCloseDebuggerDevice();
        }

        BdArchDestroy();
    }
    
    BdSerialPrint("<<< %a: %r\n", __FUNCTION__, status);

    return status;
}

// ---------------------------------------------------------------- Private API

NTSTATUS
BdDestroy (
    VOID
    )
/*++

Routine Description:

    Unloads current debugger symbols and closes the debugger port.

Arguments:

    None.

Return Value:

    STATUS_SUCCESS always.

--*/
{

    //
    // Disable the boot debugger.
    //

    if (BdConnectionActive != FALSE)
    {
        BdStop();
    }

    //
    // Destroy all remaining debugger related state.
    //

    if (BdSubsystemInitialized != FALSE)
    {
        BdSuspendAllBreakpoints();
        BdpCloseDebuggerDevice();
        BdArchDestroy();
        BdSubsystemInitialized = FALSE;
    }

    return STATUS_SUCCESS;
}


BOOLEAN
BdDebuggerInitialized (
    VOID
    )
/*++

Routine Description:

    This routine returns a BOOLEAN indicating whether the debugger subsystem
    is initialized.

Arguments:

    None.

Return Value:

    TRUE if a debugger subsystem is initialized.
    FALSE otherwise.

--*/
{

    BOOLEAN debuggerInitialized;

    //
    // N.B. This routine does not have interface enter/exit calls to ensure
    //      that it can be called from any context, most notably from physical
    //      mode in a virtual mode application.  This is required because this
    //      routine is called during the execution of asserts.
    //

    if ((BdSubsystemInitialized != FALSE) &&
        (BdArchBlockDebuggerOperation == FALSE))
    {
        debuggerInitialized = TRUE;
    }
    else
    {
        debuggerInitialized = FALSE;
    }

    return debuggerInitialized;
}

// --------------------------------------------------------- Internal Functions

VOID
BdpCloseDebuggerDevice (
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

    if (BdDebuggerType == BdSerial)
    {
        BdComCloseDebuggerDevice();
    }

    return;
}


NTSTATUS
BdpConfigureDebuggerDevice (
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

    if (Parameters->Type == BdSerial)
    {
        return BdComConfigureDebuggerDevice(Parameters);
    }

    return STATUS_INVALID_PARAMETER;
}


VOID
BdpDeviceStop (
    VOID
    )
/*++

Routine Description:

    This routine performs any device specific actions required before an
    active debugger session can be disconnection.

Arguments:

    None.

Return Value:

    None.

--*/
{
    return;
}


NTSTATUS
BdpGetConnectionParameters (
    __out PBD_CONNECTION_PARAMETERS Parameters
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

    NTSTATUS status;

    status = STATUS_UNSUCCESSFUL;
    Parameters->Type = BdSerial;
    switch (Parameters->Type)
    {
    case BdSerial:
        status = BdComGetConnectionParameters(Parameters);
        break;

    case BdNet:
    default:
        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}


NTSTATUS
BdpReconfigureDebuggerDevice (
    VOID
    )

/*++

Routine Description:

    This routine reconfigures a previously configured debugger device.

Arguments:

    None.

Return Value:

    NT Status code.

--*/

{

    NTSTATUS status;

    status = STATUS_SUCCESS;

    return status;
}

UINT64
BdpSentReceivedPacketCount(
    VOID
    )

/*++

Routine Description:

    Get the sent and received packet counts from the transport layer.

Arguments:

    None.

Return Value:

    This routine returns the total number of successfully sent KD packets in
    the top 32 bits, and the total number of successfully received KD packets in
    the bottom 32 bits of the 64 bit return value.

--*/

{
    UINT64 packetCount;

    packetCount = 0;
    if (BdDebuggerType == BdSerial)
    {
        packetCount = BdComSentReceivedPacketCount();
    }

    return packetCount;
}

VOID
BdSerialPrint (
  IN  CONST CHAR8  *Format,
  ...
  )
{
#if 0
  CHAR8    Buffer[0x100];
  VA_LIST  Marker;

  //
  // If Format is NULL, then ASSERT().
  //
  ASSERT (Format != NULL);

  //
  // Convert the message to an ASCII String
  //
  VA_START (Marker, Format);
  AsciiVSPrint (Buffer, sizeof (Buffer), Format, Marker);
  VA_END (Marker);

  //
  // Send the print string to a Serial Port
  //
  SerialPortWrite ((UINT8 *)Buffer, AsciiStrLen (Buffer));
#endif
}

