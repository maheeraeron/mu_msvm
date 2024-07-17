/** @file
  Mmu setup asm interface for ARM64

  Copyright (c) Microsoft Corporation.
  Licensed under the BSD-2-Clause-Patent license.

**/

#pragma once

//
// Asm function
//

VOID
EFIAPI
ConfigureCachesAndMmu(
    IN  VOID* TranslationTable,
        UINTN TCR,
        UINTN MAIR
    );