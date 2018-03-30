/** @file
  This file include all platform action which can be customized by IBV/OEM.

@copyright
 Copyright (c) 2004 - 2014 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
**/

// MSchange - TPM2 is assumed to be present and enabled on our platforms so
//            when merging assume TPM2_FLAG == 1

#include "BdsPlatform.h"

static EFI_BOOT_MODE                  mBootMode;
static EFI_DEVICE_PATH_PROTOCOL     **mPlatformConnectSequence;

extern USB_CLASS_FORMAT_DEVICE_PATH   gUsbClassKeyboardDevicePath;



/**
  The handle on the path we get might be not the display device.
  We must check it.

  @todo fix the parameters

  @retval  TRUE         PCI class type is VGA.
  @retval  FALSE        PCI class type isn't VGA.
**/
BOOLEAN
IsVgaHandle (
  IN EFI_HANDLE Handle
)
{
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00 Pci;
  EFI_STATUS Status;

  Status = gBS->HandleProtocol (
                  Handle,
                  &gEfiPciIoProtocolGuid,
                  (VOID **)&PciIo);

  if (!EFI_ERROR (Status)) {
    Status = PciIo->Pci.Read (
                          PciIo,
                          EfiPciIoWidthUint32,
                          0,
                          sizeof (Pci) / sizeof (UINT32),
                          &Pci);

    if (!EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "  PCI CLASS CODE    = 0x%x\n", Pci.Hdr.ClassCode [2]));
      DEBUG ((DEBUG_INFO, "  PCI SUBCLASS CODE = 0x%x\n", Pci.Hdr.ClassCode [1]));

      if (IS_PCI_VGA (&Pci) || IS_PCI_OLD_VGA (&Pci)) {
        DEBUG ((DEBUG_INFO, "  \nPCI VGA Device Found\n"));
        return TRUE;
      }
    }
  }
  return FALSE;
}

VOID
ExitPmAuth (
  VOID
  )
{
  EFI_HANDLE                  Handle;
  EFI_STATUS                  Status;


  PERF_FUNCTION_BEGIN (PERF_VERBOSITY_STANDARD); // MS_CHANGE

  DEBUG((DEBUG_INFO,"ExitPmAuth ()- Start\n"));

  //
  // Since PI1.2.1, we need signal EndOfDxe as ExitPmAuth
  //
  EfiEventGroupSignal (&gEfiEndOfDxeEventGroupGuid);

  DEBUG((DEBUG_INFO,"All EndOfDxe callbacks have returned successfully\n"));

  //
  // NOTE: We need install DxeSmmReadyToLock directly here because many boot script is added via ExitPmAuth/EndOfDxe callback.
  // If we install them at same callback, these boot script will be rejected because BootScript Driver runs first to lock them done.
  // So we seperate them to be 2 different events, ExitPmAuth is last chance to let platform add boot script. DxeSmmReadyToLock will
  // make boot script save driver lock down the interface.
  //
  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gEfiDxeSmmReadyToLockProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  DEBUG((DEBUG_INFO,"ExitPmAuth ()- End\n"));

  //
  // Dispatch deferred images after EndOfDxe event and ReadyToLock installation.
  //
  EfiBootManagerDispatchDeferredImages ();

  PERF_FUNCTION_END (PERF_VERBOSITY_STANDARD); // MS_CHANGE
}

VOID
ConnectRootBridge (
  BOOLEAN Recursive
  )
{
  UINTN                            RootBridgeHandleCount;
  EFI_HANDLE                       *RootBridgeHandleBuffer;
  UINTN                            RootBridgeIndex;

  PERF_FUNCTION_BEGIN (PERF_VERBOSITY_STANDARD); // MS_CHANGE

  RootBridgeHandleCount = 0;
  gBS->LocateHandleBuffer (
         ByProtocol,
         &gEfiPciRootBridgeIoProtocolGuid,
         NULL,
         &RootBridgeHandleCount,
         &RootBridgeHandleBuffer
         );
  for (RootBridgeIndex = 0; RootBridgeIndex < RootBridgeHandleCount; RootBridgeIndex++) {
    gBS->ConnectController (RootBridgeHandleBuffer[RootBridgeIndex], NULL, NULL, Recursive);
  }

  PERF_FUNCTION_END (PERF_VERBOSITY_STANDARD); // MS_CHANGE
}


