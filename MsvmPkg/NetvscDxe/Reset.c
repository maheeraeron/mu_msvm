/** @file
    Implementation of resetting a network adapter.

    Copyright (c) 2004 - 2007, Intel Corporation. All rights reserved.<BR>
    Copyright (c) Microsoft Corporation.
    Licensed under the BSD-2-Clause-Patent license.
**/

#include "Snp.h"


EFI_STATUS
SnpResetImpl(
  _In_ SNP_DRIVER *Snp
  )
/**

Routine Description:

    Call Netvsc to reset the NIC.

Arguments:

    Snp                 Pointer to the snp driver structure.

Return Value:

    EFI_SUCCESSFUL      The vNIC was reset.

    EFI_DEVICE_ERROR    The vNIC cannot be reset.

**/
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_NETWORK_STATISTICS savedStats;
    UINT32 savedFilters;

    CopyMem(&savedStats, &Snp->AdapterContext->NicInfo.Statistics, sizeof(EFI_NETWORK_STATISTICS));
    savedFilters = Snp->AdapterContext->NicInfo.RxFilter;

    status = NetvscShutdown(&Snp->AdapterContext->NicInfo);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    status = NetvscInit(&Snp->AdapterContext->NicInfo);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    CopyMem(&Snp->AdapterContext->NicInfo.Statistics, &savedStats, sizeof(EFI_NETWORK_STATISTICS));

    status = NetvscSetFilter(&Snp->AdapterContext->NicInfo, savedFilters);

Cleanup:
    
    if (EFI_ERROR(status))
    {
        status = EFI_DEVICE_ERROR;
    }

    return status;
}


EFI_STATUS
EFIAPI
SnpReset(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This,
    _In_ BOOLEAN                     ExtendedVerification
    )
/**

Routine Description:

    Resets a network adapter and reinitializes it with the parameters that were
    provided in the previous call to Initialize().

    This function resets a network adapter and reinitializes it with the parameters
    that were provided in the previous call to Initialize(). The transmit and 
    receive queues are emptied and all pending interrupts are cleared.
    Receive filters, the station address, the statistics, and the multicast-IP-to-HW 
    MAC addresses are not reset by this call. If the network interface was 
    successfully reset, then EFI_SUCCESS will be returned. If the driver has not 
    been initialized, EFI_DEVICE_ERROR will be returned.

Arguments:

    This                 A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

    ExtendedVerification Indicates that the driver may perform a more 
                              exhaustive verification operation of the device 
                              during reset.

Return Value:

    EFI_SUCCESS           The network interface was reset.

    EFI_NOT_STARTED       The network interface has not been started.

    EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.

    EFI_DEVICE_ERROR      The command could not be sent to the network interface.

    EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
{
    SNP_DRIVER  *snpDriver;
    EFI_TPL     oldTpl;
    EFI_STATUS  status;

    //
    // Ignoring ExtendedVerification as it doesn't change how vNIC is reset.
    //
    UNREFERENCED_PARAMETER(ExtendedVerification);

    if (This == NULL)
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

    status = SnpResetImpl(snpDriver);

Exit:

    gBS->RestoreTPL(oldTpl);

InvalidParamExit:

    return status;
}
