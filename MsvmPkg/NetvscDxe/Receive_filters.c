/** @file
    Implementation of managing the multicast receive filters of a network
    interface.

Copyright (c) 2004 - 2018, Intel Corporation. All rights reserved.<BR>
Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Snp.h"

/**
  Call undi to enable the receive filters.

  @param  Snp                Pointer to snp driver structure.
  @param  EnableFlags        Bit mask for enabling the receive filters.
  @param  MCastAddressCount  Multicast address count for a new multicast address
                             list.
  @param  MCastAddressList   List of new multicast addresses.

  @retval EFI_SUCCESS           The multicast receive filter list was updated.
  @retval EFI_INVALID_PARAMETER Invalid UNDI command.
  @retval EFI_UNSUPPORTED       Command is not supported by UNDI.
  @retval EFI_DEVICE_ERROR      Fail to execute UNDI command.

**/
EFI_STATUS
SnpRecvFilterEnable(
    IN  SNP_DRIVER      *Snp,
    IN  UINT32          EnableFlags,
    IN  UINTN           MCastAddressCount,
    IN  EFI_MAC_ADDRESS *MCastAddressList
    )
{
    UINT32       newFilter;
    EFI_STATUS   status;

    if (MCastAddressCount > 0)
    {
        //
        // All Multicast Packets are broadcasted to all vNICS by VMswitch
        // So, the worst case scenario is that the Stack would be processing
        // a lot of multicast packets it doesn't need to.
        //
        // TODO: Implement Multicast support or clarify the lack of support for Multicasting.
        //
        CopyMem(Snp->Mode.MCastFilter, MCastAddressList, MCastAddressCount*sizeof(EFI_MAC_ADDRESS));
        Snp->Mode.MCastFilterCount = (UINT32) MCastAddressCount;
    }

    newFilter = (EnableFlags & Snp->Mode.ReceiveFilterMask) | Snp->AdapterContext->NicInfo.RxFilter;

    status = NetvscSetFilter(&Snp->AdapterContext->NicInfo, newFilter);

    if (EFI_ERROR(status))
    {
        status = EFI_DEVICE_ERROR;
    }

    return status;
}

/**
  Call undi to disable the receive filters.

  @param  Snp             Pointer to snp driver structure
  @param  DisableFlags    Bit mask for disabling the receive filters
  @param  ResetMCastList  Boolean flag to reset/delete the multicast filter
                          list.

  @retval EFI_SUCCESS           The multicast receive filter list was updated.
  @retval EFI_DEVICE_ERROR      Fail to execute UNDI command.

**/
EFI_STATUS
SnpRecvFilterDisable(
    IN  SNP_DRIVER *Snp,
    IN  UINT32     DisableFlags,
    IN  BOOLEAN    ResetMCastList
    )
{
    UINT32       newFilter;
    EFI_STATUS   status;

    if (ResetMCastList)
    {
        //
        // All Multicast Packets are broadcasted to all vNICS by VMswitch
        // So, the worst case scenario is that the stack would be processing
        // a lot of multicast packets it doesn't need to.
        //
        Snp->Mode.MCastFilterCount = 0;
    }

    newFilter = (~(DisableFlags & Snp->Mode.ReceiveFilterMask)) & Snp->AdapterContext->NicInfo.RxFilter;

    status = NetvscSetFilter(&Snp->AdapterContext->NicInfo, newFilter);

    if (EFI_ERROR(status))
    {
        status = EFI_DEVICE_ERROR;
    }

    return status;
}


