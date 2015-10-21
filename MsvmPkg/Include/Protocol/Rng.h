/*++

Copyright (c) Microsoft Corporation

Module Name:

    Rng.h

Abstract:

    Provides the protocol definition for EFI_RNG_PROTOCOL, which provides
    random numbers for use in applications, or entropy for 
    seeding other random number generators.

Author:

    Arturo Lira (davlir) 20-Mar-2013

--*/
#pragma once

#include "BiosInterface.h"

//
// Random Number Generator Protocol GUID.
// 3152bca5-eade-433d-862e-c01cdc291f44
//

#define EFI_RNG_PROTOCOL_GUID \
    {0x3152bca5, 0xeade, 0x433d, {0x86, 0x2e, 0xc0, 0x1c, 0xdc, 0x29, 0x1f, 0x44}}

//
// RNG algorithms GUID Definitions
//

typedef EFI_GUID EFI_RNG_ALGORITHM;

#define EFI_RNG_ALGORITHM_DEFAULT \
    { 0x3248e0bb, 0x4246, 0x45ab, { 0x8a, 0x1c, 0x91, 0x4b, 0x17, 0xf9, 0x64, 0x1a } }

typedef struct _EFI_RNG_PROTOCOL EFI_RNG_PROTOCOL;

//
// Interfaces Prototypes
//

typedef
EFI_STATUS
(EFIAPI *EFI_RNG_GET_INFO) (
    _In_    EFI_RNG_PROTOCOL    *This,
    _Inout_ UINTN               *RNGAlgorithmListSize,
    _Out_ EFI_RNG_ALGORITHM	    *RNGAlgorithmList
);

typedef
EFI_STATUS
(EFIAPI *EFI_RNG_GET_RNG) (
    _In_     EFI_RNG_PROTOCOL  *This,
    _In_opt_ EFI_RNG_ALGORITHM *RNGAlgorithm,
    _In_     UINTN              RNGValueLength,
    _Out_    UINT8             *RNGValue
); 

//
// Protocol Interface Structure
//

typedef struct _EFI_RNG_PROTOCOL {
	EFI_RNG_GET_INFO GetInfo;
    EFI_RNG_GET_RNG	 GetRNG;						
};	

extern EFI_GUID gEfiRngProtocolGuid;

