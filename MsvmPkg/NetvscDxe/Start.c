/** @file
    Implementation of starting a network adapter.

Copyright (c) 2004 - 2018, Intel Corporation. All rights reserved.<BR>
Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Snp.h"

/**
  Change the state of a network interface from "stopped" to "started."

  This function starts a network interface. If the network interface successfully
  starts, then EFI_SUCCESS will be returned.

  @param  This                   A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

  @retval EFI_SUCCESS            The network interface was started.
  @retval EFI_ALREADY_STARTED    The network interface is already in the started state.
  @retval EFI_INVALID_PARAMETER  This parameter was NULL or did not point to a valid
                                 EFI_SIMPLE_NETWORK_PROTOCOL structure.
  @retval EFI_DEVICE_ERROR       The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED        This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
SnpStart(
    IN  EFI_SIMPLE_NETWORK_PROTOCOL *This
    )
{
    SNP_DRIVER  *snpDriver;
    EFI_STATUS  status = EFI_SUCCESS;
    EFI_TPL     oldTpl;

    if (This == NULL)
    {
        status = EFI_INVALID_PARAMETER;
        goto InvalidParamExit;
    }

    snpDriver = EFI_SIMPLE_NETWORK_DEV_FROM_THIS(This);

    oldTpl = gBS->RaiseTPL(TPL_CALLBACK);

    switch (snpDriver->Mode.State)
    {
    case EfiSimpleNetworkStopped:
        break;

    case EfiSimpleNetworkStarted:
    case EfiSimpleNetworkInitialized:
        status = EFI_ALREADY_STARTED;
        goto Exit;

    default:
        status = EFI_DEVICE_ERROR;
        goto Exit;
    }

    //
    // Set simple network state to Started and return success.
    //
    snpDriver->Mode.State = EfiSimpleNetworkStarted;

    snpDriver->Mode.MCastFilterCount = 0;

Exit:

    gBS->RestoreTPL(oldTpl);

InvalidParamExit:

    return status;
}
