/** @file
  Mmu setup for ARM64

  Copyright (c) Microsoft Corporation.
  Licensed under the BSD-2-Clause-Patent license.

**/

#pragma once

//
// Functions
//
EFI_STATUS
EFIAPI
ConfigureMmu(
    UINT64  MaxAddress
    );
