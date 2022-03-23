/*++

Copyright (c) Microsoft Corporation

Module Name:

    Rng.c

Abstract:

    Random number generator services that uses RdRand instruction access
    to provide high-quality random numbers when RdRand is present. Otherwise,
    it relies on host emulation for random number generation.
    If host emulation is used, it is required to run this lib in memory.

--*/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

// MS_HYP_CHANGE BEGIN
#include <BiosInterface.h>
#include <Library/BiosDeviceLib.h>
#include <Library/BaseMemoryLib.h>
#include <Uefi.h>
#include <Uefi/UefiBaseType.h>
#include <Library/UefiBootServicesTableLib.h>

#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)
#include <IsolationTypes.h>
#endif

#include <FailFast.h>

#define RNG 0x524E47 // "RNG"

#define WITHIN_4_GB_LL (0xFFFFFFFFLL)

static PCRYPTO_COMMAND_DESCRIPTOR   mCryptoCommandDescriptor;
static EFI_PHYSICAL_ADDRESS         mCryptoCommandDescriptorGpa;

//
// Determine if the Random Number Generaiton should rely on the host emulation
// instead of using the hardware support (RDRAND).
//
static BOOLEAN mUseHostEmulation;

// MS_HYP_CHANGE END


//
// Bit mask used to determine if RdRand instruction is supported.
//
#define RDRAND_MASK                  BIT30

//
// Limited retry number when valid random data is returned.
// Uses the recommended value defined in Section 7.3.17 of "Intel 64 and IA-32
// Architectures Software Developer's Mannual".
//
#define RDRAND_RETRY_LIMIT           10

// MS_HYP_CHANGE BEGIN

/**
  Generates a random number using host emulation if host emulation is configured.

  @param[in] SizeInBytes  Number of bytes for generating the random number.
  @param[out] Rand        Buffer pointer to store the random value.

  @retval TRUE            Random number generated successfully.
  @retval FALSE           Failed to generate the random number.

**/
BOOLEAN
ProcessUsingHostEmulation (
  UINTN SizeInBytes,
  OUT UINT8  *Rand
  )
{
  //
  // We should never be sending more than 8 bytes for this implementation.
  // Any requests coming in for larger buffers should be chunked before reaching
  // here.
  //
  ASSERT(SizeInBytes <= 8);

  //
  // Retrieve the Random Number by issuing a command to Bios device.
  //
  ZeroMem(mCryptoCommandDescriptor, sizeof(CRYPTO_COMMAND_DESCRIPTOR));
  mCryptoCommandDescriptor->Command = CryptoGetRandomNumber;
  mCryptoCommandDescriptor->Status = EFI_DEVICE_ERROR;
  mCryptoCommandDescriptor->U.GetRandomNumberParams.BufferAddress = (UINT64) Rand;
  mCryptoCommandDescriptor->U.GetRandomNumberParams.BufferSize = (UINT32) SizeInBytes;

  //
  // Perform NVRAM command.
  // Cast of descriptor is safe because we allocated mVariableModuleGlobal below 4GB.
  //
  WriteBiosDevice(BiosConfigCryptoCommand, (UINT32)mCryptoCommandDescriptorGpa);

  if (mCryptoCommandDescriptor->Status == 0)
  {
    return TRUE;
  }
  else
  {
    DEBUG((DEBUG_ERROR, "%a: Host emulation failed - %r \n", __FUNCTION__, ENCODE_ERROR(mCryptoCommandDescriptor->Status)));
    return FALSE;
  }

}

// MS_HYP_CHANGE END

/**
  The constructor function checks whether or not to use the RDRAND instruction.

  The constructor function checks whether or not RDRAND instruction is supported
  and the isolation status. If we are running isolated, we must have RDRAND present
  and not rely on the host emulation for getting random numbers.

  @param ImageHandle        Image handle this driver.
  @param SystemTable        Pointer to the System Table.

  @retval RETURN_SUCCESS    The constructor always returns EFI_SUCCESS.

**/
RETURN_STATUS
EFIAPI
RngLibConstructor (
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
  )
  
