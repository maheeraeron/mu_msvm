/*++

Copyright (c) Microsoft Corporation

Module Name:

    UefiConstants.h

Abstract:

    This file contains constants used in UEFI.

--*/

#pragma once

enum
{
    ConfigLibConsoleModeDefault = 0, // video+kbd (having a head)
    ConfigLibConsoleModeCOM1    = 1, // headless with COM1 serial console
    ConfigLibConsoleModeCOM2    = 2, // headless with COM2 serial console
    ConfigLibConsoleModeNone    = 3  // headless
};

