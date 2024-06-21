/*++

Copyright (c) Microsoft Corporation

Module Name:

    Hob.h

Abstract:

    Hob-building functionality.

Author:

    Rich Yampell (richyam) 8-Jun-2012

--*/

#pragma once

#include <PiPei.h>
#include <EfiNt.h>

void
HobAddMmioRange(
    _In_ EFI_PHYSICAL_ADDRESS BaseAddress,
    _In_ UINT64               Size
    );


void
HobAddMemoryRange(
    _Inout_ PPLATFORM_INIT_CONTEXT  Context,
    _In_ EFI_PHYSICAL_ADDRESS       BaseAddress,
    _In_ UINT64                     Size
    );


void
HobAddPersistentMemoryRange(
    _In_ EFI_PHYSICAL_ADDRESS BaseAddress,
    _In_ UINT64               Size
    );

void
HobAddSpecificPurposeMemoryRange(
    _In_ EFI_PHYSICAL_ADDRESS BaseAddress,
    _In_ UINT64               Size
    );

void
HobAddReservedMemoryRange(
    _In_ EFI_PHYSICAL_ADDRESS BaseAddress,
    _In_ UINT64               Size
    );


void
HobAddUntestedMemoryRange(
    _Inout_ PPLATFORM_INIT_CONTEXT  Context,
    _In_ EFI_PHYSICAL_ADDRESS BaseAddress,
    _In_ UINT64               Size
    );


void
HobAddAllocatedMemoryRange(
    _In_ EFI_PHYSICAL_ADDRESS BaseAddress,
    _In_ UINT64               Size
    );


void
HobAddFvMemoryRange(
    _In_ EFI_PHYSICAL_ADDRESS BaseAddress,
    _In_ UINT64               Size
    );


void
HobAddIoRange(
    _In_ EFI_PHYSICAL_ADDRESS BaseAddress,
    _In_ UINT64               Size
    );


void
HobAddCpu(
    _In_ UINT8 SizeOfMemorySpace,
    _In_ UINT8 SizeOfIoSpace
    );


void
HobAddGuidData(
    _In_ EFI_GUID* Guid,
    _In_ VOID*     Data,
    _In_ UINTN     DataSize
  );
