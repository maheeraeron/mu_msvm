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

#include "BdsPlatform.h"
#include <VirtualDeviceId.h>
#include "PlatformConsole.h"
#include <Library/BootEventLogLib.h>
#include <Guid/EventGroup.h>
#include <Library/GenericBdsLib.h>
#include <Library/DevicePathLib.h>
#include <UefiConstants.h>
#include <VmbusFileSystem.h>

//
// Private declaration of useful internal function in GenericBdsLib.
//
EFI_STATUS BdsDeleteAllInvalidEfiBootOption(VOID);

EFI_STATUS
EFIAPI
XenonBoot(
    VOID
    );

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
    BootDeviceEventFlushLog();
}


static
EFI_STATUS
SetConsoleNvVariable(
    _In_  CHAR16                    *ConVarName,
    _In_  EFI_DEVICE_PATH_PROTOCOL  *DevicePath
    )
/*++

  This function update console variable based on ConVarName, it can
  add or remove one specific console device path from the variable

  @param  ConVarName               Console related variable name, ConIn, ConOut,
                                   ErrOut.
  @param  CustomizedConDevicePath  The console device path which will be added to
                                   the console variable ConVarName, this parameter
                                   can not be multi-instance.
  @param  ExclusiveDevicePath      The console device path which will be removed
                                   from the console variable ConVarName, this
                                   parameter can not be multi-instance.

  @retval EFI_UNSUPPORTED          The added device path is same to the removed one.
  @retval EFI_SUCCESS              Success add or remove the device path from  the
                                   console variable.

--*/
{
    EFI_STATUS                status;
    UINT32                    attributes;
    UINTN                     devicePathSize;


    attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS |
                 EFI_VARIABLE_NON_VOLATILE;

    devicePathSize = GetDevicePathSize(DevicePath);
    status = gRT->SetVariable(ConVarName,
                              &gEfiGlobalVariableGuid,
                              attributes,
                              devicePathSize,
                              DevicePath);
    return status;
}


static
EFI_STATUS
ConnectConsoleDevice(
    _In_        EFI_DEVICE_PATH_PROTOCOL    *DevicePath,
    _In_        BOOLEAN                     IsOutput,
    _In_        BOOLEAN                     IsInput,
    _In_opt_    GUID                        *ChildProtocol,
    _Out_opt_   EFI_DEVICE_PATH_PROTOCOL    **AddedDevicePath
   )
