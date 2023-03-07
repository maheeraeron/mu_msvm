///
/// \copyright  Copyright (c) Microsoft Corporation. All Rights Reserved.
///
/// \file VpcivscDxe.h
///
/// \brief Header definitions for the VPCI VSC UEFI driver.
///
/// \author Chris Oo (cho)
/// \date Aug 9, 2019
///

#pragma once

#include <Uefi.h>
#include <PiDxe.h>
#include <EfiNt.h>
#include <FailFast.h>

#include <Protocol/Vmbus.h>
#include <Protocol/Emcl.h>
#include <Protocol/PciIo.h>
#include <Protocol/ComponentName.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/DevicePath.h>

#include "VpciInternal.h"
#include "PciBars.h"

typedef struct _VPCIVSC_CONTEXT VPCIVSC_CONTEXT;

typedef struct _VPCI_BAR_INFORMATION
{
    UINT64 MappedAddress; // The address where this bar was allocated and mapped
    UINT64 Size; // The size of this bar
    BOOLEAN Is64Bit; // The type of bar, 32 bit or 64 bit
    UINT8 BarIndex; // The index into the RawBars array for where this bar starts
} VPCI_BAR_INFORMATION, *PVPCI_BAR_INFORMATION;

// Context structure for each VPCI device
typedef struct _VPCI_DEVICE_CONTEXT
{
    UINTN Signature;

    // PCI Io Protocol for this device
    EFI_PCI_IO_PROTOCOL PciIo;

    EFI_HANDLE Handle;
    EFI_DEVICE_PATH_PROTOCOL *DevicePath;

    // The raw bar information returned from the VSP
    PCI_BAR_FORMAT RawBars[PCI_TYPE0_BAR_COUNT];
    VPCI_BAR_INFORMATION MappedBars[PCI_TYPE0_BAR_COUNT];

    VPCIVSC_CONTEXT *VpcivscContext;
    PCI_SLOT_NUMBER Slot;

} VPCI_DEVICE_CONTEXT, *PVPCI_DEVICE_CONTEXT;

#define VPCI_DEVICE_CONTEXT_SIGNATURE SIGNATURE_32('v', 'p', 'c', 'd')

#define VPCI_DEVICE_CONTEXT_FROM_PCI_IO(a) \
    CR(a, \
       VPCI_DEVICE_CONTEXT, \
       PciIo, \
       VPCI_DEVICE_CONTEXT_SIGNATURE \
       )

// Context structure for each VPCI channel offer
typedef struct _VPCIVSC_CONTEXT
{
    UINTN Signature;
    EFI_HANDLE Handle;

    EFI_EMCL_V2_PROTOCOL *Emcl;
    EFI_DEVICE_PATH_PROTOCOL *DevicePath;

    EFI_EVENT WaitForBusRelationsMessage;
    VPCI_DEVICE_DESCRIPTION *Devices;
    UINT32 DeviceCount;

    // We only really care about NVMe devices.
    VPCI_DEVICE_CONTEXT *NvmeDevices;
    UINT32 NvmeDeviceCount;
} VPCIVSC_CONTEXT, *PVPCIVSC_CONTEXT;

#define VPCIVSC_DRIVER_VERSION 0x1
#define VPCIVSC_CONTEXT_SIGNATURE SIGNATURE_32('v','p','c','i')

#define VPCIVSC_CONTEXT_FROM_EMCL(a) \
    CR(a, \
       VPCIVSC_CONTEXT, \
       Emcl, \
       VPCIVSC_CONTEXT_SIGNATURE \
       )

#define TPL_VPCIVSC_CALLBACK TPL_CALLBACK
#define DEBUG_VPCI_INFO DEBUG_INFO

#define VPCIVSC 0x56504349565343 // "VPCIVSC"

#define VPCIVSC_WAIT_FOR_HOST_TIMEOUT  EFI_TIMER_PERIOD_SECONDS(60)

extern EFI_DRIVER_BINDING_PROTOCOL gVpcivscDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gVpcivscComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL gVpcivscComponentName2;

