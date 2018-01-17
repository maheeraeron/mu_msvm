/*++

Copyright (c) Microsoft Corporation

Module Name:

    Config.h

Abstract:

    Worker process configuration related data and functions.

--*/

#pragma once

//
// Configuration data.
//
extern UINT8 gPhysicalAddressWidth;

//
// Functions
//
UEFI_CONFIG_HEADER*
GetStartOfConfigBlob(
    VOID
    );

EFI_STATUS
GetConfiguration(
    IN CONST EFI_PEI_SERVICES** PeiServices
    );
