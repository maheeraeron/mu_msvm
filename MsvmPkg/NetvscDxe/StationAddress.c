/** @file
    Implementation of reading the MAC address of a network adapter.

    Copyright (c) 2004 - 2007, Intel Corporation. All rights reserved.<BR>
    Copyright (c) Microsoft Corporation.
    Licensed under the BSD-2-Clause-Patent license.
**/

#include "Snp.h"


EFI_STATUS
GetStnAddr(
    _In_ SNP_DRIVER *Snp
    )
/**

Routine Description:

    Read the MAC address of the NIC_DATA_INSTANCE and update the mode structure 
    with the address. 

Arguments:

    Snp         Pointer to snp driver structure.

Return Value:

    EFI_SUCCESS       The MAC address of the NIC is read successfully.

**/
{
    //
    // Set new station address in SNP->Mode structure and return success.
    //
    CopyMem(
        &(Snp->Mode.CurrentAddress),
        &(Snp->AdapterContext->NicInfo.CurrentNodeAddress),
        Snp->Mode.HwAddressSize);

    CopyMem(
        &Snp->Mode.BroadcastAddress,
        &Snp->AdapterContext->NicInfo.BroadcastNodeAddress,
        Snp->Mode.HwAddressSize);

    CopyMem(
        &Snp->Mode.PermanentAddress,
        &Snp->AdapterContext->NicInfo.PermNodeAddress,
        Snp->Mode.HwAddressSize);

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
SnpStationAddress(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This,
    _In_ BOOLEAN                     Reset,
    _In_opt_ EFI_MAC_ADDRESS         *New
    )
/**

Routine Description:

    Modifies or resets the current station address, if supported.

    This function is NOT SUPPORTED.

Arguments:

    This  A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

    Reset Flag used to reset the station address to the network interface's 
               permanent address.

    New   New station address to be used for the network interface.

Return Value:

    EFI_NOT_STARTED       The Simple Network Protocol interface has not been 
                                started by calling Start().

    EFI_DEVICE_ERROR      The Simple Network Protocol interface has not 
                                been initialized by calling Initialize().

    EFI_DEVICE_ERROR      An error occurred attempting to set the new 
                                station address.

    EFI_UNSUPPORTED       The NIC does not support changing the network 
                                interface's station address.

**/
{
    SNP_DRIVER  *snpDriver;
    EFI_STATUS  status;
    EFI_TPL     OldTpl;

    //
    // Check for invalid parameter combinations.
    //
    if (This == NULL)
    {
        status = EFI_INVALID_PARAMETER;
        goto Exit;
    }

    snpDriver = EFI_SIMPLE_NETWORK_DEV_FROM_THIS(This);

    OldTpl = gBS->RaiseTPL(TPL_CALLBACK);

    //
    // Return error if the SNP is not initialized.
    //
    switch (snpDriver->Mode.State)
    {
    case EfiSimpleNetworkInitialized:
        //
        // Setting CurrentAddress is not supported
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

    gBS->RestoreTPL(OldTpl);

Exit:

    return status;
}
