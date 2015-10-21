/*++

Copyright (c) Microsoft Corporation

Module Name:

    Console.c

Abstract:

    Provides the implementation for Hyper-V null console.

Author:

    Larry Cleeton (lcleeton) - 01-Dec-2014

--*/

#include "console.h"

//
// The console modes supported by this implementation.
// Only the UEFI defined mode 0 and 1 included.
//
typedef struct
{
    UINTN Columns;
    UINTN Rows;
} CONSOLE_MODE;

CONSOLE_MODE gConsoleModes[] = { {80,25}, {80,50} };

//
// console device object
//
#define CONSOLE_DEVICE_SIGNATURE SIGNATURE_32('c', 'o', 'n', 's')

typedef struct
{
    UINTN                                  Signature;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL         SimpleTextIn;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL        SimpleTextOut;
    EFI_SIMPLE_TEXT_OUTPUT_MODE            SimpleTextOutputMode;
} CONSOLE_DEVICE;

#define CONSOLE_DEVICE_FROM_TEXTIN(a) \
    CR(a, CONSOLE_DEVICE, SimpleTextIn, CONSOLE_DEVICE_SIGNATURE)

#define CONSOLE_DEVICE_FROM_TEXTOUT(a) \
    CR(a, CONSOLE_DEVICE, SimpleTextOut, CONSOLE_DEVICE_SIGNATURE)

//
// Starting template for a console device object.
//
CONSOLE_DEVICE gConsoleDeviceTemplate =
{
    CONSOLE_DEVICE_SIGNATURE,
    { // SimpleTextIn
        ConsoleInReset,
        ConsoleInReadKeyStroke,
        NULL                                        // WaitForKey event
    },
    { // SimpleTextOut
        ConsoleOutReset,
        ConsoleOutOutputString,
        ConsoleOutTestString,
        ConsoleOutQueryMode,
        ConsoleOutSetMode,
        ConsoleOutSetAttribute,
        ConsoleOutClearScreen,
        ConsoleOutSetCursorPosition,
        ConsoleOutEnableCursor,
        NULL                                        // ptr to SimpleTextOutputMode
    },
    { // SimpleTextOutputMode
        ARRAY_SIZE(gConsoleModes) - 1,              // MaxMode
        0,                                          // Mode
        EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK),    // Attribute
        0,                                          // CursorColumn
        0,                                          // CursorRow
        TRUE                                        // CursorVisible
    }
};

//
// The instance of the Driver Binding Protocol for the image handle.
//
EFI_DRIVER_BINDING_PROTOCOL gConsoleDriver =
{
    ConsoleDriverSupported,
    ConsoleDriverStart,
    ConsoleDriverStop,
    0xa,
    NULL,
    NULL
};

//
// The handle of the root device.
//
EFI_HANDLE gRootDevice = NULL;


EFI_STATUS
EFIAPI
ConsoleEntryPoint(
    _In_ EFI_HANDLE         ImageHandle,
    _In_ EFI_SYSTEM_TABLE   *SystemTable
    )
