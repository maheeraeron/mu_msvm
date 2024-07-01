/** @file

    This file contains the code to fail fast.

    Copyright (c) Microsoft Corporation.
    Licensed under the BSD-2-Clause-Patent license.
**/

#pragma once

#define CRITICAL_INITIALIZATION_FAILURE 0x13D
#define KERNEL_SECURITY_CHECK_FAILURE 0x139
#define FAST_FAIL_UNEXPECTED_HOST_BEHAVIOR 58


#define FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(Info1, Info2, Info3) \
    GenerateDumpAndFailFast(FAST_FAIL_UNEXPECTED_HOST_BEHAVIOR, Info1, Info2, Info3, 0);

#define FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR_IF_FALSE(cond, Info1, Info2, Info3) \
        if (!(cond)) { FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(Info1, Info2, Info3) } \


#define FAIL_FAST(ErrorCode, Info1, Info2, Info3) \
    GenerateDumpAndFailFast(ErrorCode, Info1, Info2, Info3, 0);

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
);