/*++

Copyright (c) Microsoft Corporation

Module Name:

    SbCryptDxe.c

Abstract:

    Code file for the Secure Boot Cryptographic Driver, which implements both 
    EFI_SECUREBOOT_CRYPT_PROTOCOL and EFI_RNG_PROTOCOL protocols.

Author:

    Tejas Karandikar (tkarand) 1-Nov-2012
    Arturo Lira (davlir)       20-Mar-2013

--*/


#include "SbCryptDxe.h"
#include "BiosInterface.h"
#include <Library/BaseMemoryLib.h>
#include <Library/Baselib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

#define WITHIN_4_GB_LL (0xFFFFFFFFLL)


//
// The handle onto which the secure boot Crypt and random number generator
// protocol instances are installed
//
EFI_HANDLE  mSbCryptHandle = NULL;

//
// The secure boot crypt Protocol instance produced by this driver
//
EFI_SECUREBOOT_CRYPT_PROTOCOL  mSbCryptProtocol = {
    SbCryptComputeHash,
    SbCryptRsaPkcs1Verify,
    SbCryptPkcs7Verify,
    SbCryptAuthenticodeVerify
};

//
// The Random Number Generator Protocol instance produced by this driver
//
EFI_RNG_PROTOCOL  mRngProtocol = {
    SbCryptRngGetInfo,
    SbCryptRngGetRng
};


//
// Module variables
//

static PCRYPTO_COMMAND_DESCRIPTOR   CryptoCommandDescriptor;
static INTN                         CryptoCommandDescriptorGpa;


//
//  Private routines
//
void*
Allocate32BitMemory(
    __in UINT32 Size
    )
/*++

Routine Description:

    Allocates memory in 32-bit address space.

Arguments:

    Size - The number of bytes to allocate.

Return Value:

    None.

--*/
{
    void* answer;

    answer = (void*) WITHIN_4_GB_LL;

    if (EFI_ERROR(gBS->AllocatePages(AllocateMaxAddress,
                                     EfiBootServicesData,
                                     EFI_SIZE_TO_PAGES(Size),
                                     (EFI_PHYSICAL_ADDRESS*) &answer)))
    {
        return NULL;
    }

    return answer;
}

EFI_STATUS
IssueCryptoCommand()
/*++

Routine Description:

    Performs an enlightened NVRAM command through the BiosDevice.

Arguments:

    None.

Returns:

    EFI_STATUS

--*/
{
    //
    // Perform NVRAM command. Cast is safe, because we allocated mVariableModuleGlobal below 4GB.
    //

    IoWrite32(BiosAddressPort, BiosConfigCryptoCommand);
    IoWrite32(BiosDataPort, (UINT32)(UINTN) CryptoCommandDescriptorGpa);

    if (CryptoCommandDescriptor->Status == 0)
    {
        return EFI_SUCCESS;
    }
    else
    {
        return ENCODE_ERROR(CryptoCommandDescriptor->Status);
    }
}


BOOLEAN
EFIAPI
SbCryptComputeHash(
    __in HASH_ALG_ID HashAlgorithm,
    __in CONST VOID* Data,
    __in UINT32 DataLength,
     __out UINT8* HashValue,
    __inout UINT32* HashValueLength    
    )
/**

Routine Description:
  
    Computes a hash over a data stream of bytes

Arguments:

    HashAlgorithm - The type of hash algorithm to use

    Data - Pointer to the buffer containing the data to be hashed.
    
    DataLength - Length of Data buffer in bytes.
    
    HashValue - Pointer to a buffer that receives the hash digest value (32 bytes).
    
    HashValueLength - Pointer to the hash value length.                        

Returns:

    BOOLEAN

**/            
{
    EFI_STATUS commandStatus;
    ZeroMem(CryptoCommandDescriptor, sizeof(CRYPTO_COMMAND_DESCRIPTOR));
    CryptoCommandDescriptor->Command = CryptoComputeHash;
    CryptoCommandDescriptor->Status = EFI_DEVICE_ERROR;
    CryptoCommandDescriptor->U.ComputeHashParams.HashAlgorithm = HashAlgorithm;
    CryptoCommandDescriptor->U.ComputeHashParams.DataAddress = (UINT64) Data;
    CryptoCommandDescriptor->U.ComputeHashParams.DataLength = DataLength;
    CryptoCommandDescriptor->U.ComputeHashParams.ValueAddress = (UINT64) HashValue;
    CryptoCommandDescriptor->U.ComputeHashParams.ValueLength = *HashValueLength;

    commandStatus = IssueCryptoCommand();
    
    if (commandStatus == EFI_SUCCESS)
    {
        return TRUE;
    }
    else if (commandStatus == EFI_BUFFER_TOO_SMALL)
    {
        *HashValueLength = CryptoCommandDescriptor->U.ComputeHashParams.ValueLength;
    }

    return FALSE;
}