BOOLEAN
IsGopDevicePath (
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
{
  while (!IsDevicePathEndType (DevicePath)) {
    if (DevicePathType (DevicePath) == ACPI_DEVICE_PATH &&
        DevicePathSubType (DevicePath) == ACPI_ADR_DP) {
      return TRUE;
    }
    DevicePath = NextDevicePathNode (DevicePath);
  }
  return FALSE;
}

//
// BDS Platform Functions
//

/**
  Remove all GOP device path instance from DevicePath and add the Gop to the DevicePath.
**/
EFI_DEVICE_PATH_PROTOCOL *
UpdateDevicePath (
  EFI_DEVICE_PATH_PROTOCOL *DevicePath,
  EFI_DEVICE_PATH_PROTOCOL *Gop
  )
{
  UINTN                    Size;
  UINTN                    GopSize;
  EFI_DEVICE_PATH_PROTOCOL *Temp;
  EFI_DEVICE_PATH_PROTOCOL *Return;
  EFI_DEVICE_PATH_PROTOCOL *Instance;
  BOOLEAN                  Exist;

  Exist = FALSE;
  Return = NULL;
  GopSize = GetDevicePathSize (Gop);
  do {
    Instance = GetNextDevicePathInstance (&DevicePath, &Size);
    if (Instance == NULL) {
      break;
    }
    if (!IsGopDevicePath (Instance) ||
        (Size == GopSize && CompareMem (Instance, Gop, GopSize) == 0)
       ) {
     if (Size == GopSize && CompareMem (Instance, Gop, GopSize) == 0) {
       Exist = TRUE;
     }
     Temp = Return;
     Return = AppendDevicePathInstance (Return, Instance);
     if (Temp != NULL) {
       FreePool (Temp);
     }
    }
    FreePool (Instance);
  } while (DevicePath != NULL);

  if (!Exist) {
    Temp = Return;
    Return = AppendDevicePathInstance (Return, Gop);
    gBS->FreePool (Temp);
  }
  return Return;
}

/**
  Determines if a given device path is a child of another device path.  This
  function is used to determine if a given controller is downstream from
  a root port.

  @param Parent  The device path for the parent controller
  @param Child   The device path for the potential child controller

  @retval TRUE   The potential child device path is actually a child
  @retval FALSE  The potential child device path is NOT a child
**/
BOOLEAN
IsDevicePathAChild (
  IN EFI_DEVICE_PATH_PROTOCOL  *Parent,
  IN EFI_DEVICE_PATH_PROTOCOL  *Child
  )
{
  EFI_DEVICE_PATH_PROTOCOL  *ParentNode;
  EFI_DEVICE_PATH_PROTOCOL  *ChildNode;
  BOOLEAN                   IsAChild;

  ParentNode = Parent;
  ChildNode  = Child;
  IsAChild   = TRUE;

  //
  // While iterating over each node of the parent and child devicepaths,
  // the child relationship is FALSE if any of the following conditions are true:
  //   1. The child devicepath is terminated before the parent devicepath
  //   2. The child's devicepath node is a different length than the parent's
  //   3. The contents of the child's devicepath node are different than the parent's
  //
  while (!IsDevicePathEndType (ParentNode)) {
    if (IsDevicePathEndType (ChildNode)) {
      IsAChild = FALSE;
      break;
    }

    if (DevicePathNodeLength (ParentNode) != DevicePathNodeLength (ChildNode)) {
      IsAChild = FALSE;
      break;
    }

    if (CompareMem (ParentNode, ChildNode, DevicePathNodeLength (ParentNode)) != 0) {
      IsAChild = FALSE;
    }

    ParentNode = NextDevicePathNode (ParentNode);
    ChildNode  = NextDevicePathNode (ChildNode);
  }

  return IsAChild;
}

/**
  Finds the first VGA controller that is downstream from a given list of
  PCIe root ports.

  @param PciControllerPaths         An array of device path pointers.
                                    Each device path represents a PCIe root port
  @param PciControllerPathsLength   The length of the array

  @retval The handle to the first VGA controller that was found, or NULL if
          there is no VGA controllers downstream from the given root port.
**/
EFI_HANDLE
GetFirstDownstreamVideoController (
  IN EFI_DEVICE_PATH_PROTOCOL  **PciControllerPaths,
  IN UINTN                     PciControllerPathsLength
  )
{
  EFI_STATUS                          Status;
  UINTN                               Index;
  UINTN                               HandleIndex;
  EFI_HANDLE                          *HandleBuffer;
  UINTN                               HandleCount;
  PCI_TYPE00                          Pci;
  EFI_PCI_IO_PROTOCOL                 *PciIo;
  EFI_HANDLE                          VideoController;

  VideoController = NULL;
  //
  // Get all handles that which contain an instance of PCI_IO_PROTOCOL
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  //
  // For all PCI Devices
  //
  for (HandleIndex = 0; (HandleIndex < HandleCount) && (VideoController == NULL); HandleIndex++) {
    Status = gBS->HandleProtocol (HandleBuffer[HandleIndex], &gEfiPciIoProtocolGuid, &PciIo);
    if (!EFI_ERROR (Status)) {
      //
      // Check if this device is a VGA Controller
      //
      Status = PciIo->Pci.Read (
                      PciIo,
                      EfiPciIoWidthUint32,
                      0,
                      sizeof (Pci) / sizeof (UINT32),
                      &Pci);
      ASSERT_EFI_ERROR (Status);
      if (IS_PCI_VGA (&Pci) || IS_PCI_OLD_VGA (&Pci)) {
        //
        // Found a VGA Controller, now check if this device
        // is a child of one of the root controllers
        //
        for (Index = 0; Index < PciControllerPathsLength; Index++) {
          if (IsDevicePathAChild (PciControllerPaths[Index],
                                  DevicePathFromHandle (HandleBuffer[HandleIndex]))) {
            VideoController = HandleBuffer[HandleIndex];
            break;
          }
        }
      }
    }
  }

  return VideoController;
}

//
// There is no way to dynamically detect which root ports are the PEG/PCH ports
//
#define SA_PEG_MAX_FUN     0x03
#define PCH_PCIE_MAX_FUN   0x08

/**
  Finds the first VGA controller that is downstream from the PEG root ports.

  @retval The handle to the first VGA controller that was found, or NULL if
          there is no VGA controllers downstream from the given root port.
**/
EFI_HANDLE
GetPegVideoController (
  VOID
  )
{
  EFI_DEVICE_PATH_PROTOCOL *PegControllerPaths[SA_PEG_MAX_FUN];
  EFI_HANDLE               VideoController;
  UINT8                    Index;

  //
  // Create device paths for all the PEG root ports
  //
  for (Index = 0; Index < SA_PEG_MAX_FUN; Index++) {
    PegControllerPaths[Index] = DuplicateDevicePath (
      (EFI_DEVICE_PATH_PROTOCOL*) &(gPlatformPegRootController.PciRootBridge.Header));
    ((PLATFORM_PEG_ROOT_CONTROLLER_DEVICE_PATH*)PegControllerPaths[Index])->Pci0Device.Function = Index;
  }

  VideoController = GetFirstDownstreamVideoController (&(PegControllerPaths[0]), SA_PEG_MAX_FUN);

  //
  // Free device path structures from the heap
  //
  for (Index = 0; Index < SA_PEG_MAX_FUN; Index++) {
    FreePool (PegControllerPaths[Index]);
  }

  return VideoController;
}

EFI_HANDLE
GetPchVideoController (
  VOID
  )
{
  EFI_DEVICE_PATH_PROTOCOL *PchPcieControllerPaths[PCH_PCIE_MAX_FUN];
  EFI_HANDLE               VideoController;
  UINT8                    Index;

  //
  // Create device paths for all the PEG root ports
  //
  for (Index = 0; Index < PCH_PCIE_MAX_FUN; Index++) {
    PchPcieControllerPaths[Index] = DuplicateDevicePath (
      (EFI_DEVICE_PATH_PROTOCOL*) &(gPlatformPchPcieRootController.PciRootBridge.Header));
    ((PLATFORM_ONBOARD_CONTROLLER_DEVICE_PATH*)PchPcieControllerPaths[Index])->PciDevice.Function = Index;
  }

  VideoController = GetFirstDownstreamVideoController (&(PchPcieControllerPaths[0]), PCH_PCIE_MAX_FUN);

  //
  // Free device path structures from the heap
  //
  for (Index = 0; Index < PCH_PCIE_MAX_FUN; Index++) {
    FreePool (PchPcieControllerPaths[Index]);
  }

  return VideoController;
}

/**
  Platform Bds init. Incude the platform firmware vendor, revision
  and so crc check.
**/
VOID
EFIAPI
PlatformBootManagerBeforeConsole (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *VarConOut;
  EFI_DEVICE_PATH_PROTOCOL  *VarConIn;
  EFI_DEVICE_PATH_PROTOCOL            *TempDevicePath;
  EFI_DEVICE_PATH_PROTOCOL            *ConsoleOut;
  EFI_DEVICE_PATH_PROTOCOL            *Temp;
  EFI_DEVICE_PATH_PROTOCOL            *Instance;
  EFI_DEVICE_PATH_PROTOCOL            *Next;
  EFI_HANDLE                          Handle;
  UINT16                              PrimaryDisplay = OEM_DISPLAY_AUTO;     // Default: automatically detect primary display type.
  UINTN                               InstanceSize;
  BDS_CONSOLE_CONNECT_ENTRY          *PlatformConsoles;

  TempDevicePath = NULL;
  PrimaryDisplay = DeviceBootManagerBeforeConsole (&TempDevicePath, &PlatformConsoles);

  mBootMode = GetBootModeHob();  // BeforeConsole has to be called before AfterConsole.

  //
  // Append Usb Keyboard short form DevicePath into "ConInDev"
  //
  EfiBootManagerUpdateConsoleVariable (
    ConInDev,
    (EFI_DEVICE_PATH_PROTOCOL *) &gUsbClassKeyboardDevicePath,
    NULL
    );

  //
  // Connect Root Bridge to make PCI BAR resource allocated and all PciIo created
  //
  ConnectRootBridge (FALSE);

  //
  // Update ConOut variable accordign to the PrimaryDisplay setting
  //
  ConsoleOut = NULL;
  GetEfiGlobalVariable2 (L"ConOut",&ConsoleOut,NULL);

  //
  // Determine preferred primary display based on Surface device board ID.
  //

  switch (PrimaryDisplay) {
    case OEM_DISPLAY_AUTO: // AUTO
    case OEM_DISPLAY_PEG: // PEG
      //
      // Add PEG to ConOut if it exists
      //
      Handle = GetPegVideoController ();
      if (Handle != NULL) {
        break;
      }

      //
      // Falls to use PCH PCIe if PEG doesn't exist
      //
    case OEM_DISPLAY_PCI: // PCI
      //
      // Add PCI to ConOut if it exists
      //
      Handle = GetPchVideoController ();
      if (Handle != NULL) {
        break;
      }

      //
      // Falls to case 0 IGD case if PCH PCIe doesn't exist
      //

    case OEM_DISPLAY_IGD: // IGD
    case OEM_DISPLAY_SG:  // SG
      //
      // Add IGD to ConOut
      //
      TempDevicePath = (EFI_DEVICE_PATH_PROTOCOL *) &gPlatformIGDDevice;

      // Falls through to DisplayPath

    case OEM_DISPLAY_PATH: // Exact device path
      //
      // Use exact device path
      //
      Status = gBS->LocateDevicePath (&gEfiPciIoProtocolGuid, &TempDevicePath, &Handle);
      if (!EFI_ERROR (Status) && IsDevicePathEnd (TempDevicePath) && IsVgaHandle (Handle)) {
        break;
      }
    default:
      Handle = NULL;
      DEBUG ((DEBUG_ERROR, "[PlatformBds] No video controller!\n"));
      break;
  }

  if (Handle != NULL) {
    //
    // Connect the GOP driver
    //
    gBS->ConnectController (Handle, NULL, NULL, TRUE);

    //
    // Get the GOP device path
    // NOTE: We may get a device path that contains Controller node in it.
    //
    TempDevicePath = EfiBootManagerGetGopDevicePath (Handle);
    if (TempDevicePath != NULL) {
      Temp = ConsoleOut;
      ConsoleOut = UpdateDevicePath (ConsoleOut, TempDevicePath);
      if (Temp != NULL) {
        FreePool (Temp);
      }
      FreePool (TempDevicePath);
      Status = gRT->SetVariable (
                      L"ConOut",
                      &gEfiGlobalVariableGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                      GetDevicePathSize (ConsoleOut),
                      ConsoleOut
                      );
    }
  }

  gBS->FreePool (ConsoleOut);

  //
  // Fill ConIn/ConOut in Full Configuration boot mode
  //
  DEBUG ((DEBUG_INFO, "PlatformBootManagerInit - %x\n", mBootMode));
  if (mBootMode == BOOT_WITH_FULL_CONFIGURATION ||
      mBootMode == BOOT_WITH_DEFAULT_SETTINGS ||
      mBootMode == BOOT_WITH_FULL_CONFIGURATION_PLUS_DIAGNOSTICS ||
      mBootMode == BOOT_IN_RECOVERY_MODE) {


    VarConOut = NULL;
    GetEfiGlobalVariable2 (L"ConOut", &VarConOut, NULL);
    if (VarConOut != NULL) {
        FreePool (VarConOut);
    }
    VarConIn = NULL;
    GetEfiGlobalVariable2 (L"ConIn", &VarConIn, NULL);
    if (VarConIn  != NULL) {
        FreePool (VarConIn);
    }

    if (VarConOut == NULL || VarConIn == NULL) {
      //
      // Only fill ConIn/ConOut when ConIn/ConOut is empty because we may drop to Full Configuration boot mode in non-first boot
      //

      if (PlatformConsoles != NULL) {
          while ((*PlatformConsoles).DevicePath != NULL) {
            //
            // Update the console variable with the connect type
            //
            if (((*PlatformConsoles).ConnectType & CONSOLE_IN) == CONSOLE_IN) {
              EfiBootManagerUpdateConsoleVariable (ConIn, (*PlatformConsoles).DevicePath, NULL);
            }
            if (((*PlatformConsoles).ConnectType & CONSOLE_OUT) == CONSOLE_OUT) {
              EfiBootManagerUpdateConsoleVariable (ConOut, (*PlatformConsoles).DevicePath, NULL);
            }
            if (((*PlatformConsoles).ConnectType & STD_ERROR) == STD_ERROR) {
              EfiBootManagerUpdateConsoleVariable (ErrOut, (*PlatformConsoles).DevicePath, NULL);
            }
            PlatformConsoles++;
          }
      }
    }

    else {

      if (mBootMode == BOOT_WITH_DEFAULT_SETTINGS) {

        VarConIn = NULL;
        GetEfiGlobalVariable2 (L"ConIn", &VarConIn, NULL);

        Instance      = GetNextDevicePathInstance (&VarConIn, &InstanceSize);
        InstanceSize -= END_DEVICE_PATH_LENGTH;

        while (Instance != NULL) {
          Next = Instance;
          while (!IsDevicePathEndType (Next)) {
            Next = NextDevicePathNode (Next);
            if (DevicePathType (Next) == MESSAGING_DEVICE_PATH && DevicePathSubType (Next) == MSG_VENDOR_DP) {
              //
              // Restoring default serial device path
              //
              EfiBootManagerUpdateConsoleVariable (ConIn, NULL, Instance);
              EfiBootManagerUpdateConsoleVariable (ConOut, NULL, Instance);

            }
          }
          FreePool(Instance);
          Instance      = GetNextDevicePathInstance (&VarConIn, &InstanceSize);
          InstanceSize -= END_DEVICE_PATH_LENGTH;
        }

      }

    }
  }

  //
  // Exit PM auth before Legacy OPROM run.
  //

  ExitPmAuth ();
}

VOID
ConnectSequence (
  VOID
  )
{
  EFI_HANDLE               DeviceHandle;
  EFI_STATUS               Status;
  EFI_DEVICE_PATH_PROTOCOL **PlatformConnectSequence;

  PERF_FUNCTION_BEGIN (PERF_VERBOSITY_STANDARD); // MS_CHANGE

  //
  // Here we can get the customized platform connect sequence
  // Notes: we can connect with new variable which record the
  // last time boots connect device path sequence
  //
  PlatformConnectSequence = mPlatformConnectSequence;
  if (PlatformConnectSequence != NULL) {
      while (*PlatformConnectSequence != NULL) {
        //
        // Build the platform boot option
        //
        Status = EfiBootManagerConnectDevicePath (*PlatformConnectSequence, &DeviceHandle);
        if (!EFI_ERROR (Status)) {
            gBS->ConnectController (DeviceHandle, NULL, NULL, TRUE);
        }
        PlatformConnectSequence++;
      }
  }

  //
  // Dispatch again since Switchable Graphics driver depends on PCI_IO protocol
  //
  gDS->Dispatch ();

  PERF_FUNCTION_END (PERF_VERBOSITY_STANDARD); // MS_CHANGE
}

STATIC
EFI_STATUS
SetMorControl (
  VOID
  )
{
  UINT8                        MorControl;
  UINTN                        VariableSize;
  EFI_STATUS                   Status;

  VariableSize = sizeof (MorControl);
  MorControl = 1;

  Status = gRT->SetVariable(
                  MEMORY_OVERWRITE_REQUEST_VARIABLE_NAME,
                  &gEfiMemoryOverwriteControlDataGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  VariableSize,
                  &MorControl
                  );

  return Status;
}


/**
  The function will excute with as the platform policy, current policy
  is driven by boot mode. IBV/OEM can customize this code for their specific
  policy action.

  @param DriverOptionList - The header of the driver option link list
  @param BootOptionList   - The header of the boot option link list
  @param ProcessCapsules  - A pointer to ProcessCapsules()
  @param BaseMemoryTest   - A pointer to BaseMemoryTest()
**/
VOID
EFIAPI
PlatformBootManagerAfterConsole (
  VOID
  )
{
  EFI_STATUS                    Status;
//  EFI_INPUT_KEY                 Key;

  if (PcdGetBool(PcdTestKeyUsed) == TRUE) {
    Print(L"WARNING: Capsule Test Key is used.\n");
    DEBUG((DEBUG_INFO, "WARNING: Capsule Test Key is used.\n"));
  }

  mPlatformConnectSequence = DeviceBootManagerAfterConsole ();

  //
  // Boot Mode obtained in BeforeConsole action.
  //
  DEBUG((DEBUG_INFO,"BootMode -%x\n", mBootMode));

  //
  // Go the different platform policy with different boot mode
  // Notes: this part code can be change with the table policy
  //
  switch (mBootMode) {
    case BOOT_ON_S4_RESUME:
    case BOOT_WITH_MINIMAL_CONFIGURATION:
      DEBUG((DEBUG_ERROR, "THIS BOOT MODE IS UNSUPPORTED.  0x%X \n", mBootMode));

    case BOOT_ASSUMING_NO_CONFIGURATION_CHANGES:

      // run memory test here to mark all memory good.  This is a hack until we get real BDS.
      Status = MemoryTest(QUICK);  //we use NULL memory test so level doesn't matter.

      //
      // Perform some platform specific connect sequence
      //
      ConnectSequence ();

      break;

    case BOOT_ON_FLASH_UPDATE:
      EfiBootManagerConnectAll();
      Status = ProcessCapsules();

      // If capsule update require reboot
      // this function will not return.
      if (EFI_ERROR(Status))
      {
        SetMorControl();
        DEBUG((DEBUG_INFO, "Locate and Process Capsules returned error (Status=%r). Setting MOR to clear memory and initiating reset.\n", Status));
      }

      //If we get here we need to reboot as we never want to boot in Flash Update mode.
      gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
      break;


    case BOOT_IN_RECOVERY_MODE:
      DEBUG((DEBUG_ERROR, "THIS BOOT MODE IS UNSUPPORTED.  0x%X \n", mBootMode));

      //
      // In recovery boot mode, we still enter to the
      // frong page now
      //

      break;

    case BOOT_WITH_FULL_CONFIGURATION:
    case BOOT_WITH_FULL_CONFIGURATION_PLUS_DIAGNOSTICS:
      DEBUG((DEBUG_ERROR, "THIS BOOT MODE IS UNSUPPORTED.  0x%X \n", mBootMode));

    case BOOT_WITH_DEFAULT_SETTINGS:
    default:
      // run memory test here to mark all memory good.  This is a hack until we get real BDS.
      Status = MemoryTest(QUICK);  //we use NULL memory test so level doesn't matter.
      //
      // Perform some platform specific connect sequence
      //
      // PERF_START_EX(NULL,"EventRec", NULL, AsmReadTsc(), 0x7050); // MS_CHANGE
      ConnectSequence ();
      // PERF_END_EX(NULL,"EventRec", NULL, AsmReadTsc(), 0x7051); // MS_CHANGE

      break;
  }

  //
  // For all cases, we need to call ProcessCapsules in order to clear
  // the capsule variables. The BOOT_ON_FLASH_UPDATE case above calls this
  // routine but the system is always reset in that case before reaching this
  // point.
  //
  (void)ProcessCapsules();
}

/**
  This function is called each second during the boot manager waits the timeout.

  @param TimeoutRemain  The remaining timeout.
**/
 VOID
 EFIAPI
PlatformBootManagerWaitCallback (
  UINT16          TimeoutRemain
  )
 {
  return;
 }
