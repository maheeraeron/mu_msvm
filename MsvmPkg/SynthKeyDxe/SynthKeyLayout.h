/*++

Copyright (c) Microsoft Corporation

Module Name:

    SynthKeyLayout.h

Abstract:

    VMBUS Keyboard Layout. Handles the translation of key press messages from the 
    synthetic keyboard vdev to EFI_KEYs based on the UEFI keyboard layout.

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

#define TEST_FLAGS(_flags, _value)      (((_flags) & (_value)) != 0)

// Helpers to test key shift states.
#define EFI_KEY_SHIFT_ACTIVE(_state)    TEST_FLAGS(_state, (EFI_LEFT_SHIFT_PRESSED | EFI_RIGHT_SHIFT_PRESSED))
#define EFI_KEY_CTRL_ACTIVE(_state)     TEST_FLAGS(_state, (EFI_LEFT_CONTROL_PRESSED | EFI_RIGHT_CONTROL_PRESSED))
#define EFI_KEY_ALT_ACTIVE(_state)      TEST_FLAGS(_state, (EFI_LEFT_ALT_PRESSED | EFI_RIGHT_ALT_PRESSED))

//
// Specific scan codes that we need to process.
// only make codes are needed.
//
#define SCANCODE_CTRL_MAKE              0x1D
#define SCANCODE_ALT_MAKE               0x38
#define SCANCODE_LEFT_SHIFT_MAKE        0x2A
#define SCANCODE_RIGHT_SHIFT_MAKE       0x36
#define SCANCODE_FORWARD_SLASH          0x35
#define SCANCODE_CAPS_LOCK_MAKE         0x3A
#define SCANCODE_NUM_LOCK_MAKE          0x45
#define SCANCODE_SCROLL_LOCK_MAKE       0x46
#define SCANCODE_DELETE_MAKE            0x53
#define SCANCODE_LEFT_LOGO_MAKE         0x5B // Windows key (a.k.a. GUI key)
#define SCANCODE_RIGHT_LOGO_MAKE        0x5C
#define SCANCODE_MENU_MAKE              0x5D // Context menu key (a.k.a. APPS key)
#define SCANCODE_SYS_REQ_MAKE           0x37
#define SCANCODE_SYS_REQ_MAKE_WITH_ALT  0x54

//
// the high bit signifies a key break (release)
//
#define SCANCODE_BREAK_FLAG             0x80

//
// Structure used to map scan codes to EFI_KEY values
//
typedef struct _SYNTHKEY_KEY_MAP_ENTRY
{
    UINT8   ScanCode;           // Scan Code as defined in scan code set 1
    UINT16  EfiScanCode;        // Scan code as defined in UEFI specification
                                // SCAN_NULL when not applicable.
    CHAR16  UnicodeChar;        // Unicode character, CHAR_NULL when not applicable
    CHAR16  ShiftUnicodeChar;   // Unicode character when shift and/or caps lock is applied
}SYNTHKEY_KEY_MAP_ENTRY, *PSYNTHKEY_KEY_MAP_ENTRY;

// Used to signal the end of a translation table
#define TABLE_END                       0x0

//
// Describes the type of key state change detected by SynthKeyLayoutUpdateState.KeyState.
//
typedef enum
{
    KeyChangeNone,
    KeyChangeShift,
    KeyChangeToggle
}SynthKeyStateChangeType;

//
// Keyboard Layout public APIs
//

SynthKeyStateChangeType
SynthKeyLayoutUpdateKeyState(
    _In_        PHK_MESSAGE_KEYSTROKE       RawKey,
    _In_        PSYNTH_KEYBOARD_STATE       KeyState
    );

EFI_STATUS
SynthKeyLayoutTranslateKey(
    _In_        PHK_MESSAGE_KEYSTROKE       RawKey,
    _In_        PSYNTH_KEYBOARD_STATE       KeyState,
    _Out_       EFI_KEY_DATA               *TranslatedKey
    );
