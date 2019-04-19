/*++

Copyright (c) Microsoft Corporation

Module Name:

    SbCryptDxe.h

Abstract:

    Header file for the Secure Boot Cryptographic Driver, which implements the
    EFI_RNG_PROTOCOL protocol.

    Before RS5, we used to implement an additional protocol which provided crypto
    functions for secure boot, but this is no longer necessary as we now use
    OpenSSL.

Author:

    Tejas Karandikar (tkarand) 1-Nov-2012
    Arturo Lira (davlir) 20-Mar-2013

--*/

#pragma once

#include <EfiNt.h>
#include <Uefi.h>

#include <Include/Protocol/Rng.h>

#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS
EFIAPI
SbCryptRngGetInfo (
    _In_    EFI_RNG_PROTOCOL    *This,
    _Inout_ UINTN               *RNGAlgorithmListSize,
    _Out_   EFI_RNG_ALGORITHM	*RNGAlgorithmList
    );

EFI_STATUS
EFIAPI
SbCryptRngGetRng (
    _In_     EFI_RNG_PROTOCOL  *This,
    _In_opt_ EFI_RNG_ALGORITHM *RNGAlgorithm,
    _In_     UINTN              RNGValueLength,
    _Out_    UINT8             *RNGValue
    );
