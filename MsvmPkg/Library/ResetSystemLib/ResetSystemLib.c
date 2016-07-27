/** @file
  Reset System Library functions for Hyper-V platform

  Copyright (c) 2006 - 2014, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <PowerManagementInterface.h>

VOID
AcpiPmControl (
  UINTN SuspendType
  )
{
  UINT16 value;

  ASSERT (SuspendType < 6);

  value = IoRead16 (PowerMgmtDefaultBaseRegister + PowerMgmtControlRegOffset);
  value &= ~PowerMgmtControlSuspendTypeMask;
  value |= (UINT16) (SuspendType << 10);
  value |= PowerMgmtControlSuspendEnableMask;
  IoWrite16 (PowerMgmtDefaultBaseRegister + PowerMgmtControlRegOffset, value);
  CpuDeadLoop ();
}

/**
  Calling this function causes a system-wide reset. This sets
  all circuitry within the system to its initial state. This type of reset
  is asynchronous to system operation and operates without regard to
  cycle boundaries.

  System reset should not return, if it returns, it means the system does
  not support cold reset.
**/
VOID
EFIAPI
ResetCold (
  VOID
  )
{
  IoWrite8 (PowerMgmtDefaultBaseRegister + PowerMgmtResetRegister,
            PowerMgmtResetValue);

  CpuDeadLoop();
}

/**
  Calling this function causes a system-wide initialization. The processors
  are set to their initial state, and pending cycles are not corrupted.

  System reset should not return, if it returns, it means the system does
  not support warm reset.
**/
VOID
EFIAPI
ResetWarm (
  VOID
  )
{
  ResetCold();
}

/**
  Calling this function causes the system to enter a power state equivalent
  to the ACPI G2/S5 or G3 states.

  System shutdown should not return, if it returns, it means the system does
  not support shut down reset.
**/
VOID
EFIAPI
ResetShutdown (
  VOID
  )
{
  AcpiPmControl (0);
}


/**
  Calling this function causes the system to enter a power state for capsule
  update.

  Reset update should not return, if it returns, it means the system does
  not support capsule update.

**/
VOID
EFIAPI
EnterS3WithImmediateWake (
  VOID
  )
{
  ASSERT (FALSE);
}