EFI_STATUS
EFIAPI
VpcivscDriverBindingSupported (
    __in EFI_DRIVER_BINDING_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in_opt EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
    );

EFI_STATUS
EFIAPI
VpcivscDriverBindingStop (
    __in EFI_DRIVER_BINDING_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in UINTN NumberOfChildren,
    __in_ecount(NumberOfChildren) EFI_HANDLE *ChildHandleBuffer
    );

EFI_STATUS
EFIAPI
VpcivscDriverBindingStart (
    __in EFI_DRIVER_BINDING_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in_opt EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
    );

EFI_STATUS
EFIAPI
VpcivscComponentNameGetDriverName(
    __in EFI_COMPONENT_NAME_PROTOCOL *This,
    __in CHAR8 *Language,
    __out CHAR16 **DriverName
    );

EFI_STATUS
EFIAPI
VpcivscComponentNameGetControllerName(
    __in EFI_COMPONENT_NAME_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in_opt EFI_HANDLE ChildHandle,
    __in CHAR8 *Language,
    __out CHAR16 **ControllerName
    );

// EMCL functions, VSC protocol functions
VOID
VpciChannelReceivePacketCallback(
    __in VOID *ReceiveContext,
    __in VOID *PacketContext,
    __in_bcount_opt(BufferLength) VOID *Buffer,
    __in UINT32 BufferLength,
    __in UINT16 TransferPageSetId,
    __in UINT32 RangeCount,
    __in_ecount(RangeCount) EFI_TRANSFER_RANGE *Ranges
    );

// PciIo Protocol functions
EFI_STATUS
EFIAPI
VpcivscPciIoPollMem(
    _In_  EFI_PCI_IO_PROTOCOL          *This,
    _In_  EFI_PCI_IO_PROTOCOL_WIDTH    Width,
    _In_  UINT8                        BarIndex,
    _In_  UINT64                       Offset,
    _In_  UINT64                       Mask,
    _In_  UINT64                       Value,
    _In_  UINT64                       Delay,
    _Out_ UINT64                       *Result
    );

EFI_STATUS
EFIAPI
VpcivscPciIoPollIo(
    _In_  EFI_PCI_IO_PROTOCOL        *This,
    _In_  EFI_PCI_IO_PROTOCOL_WIDTH  Width,
    _In_  UINT8                      BarIndex,
    _In_  UINT64                     Offset,
    _In_  UINT64                     Mask,
    _In_  UINT64                     Value,
    _In_  UINT64                     Delay,
    _Out_ UINT64                     *Result
    );

EFI_STATUS
EFIAPI
VpcivscPciIoMemRead(
    _In_     EFI_PCI_IO_PROTOCOL        *This,
    _In_     EFI_PCI_IO_PROTOCOL_WIDTH  Width,
    _In_     UINT8                      BarIndex,
    _In_     UINT64                     Offset,
    _In_     UINTN                      Count,
    _Inout_ VOID                       *Buffer
    );

EFI_STATUS
EFIAPI
VpcivscPciIoMemWrite(
    _In_     EFI_PCI_IO_PROTOCOL        *This,
    _In_     EFI_PCI_IO_PROTOCOL_WIDTH  Width,
    _In_     UINT8                      BarIndex,
    _In_     UINT64                     Offset,
    _In_     UINTN                      Count,
    _Inout_ VOID                       *Buffer
    );

EFI_STATUS
EFIAPI
VpcivscPciIoIoRead(
    _In_     EFI_PCI_IO_PROTOCOL        *This,
    _In_     EFI_PCI_IO_PROTOCOL_WIDTH  Width,
    _In_     UINT8                      BarIndex,
    _In_     UINT64                     Offset,
    _In_     UINTN                      Count,
    _Inout_ VOID                       *Buffer
    );

EFI_STATUS
EFIAPI
VpcivscPciIoIoWrite(
    _In_     EFI_PCI_IO_PROTOCOL        *This,
    _In_     EFI_PCI_IO_PROTOCOL_WIDTH  Width,
    _In_     UINT8                      BarIndex,
    _In_     UINT64                     Offset,
    _In_     UINTN                      Count,
    _Inout_ VOID                       *Buffer
    );

