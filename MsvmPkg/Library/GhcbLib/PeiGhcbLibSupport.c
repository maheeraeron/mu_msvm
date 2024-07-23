/** @file
  This file implements support routines for GHCB-based calls.

  Copyright (c) Microsoft Corporation.
  Licensed under the BSD-2-Clause-Patent license.
**/

#include <PiPei.h>

EFI_TPL
GhcbpDisableInterrupts(
    VOID
    )
{
    // In PEI, interrupts are always disabled. This function is a no-op.
    return 0;
}

VOID
GhcbpEnableInterrupts(
    EFI_TPL tpl
    )
{
    // In PEI, interrupts are always disabled. This function is a no-op.
}
