/** @file
    Event handler to check for available packet.

    Copyright (c) 2004 - 2008, Intel Corporation. All rights reserved.<BR>
    Copyright (c) Microsoft Corporation.
    Licensed under the BSD-2-Clause-Patent license.
**/

#include "Snp.h"


VOID
EFIAPI
SnpWaitForPacketNotify(
    _In_ EFI_EVENT     Event,
    _In_ VOID          *SnpPtr
    )
/**

Routine Description:

    Notification call back function for WaitForPacket event.

Arguments:

    Event       EFI Event.

    SnpPtr      Pointer to SNP_DRIVER structure.

**/
{
    NIC_DATA_INSTANCE *adapterInfo;

    //
    // Do nothing if either parameter is a NULL pointer.
    //
    if (Event == NULL || SnpPtr == NULL)
    {
        return ;
    }

    //
    // Do nothing if the SNP interface is not initialized.
    //
    switch (((SNP_DRIVER *) SnpPtr)->Mode.State)
    {
    case EfiSimpleNetworkInitialized:
        break;

    case EfiSimpleNetworkStopped:
    case EfiSimpleNetworkStarted:
    default:
        return ;
    }

    adapterInfo = &(((SNP_DRIVER *) SnpPtr)->AdapterContext->NicInfo);
    if (!RxQueueIsEmpty(&adapterInfo->RxPacketQueue))
    {
        gBS->SignalEvent(Event);
    }
}
