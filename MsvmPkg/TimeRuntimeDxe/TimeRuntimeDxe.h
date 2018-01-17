/*++

Copyright (c) Microsoft Corporation

Module Name:

    TimeRuntimeDxe.h

Abstract:

    Implements the UEFI side of the RTC used for AARCH64.

--*/

#pragma once

#include <EfiNt.h>
#include <Library/BaseMemoryLib.h>
#include <Library/Baselib.h>
#include <Library/BiosDeviceLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Guid/EventGroup.h>
#include <BiosInterface.h>

#include <Protocol/RealTimeClock.h>

//
// The struct used to marshal EFI_TIME to the Bios Vdev.
//
#pragma pack(push, 1)

typedef struct _VM_EFI_TIME
{
    EFI_STATUS Status;
    EFI_TIME   Time;
} VM_EFI_TIME, *PVM_EFI_TIME;

#pragma pack(pop)

EFI_STATUS
VmSetTime(
    _In_ EFI_TIME                   *Time
    );

EFI_STATUS
VmGetTime(
    _Out_     EFI_TIME              *Time,
    _Out_opt_ EFI_TIME_CAPABILITIES *Capabilities
    );

EFI_STATUS
VmSetWakeupTime(
    _In_     BOOLEAN                Enable,
    _In_opt_ EFI_TIME               *Time
    );

EFI_STATUS
VmGetWakeupTime(
    _Out_ BOOLEAN                   *Enabled,
    _Out_ BOOLEAN                   *Pending,
    _Out_ EFI_TIME                  *Time
    );

EFI_STATUS
EFIAPI
InitializeTimeDxe(
    _In_ EFI_HANDLE                  ImageHandle,
    _In_ EFI_SYSTEM_TABLE            *SystemTable
    );