BOOLEAN
EFIAPI
SbCryptRsaPkcs1Verify (
    __in VOID* RsaContext,
    __in UINT32 RsaContextLength,
    __in CONST UINT8* MessageHash,
    __in UINT32 HashLength,
    __in UINT8* Signature,
    __in UINT32 SigLength
    )
/**

Routine Description:
  
    Verifies the RSA-SSA signature with EMSA-PKCS1-v1_5 encoding scheme defined in
    RSA PKCS#1.

Arguments:

    RsaContext  - Pointer to RSA context for signature verification.
    
    RsaContextLength - Length of the RSA context (exponent)
    
    MessageHash - Pointer to octet message hash to be checked.
    
    HashLength - Length of the message hash in bytes.
    
    Signature - Pointer to RSA PKCS1-v1_5 signature to be verified.
    
    SigLength - Length of signature in bytes.

Returns:

    BOOLEAN

**/          
{
    ZeroMem(CryptoCommandDescriptor, sizeof(CRYPTO_COMMAND_DESCRIPTOR));
    CryptoCommandDescriptor->Command = CryptoVerifyRsaPkcs1;
    CryptoCommandDescriptor->Status = EFI_DEVICE_ERROR;
    CryptoCommandDescriptor->U.RsaPkcs1Params.RsaContextAddress = (UINT64) RsaContext;
    CryptoCommandDescriptor->U.RsaPkcs1Params.RsaContextLength  = RsaContextLength;
    CryptoCommandDescriptor->U.RsaPkcs1Params.MessageHashAddress = (UINT64) MessageHash;
    CryptoCommandDescriptor->U.RsaPkcs1Params.MessageHashLength = HashLength;
    CryptoCommandDescriptor->U.RsaPkcs1Params.SignatureAddress = (UINT64) Signature;
    CryptoCommandDescriptor->U.RsaPkcs1Params.SignatureLength = SigLength;

    if (IssueCryptoCommand() == EFI_SUCCESS)
    {
        return TRUE;
    }
    return FALSE;
}


BOOLEAN
SbCryptPkcs7Verify (
    __in CONST UINT8* Pkcs7SignedData,
    __in UINT32 DataSize,
    __in CONST UINT8* TrustedCert,
    __in UINT32 CertSize,
    __in CONST UINT8* Pkcs7Content,
    __in UINT32 ContentSize
    )
/**

Routine Description:
  
    Verifies the PKCS#7 signature against a trusted certificate according to the PKCS#7 RFC

Arguments:

    Pkcs7SignedData - Pointer to the PKCS7 signature to be verified
    
    DataSize - Size of the PKCS7 signature
    
    TrustedCert - Pointer to the trusted X509 certificate
    
    CertSize - Size of the trusted X509 certificate
    
    Pkcs7Content - Pointer to the PKCS7 content
    
    ContentSize - Size of the PKCS7 content

Returns:

    BOOLEAN

**/              
{
    ZeroMem(CryptoCommandDescriptor, sizeof(CRYPTO_COMMAND_DESCRIPTOR));
    CryptoCommandDescriptor->Command = CryptoVerifyPkcs7;
    CryptoCommandDescriptor->Status = EFI_DEVICE_ERROR;
    CryptoCommandDescriptor->U.AuthenticodeOrPkcs7Params.AuthDataAddress = (UINT64) Pkcs7SignedData;
    CryptoCommandDescriptor->U.AuthenticodeOrPkcs7Params.AuthDataSize = (UINT32) DataSize;
    CryptoCommandDescriptor->U.AuthenticodeOrPkcs7Params.TrustedCertAddress = (UINT64) TrustedCert;
    CryptoCommandDescriptor->U.AuthenticodeOrPkcs7Params.TrustedCertSize = (UINT32) CertSize;
    CryptoCommandDescriptor->U.AuthenticodeOrPkcs7Params.HashOrPkcsDataAddress = (UINT64) Pkcs7Content;
    CryptoCommandDescriptor->U.AuthenticodeOrPkcs7Params.HashOrPkcsDataSize = (UINT32) ContentSize;
    
    if (IssueCryptoCommand() == EFI_SUCCESS)
    {
        return TRUE;
    }
    return FALSE;
}


