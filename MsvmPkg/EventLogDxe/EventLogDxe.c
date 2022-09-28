/*++

Copyright (c) Microsoft Corporation

Module Name:

    EventLogDxe.c

Abstract:

    Entry point and initialization for combined status code and event logging driver

Author:

    Kris Harper (kharp) - 20-Nov-2013

Environment:

    UEFI

--*/

#include "EventLogDxe.h"
#include <Library/UefiDriverEntryPoint.h>
#include "StatusCode.h"
#include "EventLogger.h"

EFI_HV_PROTOCOL *mHv;
EFI_HV_IVM_PROTOCOL *mHvIvm;

EFI_STATUS
EFIAPI
EventLogDxeEntry (
    _In_    EFI_HANDLE          ImageHandle,
    _In_    EFI_SYSTEM_TABLE   *SystemTable
    )
/*++

Routine Description:

  Entry point of DXE Status Code Driver.

  This function is the entry point of this DXE Status Code Driver.
  It installs Status Code Runtime Protocol

Arguments:

    ImageHandle     The firmware allocated handle for the EFI image.

    SystemTable     A pointer to the EFI System Table.

Return Value:

    EFI_SUCCESS on success

--*/
{
    EFI_STATUS  status;

    DEBUG((DEBUG_INIT, "EventLog Driver Starting\n"));
    //
    // Initialize the event channel management and then the status code protocol
    //
    status = EventLoggerInitialize();

    if (EFI_ERROR(status))
    {
        goto Exit;
    }

    status = gBS->LocateProtocol(&gEfiHvProtocolGuid, NULL, (VOID **)&mHv);

    if (EFI_ERROR(status))
    {
        goto Exit;
    }

    status = gBS->LocateProtocol(&gEfiHvIvmProtocolGuid, NULL, (VOID **)&mHvIvm);

    if (EFI_ERROR(status))
    {
        goto Exit;
    }

    //
    // Don't fail driver initialization if this fails.
    //
    StatusCodeRuntimeInitialize();

    //
    // Workaroud: Initialize BootEventLogLib library.  This is done because BootEventLogLib 
    //            library requires gEfiEventLogProtocolGuid, which is not available at the 
    //            time of its constructor execution. 
    //
    BootEventLogLibInit(ImageHandle, SystemTable);

Exit:

    return status;
}
