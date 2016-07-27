/*++

Copyright (c) 2006 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  S3Gop.c

Abstract:

  S3 Graphics Output Protocol driver.

Revision History

--*/

#include "S3Gop.h"

//
// EFI Driver Binding Protocol Instance
//
EFI_DRIVER_BINDING_PROTOCOL gBiosVideoDriverBinding = {
  BiosVideoDriverBindingSupported,
  BiosVideoDriverBindingStart,
  BiosVideoDriverBindingStop,
  0x3,
  NULL,
  NULL
};

EFI_STATUS
BiosVideoChildHandleInstall (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     ParentHandle,
  IN  EFI_PCI_IO_PROTOCOL            *ParentPciIo,
  IN  EFI_DEVICE_PATH_PROTOCOL       *ParentDevicePath,
  IN  EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
BiosVideoChildHandleUninstall (
  EFI_DRIVER_BINDING_PROTOCOL    *This,
  EFI_HANDLE                     Controller,
  EFI_HANDLE                     Handle
  );

VOID
BiosVideoDeviceReleaseResource (
  BIOS_VIDEO_DEV  *BiosVideoPrivate
  );

//
// Driver Entry Point
//
EFI_STATUS
EFIAPI
BiosVideoDriverEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

  Routine Description:

    Driver Entry Point.

  Arguments:

  ImageHandle - Handle of driver image.
  SystemTable - Pointer to system table.

  Returns:

    EFI_STATUS

--*/
{
  EFI_STATUS  Status;

  //
  // Install driver model protocol(s).
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gBiosVideoDriverBinding,
             ImageHandle,
             &gBiosVideoComponentName,
             &gBiosVideoComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

EFI_STATUS
EFIAPI
BiosVideoDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

  Routine Description:

    Supported.

  Arguments:

  This - Pointer to driver binding protocol
  Controller - Controller handle to connect
  RemainingDevicePath - A pointer to the remaining portion of a device path


  Returns:

  EFI_STATUS - EFI_SUCCESS:This controller can be managed by this driver,
               Otherwise, this controller cannot be managed by this driver

--*/
{
  EFI_STATUS           Status;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  UINT16               VendorId;
  UINT16               DeviceId;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Read the PCI Configuration Header from the PCI Device
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint16,
                        PCI_VENDOR_ID_OFFSET,
                        1,
                        &VendorId
                        );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint16,
                        PCI_DEVICE_ID_OFFSET,
                        1,
                        &DeviceId
                        );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  if (VendorId != S3_PCI_VENDOR_ID || DeviceId != S3_PCI_DEVICE_ID) {
    Status = EFI_UNSUPPORTED;
  }

Done:
  gBS->CloseProtocol (
         Controller,
         &gEfiPciIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  return Status;
}

EFI_STATUS
EFIAPI
BiosVideoDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

  Routine Description:

    Install Graphics Output Protocol onto VGA device handles

  Arguments:

  This - Pointer to driver binding protocol
  Controller - Controller handle to connect
  RemainingDevicePath - A pointer to the remaining portion of a device path

  Returns:

    EFI_STATUS

--*/
{
  EFI_STATUS                      Status;
  EFI_DEVICE_PATH_PROTOCOL        *ParentDevicePath;
  EFI_PCI_IO_PROTOCOL             *PciIo;

  PciIo = NULL;
  //
  // Prepare for status code
  //
  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &ParentDevicePath
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Open the IO Abstraction(s) needed
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Create child handle and install GraphicsOutputProtocol on it
  //
  Status = BiosVideoChildHandleInstall (
             This,
             Controller,
             PciIo,
             ParentDevicePath,
             RemainingDevicePath
             );

Done:
  if (EFI_ERROR (Status)) {
    if (PciIo != NULL) {
      //
      // Release PCI I/O Protocols on the controller handle.
      //
      gBS->CloseProtocol (
             Controller,
             &gEfiPciIoProtocolGuid,
             This->DriverBindingHandle,
             Controller
             );
    }
  }

  return Status;
}

EFI_STATUS
EFIAPI
BiosVideoDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++

  Routine Description:

    Stop.

  Arguments:

  This - Pointer to driver binding protocol
  Controller - Controller handle to connect
  NumberOfChilren - Number of children handle created by this driver
  ChildHandleBuffer - Buffer containing child handle created

  Returns:

  EFI_SUCCESS - Driver disconnected successfully from controller
  EFI_UNSUPPORTED - Cannot find BIOS_VIDEO_DEV structure

--*/
{
  EFI_STATUS                   Status;
  BIOS_VIDEO_DEV               *BiosVideoPrivate;
  BOOLEAN                      AllChildrenStopped;
  UINTN                        Index;

  BiosVideoPrivate = NULL;

  if (NumberOfChildren == 0) {
    //
    // Close PCI I/O protocol on the controller handle
    //
    gBS->CloseProtocol (
           Controller,
           &gEfiPciIoProtocolGuid,
           This->DriverBindingHandle,
           Controller
           );

    return EFI_SUCCESS;
  }

  AllChildrenStopped = TRUE;
  for (Index = 0; Index < NumberOfChildren; Index++) {
    Status = BiosVideoChildHandleUninstall (This, Controller, ChildHandleBuffer[Index]);

    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
    }
  }

  if (!AllChildrenStopped) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
BiosVideoChildHandleInstall (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     ParentHandle,
  IN  EFI_PCI_IO_PROTOCOL            *ParentPciIo,
  IN  EFI_DEVICE_PATH_PROTOCOL       *ParentDevicePath,
  IN  EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

Routine Description:
  Install child handles if the Handle supports MBR format.

Arguments:       
  This       - Calling context.
  Handle     - Parent Handle 
  PciIo      - Parent PciIo interface
  DevicePath - Parent Device Path

Returns:
  EFI_SUCCESS - If a child handle was added
  other       - A child handle was not added

--*/
{
  EFI_STATUS               Status;
  BIOS_VIDEO_DEV           *BiosVideoPrivate;
  ACPI_ADR_DEVICE_PATH     AcpiDeviceNode;
  PCI_TYPE00               Pci;

  //
  // Allocate the private device structure for video device
  //
  BiosVideoPrivate = AllocateZeroPool (sizeof (BIOS_VIDEO_DEV));
  if (BiosVideoPrivate == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialize the child private structure
  //
  BiosVideoPrivate->Signature = BIOS_VIDEO_DEV_SIGNATURE;
  BiosVideoPrivate->Handle    = NULL;

  //
  // Read the PCI Configuration Header from the PCI Device
  //
  Status = ParentPciIo->Pci.Read (ParentPciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Fill in the Graphics Output Protocol
  //
  BiosVideoPrivate->GraphicsOutput.QueryMode = BiosVideoGraphicsOutputQueryMode;
  BiosVideoPrivate->GraphicsOutput.SetMode   = BiosVideoGraphicsOutputSetMode;
  BiosVideoPrivate->GraphicsOutput.Blt       = BiosVideoGraphicsOutputBlt;
  BiosVideoPrivate->GraphicsOutput.Mode      = &BiosVideoPrivate->Mode;

  BiosVideoPrivate->Mode.MaxMode         = 1;
  BiosVideoPrivate->Mode.Mode            = 0xffffffff;
  BiosVideoPrivate->Mode.Info            = &BiosVideoPrivate->ModeInfo;
  BiosVideoPrivate->Mode.SizeOfInfo      = sizeof (BiosVideoPrivate->ModeInfo);
  BiosVideoPrivate->Mode.FrameBufferBase = Pci.Device.Bar[0] & 0xfffffff0;
  BiosVideoPrivate->Mode.FrameBufferSize = 0x01000000;

  BiosVideoPrivate->ModeInfo.Version              = 0;
  BiosVideoPrivate->ModeInfo.HorizontalResolution = 800;
  BiosVideoPrivate->ModeInfo.VerticalResolution   = 600;
  BiosVideoPrivate->ModeInfo.PixelFormat          = PixelBlueGreenRedReserved8BitPerColor;
  BiosVideoPrivate->ModeInfo.PixelsPerScanLine    = 800;

  //
  // Fill in Graphics Output specific mode structures
  //
  BiosVideoPrivate->HardwareNeedsStarting = TRUE;

  //
  // When check for VBE, PCI I/O protocol is needed, so use parent's protocol interface temporally
  //
  BiosVideoPrivate->PciIo = ParentPciIo;

  if (RemainingDevicePath == NULL) {
    ZeroMem (&AcpiDeviceNode, sizeof (ACPI_ADR_DEVICE_PATH));
    AcpiDeviceNode.Header.Type = ACPI_DEVICE_PATH;
    AcpiDeviceNode.Header.SubType = ACPI_ADR_DP;
    AcpiDeviceNode.ADR = ACPI_DISPLAY_ADR (1, 0, 0, 1, 0, ACPI_ADR_DISPLAY_TYPE_VGA, 0, 0);
    SetDevicePathNodeLength (&AcpiDeviceNode.Header, sizeof (ACPI_ADR_DEVICE_PATH));

    BiosVideoPrivate->DevicePath = AppendDevicePathNode (
                                      ParentDevicePath, 
                                      (EFI_DEVICE_PATH_PROTOCOL *) &AcpiDeviceNode
                                      );
  } else {
    BiosVideoPrivate->DevicePath = AppendDevicePathNode (ParentDevicePath, RemainingDevicePath);
  }

  //
  // Create child handle and install Graphics Output Protocol,EDID Discovered/Active Protocol
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &BiosVideoPrivate->Handle,
                  &gEfiDevicePathProtocolGuid,     BiosVideoPrivate->DevicePath,
                  &gEfiGraphicsOutputProtocolGuid, &BiosVideoPrivate->GraphicsOutput,
                  NULL
                  );

  if (!EFI_ERROR (Status)) {
    //
    // Open the Parent Handle for the child
    //
    Status = gBS->OpenProtocol (
                    ParentHandle,
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &BiosVideoPrivate->PciIo,
                    This->DriverBindingHandle,
                    BiosVideoPrivate->Handle,
                    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                    );
    if (EFI_ERROR (Status)) {
      goto Done;
    }
  }

Done:
  if (EFI_ERROR (Status)) {
    //
    // Free private data structure
    //
    BiosVideoDeviceReleaseResource (BiosVideoPrivate);
  }

  return Status;
}

EFI_STATUS
BiosVideoChildHandleUninstall (
  EFI_DRIVER_BINDING_PROTOCOL    *This,
  EFI_HANDLE                     Controller,
  EFI_HANDLE                     Handle
  )
/*++

Routine Description:

  Deregister an video child handle and free resources

Arguments:

  This            - Protocol instance pointer.
  Controller      - Video controller handle
  Handle          - Video child handle

Returns:

  EFI_STATUS

--*/
{
  EFI_STATUS                   Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput;
  BIOS_VIDEO_DEV               *BiosVideoPrivate;
  EFI_PCI_IO_PROTOCOL          *PciIo;

  BiosVideoPrivate = NULL;

  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **) &GraphicsOutput,
                  This->DriverBindingHandle,
                  Handle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (!EFI_ERROR (Status)) {
    BiosVideoPrivate = BIOS_VIDEO_DEV_FROM_GRAPHICS_OUTPUT_THIS (GraphicsOutput);
  }

  if (BiosVideoPrivate == NULL) {
    return EFI_UNSUPPORTED;
  }

  //
  // Close PCI I/O protocol that opened by child handle
  //
  Status = gBS->CloseProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  This->DriverBindingHandle,
                  Handle
                  );

  //
  // Uninstall protocols on child handle
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  BiosVideoPrivate->Handle,
                  &gEfiDevicePathProtocolGuid,     BiosVideoPrivate->DevicePath,
                  &gEfiGraphicsOutputProtocolGuid, &BiosVideoPrivate->GraphicsOutput,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    gBS->OpenProtocol (
           Controller,
           &gEfiPciIoProtocolGuid,
           (VOID **) &PciIo,
           This->DriverBindingHandle,
           Handle,
           EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
           );
    return Status;
  }

  //
  // Release all allocated resources
  //
  BiosVideoDeviceReleaseResource (BiosVideoPrivate);

  return EFI_SUCCESS;
}

