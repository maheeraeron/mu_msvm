/*++

Copyright (c) Microsoft Corporation

Module Name:

    SbCryptDxe.h

Abstract:

    Header file for the Secure Boot Cryptographic Driver, which implements both 
    EFI_SECUREBOOT_CRYPT_PROTOCOL and EFI_RNG_PROTOCOL protocols.

Author:

    Tejas Karandikar (tkarand) 1-Nov-2012
    Arturo Lira (davlir) 20-Mar-2013

--*/

#pragma once

#include <EfiNt.h>
#include <Uefi.h>

#include <Include/Protocol/SbCrypt.h>
#include <Include/Protocol/Rng.h>

#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>

BOOLEAN
EFIAPI
SbCryptComputeHash(
    __in HASH_ALG_ID HashAlgorithm,
    __in CONST VOID* Data,
    __in UINT32 DataLength,
     __out UINT8* HashValue,
    __inout UINT32* HashValueLength    
    );

BOOLEAN
EFIAPI
SbCryptRsaPkcs1Verify (
    __in VOID* RsaContext,
    __in UINT32 RsaContextLength,
    __in CONST UINT8* MessageHash,
    __in UINT32 HashLength,
    __in UINT8* Signature,
    __in UINT32 SigLength
    );

BOOLEAN
SbCryptPkcs7Verify (
    __in CONST UINT8* Pkcs7SignedData,
    __in UINT32 DataSize,
    __in CONST UINT8* TrustedCert,
    __in UINT32 CertSize,
    __in CONST UINT8* Pkcs7Content,
    __in UINT32 ContentSize
    );

BOOLEAN
SbCryptAuthenticodeVerify (
    __in CONST UINT8* AuthData,
    __in UINT32 DataSize,
    __in CONST UINT8* TrustedCert,
    __in UINT32 CertSize,
    __in CONST UINT8* ImageHash,
    __in UINT32 HashSize
    );

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
