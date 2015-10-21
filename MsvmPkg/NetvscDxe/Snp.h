/** @file

    ATTENTION - THIS FILE CONTAINS THIRD PARTY OPEN SOURCE CODE:
                MsvmPkg\MsvmSnpDxe\Snp.h.
    IT IS CLEARED ONLY FOR LIMITED USE BY WINDOWS CORE HYPER-V FOR THE HYPER-V ROLE IN THE 
    WINDOWS PRODUCT.  DO NOT USE OR SHARE THIS CODE WITHOUT APPROVAL PURSUANT TO THE 
    MICROSOFT OPEN SOURCE  SOFTWARE APPROVAL POLICY. 

    Declaration of strctures and functions for SnpDxe driver.

Copyright (c) 2004 - 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed
and made available under the terms and conditions of the BSD License which
accompanies this distribution. The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#pragma once

#include <Uefi.h>

#include <Netvscdxe.h>

#include <Protocol/SimpleNetwork.h>
#include <Protocol/DevicePath.h>


#include <Guid/EventGroup.h>

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>

#define FOUR_GIGABYTES  (UINT64) 0x100000000ULL

#define SNP_DRIVER_SIGNATURE  SIGNATURE_32('s', 'n', 'd', 's')

typedef struct
{
    UINT32                      Signature;
    EFI_SIMPLE_NETWORK_PROTOCOL Snp;
    EFI_SIMPLE_NETWORK_MODE     Mode;

    NETVSC_ADAPTER_CONTEXT      *AdapterContext;
} SNP_DRIVER;

#define EFI_SIMPLE_NETWORK_DEV_FROM_THIS(a) CR(a, SNP_DRIVER, Snp, SNP_DRIVER_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL    mSimpleNetworkDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL    gSimpleNetworkComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL   gSimpleNetworkComponentName2;

EFI_STATUS
SnpInitImpl(
    _In_ SNP_DRIVER *Snp
    );

EFI_STATUS
SnpShutdownImpl(
    _In_ SNP_DRIVER *Snp
    );

EFI_STATUS
SnpStopImpl(
    _In_ SNP_DRIVER *Snp
    );

EFI_STATUS
GetStnAddr(
    _In_ SNP_DRIVER *Snp
    );

EFI_STATUS
EFIAPI
SnpStart(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This
    );

EFI_STATUS
EFIAPI
SnpStop(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This
    );

EFI_STATUS
EFIAPI
SnpInitialize(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This,
    _In_ UINTN                       ExtraRxBufferSize,
    _In_ UINTN                       ExtraTxBufferSize
    );

EFI_STATUS
EFIAPI
SnpReset(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL  *This,
    _In_ BOOLEAN                      ExtendedVerification
    );

EFI_STATUS
EFIAPI
SnpShutdown(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This
    );

EFI_STATUS
EFIAPI
SnpReceiveFilters(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This,
    _In_ UINT32                      Enable,
    _In_ UINT32                      Disable,
    _In_ BOOLEAN                     ResetMCastFilter,
    _In_ UINTN                       MCastFilterCnt,
    _In_opt_ EFI_MAC_ADDRESS             *MCastFilter
    );

EFI_STATUS
EFIAPI
SnpStationAddress(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This,
    _In_ BOOLEAN                     Reset,
    _In_opt_ EFI_MAC_ADDRESS         *New
    );

EFI_STATUS
EFIAPI
SnpStatistics(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This,
    _In_ BOOLEAN                      Reset,
    _Inout_opt_ UINTN                  *StatisticsSize,
    _Inout_opt_ EFI_NETWORK_STATISTICS *StatisticsTable
    );

EFI_STATUS
EFIAPI
SnpMcastIpToMac(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This,
    _In_ BOOLEAN                     IPv6,
    _In_ EFI_IP_ADDRESS              *IP,
    _Out_ EFI_MAC_ADDRESS            *MAC
    );

EFI_STATUS
EFIAPI
SnpNvData(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This,
    _In_ BOOLEAN                     ReadWrite,
    _In_ UINTN                       Offset,
    _In_ UINTN                       BufferSize,
    _Inout_ VOID                    *Buffer
    );

EFI_STATUS
EFIAPI
SnpGetStatus(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL *This,
    _Out_opt_ UINT32                 *InterruptStatus,
    _Out_opt_ VOID                   **TxBuf
    );

EFI_STATUS
EFIAPI
SnpTransmit(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL     *This,
    _In_ UINTN                           HeaderSize,
    _In_ UINTN                           BufferSize,
    _In_reads_bytes_(BufferSize) VOID    *Buffer,
    _In_opt_ EFI_MAC_ADDRESS             *SrcAddr,
    _In_opt_ EFI_MAC_ADDRESS             *DestAddr,
    _In_opt_ UINT16                      *Protocol
    );

EFI_STATUS
EFIAPI
SnpReceive(
    _In_ EFI_SIMPLE_NETWORK_PROTOCOL         *This,
    _Out_opt_ UINTN                          *HeaderSize,
    _Inout_ UINTN                            *BufferSize,
    _Out_writes_bytes_(*BufferSize) VOID     *Buffer,
    _Out_opt_ EFI_MAC_ADDRESS                *SrcAddr,
    _Out_opt_ EFI_MAC_ADDRESS                *DestAddr,
    _Out_opt_ UINT16                         *Protocol
    );

VOID
EFIAPI
SnpWaitForPacketNotify(
    _In_ EFI_EVENT Event,
    _In_ VOID      *SnpPtr
    );

#define SNP_MEM_PAGES(x)  (((x) - 1) / 4096 + 1)

