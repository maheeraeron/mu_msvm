/*++

Copyright (c) Microsoft Corporation

Module Name:

    EfiHv.h

Abstract:

    Provides the protocol definition for EFI_HV_PROTOCOL, which provides
    UEFI access to the Hyper-V hypervisor.

Author:

    John Starks (jostarks) - 2-Jul-2012

--*/

#pragma once

#include <EfiNt.h>
#include <hvhdk.h>
#include <hvgdk.h>

#define EFI_HV_PROTOCOL_GUID { 0xa261a0f1, 0xaa53, 0x4c83, {0x94, 0xda, 0x12, 0x0c, 0xdf, 0x6d, 0x8c, 0x8d} }

typedef struct _EFI_HV_PROTOCOL EFI_HV_PROTOCOL;

typedef
VOID
(EFIAPI *EFI_HV_INTERRUPT_HANDLER)(
    __in VOID *Context
    );

typedef
EFI_STATUS
(EFIAPI *EFI_HV_CONNECT_SINT)(
    __in EFI_HV_PROTOCOL *This,
    __in_range(<, HV_SYNIC_SINT_COUNT) HV_SYNIC_SINT_INDEX SintIndex,
    __in UINT8 Vector,
    __in EFI_HV_INTERRUPT_HANDLER InterruptHandler,
    __in VOID *Context
    );

typedef
EFI_STATUS
(EFIAPI *EFI_HV_CONNECT_SINT_TO_EVENT)(
    __in EFI_HV_PROTOCOL *This,
    __in_range(<, HV_SYNIC_SINT_COUNT) HV_SYNIC_SINT_INDEX SintIndex,
    __in UINT8 Vector,
    __in EFI_EVENT Event
    );

typedef
VOID
(EFIAPI *EFI_HV_DISCONNECT_SINT)(
    __in EFI_HV_PROTOCOL *This,
    __in_range(<, HV_SYNIC_SINT_COUNT) HV_SYNIC_SINT_INDEX SintIndex
    );

typedef
HV_MESSAGE *
(EFIAPI *EFI_HV_GET_SINT_MESSAGE)(
    __in EFI_HV_PROTOCOL *This,
    __in_range(<, HV_SYNIC_SINT_COUNT) HV_SYNIC_SINT_INDEX SintIndex
    );

typedef
VOID
(EFIAPI *EFI_HV_COMPLETE_SINT_MESSAGE)(
    __in EFI_HV_PROTOCOL *This,
    __in_range(<, HV_SYNIC_SINT_COUNT) HV_SYNIC_SINT_INDEX SintIndex
    );

typedef
volatile HV_SYNIC_EVENT_FLAGS *
(EFIAPI *EFI_HV_GET_SINT_EVENT_FLAGS)(
    __in EFI_HV_PROTOCOL *This,
    __in_range(<, HV_SYNIC_SINT_COUNT) HV_SYNIC_SINT_INDEX SintIndex
    );

typedef
UINT64
(EFIAPI *EFI_HV_GET_REFERENCE_TIME)(
    __in EFI_HV_PROTOCOL *This
    );

typedef
UINT32
(EFIAPI *EFI_HV_GET_CURRENT_VP_INDEX)(
    __in EFI_HV_PROTOCOL *This
    );

typedef
EFI_STATUS
(EFIAPI *EFI_HV_CONFIGURE_TIMER)(
    __in EFI_HV_PROTOCOL *This,
    __in UINT32 TimerIndex,
    __in HV_SYNIC_SINT_INDEX SintIndex,
    __in BOOLEAN Periodic,
    __in BOOLEAN DirectMode,
    __in UINT8 Vector
    );

typedef
VOID
(EFIAPI *EFI_HV_SET_TIMER)(
    __in EFI_HV_PROTOCOL *This,
    __in UINT32 TimerIndex,
    __in UINT64 Expiration
    );

typedef
EFI_STATUS
(EFIAPI *EFI_HV_POST_MESSAGE)(
    __in EFI_HV_PROTOCOL *This,
    __in HV_CONNECTION_ID ConnectionId,
    __in HV_MESSAGE_TYPE MessageType,
    __in_bcount(PayloadSize) VOID *Payload,
    __in_range(0, HV_MESSAGE_PAYLOAD_BYTE_COUNT) UINT32 PayloadSize
    );

typedef
EFI_STATUS
(EFIAPI *EFI_HV_SIGNAL_EVENT)(
    __in EFI_HV_PROTOCOL *This,
    __in HV_CONNECTION_ID ConnectionId,
    __in UINT16 FlagNumber
    );

struct _EFI_HV_PROTOCOL
{
    EFI_HV_CONNECT_SINT ConnectSint;
    EFI_HV_CONNECT_SINT_TO_EVENT ConnectSintToEvent;
    EFI_HV_DISCONNECT_SINT DisconnectSint;

    EFI_HV_GET_SINT_MESSAGE GetSintMessage;
    EFI_HV_COMPLETE_SINT_MESSAGE CompleteSintMessage;
    EFI_HV_GET_SINT_EVENT_FLAGS GetSintEventFlags;

    EFI_HV_GET_REFERENCE_TIME GetReferenceTime;
    EFI_HV_GET_CURRENT_VP_INDEX GetCurrentVpIndex;

    EFI_HV_CONFIGURE_TIMER ConfigureTimer;
    EFI_HV_SET_TIMER SetTimer;

    EFI_HV_POST_MESSAGE PostMessage;
    EFI_HV_SIGNAL_EVENT SignalEvent;
};

extern GUID gEfiHvProtocolGuid;

