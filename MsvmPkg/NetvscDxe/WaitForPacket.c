/** @file

    ATTENTION - THIS FILE CONTAINS THIRD PARTY OPEN SOURCE CODE:
                MsvmPkg\MsvmSnpDxe\WaitForPacket.c.
    IT IS CLEARED ONLY FOR LIMITED USE BY WINDOWS CORE HYPER-V FOR THE HYPER-V ROLE IN THE 
    WINDOWS PRODUCT.  DO NOT USE OR SHARE THIS CODE WITHOUT APPROVAL PURSUANT TO THE 
    MICROSOFT OPEN SOURCE  SOFTWARE APPROVAL POLICY. 

    Event handler to check for available packet.

Copyright (c) 2004 - 2008, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed 
and made available under the terms and conditions of the BSD License which 
accompanies this distribution. The full text of the license may be found at 
http://opensource.org/licenses/bsd-license.php 

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

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
