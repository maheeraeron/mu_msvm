/** @file -- Tcg2PreInitLibPei.c
Tpm2 intialization hooks specific to the vDevice in Hyper-V.

Copyright (c) 2018, Microsoft Corporation

All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**/

#include <Uefi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/Tpm2DeviceLib.h>

#include <TpmInterface.h>           // Definitions specific to Hyper-V VDev.


VOID
WriteTpmPort(
    IN UINT32 AddressRegisterValue,
    IN UINT32 DataRegisterValue
);

UINT32
ReadTpmPort(
    IN UINT32 AddressRegisterValue
);

/**
  Performs basic, one-time initialization for the Hyper-V TPM vDevice.
  Will allocate a CRB buffer and configure that buffer with the device.

  @retval     EFI_SUCCESS   Everything is fine. Continue with init.
  @retval     Others        Something has gone wrong. Do not initialize TPM any further.

**/
EFI_STATUS
EFIAPI
HyperVTpmDeviceInitEarlyBoot(
	VOID
  )
{
    EFI_STATUS            Status = EFI_SUCCESS;
    EFI_PHYSICAL_ADDRESS  CrBuffer = 0;
    UINT32                TpmIoEstablishedResponse;
    UINT64                TpmBaseAddress;

    Status = PeiServicesAllocatePages(EfiRuntimeServicesData, 2, &CrBuffer);
    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, __FUNCTION__" - Failed to allocate CRB for TPM VDev!\n"));
        return Status;
    }

    if (CrBuffer > 0xFFFFFFFFULL) {
        // PEI memory was published as - Base at 1MB, size max 64MB.
        // It is guaranteed that physical address is below 4 GB.
        DEBUG((DEBUG_ERROR, __FUNCTION__" - CRB allocation for TPM VDev is incorrect!\n"));
        ASSERT(FALSE);
        return EFI_DEVICE_ERROR;
    }

    DEBUG((DEBUG_VERBOSE, __FUNCTION__" - CrBuffer == 0x%016lX\n", CrBuffer));

    ZeroMem((UINT8*)CrBuffer, 2 * EFI_PAGE_SIZE);

    TpmBaseAddress = FixedPcdGet64(PcdTpmBaseAddress);
    TpmBaseAddress += PcdGetBool(PcdTpmLocalityRegsEnabled) ? 0x40 : 0;

    DEBUG((DEBUG_VERBOSE, __FUNCTION__" - TpmBaseAddress == 0x%016lX\n", TpmBaseAddress));

    //
    // Send the request to the TPM VDev.
    // Cast of command buffer GPA is safe as it was allocated below 4GB.
    //
    WriteTpmPort(TpmIoMapSharedMemory, (UINT32)CrBuffer);

    //
    // Query vDev the mapping result
    //
    TpmIoEstablishedResponse = ReadTpmPort(TpmIoEstablished);
    if (TpmIoEstablishedResponse == 0) {
        //
        // Couldn't establish memory mapping with vDev.
        //
        DEBUG((DEBUG_ERROR, __FUNCTION__" - Couldn't establish memory mapping with vDev!\n"));
        return EFI_NO_MAPPING;
    }

    DEBUG((DEBUG_VERBOSE, __FUNCTION__" - TpmIoEstablishedResponse == 0x%08X\n", TpmIoEstablishedResponse));

    Tpm2RegisterTpm2DeviceLib((TPM2_DEVICE_INTERFACE*)TpmBaseAddress);

    return Status;
} // HyperVTpmDeviceInitEarlyBoot()


/**
  Constructor for the lib.
  Important that this runs prior to Tcg2Pei because it may disable some of
  the intended functionality.

  IMPORTANT NOTE: Because Tcg2Pei requests to be shadowed, this constructor
                  will be invoked twice. We need to make sure that we don't perform
                  some of these behaviors twice.

  @retval EFI_SUCCESS     The entry point executed successfully.
  @retval other           Some error occured when executing this entry point.

**/
EFI_STATUS
EFIAPI
HyperVTpm2InitLibConstructorPei (
  IN       EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  )
{
  EFI_STATUS        Status = EFI_SUCCESS;
  BOOLEAN           TpmEnabled = FALSE;
  UINTN             GuidSize = sizeof( EFI_GUID );
  static BOOLEAN    EarlyInitComplete = FALSE;

  DEBUG(( DEBUG_INFO, __FUNCTION__"()\n" ));

  //
  // If the TPM is disabled in the Hyper-V UI, don't perform
  // any more TPM init.
  // NOTE: Should occur after the PlatformPei init module from Hyper-V.
  //       This is because of the depex on gEfiPeiMasterBootModePpiGuid.
  TpmEnabled = PcdGetBool( PcdTpmEnabled );
  if (!TpmEnabled) {
    DEBUG(( DEBUG_INFO, __FUNCTION__" - Detected a disabled TPM. Bypassing init.\n" ));
    Status = PcdSetPtrS( PcdTpmInstanceGuid, &GuidSize, &gEfiTpmDeviceInstanceNoneGuid );
    if (EFI_ERROR(Status))
    {
        DEBUG((DEBUG_ERROR, __FUNCTION__" - Failed to set the PCD PcdTpmInstanceGuid::0x%x \n", Status));
        ASSERT_EFI_ERROR( Status );
    } 
  }

  //
  // If we're still good to continue init, perform the required Hyper-V init.
  if (TpmEnabled && !EarlyInitComplete) {
    Status = HyperVTpmDeviceInitEarlyBoot();
    if (EFI_ERROR( Status )) {
      DEBUG(( DEBUG_ERROR, __FUNCTION__" - HyperVTpmDeviceInitEarlyBoot() returned %r!\n", Status ));
      ASSERT_EFI_ERROR( Status );
    }
    EarlyInitComplete = TRUE;
  }

  // return Status;
  return EFI_SUCCESS;     // Library constructors ASSERT if anything other than EFI_SUCCESS is returned.
} // HyperVTpm2InitLibConstructor()
