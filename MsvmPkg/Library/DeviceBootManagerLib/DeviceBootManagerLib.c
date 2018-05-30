/** @file
 *Device Boot Manager  - Device Extensions to BdsDxe.

Copyright (c) 2017, Microsoft Corporation

All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**/


#include <Uefi.h>
#include <EfiNt.h>

#include <Protocol/Emcl.h>

#include <Library/DeviceBootManagerLib.h>
#include <Library/MsLogoLib.h>
#include <Library/MsPlatBdsLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/EmclLib.h>
#include <VirtualDeviceId.h>
#include <hyperkbdprotocol.h>
#include <Library/DevicePathLib.h>
#include <Library/PcdLib.h>

//
// Predefined platform default console device path
//
static BDS_CONSOLE_CONNECT_ENTRY   gPlatformConsoles[] = {
  //
  // Place holder for serial console. Any non USB device used for
  // CONIN must be in this table.  Any non display used for CONOUT
  // must also be in this list.
  //
  {
    NULL,
    CONSOLE_IN
  },
  {
    NULL,
    0
  }
};


/**
 * Constructor   - This runs when BdsDxe is loaded, before BdsArch protocol is published
 *
 * @return EFI_STATUS
 */
EFI_STATUS
EFIAPI
DeviceBootManagerConstructor (
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
  ) {

    return EFI_SUCCESS;
}

/**
  OnDemandConInCOnnect
 */
EFI_DEVICE_PATH_PROTOCOL **
EFIAPI
DeviceBootManagerOnDemandConInConnect (
  VOID
  ) {

    return NULL;
}

/**
  Do the device specific action at start of BdsEntry (callback into BdsArch from DXE Dispatcher)
**/
VOID
EFIAPI
DeviceBootManagerBdsEntry (
  VOID
  ) {
   PlatformBdsInit();
}

/**
  Do the device specific action before the console is connected.

  Such as:
      Initialize the platform boot order
      Supply Console information
**/
EFI_HANDLE
EFIAPI
DeviceBootManagerBeforeConsole (
  EFI_DEVICE_PATH_PROTOCOL    **DevicePath,
  BDS_CONSOLE_CONNECT_ENTRY   **PlatformConsoles
  ) {
    EFI_STATUS               Status;
    EFI_HANDLE              *HandleBuffer;
    UINTN                    HandleCount;
    UINTN                    Index;
    EFI_HANDLE               ConsoleIn = NULL;
    EFI_HANDLE               ConsoleOut = NULL;

    *DevicePath = NULL;
    *PlatformConsoles = NULL;

    Status = gBS->LocateHandleBuffer (
       ByProtocol,
       &gEfiVmbusProtocolGuid,
       NULL,
       &HandleCount,
       &HandleBuffer
       );
    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "Handles with gEfiVmbusProtocolGuid not found. Status = %r\n", Status));
        goto Exit;
    }

    DEBUG((DEBUG_INFO, "Count of handles with gEfiVmbusProtocolGuid = %d\n", HandleCount));

    for (Index = 0; Index < HandleCount; Index++) {
        if (ConsoleIn == NULL) {
            Status = EmclChannelTypeSupported(HandleBuffer[Index],
                                              &HK_INTERFACE_GUID,
                                              NULL);
            if (!EFI_ERROR(Status)) {
                ConsoleIn = HandleBuffer[Index]; 
            }
        }

        if (ConsoleOut == NULL) {
            Status = EmclChannelTypeSupported(HandleBuffer[Index],
                                              &SYNTHVID_CLASS_ID,
                                              NULL);
            if (!EFI_ERROR(Status)) {
                ConsoleOut = HandleBuffer[Index]; 
            }
            else {
                Status = EmclChannelTypeSupported(HandleBuffer[Index],
                                                  &SYNTH3DVID_DEVICE_ID,
                                                  NULL);
                if (!EFI_ERROR(Status)) {
                    ConsoleOut = HandleBuffer[Index]; 
                }
            }
        }
    }

    if (ConsoleIn != NULL) {
        Status = gBS->HandleProtocol (
                        ConsoleIn,
                        &gEfiDevicePathProtocolGuid,
                        &(gPlatformConsoles[0].DevicePath)     // device path for ConIn
                        );
        if (EFI_ERROR (Status)) {
            DEBUG((DEBUG_ERROR, "Device Path on handle of Hyper-V keyboard device not found.  Status = %r\n", Status));
        }
    }
    else {
        DEBUG((DEBUG_ERROR, "Handle for Hyper-V keyboard device not found\n"));
    }

    if (ConsoleOut != NULL) {
        Status = gBS->HandleProtocol (
                        ConsoleOut,
                        &gEfiDevicePathProtocolGuid,
                        DevicePath                             // device path for ConOut
                        );
        if (EFI_ERROR (Status)) {
            ConsoleOut = NULL;
            DEBUG((DEBUG_ERROR, "Device Path on handle of Hyper-V video device not found.  Status = %r\n", Status));
        }
        else {
            *DevicePath = DuplicateDevicePath(*DevicePath);
            if (*DevicePath == NULL) {
                ConsoleOut = NULL;
            }
        }
    }
    else {
        DEBUG((DEBUG_ERROR, "Handle for Hyper-V video device not found\n"));
    }

    *PlatformConsoles = (BDS_CONSOLE_CONNECT_ENTRY *)&gPlatformConsoles;
    gBS->FreePool(HandleBuffer);