{
  // MS_HYP_CHANGE BEGIN
#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)

  UINT32  RegEcx;

  //
  // Determine RDRAND support by examining bit 30 of the ECX register returned by
  // CPUID. A value of 1 indicates that processor support RDRAND instruction.
  //
  AsmCpuid (1, 0, 0, &RegEcx, 0);
  if ((RegEcx & RDRAND_MASK) == RDRAND_MASK)
  {
    DEBUG((DEBUG_INFO, "%a: RDRAND present.\n", __FUNCTION__));
    mUseHostEmulation = FALSE;
  }
  else
  {
    //
    // If we are running isolated, we must use RDRAND for a secure implementation of 
    // random number generation.
    //
    if (IsHardwareIsolated()) 
    {
      DEBUG((DEBUG_ERROR, "%a: RDRAND is not present on an isolated guest..\n", __FUNCTION__));
      FAIL_FAST(CRITICAL_INITIALIZATION_FAILURE, RNG, __LINE__, GetIsolationType());
    }

    DEBUG((DEBUG_INFO, "%a: RDRAND is not present. Using host emulation.\n", __FUNCTION__));
    mUseHostEmulation = TRUE;
  }
#elif defined(MDE_CPU_AARCH64)
  DEBUG((DEBUG_VERBOSE, "%a: Using host emulation due to architectural limitations.\n", __FUNCTION__));
  mUseHostEmulation = TRUE;
#else
#error Unsupported Architecture
#endif

  if (mUseHostEmulation)
  {

    EFI_PHYSICAL_ADDRESS address = WITHIN_4_GB_LL;

    if (EFI_ERROR(gBS->AllocatePages(AllocateMaxAddress,
                                     EfiBootServicesData,
                                     EFI_SIZE_TO_PAGES(sizeof(CRYPTO_COMMAND_DESCRIPTOR)),
                                     &address)))
    {
      // Fail fast since there is no way forward from this failure.
      FAIL_FAST(CRITICAL_INITIALIZATION_FAILURE, RNG, __LINE__, 0);
    }

    mCryptoCommandDescriptor = (PCRYPTO_COMMAND_DESCRIPTOR) address;

    if (mCryptoCommandDescriptor == NULL)
    {
      // Fail fast since there is no way forward from this failure.
      FAIL_FAST(CRITICAL_INITIALIZATION_FAILURE, RNG, __LINE__, 0);
    }

    mCryptoCommandDescriptorGpa = (EFI_PHYSICAL_ADDRESS) mCryptoCommandDescriptor;
  }


  return RETURN_SUCCESS;
  // MS_HYP_CHANGE END
}

/**
  Generates a 16-bit random number.

  if Rand is NULL, then ASSERT().

  @param[out] Rand     Buffer pointer to store the 16-bit random value.

  @retval TRUE         Random number generated successfully.
  @retval FALSE        Failed to generate the random number.

**/
BOOLEAN
EFIAPI
GetRandomNumber16 (
  OUT     UINT16                    *Rand
  )
{
  ASSERT (Rand != NULL);
  if (mUseHostEmulation)
  {
    return ProcessUsingHostEmulation(2, (UINT8 *)Rand);  // MS_HYP_CHANGE    
  }
  else
  {
#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)
    UINT32  Index;
    //
    // A loop to fetch a 16 bit random value with a retry count limit.
    //
    for (Index = 0; Index < RDRAND_RETRY_LIMIT; Index++) {
      if (AsmRdRand16 (Rand)) {
        return TRUE;
      }
    }
#else
  // This is unexpected.
  ASSERT(FALSE);
#endif
  }

  return FALSE;
}

/**
  Generates a 32-bit random number.

  if Rand is NULL, then ASSERT().

  @param[out] Rand     Buffer pointer to store the 32-bit random value.

  @retval TRUE         Random number generated successfully.
  @retval FALSE        Failed to generate the random number.

**/
BOOLEAN
EFIAPI
GetRandomNumber32 (
  OUT     UINT32                    *Rand
  )
{
  ASSERT (Rand != NULL);
  if (mUseHostEmulation)
  {
    return ProcessUsingHostEmulation(4, (UINT8 *)Rand);  // MS_HYP_CHANGE
  }
  else
  {
#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)
    UINT32  Index;

    //
    // A loop to fetch a 32 bit random value with a retry count limit.
    //
    for (Index = 0; Index < RDRAND_RETRY_LIMIT; Index++) {
      if (AsmRdRand32 (Rand)) {
        return TRUE;
      }
    }
#else
  // This is unexpected.
  ASSERT(FALSE);
#endif
  }

  return FALSE;
}

/**
  Generates a 64-bit random number.

  if Rand is NULL, then ASSERT().

  @param[out] Rand     Buffer pointer to store the 64-bit random value.

  @retval TRUE         Random number generated successfully.
  @retval FALSE        Failed to generate the random number.

**/
BOOLEAN
EFIAPI
GetRandomNumber64 (
  OUT     UINT64                    *Rand
  )
{
  ASSERT (Rand != NULL);
  if (mUseHostEmulation)
  {
    return ProcessUsingHostEmulation(8, (UINT8 *)Rand);  // MS_HYP_CHANGE
  }
  else
  {
#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)
    UINT32  Index;

    //
    // A loop to fetch a 64 bit random value with a retry count limit.
    //
    for (Index = 0; Index < RDRAND_RETRY_LIMIT; Index++) {
      if (AsmRdRand64 (Rand)) {
        return TRUE;
      }
    }
#else
  // This is unexpected.
  ASSERT(FALSE);
#endif
  }
  return FALSE;
}

/**
  Generates a 128-bit random number.

  if Rand is NULL, then ASSERT().

  @param[out] Rand     Buffer pointer to store the 128-bit random value.

  @retval TRUE         Random number generated successfully.
  @retval FALSE        Failed to generate the random number.

**/
BOOLEAN
EFIAPI
GetRandomNumber128 (
  OUT     UINT64                    *Rand
  )
{
  ASSERT (Rand != NULL);

  //
  // Read first 64 bits
  //
  if (!GetRandomNumber64 (Rand)) {
    return FALSE;
  }

  //
  // Read second 64 bits
  //
  return GetRandomNumber64 (++Rand);
}