EFI_STATUS
EFIAPI
VpcivscPciIoConfigRead(
    _In_     EFI_PCI_IO_PROTOCOL        *This,
    _In_     EFI_PCI_IO_PROTOCOL_WIDTH  Width,
    _In_     UINT32                     Offset,
    _In_     UINTN                      Count,
    _Inout_ VOID                       *Buffer
    );

EFI_STATUS
EFIAPI
VpcivscPciIoConfigWrite(
    _In_     EFI_PCI_IO_PROTOCOL        *This,
    _In_     EFI_PCI_IO_PROTOCOL_WIDTH  Width,
    _In_     UINT32                     Offset,
    _In_     UINTN                      Count,
    _Inout_ VOID                       *Buffer
    );

EFI_STATUS
EFIAPI
VpcivscPciIoCopyMem(
    IN EFI_PCI_IO_PROTOCOL              *This,
    _In_     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
    _In_     UINT8                        DestBarIndex,
    _In_     UINT64                       DestOffset,
    _In_     UINT8                        SrcBarIndex,
    _In_     UINT64                       SrcOffset,
    _In_     UINTN                        Count
    );

EFI_STATUS
EFIAPI
VpcivscPciIoMap(
    _In_     EFI_PCI_IO_PROTOCOL            *This,
    _In_     EFI_PCI_IO_PROTOCOL_OPERATION  Operation,
    _In_     VOID                           *HostAddress,
    _Inout_ UINTN                          *NumberOfBytes,
    _Out_    EFI_PHYSICAL_ADDRESS           *DeviceAddress,
    _Out_    VOID                           **Mapping
    );

EFI_STATUS
EFIAPI
VpcivscPciIoUnmap(
    _In_  EFI_PCI_IO_PROTOCOL  *This,
    _In_  VOID                 *Mapping
    );

EFI_STATUS
EFIAPI
VpcivscPciIoAllocateBuffer(
    _In_  EFI_PCI_IO_PROTOCOL   *This,
    _In_  EFI_ALLOCATE_TYPE     Type,
    _In_  EFI_MEMORY_TYPE       MemoryType,
    _In_  UINTN                 Pages,
    _Out_ VOID                  **HostAddress,
    _In_  UINT64                Attributes
    );

EFI_STATUS
EFIAPI
VpcivscPciIoFreeBuffer(
    _In_  EFI_PCI_IO_PROTOCOL   *This,
    _In_  UINTN                 Pages,
    _In_  VOID                  *HostAddress
    );

EFI_STATUS
EFIAPI
VpcivscPciIoFlush(
    _In_  EFI_PCI_IO_PROTOCOL  *This
    );

EFI_STATUS
EFIAPI
VpcivscPciIoGetLocation(
    _In_  EFI_PCI_IO_PROTOCOL  *This,
    _Out_ UINTN                *Segment,
    _Out_ UINTN                *Bus,
    _Out_ UINTN                *Device,
    _Out_ UINTN                *Function
    );

EFI_STATUS
EFIAPI
VpcivscPciIoAttributes(
    _In_ EFI_PCI_IO_PROTOCOL                       * This,
    _In_  EFI_PCI_IO_PROTOCOL_ATTRIBUTE_OPERATION  Operation,
    _In_  UINT64                                   Attributes,
    _Out_ UINT64                                   *Result OPTIONAL
    );

EFI_STATUS
EFIAPI
VpcivscPciIoGetBarAttributes(
    _In_ EFI_PCI_IO_PROTOCOL             * This,
    _In_  UINT8                          BarIndex,
    _Out_ UINT64                         *Supports, OPTIONAL
    _Out_ VOID                           **Resources OPTIONAL
    );

EFI_STATUS
EFIAPI
VpcivscPciIoSetBarAttributes(
    _In_ EFI_PCI_IO_PROTOCOL              *This,
    _In_     UINT64                       Attributes,
    _In_     UINT8                        BarIndex,
    _Inout_ UINT64                       *Offset,
    _Inout_ UINT64                       *Length
    );