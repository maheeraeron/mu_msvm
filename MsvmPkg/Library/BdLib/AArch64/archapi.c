/*++

Copyright (c) 2017  Microsoft Corporation

Module Name:

    archapi.c

Abstract:

    This module implements architecture specific functions.

--*/

#include <bd.h>
#include <Library/ArmLib.h>
#include <Library/CacheMaintenanceLib.h>

VOID
BlArchSweepIcacheRange (
  __in PVOID BaseAddress,
  __in SIZE_T Length
  )
{
    WriteBackDataCacheRange(BaseAddress, Length);

    ArmInvalidateInstructionCache();
}



