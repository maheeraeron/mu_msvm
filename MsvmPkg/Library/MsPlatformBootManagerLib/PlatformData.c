/** @file
  Defined the platform specific device path which will be used by
  platform Bbd to perform the platform policy connect.

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

#include "BdsPlatform.h"


GLOBAL_REMOVE_IF_UNREFERENCED USB_CLASS_FORMAT_DEVICE_PATH gUsbClassKeyboardDevicePath = {
  {
    {
      MESSAGING_DEVICE_PATH,
      MSG_USB_CLASS_DP,
      (UINT8) (sizeof (USB_CLASS_DEVICE_PATH)),
      (UINT8) ((sizeof (USB_CLASS_DEVICE_PATH)) >> 8)
    },
    0xffff,           // VendorId
    0xffff,           // ProductId
    CLASS_HID,        // DeviceClass
    SUBCLASS_BOOT,    // DeviceSubClass
    PROTOCOL_KEYBOARD // DeviceProtocol
  },

  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    END_DEVICE_PATH_LENGTH,
    0
  }
};

//
// Onboard VGA controller device path
//
GLOBAL_REMOVE_IF_UNREFERENCED PLATFORM_ONBOARD_CONTROLLER_DEVICE_PATH         gPlatformIGDDevice = {
  gPciRootBridge,
  {
    HARDWARE_DEVICE_PATH,
    HW_PCI_DP,
    (UINT8) (sizeof (PCI_DEVICE_PATH)),
    (UINT8) ((sizeof (PCI_DEVICE_PATH)) >> 8),
    0x0,
    0x2
  },
  gEndEntire
};

GLOBAL_REMOVE_IF_UNREFERENCED PLATFORM_PEG_ROOT_CONTROLLER_DEVICE_PATH        gPlatformPegRootController = {
  gPciRootBridge,
  {
    HARDWARE_DEVICE_PATH,
    HW_PCI_DP,
    (UINT8) (sizeof (PCI_DEVICE_PATH)),
    (UINT8) ((sizeof (PCI_DEVICE_PATH)) >> 8),
    0x0,
    0x1
  },
  gEndEntire
};

GLOBAL_REMOVE_IF_UNREFERENCED PLATFORM_ONBOARD_CONTROLLER_DEVICE_PATH         gPlatformPchPcieRootController = {
  gPciRootBridge,
  {
    HARDWARE_DEVICE_PATH,
    HW_PCI_DP,
    (UINT8) (sizeof (PCI_DEVICE_PATH)),
    (UINT8) ((sizeof (PCI_DEVICE_PATH)) >> 8),
    0x0,
    0x1C
  },
  gEndEntire
};
