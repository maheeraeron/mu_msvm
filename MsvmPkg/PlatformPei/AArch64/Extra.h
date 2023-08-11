/*++

Copyright (c) Microsoft Corporation

Module Name:

    Extra.h

Abstract:

    Mmu setup asm interface for ARM64

--*/

#pragma once

//
// Asm function
//

VOID
EFIAPI
ConfigureCachesAndMmu(
    IN VOID* TranslationTable,
    IN UINTN TCR,
    IN UINTN MAIR
    );