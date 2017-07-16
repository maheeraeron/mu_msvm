/*++

Copyright (c) Microsoft Corporation

Module Name:

    Emcl.h

Abstract:

    Provides the protocol definition for EFI_EMCL_PROTOCOL, which provides
    ring buffer management and packet transport for VMBus channels.

Author:

    Arseney Romanenko (arseneyr) - 16-Jul-2012

--*/

#pragma once

#define EFI_EMCL_PROTOCOL_GUID \
    {0x7cbfb8f7, 0xdd49, 0x4699, {0xa1, 0x63, 0xa6, 0xa6, 0xbf, 0x88, 0x13, 0xdf}}

#define TPL_EMCL (TPL_HIGH_LEVEL - 1)

typedef struct _EFI_EMCL_PROTOCOL EFI_EMCL_PROTOCOL;

typedef struct _EFI_TRANSFER_RANGE
{
    UINT32 ByteCount;
    UINT32 ByteOffset;

} EFI_TRANSFER_RANGE;

typedef struct _EFI_EXTERNAL_BUFFER
{
    VOID *Buffer;
    UINT32 BufferSize;

} EFI_EXTERNAL_BUFFER;

typedef
VOID
(*EFI_EMCL_COMPLETION_ROUTINE)(
    __in_opt VOID *Context,
    __in_bcount(BufferLength) VOID *Buffer,
    __in UINT32 BufferLength
    );

typedef
VOID
(*EFI_EMCL_RECEIVE_PACKET)(
    __in VOID *ReceiveContext,
    __in VOID *PacketContext,
    __in_bcount_opt(BufferLength) VOID *Buffer,
    __in UINT32 BufferLength,
    __in UINT16 TransferPageSetId,
    __in UINT32 RangeCount,
    __in_ecount(RangeCount) EFI_TRANSFER_RANGE *Ranges
    );

typedef
EFI_STATUS
(EFIAPI *EFI_EMCL_START_CHANNEL)(
    __in EFI_EMCL_PROTOCOL *This,
    __in UINT32 IncomingRingBufferPageCount,
    __in UINT32 OutgoingRingBufferPageCount
    );

typedef
VOID
(EFIAPI *EFI_EMCL_STOP_CHANNEL)(
    __in EFI_EMCL_PROTOCOL *This
    );

typedef
EFI_STATUS
(EFIAPI *EFI_EMCL_SEND_PACKET)(
    __in EFI_EMCL_PROTOCOL *This,
    __in_bcount(InlineBufferLength) VOID *InlineBuffer,
    __in UINT32 InlineBufferLength,
    __in_ecount(ExternalBufferCount) EFI_EXTERNAL_BUFFER *ExternalBuffers,
    __in UINT32 ExternalBufferCount,
    __in_opt EFI_EMCL_COMPLETION_ROUTINE CompletionRoutine,
    __in_opt VOID *CompletionContext
    );

typedef
EFI_STATUS
(EFIAPI *EFI_EMCL_COMPLETE_PACKET)(
    __in EFI_EMCL_PROTOCOL *This,
    __in VOID *PacketContext,
    __in_bcount(BufferLength) VOID *Buffer,
    __in UINT32 BufferLength
    );

typedef
EFI_STATUS
(EFIAPI *EFI_EMCL_SET_RECEIVE_CALLBACK)(
    __in EFI_EMCL_PROTOCOL *This,
    __in_opt EFI_EMCL_RECEIVE_PACKET ReceiveCallback,
    __in_opt VOID *ReceiveContext,
    __in_range(<=, TPL_EMCL) EFI_TPL Tpl
    );

typedef
EFI_STATUS
(EFIAPI *EFI_EMCL_CREATE_GPADL)(
    __in EFI_EMCL_PROTOCOL *This,
    __in_bcount(BufferLength) VOID *Buffer,
    __in UINT32 BufferLength,
    __out UINT32 *GpadlHandle
    );

typedef
EFI_STATUS
(EFIAPI *EFI_EMCL_DESTROY_GPADL)(
    __in EFI_EMCL_PROTOCOL *This,
    __in UINT32 GpadlHandle
    );

typedef
EFI_STATUS
(EFIAPI *EFI_EMCL_CREATE_GPA_RANGE)(
    __in EFI_EMCL_PROTOCOL *This,
    __in UINT32 Handle,
    __in_ecount(ExternalBufferCount) EFI_EXTERNAL_BUFFER *ExternalBuffers,
    __in UINT32 ExternalBufferCount,
    __in BOOLEAN Writable
    );

typedef
EFI_STATUS
(EFIAPI *EFI_EMCL_DESTROY_GPA_RANGE)(
    __in EFI_EMCL_PROTOCOL *This,
    __in UINT32 Handle
    );

struct _EFI_EMCL_PROTOCOL
{
    EFI_EMCL_START_CHANNEL StartChannel;
    EFI_EMCL_STOP_CHANNEL StopChannel;

    EFI_EMCL_SEND_PACKET SendPacket;
    EFI_EMCL_COMPLETE_PACKET CompletePacket;
    EFI_EMCL_SET_RECEIVE_CALLBACK SetReceiveCallback;

    EFI_EMCL_CREATE_GPADL CreateGpadl;
    EFI_EMCL_DESTROY_GPADL DestroyGpadl;

    EFI_EMCL_CREATE_GPA_RANGE CreateGpaRange;
    EFI_EMCL_DESTROY_GPA_RANGE DestroyGpaRange;
};

extern EFI_GUID gEfiEmclProtocolGuid;