Exit:
    return ConsoleOut; 
}

/**
  Do the device specific action after the console is connected.

  Such as:
**/
EFI_DEVICE_PATH_PROTOCOL **
EFIAPI
DeviceBootManagerAfterConsole (
  VOID
  ) {

    EnableQuietBoot(PcdGetPtr(PcdLogoFile));

    //
    // For an optimized boot, Bds only connects the device path as provided
    // in a boot option.  There are boot scenarios where the boot loader is on
    // one device, and the OS that is being booted is on another device. The
    // fully optimized Bds will "fail" to boot in this scenario.
    //
    // For Hyper-V, there is a hint called PcdIsVmbfsBoot.  If this hint is true,
    // locate and insure all VmBus disks are enumerated.  This code an be improved
    // later by querying Hyper-V for the list of disks.  For the time being,
    // connect all of the devices provided by VmBus.
    //
    if (PcdGetBool(PcdIsVmbfsBoot)) {

        EFI_HANDLE  *HandleBuffer = NULL;
        UINTN        HandleCount;
        UINTN        Index;
        EFI_STATUS   Status;

        do {
            HandleBuffer = NULL;
            HandleCount = 0;
            //
            // Find all instances of VmbusRoot protocol.
            //
            gBS->LocateHandleBuffer( ByProtocol,
                                    &gEfiVmbusRootProtocolGuid,
                                     NULL,
                                    &HandleCount,
                                    &HandleBuffer );

            for (Index = 0; Index < HandleCount; Index++) {
                gBS->ConnectController(HandleBuffer[Index], NULL, NULL, TRUE);
            }

            if (HandleBuffer != NULL) {
                gBS->FreePool (HandleBuffer);
            }

            Status = gDS->Dispatch();

        } while (!EFI_ERROR(Status));
    }

    return NULL;
}

/**
ProcessBootCompletion
*/
BOOLEAN
EFIAPI
DeviceBootManagerProcessBootCompletion (
  IN EFI_BOOT_MANAGER_LOAD_OPTION *BootOption
) {

    return FALSE;
}

/**
 * Check for HardKeys during boot.  If the hard keys are pressed, builds
 * a boot option for the specific hard key setting.
 *
 *
 * @param BootOption   - Boot Option filled in based on which hard key is pressed
 *
 * @return EFI_STATUS  - EFI_NOT_FOUND - no hard key pressed, no BootOption
 *                       EFI_SUCCESS   - BootOption is valid
 *                       other error   - Unable to build BootOption
 */
EFI_STATUS
EFIAPI
DeviceBootManagerPriorityBoot (
    EFI_BOOT_MANAGER_LOAD_OPTION   *BootOption
    ) {

    return EFI_NOT_FOUND;
}
