/*++

Copyright (c) Microsoft Corporation

Module Name:

    Mmu.h

Abstract:

    Mmu setup for ARM64

--*/

#pragma once

//
// Functions
//
EFI_STATUS
EFIAPI
ConfigureMmu(
    IN  UINT64  MaxAddress
    );
