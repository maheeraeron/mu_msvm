/**@file KdProtocolLib.h

This file contains definitions for consumers of the KdProtocolLib.

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

#ifndef __KDPROTOCOLLIB_H__
#define __KDPROTOCOLLIB_H__

/**
  This routine performs any necessary initialization for the KdProtcolLib.

  @param  None.

  @retval EFI_SUCCESS           The protocol library successfully initialized.
  @retval EFI_OUT_OF_RESOURCES  The protocol library was unable to allocate
                                sufficient memory.

**/
EFI_STATUS
KdProtocolLibInitialize (
  VOID
  );

/**
  This routine sends an exception state change packet to the kernel debugger and
  waits for a manipulate state return message.

  @param  ExceptionRecord   Pointer to an exception record.
  @param  Context           Pointer to a context record.
  @param  FirstChance       Indication of whether this exception should be
                            handled as a first or second chance exception.

  @retval TRUE              Exception handled successfully.
  @retval FALSE             Exception handling failed.

**/
BOOLEAN
KdProtocolReportExceptionStateChange (
  EXCEPTION_RECORD    *ExceptionRecord,
  EFI_SYSTEM_CONTEXT  *Context,
  BOOLEAN             FirstChance
  );

/**
  This routine prints a string to the kernel debugger.

  @param  Output        Pointer to the string to print.

  @retval TRUE          Breakin packet detected.
  @retval FALSE         Breakin packet not detected.

**/
BOOLEAN
KdProtocolPrintString (
  KD_STRING  *Output
  );

/**
  This routine prints a string to the kernel debugger, then reads a reply string
  from the kernel debugger.

  @param  Output        Pointer to the string to be sent to the debugger.
  @param  Input         Pointer to a string for the string returned.

  @retval TRUE          Breakin packet detected.
  @retval FALSE         Breakin packet not detected.

**/
UINT32
KdProtocolPromptString (
  KD_STRING  *Output,
  KD_STRING  *Input
  );

/**
  This routine determines if a breakin packet is pending.

  @param  None.

  @retval TRUE          Breakin packet detected.
  @retval FALSE         Breakin packet not detected.

**/
BOOLEAN
KdProtocolPollBreakIn (
  VOID
  );

/**
  This routine sends a load symbols state change packet to the kernel debugger and
  waits for a manipulate state message.

  @param      PathName       Supplies a pointer to the pathname of the image
                             whose symbols are to be loaded.
  @param      SymbolInfo     The symbol information for the image that was
                             loaded.
  @param      UnloadSymbols  TRUE if the symbols that were previous loaded for
                             the named image are to be unloaded from the
                             debugger
  @param      ContextRecord  Pointer to the current execution context record.
**/
VOID
KdProtocolReportLoadSymbolsStateChange (
  KD_STRING           *PathName,
  KD_SYMBOLS_INFO     *SymbolInfo,
  BOOLEAN             UnloadSymbols,
  EFI_SYSTEM_CONTEXT  *ContextRecord
  );

#endif
