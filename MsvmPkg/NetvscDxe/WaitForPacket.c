/** @file
    Event handler to check for available packet.

Copyright (c) 2004 - 2018, Intel Corporation. All rights reserved.<BR>
Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Snp.h"

/**
  Notification call back function for WaitForPacket event.

  @param  Event       EFI Event.
  @param  SnpPtr      Pointer to SNP_DRIVER structure.

**/
VOID
EFIAPI
SnpWaitForPacketNotify (
  EFI_EVENT  Event,
  VOID       *SnpPtr
  )
{
    NIC_DATA_INSTANCE *adapterInfo;

  //
  // Do nothing if either parameter is a NULL pointer.
  //
  if ((Event == NULL) || (SnpPtr == NULL)) {
    return;
  }

  //
  // Do nothing if the SNP interface is not initialized.
  //
  switch (((SNP_DRIVER *)SnpPtr)->Mode.State) {
    case EfiSimpleNetworkInitialized:
      break;

    case EfiSimpleNetworkStopped:
    case EfiSimpleNetworkStarted:
    default:
      return;
  }

    adapterInfo = &(((SNP_DRIVER *) SnpPtr)->AdapterContext->NicInfo);
    if (!RxQueueIsEmpty(&adapterInfo->RxPacketQueue))
    {
        gBS->SignalEvent(Event);
    }
}
