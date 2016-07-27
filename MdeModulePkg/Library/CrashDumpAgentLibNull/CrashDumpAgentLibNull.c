/** @file
  Crash Dump Agent library implementition with empty functions.

**/

/**
  Called to initialize the crash dump agent.

  @param[in] HobList     Pointer to the HOB list.

**/
VOID
EFIAPI
InitializeCrashDumpAgent(
    IN VOID               *HobList
    )
{
}

/**
  Called when a fatal error is detected and the system cannot continue.
  It is not expected that this function returns.

  @param  BugCheckCode  Reason for the fatal error.
  @param  Param1        Bugcheck code specific parameter.
  @param  Param2        Bugcheck code specific parameter.
  @param  Param3        Bugcheck code specific parameter.
  @param  Param4        Bugcheck code specific parameter.

**/
VOID
EfiBugCheck(
    IN  UINT32             BugCheckCode,
    IN  UINTN              Param1,
    IN  UINTN              Param2,
    IN  UINTN              Param3,
    IN  UINTN              Param4
    )
{
}
