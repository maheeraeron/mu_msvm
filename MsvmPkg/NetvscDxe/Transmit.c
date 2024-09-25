/** @file
    Implementation of transmitting a packet.

Copyright (c) 2004 - 2018, Intel Corporation. All rights reserved.<BR>
Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Snp.h"


VOID
FillPacketHeader(
    IN  SNP_DRIVER          *Snp,
    OUT VOID                *MacHeaderPtr,
    IN  EFI_MAC_ADDRESS     *DestAddr,
    IN  EFI_MAC_ADDRESS     *SrcAddr OPTIONAL,
    IN  UINT16              *ProtocolPtr
    )
/**

Routine Description:

    Create the meadia header for the given data buffer.

Arguments:

    Snp                 pointer to SNP driver structure

    MacHeaderPtr     Address where the media header will be filled in.

    DestAddr         Address of the destination mac address buffer.

    SrcAddr          Address of the source mac address buffer.

    ProtocolPtr      Address of the protocol type.

**/
{
    ETHERNET_HEADER    *macHeader;
    UINT32             index;
    EFI_MAC_ADDRESS    *sourceAddr;

    sourceAddr = SrcAddr == NULL? &Snp->Mode.CurrentAddress: SrcAddr;
    macHeader = (ETHERNET_HEADER *) MacHeaderPtr;

    macHeader->Type = (UINT16) PXE_SWAP_UINT16 (*ProtocolPtr);

    for (index = 0; index < PXE_HWADDR_LEN_ETHER; index++)
    {
        macHeader->DestAddr[index] = DestAddr->Addr[index];
        macHeader->SrcAddr[index] = sourceAddr->Addr[index];
    }
}

/**
  This routine calls undi to transmit the given data buffer

  @param  Snp                 pointer to SNP driver structure
  @param  Buffer           data buffer pointer
  @param  BufferSize        Size of data in the Buffer

  @retval EFI_SUCCESS         if successfully completed the undi call
  @retval Other               error return from undi call.

**/
EFI_STATUS
SnpTransmitImpl(
    IN  SNP_DRIVER      *Snp,
    IN  VOID            *Buffer,
    IN  UINTN           BufferSize
    )
{
    EFI_STATUS status;

    status = NetvscTransmit(&Snp->AdapterContext->NicInfo, Buffer, (UINT32) BufferSize);

    switch (status)
    {
    case EFI_SUCCESS:
    case EFI_NOT_READY:
    case EFI_DEVICE_ERROR:
        break;
    default:
        status = EFI_DEVICE_ERROR;
    }

    return status;
}

/**
  Places a packet in the transmit queue of a network interface.

  This function places the packet specified by Header and Buffer on the transmit
  queue. If HeaderSize is nonzero and HeaderSize is not equal to
  This->Mode->MediaHeaderSize, then EFI_INVALID_PARAMETER will be returned. If
  BufferSize is less than This->Mode->MediaHeaderSize, then EFI_BUFFER_TOO_SMALL
  will be returned. If Buffer is NULL, then EFI_INVALID_PARAMETER will be
  returned. If HeaderSize is nonzero and DestAddr or Protocol is NULL, then
  EFI_INVALID_PARAMETER will be returned. If the transmit engine of the network
  interface is busy, then EFI_NOT_READY will be returned. If this packet can be
  accepted by the transmit engine of the network interface, the packet contents
  specified by Buffer will be placed on the transmit queue of the network
  interface, and EFI_SUCCESS will be returned. GetStatus() can be used to
  determine when the packet has actually been transmitted. The contents of the
  Buffer must not be modified until the packet has actually been transmitted.
  The Transmit() function performs nonblocking I/O. A caller who wants to perform
  blocking I/O, should call Transmit(), and then GetStatus() until the
  transmitted buffer shows up in the recycled transmit buffer.
  If the driver has not been initialized, EFI_DEVICE_ERROR will be returned.

  @param This       A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
  @param HeaderSize The size, in bytes, of the media header to be filled in by the
                    Transmit() function. If HeaderSize is nonzero, then it must
                    be equal to This->Mode->MediaHeaderSize and the DestAddr and
                    Protocol parameters must not be NULL.
  @param BufferSize The size, in bytes, of the entire packet (media header and
                    data) to be transmitted through the network interface.
  @param Buffer     A pointer to the packet (media header followed by data) to be
                    transmitted. This parameter cannot be NULL. If HeaderSize is
                    zero, then the media header in Buffer must already be filled
                    in by the caller. If HeaderSize is nonzero, then the media
                    header will be filled in by the Transmit() function.
  @param SrcAddr    The source HW MAC address. If HeaderSize is zero, then this
                    parameter is ignored. If HeaderSize is nonzero and SrcAddr
                    is NULL, then This->Mode->CurrentAddress is used for the
                    source HW MAC address.
  @param DestAddr   The destination HW MAC address. If HeaderSize is zero, then
                    this parameter is ignored.
  @param Protocol   The type of header to build. If HeaderSize is zero, then this
                    parameter is ignored. See RFC 1700, section "Ether Types,"
                    for examples.

  @retval EFI_SUCCESS           The packet was placed on the transmit queue.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_NOT_READY         The network interface is too busy to accept this
                                transmit request.
  @retval EFI_BUFFER_TOO_SMALL  The BufferSize parameter is too small.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported
                                value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
SnpTransmit(
    IN  EFI_SIMPLE_NETWORK_PROTOCOL *This,
    IN  UINTN                       HeaderSize,
    IN  UINTN                       BufferSize,
    IN  VOID                        *Buffer,
    IN  EFI_MAC_ADDRESS             *SrcAddr OPTIONAL,
    IN  EFI_MAC_ADDRESS             *DestAddr OPTIONAL,
    IN  UINT16                      *Protocol OPTIONAL
    )
{
    SNP_DRIVER  *snpDriver;
    EFI_STATUS  status;
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

    if (Buffer == NULL)
    {
        status = EFI_INVALID_PARAMETER;
        goto Exit;
    }

    if (BufferSize < snpDriver->Mode.MediaHeaderSize)
    {
        status = EFI_BUFFER_TOO_SMALL;
        goto Exit;
    }

    if (BufferSize > snpDriver->Mode.MaxPacketSize)
    {
        status = EFI_INVALID_PARAMETER;
        goto Exit;
    }

    //
    // If the HeaderSize is non-zero, we need to fill up the header and for that
    // we need the destination address and the protocol.
    //
    if (HeaderSize != 0)
    {
        if (HeaderSize != snpDriver->Mode.MediaHeaderSize || DestAddr == 0 || Protocol == 0)
        {
            status = EFI_INVALID_PARAMETER;
            goto Exit;
        }

        FillPacketHeader(
            snpDriver,
            Buffer,
            DestAddr,
            SrcAddr,
            Protocol);
    }

    status = SnpTransmitImpl(snpDriver, Buffer, BufferSize);

Exit:

    gBS->RestoreTPL(oldTpl);

InvalidParamExit:

    return status;
}
