/** @file
  Platform BDS customizations.

  Copyright (c) 2004 - 2009, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BootEventLogLib.h>
#include <Guid/EventGroup.h>

static EFI_EVENT    mExitBootServicesEvent = NULL;

static
VOID
EFIAPI
ExitBootServicesHandler(
    _In_ EFI_EVENT Event,
    _In_ void*     Context
    )
/*++

Routine Description:

    Called when the Exit Boot Services event is signalled.

Arguments:

    Event - Event.

    Context - Context.

Returns:

--*/
{
    //
    // Complete the currently pending boot event and flush the event log
    //
    BootDeviceEventUpdate(BootDeviceOsLoaded, EFI_SUCCESS);
    DEBUG((DEBUG_INFO, "[HVBE] Updating boot event: BootDeviceOsLoaded, EFI_SUCCESS\n"));
    BootDeviceEventFlushLog();
    DEBUG((DEBUG_INFO, "[HVBE] Flushing boot event log\n"));
}


STATIC
VOID
EFIAPI
EmptyCallbackFunction (
    IN EFI_EVENT                Event,
    IN VOID                     *Context
    )
/*++

Routine Description:

    Empty callback function for CreateEventEx

--*/
{
}

//============================================================================
// Begin Exported Platform BDS Library Functions
//============================================================================


VOID
EFIAPI
PlatformBdsInit(
      VOID
      )
/*++

Routine Description:

    Platform BDS initialization.

Arguments:

    None.

Returns:

    n/a.

--*/
{
    EFI_EVENT endOfDxeEvent;
    EFI_STATUS status;

    DEBUG((EFI_D_INFO, "MsPlatBdsInit\n"));

    //
    // Register to get a callback when ExitBootServices is called.
    //
    status = gBS->CreateEventEx(EVT_NOTIFY_SIGNAL,
                                TPL_NOTIFY,
                                ExitBootServicesHandler,
                                NULL,
                                &gEfiEventExitBootServicesGuid,
                                &mExitBootServicesEvent);
    ASSERT_EFI_ERROR(status);

    //
    // Signal EndOfDxe PI Event
    //
    status = gBS->CreateEventEx (
                 EVT_NOTIFY_SIGNAL,
                 TPL_CALLBACK,
                 EmptyCallbackFunction,
                 NULL,
                 &gEfiEndOfDxeEventGroupGuid,
                 &endOfDxeEvent
                 );
    if (!EFI_ERROR (status)) {
        gBS->SignalEvent (endOfDxeEvent);
        gBS->CloseEvent (endOfDxeEvent);
    }
}

// ============================================================================
// End Exported Platform BDS Library Functions
// ============================================================================

