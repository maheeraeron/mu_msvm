/**@file KdDxe.c

This file contains the initialization routines for KdDxe.efi.

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

#include <Guid/EventGroup.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SourceDebugEnabledLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/DebugAgentLib.h>
#include <Protocol/KdDebugPrint.h>
#include <KdTypes.h>
#include <Library/KdTransportLib.h>
#include <Library/KdProtocolLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/Cpu.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/FirmwareVolume2.h>
#include "KdDxe.h"
#include "Symbols.h"

EFI_EVENT   mExitBootServicesEvent;
EFI_HANDLE  mKdDxeDebugPrintHandle;
BOOLEAN     mKdDxeInitialized;
EFI_EVENT   mKdDxeTimerEvent;

KDDXE_PRINT_PROTOCOL  mKdPrint = {
  KdDxeDebugPrint,
  KdTransportIsDebuggerConnected
};

//
// This test pattern is useful when verifying debugger functionality.  It has no
// other functional purpose.
//

volatile UINT32  mTestPattern[4];

/**
  This routine forwards a debug print request to the debugger.

  @param  Buffer            Supplies a pointer to the debug string to be printed.
  @param  BufferLength      Supplies the string length.

**/
VOID
EFIAPI
KdDxeDebugPrint (
  UINT8   *Buffer,
  UINT16  BufferLength
  )
{
  KD_STRING  Output;
  BOOLEAN    BreakIn;

  //
  // BUGBUG: Determine if the debugger is connected or not.  If not, send the
  // message directly to serial port.
  //

  Output.Buffer = Buffer;
  Output.Length = BufferLength;
  BreakIn       = KdProtocolPrintString (&Output);
  if (BreakIn != FALSE) {
    KdDxeKdBreakPointWithStatus (0);
  }

  return;
}

/**
  This routine handles timer events.

  @param  Event            Not used.
  @param  Context          Not used.
**/
VOID
KdDxeTimerRoutine (
  EFI_EVENT  Event,
  VOID       *Context
  )
{
  BOOLEAN  BreakIn;

  BreakIn = KdProtocolPollBreakIn ();
  if (BreakIn != FALSE) {
    KdDxeKdBreakPointWithStatus (0);
  }

  return;
}

/**
  This routine initializes timer events.

  N.B. Any failures in this routine are intentionally ignored.  Control-C
       functionality will not work without timers, but exception handling will
       still work.

**/
VOID
KdDxeTimerInitialize (
  )
{
  EFI_STATUS  Status;

  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KdDxeTimerRoutine,
                  NULL,
                  &mKdDxeTimerEvent
                  );

  if (EFI_ERROR (Status) == FALSE) {
    Status = gBS->SetTimer (
                    mKdDxeTimerEvent,
                    TimerPeriodic,
                    10000000
                    );
    DEBUG ((DEBUG_INFO, "%a: Setting Timer Event. Code=%r\n", __func__, Status));
  }

  return;
}

/**
  This routine terminates the timer event.

**/
VOID
KdDxeTimerDestroy (
  )
{
  if (mKdDxeTimerEvent == NULL) {
    gBS->CloseEvent (mKdDxeTimerEvent);
  }

  return;
}

/**
  This routine handles the EXIT_BOOT_SERVICES notification.

  @param  Event            Not used.
  @param  Context          Not used.

**/
VOID
EFIAPI
KdDxeExitBootServices (
  EFI_EVENT  Event,
  VOID       *Context
  )
{
  KdDxeTimerDestroy ();
  KdDxeExceptionDestroy ();
  return;
}

/**
  This routine is the entry point for the KdDxe.efi driver.

  @param  ImageHandle       Not used.
  @param  MapKey            Not used.

  @retval EFI_SUCCESS       On success.
  @retval EFI_STATUS        On failure.

**/
EFI_STATUS
EFIAPI
KdDxeInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )

{
  EFI_STATUS  Status;

  DEBUG ((DEBUG_INFO, "%a: Entry.\n", __func__));

  //
  // Initialize the test pattern.
  //
  mTestPattern[0] = 0x11111111;
  mTestPattern[1] = 0x22222222;
  mTestPattern[2] = 0x33333333;
  mTestPattern[3] = 0x44444444;

  mKdDxeInitialized = FALSE;

  //
  // Determine if the debugger should be initialized.  If not, exit early.
  //

  if (IsSourceDebugEnabled (DEBUG_AGENT_INIT_DXE_CORE) == FALSE) {
    DEBUG ((DEBUG_INFO, "%a: debugger not enabled, going to cleanup!\n", __func__));
    Status = EFI_SUCCESS;
    goto Cleanup;
  }

  //
  // Initialize Exception Handling
  //

  Status = KdDxeExceptionInitialize ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: KdDxeExceptionInitialize failed, Status = (%r).\n", __func__, Status));
    ASSERT_EFI_ERROR (Status);
    goto Cleanup;
  }

  //
  // Register for EXIT_BOOT_SERVICES notification.
  //

  gBS->CreateEventEx (
         EVT_NOTIFY_SIGNAL,
         TPL_NOTIFY,
         KdDxeExitBootServices,
         NULL,
         &gEfiEventExitBootServicesGuid,
         &mExitBootServicesEvent
         );
  //
  // Initialize the transport and protocol libraries.
  //
  Status = KdTransportLibInitialize ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: KdTransportLibInitialize failed, Status = (%r)\n", __func__, Status));
    goto Cleanup;
  }

  Status = KdProtocolLibInitialize ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: KdProtocolLibInitialize failed, Status = (%r)\n", __func__, Status));
    goto Cleanup;
  }

  //
  // Install the protocol to allow KdPrints.
  //
  Status = gBS->InstallProtocolInterface (
                  &mKdDxeDebugPrintHandle,
                  &gKdDebugPrintGuid,
                  EFI_NATIVE_INTERFACE,
                  (VOID *)&mKdPrint
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: mMsKdDxeDebugPrintProtocol install failed, Status = (%r)\n", __func__, Status));
    goto Cleanup;
  }

  mKdDxeInitialized = TRUE;

  //
  // This will attempt to be printed to the KD. If it doesn't succeeed, the
  // transport lib will recognize that the KD is not connected.
  //
  DEBUG ((DEBUG_ERROR, "%a - EfiKd: Trying to initialize!\n", __func__));

  //
  // Initialize symbol support.
  //
  // N.B:  This must happen after mKdDxeInitialized as loaded modules will be
  //       reported to the debugger.
  //

  KdDxeSymbolInitialize ();

  //
  // Register for clock interrupts, which are used to catch Control-Cs.
  //

  KdDxeTimerInitialize ();

Cleanup:

  DEBUG ((DEBUG_INFO, "%a: Exit (%r).\n", __func__, Status));
  return Status;
}
