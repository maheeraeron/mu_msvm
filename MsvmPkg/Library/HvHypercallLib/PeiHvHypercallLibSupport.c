/** @file
  This file implements support routines for PEI hypercalls.

  Copyright (c) Microsoft Corporation.
  Licensed under the BSD-2-Clause-Patent license.
**/

#include <PiPei.h>

#include <HvHypercallLibP.h>

VOID
HvHypercallpDisableInterrupts(
    VOID
    )
{
    // In PEI, interrupts are always disabled. This function is a no-op.
}

VOID
HvHypercallpEnableInterrupts(
    VOID
    )
{
    // In PEI, interrupts are always disabled. This function is a no-op.
}