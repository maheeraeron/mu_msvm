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

#include <EfiNt.h>

#define EFI_VMBUS_LEGACY_PROTOCOL_GUID \
    {0x59e6efc9, 0x9695, 0x470a, {0x9d, 0x87, 0x2, 0x61, 0xd8, 0x45, 0x1d, 0xd8}}
#define EFI_VMBUS_PROTOCOL_GUID \
    {0x998629a6, 0xbbd0, 0x476b, {0x81, 0xef, 0x05, 0x99, 0x41, 0xe9, 0xe6, 0xf9}}
#define EFI_VMBUS_LEGACY_PROTOCOL_IVM_GUID \
    {0x8e03933f, 0x8048, 0x4a87, {0x81, 0x47, 0x7f, 0x05, 0xc3, 0x38, 0x28, 0x5d}}

#define EFI_VMBUS_PROTOCOL_FLAGS_PIPE_MODE  0x1

typedef struct _EFI_VMBUS_PROTOCOL EFI_VMBUS_PROTOCOL;
typedef struct _EFI_VMBUS_LEGACY_PROTOCOL EFI_VMBUS_LEGACY_PROTOCOL;

typedef struct _EFI_VMBUS_GPADL EFI_VMBUS_GPADL;
typedef UINT32 HV_MAP_GPA_FLAGS, *PHV_MAP_GPA_FLAGS;

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_CREATE_GPADL_LEGACY)(
    __in EFI_VMBUS_LEGACY_PROTOCOL *This,
    __in_bcount(BufferLength) VOID *Buffer,
    __in UINT32 BufferLength,
    __out UINT32 *GpadlHandle
    );

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_DESTROY_GPADL_LEGACY)(
    __in EFI_VMBUS_LEGACY_PROTOCOL *This,
    __in UINT32 GpadlHandle
    );

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_OPEN_CHANNEL_LEGACY)(
    __in EFI_VMBUS_LEGACY_PROTOCOL *This,
    __in UINT32 RingBufferGpadlHandle,
    __in UINT32 RingBufferPageOffset
    );

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_CLOSE_CHANNEL_LEGACY)(
    __in EFI_VMBUS_LEGACY_PROTOCOL *This
    );

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_REGISTER_ISR_LEGACY)(
    __in EFI_VMBUS_LEGACY_PROTOCOL *This,
    __in_opt EFI_EVENT Event
    );

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_SEND_INTERRUPT_LEGACY)(
    __in EFI_VMBUS_LEGACY_PROTOCOL *This
    );

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_PREPARE_GPADL)(
    __in EFI_VMBUS_PROTOCOL *This,
    __in_bcount(BufferLength) VOID *Buffer,
    __in UINT32 BufferLength,
    __in BOOLEAN ZeroPages,
    __in HV_MAP_GPA_FLAGS MapFlags,
    __out EFI_VMBUS_GPADL **Gpadl
    );

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_CREATE_GPADL)(
    __in EFI_VMBUS_PROTOCOL *This,
    __in EFI_VMBUS_GPADL *Gpadl
    );

typedef
UINT32
(EFIAPI *EFI_VMBUS_GET_GPADL_HANDLE)(
    __in EFI_VMBUS_PROTOCOL *This,
    __in EFI_VMBUS_GPADL *Gpadl
    );

typedef
PVOID
(EFIAPI *EFI_VMBUS_GET_GPADL_BUFFER)(
    __in EFI_VMBUS_PROTOCOL *This,
    __in EFI_VMBUS_GPADL *Gpadl
    );

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_DESTROY_GPADL)(
    __in EFI_VMBUS_PROTOCOL *This,
    __in EFI_VMBUS_GPADL *Gpadl
    );

typedef
EFI_STATUS
(EFIAPI *EFI_VMBUS_OPEN_CHANNEL)(
    __in EFI_VMBUS_PROTOCOL *This,
    __in EFI_VMBUS_GPADL *RingBufferGpadl,
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

struct _EFI_VMBUS_LEGACY_PROTOCOL
{
    EFI_VMBUS_CREATE_GPADL_LEGACY CreateGpadl;
    EFI_VMBUS_DESTROY_GPADL_LEGACY DestroyGpadl;

    EFI_VMBUS_OPEN_CHANNEL_LEGACY OpenChannel;
    EFI_VMBUS_CLOSE_CHANNEL_LEGACY CloseChannel;

    EFI_VMBUS_REGISTER_ISR_LEGACY RegisterIsr;
    EFI_VMBUS_SEND_INTERRUPT_LEGACY SendInterrupt;

    UINT32 Flags;
};

struct _EFI_VMBUS_PROTOCOL
{
    EFI_VMBUS_PREPARE_GPADL PrepareGpadl;
    EFI_VMBUS_CREATE_GPADL CreateGpadl;
    EFI_VMBUS_DESTROY_GPADL DestroyGpadl;
    EFI_VMBUS_GET_GPADL_BUFFER GetGpadlBuffer;
    EFI_VMBUS_GET_GPADL_HANDLE GetGpadlHandle;

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
extern EFI_GUID gEfiVmbusLegacyProtocolGuid;
