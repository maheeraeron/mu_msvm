/** @file
  Ring buffer types, definitions and public functions.
  See EfiRing.c for implementation details.

  Copyright (c) Microsoft Corporation.
  Licensed under the BSD-2-Clause-Patent license.
**/

#pragma once

typedef UINT64   RING_HANDLE;
typedef UINT32   RING_HANDLE_KEY;

#define INVALID_RING_HANDLE     ((UINT64)(-1))


//
// Overwrite older events if the buffer gets full
// if this is not specified, new events are dropped
// once the buffer is full.
//
#define RING_BUFFER_OVERWRITE   0x00000001


typedef struct
{
    UINT32  LostWrites;
    UINT32  Reserve;
    UINT32  Remove;
} RING_BUFFER_STATS;


typedef struct
{
    //
    // Size of the buffer, must be a power of 2
    //
    UINT32              Size;
    //
    // Mask for offset wrapping
    //
    UINT32              Mask;
    //
    // Offset within the buffer where the next record will be written
    //
    UINT32              Head;
    //
    // Offset within the buffer of the oldest record.
    //
    UINT32              Tail;
    //
    // Behavior flags, see RING_BUFFER_nnnnn
    //
    UINT32              Flags;

    RING_BUFFER_STATS   Stats;
    //
    // Used to invalidate data handles for destructive ring operation.
    //
    RING_HANDLE_KEY     HandleKey;

    _Field_size_(Size)
    UINT8               Buffer[];
} EFI_RING_BUFFER;

//
// IO operations that can be performed
// See RingBufferIo
//
typedef enum
{
    DataRead,
    DataWrite
} RING_IO_OPERATION;


EFI_STATUS
RingBufferReserve(
    _In_        EFI_RING_BUFFER        *Ring,
    _In_        const UINT32            DataSize,
    _Out_opt_   RING_HANDLE            *DataHandle
    );


EFI_STATUS
RingBufferAdd(
    _In_        EFI_RING_BUFFER        *Ring,
    _In_bytecount_(DataSize)
                const VOID             *Data,
    _In_        const UINT32            DataSize,
    _Out_opt_   RING_HANDLE            *DataHandle
    );


EFI_STATUS
RingBufferRemove(
    _In_        EFI_RING_BUFFER        *Ring,
    _Out_writes_bytes_to_opt_(*DataSize, *DataSize)
                VOID                   *Data,
    _Inout_opt_ UINT32                 *DataSize
    );


EFI_STATUS
RingBufferIo(
    _In_        EFI_RING_BUFFER        *Ring,
    _In_        const RING_HANDLE       DataHandle,
    _In_        const RING_IO_OPERATION Op,
    _In_        const UINT32            Offset,
    _When_(Op == DataRead, _Out_writes_bytes_to_(*DataSize, *DataSize))
    _When_(Op == DataWrite, _In_bytecount_(*DataSize))
                VOID                   *Data,
    _Inout_     UINT32                 *DataSize
    );


EFI_STATUS
RingBufferHandleEnumerate(
    _In_        const EFI_RING_BUFFER  *Ring,
    _Inout_     RING_HANDLE            *Enumerator,
    _Out_       RING_HANDLE            *Item,
    _Out_opt_   UINT32                 *ItemSize
    );


EFI_STATUS
RingBufferHandleIsValid(
    _In_        EFI_RING_BUFFER        *Ring,
    _In_        const RING_HANDLE       DataHandle
    );


VOID
RingBufferReset(
    _In_        EFI_RING_BUFFER        *Ring
    );


EFI_STATUS
RingBufferFlatten(
    _In_        const EFI_RING_BUFFER  *Ring,
    _Inout_     UINT32                 *BufferSize,
    _Out_bytecap_post_bytecount_(*BufferSize, *BufferSize)
                VOID                   *Buffer
    );

EFI_STATUS
RingBufferInitialize(
    _In_bytecount_(sizeof(EFI_RING_BUFFER) + Capacity)
                EFI_RING_BUFFER        *Ring,
    _In_        const UINT32            Capacity,
    _In_        const UINT32            Flags
    );
