/** @file
     Implementation of initializing a network adapter.

Copyright (c) 2004 - 2018, Intel Corporation. All rights reserved.<BR>
Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Snp.h"

/**
  Call UNDI to initialize the interface.

  @param  Snp                   Pointer to snp driver structure.
  @param  CableDetectFlag       Do/don't detect the cable (depending on what
                                undi supports).

  @retval EFI_SUCCESS           UNDI is initialized successfully.
  @retval EFI_DEVICE_ERROR      UNDI could not be initialized.
  @retval Other                 Other errors as indicated.

**/
EFI_STATUS
SnpInitImpl(
    IN  SNP_DRIVER *Snp
    )
{
    EFI_STATUS status;

    status = NetvscInit(&Snp->AdapterContext->NicInfo);

    if (status == EFI_SUCCESS)
    {
        Snp->Mode.State = EfiSimpleNetworkInitialized;
    }
    else
    {
        status = EFI_DEVICE_ERROR;
    }

    if (Snp->Mode.MediaPresentSupported)
    {
        Snp->Mode.MediaPresent = Snp->AdapterContext->NicInfo.MediaPresent;
    }

    return status;
}


/**
  Resets a network adapter and allocates the transmit and receive buffers
  required by the network interface; optionally, also requests allocation of
  additional transmit and receive buffers.

  This function allocates the transmit and receive buffers required by the network
  interface. If this allocation fails, then EFI_OUT_OF_RESOURCES is returned.
  If the allocation succeeds and the network interface is successfully initialized,
  then EFI_SUCCESS will be returned.

  @param This               A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

  @param ExtraRxBufferSize  The size, in bytes, of the extra receive buffer space
                            that the driver should allocate for the network interface.
                            Some network interfaces will not be able to use the
                            extra buffer, and the caller will not know if it is
                            actually being used.
  @param ExtraTxBufferSize  The size, in bytes, of the extra transmit buffer space
                            that the driver should allocate for the network interface.
                            Some network interfaces will not be able to use the
                            extra buffer, and the caller will not know if it is
                            actually being used.

  @retval EFI_SUCCESS           The network interface was initialized.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_OUT_OF_RESOURCES  There was not enough memory for the transmit and
                                receive buffers.
  @retval EFI_INVALID_PARAMETER This parameter was NULL or did not point to a valid
                                EFI_SIMPLE_NETWORK_PROTOCOL structure.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       The increased buffer size feature is not supported.

**/
EFI_STATUS
EFIAPI
SnpInitialize(
    IN  EFI_SIMPLE_NETWORK_PROTOCOL *This,
    IN  UINTN                       ExtraRxBufferSize,
    IN  UINTN                       ExtraTxBufferSize
    )
{
    EFI_STATUS  efiStatus;
    SNP_DRIVER  *snpDriver;
    EFI_TPL     oldTpl;

    if (This == NULL)
    {
        efiStatus = EFI_INVALID_PARAMETER;
        goto InvalidParamExit;
    }

    snpDriver = EFI_SIMPLE_NETWORK_DEV_FROM_THIS(This);

    oldTpl = gBS->RaiseTPL(TPL_CALLBACK);

    switch (snpDriver->Mode.State)
    {
    case EfiSimpleNetworkStarted:
        break;

    case EfiSimpleNetworkStopped:
        efiStatus = EFI_NOT_STARTED;
        goto Exit;

    default:
        efiStatus = EFI_DEVICE_ERROR;
        goto Exit;
    }

    efiStatus = gBS->CreateEvent(
        EVT_NOTIFY_WAIT,
        TPL_NOTIFY,
        &SnpWaitForPacketNotify,
        snpDriver,
        &snpDriver->Snp.WaitForPacket);

    if (EFI_ERROR(efiStatus))
    {
        snpDriver->Snp.WaitForPacket = NULL;
        efiStatus = EFI_DEVICE_ERROR;
        goto Exit;
    }

    snpDriver->Mode.MCastFilterCount      = 0;
    snpDriver->Mode.ReceiveFilterSetting  = 0;
    ZeroMem(snpDriver->Mode.MCastFilter, sizeof snpDriver->Mode.MCastFilter);
    CopyMem(
        &snpDriver->Mode.CurrentAddress,
        &snpDriver->Mode.PermanentAddress,
        sizeof (EFI_MAC_ADDRESS));

    efiStatus = SnpInitImpl(snpDriver);

    if (EFI_ERROR(efiStatus))
    {
        gBS->CloseEvent(snpDriver->Snp.WaitForPacket);
    }

Exit:

    gBS->RestoreTPL (oldTpl);

InvalidParamExit:

    return efiStatus;
}
