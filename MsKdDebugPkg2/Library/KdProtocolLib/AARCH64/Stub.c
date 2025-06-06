/**@file Stub.c

This file contains stub routines, whose implementations are not necessary on
ARM64.

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

/**
  This routine determines if the specified address can be written.

  @param  Address       Supplies the virtual address to check.

  @retval UINT8 *       Address

**/
UINT8 *
KdProtocolWriteCheck (
  UINT8  *Address
  )
{
  return Address;
}

/**
  This routine determines if the specified address can be written.

  @param  Address   Supplies the virtual address to check.

  @retval UINT8 *   Address
  @retval NULL      If the address is not valid or readable.

**/
UINT8 *
KdProtocolReadCheck (
  UINT8  *Address
  )
{
  return Address;
}

/**
  This routine returns the physical address for a virtual address
  which is valid (mapped).

  @param  Address       Supplies the physical address to check.

  @retval UINT8 *       Address
  @retval NULL          If the address is not valid or readable.

**/
UINT8 *
KdProtocolTranslatePhysicalAddress (
  UINT64  Address
  )
{
  //
  // EFI environment is identity mapped.
  //

  return (UINT8 *)Address;
}

VOID
KdProtocolUnmapVirtualAddress (
  UINT8  *Va
  )
{
  return;
}
