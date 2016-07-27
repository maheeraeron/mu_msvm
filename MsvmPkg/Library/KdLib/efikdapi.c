/*++

Copyright (c) 1990-2004  Microsoft Corporation

Module Name:

    EfiKdApi.c

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
#include <Library/KdDebugLib.h>

#include "EfiKd.h"

// -------------------------------------------------------------------- Defines

#define CONNECTION_RETRIES 10

// -------------------------------------------------------------------- Globals

BOOLEAN EfiKdConnectionActive = FALSE;
BOOLEAN EfiKdDebuggerInitialConnection = FALSE;
LIST_ENTRY EfiKdModuleList = { &EfiKdModuleList, &EfiKdModuleList };
UINT32 EfiKdModuleListCount = 0;
PLDR_DATA_TABLE_ENTRY EfiKdModuleDataTableEntry = NULL;
KDDEBUGGER_DATA64 EfiKdDebuggerDataBlock;
LIST_ENTRY EfiKdDebuggerDataListHead = { &EfiKdDebuggerDataListHead, &EfiKdDebuggerDataListHead };
PEFI_UNLOADED_MODULE EfiKdUnloadedModulesPtr = EfiKdUnloadedModules;

PEFI_KD_RECEIVE_PACKET EfiKdReceivePacket;
PEFI_KD_SEND_PACKET EfiKdSendPacket;

EFI_KD_CONNECTION_TYPE EfiKdDebuggerType;
EFI_KD_CONNECTION_PARAMETERS EfiKdParameters;

// ----------------------------------------------------------------- Prototypes

VOID
EfiKdpCloseDebuggerDevice (
    VOID
    );

NTSTATUS
EfiKdpConfigureDebuggerDevice (
    __in PEFI_KD_CONNECTION_PARAMETERS Parameters
    );

VOID
EfiKdpDeviceStop (
    VOID
    );

NTSTATUS
EfiKdpGetConnectionParameters (
    __out PEFI_KD_CONNECTION_PARAMETERS Parameters
    );

VOID
EfiKdpFreeDataTableEntry (
    __in PLDR_DATA_TABLE_ENTRY TableEntry
    );

NTSTATUS
EfiKdpModuleNameFromImage (
    __in_bcount(ImageSize) PVOID ImageBase,
    __in UINT32 ImageSize,
    __out PSTRING ModuleName
    );

UINT64
EfiKdpSentReceivedPacketCount(
    VOID
    );

NTSTATUS
EfiKdpReconfigureDebuggerDevice (
    VOID
    );

// ------------------------------------------------------------------ Functions

BOOLEAN
EfiKdDebuggerEnabled (
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

    debuggerInitialized = EfiKdDebuggerInitialized();
    if ((debuggerInitialized != FALSE) && (EfiKdDebuggerNotPresent == FALSE))
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
EfiKdNotifyShutdown(
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
EfiKdStop (
    VOID
    )
/*++

Routine Description:

    This routine will unload the image symbols and temporarily stop debugging.

    It will not close the debugger port or reset any global state information.
    A call to EfiKdStart will resume the debugger session.

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

    if ((EfiKdConnectionActive != FALSE))
    {
        //
        // If the initial debugger connection succeeded, or if any packets were
        // successfully sent to or received from the host, then ensure that the
        // disconnect message is sent as well.
        //

        if ((EfiKdDebuggerInitialConnection != FALSE) ||
            (EfiKdpSentReceivedPacketCount() != 0))
        {
            EfiKdDebuggerNotPresent = FALSE;
        }

        EfiKdNotifyShutdown(KD_REBOOT);
        EfiKdArchStop();
        EfiKdpDeviceStop();
        EfiKdConnectionActive = FALSE;
    }

    return;
}

VOID
EfiKdStart (
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

    if ((EfiKdSubsystemInitialized == FALSE) || (EfiKdConnectionActive != FALSE))
    {
        goto StartEnd;
    }

    //
    // Reconfigure the debugger device for usage.  This is required if another
    // application configured the device since it was first initialized.
    //

    status = EfiKdpReconfigureDebuggerDevice();
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

    EfiKdArchStart();

    //
    // A connection attempt will be made.  Mark the debugger connection
    // active, it will be marked inactive when the connection is stopped.
    //

    EfiKdConnectionActive = TRUE;

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
        EfiKdDebuggerNotPresent = FALSE;
#define INIT_STRING "EfiKd: EFI Debugger Initialized\n"
        DebugService2(INIT_STRING, (PVOID)sizeof(INIT_STRING), BREAKPOINT_PRINT);
        retries -= 1;
    } while ((EfiKdDebuggerNotPresent != FALSE) && (retries > 0));

    //
    // Track initial connection success.
    //

    EfiKdDebuggerInitialConnection = !EfiKdDebuggerNotPresent;

StartEnd:
    return;
}


VOID
EfiKdInitializeDebuggerDataBlock (
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
    ZeroMem(EfiKdUnloadedModules, (sizeof(EFI_UNLOADED_MODULE) * EFI_KD_MAX_UNLOADED_MODULES));

    EfiKdDebuggerDataBlock.Header.OwnerTag = 'GBDK';
    EfiKdDebuggerDataBlock.Header.Size = sizeof(EfiKdDebuggerDataBlock);
    EfiKdDebuggerDataBlock.PsLoadedModuleList = (UINTN)&EfiKdModuleList;
    EfiKdDebuggerDataBlock.BreakpointWithStatus = (UINTN)EfiKdBreakPointWithStatus;
    // kernel base will be filled in after debugger has initialized.
    EfiKdDebuggerDataBlock.KernBase = 0;
    EfiKdDebuggerDataBlock.SizePrcb = sizeof(KPRCB);
    EfiKdDebuggerDataBlock.SizePcr  = sizeof(KPCR);
    //
    // MmUnloadedDrivers contains the address of a pointer to the base of
    // the unloaded modules array.
    //
    EfiKdDebuggerDataBlock.MmUnloadedDrivers = (UINT64)&EfiKdUnloadedModulesPtr;
    EfiKdDebuggerDataBlock.MmLastUnloadedDriver = (UINT64)&EfiKdLastUnloadedModule;
    InsertTailList(&EfiKdDebuggerDataListHead,
                   &EfiKdDebuggerDataBlock.Header.List);
}


NTSTATUS
EfiKdInitialize (
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

    EfiKdNextPacketIdToSend = INITIAL_PACKET_ID | SYNC_PACKET_ID;
    EfiKdPacketIdExpected = INITIAL_PACKET_ID;

    status = EfiKdArchInitialize();
    if (!NT_SUCCESS(status))
    {
        goto InitializeEnd;
    }

    //
    // One time only initialization.
    //
    if (EfiKdSubsystemInitialized == FALSE)
    {
        EfiKdInitializeDebuggerDataBlock();

        //
        // Stop after minimal initialization if the debugger is not enabled.
        //
        if (!EfiKdSubsystemEnabled)
        {
            goto InitializeEnd;
        }

        //
        // Only initialize the boot debugger if specified in the application's
        // options.  Considered exit successful if there was no request to
        // initialize the debugger.
        //

        optionStatus = EfiKdpGetConnectionParameters(&EfiKdParameters);
        if (!NT_SUCCESS(optionStatus))
        {
            goto InitializeEnd;
        }

        //
        // Store the type of debugger being used and configure the debugger device
        // with the given parameters.
        //

        EfiKdDebuggerType = EfiKdParameters.Type;
        if (EfiKdDebuggerType == EfiKdSerial)
        {
            KdTransportMaxPacketSize = LEGACY_PACKET_MAX_SIZE;
            EfiKdSendPacket = EfiKdComSendPacket;
            EfiKdReceivePacket = EfiKdComReceivePacket;
        }

        //
        // Perform architecture specific boot debugger initialization.
        //

        status = EfiKdpConfigureDebuggerDevice(&EfiKdParameters);
        if (!NT_SUCCESS(status))
        {
            goto InitializeEnd;
        }

        portOpened = TRUE;

        //
        // The debugger port has been initialized.  Attempt to initialize
        // debugger settings.
        //

        EfiKdBreakpointInstruction = EFI_KD_BREAKPOINT_VALUE;
        for (index = 0; index < BREAKPOINT_TABLE_SIZE; index += 1)
        {
            EfiKdBreakpointTable[index].Flags = 0;
            EfiKdBreakpointTable[index].Address = 0;
        }

        //
        // Get the initial DebugPrint mask from PCD.
        //

        EfiKdDebugPrintGlobalMask = PcdGet32(PcdDebugPrintErrorLevel);

        //
        // Subsystem initialization is now complete.  Indicate that subsystem
        // resources are now allocated and can only be freed by calling the
        // subsystem destruction routine.
        //

        EfiKdSubsystemInitialized = TRUE;

    }

    //
    // Common initialization.
    // This is also needed when we are reinitializing the debugger.
    //

    //
    // Attempt to initiate the debugger session.
    //

    EfiKdStart();

InitializeEnd:
    if (!NT_SUCCESS(status))
    {
        if (portOpened != FALSE)
        {
            EfiKdpCloseDebuggerDevice();
        }

        EfiKdArchDestroy();
    }

    return status;
}

// ---------------------------------------------------------------- Private API

NTSTATUS
EfiKdDestroy (
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

    if (EfiKdConnectionActive != FALSE)
    {
        EfiKdStop();
    }

    //
    // Destroy all remaining debugger related state.
    //

    if (EfiKdSubsystemInitialized != FALSE)
    {
        EfiKdSuspendAllBreakpoints();
        EfiKdpCloseDebuggerDevice();
        EfiKdArchDestroy();
        EfiKdSubsystemInitialized = FALSE;
    }

    return STATUS_SUCCESS;
}


BOOLEAN
EfiKdDebuggerInitialized (
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

    if ((EfiKdSubsystemInitialized != FALSE) &&
        (EfiKdArchBlockDebuggerOperation == FALSE))
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
EfiKdpCloseDebuggerDevice (
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

    if (EfiKdDebuggerType == EfiKdSerial)
    {
        EfiKdComCloseDebuggerDevice();
    }

    return;
}


NTSTATUS
EfiKdpConfigureDebuggerDevice (
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

    if (Parameters->Type == EfiKdSerial)
    {
        return EfiKdComConfigureDebuggerDevice(Parameters);
    }

    return STATUS_INVALID_PARAMETER;
}


VOID
EfiKdpDeviceStop (
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
EfiKdpGetConnectionParameters (
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

    NTSTATUS status;

    status = STATUS_UNSUCCESSFUL;
    Parameters->Type = EfiKdSerial;
    switch (Parameters->Type)
    {
    case EfiKdSerial:
        status = EfiKdComGetConnectionParameters(Parameters);
        break;

    case EfiKdNet:
    default:
        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}


NTSTATUS
EfiKdpReconfigureDebuggerDevice (
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
EfiKdpSentReceivedPacketCount(
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
    if (EfiKdDebuggerType == EfiKdSerial)
    {
        packetCount = EfiKdComSentReceivedPacketCount();
    }
    
    return packetCount;
}