BOOLEAN
SbCryptAuthenticodeVerify (
    __in CONST UINT8* AuthData,
    __in UINT32 DataSize,
    __in CONST UINT8* TrustedCert,
    __in UINT32 CertSize,
    __in CONST UINT8* ImageHash,
    __in UINT32 HashSize
    )
/**

Routine Description:
  
    Verifies the authenticode signature against a trusted certificate according to the Authenticode specification

Arguments:

    AuthData - Pointer to authenticode signature for signature verification.
    
    DataSize - Size of the authenticode signature
    
    TrustedCert - Pointer to the trusted X509 certificate
    
    CertSize - Size of the trusted X509 certificate
    
    ImageHash - Hash value of the image as computed by the authenticode specification
    
    HashSize  - Hash value size
    
Returns:

    BOOLEAN

**/                
{
    ZeroMem(CryptoCommandDescriptor, sizeof(CRYPTO_COMMAND_DESCRIPTOR));
    CryptoCommandDescriptor->Command = CryptoVerifyAuthenticode;
    CryptoCommandDescriptor->Status = EFI_DEVICE_ERROR;
    CryptoCommandDescriptor->U.AuthenticodeOrPkcs7Params.AuthDataAddress = (UINT64) AuthData;
    CryptoCommandDescriptor->U.AuthenticodeOrPkcs7Params.AuthDataSize = (UINT32) DataSize;
    CryptoCommandDescriptor->U.AuthenticodeOrPkcs7Params.TrustedCertAddress = (UINT64) TrustedCert;
    CryptoCommandDescriptor->U.AuthenticodeOrPkcs7Params.TrustedCertSize = (UINT32) CertSize;
    CryptoCommandDescriptor->U.AuthenticodeOrPkcs7Params.HashOrPkcsDataAddress = (UINT64) ImageHash;
    CryptoCommandDescriptor->U.AuthenticodeOrPkcs7Params.HashOrPkcsDataSize = (UINT32) HashSize;
    
    if (IssueCryptoCommand() == EFI_SUCCESS)
    {
        return TRUE;
    }
    return FALSE;
}

EFI_STATUS
EFIAPI 
SbCryptRngGetInfo (
    _In_    EFI_RNG_PROTOCOL    *This,
    _Inout_ UINTN               *RNGAlgorithmListSize,
    _Out_ EFI_RNG_ALGORITHM	    *RNGAlgorithmList
    )
/**

Routine Description:
  
    Returns information about supported RNG algorithms.

Arguments:
    
    This - A pointer to the EFI_RNG_PROTOCOL instance.

    RNGAlgorithmListSize - On input, the size in bytes of RNGAlgorithmList. On output 
                           with a return code of EFI_SUCCESS, the size in bytes of the
                           data returned in RNGAlgorithmList. With a return code of
                           EFI_BUFFER_TOO_SMALL, the size of RNGAlgorithmList required
                           to obtain the list.
    
    RNGAlgorithmList - A memory buffer filled with one EFI_RNG_ALGORITHM element for each
                       supported RNG algorithm. The list must not change across multiple 
                       calls to the same driver. The first algorithm in the list is the 
                       default algorithm for the driver.
    
Returns:

    EFI_STATUS

**/        
{
    EFI_STATUS status;
    EFI_GUID defaultAlgorithm = EFI_RNG_ALGORITHM_DEFAULT;

    //
    // Check parameters
    //

    if (This == NULL || 
        RNGAlgorithmListSize == NULL || 
        RNGAlgorithmList == NULL)
    {
        return EFI_INVALID_PARAMETER;
    }

    //
    // We just support the default algorithm.
    //

    if (sizeof(defaultAlgorithm) <= *RNGAlgorithmListSize)
    {
        *RNGAlgorithmList = defaultAlgorithm;
        status = EFI_SUCCESS;
    }
    else
    {
        status = EFI_BUFFER_TOO_SMALL;
    }

    *RNGAlgorithmListSize = sizeof(defaultAlgorithm);
    return status;
}

