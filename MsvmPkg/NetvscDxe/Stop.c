/** @file
    Implementation of stopping a network interface.

    Copyright (c) 2004 - 2007, Intel Corporation. All rights reserved.<BR>
    Copyright (c) Microsoft Corporation.
    Licensed under the BSD-2-Clause-Patent license.
**/

#include "Snp.h"

EFI_STATUS
SnpStopImpl(
    _In_ SNP_DRIVER *Snp
    )
/*++

Routine Description:

    Call Netvsc to shut down the interface and destroy all allocated objects

Arguments:

    Snp   Pointer to snp driver structure.

Return Value

    EFI_SUCCESS        vNIC is shut down successfully.

    EFI_DEVICE_ERROR   vNIC could not be shut down.

--*/
{
    EFI_STATUS status = EFI_SUCCESS;

    switch (Snp->Mode.State)
    {
    case EfiSimpleNetworkStarted:
      break;

    case EfiSimpleNetworkStopped:
        status = EFI_NOT_STARTED;
        goto Exit;

    default:
        status = EFI_DEVICE_ERROR;
        goto Exit;
    }

    //
    // Set simple network state to Stopped and return success.
    //
    Snp->Mode.State = EfiSimpleNetworkStopped;

Exit:

    return status;  
}


EFI_STATUS
EFIAPI
SnpStop(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This
    )
/**

Routine Description:

    Changes the state of a network interface from "started" to "stopped."

    This function stops a network interface. This call is only valid if the network
    interface is in the started state. If the network interface was successfully
    stopped, then EFI_SUCCESS will be returned.

Arguments:

    This                    A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL 
                              instance.

Return Value:

    EFI_SUCCESS             The network interface was stopped.

    EFI_NOT_STARTED         The network interface has not been started.

    EFI_INVALID_PARAMETER   This parameter was NULL or did not point to a 
                                  valid EFI_SIMPLE_NETWORK_PROTOCOL structure.

    EFI_DEVICE_ERROR        The command could not be sent to the network 
                                  interface.

**/
{
    SNP_DRIVER  *snpDriver;
    EFI_TPL     oldTpl;
    EFI_STATUS  status = EFI_SUCCESS;

    if (This == NULL)
    {
        status = EFI_INVALID_PARAMETER;
        goto InvalidParamExit;
    }

    snpDriver = EFI_SIMPLE_NETWORK_DEV_FROM_THIS(This);

    oldTpl = gBS->RaiseTPL(TPL_CALLBACK);

    status = SnpStopImpl(snpDriver);

    gBS->RestoreTPL(oldTpl);

InvalidParamExit:

    return status;
}
