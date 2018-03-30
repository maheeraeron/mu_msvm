/**@file
  Header file for BDS Platform specific code

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

#ifndef _BDS_PLATFORM_H
#define _BDS_PLATFORM_H

#include <PiDxe.h>

#include <IndustryStandard/Pci30.h>

#include <Guid/CapsuleVendor.h>
#include <Guid/EventGroup.h>
#include <Guid/FileInfo.h>
#include <Guid/GlobalVariable.h>
#include <Guid/ImageAuthentication.h>
#include <Guid/MemoryOverwriteControl.h>
#include <Guid/MemoryTypeInformation.h>

#include <Protocol/DevicePath.h>
#include <Protocol/DxeSmmReadyToLock.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/GenericMemoryTest.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/LoadFile.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SimpleNetwork.h>
#include <Protocol/UgaDraw.h>
#include <Protocol/UsbIo.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CapsuleLib.h>
#include <Library/DebugLib.h>
#include <Library/DeviceBootManagerLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PciLib.h>
#include <Library/Performance2Lib.h>
#include <Library/PlatformBootManagerLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>


extern EFI_HII_HANDLE            gPlatformBdsLibStringPackHandle;

#define gPciRootBridge \
  { \
    ACPI_DEVICE_PATH, ACPI_DP, (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)), (UINT8) \
      ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8), EISA_PNP_ID (0x0A03), 0 \
  }

#define gEndEntire \
  { \
    END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, END_DEVICE_PATH_LENGTH, 0 \
  }

typedef enum {
  AcpiFloppyBoot,
  MessageAtapiBoot,
  MessageSataBoot,
  MessageUsbBoot,
  MessageScsiBoot,
  MessageNetworkBoot,
  UnsupportedBoot
} BOOT_TYPE;

//
// Platform Root Bridge
//
typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_ROOT_BRIDGE_DEVICE_PATH;



typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           PciDevice;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_ONBOARD_CONTROLLER_DEVICE_PATH;

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           Pci0Device;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_PEG_ROOT_CONTROLLER_DEVICE_PATH;

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           PciBridge;
  PCI_DEVICE_PATH           PciDevice;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_PCI_CONTROLLER_DEVICE_PATH;

//
// Below is the boot option device path
//


#define CLASS_HID           3
#define SUBCLASS_BOOT       1
#define PROTOCOL_KEYBOARD   1

typedef struct {
  USB_CLASS_DEVICE_PATH           UsbClass;
  EFI_DEVICE_PATH_PROTOCOL        End;
} USB_CLASS_FORMAT_DEVICE_PATH;


extern EFI_DEVICE_PATH_PROTOCOL                  *gPlatformRootBridges[];
extern EFI_DEVICE_PATH_PROTOCOL                  *gPlatformDriverOption[];
extern EFI_DEVICE_PATH_PROTOCOL                  *gPlatformBootOption[];
extern PLATFORM_ONBOARD_CONTROLLER_DEVICE_PATH   gPlatformIGDDevice;
extern PLATFORM_PEG_ROOT_CONTROLLER_DEVICE_PATH  gPlatformPegRootController;
extern PLATFORM_ONBOARD_CONTROLLER_DEVICE_PATH   gPlatformPchPcieRootController;
extern USB_CLASS_FORMAT_DEVICE_PATH              gUsbClassKeyboardDevicePath;

//
// Platform BDS Functions
//

typedef
VOID
(*PROCESS_VARIABLE) (
  VOID  **Variable,
  UINTN *VariableSize
  );
VOID
UpdateEfiGlobalVariable (
  CHAR16           *VariableName,
  EFI_GUID         *AgentGuid,
  PROCESS_VARIABLE ProcessVariable
  )
/*++

Routine Description:

  Generic function to update the console variable.
  Please refer to FastBootSupport.c for how to use it.

Arguments:

  VariableName    - The name of the variable to be updated
  AgentGuid       - The Agent GUID
  ProcessVariable - The function pointer to update the variable
                    NULL means to restore to the original value

--*/
;

/**
  Perform the memory test base on the memory test intensive level,
  and update the memory resource.

  @param  Level         The memory test intensive level.

  @retval EFI_STATUS    Success test all the system memory and update
                        the memory resource

**/
EFI_STATUS
MemoryTest (
  IN EXTENDMEM_COVERAGE_LEVEL Level
  );


/*++

Routine Description:

Connect with predeined platform connect sequence,
the OEM/IBV can customize with their own connect sequence.

Arguments:

BootMode                              Boot mode of this boot.

Returns:

None.

--*/
VOID
ConnectSequence (
  VOID
  );



BOOLEAN
EFIAPI
CompareBootOption (
  CONST EFI_BOOT_MANAGER_LOAD_OPTION  *Left,
  CONST EFI_BOOT_MANAGER_LOAD_OPTION  *Right
  );


/**
  Print the boot prompt.
**/
VOID
PrintBootPrompt (
  VOID
  );

VOID
BootUi (
  VOID
  );


EFI_STATUS
EFIAPI
EfiPlatformBootManagerProcessCapsules (
  VOID
  );

//
// MSchange - Make this function available to TcgPhysicalPresence.c
//
BOOLEAN
IsGopDevicePath (
  EFI_DEVICE_PATH_PROTOCOL *DevicePath
  );

#endif
