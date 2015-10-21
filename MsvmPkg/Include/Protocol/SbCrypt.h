/*++

Copyright (c) Microsoft Corporation

Module Name:

    SbCrypt.h

Abstract:

    Provides the protocol definition for EFI_SECUREBOOT_CRYPT_PROTOCOL, which provides
    cryptographic services for UEFI secure boot

Author:

    Tejas Karandikar (tkarand) 1-Nov-2012

--*/
#pragma once
#include "BiosInterface.h"

#ifndef __EFI_SECUREBOOT_CRYPT_PROTOCOL__
#define __EFI_SECUREBOOT_CRYPT_PROTOCOL__


///
/// Secure boot Cryptographic Protocol GUID.
/// 04e5c836-ff76-4eb9-8de8-f7f7a921fab6
//
#define EFI_SECUREBOOT_CRYPT_PROTOCOL_GUID \
{ \
    0x04e5c836, 0xff76, 0x4eb9, { 0x8d, 0xe8, 0xf7, 0xf7, 0xa9, 0x21, 0xfa, 0xb6 } \
}

typedef
BOOLEAN
(EFIAPI *EFI_SECUREBOOT_CRYPT_COMPUTE_HASH) (
    __in HASH_ALG_ID HashAlgorithm,
    __in CONST VOID* Data,
    __in UINT32 DataLength,
     __out UINT8* HashValue,
    __inout UINT32* HashValueLength    
    );

typedef
BOOLEAN
(EFIAPI *EFI_SECUREBOOT_CRYPT_RSA_PKCS1_VERIFY) (
    __in VOID* RsaContext,
    __in UINT32 RsaContextLength,
    __in CONST UINT8* MessageHash,
    __in UINT32 HashLength,
    __in UINT8* Signature,
    __in UINT32 SigLength
    );

typedef
BOOLEAN
(EFIAPI *EFI_SECUREBOOT_CRYPT_PKCS7_VERIFY) (
    __in CONST UINT8* Pkcs7SignedData,
    __in UINT32 DataSize,
    __in CONST UINT8* TrustedCert,
    __in UINT32 CertSize,
    __in CONST UINT8* Pkcs7Content,
    __in UINT32 ContentSize
    );

typedef
BOOLEAN
(EFIAPI *EFI_SECUREBOOT_CRYPT_AUTHENTICODE_VERIFY) (
    __in CONST UINT8* AuthData,
    __in UINT32 DataSize,
    __in CONST UINT8* TrustedCert,
    __in UINT32 CertSize,
    __in CONST UINT8* ImageHash,
    __in UINT32 HashSize
    );


///
/// Runtime Cryptographic Protocol Structure.
///
typedef struct {
    EFI_SECUREBOOT_CRYPT_COMPUTE_HASH          ComputeHash;
    EFI_SECUREBOOT_CRYPT_RSA_PKCS1_VERIFY      RsaPkcs1Verify;
    EFI_SECUREBOOT_CRYPT_PKCS7_VERIFY          Pkcs7Verify;
    EFI_SECUREBOOT_CRYPT_AUTHENTICODE_VERIFY   AuthenticodeVerify;
} EFI_SECUREBOOT_CRYPT_PROTOCOL;

extern EFI_GUID gEfiSecureBootCryptProtocolGuid;

#endif
