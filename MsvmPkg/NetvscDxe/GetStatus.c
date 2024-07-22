/** @file
    Implementation of reading the current interrupt status and recycled transmit
    buffer status from a network interface.

    Copyright (c) 2004 - 2010, Intel Corporation. All rights reserved.<BR>
    Copyright (c) Microsoft Corporation.
    Licensed under the BSD-2-Clause-Patent license.
**/

#include "Snp.h"


EFI_STATUS
SnpGetStatusImpl(
    _In_ SNP_DRIVER      *Snp,
    _Out_opt_ UINT32     *InterruptStatusPtr,
    _Out_opt_ VOID       **TransmitBufferListPtr
    )
/**

Routine Description:

    Get the status of the interrupts, get the list of transmit
    buffers that completed transmitting.

Arguments:

    Snp                     Pointer to snp driver structure.

    InterruptStatusPtr      A non null pointer to contain the interrupt
                              status.

    TransmitBufferListPtrs  A non null pointer to contain the list of
                              pointers of previous transmitted buffers whose
                              transmission was completed asynchronously.

Return Value:

    EFI_SUCCESS         The status of the network interface was retrieved.

    EFI_DEVICE_ERROR    The command could not be sent to the network
                              interface.

**/
{
    NIC_DATA_INSTANCE *adapterInfo;

    adapterInfo = &Snp->AdapterContext->NicInfo;

    if (InterruptStatusPtr != NULL)
    {
        *InterruptStatusPtr = 0;

        if (adapterInfo->RxInterrupt)
        {
            *InterruptStatusPtr |= EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT;
            adapterInfo->RxInterrupt = FALSE;
        }

        if (adapterInfo->TxedInterrupt)
        {
            *InterruptStatusPtr |= EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT;
            adapterInfo->TxedInterrupt = FALSE;
        }
    }

    if (TransmitBufferListPtr != NULL)
    {
        if (TxQueueIsEmpty(&adapterInfo->TxedBuffersQueue))
        {
            *TransmitBufferListPtr = NULL;
        }
        else
        {
            TxQueueDequeue(&adapterInfo->TxedBuffersQueue, TransmitBufferListPtr);
        }
    }

    if (Snp->Mode.MediaPresentSupported)
    {
        Snp->Mode.MediaPresent = adapterInfo->MediaPresent;
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
SnpGetStatus(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This,
    _Out_opt_ UINT32                     *InterruptStatus,
    _Out_opt_ VOID                       **TxBuf
    )
/**

Routine Description:

    Reads the current interrupt status and recycled transmit buffer status from a
    network interface.

    This function gets the current interrupt and recycled transmit buffer status
    from the network interface. The interrupt status is returned as a bit mask in
    InterruptStatus. If InterruptStatus is NULL, the interrupt status will not be
    read. If TxBuf is not NULL, a recycled transmit buffer address will be retrieved.
    If a recycled transmit buffer address is returned in TxBuf, then the buffer has
    been successfully transmitted, and the status for that buffer is cleared. If
    the status of the network interface is successfully collected, EFI_SUCCESS
    will be returned. If the driver has not been initialized, EFI_DEVICE_ERROR will
    be returned.

Arguments:

    This                 A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

    InterruptStatus A pointer to the bit mask of the currently active
                         interrupts (see "Related Definitions"). If this is NULL,
                         the interrupt status will not be read from the device.
                         If this is not NULL, the interrupt status will be read
                         from the device. When the interrupt status is read, it
                         will also be cleared. Clearing the transmit interrupt does
                         not empty the recycled transmit buffer array.

    TxBuf             Recycled transmit buffer address. The network interface
                         will not transmit if its internal recycled transmit
                         buffer array is full. Reading the transmit buffer does
                         not clear the transmit interrupt. If this is NULL, then
                         the transmit buffer status will not be read. If there
                         are no transmit buffers to recycle and TxBuf is not NULL,
                         TxBuf will be set to NULL.

Return Value:

    EFI_SUCCESS           The status of the network interface was retrieved.

    EFI_NOT_STARTED       The network interface has not been started.

    EFI_INVALID_PARAMETER This parameter was NULL or did not point to a valid
                                EFI_SIMPLE_NETWORK_PROTOCOL structure.

    EFI_DEVICE_ERROR      The command could not be sent to the network
                                interface.

**/
{
    SNP_DRIVER  *snpDriver;
    EFI_TPL     oldTpl;
    EFI_STATUS  status;

    if (This == NULL)
    {
        status = EFI_INVALID_PARAMETER;
        goto InvalidParamExit;
    }

    if (InterruptStatus == NULL && TxBuf == NULL)
    {
        status = EFI_INVALID_PARAMETER;
        goto InvalidParamExit;
    }

    snpDriver = EFI_SIMPLE_NETWORK_DEV_FROM_THIS(This);

    oldTpl = gBS->RaiseTPL(TPL_CALLBACK);

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

    status = SnpGetStatusImpl(snpDriver, InterruptStatus, TxBuf);

Exit:

    gBS->RestoreTPL(oldTpl);

InvalidParamExit:

    return status;
}
