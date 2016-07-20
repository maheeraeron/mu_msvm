/** @file
  Platform BDS customizations include file.

  Copyright (c) 2006 - 2007, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  BdsPlatform.h

Abstract:

  Head file for BDS Platform specific code

**/

#ifndef _PLATFORM_SPECIFIC_BDS_PLATFORM_H_
#define _PLATFORM_SPECIFIC_BDS_PLATFORM_H_


#include <PiDxe.h>

#include <IndustryStandard/Pci.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/SmBios.h>
#include <IndustryStandard/PeImage.h>

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/PciLib.h>
#include <Library/GenericBdsLib.h>
#include <Library/PlatformBdsLib.h>
#include <Library/HobLib.h>
#include <Library/UefiLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>

#include <Protocol/Decompress.h>
#include <Protocol/PciIo.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SerialIo.h>

#include <Guid/Acpi.h>
#include <Guid/SmBios.h>
#include <Guid/Mps.h>
#include <Guid/HobList.h>
#include <Guid/GlobalVariable.h>

#include <EfiNt.h>
#include <Protocol/vmbus.h>
#include <Protocol/Emcl.h>
#include <Library/EmclLib.h>
#include "PlatformConsole.h"

//
// needed to get VMBUS guids
//
#include <hyperkbdprotocol.h>


extern BDS_CONSOLE_CONNECT_ENTRY  gPlatformConsole[];
extern EFI_DEVICE_PATH_PROTOCOL   *gPlatformConnectSequence[];
extern EFI_DEVICE_PATH_PROTOCOL   *gPlatformDriverOption[];
extern VENDOR_DEVICE_PATH         gTerminalTypeDeviceNode;

//
//
//
#define VarConsoleInpDev        L"ConInDev"
#define VarConsoleInp           L"ConIn"
#define VarConsoleOutDev        L"ConOutDev"
#define VarConsoleOut           L"ConOut"
#define VarErrorOutDev          L"ErrOutDev"
#define VarErrorOut             L"ErrOut"

#define gVtUtf8Terminal \
  { \
    { \
      MESSAGING_DEVICE_PATH, \
      MSG_VENDOR_DP, \
      { \
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)), \
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8) \
      } \
    }, \
    DEVICE_PATH_MESSAGING_VT_UTF8 \
  }

//
// Platform BDS Functions
//

VOID
PlatformBdsGetDriverOption (
  IN LIST_ENTRY               *BdsDriverLists
  );


EFI_STATUS
PlatformBdsShowProgress (
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL TitleForeground,
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL TitleBackground,
  CHAR16                        *Title,
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL ProgressColor,
  UINTN                         Progress,
  UINTN                         PreviousValue
  );

VOID
PlatformBdsConnectSequence (
  VOID
  );

EFI_STATUS
PlatformBdsConnectConsole (
  IN BDS_CONSOLE_CONNECT_ENTRY   *PlatformConsole
  );

EFI_STATUS
PlatformBdsNoConsoleAction (
  VOID
  );

EFI_STATUS
PlatformBdsSetConsoleMode(
  VOID
  );

extern EFI_GUID gMsvmConsoleProtocolGuid;

#endif // _PLATFORM_SPECIFIC_BDS_PLATFORM_H_
