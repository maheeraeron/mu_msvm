/*++

Copyright (c) Microsoft Corporation

Module Name:

    BiosConfigPageGuid.h

Abstract:

    Definitions for the BIOS Config Page HOB GUID.

Author:

   Larry Cleeton (lcleeton) - 28-Nov-2012

--*/

#pragma once


//
// BIOS Config Page HOB GUID.
//
#define MSVM_CONFIG_PAGE_V2_GUID \
    { \
        0x85d69e2f, 0x840c, 0x49a6, { 0x8c, 0xfd, 0xc3, 0x9c, 0xab, 0xb8, 0xad, 0x37 } \
    }

#define MSVM_CONFIG_PAGE_V3_GUID \
    { \
        0x8cfe1fb4, 0xe11f, 0x431a, { 0xb3, 0xa3, 0x56, 0x8f, 0x58, 0x18, 0xd3, 0x42 } \
    }

//
// Declaration of autogen'd global GUID structure.
//
extern EFI_GUID gMsvmConfigPageV2Guid;
extern EFI_GUID gMsvmConfigPageV3Guid;