/**
  Manages the multicast receive filters of a network interface.

  This function is used enable and disable the hardware and software receive
  filters for the underlying network device.
  The receive filter change is broken down into three steps:
  * The filter mask bits that are set (ON) in the Enable parameter are added to
    the current receive filter settings.
  * The filter mask bits that are set (ON) in the Disable parameter are subtracted
    from the updated receive filter settings.
  * If the resulting receive filter setting is not supported by the hardware a
    more liberal setting is selected.
  If the same bits are set in the Enable and Disable parameters, then the bits
  in the Disable parameter takes precedence.
  If the ResetMCastFilter parameter is TRUE, then the multicast address list
  filter is disabled (irregardless of what other multicast bits are set in the
  Enable and Disable parameters). The SNP->Mode->MCastFilterCount field is set
  to zero. The Snp->Mode->MCastFilter contents are undefined.
  After enabling or disabling receive filter settings, software should verify
  the new settings by checking the Snp->Mode->ReceiveFilterSettings,
  Snp->Mode->MCastFilterCount and Snp->Mode->MCastFilter fields.
  Note: Some network drivers and/or devices will automatically promote receive
    filter settings if the requested setting can not be honored. For example, if
    a request for four multicast addresses is made and the underlying hardware
    only supports two multicast addresses the driver might set the promiscuous
    or promiscuous multicast receive filters instead. The receiving software is
    responsible for discarding any extra packets that get through the hardware
    receive filters.
    Note: Note: To disable all receive filter hardware, the network driver must
      be Shutdown() and Stopped(). Calling ReceiveFilters() with Disable set to
      Snp->Mode->ReceiveFilterSettings will make it so no more packets are
      returned by the Receive() function, but the receive hardware may still be
      moving packets into system memory before inspecting and discarding them.
      Unexpected system errors, reboots and hangs can occur if an OS is loaded
      and the network devices are not Shutdown() and Stopped().
  If ResetMCastFilter is TRUE, then the multicast receive filter list on the
  network interface will be reset to the default multicast receive filter list.
  If ResetMCastFilter is FALSE, and this network interface allows the multicast
  receive filter list to be modified, then the MCastFilterCnt and MCastFilter
  are used to update the current multicast receive filter list. The modified
  receive filter list settings can be found in the MCastFilter field of
  EFI_SIMPLE_NETWORK_MODE. If the network interface does not allow the multicast
  receive filter list to be modified, then EFI_INVALID_PARAMETER will be returned.
  If the driver has not been initialized, EFI_DEVICE_ERROR will be returned.
  If the receive filter mask and multicast receive filter list have been
  successfully updated on the network interface, EFI_SUCCESS will be returned.

  @param This             A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
  @param Enable           A bit mask of receive filters to enable on the network
                          interface.
  @param Disable          A bit mask of receive filters to disable on the network
                          interface. For backward compatibility with EFI 1.1
                          platforms, the EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST bit
                          must be set when the ResetMCastFilter parameter is TRUE.
  @param ResetMCastFilter Set to TRUE to reset the contents of the multicast
                          receive filters on the network interface to their
                          default values.
  @param MCastFilterCnt   Number of multicast HW MAC addresses in the new MCastFilter
                          list. This value must be less than or equal to the
                          MCastFilterCnt field of EFI_SIMPLE_NETWORK_MODE.
                          This field is optional if ResetMCastFilter is TRUE.
  @param MCastFilter      A pointer to a list of new multicast receive filter HW
                          MAC addresses. This list will replace any existing
                          multicast HW MAC address list. This field is optional
                          if ResetMCastFilter is TRUE.

  @retval EFI_SUCCESS            The multicast receive filter list was updated.
  @retval EFI_NOT_STARTED        The network interface has not been started.
  @retval EFI_INVALID_PARAMETER  One or more of the following conditions is TRUE:
                                 * This is NULL
                                 * There are bits set in Enable that are not set
                                   in Snp->Mode->ReceiveFilterMask
                                 * There are bits set in Disable that are not set
                                   in Snp->Mode->ReceiveFilterMask
                                 * Multicast is being enabled (the
                                   EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST bit is
                                   set in Enable, it is not set in Disable, and
                                   ResetMCastFilter is FALSE) and MCastFilterCount
                                   is zero
                                 * Multicast is being enabled and MCastFilterCount
                                   is greater than Snp->Mode->MaxMCastFilterCount
                                 * Multicast is being enabled and MCastFilter is NULL
                                 * Multicast is being enabled and one or more of
                                   the addresses in the MCastFilter list are not
                                   valid multicast MAC addresses
  @retval EFI_DEVICE_ERROR       One or more of the following conditions is TRUE:
                                 * The network interface has been started but has
                                   not been initialized
                                 * An unexpected error was returned by the
                                   underlying network driver or device
  @retval EFI_UNSUPPORTED        This function is not supported by the network
                                 interface.

**/
EFI_STATUS
EFIAPI
SnpReceiveFilters(
    IN  EFI_SIMPLE_NETWORK_PROTOCOL *This,
    IN  UINT32                      Enable,
    IN  UINT32                      Disable,
    IN  BOOLEAN                     ResetMCastFilter,
    IN  UINTN                       MCastFilterCnt,
    IN  EFI_MAC_ADDRESS         *MCastFilter OPTIONAL
    )
{
    SNP_DRIVER  *snpDriver;
    EFI_STATUS  status = EFI_SUCCESS;
    EFI_TPL     oldTpl;

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

    //
    // Check if we are asked to enable or disable something that the VSP
    // does not even support.
    //
    if (((Enable &~snpDriver->Mode.ReceiveFilterMask) != 0) ||
        ((Disable &~snpDriver->Mode.ReceiveFilterMask) != 0))
    {
        status = EFI_INVALID_PARAMETER;
        goto Exit;
    }

    if (ResetMCastFilter)
    {
        Disable |= EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST & snpDriver->Mode.ReceiveFilterMask;
        MCastFilterCnt = 0;
        MCastFilter    = NULL;
    }
    else
    {
        if (MCastFilterCnt != 0)
        {
            if ((MCastFilterCnt > snpDriver->Mode.MaxMCastFilterCount) ||
                (MCastFilter == NULL))
            {
                status = EFI_INVALID_PARAMETER;
                goto Exit;
            }
        }
    }

    if (Enable == 0 && Disable == 0 && !ResetMCastFilter && MCastFilterCnt == 0)
    {
        status = EFI_SUCCESS;
        goto Exit;
    }

    if ((Enable & EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST) != 0 && MCastFilterCnt == 0)
    {
        status = EFI_INVALID_PARAMETER;
        goto Exit;
    }

    if ((Enable != 0) || (MCastFilterCnt != 0))
    {
        status = SnpRecvFilterEnable(
            snpDriver,
            Enable,
            MCastFilterCnt,
            MCastFilter);

        if (EFI_ERROR (status))
        {
            goto Exit;
        }
    }
    snpDriver->Mode.ReceiveFilterSetting = snpDriver->AdapterContext->NicInfo.RxFilter;

    if ((Disable != 0) || ResetMCastFilter)
    {
        status = SnpRecvFilterDisable(snpDriver, Disable, ResetMCastFilter);

        if (EFI_ERROR(status))
        {
            goto Exit;
        }
    }

    snpDriver->Mode.ReceiveFilterSetting = snpDriver->AdapterContext->NicInfo.RxFilter;

Exit:

    gBS->RestoreTPL(oldTpl);

InvalidParamExit:

    return status;
}
