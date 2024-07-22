/** @file
    Implementation of reading and writing operations on the NVRAM device
    attached to a network interface.

    Copyright (c) 2004 - 2009, Intel Corporation. All rights reserved.<BR>
    Copyright (c) Microsoft Corporation.
    Licensed under the BSD-2-Clause-Patent license.
**/

#include "Snp.h"


EFI_STATUS
EFIAPI
SnpNvData(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This,
    _In_ BOOLEAN                     ReadWrite,
    _In_ UINTN                       Offset,
    _In_ UINTN                       BufferSize,
    _Inout_ VOID                    *Buffer
    )
/**

Routine Description:

    Performs read and write operations on the NVRAM device attached to a network 
    interface.

    This function is NOT SUPPORTED.  

Arguments:

    This       A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

    ReadWrite  TRUE for read operations, FALSE for write operations.

    Offset     Byte offset in the NVRAM device at which to start the read or 
                    write operation. This must be a multiple of NvRamAccessSize 
                    and less than NvRamSize. (See EFI_SIMPLE_NETWORK_MODE)  

    BufferSize The number of bytes to read or write from the NVRAM device. 
                    This must also be a multiple of NvramAccessSize.

    Buffer     A pointer to the data buffer.

Return Value:

    EFI_NOT_STARTED       The network interface has not been started.

    EFI_DEVICE_ERROR      The command could not be sent to the network 
                                interface.

    EFI_UNSUPPORTED       This function is not supported by the network
                                interface.

**/
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
