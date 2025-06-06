/**@file Symbols.h

This file contains definitions related to symbols support for KdDxe.efi.

Copyright (c) 2018, Microsoft Corporation

All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__

typedef struct _UNICODE_STRING {
  UINT16    Length;
  UINT16    MaximumLength;
  UINT16    *Buffer;
} UNICODE_STRING;

//
// Portable debugger structures.
//
// The debugger assumes several structures exist (thread, process, APC state,
// etc.) and that these structures are linked together in specific ways.
// To allow the debugger to continue to work, we define portable versions
// of these structures below. In an NT system, these would correspond to
// real structures that the OS maintains.
//
// Note that the offsets of the interesting fields within these structures
// is passed into the debugger, so we do not need to maintain the exact
// layout of NT's KTHREAD or EPROCESS.
//

typedef struct _NON_PAGED_DEBUG_INFO {
  UINT16    Signature;
  UINT16    Flags;
  UINT32    Size;
  UINT16    Machine;
  UINT16    Characteristics;
  UINT32    TimeDateStamp;
  UINT32    CheckSum;
  UINT32    SizeOfImage;
  UINT64    ImageBase;
  // DebugDirectorySize      //IMAGE_DEBUG_DIRECTORY
} NON_PAGED_DEBUG_INFO;

typedef struct _LDR_DATA_TABLE_ENTRY {
  LIST_ENTRY              InLoadOrderLinks;
  UINTN                   __Undefined1;
  UINTN                   __Undefined2;
  UINTN                   __Undefined3;
  NON_PAGED_DEBUG_INFO    *NonPagedDebugInfo;
  UINTN                   DllBase;
  UINTN                   EntryPoint;
  UINT32                  SizeOfImage;
  UNICODE_STRING          FullDllName;
  UNICODE_STRING          BaseDllName;
  UINT32                  Flags;
  UINT16                  LoadCount;
  UINT16                  __Undefined5;
  UINTN                   __Undefined6;
  UINT32                  CheckSum;
  UINT32                  __padding1;
  UINT32                  TimeDateStamp;
  UINT32                  __padding2;
} LDR_DATA_TABLE_ENTRY;

VOID
KdDxeSymbolInitialize (
  );

EFI_STATUS
KdDxeAddSymbolInfo (
  EFI_LOADED_IMAGE_PROTOCOL  *Image,
  LDR_DATA_TABLE_ENTRY       **AddedEntry
  );

VOID
KdDxeSymbolNotification (
  LDR_DATA_TABLE_ENTRY  *Image,
  UINT8                 Operation
  );

UINT16 *
KdDxeGetModuleName (
  EFI_LOADED_IMAGE_PROTOCOL  *Image
  );

#endif