EFI_STATUS
EFIAPI
SbCryptRngGetRng (
    _In_     EFI_RNG_PROTOCOL  *This,
    _In_opt_ EFI_RNG_ALGORITHM *RNGAlgorithm,
    _In_     UINTN              RNGValueLength,
    _Out_    UINT8             *RNGValue
    )
/**

Routine Description:
  
    Produces and returns an RNG value using either the default or specified RNG algorithm.

Arguments:
    
    This - A pointer to the EFI_RNG_PROTOCOL instance.

    RNGAlgorithm - A pointer to the EFI_RNG_ALGORITHM that identifies the RNG algorithm to use.
                   May be NULL in which case the function will use its default RNG algorithm.
    
    RNGValueLength - The length in bytes of the memory buffer pointed to by RNGValue.

    RNGValue - A memory buffer filled with the resulting RNG value.

Returns:

    EFI_STATUS

**/       
{
    EFI_GUID defaultAlgorith = EFI_RNG_ALGORITHM_DEFAULT;

    //
    // Check parameters
    //

    if (This == NULL || 
        RNGValue == NULL)
    {
        return EFI_INVALID_PARAMETER;
    }

    //
    // We just support EFI_RNG_ALGORITHM_DEFAULT. Test if we receive this GUID
    // or it is NULL, both cases are fine. Any other GUID is a failure.
    //

    if (RNGAlgorithm != NULL &&
        !CompareGuid(&defaultAlgorith, RNGAlgorithm))
    {
        return EFI_UNSUPPORTED;
    }

    //
    // Retrieve the Random Number by issuing a command to Bios device.
    //

    ZeroMem(CryptoCommandDescriptor, sizeof(CRYPTO_COMMAND_DESCRIPTOR));
    CryptoCommandDescriptor->Command = CryptoGetRandomNumber;
    CryptoCommandDescriptor->Status = EFI_DEVICE_ERROR;
    CryptoCommandDescriptor->U.GetRandomNumberParams.BufferAddress = (UINT64) RNGValue;
    CryptoCommandDescriptor->U.GetRandomNumberParams.BufferSize = (UINT32) RNGValueLength;

    return IssueCryptoCommand();     
}


EFI_STATUS
EFIAPI
SbCryptDriverInitialize(
    __in EFI_HANDLE ImageHandle,
    __in EFI_SYSTEM_TABLE *SystemTable
    )
/**

Routine Description:
  
    Entry Point for secure boot Cryptographic Driver. This function installs secure boot Crypt Protocol.

Arguments:
    
    ImageHandle - Image handle of this driver.
    
    SystemTable - A Pointer to the EFI System Table.
    
Returns:

    EFI_STATUS

**/                  
{
    EFI_STATUS  Status;
    
    CryptoCommandDescriptor = (PCRYPTO_COMMAND_DESCRIPTOR) Allocate32BitMemory(sizeof(CRYPTO_COMMAND_DESCRIPTOR));
    
    if (CryptoCommandDescriptor == NULL)
    {
        return EFI_NOT_STARTED;
    }
    
    CryptoCommandDescriptorGpa = (UINTN) CryptoCommandDescriptor;
  
    //
    // Install the protocols onto a new handle
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                 &mSbCryptHandle, 
                 &gEfiSecureBootCryptProtocolGuid,
                 &mSbCryptProtocol,
                 &gEfiRngProtocolGuid,
                 &mRngProtocol,
                 NULL
                 );
    ASSERT_EFI_ERROR (Status);
  
    return Status;
}
