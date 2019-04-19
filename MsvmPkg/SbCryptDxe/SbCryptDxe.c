/*++

Copyright (c) Microsoft Corporation

Module Name:

    SbCryptDxe.c

Abstract:

    Code file for the Secure Boot Cryptographic Driver, which implement the
    EFI_RNG_PROTOCOL protocol.

    Before RS5, we used to implement an additional protocol which provided crypto
    functions for secure boot, but this is no longer necessary as we now use
    OpenSSL.

Author:

    Tejas Karandikar (tkarand) 1-Nov-2012
    Arturo Lira (davlir)       20-Mar-2013

--*/


#include "SbCryptDxe.h"
#include <Library/BaseMemoryLib.h>
#include <Library/Baselib.h>
#include <Library/BiosDeviceLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <BiosInterface.h>

#define WITHIN_4_GB_LL (0xFFFFFFFFLL)

#define EFI_RNG_ALGORITHM_DEFAULT \
    { 0x3248e0bb, 0x4246, 0x45ab, { 0x8a, 0x1c, 0x91, 0x4b, 0x17, 0xf9, 0x64, 0x1a } }

//
// The handle onto which the random number generator protocol instance is installed
//
EFI_HANDLE  mSbCryptHandle = NULL;

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
static EFI_PHYSICAL_ADDRESS         CryptoCommandDescriptorGpa;


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
    EFI_PHYSICAL_ADDRESS answer;

    answer = WITHIN_4_GB_LL;

    if (EFI_ERROR(gBS->AllocatePages(AllocateMaxAddress,
                                     EfiBootServicesData,
                                     EFI_SIZE_TO_PAGES(Size),
                                     &answer)))
    {
        return NULL;
    }

    return (void*)(UINTN)answer;
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
    // Perform NVRAM command.
    // Cast of descriptor is safe because we allocated mVariableModuleGlobal below 4GB.
    //
    WriteBiosDevice(BiosConfigCryptoCommand, (UINT32)CryptoCommandDescriptorGpa);

    if (CryptoCommandDescriptor->Status == 0)
    {
        return EFI_SUCCESS;
    }
    else
    {
        return ENCODE_ERROR(CryptoCommandDescriptor->Status);
    }
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

    CryptoCommandDescriptorGpa = (EFI_PHYSICAL_ADDRESS) CryptoCommandDescriptor;

    //
    // Install the protocol onto a new handle
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                 &mSbCryptHandle,
                 &gEfiRngProtocolGuid,
                 &mRngProtocol,
                 NULL
                 );
    ASSERT_EFI_ERROR (Status);

    return Status;
}
