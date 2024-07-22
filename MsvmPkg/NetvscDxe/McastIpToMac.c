/** @file
    Implementation of converting an multicast IP address to multicast HW MAC
    address.

    Copyright (c) 2004 - 2007, Intel Corporation. All rights reserved.<BR>
    Copyright (c) Microsoft Corporation.
    Licensed under the BSD-2-Clause-Patent license.
**/

#include "Snp.h"

EFI_STATUS
SnpIp2MacImpl(
    _In_ BOOLEAN             IPv6,
    _In_ EFI_IP_ADDRESS      *IP,
    _Out_ EFI_MAC_ADDRESS     *MAC
    )
/**

Routine Description

    Convert an multicast IP address to a MAC address.

Arguments:

    IPv6  Flag to indicate if this is an ipv6 address.

    IP    Multicast IP address.

    MAC   Pointer to hold the return MAC address.

Return Value:

    EFI_SUCCESS    IP converted to a multicast MAC address

    EFI_INVALID_PARAMETER THe IP was not a multicast IP

**/
{
    EFI_STATUS status = EFI_SUCCESS;

    if (IPv6)
    {
        if (IP->v6.Addr[0] != 0xFF)
        {
            status = EFI_INVALID_PARAMETER;
        }
        else
        {
            MAC->Addr[0] = 0x33;
            MAC->Addr[1] = 0x33;
            MAC->Addr[2] = IP->v6.Addr[12];
            MAC->Addr[3] = IP->v6.Addr[13];
            MAC->Addr[4] = IP->v6.Addr[14];
            MAC->Addr[5] = IP->v6.Addr[15];
        }
    }
    else
    {
        if ((IP->v4.Addr[0] & 0xF0) != 0xE0)
        {
            status = EFI_INVALID_PARAMETER;
        }
        else
        {
            MAC->Addr[0] = 0x01;
            MAC->Addr[1] = 0x00;
            MAC->Addr[2] = 0x5E;
            MAC->Addr[3] = (IP->v4.Addr[1] & 0x7F);
            MAC->Addr[4] = IP->v4.Addr[2];
            MAC->Addr[5] = IP->v4.Addr[3];
        }
    }

    return status;
}


EFI_STATUS
EFIAPI
SnpMcastIpToMac(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This,
    _In_ BOOLEAN                     IPv6,
    _In_ EFI_IP_ADDRESS              *IP,
    _Out_ EFI_MAC_ADDRESS            *MAC
    )
/**

Routine Description:

    Converts a multicast IP address to a multicast HW MAC address.

    This function converts a multicast IP address to a multicast HW MAC address 
    for all packet transactions. If the mapping is accepted, then EFI_SUCCESS will
    be returned.

Arguments:

    This     A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

    IPv6 Set to TRUE if the multicast IP address is IPv6 [RFC 2460].
              Set to FALSE if the multicast IP address is IPv4 [RFC 791]. 

    IP   The multicast IP address that is to be converted to a multicast 
              HW MAC address.

    MAC  The multicast HW MAC address that is to be generated from IP.

Return Value:

    EFI_SUCCESS           The multicast IP address was mapped to the
                                multicast HW MAC address.

    EFI_NOT_STARTED       The Simple Network Protocol interface has not 
                                been started by calling Start().

    EFI_INVALID_PARAMETER IP is NULL.

    EFI_INVALID_PARAMETER MAC is NULL.

    EFI_INVALID_PARAMETER IP does not point to a valid IPv4 or IPv6 
                                multicast address.

**/
{
    SNP_DRIVER  *snpDriver;
    EFI_TPL     oldTpl;
    EFI_STATUS  status;

    //
    // Get pointer to SNP driver instance for *this.
    //
    if (This == NULL)
    {
        status = EFI_INVALID_PARAMETER;
        goto InvalidParamExit;
    }

    if (IP == NULL || MAC == NULL)
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

    status = SnpIp2MacImpl(IPv6, IP, MAC);

Exit:

    gBS->RestoreTPL(oldTpl);

InvalidParamExit:

    return status;
}
