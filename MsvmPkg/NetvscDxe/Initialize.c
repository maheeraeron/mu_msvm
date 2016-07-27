/** @file

    ATTENTION - THIS FILE CONTAINS THIRD PARTY OPEN SOURCE CODE:
                MsvmPkg\MsvmSnpDxe\Initialize.c.
    IT IS CLEARED ONLY FOR LIMITED USE BY WINDOWS CORE HYPER-V FOR THE HYPER-V ROLE IN THE 
    WINDOWS PRODUCT.  DO NOT USE OR SHARE THIS CODE WITHOUT APPROVAL PURSUANT TO THE 
    MICROSOFT OPEN SOURCE  SOFTWARE APPROVAL POLICY. 

    Implementation of initializing a network adapter.

Copyright (c) 2004 - 2008, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed 
and made available under the terms and conditions of the BSD License which 
accompanies this distribution. The full text of the license may be found at 
http://opensource.org/licenses/bsd-license.php 

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include "Snp.h"

EFI_STATUS
SnpInitImpl(
    _In_ SNP_DRIVER *Snp
    )
/**

Routine Description:

    Initialize the interface.

Arguments:

    Snp                   Pointer to snp driver structure.

    CableDetectFlag       Do/don't detect the cable 

Return Value:

    EFI_SUCCESS           Initialized successfully.

    EFI_DEVICE_ERROR      Initialization error.

    Other                 Other errors as indicated.

**/
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


EFI_STATUS
EFIAPI
SnpInitialize(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This,
    _In_ UINTN                       ExtraRxBufferSize,
    _In_ UINTN                       ExtraTxBufferSize
    )
/**

Routine Description:

    Resets a network adapter and allocates the transmit and receive buffers 
    required by the network interface; optionally, also requests allocation of 
    additional transmit and receive buffers.

    This function allocates the transmit and receive buffers required by the network
    interface. If this allocation fails, then EFI_OUT_OF_RESOURCES is returned.
    If the allocation succeeds and the network interface is successfully initialized,
    then EFI_SUCCESS will be returned.

Arguments:

    This               A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

    ExtraRxBufferSize  The size, in bytes, of the extra receive buffer space
                            that the driver should allocate for the network interface.
                            Some network interfaces will not be able to use the 
                            extra buffer, and the caller will not know if it is 
                            actually being used.

    ExtraTxBufferSize  The size, in bytes, of the extra transmit buffer space
                            that the driver should allocate for the network interface.
                            Some network interfaces will not be able to use the
                            extra buffer, and the caller will not know if it is
                            actually being used.

Return Value:

    EFI_SUCCESS           The network interface was initialized.

    EFI_NOT_STARTED       The network interface has not been started.

    EFI_OUT_OF_RESOURCES  There was not enough memory for the transmit and
                                receive buffers.

    EFI_INVALID_PARAMETER This parameter was NULL or did not point to a valid
                                EFI_SIMPLE_NETWORK_PROTOCOL structure.

    EFI_DEVICE_ERROR      The command could not be sent to the network interface.

    EFI_UNSUPPORTED       The increased buffer size feature is not supported.

**/
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