VOID
BiosVideoDeviceReleaseResource (
  BIOS_VIDEO_DEV  *BiosVideoPrivate
  )
/*++
Routing Description:

  Release resources of an video child device before stopping it.

Arguments:

  BiosVideoPrivate  -  Video child device private data structure

Returns:

    NONE
    
---*/
{
  if (BiosVideoPrivate == NULL) {
    return;
  }

  //
  // Free graphics output protocol occupied resource
  //
  if (BiosVideoPrivate->DevicePath!= NULL) {
    FreePool (BiosVideoPrivate->DevicePath);
  }

  FreePool (BiosVideoPrivate);

  return;
}

//
// Graphics Output Protocol Member Functions for VESA BIOS Extensions
//
EFI_STATUS
EFIAPI
BiosVideoGraphicsOutputQueryMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
  IN  UINT32                                ModeNumber,
  OUT UINTN                                 *SizeOfInfo,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
  )
/*++

Routine Description:

  Graphics Output protocol interface to get video mode

  Arguments:
    This                  - Protocol instance pointer.
    ModeNumber            - The mode number to return information on.
    Info                  - Caller allocated buffer that returns information about ModeNumber.
    SizeOfInfo            - A pointer to the size, in bytes, of the Info buffer.

  Returns:
    EFI_SUCCESS           - Mode information returned.
    EFI_DEVICE_ERROR      - A hardware error occurred trying to retrieve the video mode.
    EFI_NOT_STARTED       - Video display is not initialized. Call SetMode ()
    EFI_INVALID_PARAMETER - One of the input args was NULL.

--*/
{
  BIOS_VIDEO_DEV        *BiosVideoPrivate;

  if (This == NULL || Info == NULL || SizeOfInfo == NULL || ModeNumber >= This->Mode->MaxMode) {
    return EFI_INVALID_PARAMETER;
  }

  BiosVideoPrivate = BIOS_VIDEO_DEV_FROM_GRAPHICS_OUTPUT_THIS (This);

  *Info = AllocateCopyPool (sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION), &BiosVideoPrivate->ModeInfo);
  if (*Info == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  *SizeOfInfo = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
BiosVideoGraphicsOutputSetMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL * This,
  IN  UINT32                       ModeNumber
  )
