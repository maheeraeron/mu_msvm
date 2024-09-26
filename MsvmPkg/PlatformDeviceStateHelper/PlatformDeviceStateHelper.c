/** @file
  Sets up the device state variable for use on displaying the device state

  Copyright (c) Microsoft Corporation.
  Licensed under the BSD-2-Clause-Patent license.
**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DeviceStateLib.h>
#include <Library/PcdLib.h>
#include <IsolationTypes.h>

#include <Guid/GlobalVariable.h>


/**
    Check if secure boot is enabled

    @retval     TRUE  Secure boot is enabled
                FALSE Secure boot is disabled
**/
BOOLEAN
IsSecureBootOn()
{
    EFI_STATUS  Status = EFI_DEVICE_ERROR;
    UINT8      *Value = NULL;
    UINTN       Size = 0;

    //
    // For now, no hardware isolated platforms with no paravisor support secure boot.
    //
    if (IsHardwareIsolatedNoParavisor()) {
        return FALSE;
    }

    Status = GetVariable2(L"SecureBoot", &gEfiGlobalVariableGuid, (VOID **)&Value, &Size);
    if (EFI_ERROR (Status) || (Value == NULL)) {
        DEBUG ((DEBUG_ERROR, "%a - Failed to read SecureBoot variable.  Status = %r\n", __FUNCTION__, Status));
        return FALSE;
    }

    ASSERT(Size == 1);

    if(*Value == 1) {
        DEBUG((DEBUG_INFO, "%a - Secure boot on\n", __FUNCTION__));
        FreePool (Value);
        return TRUE;
    }

    DEBUG((DEBUG_INFO, "%a - Secure boot off\n", __FUNCTION__));
    FreePool (Value);
    return FALSE;
}

/**
    Set up the device state variable for use later in displaying the device state

    @param[in]  FileHandle   Handle of the file being invoked.

    @param[in]  PeiServices  General purpose services available to every PEIM.

    @retval     EFI_SUCCESS  Always returns success.
**/
EFI_STATUS
EFIAPI
PlatformDeviceStateHelperInit(
    IN EFI_HANDLE                   ImageHandle,
    IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
    DEVICE_STATE CoreNotifications = 0;

    DEBUG((DEBUG_INFO, "Starting %a \n", __FUNCTION__));

    //
    //Handle checking for "Common" On screen notifications
    //
    if (!IsSecureBootOn())
    {
        CoreNotifications |= DEVICE_STATE_SECUREBOOT_OFF;
    }

    if (PcdGetBool(PcdDebuggerEnabled))
    {
        CoreNotifications |= DEVICE_STATE_SOURCE_DEBUG_ENABLED;
    }

#if defined(DEBUG_PLATFORM)
    CoreNotifications |= DEVICE_STATE_DEVELOPMENT_BUILD_ENABLED;
#endif

    AddDeviceState(CoreNotifications);

    return EFI_SUCCESS;
}