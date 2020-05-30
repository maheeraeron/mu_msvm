/*++

Copyright (c) Microsoft Corporation

Module Name:

    StorvscDxe.h

Abstract:

    EFI Driver for Synthetic SCSI Controller.

Author:

    Marius Buleandra (mariub) - 20-Jul-2012

--*/


#pragma once

#include <Uefi.h>
#include <EfiNt.h>

#include <Protocol/Vmbus.h>
#include <Protocol/Emcl.h>
#include <Protocol/ScsiPassThruExt.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/EmclLib.h>

#include <VstorageProtocol.h>


#define STORVSC_VERSION 1
#define STORVSC_ADAPTER_CONTEXT_SIGNATURE SIGNATURE_32 ('S','V','s','c')

#define VMSTOR_MAX_TARGETS 2
#define TPL_STORVSC_CALLBACK (TPL_CALLBACK + 1)
#define TPL_STORVSC_NOTIFY TPL_NOTIFY

// TODO-19259739: Have a better way of reporting UEFI errors.
#define STORVSC_FAIL_FAST() \
                    ASSERT(FALSE); \
                    CpuDeadLoop();

#define STORVSC_FAIL_FAST_IF_FALSE(cond) \
                    if (!(cond)) {STORVSC_FAIL_FAST()}


#define STORVSC_MAX_LUN_TRANSFER_LENGTH (sizeof(UCHAR) * 8 * SCSI_MAXIMUM_LUNS_PER_TARGET)


typedef struct _STORVSC_CHANNEL_CONTEXT
{
    EFI_EMCL_V2_PROTOCOL *Emcl;
    VMSTORAGE_CHANNEL_PROPERTIES Properties;
    UINT16 ProtocolVersion;

    UINT16 MaxPacketSize;
    UINT16 MaxSrbLength;
    UINT8 MaxSrbSenseDataLength;
} STORVSC_CHANNEL_CONTEXT, *PSTORVSC_CHANNEL_CONTEXT;

typedef struct _STORVSC_ADAPTER_CONTEXT
{
    UINTN Signature;
    EFI_HANDLE Handle;

    EFI_EMCL_V2_PROTOCOL *Emcl;
    EFI_EXT_SCSI_PASS_THRU_PROTOCOL ExtScsiPassThru;
    EFI_EXT_SCSI_PASS_THRU_MODE ExtScsiPassThruMode;

    PSTORVSC_CHANNEL_CONTEXT ChannelContext;
    LIST_ENTRY LunList;
} STORVSC_ADAPTER_CONTEXT, *PSTORVSC_ADAPTER_CONTEXT;

typedef struct _STORVSC_CHANNEL_REQUEST
{
    EFI_EXT_SCSI_PASS_THRU_SCSI_REQUEST_PACKET *ScsiRequest;
    EFI_EVENT Event;
} STORVSC_CHANNEL_REQUEST, *PSTORVSC_CHANNEL_REQUEST;

typedef struct _TARGET_LUN
{
    LIST_ENTRY ListEntry;
    UCHAR TargetId;
    UCHAR Lun;
} TARGET_LUN, *PTARGET_LUN;


#define STORVSC_ADAPTER_CONTEXT_FROM_EXT_SCSI_PASS_THRU_THIS(a) \
    CR( \
        a, \
        STORVSC_ADAPTER_CONTEXT, \
        ExtScsiPassThru, \
        STORVSC_ADAPTER_CONTEXT_SIGNATURE \
        )


extern EFI_DRIVER_BINDING_PROTOCOL gStorvscDriverBinding;
extern EFI_COMPONENT_NAME2_PROTOCOL gStorvscComponentName2;
extern EFI_COMPONENT_NAME_PROTOCOL gStorvscComponentName;


EFI_STATUS
EFIAPI
StorvscDriverEntryPoint (
    __in EFI_HANDLE ImageHandle,
    __in EFI_SYSTEM_TABLE *SystemTable
    );

EFI_STATUS
EFIAPI
StorvscDriverBindingSupported (
    __in EFI_DRIVER_BINDING_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in_opt EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
    );

EFI_STATUS
EFIAPI
StorvscDriverBindingStart (
    __in EFI_DRIVER_BINDING_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in_opt EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
    );

EFI_STATUS
EFIAPI
StorvscDriverBindingStop (
    __in EFI_DRIVER_BINDING_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in UINTN NumberOfChildren,
    __in_ecount(NumberOfChildren) EFI_HANDLE *ChildHandleBuffer
    );

EFI_STATUS
EFIAPI
StorvscComponentNameGetDriverName (
    __in EFI_COMPONENT_NAME2_PROTOCOL *This,
    __in CHAR8 *Language,
    __out CHAR16 **DriverName
    );

EFI_STATUS
EFIAPI
StorvscComponentNameGetControllerName (
    __in EFI_COMPONENT_NAME2_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in_opt EFI_HANDLE ChildHandle,
    __in CHAR8 *Language,
    __out CHAR16 **ControllerName
    );

