/**@file Check.c

This file contains routines to check whether an address is readable or writable.

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

// This should be determined, but we work in a limited memory region so the high address
// bits should be zero anyway.   Mask is for 39 bits of physical address, enough for 512GB.
#define ADDRESS_BITS  0x0000007FFFFFF000ull
#define Pml4Index(a)   ((UINT64) ((a & 0x0000FF8000000000ull)) >> 39) // 12+9+9+9
#define PdpteIndex(a)  ((UINT64) ((a & 0x0000007FC0000000ull)) >> 30) // 12+9+9
#define PdeIndex(a)    ((UINT64) ((a & 0x000000003FE00000ull)) >> 21) // 12+9
#define PteIndex(a)    ((UINT64) ((a & 0x00000000001FF000ull)) >> 12) // 12

typedef union {
  struct {
    UINT64    Present              : 1;  // 0 = Not present in memory, 1 = Present in memory
    UINT64    ReadWrite            : 1;  // 0 = Read-Only, 1= Read/Write
    UINT64    UserSupervisor       : 1;  // 0 = Supervisor, 1=User
    UINT64    WriteThrough         : 1;  // 0 = Write-Back caching, 1=Write-Through caching
    UINT64    CacheDisabled        : 1;  // 0 = Cached, 1=Non-Cached
    UINT64    Accessed             : 1;  // 0 = Not accessed, 1 = Accessed (set by CPU)
    UINT64    Reserved             : 1;  // Reserved
    UINT64    MustBeZero           : 2;  // Must Be Zero
    UINT64    Available            : 3;  // Available for use by system software
    UINT64    PageTableBaseAddress : 40; // Page Table Base Address
    UINT64    AvabilableHigh       : 11; // Available for use by system software
    UINT64    Nx                   : 1;  // No Execute bit
  } Bits;
  UINT64    Uint64;
} PAGE_MAP_AND_DIRECTORY_POINTER;

//
// Page Table Entry 4KB
//
typedef union {
  struct {
    UINT64    Present              : 1;  // 0 = Not present in memory, 1 = Present in memory
    UINT64    ReadWrite            : 1;  // 0 = Read-Only, 1= Read/Write
    UINT64    UserSupervisor       : 1;  // 0 = Supervisor, 1=User
    UINT64    WriteThrough         : 1;  // 0 = Write-Back caching, 1=Write-Through caching
    UINT64    CacheDisabled        : 1;  // 0 = Cached, 1=Non-Cached
    UINT64    Accessed             : 1;  // 0 = Not accessed, 1 = Accessed (set by CPU)
    UINT64    Dirty                : 1;  // 0 = Not Dirty, 1 = written by processor on access to page
    UINT64    PAT                  : 1;  //
    UINT64    Global               : 1;  // 0 = Not global page, 1 = global page TLB not cleared on CR3 write
    UINT64    Available            : 3;  // Available for use by system software
    UINT64    PageTableBaseAddress : 40; // Page Table Base Address
    UINT64    AvabilableHigh       : 11; // Available for use by system software
    UINT64    Nx                   : 1;  // 0 = Execute Code, 1 = No Code Execution
  } Bits;
  UINT64    Uint64;
} PAGE_TABLE_4K_ENTRY;

//
// Page Table Entry 2MB
//
typedef union {
  struct {
    UINT64    Present              : 1;  // 0 = Not present in memory, 1 = Present in memory
    UINT64    ReadWrite            : 1;  // 0 = Read-Only, 1= Read/Write
    UINT64    UserSupervisor       : 1;  // 0 = Supervisor, 1=User
    UINT64    WriteThrough         : 1;  // 0 = Write-Back caching, 1=Write-Through caching
    UINT64    CacheDisabled        : 1;  // 0 = Cached, 1=Non-Cached
    UINT64    Accessed             : 1;  // 0 = Not accessed, 1 = Accessed (set by CPU)
    UINT64    Dirty                : 1;  // 0 = Not Dirty, 1 = written by processor on access to page
    UINT64    MustBe1              : 1;  // Must be 1
    UINT64    Global               : 1;  // 0 = Not global page, 1 = global page TLB not cleared on CR3 write
    UINT64    Available            : 3;  // Available for use by system software
    UINT64    PAT                  : 1;  //
    UINT64    MustBeZero           : 8;  // Must be zero;
    UINT64    PageTableBaseAddress : 31; // Page Table Base Address
    UINT64    AvabilableHigh       : 11; // Available for use by system software
    UINT64    Nx                   : 1;  // 0 = Execute Code, 1 = No Code Execution
  } Bits;
  UINT64    Uint64;
} PAGE_TABLE_ENTRY;

//
// Page Table Entry 1GB
//
typedef union {
  struct {
    UINT64    Present              : 1;  // 0 = Not present in memory, 1 = Present in memory
    UINT64    ReadWrite            : 1;  // 0 = Read-Only, 1= Read/Write
    UINT64    UserSupervisor       : 1;  // 0 = Supervisor, 1=User
    UINT64    WriteThrough         : 1;  // 0 = Write-Back caching, 1=Write-Through caching
    UINT64    CacheDisabled        : 1;  // 0 = Cached, 1=Non-Cached
    UINT64    Accessed             : 1;  // 0 = Not accessed, 1 = Accessed (set by CPU)
    UINT64    Dirty                : 1;  // 0 = Not Dirty, 1 = written by processor on access to page
    UINT64    MustBe1              : 1;  // Must be 1
    UINT64    Global               : 1;  // 0 = Not global page, 1 = global page TLB not cleared on CR3 write
    UINT64    Available            : 3;  // Available for use by system software
    UINT64    PAT                  : 1;  //
    UINT64    MustBeZero           : 17; // Must be zero;
    UINT64    PageTableBaseAddress : 22; // Page Table Base Address
    UINT64    AvabilableHigh       : 11; // Available for use by system software
    UINT64    Nx                   : 1;  // 0 = Execute Code, 1 = No Code Execution
  } Bits;
  UINT64    Uint64;
} PAGE_TABLE_1G_ENTRY;

/**
  This routine validates that the virtual memory address (Addr) is valid
  according to the page tables.

  @param  WriteCheck       BLAH BLAH BLAH
  @param  Addr             BLAH BLAH BLAH

  @retval UINT8 *          Address
  @retval NULL             If the address is not valid or doesn't have the
                           proper read or write access.

**/
UINT8 *
CheckAddress (
  BOOLEAN  WriteCheck,
  UINT8    *Addr
  )
{
  UINT64                          Cr3;
  PAGE_MAP_AND_DIRECTORY_POINTER  *Pml4;
  PAGE_TABLE_1G_ENTRY             *Pte1G;
  PAGE_TABLE_ENTRY                *Pte2M;
  PAGE_TABLE_4K_ENTRY             *Pte4K;
  UINT64                          Address;

  Address = (UINT64)Addr;

  Cr3 = AsmReadCr3 () & ADDRESS_BITS;

  Pml4 = (PAGE_MAP_AND_DIRECTORY_POINTER *)(Cr3 + Pml4Index (Address));

  if (!Pml4->Bits.Present) {
    return NULL;
  }

  Pte1G = (PAGE_TABLE_1G_ENTRY *)(Pml4->Uint64 & ADDRESS_BITS) + PdpteIndex (Address);

  if (Pte1G == NULL) {
    return NULL;
  }

  if (!Pte1G->Bits.Present) {
    return NULL;
  }

  if (Pte1G->Bits.MustBe1) {
    // 1GB mapped page
    if (WriteCheck && !Pte1G->Bits.ReadWrite) {
      return NULL;
    }

    return Addr;
  }

  Pte2M = (PAGE_TABLE_ENTRY *)(Pte1G->Uint64 & ADDRESS_BITS) + PdeIndex (Address);
  if (Pte2M == NULL) {
    return NULL;
  }

  if (!Pte2M->Bits.Present) {
    return NULL;
  }

  if (Pte2M->Bits.MustBe1) {
    // 2MB mapped page
    if (WriteCheck && !Pte2M->Bits.ReadWrite) {
      return NULL;
    }

    return Addr;
  }

  Pte4K = (PAGE_TABLE_4K_ENTRY *)(Pte2M->Uint64 & ADDRESS_BITS) + PteIndex (Address);
  if (Pte4K == NULL) {
    return NULL;
  }

  if (!Pte4K->Bits.Present) {
    return NULL;
  }

  if (WriteCheck && !Pte4K->Bits.ReadWrite) {
    return NULL;
  }

  return Addr;
}

/**
  This routine determines if the specified address can be written.

  @param  Address       Supplies the virtual address to check.

  @retval UINT8 *          Address
  @retval NULL             If the address is not valid or doesn't have the
                           proper read or write access.

**/
UINT8 *
KdProtocolWriteCheck (
  UINT8  *Address
  )
{
  return CheckAddress (TRUE, Address);
}

/**
  This routine determines if the specified address can be read.

  @param  Address       Supplies the virtual address to check.

  @retval UINT8 *          Address
  @retval NULL             If the address is not valid or doesn't have the
                           proper read or write access.

**/
UINT8 *
KdProtocolReadCheck (
  UINT8  *Address
  )
{
  return CheckAddress (FALSE, Address);
}

/**
  This routine returns the physical address for a virtual address
  which is valid (mapped).

  @retval UINT8 *          Address
  @retval NULL             If the address is not valid or doesn't have the
                           proper read or write access.

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