/*++

Routine Description:

    Connects the device specified by the given device path. Adds the device path
    or the device's child device path to the variables that are the possible
    console devices (ConInDev, ConOutDev, ErrOutDev).

Arguments:

    DevicePath              - device path of the device to connect and add to
                              the console devices variables

    IsOutput                - True if an console output device.

    IsInput                 - True if an console input device.

    ChildProtocol           - Guid of a protocol to look for after the device is connected.
                              If NULL no search is performed and DevicePath is added to the
                              appropriate console device list variables. If not NULL and a
                              matching child handle is found then the child device path
                              is added to the console device list variables.

    AddedDevicePath         - Returns the device path that was added to the console device
                              list variables.

Returns:

      EFI_SUCCESS           - Device was connected and the variable updated.
      EFI_STATUS            - An error occurred.

--*/
{
    UINTN                       index;
    EFI_STATUS                  status;
    EFI_HANDLE                  deviceHandle;
    EFI_DEVICE_PATH_PROTOCOL    *tempDevicePath;
    EFI_DEVICE_PATH_PROTOCOL    *childDevicePath;
    UINTN                       handleCount;
    EFI_HANDLE                  *handleBuffer;

    if (DevicePath == NULL)
    {
        return EFI_INVALID_PARAMETER;
    }


    tempDevicePath = DevicePath;
    status = gBS->LocateDevicePath(&gEfiDevicePathProtocolGuid,
                                   &tempDevicePath,
                                   &deviceHandle);
    if (EFI_ERROR(status))
    {
        return status;
    }

    //
    // Connect this handle this will cause the driver to start and
    // register any protocols.
    //
    status = gBS->ConnectController(deviceHandle, NULL, NULL, FALSE);
    if (EFI_ERROR(status))
    {
        return status;
    }

    //
    // If no ChildProtocol is specified then update the console variables
    // with the input device path and output it as the registered path.
    //
    if (ChildProtocol == NULL)
    {
        if (IsOutput)
        {
            BdsLibUpdateConsoleVariable(VarConsoleOutDev, DevicePath, NULL);
            BdsLibUpdateConsoleVariable(VarErrorOutDev, DevicePath, NULL);
        }
        if (IsInput)
        {
            BdsLibUpdateConsoleVariable(VarConsoleInpDev, DevicePath, NULL);
        }
        if (AddedDevicePath != NULL)
        {
            *AddedDevicePath = DevicePath;
        }

        return EFI_SUCCESS;
    }

    //
    // Get the handles for the requested protocol.
    //
    status = gBS->LocateHandleBuffer(ByProtocol,
                                     ChildProtocol,
                                     NULL,
                                     &handleCount,
                                     &handleBuffer
                                     );
    if (EFI_ERROR(status))
    {
        return status;
    }

    //
    // Loop through the handles matching the requested type.
    //
    for (index = 0; index < handleCount; index++)
    {
        //
        // Get the device path on the handle.
        //
        status = gBS->HandleProtocol(handleBuffer[index],
                                     &gEfiDevicePathProtocolGuid,
                                     (VOID*)&childDevicePath);
        if (EFI_ERROR(status))
        {
            continue;
        }

        //
        // If the base device path is a prefix of the child device path
        // then update the console variables with the child device path
        // and output the child device path.
        //
        if (CompareMem(DevicePath,
                       childDevicePath,
                       GetDevicePathSize(DevicePath) - END_DEVICE_PATH_LENGTH) == 0)
        {
            if (IsOutput)
            {
                BdsLibUpdateConsoleVariable(VarConsoleOutDev, childDevicePath, NULL);
                BdsLibUpdateConsoleVariable(VarErrorOutDev, childDevicePath, NULL);
            }
            else
            {
                BdsLibUpdateConsoleVariable(VarConsoleInpDev, childDevicePath, NULL);
            }
            if (AddedDevicePath != NULL)
            {
                *AddedDevicePath = childDevicePath;
            }
        }
    }

    gBS->FreePool(handleBuffer);

    return EFI_SUCCESS;
}


static
BOOLEAN
VmbusScanForDevices(
    _In_ UINT32             ConsoleMode
    )
/*++

Routine Description:

    Scans for existing VMBus devices, connects them, and adds
    them to the console device list variables.
    Sets the console variables if the console mode is "default".

Arguments:

    ConsoleMode - the current configured console mode

Returns:

    TRUE - if a VMBus console was found and the console variables
           were set to the device paths.

    FALSE - otherwise.

--*/
{
    EFI_STATUS                status;
    UINTN                     handleCount;
    EFI_HANDLE                *handleBuffer;
    UINTN                     index;
    VOID                      *instance;
    EFI_DEVICE_PATH_PROTOCOL  *devicePath;
    EFI_DEVICE_PATH_PROTOCOL  *registeredDevicePath;
    BOOLEAN                   found = FALSE;

    //
    // Find all possible VMBus device by looking for the protocol.
    //
    handleCount = 0;
    handleBuffer = NULL;
    status = gBS->LocateHandleBuffer(ByProtocol,
                                     &gEfiVmbusProtocolGuid,
                                     NULL,
                                     &handleCount,
                                     &handleBuffer);
    if (EFI_ERROR(status))
    {
        return found;
    }

    //
    // Loop through the devices and look for specific devices.
    //
    for (index = 0; index < handleCount; index++)
    {
        BOOLEAN isInput = FALSE;
        BOOLEAN isOutput = FALSE;
        status = gBS->HandleProtocol(handleBuffer[index], &gEfiVmbusProtocolGuid, &instance);
        if (EFI_ERROR(status))
        {
            continue;
        }

        //
        // Check for output (video) devices and input (keyboard) device.
        //
        if (EmclChannelTypeSupported(handleBuffer[index],
                                     &SYNTHVID_CLASS_ID,            // synth video
                                     gImageHandle) == EFI_SUCCESS)
        {
            isOutput = TRUE;
        }
        else if (EmclChannelTypeSupported(handleBuffer[index],
                                          &SYNTH3DVID_DEVICE_ID,    // RemoteFX synth video
                                          gImageHandle) == EFI_SUCCESS)
        {
            isOutput = TRUE;
        }
        else if (EmclChannelTypeSupported(handleBuffer[index],
                                          &HK_INTERFACE_GUID,       // synth keyboard
                                          gImageHandle) == EFI_SUCCESS)
        {
            isInput = TRUE;
        }

        if (isInput || isOutput)
        {
            //
            // Get the device path of the device.
            //
            devicePath = NULL;
            status = gBS->HandleProtocol(handleBuffer[index],
                                         &gEfiDevicePathProtocolGuid,
                                         (VOID*)&devicePath);
            if (EFI_ERROR(status))
            {
                continue;
            }

            //
            // Connect device and update console variables.
            //
            registeredDevicePath = NULL;
            status = ConnectConsoleDevice(devicePath,
                                          isOutput,
                                          isInput,
                                          NULL,
                                          &registeredDevicePath);
            if (!EFI_ERROR(status))
            {
                if (ConsoleMode == ConfigLibConsoleModeDefault)
                {
                    if (isOutput)
                    {
                        SetConsoleNvVariable(VarConsoleOut, registeredDevicePath);
                        SetConsoleNvVariable(VarErrorOut, registeredDevicePath);
                        found = TRUE;
                    }
                    else
                    {
                        SetConsoleNvVariable(VarConsoleInp, registeredDevicePath);
                        found = TRUE;
                    }
                }
            }
        }

    }

    gBS->FreePool(handleBuffer);

    return found;
}


