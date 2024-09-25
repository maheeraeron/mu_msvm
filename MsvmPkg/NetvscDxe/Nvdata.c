/** @file
   Implementation of reading and writing operations on the NVRAM device
   attached to a network interface.

Copyright (c) 2004 - 2018, Intel Corporation. All rights reserved.<BR>
Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Snp.h"

/**
  Performs read and write operations on the NVRAM device attached to a network
  interface.

  This function performs read and write operations on the NVRAM device attached
  to a network interface. If ReadWrite is TRUE, a read operation is performed.
  If ReadWrite is FALSE, a write operation is performed. Offset specifies the
  byte offset at which to start either operation. Offset must be a multiple of
  NvRamAccessSize , and it must have a value between zero and NvRamSize.
  BufferSize specifies the length of the read or write operation. BufferSize must
  also be a multiple of NvRamAccessSize, and Offset + BufferSize must not exceed
  NvRamSize.
  If any of the above conditions is not met, then EFI_INVALID_PARAMETER will be
  returned.
  If all the conditions are met and the operation is "read," the NVRAM device
  attached to the network interface will be read into Buffer and EFI_SUCCESS
  will be returned. If this is a write operation, the contents of Buffer will be
  used to update the contents of the NVRAM device attached to the network
  interface and EFI_SUCCESS will be returned.

  It does the basic checking on the input parameters and retrieves snp structure
  and then calls the read_nvdata() call which does the actual reading

  @param This       A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
  @param ReadWrite  TRUE for read operations, FALSE for write operations.
  @param Offset     Byte offset in the NVRAM device at which to start the read or
                    write operation. This must be a multiple of NvRamAccessSize
                    and less than NvRamSize. (See EFI_SIMPLE_NETWORK_MODE)
  @param BufferSize The number of bytes to read or write from the NVRAM device.
                    This must also be a multiple of NvramAccessSize.
  @param Buffer     A pointer to the data buffer.

  @retval EFI_SUCCESS           The NVRAM access was performed.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER One or more of the following conditions is TRUE:
                                * The This parameter is NULL
                                * The This parameter does not point to a valid
                                  EFI_SIMPLE_NETWORK_PROTOCOL  structure
                                * The Offset parameter is not a multiple of
                                  EFI_SIMPLE_NETWORK_MODE.NvRamAccessSize
                                * The Offset parameter is not less than
                                  EFI_SIMPLE_NETWORK_MODE.NvRamSize
                                * The BufferSize parameter is not a multiple of
                                  EFI_SIMPLE_NETWORK_MODE.NvRamAccessSize
                                * The Buffer parameter is NULL
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network
                                interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network
                                interface.

**/
EFI_STATUS
EFIAPI
SnpNvData(
    IN      EFI_SIMPLE_NETWORK_PROTOCOL *This,
    IN      BOOLEAN                     ReadWrite,
    IN      UINTN                       Offset,
    IN      UINTN                       BufferSize,
    IN OUT  VOID                        *Buffer
    )
{
    SNP_DRIVER  *snpDriver;
    EFI_TPL     oldTpl;
    EFI_STATUS  status;

    //
    // Get pointer to SNP driver instance from *This.
    //
    if (This == NULL)
    {
        status = EFI_INVALID_PARAMETER;
        goto Exit;
    }

    snpDriver = EFI_SIMPLE_NETWORK_DEV_FROM_THIS(This);

    oldTpl = gBS->RaiseTPL(TPL_CALLBACK);

    //
    // Return error if the SNP is not initialized.
    //
    switch (snpDriver->Mode.State)
    {
    case EfiSimpleNetworkInitialized:
        //
        // Nvdata is NOT SUPPORTED.
        //
        status = EFI_UNSUPPORTED;
        break;

    case EfiSimpleNetworkStopped:
        status = EFI_NOT_STARTED;
        break;

    default:
        status = EFI_DEVICE_ERROR;
        break;
    }

    gBS->RestoreTPL(oldTpl);

Exit:

    return status;
}
