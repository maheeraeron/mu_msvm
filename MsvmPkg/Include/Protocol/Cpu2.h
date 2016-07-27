/*++

Copyright (c) Microsoft Corporation

Module Name:

    Cpu2.h

Abstract:

    Provides the definition of the EFI_CPU2_PROTOCOL protocol. This protocol
    provides a single routine that allows for safe idling of the processor
    even in the presense of device interrupts.

Author:

    John Starks (jostarks) - 2-Jul-2012

--*/

#pragma once

#define EFI_CPU2_PROTOCOL_GUID \
  { 0x55198405, 0x26c0, 0x4765, {0x8b, 0x7d, 0xbe, 0x1d, 0xf5, 0xf9, 0x97, 0x12} }

typedef struct _EFI_CPU2_PROTOCOL EFI_CPU2_PROTOCOL;

/**
  This function waits for an interrupt to arrive, then enables CPU interrupts.

  @param  This              Protocol instance structure

  @retval EFI_SUCCESS       If interrupts were enabled in the CPU
  @retval EFI_DEVICE_ERROR  If interrupts could not be enabled on the CPU.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_CPU_WAIT_FOR_AND_ENABLE_INTERRUPT)(
  IN EFI_CPU2_PROTOCOL              *This
  );

struct _EFI_CPU2_PROTOCOL {
  EFI_CPU_WAIT_FOR_AND_ENABLE_INTERRUPT WaitForAndEnableInterrupt;
};

extern EFI_GUID gEfiCpu2ProtocolGuid;