static
EFI_STATUS
GetSerialPortUid(
    _In_ EFI_DEVICE_PATH_PROTOCOL *DevicePath,
    _Out_ UINT32                  *Uid
    )
/*++

Routine Description:

    Check if a device path is a serial port and outputs its UID.
    The UID is 1 if COM1 and 2 if COM2.

Arguments:

    DevicePath              A pointer to a device path.

    Uid                     Updated with the UID of the device if present.

Return Value:

    EFI_SUCCESS             if a serial port and UID is output.
    EFI_NOT_FOUND           if not a serial port.

--*/
{
    EFI_DEV_PATH *node;

    while (!IsDevicePathEnd(DevicePath))
    {
        node = (EFI_DEV_PATH *)DevicePath;

        if (node->DevPath.Type == ACPI_DEVICE_PATH &&
            node->DevPath.SubType == ACPI_DP &&
            node->Acpi.HID == EISA_PNP_ID(0x501))
        {
            *Uid = node->Acpi.UID;
            return EFI_SUCCESS;
        }
        DevicePath = NextDevicePathNode(DevicePath);
    }
    return EFI_NOT_FOUND;
}


static
BOOLEAN
SerialScanForDevices(
    _In_ UINT32             ConsoleMode
    )
/*++

Routine Description:

    Scans for existing serial devices, connects them, and adds
    them to the console device list variables.
    Sets the console variables if the console mode is a serial port.

Arguments:

    ConsoleMode - the current configured console mode


Returns:

    TRUE - if a serial device was found and the console variables
           were set to its device paths.

    FALSE - otherwise

--*/
{
    EFI_STATUS                status;
    UINTN                     handleCount;
    EFI_HANDLE                *handleBuffer;
    UINTN                     index;
    EFI_DEVICE_PATH_PROTOCOL  *devicePath;
    EFI_DEVICE_PATH_PROTOCOL  *registeredDevicePath;
    UINT32                    uid;
    BOOLEAN                   found = FALSE;


    //
    // Find all possible serial devices by looking for the protocol.
    //
    handleCount = 0;
    handleBuffer = NULL;
    status = gBS->LocateHandleBuffer(ByProtocol,
                                     &gEfiSerialIoProtocolGuid,
                                     NULL,
                                     &handleCount,
                                     &handleBuffer);
    if (EFI_ERROR(status))
    {
        return found;
    }

    //
    // Loop through the devices and look for specific devices.
    //
    for (index = 0; index < handleCount; index++)
    {
        //
        // Get the device path of the device.
        //
        devicePath = NULL;
        status = gBS->HandleProtocol(handleBuffer[index],
                                     &gEfiDevicePathProtocolGuid,
                                     (VOID*)&devicePath);
        if (EFI_ERROR(status))
        {
            continue;
        }

        //
        // Connect device and update UEFI console variables.
        //
        registeredDevicePath = NULL;
        status = ConnectConsoleDevice(devicePath,
                                     TRUE,
                                     TRUE,
                                     &gEfiSimpleTextOutProtocolGuid,
                                     &registeredDevicePath);
        if (EFI_ERROR(status))
        {
            continue;
        }

        //
        // What COM port is this?
        //
        status = GetSerialPortUid(devicePath, &uid);
        if (EFI_ERROR(status))
        {
            continue;
        }

        //
        // If UID matches ConsoleMode then set it as the console devices.
        //
        if (ConsoleMode == uid)
        {
            SetConsoleNvVariable(VarConsoleOut, registeredDevicePath);
            SetConsoleNvVariable(VarErrorOut, registeredDevicePath);
            SetConsoleNvVariable(VarConsoleInp, registeredDevicePath);
            found = TRUE;
        }

    }

    gBS->FreePool(handleBuffer);

    return found;
}


