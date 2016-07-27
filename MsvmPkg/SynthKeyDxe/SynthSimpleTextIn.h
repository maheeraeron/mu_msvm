/*++ @file
Copyright (c) Microsoft Corporation

Module Name:

    SynthKeySimpleTextIn.h

Abstract:

    Functions and Prototypes for implementing UEFI simple text input protocols
    This is a generic as possible implementation and provides an API set 
    for drivers to initialize the text input layer and for lower layers (like VMBUS or PS2)
    to queue processed key presses.

Author:

    Kris Harper (kharp) - 15-Oct-2012

ATTENTION - THIS FILE CONTAINS THIRD PARTY OPEN SOURCE CODE: 
    IntelFrameworkModulePkg\Bus\Isa\Ps2KeyboardDxe\Ps2Keyboard.h

IT IS CLEARED ONLY FOR LIMITED USE BY WINDOWS CORE HYPER-V FOR THE HYPER-V ROLE
IN THE WINDOWS PRODUCT. DO NOT USE OR SHARE THIS CODE WITHOUT APPROVAL PURSUANT
TO THE MICROSOFT OPEN SOURCE SOFTWARE APPROVAL POLICY.

Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

--*/
#pragma once


//
// Public Simple Text In APIs
//

EFI_STATUS
SimpleTextInInitialize(
    _Inout_     PSYNTH_KEYBOARD_DEVICE      pDevice
    );


VOID
SimpleTextInCleanup(
    _In_        PSYNTH_KEYBOARD_DEVICE      pDevice
    );


VOID
SimpleTextInQueueKey(
    _In_        PSYNTH_KEYBOARD_DEVICE      pDevice,
    _In_        EFI_KEY_DATA               *Key
    );