EFI_STATUS
EFIAPI
StorvscExtScsiPassThruPassThru (
    __in EFI_EXT_SCSI_PASS_THRU_PROTOCOL *This,
    __in UINT8 *Target,
    __in UINT64 Lun,
    __inout EFI_EXT_SCSI_PASS_THRU_SCSI_REQUEST_PACKET *Packet,
    __in_opt EFI_EVENT Event
    );

EFI_STATUS
EFIAPI
StorvscExtScsiPassThruGetNextTargetLun (
    IN EFI_EXT_SCSI_PASS_THRU_PROTOCOL *This,
    IN OUT UINT8 **Target,
    IN OUT UINT64 *Lun
    );

EFI_STATUS
EFIAPI
StorvscExtScsiPassThruBuildDevicePath (
    IN EFI_EXT_SCSI_PASS_THRU_PROTOCOL *This,
    IN UINT8 *Target,
    IN UINT64 Lun,
    IN OUT EFI_DEVICE_PATH_PROTOCOL **DevicePath
    );

EFI_STATUS
EFIAPI
StorvscExtScsiPassThruGetTargetLun (
    IN EFI_EXT_SCSI_PASS_THRU_PROTOCOL *This,
    IN EFI_DEVICE_PATH_PROTOCOL *DevicePath,
    OUT UINT8 **Target,
    OUT UINT64 *Lun
    );

EFI_STATUS
EFIAPI
StorvscExtScsiPassThruResetChannel (
    IN EFI_EXT_SCSI_PASS_THRU_PROTOCOL *This
    );

EFI_STATUS
EFIAPI
StorvscExtScsiPassThruResetTargetLun (
    IN EFI_EXT_SCSI_PASS_THRU_PROTOCOL *This,
    IN UINT8 *Target,
    IN UINT64 Lun
    );

EFI_STATUS
EFIAPI
StorvscExtScsiPassThruGetNextTarget (
    IN EFI_EXT_SCSI_PASS_THRU_PROTOCOL *This,
    IN OUT UINT8 **Target
    );

EFI_STATUS
StorChannelOpen (
    __in EFI_EMCL_V2_PROTOCOL* Emcl,
    __out PSTORVSC_CHANNEL_CONTEXT *ChannelContext
    );

VOID
StorChannelClose (
    __in PSTORVSC_CHANNEL_CONTEXT ChannelContext
    );

EFI_STATUS
StorChannelInitScsiPacket (
    __in EFI_EXT_SCSI_PASS_THRU_SCSI_REQUEST_PACKET *ScsiRequest,
    __in UINT8 *Target,
    __in UINT64 Lun,
    __out VSTOR_PACKET *Packet,
    __out EFI_EXTERNAL_BUFFER *ExternalBuffer
    );

VOID
StorChannelCopyPacketDataToRequest (
    __in PVSTOR_PACKET Packet,
    __inout EFI_EXT_SCSI_PASS_THRU_SCSI_REQUEST_PACKET *ScsiRequest
    );

VOID
StorChannelCompletionRoutine (
    __in_opt VOID *Context,
    __in_bcount(BufferLength) VOID *Buffer,
    __in UINT32 BufferLength
    );

EFI_STATUS
StorChannelSendScsiRequest (
    __in PSTORVSC_CHANNEL_CONTEXT ChannelContext,
    __inout EFI_EXT_SCSI_PASS_THRU_SCSI_REQUEST_PACKET *ScsiRequest,
    __in UINT8 *Target,
    __in UINT64 Lun,
    __in_opt EFI_EVENT Event
    );

EFI_STATUS
StorChannelSendScsiRequestSync (
    __in PSTORVSC_CHANNEL_CONTEXT ChannelContext,
    __inout EFI_EXT_SCSI_PASS_THRU_SCSI_REQUEST_PACKET *ScsiRequest,
    __in UINT8 *Target,
    __in UINT64 Lun
    );

VOID
StorChannelReceivePacketCallback (
    __in VOID *ReceiveContext,
    __in VOID *PacketContext,
    __in_bcount_opt(BufferLength) VOID *Buffer,
    __in UINT32 BufferLength,
    __in UINT16 TransferPageSetId,
    __in UINT32 RangeCount,
    __in_ecount(RangeCount) EFI_TRANSFER_RANGE *Ranges
    );

VOID
StorChannelInitSyntheticVstorPacket (
    __out PVSTOR_PACKET Packet
    );

EFI_STATUS
StorChannelSendSyntheticVstorPacket (
    __in PSTORVSC_CHANNEL_CONTEXT ChannelContext,
    __inout PVSTOR_PACKET Packet
    );

EFI_STATUS
StorChannelEstablishCommunications (
    __in PSTORVSC_CHANNEL_CONTEXT ChannelContext
    );

VOID
StorChannelTeardownReportLunsRequest (
    __inout EFI_EXT_SCSI_PASS_THRU_SCSI_REQUEST_PACKET *Request
    );

EFI_STATUS
StorChannelBuildLunList(
    __in PSTORVSC_CHANNEL_CONTEXT ChannelContext,
    __out LIST_ENTRY *LunList
    );

VOID
StorChannelFreeLunList(
    __inout LIST_ENTRY *LunList
    );

LIST_ENTRY*
StorChannelSearchLunList (
    __in LIST_ENTRY *LunList,
    __in UCHAR Target,
    __in UCHAR Lun
    );

