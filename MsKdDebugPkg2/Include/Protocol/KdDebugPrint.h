/**@file KdDebugPrint.h

This file contains protocol definitions for printing thru the Windows kernel
debugger.

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

#ifndef __KDDEBUGPRINT_H__
#define __KDDEBUGPRINT_H__

extern EFI_GUID  gKdDebugPrintGuid;

typedef struct _KDDXE_PRINT_PROTOCOL KDDXE_PRINT_PROTOCOL;

/**
  This function prints a string to the kernel debugger.

  @param  Buffer        Pointer to the string to be printed.
  @param  BufferLength  The length of the string.

  @retval None.

**/
typedef
VOID
(EFIAPI *KDDXE_DEBUG_PRINT)(
  UINT8  *Buffer,
  UINT16 BufferLength
  );

/**
  This routine determines if the transport library believes a debugger is
  connected.

  @retval TRUE      A debugger is connected.
  @retval FALSE     A debugger is not connected.

**/
typedef
BOOLEAN
(*KDDXE_IS_CONNECTED)(
  );

struct _KDDXE_PRINT_PROTOCOL {
  KDDXE_DEBUG_PRINT     DebugPrint;
  KDDXE_IS_CONNECTED    IsConnected;
};

#endif