/*++

Routine Description:

    Entry point into this driver.

Arguments:

    ImageHandle - Handle of the driver image.

    SystemTable - EFI system table.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;
    EFI_DEVICE_PATH_PROTOCOL    *devicePath = NULL;
    EFI_DEV_PATH                node;

    //
    // Install the driver model protocols on the image handle.
    //
    status = EfiLibInstallDriverBindingComponentName2(ImageHandle,
                                                      SystemTable,
                                                      &gConsoleDriver,
                                                      ImageHandle,
                                                      &gConsoleComponentName,
                                                      &gConsoleComponentName2);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    //
    // This driver creates a permanent single root controller handle
    // that it subsequently manages.
    //
    // First build a VenHW device path for the controller.
    // Use the the private console protocol GUID as the vendor guid.
    //
    node.DevPath.Type = HARDWARE_DEVICE_PATH;
    node.DevPath.SubType = HW_VENDOR_DP;
    SetDevicePathNodeLength(&node.DevPath, sizeof(VENDOR_DEVICE_PATH));
    CopyMem(&node.Vendor.Guid, &gMsvmConsoleProtocolGuid, sizeof(EFI_GUID));
    devicePath = AppendDevicePathNode(devicePath, &node.DevPath);
    if (devicePath == NULL)
    {
        status = EFI_OUT_OF_RESOURCES;
        goto Cleanup;
    }

    //
    // Create a controller handle and install the following protocols:
    // - the device path protocol
    // - the private tag protocol
    //
    status = gBS->InstallMultipleProtocolInterfaces(&gRootDevice,
                                                    &gEfiDevicePathProtocolGuid,
                                                    devicePath,
                                                    &gMsvmConsoleProtocolGuid,
                                                    NULL,
                                                    NULL);
Cleanup:

    if (EFI_ERROR(status))
    {
        if (devicePath != NULL)
        {
            gBS->FreePool(devicePath);
        }
    }

    return status;
}


EFI_STATUS
EFIAPI
ConsoleDriverSupported(
    _In_ EFI_DRIVER_BINDING_PROTOCOL    *This,
    _In_ EFI_HANDLE                     ControllerHandle,
    _In_ EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
    )
/*++

Routine Description:

    Check to see if this driver supports the given controller

Arguments:

    This                    A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.

    ControllerHandle        The handle of the controller to test.

    RemainingDevicePath     A pointer to the remaining portion of a device path.


Return Value:

    EFI_SUCCESS             This driver can support the given controller

    EFI_ALREADY_STARTED     A driver already is managing this controller.

    EFI_UNSUPPORTED         This driver cannot support the given controller.

--*/
{
    EFI_STATUS  status;
    VOID        *protocol;

    //
    // Open the protocol that determines if this driver supports this
    // controller or if this driver is already managing this controller.
    //
    // In this case it is the private MSVM console tag protocol.
    //
    status = gBS->OpenProtocol(ControllerHandle,
                               &gMsvmConsoleProtocolGuid,
                               &protocol, // required but returns null for tag protocol
                               This->DriverBindingHandle,
                               ControllerHandle,
                               EFI_OPEN_PROTOCOL_BY_DRIVER);
    if (EFI_ERROR(status))
    {
        return status;
    }

    //
    // Close the protocol used to perform the supported test.
    //
    gBS->CloseProtocol(ControllerHandle,
                       &gMsvmConsoleProtocolGuid,
                       This->DriverBindingHandle,
                       ControllerHandle);

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
ConsoleDriverStart(
    _In_ EFI_DRIVER_BINDING_PROTOCOL    *This,
    _In_ EFI_HANDLE                     ControllerHandle,
    _In_ EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
    )
/*++

Routine Description:

    Start managing a controller.

Arguments:

    This                    A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.

    ControllerHandle              The handle of the controller to manage.

    RemainingDevicePath     A pointer to the remaining portion of a device path.

Return Value:

    EFI_SUCCESS             Driver started successfully.

    EFI_ALREADY_STARTED     A driver already is managing this controller.

--*/
{
    EFI_STATUS      status;
    VOID            *protocol;
    CONSOLE_DEVICE  *consoleDevice = NULL;

    //
    // Connect to the private MSVM console protocol on the controller handle.
    //
    status = gBS->OpenProtocol(ControllerHandle,
                               &gMsvmConsoleProtocolGuid,
                               &protocol,
                               This->DriverBindingHandle,
                               ControllerHandle,
                               EFI_OPEN_PROTOCOL_BY_DRIVER);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    //
    // Initialize a console device instance
    //
    consoleDevice = AllocateCopyPool(sizeof(CONSOLE_DEVICE), &gConsoleDeviceTemplate);
    if (consoleDevice == NULL)
    {
        status = EFI_OUT_OF_RESOURCES;
        goto Cleanup;
    }

    //
    // Create the event for Simple Text Input
    //
    status = gBS->CreateEvent(0,
                              0,
                              NULL,
                              NULL,
                              &consoleDevice->SimpleTextIn.WaitForKey);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    //
    // Pointer to Mode.
    //
    consoleDevice->SimpleTextOut.Mode = &(consoleDevice->SimpleTextOutputMode);

    //
    // Install the console protocols.
    //
    status = gBS->InstallMultipleProtocolInterfaces(&ControllerHandle,
                                                    &gEfiSimpleTextInProtocolGuid,
                                                    &consoleDevice->SimpleTextIn,
                                                    &gEfiSimpleTextOutProtocolGuid,
                                                    &consoleDevice->SimpleTextOut,
                                                    NULL);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }


Cleanup:

    if (EFI_ERROR(status))
    {
        if (consoleDevice != NULL)
        {
            if (consoleDevice->SimpleTextIn.WaitForKey != NULL)
            {
                (void)gBS->CloseEvent(consoleDevice->SimpleTextIn.WaitForKey);
            }
            gBS->FreePool(consoleDevice);
        }

        (void)gBS->CloseProtocol(ControllerHandle,
                                 &gMsvmConsoleProtocolGuid,
                                 This->DriverBindingHandle,
                                 ControllerHandle);
    }

    return status;
}


