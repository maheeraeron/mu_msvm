/*++

Copyright (c) Microsoft Corporation

Module Name:

    Vmbus.h

Abstract:

    Provides the protocol definition for EFI_VMBUS_PROTOCOL, which manages
    VMBus channels.

Author:

    Arseney Romanenko (arseneyr) - 16-Jul-2012

--*/

#pragma once

#define EFI_VMBUS_PROTOCOL_GUID \
    {0x59e6efc9, 0x9695, 0x470a, {0x9d, 0x87, 0x2, 0x61, 0xd8, 0x45, 0x1d, 0xd8}}

#define EFI_VMBUS_PROTOCOL_FLAGS_PIPE_MODE  0x1

typedef struct _EFI_VMBUS_PROTOCOL EFI_VMBUS_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_CREATE_GPADL)(
    __in EFI_VMBUS_PROTOCOL *This,
    __in_bcount(BufferLength) VOID *Buffer,
    __in UINT32 BufferLength,
    __out UINT32 *GpadlHandle
    );

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_DESTROY_GPADL)(
    __in EFI_VMBUS_PROTOCOL *This,
    __in UINT32 GpadlHandle
    );

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_OPEN_CHANNEL)(
    __in EFI_VMBUS_PROTOCOL *This,
    __in UINT32 RingBufferGpadlHandle,
    __in UINT32 RingBufferPageOffset
    );

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_CLOSE_CHANNEL)(
    __in EFI_VMBUS_PROTOCOL *This
    );

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_REGISTER_ISR)(
    __in EFI_VMBUS_PROTOCOL *This,
    __in_opt EFI_EVENT Event
    );

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_SEND_INTERRUPT)(
    __in EFI_VMBUS_PROTOCOL *This
    );

struct _EFI_VMBUS_PROTOCOL
{
    EFI_VMBUS_CREATE_GPADL CreateGpadl;
    EFI_VMBUS_DESTROY_GPADL DestroyGpadl;

    EFI_VMBUS_OPEN_CHANNEL OpenChannel;
    EFI_VMBUS_CLOSE_CHANNEL CloseChannel;

    EFI_VMBUS_REGISTER_ISR RegisterIsr;
    EFI_VMBUS_SEND_INTERRUPT SendInterrupt;

    UINT32 Flags;
};

typedef struct _VMBUS_DEVICE_PATH
{
    VENDOR_DEVICE_PATH VendorDevicePath;

    GUID InterfaceType;
    GUID InterfaceInstance;

} VMBUS_DEVICE_PATH;

extern EFI_GUID gEfiVmbusProtocolGuid;

