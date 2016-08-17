/** @file
  Boot Logo protocol is used to convey information of Logo dispayed during boot.

Copyright (c) 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under
the terms and conditions of the BSD License that accompanies this distribution.
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _BOOT_LOGO_H_
#define _BOOT_LOGO_H_

#include <Protocol/GraphicsOutput.h>

#define EFI_BOOT_LOGO_PROTOCOL_GUID \
  { \
    0xcdea2bd3, 0xfc25, 0x4c1c, { 0xb9, 0x7c, 0xb3, 0x11, 0x86, 0x6, 0x49, 0x90 } \
  }

//
// Forward reference for pure ANSI compatability
//
typedef struct _EFI_BOOT_LOGO_PROTOCOL  EFI_BOOT_LOGO_PROTOCOL;

/**
  Update information of logo image drawn on screen.

  @param  This           The pointer to the Boot Logo protocol instance.
  @param  BltBuffer      The BLT buffer for logo drawn on screen. If BltBuffer
                         is set to NULL, it indicates that logo image is no
                         longer on the screen.
  @param  DestinationX   X coordinate of destination for the BltBuffer.
  @param  DestinationY   Y coordinate of destination for the BltBuffer.
  @param  Width          Width of rectangle in BltBuffer in pixels.
  @param  Height         Hight of rectangle in BltBuffer in pixels.

  @retval EFI_SUCCESS             The boot logo information was updated.
  @retval EFI_INVALID_PARAMETER   One of the parameters has an invalid value.
  @retval EFI_OUT_OF_RESOURCES    The logo information was not updated due to
                                  insufficient memory resources.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SET_BOOT_LOGO)(
  IN EFI_BOOT_LOGO_PROTOCOL            *This,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL     *BltBuffer       OPTIONAL,
  IN UINTN                             DestinationX,
  IN UINTN                             DestinationY,
  IN UINTN                             Width,
  IN UINTN                             Height
  );


/**
  Returns information about the current boot logo if it is valid.

  @param  This           The pointer to the Boot Logo protocol instance.
  @param  BltBuffer      If supplied, and the boot logo is valid, contains an allocated
                         Blt buffer containing the boot logo.  Callers must free this
                         buffer when no longer needed.
  @param  OffsetX        X offset of the actual logo from the start of the bitmap.
  @param  OffsetY        Y offset of the actual logo from the start of the bitmap.
  @param  Width          Width of logo.
  @param  Height         Height of logo.
  @param  Valid          TRUE if the logo has been set and is valid (not corrupt).
  
  @retval EFI_SUCCESS             The boot logo information was updated.
  @retval EFI_INVALID_PARAMETER   One of the parameters has an invalid value.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_GET_BOOT_LOGO)(
  IN EFI_BOOT_LOGO_PROTOCOL            *This,
  OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL    **BltBuffer       OPTIONAL,
  OUT UINTN                            *OffsetX,
  OUT UINTN                            *OffsetY,
  OUT UINTN                            *Width,
  OUT UINTN                            *Height,
  OUT BOOLEAN                          *Valid
  );

struct _EFI_BOOT_LOGO_PROTOCOL {
  EFI_SET_BOOT_LOGO             SetBootLogo;
  EFI_GET_BOOT_LOGO             GetBootLogoAttributes;
};

extern EFI_GUID gEfiBootLogoProtocolGuid;

#endif
