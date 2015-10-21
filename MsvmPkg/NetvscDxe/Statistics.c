/** @file

    ATTENTION - THIS FILE CONTAINS THIRD PARTY OPEN SOURCE CODE:
                MsvmPkg\MsvmSnpDxe\Statistics.c.
    IT IS CLEARED ONLY FOR LIMITED USE BY WINDOWS CORE HYPER-V FOR THE HYPER-V ROLE IN THE 
    WINDOWS PRODUCT.  DO NOT USE OR SHARE THIS CODE WITHOUT APPROVAL PURSUANT TO THE 
    MICROSOFT OPEN SOURCE  SOFTWARE APPROVAL POLICY. 

    Implementation of collecting the statistics on a network interface.

Copyright (c) 2004 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed
and made available under the terms and conditions of the BSD License which
accompanies this distribution. The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include "Snp.h"


EFI_STATUS
EFIAPI
SnpStatistics(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL    *This,
    _In_ BOOLEAN                        Reset,
    _Inout_opt_ UINTN                   *StatisticsSize,
    _Inout_opt_ EFI_NETWORK_STATISTICS  *StatisticsTable
    )
/**

Routine Description:

    Resets or collects the statistics on a network interface.

    This function resets or collects the statistics on a network interface. If the
    size of the statistics table specified by StatisticsSize is not big enough for
    all the statistics that are collected by the network interface, then a partial
    buffer of statistics is returned in StatisticsTable, StatisticsSize is set to
    the size required to collect all the available statistics, and
    EFI_BUFFER_TOO_SMALL is returned.
    If StatisticsSize is big enough for all the statistics, then StatisticsTable
    will be filled, StatisticsSize will be set to the size of the returned
    StatisticsTable structure, and EFI_SUCCESS is returned.
    If the driver has not been initialized, EFI_DEVICE_ERROR will be returned.
    If Reset is FALSE, and both StatisticsSize and StatisticsTable are NULL, then
    no operations will be performed, and EFI_SUCCESS will be returned.
    If Reset is TRUE, then all of the supported statistics counters on this network
    interface will be reset to zero.

Arguments:

    This            A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

    Reset           Set to TRUE to reset the statistics for the network interface.

    StatisticsSize  On input the size, in bytes, of StatisticsTable. On output
                         the size, in bytes, of the resulting table of statistics.

    StatisticsTable A pointer to the EFI_NETWORK_STATISTICS structure that
                         contains the statistics. Type EFI_NETWORK_STATISTICS is
                         defined in "Related Definitions" below.

Return Value:

    EFI_SUCCESS           The requested operation succeeded.

    EFI_NOT_STARTED       The Simple Network Protocol interface has not been
                                started by calling Start().

    EFI_BUFFER_TOO_SMALL  StatisticsSize is not NULL and StatisticsTable is
                                NULL. The current buffer size that is needed to
                                hold all the statistics is returned in StatisticsSize.

    EFI_BUFFER_TOO_SMALL  StatisticsSize is not NULL and StatisticsTable is
                                not NULL. The current buffer size that is needed
                                to hold all the statistics is returned in
                                StatisticsSize. A partial set of statistics is
                                returned in StatisticsTable.

    EFI_INVALID_PARAMETER StatisticsSize is NULL and StatisticsTable is not
                                NULL.

    EFI_DEVICE_ERROR      The Simple Network Protocol interface has not
                                been initialized by calling Initialize().

    EFI_DEVICE_ERROR      An error was encountered collecting statistics
                                from the NIC.

**/
{
    SNP_DRIVER        *snpDriver;
    UINT64            *outStp, *inStp;
    UINTN             requiredSize;
    UINTN             index;
    EFI_TPL           oldTpl;
    EFI_STATUS        status;
    NIC_DATA_INSTANCE *adapterInfo;

    //
    // Get pointer to SNP driver instance for *This.
    //
    if (This == NULL)
    {
        status = EFI_INVALID_PARAMETER;
        goto InvalidParamExit;
    }

    snpDriver = EFI_SIMPLE_NETWORK_DEV_FROM_THIS(This);
    adapterInfo = &snpDriver->AdapterContext->NicInfo;

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
    //
    // if we are not resetting the counters, we have to have a valid stat table
    // with >0 size. if no reset, no table and no size, return success.
    //
    if (!Reset && StatisticsSize == NULL)
    {
        status = (StatisticsTable != NULL) ? EFI_INVALID_PARAMETER : EFI_SUCCESS;
        goto Exit;
    }

    if (Reset)
    {
        NetvscResetStatistics(adapterInfo);
        status = EFI_SUCCESS;
        goto Exit;
    }

    if (StatisticsTable == NULL)
    {
        *StatisticsSize = sizeof (EFI_NETWORK_STATISTICS);
        status = EFI_BUFFER_TOO_SMALL;
        goto Exit;
    }
    //
    // Convert Adapter's statistics information to SNP statistics
    // information.
    //
    ZeroMem(StatisticsTable, *StatisticsSize);
    outStp   = (UINT64 *) StatisticsTable;
    inStp = (UINT64 *) &adapterInfo->Statistics;

    for (index = 0; index < 64; index++, outStp++, inStp++)
    {
        //
        // There must be room for a full UINT64.  Partial
        // numbers will not be stored.
        //
        if ((index + 1) * sizeof (UINT64) > *StatisticsSize)
        {
            break;
        }

        *outStp  = *inStp;
    }

    requiredSize = snpDriver->AdapterContext->NicInfo.SupportedStatisticsSize;
    if (*StatisticsSize >= requiredSize)
    {
        status = EFI_SUCCESS;
    }
    else
    {
        status = EFI_BUFFER_TOO_SMALL;
    }
    *StatisticsSize = requiredSize;

Exit:

    gBS->RestoreTPL(oldTpl);

InvalidParamExit:

    return status;
}
