/** @file
  Task priority (TPL) functions.

Copyright (c) 2006 - 2015, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "DxeMain.h"
#include "Event.h"

/**
  Set Interrupt State.

  @param  Enable  The state of enable or disable interrupt

**/
VOID
CoreSetInterruptState (
  IN BOOLEAN      Enable
  )
{
  EFI_STATUS  Status;
  BOOLEAN     InSmm;
  
  if (gCpu == NULL) {
    return;
  }
  if (!Enable) {
    gCpu->DisableInterrupt (gCpu);
    return;
  }
  if (gSmmBase2 == NULL) {
    gCpu->EnableInterrupt (gCpu);
    return;
  }
  Status = gSmmBase2->InSmm (gSmmBase2, &InSmm);
  if (!EFI_ERROR (Status) && !InSmm) {
    gCpu->EnableInterrupt(gCpu);
  }
}


/**
  Raise the task priority level to the new level.
  High level is implemented by disabling processor interrupts.

  @param  NewTpl  New task priority level

  @return The previous task priority level

**/
EFI_TPL
EFIAPI
CoreRaiseTpl (
  IN EFI_TPL      NewTpl
  )
{
  BOOLEAN     InterruptState;
  UINT32      Mask;
  EFI_TPL     OldTpl;

  OldTpl = gEfiCurrentTpl;
  if (OldTpl > NewTpl) {
    DEBUG ((EFI_D_ERROR, "FATAL ERROR - RaiseTpl with OldTpl(0x%x) > NewTpl(0x%x)\n", OldTpl, NewTpl));
    ASSERT (FALSE);
  }
  ASSERT (VALID_TPL (NewTpl));

  InterruptState = GetInterruptState ();

  //
  // If raising to high level, disable interrupts
  //
  if (NewTpl >= TPL_HIGH_LEVEL  &&  OldTpl < TPL_HIGH_LEVEL) {
    CoreSetInterruptState (FALSE);
  }

  //
  // Set the new value
  //
  gEfiCurrentTpl = NewTpl;

  //
  // Remember the interrupt state so that it can be restored
  // when lowering. This is necessary to avoid infinite stack
  // recursion when CoreRaiseTpl is called in an interrupt handler
  // with interrupts disabled and CoreRestoreTpl is later called.
  // If interrupts are not disabled before restoring TPL, then
  // another interrupt can arrive at the same TPL. By remembering
  // this value, each recursive interrupt will at least increase
  // TPL by one, resulting in a maximum recursion depth of
  // TPL_HIGH_LEVEL.
  //
  // N.B. This must be done only after setting the TPL since the value
  // may be updated by an interrupt handler at any time before this.
  // It is safe to access gEfiOldInterruptState with interrupts
  // enabled because the value of the high bits that may be changing
  // in an interrupt handler will be ignored.
  //

  //
  // The mask will contain all bits between NewTpl-1 and OldTpl
  // This will propogate the current interrupt state to all TPLs between
  // the new and old, such that when restoring the TPL interrupts are not
  // prematurely restored.  Interrupts will only be restored once the TPL
  // has been restored to the original old TPL.
  //
  Mask = (1 << NewTpl) - (1 << OldTpl);
  if (InterruptState == FALSE) {
    gEfiOldInterruptState &= ~Mask;
  } else {
    gEfiOldInterruptState |= Mask;
  }

  return OldTpl;
}




/**
  Lowers the task priority to the previous value.   If the new
  priority unmasks events at a higher priority, they are dispatched.

  @param  NewTpl  New, lower, task priority

**/
VOID
EFIAPI
CoreRestoreTpl (
  IN EFI_TPL NewTpl
  )
{
  EFI_TPL     OldTpl;

  OldTpl = gEfiCurrentTpl;
  if (NewTpl > OldTpl) {
    DEBUG ((EFI_D_ERROR, "FATAL ERROR - RestoreTpl with NewTpl(0x%x) > OldTpl(0x%x)\n", NewTpl, OldTpl));
    ASSERT (FALSE);
  }
  ASSERT (VALID_TPL (NewTpl));

  if (NewTpl == OldTpl) {
    //
    // Nothing needs to be done in this case.
    // There should not be any pending events at a higher TPL.
    //
    ASSERT(((-2 << NewTpl) & gEventPending) == 0);
    return;
  }

  if (OldTpl >= TPL_HIGH_LEVEL  &&  NewTpl < TPL_HIGH_LEVEL) {
    gEfiCurrentTpl = TPL_HIGH_LEVEL;
  }

  //
  // Interrupts will be disabled while checking for pending events
  // to ensure that once we have processed all events no
  // events have a chance to be signalled before the TPL is actually restored
  //
  // Dispatch any pending events at a higher TPL.
  // This will temporarily raise the TPL to match the Notify TPL
  // of each signalled event group.
  //
  // -2 << NewTpl results in a mask for all higher TPLs
  // but *not* the current or lower TPLs
  // (-2 = 0xfe = 11111110b)
  // 
  CoreSetInterruptState (FALSE);

  while (((-2 << NewTpl) & gEventPending) != 0) {
    gEfiCurrentTpl = (UINTN) HighBitSet64 (gEventPending);
    if (gEfiCurrentTpl < TPL_HIGH_LEVEL) {
      CoreSetInterruptState (TRUE);
    }
    CoreDispatchEventNotifies (gEfiCurrentTpl);

    CoreSetInterruptState (FALSE);
  }

  //
  // Set the new value and enable interrupts if they were previously enabled.
  // 
  gEfiCurrentTpl = NewTpl;
  
  if ((gEfiOldInterruptState & (1 << NewTpl))) {
    CoreSetInterruptState (TRUE);
  }

}