/*++

Routine Description:

  Graphics Output protocol interface to set video mode

  Arguments:
    This             - Protocol instance pointer.
    ModeNumber       - The mode number to be set.

  Returns:
    EFI_SUCCESS      - Graphics mode was changed.
    EFI_DEVICE_ERROR - The device had an error and could not complete the request.
    EFI_UNSUPPORTED  - ModeNumber is not supported by this device.

--*/
{
  BIOS_VIDEO_DEV          *BiosVideoPrivate;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }

  BiosVideoPrivate = BIOS_VIDEO_DEV_FROM_GRAPHICS_OUTPUT_THIS (This);

  if (!BiosVideoPrivate->HardwareNeedsStarting && ModeNumber == This->Mode->Mode) {
    return EFI_SUCCESS;
  }

  This->Mode->Mode = ModeNumber;

  BiosVideoPrivate->HardwareNeedsStarting = FALSE;

  IoWrite16 (0x46E8, 0x0016);
  IoWrite16 (0x0102, 0x0001);
  IoWrite16 (0x46E8, 0x000E);
  IoWrite16 (0x4AE8, 0x0000);
  IoWrite16 (0x03C4, 0x0100);
  IoWrite16 (0x03C4, 0x2901);
  IoWrite16 (0x03C4, 0x0F02);
  IoWrite16 (0x03C4, 0x0003);
  IoWrite16 (0x03C4, 0x0604);
  IoWrite8 (0x03C2, 0x63);
  IoWrite16 (0x03C4, 0x0300);

  IoWrite16 (0x03D4, 0xFF72);

  IoWrite8 (0x03C4, 0x01);
  IoRead8 (0x03C5); 
  IoWrite8 (0x03C5, 0x20);
  IoRead8 (0x03CC); 
  IoWrite16 (0x03D4, 0x4838);
  IoWrite16 (0x03D4, 0xA039);
  IoRead8 (0x03CC); 
  IoWrite8 (0x03D4, 0x40);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0x01);
  IoWrite8 (0x4AE8, 0x02);
  IoWrite16 (0x03D4, 0x0040);
  IoWrite8 (0x03D4, 0x31);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0x85);
  IoWrite8 (0x03D4, 0x50);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x51);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x53);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x54);
  IoWrite8 (0x03D5, 0x38);
  IoWrite8 (0x03D4, 0x55);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x58);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0xC0);
  IoWrite8 (0x03D4, 0x5C);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0x80);
  IoWrite8 (0x03D4, 0x5D);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x5E);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x60);
  IoWrite8 (0x03D5, 0x07);
  IoWrite8 (0x03D4, 0x61);
  IoWrite8 (0x03D5, 0x80);
  IoWrite8 (0x03D4, 0x62);
  IoWrite8 (0x03D5, 0xA1);
  IoWrite8 (0x03D4, 0x63);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x64);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x65);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x32);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x33);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x34);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x35);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x3A);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0x05);
  IoWrite8 (0x03D4, 0x3B);
  IoWrite8 (0x03D5, 0x5A);
  IoWrite8 (0x03D4, 0x3C);
  IoWrite8 (0x03D5, 0x10);
  IoWrite8 (0x03D4, 0x43);
  IoWrite8 (0x03D5, 0x00);
