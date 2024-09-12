/** @file
  VMBUS Keyboard Channel implementation for EFI.  This contains the VMBUS
  specific implementation of the synthetic keyboard driver.

  Copyright (c) Microsoft Corporation.
  Licensed under the BSD-2-Clause-Patent license.

**/

#pragma once

#include <Protocol/hyperkbdprotocol.h>
#include <Vmbus/VmBusPacketFormat.h>

EFI_STATUS
SynthKeyChannelOpen(
    _In_        PSYNTH_KEYBOARD_DEVICE      pDevice
    );

EFI_STATUS
SynthKeyChannelClose(
    _In_        PSYNTH_KEYBOARD_DEVICE      pDevice
    );

EFI_STATUS
SynthKeyChannelSetIndicators(
    _In_        PSYNTH_KEYBOARD_DEVICE      pDevice
    );

FORCEINLINE
VOID
SynthKeyChannelInitMessage(
    _Inout_updates_bytes_(ByteCount)
                PHK_MESSAGE_HEADER          Header,
    _In_        HK_MESSAGE_TYPE             Type,
    _In_range_(>=, sizeof(HK_MESSAGE_HEADER))
                UINT32                      ByteCount
    )
/*++

Routine Description:

    A utility function to initialize a message header.

Arguments:

    Header    - Message header

    Type      - Message type

    ByteCount - Size of the message header in bytes

Return Value:

    None.

--*/
{
    ZeroMem(Header, ByteCount);
    Header->MessageType = Type;
}
