/**@file Context.c

This file contains routines to convert between EFI system contexts and KD context
used by the kernel debugger.

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

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <KdTypes.h>
#include <KdProtocol.h>

void
KdProtocolContextToKdContext (
  EFI_SYSTEM_CONTEXT  *SystemContext,
  KD_CONTEXT          *KdContext
  )
{
  EFI_SYSTEM_CONTEXT_X64  *Context;

  Context = SystemContext->SystemContextX64;

  //
  // Copy from the UEFI system context to the KD version.
  //

  KdContext->ContextFlags = CONTEXT_FULL;
  KdContext->SegCs        = (UINT16)Context->Cs;
  KdContext->SegDs        = (UINT16)Context->Ds;
  KdContext->SegEs        = (UINT16)Context->Es;
  KdContext->SegFs        = (UINT16)Context->Fs;
  KdContext->SegGs        = (UINT16)Context->Gs;
  KdContext->SegSs        = (UINT16)Context->Ss;
  KdContext->EFlags       = (UINT32)Context->Rflags;
  KdContext->Dr0          = Context->Dr0;
  KdContext->Dr1          = Context->Dr1;
  KdContext->Dr2          = Context->Dr2;
  KdContext->Dr3          = Context->Dr3;
  KdContext->Dr6          = Context->Dr6;
  KdContext->Dr7          = Context->Dr7;
  KdContext->Rax          = Context->Rax;
  KdContext->Rcx          = Context->Rcx;
  KdContext->Rdx          = Context->Rdx;
  KdContext->Rbx          = Context->Rbx;
  KdContext->Rsp          = Context->Rsp;
  KdContext->Rbp          = Context->Rbp;
  KdContext->Rsi          = Context->Rsi;
  KdContext->Rdi          = Context->Rdi;
  KdContext->R8           = Context->R8;
  KdContext->R9           = Context->R9;
  KdContext->R10          = Context->R10;
  KdContext->R11          = Context->R11;
  KdContext->R12          = Context->R12;
  KdContext->R13          = Context->R13;
  KdContext->R14          = Context->R14;
  KdContext->R15          = Context->R15;
  KdContext->Rip          = Context->Rip;

  //
  // BUGBUG: Do we care about the FP/MMX/XMM registers?  Skipping for now.
  //

  return;
}

void
KdProtocolKdContextToContext (
  KD_CONTEXT          *KdContext,
  EFI_SYSTEM_CONTEXT  *SystemContext
  )
{
  EFI_SYSTEM_CONTEXT_X64  *Context;

  Context = SystemContext->SystemContextX64;

  //
  // Copy from the UEFI system context to the KD version.
  //

  Context->Cs     = KdContext->SegCs;
  Context->Ds     = KdContext->SegDs;
  Context->Es     = KdContext->SegEs;
  Context->Fs     = KdContext->SegFs;
  Context->Gs     = KdContext->SegGs;
  Context->Ss     = KdContext->SegSs;
  Context->Rflags = KdContext->EFlags;
  Context->Dr0    = KdContext->Dr0;
  Context->Dr1    = KdContext->Dr1;
  Context->Dr2    = KdContext->Dr2;
  Context->Dr3    = KdContext->Dr3;
  Context->Dr6    = KdContext->Dr6;
  Context->Dr7    = KdContext->Dr7;
  Context->Rax    = KdContext->Rax;
  Context->Rcx    = KdContext->Rcx;
  Context->Rdx    = KdContext->Rdx;
  Context->Rbx    = KdContext->Rbx;
  Context->Rsp    = KdContext->Rsp;
  Context->Rbp    = KdContext->Rbp;
  Context->Rsi    = KdContext->Rsi;
  Context->Rdi    = KdContext->Rdi;
  Context->R8     = KdContext->R8;
  Context->R9     = KdContext->R9;
  Context->R10    = KdContext->R10;
  Context->R11    = KdContext->R11;
  Context->R12    = KdContext->R12;
  Context->R13    = KdContext->R13;
  Context->R14    = KdContext->R14;
  Context->R15    = KdContext->R15;
  Context->Rip    = KdContext->Rip;

  return;
}