#if 0 // No S3 hardware cursor
  IoWrite8 (0x03D4, 0x45);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x46);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x47);
  IoWrite8 (0x03D5, 0xFF);
  IoWrite8 (0x03D4, 0x48);
  IoWrite8 (0x03D5, 0xFC);
  IoWrite8 (0x03D4, 0x49);
  IoWrite8 (0x03D5, 0xFF);
  IoWrite8 (0x03D4, 0x4A);
  IoWrite8 (0x03D5, 0xFF);
  IoWrite8 (0x03D4, 0x4B);
  IoWrite8 (0x03D5, 0xFF);
  IoWrite8 (0x03D4, 0x4C);
  IoWrite8 (0x03D5, 0xFF);
  IoWrite8 (0x03D4, 0x4D);
  IoWrite8 (0x03D5, 0xFF);
  IoWrite8 (0x03D4, 0x4E);
  IoWrite8 (0x03D5, 0xFF);
  IoWrite8 (0x03D4, 0x4F);
  IoWrite8 (0x03D5, 0xDF);
#endif
  IoWrite8 (0x03D4, 0x40);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0x08);
  IoWrite8 (0x03D4, 0x42);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x73);
  IoWrite8 (0x03D5, 0x53);
  IoWrite16 (0x03C4, 0x0E04);
  IoRead8 (0x03CC); 
  IoWrite8 (0x03D4, 0x40);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0x09);
  IoWrite8 (0x4AE8, 0x03);
  IoWrite16 (0x03D4, 0x0840);
  IoRead8 (0x03DA); 
  IoWrite8 (0x03C0, 0x00);
  IoWrite16 (0x03C4, 0x2001);
  IoWrite16 (0x03C4, 0x2101);
  IoWrite16 (0x03C4, 0x0F02);
  IoWrite16 (0x03C4, 0x0003);
  IoRead8 (0x03CC); 
  IoWrite8 (0x03D4, 0x5C);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0x80);
  IoWrite8 (0x03D4, 0x42);
  IoWrite8 (0x03D5, 0x00);
  IoWrite16 (0x03D4, 0x0011);
  IoWrite16 (0x03D4, 0x5F00);
  IoWrite8 (0x03C2, 0x2F);
  IoRead8 (0x03DA); 
  IoWrite8 (0x03C0, 0x00);
  IoWrite8 (0x03C0, 0x00);
  IoWrite8 (0x03C0, 0x01);
  IoWrite8 (0x03C0, 0x01);
  IoWrite8 (0x03C0, 0x02);
  IoWrite8 (0x03C0, 0x02);
  IoWrite8 (0x03C0, 0x03);
  IoWrite8 (0x03C0, 0x03);
  IoWrite8 (0x03C0, 0x04);
  IoWrite8 (0x03C0, 0x04);
  IoWrite8 (0x03C0, 0x05);
  IoWrite8 (0x03C0, 0x05);
  IoWrite8 (0x03C0, 0x06);
  IoWrite8 (0x03C0, 0x14);
  IoWrite8 (0x03C0, 0x07);
  IoWrite8 (0x03C0, 0x07);
  IoWrite8 (0x03C0, 0x08);
  IoWrite8 (0x03C0, 0x38);
  IoWrite8 (0x03C0, 0x09);
  IoWrite8 (0x03C0, 0x39);
  IoWrite8 (0x03C0, 0x0A);
  IoWrite8 (0x03C0, 0x3A);
  IoWrite8 (0x03C0, 0x0B);
  IoWrite8 (0x03C0, 0x3B);
  IoWrite8 (0x03C0, 0x0C);
  IoWrite8 (0x03C0, 0x3C);
  IoWrite8 (0x03C0, 0x0D);
  IoWrite8 (0x03C0, 0x3D);
  IoWrite8 (0x03C0, 0x0E);
  IoWrite8 (0x03C0, 0x3E);
  IoWrite8 (0x03C0, 0x0F);
  IoWrite8 (0x03C0, 0x3F);
  IoWrite8 (0x03C0, 0x10);
  IoWrite8 (0x03C0, 0x01);
  IoWrite8 (0x03C0, 0x11);
  IoWrite8 (0x03C0, 0x00);
  IoWrite8 (0x03C0, 0x12);
  IoWrite8 (0x03C0, 0x0F);
  IoWrite8 (0x03C0, 0x13);
  IoWrite8 (0x03C0, 0x00);
  IoWrite8 (0x03C0, 0x20);
  IoWrite16 (0x03CE, 0x0000);
  IoWrite16 (0x03CE, 0x0001);
  IoWrite16 (0x03CE, 0x0002);
  IoWrite16 (0x03CE, 0x0003);
  IoWrite16 (0x03CE, 0x0004);
  IoWrite16 (0x03CE, 0x0005);
  IoWrite16 (0x03CE, 0x0506);
  IoWrite16 (0x03CE, 0x0F07);
  IoWrite16 (0x03CE, 0xFF08);
  IoRead8 (0x03CC); 
  IoWrite8 (0x03D4, 0x11);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0x00);
  IoWrite16 (0x03D4, 0x7F00);
  IoWrite16 (0x03D4, 0x6301);
  IoWrite16 (0x03D4, 0x6402);
  IoWrite16 (0x03D4, 0x8203);
  IoWrite16 (0x03D4, 0x6A04);
  IoWrite16 (0x03D4, 0x1A05);
  IoWrite16 (0x03D4, 0x7406);
  IoWrite16 (0x03D4, 0x5007);
  IoWrite16 (0x03D4, 0x0008);
  IoWrite16 (0x03D4, 0x6009);
  IoWrite16 (0x03D4, 0x000A);
  IoWrite16 (0x03D4, 0x000B);
  IoWrite16 (0x03D4, 0x000C);
  IoWrite16 (0x03D4, 0x000D);
  IoWrite16 (0x03D4, 0xFF0E);
  IoWrite16 (0x03D4, 0x000F);
  IoWrite16 (0x03D4, 0x5810);
  IoWrite16 (0x03D4, 0x0C11);
  IoWrite16 (0x03D4, 0x5712);
  IoWrite16 (0x03D4, 0x8013);
  IoWrite16 (0x03D4, 0x0014);
  IoWrite16 (0x03D4, 0x5715);
  IoWrite16 (0x03D4, 0x7316);
  IoWrite16 (0x03D4, 0xE317);
  IoWrite16 (0x03D4, 0xFF18);
  IoWrite8 (0x03D4, 0x42);
  IoWrite8 (0x03D5, 0x02);
  IoWrite8 (0x03D4, 0x3B);
  IoWrite8 (0x03D5, 0xF8);
  IoWrite8 (0x03D4, 0x3C);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x31);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0x8D);
  IoWrite8 (0x03D4, 0x3A);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0x15);
  IoWrite8 (0x03D4, 0x40);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0x08);
  IoWrite8 (0x03D4, 0x50);
  IoWrite8 (0x03D5, 0xB0);
  IoWrite8 (0x03D4, 0x54);
  IoWrite8 (0x03D5, 0x18);
  IoWrite8 (0x03D4, 0x5D);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x60);
  IoWrite8 (0x03D5, 0x2F);
  IoWrite8 (0x03D4, 0x61);
  IoWrite8 (0x03D5, 0x81);
  IoWrite8 (0x03D4, 0x62);
  IoWrite8 (0x03D5, 0x92);
  IoWrite8 (0x03D4, 0x58);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0xC0);
  IoWrite8 (0x03D4, 0x33);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x43);
  IoWrite8 (0x03D5, 0x04);
  IoWrite8 (0x03D4, 0x13);
  IoWrite8 (0x03D5, 0x90);
  IoWrite8 (0x03D4, 0x5E);
  IoWrite8 (0x03D5, 0x40);
  IoWrite8 (0x03D4, 0x51);
  IoWrite8 (0x03D5, 0x00);
  IoWrite8 (0x03D4, 0x5C);
  IoWrite8 (0x03D5, 0xF0);
  IoWrite8 (0x03D4, 0x34);
  IoWrite8 (0x03D5, 0x00);
  IoRead8 (0x03CC); 
  IoWrite8 (0x03D4, 0x67);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0xD0);
  IoRead8 (0x03CC); 
  IoWrite8 (0x03D4, 0x11);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0x8C);
  IoRead8 (0x03CC); 
  IoWrite16 (0x03D4, 0x8438);
  IoWrite16 (0x03D4, 0x4039);
  IoWrite8 (0x03C4, 0x01);
  IoRead8 (0x03C5); 
  IoWrite8 (0x03C5, 0x01);
  IoWrite8 (0x03D4, 0x58);
  IoRead8 (0x03D5); 
  IoWrite8 (0x03D5, 0xD3);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
