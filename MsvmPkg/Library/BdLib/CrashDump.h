/*++

Copyright (c) Microsoft Corporation

Module Name:

    CrashDump.h

Abstract:

    Internal functions and types for UEFI crash dump support.

Author:

    Kris Harper (kharp) 16-Sep-2013

Environment:

    UEFI

--*/
#pragma once

#include "MemoryDumpRequest.h"

#define KMODE_EXCEPTION_NOT_HANDLED ((ULONG)0x0000001EL)

/**
 *  An EFI_DUMP_BUFFER is used to build a packed list
 *  of memory dump blocks.  It, along with the Efi Dump Buffer
 *  functions, serves as a mini allocator to assist in packing
 *  variable length blocks into one contiguous memory block.
 *
 */
typedef struct
{
    UINT8*  Buffer;
    UINT32  BufferSize;
    UINT32  Offset;
    UINT32  BlockCount;

}EFI_DUMP_BUFFER;

//
// Dump buffer allocated for use by crash dump routines.
//
extern EFI_DUMP_BUFFER      BdDumpBuffer;
extern MemoryDumpType       BdDumpType;


/**
  Initializes a dump buffer with the given
  backing buffer and size.
 
  @param    DumpBuffer  EFI_DUMP_BUFFER to initialize
  @param    Buffer      Backing store buffer to use
  @param    BufferSize  Size of backing store.

**/
VOID
EfiDumpBufferInitialize(
    _Inout_     EFI_DUMP_BUFFER*    DumpBuffer,
    _In_bytecount_(BufferSize)
                UINT8*              Buffer,
    _In_        UINT32              BufferSize
    );


/**
  Allocates space for the given block type and initializes the block header

  @param    DumpBuffer  EFI_DUMP_BUFFER to use
  @param    Type        Block type to allocate
  @param    BlockStart  On successfull return contains a pointer to the start
                        of the allocated block.

  @return  TRUE on success, FALSE on failure (not enough space)
  
**/
BOOLEAN
EfiDumpBufferAllocateBlock(
    _Inout_     EFI_DUMP_BUFFER*    DumpBuffer,
    _In_        MemoryBlockType     Type,
    _Outptr_    VOID**              BlockStart
    );


/**
  Allocates space for the given block type plus additional requested space.
  The block header is initialized as well.

  @param    DumpBuffer      EFI_DUMP_BUFFER to use
  @param    Type            Block type to allocate
  @param    AdditionalSize  Number of additional bytes to allocate along with the
                            standard block structure.
  @param    BlockStart      On successfull return contains a pointer to the start
                            of the allocated block.

  @return  TRUE on success, FALSE on failure (not enough space)
  
**/
BOOLEAN
EfiDumpBufferAllocateBlockEx(
    _Inout_     EFI_DUMP_BUFFER*    DumpBuffer,
    _In_        MemoryBlockType     Type,
    _In_        UINT32              AdditionalSize,
    _Outptr_    VOID**              BlockStart
    );


/**
  Common bugcheck implemention for use when the context and exception record are known.

  @param  Context           Saved context.
  @param  ExceptionRecord   Optional exception record
  @param  BugCheckCode      Reason for the fatal error.
  @param  Param1            Bugcheck code specific parameter.
  @param  Param2            Bugcheck code specific parameter.
  @param  Param3            Bugcheck code specific parameter.
  @param  Param4            Bugcheck code specific parameter.

**/
VOID
EfiBugCheckWithContext(
    _In_        CONTEXT              *Context,
    _In_opt_    EXCEPTION_RECORD     *ExceptionRecord,
    _In_        UINT32                BugCheckCode,
    _In_        UINTN                 Param1,
    _In_        UINTN                 Param2,
    _In_        UINTN                 Param3,
    _In_        UINTN                 Param4
    );


/**
  Creates the guest memory dump request structure
  gathering the needed information and memory regions.

  @param  Context           Saved context.
  @param  ExceptionRecord   Optional exception record
  @param  BugCheckCode      Reason for the fatal error.
  @param  Param1            Bugcheck code specific parameter.
  @param  Param2            Bugcheck code specific parameter.
  @param  Param3            Bugcheck code specific parameter.
  @param  Param4            Bugcheck code specific parameter.

**/
EFI_STATUS
EfiBuildCrashDump(
    _In_        CONTEXT              *Context,
    _In_opt_    EXCEPTION_RECORD     *ExceptionRecord,
    _In_        UINT32                BugCheckCode,
    _In_        UINTN                 Param1,
    _In_        UINTN                 Param2,
    _In_        UINTN                 Param3,
    _In_        UINTN                 Param4
    );


/**
  Adds triage dump specific blocks to the global dump buffer

  @param  Context           Saved context.
  @param  ExceptionRecord   Optional exception record

  @return EFI_STATUS
**/
EFI_STATUS
EfiCrashDumpAddTriageInfo(
    _In_        CONTEXT              *Context,
    _In_opt_    EXCEPTION_RECORD     *ExceptionRecord
    );