static
BOOLEAN
NullScanForDevices(
    void
    )
/*++

Routine Description:

    Scans for an existing null console device, connects it, and adds
    it to the console device list variables.
    Always sets the console variables if a device is found.

Arguments:

    VariablesSet - Pointer to boolean that is set to TRUE if a device was found
                   and the console variables were set.

Returns:

    n/a

--*/
{
    EFI_STATUS                status;
    UINTN                     handleCount;
    EFI_HANDLE                *handleBuffer;
    EFI_DEVICE_PATH_PROTOCOL  *devicePath;
    EFI_DEVICE_PATH_PROTOCOL  *registeredDevicePath;
    BOOLEAN                   found = FALSE;

    //
    // Find the console device by looking for the private tag protocol.
    //
    handleCount = 0;
    handleBuffer = NULL;
    status = gBS->LocateHandleBuffer(ByProtocol,
                                     &gMsvmConsoleProtocolGuid,
                                     NULL,
                                     &handleCount,
                                     &handleBuffer);
    if (EFI_ERROR(status))
    {
        return found;
    }

    //
    // There should be only one.
    //
    if (handleCount > 0)
    {
        //
        // Get the device path of the device.
        //
        devicePath = NULL;
        status = gBS->HandleProtocol(handleBuffer[0],
                                     &gEfiDevicePathProtocolGuid,
                                     (VOID*)&devicePath);
        if (EFI_ERROR(status))
        {
            return found;
        }

        //
        // Connect device and update UEFI console variables.
        //
        registeredDevicePath = NULL;
        status = ConnectConsoleDevice(devicePath,
                                      TRUE,
                                      TRUE,
                                      &gEfiSimpleTextOutProtocolGuid,
                                      &registeredDevicePath);
        if (EFI_ERROR(status))
        {
            return found;
        }

        //
        // Set it as the console device.
        //
        SetConsoleNvVariable(VarConsoleOut, registeredDevicePath);
        SetConsoleNvVariable(VarErrorOut, registeredDevicePath);
        SetConsoleNvVariable(VarConsoleInp, registeredDevicePath);
        found = TRUE;

    }

    gBS->FreePool(handleBuffer);

    return found;
}


static
EFI_STATUS
SetConsoleMode()
/*++

Routine Description:

    Sets the console mode based on the PCD default values.
    If the defaults cannot be set  Mode 0 (80x25) will be used.

Arguments:

    None

Returns:

    EFI_SUCCESS - Mode was set successfully.
    EFI_STATUS - failure to set mode.
--*/
{
      UINT32 Mode = PcdGet16(PcdPlatformBootConsoleMode);

      gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_BLACK));
      return gST->ConOut->SetMode(gST->ConOut, Mode);
}


static
EFI_STATUS
ConnectConsole(
    VOID
    )