BiosVideoGraphicsOutputBlt (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL       *This,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL      *BltBuffer, OPTIONAL
  IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION  BltOperation,
  IN  UINTN                              SourceX,
  IN  UINTN                              SourceY,
  IN  UINTN                              DestinationX,
  IN  UINTN                              DestinationY,
  IN  UINTN                              Width,
  IN  UINTN                              Height,
  IN  UINTN                              Delta
  )
/*++

Routine Description:

  Graphics Output protocol instance to block transfer for VBE device

Arguments:

  This          - Pointer to Graphics Output protocol instance
  BltBuffer     - The data to transfer to screen
  BltOperation  - The operation to perform
  SourceX       - The X coordinate of the source for BltOperation
  SourceY       - The Y coordinate of the source for BltOperation
  DestinationX  - The X coordinate of the destination for BltOperation
  DestinationY  - The Y coordinate of the destination for BltOperation
  Width         - The width of a rectangle in the blt rectangle in pixels
  Height        - The height of a rectangle in the blt rectangle in pixels
  Delta         - Not used for EfiBltVideoFill and EfiBltVideoToVideo operation.
                  If a Delta of 0 is used, the entire BltBuffer will be operated on.
                  If a subrectangle of the BltBuffer is used, then Delta represents
                  the number of bytes in a row of the BltBuffer.

Returns:

  EFI_INVALID_PARAMETER - Invalid parameter passed in
  EFI_SUCCESS - Blt operation success

--*/
{
  BIOS_VIDEO_DEV  *BiosVideoPrivate;
  EFI_TPL         OriginalTPL;
  UINTN           DstY;
  UINTN           SrcY;
  UINTN           BytesPerScanLine;
  UINTN           BytesPerLine;
  UINTN           Index;

  //
  // Check parameters
  //
  if (This == NULL || ((UINTN) BltOperation) >= EfiGraphicsOutputBltOperationMax) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width == 0 || Height == 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get the private context
  //
  BiosVideoPrivate  = BIOS_VIDEO_DEV_FROM_GRAPHICS_OUTPUT_THIS (This);

  if (BltOperation == EfiBltVideoToBltBuffer) {
    //
    // Video to BltBuffer: Source is Video, destination is BltBuffer
    //
    if (SourceY + Height > BiosVideoPrivate->ModeInfo.VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (SourceX + Width > BiosVideoPrivate->ModeInfo.HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  } else {
    //
    // BltBuffer to Video: Source is BltBuffer, destination is Video
    //
    if (DestinationY + Height > BiosVideoPrivate->ModeInfo.VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (DestinationX + Width > BiosVideoPrivate->ModeInfo.HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  }

  BytesPerScanLine = BiosVideoPrivate->ModeInfo.PixelsPerScanLine * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  BytesPerLine     = Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);

  //
  // If Delta is zero, then the entire BltBuffer is being used, so Delta
  // is the number of bytes in each row of BltBuffer.  Since BltBuffer is Width pixels size,
  // the number of bytes in each row can be computed.
  //
  if (Delta == 0) {
    Delta = Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  }
  //
  // We have to raise to TPL Notify, so we make an atomic write the frame buffer.
  // We would not want a timer based event (Cursor, ...) to come in while we are
  // doing this operation.
  //
  OriginalTPL = gBS->RaiseTPL (TPL_NOTIFY);

  switch (BltOperation) {
  case EfiBltVideoToBltBuffer:
    for (SrcY = SourceY, DstY = DestinationY; DstY < (Height + DestinationY); SrcY++, DstY++) {
      CopyMem (
        (UINT8 *)BltBuffer + (DstY * Delta) + (DestinationX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)), 
        (UINT8 *)BiosVideoPrivate->Mode.FrameBufferBase + (SrcY * BytesPerScanLine) + (SourceX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)), 
        BytesPerLine
        );
    }
    break;
  case EfiBltVideoToVideo:
    for (Index = 0; Index < Height; Index++) {
      if (DestinationY <= SourceY) {
        SrcY  = SourceY + Index;
        DstY  = DestinationY + Index;
      } else {
        SrcY  = SourceY + Height - Index - 1;
        DstY  = DestinationY + Height - Index - 1;
      }

      CopyMem (
        (UINT8 *)BiosVideoPrivate->Mode.FrameBufferBase + (DstY * BytesPerScanLine) + (DestinationX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)), 
        (UINT8 *)BiosVideoPrivate->Mode.FrameBufferBase + (SrcY * BytesPerScanLine) + (SourceX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)), 
        BytesPerLine
        );
    }
    break;
  case EfiBltVideoFill:
    SetMem32 (
      (UINT8 *)BiosVideoPrivate->Mode.FrameBufferBase + (DestinationY * BytesPerScanLine) + (DestinationX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)),
      BytesPerLine,
      *(UINT32 *)BltBuffer
      );
    for (DstY = DestinationY + 1; DstY < (Height + DestinationY); DstY++) {
      CopyMem (
        (UINT8 *)BiosVideoPrivate->Mode.FrameBufferBase + (DstY * BytesPerScanLine) + (DestinationX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)),
        (UINT8 *)BiosVideoPrivate->Mode.FrameBufferBase + (DestinationY * BytesPerScanLine) + (DestinationX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)),
        BytesPerLine
        );
    }
    break;
  case EfiBltBufferToVideo:
    for (SrcY = SourceY, DstY = DestinationY; SrcY < (Height + SourceY); SrcY++, DstY++) {
      CopyMem (
        (UINT8 *)BiosVideoPrivate->Mode.FrameBufferBase + (DstY * BytesPerScanLine) + (DestinationX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)), 
        (UINT8 *)BltBuffer + (SrcY * Delta) + (SourceX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)), 
        BytesPerLine
        );
    }
    break;
  }

  gBS->RestoreTPL (OriginalTPL);

  return EFI_SUCCESS;
}
