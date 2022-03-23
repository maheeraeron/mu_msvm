/*++

Copyright (c) Microsoft Corporation

Module Name:

    FailFast.h

Abstract:

    This file contains the code to fail fast.

--*/

#pragma once

#define CRITICAL_INITIALIZATION_FAILURE 0x13D
#define KERNEL_SECURITY_CHECK_FAILURE 0x139
#define FAST_FAIL_UNEXPECTED_HOST_BEHAVIOR 58


#if defined(MDE_CPU_AARCH64)
#define FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(Info1, Info2, Info3) \
    { ASSERT(FALSE); CpuDeadLoop(); }
#elif defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)
#define FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(Info1, Info2, Info3) \
    GenerateDumpAndFailFast(FAST_FAIL_UNEXPECTED_HOST_BEHAVIOR, Info1, Info2, Info3, 0);
#else
#error Unsupported Architecture
#endif


#define FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR_IF_FALSE(cond, Info1, Info2, Info3) \
        if (!(cond)) { FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(Info1, Info2, Info3) } \


#if defined(MDE_CPU_AARCH64)
#define FAIL_FAST(ErrorCode, Info1, Info2, Info3) \
    { ASSERT(FALSE); CpuDeadLoop(); }
#elif defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)
#define FAIL_FAST(ErrorCode, Info1, Info2, Info3) \
    GenerateDumpAndFailFast(ErrorCode, Info1, Info2, Info3, 0);
#else
#error Unsupported Architecture
#endif

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