EFI_STATUS
EFIAPI
ConsoleDriverStop(
    _In_  EFI_DRIVER_BINDING_PROTOCOL   *This,
    _In_  EFI_HANDLE                    ControllerHandle,
    _In_  UINTN                         NumberOfChildren,
    _In_  EFI_HANDLE                    *ChildHandleBuffer
    )
/*++

Routine Description:

    Disconnect this driver from a controller and uninstall related protocol instances.

Arguments:

    This                    A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.

    ControllerHandle              The handle of the controller to stop.

    NumberOfChildren        The Number of child devices. Can be zero to indicate
                            stop managing the Controller.

    ChildHandleBuffer       An array of child device handles. Null if NumberOfChildren
                            is zero.

Return Value:

    EFI_SUCCESS             Driver stopped successfully. Error code otherwise.



--*/
{
    EFI_STATUS                      status;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *simpleTextIn;
    CONSOLE_DEVICE                  *consoleDevice;

    //
    // Get our device object context back.
    //
    status = gBS->OpenProtocol(ControllerHandle,
                               &gEfiSimpleTextInProtocolGuid,
                               (VOID **)&simpleTextIn,
                               This->DriverBindingHandle,
                               ControllerHandle,
                               EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(status))
    {
        goto Exit;
    }

    consoleDevice = CONSOLE_DEVICE_FROM_TEXTIN(simpleTextIn);

    //
    // Remove the console protocols.
    //
    status = gBS->UninstallMultipleProtocolInterfaces(ControllerHandle,
                                                   &gEfiSimpleTextInProtocolGuid,
                                                   &consoleDevice->SimpleTextIn,
                                                   &gEfiSimpleTextOutProtocolGuid,
                                                   &consoleDevice->SimpleTextOut,
                                                   NULL);
    if (EFI_ERROR(status))
    {
        //
        // On failure don't continue and close the tag protocol.
        // Thus the handle will still be considered managed by this driver.
        //
        goto Exit;
    }

    //
    // Disconnect from the private MSVM console protocol on the controller handle.
    //
    (void)gBS->CloseProtocol(ControllerHandle,
                             &gMsvmConsoleProtocolGuid,
                             This->DriverBindingHandle,
                             ControllerHandle
                             );
    //
    // Close the WaitForKey event.
    //
    (void)gBS->CloseEvent(consoleDevice->SimpleTextIn.WaitForKey);

    //
    // Free the device object.
    //
    gBS->FreePool(consoleDevice);

Exit:

    return status;
}


