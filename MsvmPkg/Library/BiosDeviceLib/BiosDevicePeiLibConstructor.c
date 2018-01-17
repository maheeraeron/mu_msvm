/*++

    Copyright (c) Microsoft Corporation

Module Name:

    BiosDevicePeiLibConstructor.c
    
Abstract:

    PEI library constructor for PEI version of BiosDeviceLib

--*/

#include <PiPei.h>

extern void SetupBaseAddress();

EFI_STATUS
EFIAPI
BiosDevicePeiLibConstructor (
  IN       EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  )
{
    SetupBaseAddress();
    return EFI_SUCCESS;
}

