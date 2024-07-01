/** @file
  This file contains architecture specific functions for debugging a failure.

  Copyright (c) Microsoft Corporation.
  Licensed under the BSD-2-Clause-Patent license.
**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  Called when a fatal error is detected and the system cannot continue.
  It is not expected that this function returns.

  @param  ErrorCode     Bugcheck code
  @param  Param1        Bugcheck code specific parameter.
  @param  Param2        Bugcheck code specific parameter.
  @param  Param3        Bugcheck code specific parameter.
  @param  Param4        Bugcheck code specific parameter.

**/
VOID
GenerateDumpAndFailFast(
    IN  UINTN              ErrorCode,
    IN  UINTN              Param1,
    IN  UINTN              Param2,
    IN  UINTN              Param3,
    IN  UINTN              Param4
    )
{
  // TODO - use a better fail fast mechanism.
  ASSERT(FALSE);
  CpuDeadLoop();
}