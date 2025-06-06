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
  EFI_SYSTEM_CONTEXT_AARCH64  *Context;
  UINTN                       Length;

  Context = SystemContext->SystemContextAArch64;

  //
  // Copy from the UEFI system context to the KD version.
  //

  KdContext->ContextFlags = CONTEXT_FULL;
  KdContext->Cpsr         =  (UINT32)Context->SPSR;

  Length = OFFSET_OF (EFI_SYSTEM_CONTEXT_AARCH64, X28) -
           OFFSET_OF (EFI_SYSTEM_CONTEXT_AARCH64, X0) +
           sizeof (Context->X28);

  CopyMem (&KdContext->X_0, &Context->X0, Length);

  KdContext->Fp = Context->FP;
  KdContext->Lr = Context->LR;
  KdContext->Sp = Context->SP;
  KdContext->Pc = Context->ELR;

  Length = OFFSET_OF (EFI_SYSTEM_CONTEXT_AARCH64, V31[1]) -
           OFFSET_OF (EFI_SYSTEM_CONTEXT_AARCH64, V0[0]) +
           sizeof (Context->V31[1]);

  CopyMem (&KdContext->V0[0], &Context->V0[0], Length);
  return;
}

void
KdProtocolKdContextToContext (
  KD_CONTEXT          *KdContext,
  EFI_SYSTEM_CONTEXT  *SystemContext
  )
{
  EFI_SYSTEM_CONTEXT_AARCH64  *Context;
  UINTN                       Length;

  Context = SystemContext->SystemContextAArch64;

  //
  // Copy from the UEFI system context to the KD version.
  //

  Context->SPSR = KdContext->Cpsr;
  Length        = OFFSET_OF (EFI_SYSTEM_CONTEXT_AARCH64, X28) -
                  OFFSET_OF (EFI_SYSTEM_CONTEXT_AARCH64, X0) +
                  sizeof (Context->X28);

  CopyMem (&Context->X0, &KdContext->X_0, Length);
  Context->FP  = KdContext->Fp;
  Context->LR  = KdContext->Lr;
  Context->SP  = KdContext->Sp;
  Context->ELR = KdContext->Pc;
  Length       = OFFSET_OF (EFI_SYSTEM_CONTEXT_AARCH64, V31[1]) -
                 OFFSET_OF (EFI_SYSTEM_CONTEXT_AARCH64, V0[0]) +
                 sizeof (Context->V31[1]);

  CopyMem (&Context->V0[0], &KdContext->V0[0], Length);
  return;
}
