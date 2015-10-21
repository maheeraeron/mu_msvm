/*++

Copyright (c) Microsoft Corporation

Module Name:

    BootEventLogLib.h

Abstract:

    Library wrapper around EFI_EVENTLOG_PROTOCOL for logging boot events

Author:

    Kris Harper (kharp) - 11-Dec-2013

--*/

#pragma once
#include <EfiNt.h>
#include <Library/EventLogLib.h>
#include <BiosBootLogInterface.h>

extern EFI_GUID gBootEventChannelGuid;


EFI_STATUS
EFIAPI
BootDeviceEventStart(
    _In_    const EFI_DEVICE_PATH_PROTOCOL     *DevicePath,
    _In_    UINT16                              BootVariableNumber,
    _In_    BOOT_DEVICE_STATUS                  Status,
    _In_    EFI_STATUS                          ExtendedStatus
    );


EFI_STATUS
EFIAPI
BootDeviceEventUpdate(
    _In_    BOOT_DEVICE_STATUS                  Status,
    _In_    EFI_STATUS                          ExtendedStatus
    );


EFI_STATUS
EFIAPI
BootDeviceEventPendingStatus(
    _Out_   BOOT_DEVICE_STATUS                 *Status,
    _Out_   EFI_STATUS                         *ExtendedStatus
    );


EFI_STATUS
EFIAPI
BootDeviceEventComplete(
    VOID
    );


EFI_STATUS
EFIAPI
BootDeviceEventResetLog(
    VOID
    );


EFI_STATUS
EFIAPI
BootDeviceEventFlushLog(
    VOID
    );


EFI_STATUS
EFIAPI
BootDeviceEventStatistics(
    _Out_   EVENT_CHANNEL_STATISTICS           *Stats
    );


EFI_STATUS
EFIAPI
BootDeviceEventEnumerate(
    _In_    EFI_EVENTLOG_ENUMERATE_CALLBACK     Callback,
    _In_    const VOID                         *Context
    );