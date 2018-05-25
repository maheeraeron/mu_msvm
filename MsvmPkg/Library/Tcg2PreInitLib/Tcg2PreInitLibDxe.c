/** @file -- Tcg2PreInitLibDxe.c
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
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/Tpm2DeviceLib.h>

#include <TpmInterface.h>           // Definitions specific to Hyper-V VDev.


// Prototype for function in Tpm2Acpi.c
EFI_STATUS
EFIAPI
IntallTpm2AcpiTable (
    VOID
    );


/**
  Constructor for the lib.
  Important that this runs prior to Tcg2Dxe because it may disable some of
  the intended functionality.

  @retval EFI_SUCCESS     The entry point executed successfully.
  @retval other           Some error occured when executing this entry point.

**/
EFI_STATUS
EFIAPI
HyperVTpm2InitLibConstructor (
  IN    EFI_HANDLE                  ImageHandle,
  IN    EFI_SYSTEM_TABLE            *SystemTable
  )
{
  EFI_STATUS    Status = EFI_SUCCESS;
  UINT32        TcgProtocolVersion;

  DEBUG(( DEBUG_INFO, __FUNCTION__"()\n" ));

  //
  // If the TPM is disabled in the Hyper-V UI, don't perform
  // any more TPM init.
  if (CompareGuid (PcdGetPtr(PcdTpmInstanceGuid), &gEfiTpmDeviceInstanceNoneGuid) ||
      CompareGuid (PcdGetPtr(PcdTpmInstanceGuid), &gEfiTpmDeviceInstanceTpm12Guid)){
    DEBUG ((DEBUG_INFO, "No TPM2 instance required!\n"));
    return EFI_SUCCESS;
  }

  //
  // Query vDev the Tcg protocol version
  IoWrite32( TpmControlPort, TpmIoGetTcgProtocolVersion );
  TcgProtocolVersion = IoRead32( TpmDataPort );
  if ((TcgProtocolVersion != TcgProtocolTrEE) && (TcgProtocolVersion != TcgProtocolTcg2)) {
    DEBUG(( DEBUG_ERROR, __FUNCTION__" - TPM vDev reports bad version! 0x%X\n", TcgProtocolVersion ));
    Status = EFI_DEVICE_ERROR;
  }

  // If we're good, we need to make sure that our instance of Tpm2DeviceLib
  // can talk with the vDevice.
  if (!EFI_ERROR( Status )) {
    Tpm2RegisterTpm2DeviceLib( (TPM2_DEVICE_INTERFACE*)(UINTN)FixedPcdGet32( PcdTpmBaseAddress ) );
    Status = IntallTpm2AcpiTable();
  }

  // NOTE: This will cause an ASSERT if the TCG protocol version is incorrect.
  //       It is assumed this would indicate a software misconfiguration.
  return Status;
} // HyperVTpm2InitLibConstructor()
