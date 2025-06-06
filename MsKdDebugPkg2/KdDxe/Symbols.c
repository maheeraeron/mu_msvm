/**@file Symbols.c

This file contains support functions to load symbols in the kernel debugger.

Copyright (c) 2018, Microsoft Corporation

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

#include <Guid/EventGroup.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/DebugAgentLib.h>
#include <Protocol/KdDebugPrint.h>
#include <KdTypes.h>
#include <Library/KdTransportLib.h>
#include <Library/KdProtocolLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Protocol/Cpu.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/FirmwareVolume2.h>
#include "KdDxe.h"
#include "Symbols.h"

LIST_ENTRY  mModuleList;

#define IMAGE_BUFFER_SIZE  0x10000
UINT8   mImageBuffer[IMAGE_BUFFER_SIZE];
UINT64  mImageBufferEnd;
UINT64  mImageListCount;
UINT64  mImageNextFreeEntry;

VOID       *mImageEventRegistration;
EFI_EVENT  mImageEvent;

/**
  This routine is called whenever a gEfiLoadedImageProtocolGuid is registered.
  KdDxe uses this event to gather symbol information.

  @param  Event       Not used.
  @param  Context     Not used.

**/
VOID
EFIAPI
KdDxeLoadedImageCallback (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  )
{
  UINTN                      BufferSize;
  LDR_DATA_TABLE_ENTRY       *Entry;
  EFI_HANDLE                 Handle;
  EFI_LOADED_IMAGE_PROTOCOL  *LoadedImage;
  EFI_STATUS                 Status;

  BufferSize = sizeof (EFI_HANDLE);

  //
  // Call LocateHandle until new ones are not found on this event registration.
  //
  while (TRUE) {
    Status = gBS->LocateHandle (
                    ByRegisterNotify,
                    NULL,
                    mImageEventRegistration,
                    &BufferSize,
                    &Handle
                    );

    if (EFI_ERROR (Status)) {
      break;
    }

    Status = gBS->HandleProtocol (
                    Handle,
                    &gEfiLoadedImageProtocolGuid,
                    (VOID **)&LoadedImage
                    );

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: HandleProtocol failed, Status = %r\n", __FUNCTION__, Status));
      break;
    }

    //
    // Add the symbol information.
    //

    KdDxeAddSymbolInfo (LoadedImage, &Entry);

    //
    // Inform the debugger about the loaded module.
    //

    KdDxeSymbolNotification (Entry, 3);     // BREAKPOINT_LOAD_SYMBOLS == 3.  Fix the headers.
  }

  return;
}

/**
  This routine is called to initialize KdDxe's symbol support.

**/
VOID
KdDxeSymbolInitialize (
  )
{
  InitializeListHead (&mModuleList);
  mImageEventRegistration = NULL;
  ZeroMem (&mImageBuffer, sizeof (mImageBuffer));
  mImageNextFreeEntry = (UINT64)&mImageBuffer[0];
  mImageBufferEnd     = ((UINT64)&mImageBuffer[IMAGE_BUFFER_SIZE] - 1);

  //
  // BUGBUG: We'll need to close this event on unload.
  //

  mImageEvent = EfiCreateProtocolNotifyEvent (
                  &gEfiLoadedImageProtocolGuid,
                  TPL_CALLBACK,
                  KdDxeLoadedImageCallback,
                  NULL,
                  &mImageEventRegistration
                  );

  return;
}

/**
  Finds the base module name for an image.

  @param  Image            Loaded image information.

  @retval  UINT16 *        Name of the driver.
  @retval  NULL            Driver name not found.

**/
UINT16 *
KdDxeGetModuleName (
  EFI_LOADED_IMAGE_PROTOCOL  *Image
  )
{
  EFI_STATUS                         Status;
  EFI_DEVICE_PATH_PROTOCOL           *DevPathNode;
  EFI_DEVICE_PATH_PROTOCOL           *AlignedDevPathNode;
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH  *FvFilePath;
  VOID                               *Buffer;
  UINTN                              BufferSize;
  UINT32                             AuthenticationStatus;
  EFI_GUID                           *NameGuid;
  EFI_FIRMWARE_VOLUME2_PROTOCOL      *Fv2;

  Fv2        = NULL;
  Buffer     = NULL;
  BufferSize = 0;

  if (Image->FilePath == NULL) {
    return NULL;
  }

  DevPathNode = Image->FilePath;

  while (!IsDevicePathEnd (DevPathNode)) {
    //
    // Make sure device path node is aligned when accessing it's FV Name Guid field.
    //
    AlignedDevPathNode = AllocateCopyPool (DevicePathNodeLength (DevPathNode), DevPathNode);

    //
    // Find the Fv File path
    //
    NameGuid = EfiGetNameGuidFromFwVolDevicePathNode ((MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *)AlignedDevPathNode);
    if (NameGuid != NULL) {
      FvFilePath = (MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *)AlignedDevPathNode;
      Status     = gBS->HandleProtocol (
                          Image->DeviceHandle,
                          &gEfiFirmwareVolume2ProtocolGuid,
                          (VOID **)&Fv2
                          );
      //
      // Locate Image EFI UI section to get the image name.
      //
      if (!EFI_ERROR (Status)) {
        Status = Fv2->ReadSection (
                        Fv2,
                        &FvFilePath->FvFileName,
                        EFI_SECTION_USER_INTERFACE,
                        0,
                        &Buffer,
                        &BufferSize,
                        &AuthenticationStatus
                        );
        if (!EFI_ERROR (Status)) {
          FreePool (AlignedDevPathNode);
          break;
        }

        Buffer = NULL;
      }
    }

    FreePool (AlignedDevPathNode);

    //
    // Next device path node
    //
    DevPathNode = NextDevicePathNode (DevPathNode);
  }

  return Buffer;
}