/*++

Routine Description:

    Connect the console devices.

Arguments:

    PlatformConsole         - Predfined platform default console device array.

Returns:

    EFI_SUCCESS
    EFI_STATUS
--*/
{
    EFI_STATUS  status;
    UINT32      consoleMode;
    BOOLEAN     vmbusConsoleFound = FALSE;
    BOOLEAN     serialConsoleFound = FALSE;
    BOOLEAN     nullConsoleFound = FALSE;

    //
    // Get the configured console mode.
    //
    consoleMode = PcdGet8(PcdConsoleMode);

    //
    // Detect VMBUS based devices.
    //
    vmbusConsoleFound = VmbusScanForDevices(consoleMode);

    //
    // Detect Serial devices.
    //
    serialConsoleFound = SerialScanForDevices(consoleMode);

    if (!vmbusConsoleFound && !serialConsoleFound)
    {
        nullConsoleFound = NullScanForDevices();
    }

    //
    // Connect the all default consoles with current console variables
    //
    status = BdsLibConnectAllDefaultConsoles();
    if (EFI_ERROR(status))
    {
        return status;
    }

    return EFI_SUCCESS;
}


static
VOID
ConnectSequence(
    VOID
    )
/*++

Routine Description:

    Connect with predeined platform connect sequence,
    the OEM/IBV can customize with their own connect sequence.

Arguments:

    None.

Returns:

    None.

--*/
{
    DEBUG((EFI_D_INFO, "ConnectSequence\n"));

    BdsLibConnectAll();

    //
    // This is required as replica scenarios can result in
    // the boot order variables being inconsistent.
    //
    BdsDeleteAllInvalidEfiBootOption();

    return;
}


VOID
Diagnostics(
 _In_ EXTENDMEM_COVERAGE_LEVEL    MemoryTestLevel,
 _In_ BOOLEAN                     QuietBoot,
 _In_ BASEM_MEMORY_TEST           BaseMemoryTest
  )
