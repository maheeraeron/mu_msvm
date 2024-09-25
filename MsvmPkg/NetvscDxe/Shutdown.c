/** @file
    Implementation of shutting down a network adapter.

Copyright (c) 2004 - 2018, Intel Corporation. All rights reserved.<BR>
Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Snp.h"

/**
  Call UNDI to shut down the interface.

  @param  Snp   Pointer to snp driver structure.

  @retval EFI_SUCCESS        UNDI is shut down successfully.
  @retval EFI_DEVICE_ERROR   UNDI could not be shut down.

**/
EFI_STATUS
SnpShutdownImpl(
    IN  SNP_DRIVER *Snp
    )
{
    EFI_STATUS status;

    status = NetvscShutdown(&Snp->AdapterContext->NicInfo);

    if (EFI_ERROR(status))
    {
        status = EFI_DEVICE_ERROR;
    }

    Snp->Mode.State = EfiSimpleNetworkStarted;

    return status;
}


/**
  Resets a network adapter and leaves it in a state that is safe for another
  driver to initialize.

  This function releases the memory buffers assigned in the Initialize() call.
  Pending transmits and receives are lost, and interrupts are cleared and disabled.
  After this call, only the Initialize() and Stop() calls may be used. If the
  network interface was successfully shutdown, then EFI_SUCCESS will be returned.
  If the driver has not been initialized, EFI_DEVICE_ERROR will be returned.

  @param  This  A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

  @retval EFI_SUCCESS           The network interface was shutdown.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER This parameter was NULL or did not point to a valid
                                EFI_SIMPLE_NETWORK_PROTOCOL structure.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.

**/
EFI_STATUS
EFIAPI
SnpShutdown(
    IN  EFI_SIMPLE_NETWORK_PROTOCOL *This
    )
{
    SNP_DRIVER  *snpDriver;
    EFI_STATUS  status;
    EFI_TPL     oldTpl;

    //
    // Get pointer to SNP driver instance from *This.
    //
    if (This == NULL)
    {
        status = EFI_INVALID_PARAMETER;
        goto InvalidParamExit;
    }

    snpDriver = EFI_SIMPLE_NETWORK_DEV_FROM_THIS(This);

    oldTpl = gBS->RaiseTPL(TPL_CALLBACK);

    //
    // Return error if the SNP is not initialized.
    //
    switch (snpDriver->Mode.State)
    {
    case EfiSimpleNetworkInitialized:
        break;

    case EfiSimpleNetworkStopped:
        status = EFI_NOT_STARTED;
        goto Exit;

    default:
        status = EFI_DEVICE_ERROR;
        goto Exit;
    }

    status = SnpShutdownImpl(snpDriver);

    snpDriver->Mode.ReceiveFilterSetting  = 0;

    snpDriver->Mode.MCastFilterCount      = 0;
    snpDriver->Mode.ReceiveFilterSetting  = 0;
    ZeroMem(snpDriver->Mode.MCastFilter, sizeof snpDriver->Mode.MCastFilter);
    CopyMem(
        &snpDriver->Mode.CurrentAddress,
        &snpDriver->Mode.PermanentAddress,
        sizeof(EFI_MAC_ADDRESS));

    gBS->CloseEvent(snpDriver->Snp.WaitForPacket);

Exit:

    gBS->RestoreTPL(oldTpl);

InvalidParamExit:

    return status;
}