/**
  Adds a module to the loaded module list by image base.

  @param  Image            Loaded image protocol information.
  @param  AddedEntry       A pointer to the entry to the module list.

  @retval EFI_SUCCESS      On success.
  @retval EFI_STATUS       On failure.

**/
EFI_STATUS
KdDxeAddSymbolInfo (
  EFI_LOADED_IMAGE_PROTOCOL  *Image,
  LDR_DATA_TABLE_ENTRY       **AddedEntry
  )
{
  LDR_DATA_TABLE_ENTRY  *entry;
  UINT16                *moduleName;
  UINTN                 moduleNameLength = 0;
  UINTN                 nameByteCount    = 0;

  moduleName = KdDxeGetModuleName (Image);
  if (moduleName != NULL) {
    moduleNameLength = StrLen (moduleName);
    nameByteCount    = moduleNameLength * sizeof (UINT16) + sizeof (L".efi");
  }

  //
  // N.B.
  //   The string buffer for BaseDllName is directly after the LDR_DATA_TABLE_ENTRY
  //   struct.  If this changes EfiKdAddUnloadedModuleInfo will also need to change.
  //

  //
  // If there is not enough image data buffer remaining, return OUT_OF_RESOURCES.
  //

  if ((mImageNextFreeEntry + sizeof (LDR_DATA_TABLE_ENTRY) + nameByteCount) > mImageBufferEnd) {
    *AddedEntry = NULL;
    return EFI_OUT_OF_RESOURCES;
  }

  entry                = (LDR_DATA_TABLE_ENTRY *)mImageNextFreeEntry;
  mImageNextFreeEntry += (sizeof (LDR_DATA_TABLE_ENTRY) + nameByteCount);
  mImageNextFreeEntry  = (UINT64)ALIGN_POINTER (mImageNextFreeEntry, sizeof (UINT64));

  entry->SizeOfImage = (UINT32)Image->ImageSize;
  entry->DllBase     = (UINTN)Image->ImageBase;
  entry->EntryPoint  = 0;

  //
  // Checksum is not provided in the EFI image context
  // get it from the PE header directly.
  //
  // BUGBUG: Checksum stuff removed.
  //

  if (nameByteCount != 0) {
    entry->BaseDllName.Buffer = (UINT16 *)(entry + 1);
    UnicodeSPrint (
      entry->BaseDllName.Buffer,
      nameByteCount,
      L"%s.efi",
      moduleName
      );

    entry->BaseDllName.Length        = (UINT16)nameByteCount - sizeof (UINT16);
    entry->BaseDllName.MaximumLength = (UINT16)nameByteCount;
  }

  entry->LoadCount = 1;
  InsertTailList (&mModuleList, &entry->InLoadOrderLinks);
  mImageListCount++;
  *AddedEntry = entry;

  return EFI_SUCCESS;
}

/**
  Notifies the debugger about an image load or unload.

  @param  Image         Loaded image protocol information.
  @param  Operation     BREAKPOINT_LOAD_SYMBOLS or BREAKPOINT_UNLOAD_SYMBOLS

**/
VOID
KdDxeSymbolNotification (
  LDR_DATA_TABLE_ENTRY  *Image,
  UINT8                 Operation
  )
{
  BOOLEAN          BreakIn;
  UINT8            buffer[128];
  KD_SYMBOLS_INFO  symbolInfo;
  UNICODE_STRING   *baseDllName;
  KD_STRING        string;

  ZeroMem (&buffer[0], sizeof (buffer));
  string.Buffer        = buffer;
  string.MaximumLength = ARRAY_SIZE (buffer);
  string.Length        = 0;

  //
  // The module entry stores the PDB name as Unicode
  // If it's valid, convert the name to ASCII for the debug service API.
  //
  baseDllName = &Image->BaseDllName;
  if (baseDllName->Length > 0) {
    string.Length = (UINT16)AsciiSPrintUnicodeFormat (
                              (CHAR8 *)string.Buffer,
                              string.MaximumLength,
                              L"%s",
                              baseDllName->Buffer
                              );
  }

  symbolInfo.SizeOfImage = Image->SizeOfImage;
  symbolInfo.CheckSum    = Image->CheckSum;
  symbolInfo.BaseOfDll   = (VOID *)Image->DllBase;
  symbolInfo.ProcessId   = (UINT64)-1;

  DebugService2 (&string, &symbolInfo, Operation);

  BreakIn = KdProtocolPollBreakIn ();
  if (BreakIn != FALSE) {
    KdDxeKdBreakPointWithStatus (0);
  }
}