/*++

Routine Description:

  Perform the platform diagnostic, such like test memory. OEM/IBV also
  can customize this fuction to support specific platform diagnostic.

Arguments:

  MemoryTestLevel  - The memory test intensive level

  QuietBoot        - Indicate if need to enable the quiet boot

  BaseMemoryTest   - A pointer to BaseMemoryTest()

Returns:

  None.

--*/
{
  DEBUG((EFI_D_INFO, "PlatformBdsDiagnostics\n"));

  if (QuietBoot) {

    //
    // Set console to default mode.
    //
    SetConsoleMode();

    //
    // Display the logo.
    //
    EnableQuietBoot(PcdGetPtr(PcdLogoFile));

    //
    // Skip the memory test as it is unnecessary in a VM
    //
  }

  return;
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

    DEBUG((EFI_D_INFO, "PlatformBdsInit\n"));

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


VOID
EFIAPI
PlatformBdsPolicyBehavior(
    _In_ OUT LIST_ENTRY             *DriverOptionList,
    _In_ OUT LIST_ENTRY             *BootOptionList,
    _In_ PROCESS_CAPSULES           ProcessCapsules,
    _In_ BASEM_MEMORY_TEST          BaseMemoryTest
    )
/*++

Routine Description:

    The function will excute with the platform policy, current policy
    is driven by boot mode. IBV/OEM can customize this code for their specific
    policy action.

Arguments:

    DriverOptionList - The header of the driver option link list

    BootOptionList   - The header of the boot option link list

    ProcessCapsules  - A pointer to ProcessCapsules()

    BaseMemoryTest   - A pointer to BaseMemoryTest()

Returns:

    n/a

--*/
{
    DEBUG((EFI_D_INFO, "PlatformBdsPolicyBehavior\n"));

    //
    // Connect the console devices.
    //
    ConnectConsole();

    //
    // Initialize the console after the ConIn/ConOut are connected.
    //
    PlatformConsoleInitialize();

    //
    // Memory test and Logo show
    //
    Diagnostics(IGNORE, TRUE, BaseMemoryTest);

    //
    // Perform platform specific connect sequence
    //
    ConnectSequence();

    return ;
}


VOID
EFIAPI
PlatformBdsBootFail(
    _In_  BDS_COMMON_OPTION  *Option,
    _In_  EFI_STATUS         Status,
    _In_  CHAR16             *ExitData,
    _In_  UINTN              ExitDataSize,
    _In_  BOOLEAN            LastBootOption
    )
/*++

Routine Description:

    Hook point after a boot attempt fails.

Arguments:

    Option - Pointer to Boot Option that failed to boot.

    Status - Status returned from failed boot.

    ExitData - Exit data returned from failed boot.

    ExitDataSize - Exit data size returned from failed boot.

    LastBootOption - Whether this is the last boot option.

Returns:

    None.

Status Codes:
    These are present in the Status parameter and indicate the reason for boot
    failure.

    EFI_NOT_FOUND         No Boot Image Found
    EFI_ACCESS_DENIED     Secure Boot Failed.

--*/
{
    DEBUG((EFI_D_INFO, "PlatformBdsBootFail\n"));

    if (PcdGetBool(PcdPauseAfterBootFailure) && !LastBootOption)
    {
        PlatformConsoleShow();
        PlatformConsoleBootSummary(STRING_TOKEN(STR_BOOT_NEXT_ENTRY));
        PlatformConsoleHide();
    }

    gBS->FreePool(Option->StatusString);
    Option->StatusString = NULL;
}


VOID
EFIAPI
PlatformBdsBootSuccess(
    _In_ BDS_COMMON_OPTION *Option
    )
/*++

Routine Description:

    Hook point after a boot attempt succeeds. We don't expect a boot option to
    return, so the EFI 1.0 specification defines that you will default to an
    interactive mode and stop processing the BootOrder list in this case. This
    is also a platform implementation and can be customized by IBV/OEM.

Arguments:

  Option - Pointer to Boot Option that succeeded to boot.

Returns:

  None.

--*/
{
    DEBUG((EFI_D_INFO, "PlatformBdsBootSuccess\n"));

    //
    // While not documented anywhere, the current Bds code implicitly expects that
    // the status string passed in is freed (i.e. callers of PlatformBdsBootSuccess
    // and PlatformBdsBootFail allocate the string from an HII resource and never free it.)
    //
    gBS->FreePool(Option->StatusString);
    Option->StatusString = NULL;
}


VOID
EFIAPI
PlatformBdsLockNonUpdatableFlash(
    VOID
    )
{
    DEBUG((EFI_D_INFO, "PlatformBdsLockNonUpdatableFlash\n"));
    return;
}


/**
  This function is the main entry of the platform setup entry.
  The function will present the main menu of the system setup,
  this is the platform reference part and can be customize.


  @param TimeoutDefault     The fault time out value before the system
                            continue to boot.
  @param ConnectAllHappened The indicater to check if the connect all have
                            already happened.

**/
VOID
PlatformBdsEnterFrontPage(
     _In_ UINT16                    TimeoutDefault,
     _In_ BOOLEAN                   ConnectAllHappened
    )
{
    EVENT_CHANNEL_STATISTICS stats;

    //
    // If we get here boot failed for one of the following reasons...
    //  - No available boot devices
    //  - All boot devices failed (no Loaders)
    //  - All boot loaders returned (didn't actually load an OS,
    //    this can happen for PXE boot when no key is pressed).
    //

    //
    // Disable the boot watchdog
    // FUTURE: 03-27-2013 kharp  A periodic timer could be used at this point
    // instead of completely disabling the watchdog.
    //
    gBS->SetWatchdogTimer(0, 0, 0, NULL);

    ZeroMem(&stats, sizeof(stats));
    BootDeviceEventStatistics(&stats);

    if (stats.Written == 0)
    {
      //
      // Log a specific event for no boot devices.
      //
      BootDeviceEventStart(NULL, (UINT16)-1, BootDeviceNoDevices, EFI_NOT_FOUND);
      BootDeviceEventComplete();
    }

    BootDeviceEventFlushLog();
    PlatformConsoleShow();
    PlatformConsoleBootSummary(STRING_TOKEN(STR_BOOT_RETRY));
    PlatformConsoleHide();

    //
    // Clear the event log before trying the boot list again.
    //
    BootDeviceEventResetLog();

    //
    // Re-enable watchdog.
    //
    gBS->SetWatchdogTimer(5 * 60, 0, 0, NULL);
}


// ============================================================================
// End Exported Platform BDS Library Functions
// ============================================================================

