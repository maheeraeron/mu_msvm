/** @file

    ATTENTION - THIS FILE CONTAINS THIRD PARTY OPEN SOURCE CODE:
                MsvmPkg\MsvmSnpDxe\Shutdown.c.
    IT IS CLEARED ONLY FOR LIMITED USE BY WINDOWS CORE HYPER-V FOR THE HYPER-V ROLE IN THE 
    WINDOWS PRODUCT.  DO NOT USE OR SHARE THIS CODE WITHOUT APPROVAL PURSUANT TO THE 
    MICROSOFT OPEN SOURCE  SOFTWARE APPROVAL POLICY. 

    Implementation of shuting down a network adapter.
 
Copyright (c) 2004 - 2007, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed 
and made available under the terms and conditions of the BSD License which 
accompanies this distribution. The full text of the license may be found at 
http://opensource.org/licenses/bsd-license.php 

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Snp.h"

EFI_STATUS
SnpShutdownImpl(
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
    EFI_STATUS status;

    status = NetvscShutdown(&Snp->AdapterContext->NicInfo);

    if (EFI_ERROR(status))
    {
        status = EFI_DEVICE_ERROR;
    }

    Snp->Mode.State = EfiSimpleNetworkStarted;

    return status;  
}


EFI_STATUS
EFIAPI
SnpShutdown(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This
    )
/*++

Routine Description:

      Resets a network adapter and leaves it in a state that is safe for another 
      driver to initialize. 
      
      This function releases the memory buffers assigned in the Initialize() call.
      Pending transmits and receives are lost, and interrupts are cleared and disabled.
      After this call, only the Initialize() and Stop() calls may be used. If the 
      network interface was successfully shutdown, then EFI_SUCCESS will be returned.
      If the driver has not been initialized, EFI_DEVICE_ERROR will be returned.

Arguments:

    This  A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

Return Value:

    EFI_SUCCESS           The network interface was shutdown.

    EFI_NOT_STARTED       The network interface has not been started.

    EFI_INVALID_PARAMETER This parameter was NULL or did not point to a valid 
                                EFI_SIMPLE_NETWORK_PROTOCOL structure.

    EFI_DEVICE_ERROR      The command could not be sent to the network interface.

--*/
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
