/** @file
  
  ATTENTION - THIS FILE CONTAINS THIRD PARTY OPEN SOURCE CODE:
              MsvmPkg\MsvmSnpDxe\Receive.c.
  IT IS CLEARED ONLY FOR LIMITED USE BY WINDOWS CORE HYPER-V FOR THE HYPER-V ROLE IN THE 
  WINDOWS PRODUCT.  DO NOT USE OR SHARE THIS CODE WITHOUT APPROVAL PURSUANT TO THE 
  MICROSOFT OPEN SOURCE  SOFTWARE APPROVAL POLICY. 
  
  Implementation of receiving a packet from a network interface.

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
SnpReceiveImpl(
    _In_ SNP_DRIVER      *Snp,
    _Out_writes_bytes_(*BufferSize) VOID    *Buffer,
    _Inout_ UINTN                           *BufferSize,
    _Out_opt_ UINTN                         *HeaderSize,
    _Out_opt_ EFI_MAC_ADDRESS               *SrcAddr,
    _Out_opt_ EFI_MAC_ADDRESS               *DestAddr,
    _Out_opt_ UINT16                        *Protocol
    )
/*++

Routine Description:

    Call Netvsc to receive a packet and fills in the data in the input pointers.

Arguments:

    Snp          Pointer to snp driver structure

    Buffer       Pointer to the memory for the received data

    BufferSize   Pointer to the length of the buffer on entry and contains
                       the length of the received data on return

    HeaderSize   Pointer to the header portion of the data received.

    SrcAddr      Pointer to contain the source ethernet address on return

    DestAddr     Pointer to contain the destination ethernet address on
                       return

    Protocol     Pointer to contain the protocol type from the ethernet
                       header on return

Return Value:

    EFI_SUCCESS           The received data was stored in Buffer, and
                                BufferSize has been updated to the number of
                                bytes received.

    EFI_DEVICE_ERROR      Other failure.

    EFI_NOT_READY         No packets have been received on the network
                                interface.

    EFI_BUFFER_TOO_SMALL  BufferSize is too small for the received
                                packets. BufferSize has been updated to the
                                required size.

--*/
{
    UINTN           buffSize;
    EFI_STATUS      status;

    buffSize  = *BufferSize;

    status = NetvscReceive(
        &Snp->AdapterContext->NicInfo,
        Buffer,
        BufferSize,
        HeaderSize,
        SrcAddr,
        DestAddr,
        Protocol);

    switch(status)
    {
    case EFI_SUCCESS:
        break;
        
    case EFI_NOT_READY:
        return status;

    default:
        return EFI_DEVICE_ERROR;
    }

    return (*BufferSize <= buffSize) ? EFI_SUCCESS : EFI_BUFFER_TOO_SMALL;
}


EFI_STATUS
EFIAPI
SnpReceive(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL        *This,
    _Out_opt_ UINTN                         *HeaderSize,
    _Inout_ UINTN                           *BufferSize,
    _Out_writes_bytes_(*BufferSize) VOID    *Buffer,
    _Out_opt_ EFI_MAC_ADDRESS               *SrcAddr,
    _Out_opt_ EFI_MAC_ADDRESS               *DestAddr,
    _Out_opt_ UINT16                        *Protocol
    )
/*++

Routine Description:

    Receives a packet from a network interface.

    This function retrieves one packet from the receive queue of a network interface.
    If there are no packets on the receive queue, then EFI_NOT_READY will be
    returned. If there is a packet on the receive queue, and the size of the packet
    is smaller than BufferSize, then the contents of the packet will be placed in
    Buffer, and BufferSize will be updated with the actual size of the packet.
    In addition, if SrcAddr, DestAddr, and Protocol are not NULL, then these values
    will be extracted from the media header and returned. EFI_SUCCESS will be
    returned if a packet was successfully received.
    If BufferSize is smaller than the received packet, then the size of the receive
    packet will be placed in BufferSize and EFI_BUFFER_TOO_SMALL will be returned.
    If the driver has not been initialized, EFI_DEVICE_ERROR will be returned.

Arguments:

    This       A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

    HeaderSize The size, in bytes, of the media header received on the network
                    interface. If this parameter is NULL, then the media header size
                    will not be returned.

    BufferSize On entry, the size, in bytes, of Buffer. On exit, the size, in
                    bytes, of the packet that was received on the network interface.

    Buffer      A pointer to the data buffer to receive both the media
                    header and the data.

    SrcAddr    The source HW MAC address. If this parameter is NULL, the HW
                    MAC source address will not be extracted from the media header.

    DestAddr   The destination HW MAC address. If this parameter is NULL,
                    the HW MAC destination address will not be extracted from
                    the media header.

    Protocol   The media header type. If this parameter is NULL, then the
                    protocol will not be extracted from the media header.

Return Value:

    EFI_SUCCESS           The received data was stored in Buffer, and
                                   BufferSize has been updated to the number of
                                   bytes received.

    EFI_NOT_STARTED       The network interface has not been started.

    EFI_NOT_READY         No packets have been received on the network interface.

    EFI_BUFFER_TOO_SMALL  BufferSize is too small for the received packets.
                                BufferSize has been updated to the required size.

    EFI_INVALID_PARAMETER One or more of the following conditions is TRUE:
                                * The This parameter is NULL
                                * The This parameter does not point to a valid
                                  EFI_SIMPLE_NETWORK_PROTOCOL structure.
                                * The BufferSize parameter is NULL
                                * The Buffer parameter is NULL

    EFI_DEVICE_ERROR      The command could not be sent to the network interface.

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

    if ((BufferSize == NULL) || (Buffer == NULL))
    {
        status = EFI_INVALID_PARAMETER;
        goto Exit;
    }

    if (snpDriver->Mode.ReceiveFilterSetting == 0)
    {
        status = EFI_DEVICE_ERROR;
        goto Exit;
    }

    status = SnpReceiveImpl(
        snpDriver,
        Buffer,
        BufferSize,
        HeaderSize,
        SrcAddr,
        DestAddr,
        Protocol);

Exit:

    gBS->RestoreTPL(oldTpl);

InvalidParamExit:

    return status;
}
