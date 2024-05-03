/** @file
  Platform Console routines for showing the Hyper-V diagnostic console

  Copyright (c) Microsoft Corporation.
  Licensed under the BSD-2-Clause-Patent license.
**/
#pragma once

EFI_STATUS
PlatformConsoleInitialize();

VOID
PlatformConsoleBootSummary(
    _In_    EFI_STRING_ID                   Id
    );

//
// Platform String Helpers.
//

EFI_STATUS
PlatformStringInitialize();


CHAR16*
PlatformStringById(
    _In_    EFI_STRING_ID                   Id
    );

UINTN
PlatformStringPrintById(
    _In_    EFI_STRING_ID                   Id,
    ...
    );

UINTN
PlatformStringPrintSById(
    _Out_writes_z_(BufferSize)
            CHAR16                         *StartOfBuffer,
    _In_    UINTN                           BufferSize,
    _In_    EFI_STRING_ID                   Id,
    ...
    );

UINTN
PlatformStringPrint(
    _In_    CHAR16                         *Format,
    ...
    );