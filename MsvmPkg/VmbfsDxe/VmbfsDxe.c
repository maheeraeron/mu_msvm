/*++

Copyright (c) 1990-2014  Microsoft Corporation

Module Name:

    VmbfsDxe.c

Abstract:

    EFI simple file system protocol over vmbus driver entry and
    driver binding protocol implementation.

Author:

    Xinnuo Zhang (xinnuoz) 21-Nov-2014

Environment:

    EFI

--*/

#include "VmbfsEfi.h"

VOID
VmbfsCleanup (
    _In_ EFI_DRIVER_BINDING_PROTOCOL *This,
    _In_ EFI_HANDLE ControllerHandle,
    _In_ PVMBFS_SIMPLE_FILE_SYSTEM_PROTOCOL SimpleFileSystemProtocol
    );

EFI_STATUS
EFIAPI
VmbfsSupported(
    _In_ EFI_DRIVER_BINDING_PROTOCOL    *This,
    _In_ EFI_HANDLE                     Controller,
    _In_ EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
    )

/*++

Routine Description:

    Test to see if this driver supports ControllerHandle. This service
    is called by the EFI boot service ConnectController(). In
    order to make drivers as small as possible, there are a few calling
    restrictions for this service. ConnectController() must
    follow these calling restrictions. If any other agent wishes to call
    Supported() it must also follow these calling restrictions.

Arguments:

    This                Protocol instance pointer.

    ControllerHandle    Handle of device to test.

    RemainingDevicePath Optional parameter use to pick a specific child
                              device to start.

Return Value:

    EFI_SUCCESS         This driver supports this device.

    EFI_ALREADY_STARTED This driver is already running on this device.

    other               This driver does not support this device.

--*/

{
    EFI_STATUS status;
    EFI_VMBUS_PROTOCOL *vmbus;

    UNREFERENCED_PARAMETER(RemainingDevicePath);

    status = gBS->OpenProtocol(
        Controller,
        &gEfiVmbusProtocolGuid,
        (VOID **) &vmbus,
        This->DriverBindingHandle,
        Controller,
        EFI_OPEN_PROTOCOL_BY_DRIVER);

    if (EFI_ERROR(status))
    {
        goto Exit;
    }

    gBS->CloseProtocol(
        Controller,
        &gEfiVmbusProtocolGuid,
        This->DriverBindingHandle,
        Controller);

    status = EmclChannelTypeSupported(
        Controller,
        &GUID_VMBFS_INTERFACE_TYPE,
        This->DriverBindingHandle);

Exit:
  return status;
}


EFI_STATUS
EFIAPI
VmbfsStart (
    _In_ EFI_DRIVER_BINDING_PROTOCOL *This,
    _In_ EFI_HANDLE ControllerHandle,
    _In_opt_ EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
    )

/*++

Routine Description:

    Start this driver on ControllerHandle. This service is called by the
    EFI boot service ConnectController(). In order to make
    drivers as small as possible, there are a few calling restrictions for
    this service. ConnectController() must follow these
    calling restrictions. If any other agent wishes to call Start() it
    must also follow these calling restrictions.

Arguments:

    This                 Protocol instance pointer.

    ControllerHandle     Handle of device to bind driver to.

    RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start.

Return Value:

    EFI_SUCCESS          This driver is added to ControllerHandle

    EFI_ALREADY_STARTED  This driver is already running on ControllerHandle

    other                This driver does not support this device

--*/


{
    EFI_STATUS status;
    PVMBFS_SIMPLE_FILE_SYSTEM_PROTOCOL simpleFileSystemProtocol = NULL;
    PFILESYSTEM_INFORMATION fileSystemInformation = NULL;
    EFI_EMCL_PROTOCOL *emclProtocol = NULL;
    EFI_DEVICE_PATH_PROTOCOL *devicePath = NULL;
    BOOLEAN EmclInstalled = FALSE;

    UNREFERENCED_PARAMETER(RemainingDevicePath);

    //
    // Check if device already running.
    //

    status = gBS->OpenProtocol(
        ControllerHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID **)&simpleFileSystemProtocol,
        This->DriverBindingHandle,
        ControllerHandle,
        EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

    if (!EFI_ERROR(status))
    {
        return EFI_ALREADY_STARTED;
    }

    simpleFileSystemProtocol = NULL;

    //
    // Connect to EMCL.
    //
    status = EmclInstallProtocol(ControllerHandle);

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    EmclInstalled = TRUE;

    status =
        gBS->AllocatePool(EfiBootServicesData,
                          sizeof(*simpleFileSystemProtocol),
                          &simpleFileSystemProtocol);

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    fileSystemInformation = &simpleFileSystemProtocol->FileSystemInformation;
    ZeroMem(fileSystemInformation, sizeof(*fileSystemInformation));

    status = gBS->OpenProtocol(
        ControllerHandle,
        &gEfiDevicePathProtocolGuid,
        &devicePath,
        This->DriverBindingHandle,
        ControllerHandle,
        EFI_OPEN_PROTOCOL_BY_DRIVER);

    if(EFI_ERROR(status))
    {
        goto Cleanup;
    }

    fileSystemInformation->DevicePathProtocol = devicePath;

    status = gBS->OpenProtocol(
        ControllerHandle,
        &gEfiEmclProtocolGuid,
        (VOID **)&emclProtocol,
        This->DriverBindingHandle,
        ControllerHandle,
        EFI_OPEN_PROTOCOL_BY_DRIVER);

    if(EFI_ERROR(status))
    {
        goto Cleanup;
    }

    fileSystemInformation->EmclProtocol = emclProtocol;

    CopyMem(&simpleFileSystemProtocol->EfiSimpleFileSystemProtocol,
            &gVmbFsSimpleFileSystemProtocol,
            sizeof(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL));

    status = gBS->InstallMultipleProtocolInterfaces(&ControllerHandle,
                                                    &gEfiSimpleFileSystemProtocolGuid,
                                                    simpleFileSystemProtocol,
                                                    NULL);

    if(EFI_ERROR(status))
    {
        goto Cleanup;
    }

Cleanup:
    if (EFI_ERROR(status))
    {
        if (simpleFileSystemProtocol != NULL)
        {
            VmbfsCleanup(This, ControllerHandle, simpleFileSystemProtocol);
        }

        if (EmclInstalled)
        {
            EmclUninstallProtocol(ControllerHandle);
        }
    }

    return status;
}