EFI_STATUS
EFIAPI
ConsoleInReset(
    _In_ EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
    _In_ BOOLEAN                        ExtendedVerification
    )
{
    UNREFERENCED_PARAMETER(This);
    UNREFERENCED_PARAMETER(ExtendedVerification);
    //
    // Silently succeed.
    //
    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
ConsoleInReadKeyStroke(
    _In_ EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
    _Out_ EFI_INPUT_KEY                 *Key
    )
{
    UNREFERENCED_PARAMETER(This);
    UNREFERENCED_PARAMETER(Key);

    //
    // NOT_READY means no key available.
    //
    return EFI_NOT_READY;
}


EFI_STATUS
ConsoleOutReset(
    _In_ EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    _In_ BOOLEAN                         ExtendedVerification
    )
{
    CONSOLE_DEVICE *consoleDevice = CONSOLE_DEVICE_FROM_TEXTOUT(This);
    UNREFERENCED_PARAMETER(ExtendedVerification);

    //
    // Reset to defaults.
    //
    consoleDevice->SimpleTextOutputMode.Attribute = EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK);
    consoleDevice->SimpleTextOutputMode.CursorColumn = 0;
    consoleDevice->SimpleTextOutputMode.CursorRow = 0;
    consoleDevice->SimpleTextOutputMode.CursorVisible = TRUE;

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
ConsoleOutOutputString(
    _In_ EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    _In_ CHAR16                          *String
    )
{
    UNREFERENCED_PARAMETER(This);
    UNREFERENCED_PARAMETER(String);
    //
    // Silently succeed.
    //
    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
ConsoleOutTestString(
    _In_ EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    _In_ CHAR16                          *String
    )
{
    UNREFERENCED_PARAMETER(This);
    UNREFERENCED_PARAMETER(String);
    //
    // Silently succeed.
    //
    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
ConsoleOutQueryMode(
    _In_ EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    _In_ UINTN ModeNumber,
    _Out_ UINTN *Columns,
    _Out_ UINTN *Rows
    )
{
    CONSOLE_DEVICE *consoleDevice = CONSOLE_DEVICE_FROM_TEXTOUT(This);

    if (ModeNumber >= (UINTN) consoleDevice->SimpleTextOutputMode.MaxMode)
    {
        return EFI_UNSUPPORTED;
    }
    *Columns = gConsoleModes[ModeNumber].Columns;
    *Rows = gConsoleModes[ModeNumber].Rows;

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
ConsoleOutSetMode(
    _In_ EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    _In_ UINTN                           ModeNumber
    )
{
    CONSOLE_DEVICE *consoleDevice = CONSOLE_DEVICE_FROM_TEXTOUT(This);

    if (ModeNumber >= (UINTN) consoleDevice->SimpleTextOutputMode.MaxMode)
    {
        return EFI_UNSUPPORTED;
    }

    consoleDevice->SimpleTextOutputMode.Mode = (UINT32)ModeNumber;

    ConsoleOutReset(This, FALSE);

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
ConsoleOutSetAttribute(
    _In_ EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    _In_ UINTN                           Attribute
    )
{
    CONSOLE_DEVICE *consoleDevice = CONSOLE_DEVICE_FROM_TEXTOUT(This);

    consoleDevice->SimpleTextOutputMode.Attribute = (UINT32)Attribute;

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
ConsoleOutClearScreen(
    _In_ EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
    )
{
    CONSOLE_DEVICE *consoleDevice = CONSOLE_DEVICE_FROM_TEXTOUT(This);

    consoleDevice->SimpleTextOutputMode.CursorColumn = 0;
    consoleDevice->SimpleTextOutputMode.CursorRow = 0;

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
ConsoleOutSetCursorPosition(
    _In_ EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    _In_ UINTN                           Column,
    _In_ UINTN                           Row
    )
{
    CONSOLE_DEVICE *consoleDevice = CONSOLE_DEVICE_FROM_TEXTOUT(This);

    if ((Column >= gConsoleModes[consoleDevice->SimpleTextOutputMode.Mode].Columns) ||
        (Row >= gConsoleModes[consoleDevice->SimpleTextOutputMode.Mode].Rows))
    {
        return EFI_UNSUPPORTED;
    }

    consoleDevice->SimpleTextOutputMode.CursorColumn = (UINT32)Column;
    consoleDevice->SimpleTextOutputMode.CursorRow = (UINT32)Row;

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
ConsoleOutEnableCursor(
    _In_ EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    _In_ BOOLEAN                         Visible
    )
{
    CONSOLE_DEVICE *consoleDevice = CONSOLE_DEVICE_FROM_TEXTOUT(This);

    consoleDevice->SimpleTextOutputMode.CursorVisible = Visible;

    return EFI_SUCCESS;
}

