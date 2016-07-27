/*++

Copyright (c) Microsoft Corporation

Module Name:

    ConfigPpi.h

Abstract:

    Provides the definition of the EFI_CONFIG_PPI interface. This interface
    provides version agnostic access to the virtual machine's configuration from PEIMs.

Author:

    Larry Cleeton (lcleeton) - 30-Apr-2014

--*/

#pragma once

#define EFI_CONFIG_PPI_GUID \
  { 0xc8733a35, 0x38ec, 0x4e58, {0x8c, 0x3d, 0x06, 0x96, 0x75, 0x7c, 0x54, 0x97} }

typedef struct _EFI_CONFIG_PPI  EFI_CONFIG_PPI;

/**
  A function prototype for getting a configuration BOOLEAN value.

  @param PeiServices  An indirect pointer to the PEI Services Table published 
                          by the PEI Foundation.
  @param This         The pointer to local data for the interface.
  @param Value        Recieves the value.

  @retval EFI_SUCCESS       If the data was successfully output.
  @retval EFI_DEVICE_ERROR  If the data could not be output.
  
**/
typedef
BOOLEAN
(EFIAPI *CONFIG_PPI_GET_BOOLEAN)(
    VOID
    );

//
// EFI_CONFIG_PPI provides access to the virtual machine's configuration.
//
struct _EFI_CONFIG_PPI {
  CONFIG_PPI_GET_BOOLEAN GetDebuggerEnabled;
  CONFIG_PPI_GET_BOOLEAN GetTpmEnabled;
};

extern EFI_GUID gEfiConfigPpiGuid;