VOID
VmbfsCleanup (
    _In_ EFI_DRIVER_BINDING_PROTOCOL *This,
    _In_ EFI_HANDLE ControllerHandle,
    _In_ PVMBFS_SIMPLE_FILE_SYSTEM_PROTOCOL SimpleFileSystemProtocol
    )

/*++

Routine Description:

    Cleanup the given VMBus simple file system protocol structure,
    freeing resources and releasing handles.

Arguments:

    This - A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.

    ControllerHandle - A handle to the device being stopped. The handle must
        support a bus specific I/O protocol for the driver to use to stop the
        device.

    SimpleFileSystemProtocol - Pointer to the Vmbus simple file system protocol
        to cleanup.

Return Value:

    None.

--*/

{
    PFILESYSTEM_INFORMATION fileSystemInformation;

    fileSystemInformation = &SimpleFileSystemProtocol->FileSystemInformation;

    if (fileSystemInformation->EmclProtocol != NULL)
    {
        gBS->CloseProtocol(ControllerHandle,
                           &gEfiEmclProtocolGuid,
                           This->DriverBindingHandle,
                           ControllerHandle);
    }

    if (fileSystemInformation->DevicePathProtocol != NULL)
    {
        gBS->CloseProtocol(ControllerHandle,
                           &gEfiDevicePathProtocolGuid,
                           This->DriverBindingHandle,
                           ControllerHandle);
    }

    gBS->FreePool(SimpleFileSystemProtocol);
}


EFI_STATUS
EFIAPI
VmbfsStop (
    _In_ EFI_DRIVER_BINDING_PROTOCOL *This,
    _In_ EFI_HANDLE ControllerHandle,
    _In_ UINTN NumberOfChildren,
    _In_opt_ EFI_HANDLE *ChildHandleBuffer
)

/*++

Routine Description:

    If NumberOfChildren is zero, then the driver specified by This is either
    a device driver or a bus driver, and it is being requested to stop the
    controller specified by ControllerHandle. If ControllerHandle is stopped,
    then EFI_SUCCESS is returned. In either case, this function is required
    to undo what was performed in Start(). Whatever resources are allocated
    in Start() must be freed in Stop().

Arguments:

    This - A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.

    ControllerHandle - A handle to the device being stopped. The handle must
        support a bus specific I/O protocol for the driver to use to stop the
        device.

    NumberOfChildren - The number of child device handles in ChildHandleBuffer.
        Expected to be 0 since Vmbfs is not a bus driver.

    ChildHandleBuffer - An array of child handles to be freed.
        May be NULL if NumberOfChildren is 0.

Return Value:

    EFI_SUCCESS - The device was stopped.

    EFI_DEVICE_ERROR - The device could not be stopped due to a device error.

--*/

{
    EFI_STATUS status;
    PVMBFS_SIMPLE_FILE_SYSTEM_PROTOCOL simpleFileSystemProtocol;

    status = gBS->OpenProtocol(
        ControllerHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID **)&simpleFileSystemProtocol,
        This->DriverBindingHandle,
        ControllerHandle,
        EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

    if (EFI_ERROR(status))
    {
        return EFI_DEVICE_ERROR;
    }

    status = gBS->UninstallMultipleProtocolInterfaces(ControllerHandle,
                                                      gEfiSimpleFileSystemProtocolGuid,
                                                      NULL);

    ASSERT(simpleFileSystemProtocol->FileSystemInformation.ReferenceCount == 0);

    VmbfsCleanup(This, ControllerHandle, simpleFileSystemProtocol);

    EmclUninstallProtocol(ControllerHandle);

    return EFI_SUCCESS;
}


EFI_DRIVER_BINDING_PROTOCOL gVmbfsDriverBindingProtocol =
{
    VmbfsSupported,
    VmbfsStart,
    VmbfsStop,
    0x1,
    0,
    0
};


EFI_STATUS
EFIAPI
VmbfsEntry (
    _In_ EFI_HANDLE       ImageHandle,
    _In_ EFI_SYSTEM_TABLE *SystemTable
    )
/*++

Routine Description:

    VMBus File System driver entry point.

Arguments:

    ImageHandle - The driver image handle.

    SystemTable       The system table.

Return Value:

    EFI_SUCEESS - Initialization routine has found a device,
                      loaded it's ROM, and installed a notify event.

    Other - Return value from InstallMultipleProtocolInterfaces.

--*/
{
    EFI_STATUS status;

    gVmbfsDriverBindingProtocol.ImageHandle = ImageHandle;
    gVmbfsDriverBindingProtocol.DriverBindingHandle = ImageHandle;

    status = gBS->InstallMultipleProtocolInterfaces(
                    &ImageHandle,
                    &gEfiDriverBindingProtocolGuid,
                    &gVmbfsDriverBindingProtocol,
                    NULL);

    return status;
}


