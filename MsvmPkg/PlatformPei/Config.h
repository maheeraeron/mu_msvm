/*++

Copyright (c) Microsoft Corporation

Module Name:

    Config.h

Abstract:

    Worker process configuration related data and functions.

--*/

#pragma once

//
// Functions
//
UEFI_CONFIG_HEADER*
GetStartOfConfigBlob(
    VOID
    );

EFI_STATUS
GetConfiguration(
    IN CONST EFI_PEI_SERVICES** PeiServices,
    OUT UINT8* PhysicalAddressWidth
    );

EFI_STATUS
GetIgvmConfigInfo(
    VOID
    );

BOOLEAN
ConfigSetProcessorInfo(
    UEFI_CONFIG_PROCESSOR_INFORMATION *ProcessorInfo
    );

BOOLEAN
ConfigSetUefiConfigFlags(
    UEFI_CONFIG_FLAGS *ConfigFlags
    );
