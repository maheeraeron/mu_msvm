/** @file
  Initialize TPM2 device and measure FVs before handing off control to DXE.

Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiPei.h>

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Ppi/FirmwareVolumeInfo.h>
#include <Ppi/LockPhysicalPresence.h>
#include <Ppi/TpmInitialized.h>
#include <Ppi/FirmwareVolume.h>
#include <Ppi/EndOfPeiPhase.h>
#include <Ppi/FirmwareVolumeInfoMeasurementExcluded.h>

#include <Guid/TcgEventHob.h>
#include <Guid/MeasuredFvHob.h>
#include <Guid/TpmInstance.h>

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/HashLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Protocol/TrEEProtocol.h>
#include <Library/PerformanceLib.h>
#include <IndustryStandard/Tpm2Acpi.h>

#include <Ppi/ConfigPpi.h>
#include <tpminterface.h>
#include <BiosInterface.h>
#include <BiosConfigPageGuid.h>

#define PERF_ID_TREE_PEI  0x3080

typedef struct {
  EFI_GUID               *EventGuid;
  TREE_EVENT_LOG_FORMAT  LogFormat;
  UINT32                 BootHashAlg;
  UINT16                 DigestAlgID;
  TPMI_ALG_HASH          TpmHashAlgo;
} TREE_EVENT_INFO_STRUCT;

TREE_EVENT_INFO_STRUCT mTreeEventInfo[] =
{
    {&gTcgEventEntryHobGuid, TREE_EVENT_LOG_FORMAT_TCG_1_2, TREE_BOOT_HASH_ALG_SHA1, 0, TPM_ALG_SHA1},
};

EFI_PEI_FILE_HANDLE     mFileHandle;

EFI_PEI_PPI_DESCRIPTOR  mTpmInitializedPpiList = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gPeiTpmInitializedPpiGuid,
  NULL
};

EFI_PLATFORM_FIRMWARE_BLOB mMeasuredBaseFvInfo[FixedPcdGet32 (PcdPeiCoreMaxFvSupported)];
UINT32 mMeasuredBaseFvIndex = 0;

EFI_PLATFORM_FIRMWARE_BLOB mMeasuredChildFvInfo[FixedPcdGet32 (PcdPeiCoreMaxFvSupported)];
UINT32 mMeasuredChildFvIndex = 0;

BOOLEAN mFirmwareDebuggerEnabled = FALSE;


/**
  Measure and record the Firmware Volum Information once FvInfoPPI install.

  @param[in] PeiServices       An indirect pointer to the EFI_PEI_SERVICES table published by the PEI Foundation.
  @param[in] NotifyDescriptor  Address of the notification descriptor data structure.
  @param[in] Ppi               Address of the PPI that was installed.

  @retval EFI_SUCCESS          The FV Info is measured and recorded to TPM.
  @return Others               Fail to measure FV.

**/
EFI_STATUS
EFIAPI
FirmwareVolmeInfoPpiNotifyCallback (
  IN EFI_PEI_SERVICES              **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR     *NotifyDescriptor,
  IN VOID                          *Ppi
  );

/**
  Record all measured Firmware Volum Information into a Guid Hob

  @param[in] PeiServices       An indirect pointer to the EFI_PEI_SERVICES table published by the PEI Foundation.
  @param[in] NotifyDescriptor  Address of the notification descriptor data structure.
  @param[in] Ppi               Address of the PPI that was installed.

  @retval EFI_SUCCESS          The FV Info is measured and recorded to TPM.
  @return Others               Fail to measure FV.

**/
EFI_STATUS
EFIAPI
EndofPeiSignalNotifyCallBack (
  IN EFI_PEI_SERVICES              **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR     *NotifyDescriptor,
  IN VOID                          *Ppi
  );

EFI_PEI_NOTIFY_DESCRIPTOR           mNotifyList[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK,
    &gEfiPeiFirmwareVolumeInfoPpiGuid,
    FirmwareVolmeInfoPpiNotifyCallback
  },
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiEndOfPeiSignalPpiGuid,
    EndofPeiSignalNotifyCallBack
  }
};

EFI_PEI_FIRMWARE_VOLUME_INFO_MEASUREMENT_EXCLUDED_PPI *mMeasurementExcludedFvPpi;


/**
  This function get digest from digest list.

  @param HashAlg    digest algorithm
  @param DigestList digest list
  @param Digest     digest

  @retval EFI_SUCCESS   Sha1Digest is found and returned.
  @retval EFI_NOT_FOUND Sha1Digest is not found.
**/
EFI_STATUS
Tpm2GetDigestFromDigestList (
    IN  TPMI_ALG_HASH      HashAlg,
    IN  TPML_DIGEST_VALUES *DigestList,
    OUT VOID               *Digest
    )
{
    UINTN  index;
    UINT16 digestSize;

    digestSize = GetHashSizeFromAlgo (HashAlg);
    // Support Sha1Digest only. Sha2Digest has larger buffer size.
    ASSERT(digestSize == SHA1_DIGEST_SIZE);
    for (index = 0; index < DigestList->count; index++)
    {
        if (DigestList->digests[index].hashAlg == HashAlg)
        {
            CopyMem (Digest, &DigestList->digests[index].digest, digestSize);
            return EFI_SUCCESS;
        }
    }

    return EFI_NOT_FOUND;
}

/**
  Record all measured Firmware Volum Information into a Guid Hob
  Guid Hob payload layout is

     UINT32 *************************** FIRMWARE_BLOB number
     EFI_PLATFORM_FIRMWARE_BLOB******** BLOB Array

  @param[in] PeiServices       An indirect pointer to the EFI_PEI_SERVICES table published by the PEI Foundation.
  @param[in] NotifyDescriptor  Address of the notification descriptor data structure.
  @param[in] Ppi               Address of the PPI that was installed.

  @retval EFI_SUCCESS          The FV Info is measured and recorded to TPM.
  @return Others               Fail to measure FV.

**/
EFI_STATUS
EFIAPI
EndofPeiSignalNotifyCallBack (
  IN EFI_PEI_SERVICES              **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR     *NotifyDescriptor,
  IN VOID                          *Ppi
  )
{
    MEASURED_HOB_DATA *MeasuredHobData;

    MeasuredHobData = NULL;

    //
    // Create a Guid hob to save all measured Fv
    //
    MeasuredHobData = BuildGuidHob(
                      &gMeasuredFvHobGuid,
                      sizeof(UINTN) + sizeof(EFI_PLATFORM_FIRMWARE_BLOB) * (mMeasuredBaseFvIndex + mMeasuredChildFvIndex)
                      );

    if (MeasuredHobData != NULL)
    {
        //
        // Save measured FV info enty number
        //
        MeasuredHobData->Num = mMeasuredBaseFvIndex + mMeasuredChildFvIndex;

        //
        // Save measured base Fv info
        //
        CopyMem(MeasuredHobData->MeasuredFvBuf, mMeasuredBaseFvInfo, sizeof(EFI_PLATFORM_FIRMWARE_BLOB) * (mMeasuredBaseFvIndex));

        //
        // Save measured child Fv info
        //
        CopyMem(&MeasuredHobData->MeasuredFvBuf[mMeasuredBaseFvIndex] , mMeasuredChildFvInfo, sizeof(EFI_PLATFORM_FIRMWARE_BLOB) * (mMeasuredChildFvIndex));
    }

    return EFI_SUCCESS;
}

/**
  Add a new entry to the Event Log.

  @param[in]     DigestList    A list of digest.
  @param[in,out] NewEventHdr   Pointer to a TCG_PCR_EVENT_HDR data structure.
  @param[in]     NewEventData  Pointer to the new event data.

  @retval EFI_SUCCESS           The new event log entry was added.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to log the new event.
**/
EFI_STATUS
LogHashEvent (
    IN TPML_DIGEST_VALUES        *DigestList,
    IN OUT  TCG_PCR_EVENT_HDR    *NewEventHdr,
    IN UINT8                     *NewEventData
    )
{
    VOID        *hobData;
    EFI_STATUS  status = EFI_SUCCESS;

    status = Tpm2GetDigestFromDigestList(TPM_ALG_SHA1, DigestList, &NewEventHdr->Digest);
    if (EFI_ERROR (status))
    {
        goto Cleanup;
    }

    hobData = BuildGuidHob(
             &gTcgEventEntryHobGuid,
             sizeof(*NewEventHdr) + NewEventHdr->EventSize
             );
    if (hobData == NULL)
    {
        status = EFI_OUT_OF_RESOURCES;
        goto Cleanup;
    }

    CopyMem(hobData, NewEventHdr, sizeof(*NewEventHdr));
    hobData = (VOID *) ((UINT8*)hobData + sizeof(*NewEventHdr));
    CopyMem(hobData, NewEventData, NewEventHdr->EventSize);

Cleanup:

    return status;
}

/**
  Do a hash operation on a data buffer, extend a specific TPM PCR with the hash result,
  and build a GUIDed HOB recording the event which will be passed to the DXE phase and
  added into the Event Log.

  @param[in]      Flags         Bitmap providing additional information.
  @param[in]      HashData      Physical address of the start of the data buffer
                                to be hashed, extended, and logged.
  @param[in]      HashDataLen   The length, in bytes, of the buffer referenced by HashData.
  @param[in]      NewEventHdr   Pointer to a TCG_PCR_EVENT_HDR data structure.
  @param[in]      NewEventData  Pointer to the new event data.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to log the new event.
  @retval EFI_DEVICE_ERROR      The command was unsuccessful.

**/
EFI_STATUS
HashLogExtendEvent (
  IN      UINT64                    Flags,
  IN      UINT8                     *HashData,
  IN      UINTN                     HashDataLen,
  IN      TCG_PCR_EVENT_HDR         *NewEventHdr,
  IN      UINT8                     *NewEventData
  )
{
    EFI_STATUS          status = EFI_SUCCESS;
    TPML_DIGEST_VALUES  digestList;

    status = HashAndExtend (
             NewEventHdr->PCRIndex,
             HashData,
             HashDataLen,
             &digestList
             );
    if (EFI_ERROR (status))
    {
        goto Cleanup;
    }

    if ((Flags & TREE_EXTEND_ONLY) == 0)
    {
        status = LogHashEvent(&digestList, NewEventHdr, NewEventData);
    }

Cleanup:

    return status;
}


/**
  Measure and log launch of FirmwareDebugger, and extend the measurement result into PCR[7].

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.

**/
EFI_STATUS
MeasureLaunchOfFirmwareDebugger()
{
    TCG_PCR_EVENT_HDR       TcgEventHdr;

    TcgEventHdr.PCRIndex  = 7;
    TcgEventHdr.EventType = EV_EFI_ACTION;
    TcgEventHdr.EventSize = sizeof(FIRMWARE_DEBUGGER_EVENT_STRING) - 1;
    return HashLogExtendEvent (
           0,
           (UINT8 *)FIRMWARE_DEBUGGER_EVENT_STRING,
           sizeof(FIRMWARE_DEBUGGER_EVENT_STRING) - 1,
           &TcgEventHdr,
           (UINT8 *)FIRMWARE_DEBUGGER_EVENT_STRING
           );
}


/**
  Measure CRTM version.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to log the new event.
  @retval EFI_DEVICE_ERROR      The command was unsuccessful.

**/
EFI_STATUS
MeasureCRTMVersion ()
{
    TCG_PCR_EVENT_HDR                 TcgEventHdr;

    //
    // Use FirmwareVersion string to represent CRTM version.
    // OEMs should get real CRTM version string and measure it.
    //

    TcgEventHdr.PCRIndex  = 0;
    TcgEventHdr.EventType = EV_S_CRTM_VERSION;
    TcgEventHdr.EventSize = (UINT32) StrSize((CHAR16*)PcdGetPtr (PcdFirmwareVersionString));

    return HashLogExtendEvent (
           0,
           (UINT8*)PcdGetPtr (PcdFirmwareVersionString),
           TcgEventHdr.EventSize,
           &TcgEventHdr,
           (UINT8*)PcdGetPtr (PcdFirmwareVersionString)
           );
}

/**
  Measure FV image.
  Add it into the measured FV list after the FV is measured successfully.

  @param[in]  FvBase            Base address of FV image.
  @param[in]  FvLength          Length of FV image.

  @retval EFI_SUCCESS           Fv image is measured successfully
                                or it has been already measured.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to log the new event.
  @retval EFI_DEVICE_ERROR      The command was unsuccessful.

**/
EFI_STATUS
MeasureFvImage (
    IN EFI_PHYSICAL_ADDRESS       FvBase,
    IN UINT64                     FvLength
    )
{
    UINT32                        Index;
    EFI_STATUS                    Status = EFI_SUCCESS;
    EFI_PLATFORM_FIRMWARE_BLOB    FvBlob;
    TCG_PCR_EVENT_HDR             TcgEventHdr;

    //
    // Check if it is in Excluded FV list
    //
    if (mMeasurementExcludedFvPpi != NULL) {
        for (Index = 0; Index < mMeasurementExcludedFvPpi->Count; Index ++) {
          if (mMeasurementExcludedFvPpi->Fv[Index].FvBase == FvBase) {
            DEBUG ((DEBUG_INFO, "The FV which is excluded by TrEEPei starts at: 0x%x\n", FvBase));
            DEBUG ((DEBUG_INFO, "The FV which is excluded by TrEEPei has the size: 0x%x\n", FvLength));
            return EFI_SUCCESS;
          }
        }
    }

    //
    // Check whether FV is in the measured FV list.
    //
    for (Index = 0; Index < mMeasuredBaseFvIndex; Index ++) {
        if (mMeasuredBaseFvInfo[Index].BlobBase == FvBase) {
          return EFI_SUCCESS;
        }
    }

    //
    // Measure and record the FV to the TPM
    //
    FvBlob.BlobBase   = FvBase;
    FvBlob.BlobLength = FvLength;

    DEBUG ((DEBUG_INFO, "The FV which is measured by TrEEPei starts at: 0x%x\n", FvBlob.BlobBase));
    DEBUG ((DEBUG_INFO, "The FV which is measured by TrEEPei has the size: 0x%x\n", FvBlob.BlobLength));

    TcgEventHdr.PCRIndex = 0;
    TcgEventHdr.EventType = EV_EFI_PLATFORM_FIRMWARE_BLOB;
    TcgEventHdr.EventSize = sizeof (FvBlob);

    Status = HashLogExtendEvent (
             0,
             (UINT8*) (UINTN) FvBlob.BlobBase,
             (UINTN) FvBlob.BlobLength,
             &TcgEventHdr,
             (UINT8*) &FvBlob
             );
    ASSERT_EFI_ERROR (Status);

    //
    // Add new FV into the measured FV list.
    //
    if (mMeasuredBaseFvIndex < FixedPcdGet32 (PcdPeiCoreMaxFvSupported))
    {
        mMeasuredBaseFvInfo[mMeasuredBaseFvIndex].BlobBase   = FvBase;
        mMeasuredBaseFvInfo[mMeasuredBaseFvIndex].BlobLength = FvLength;
        mMeasuredBaseFvIndex++;
    }

    return Status;
}

/**
  Measure main BIOS.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to log the new event.
  @retval EFI_DEVICE_ERROR      The command was unsuccessful.

**/
EFI_STATUS
MeasureMainBios()
{
    EFI_STATUS                    status = EFI_SUCCESS;
    UINT32                        fvInstances;
    EFI_PEI_FV_HANDLE             volumeHandle;
    EFI_FV_INFO                   volumeInfo;
    EFI_PEI_FIRMWARE_VOLUME_PPI   *fvPpi;

    PERF_START_EX (mFileHandle, "EventRec", "TrEEPei", 0, PERF_ID_TREE_PEI);
    for (fvInstances = 0;; fvInstances++)
    {
        //
        // Traverse all firmware volume instances of Static Core Root of Trust for Measurement
        // (S-CRTM), this firmware volume measure policy can be modified/enhanced by special
        // platform for special CRTM TPM measuring.
        //
        status = PeiServicesFfsFindNextVolume (fvInstances, &volumeHandle);
        if (EFI_ERROR (status))
        {
          break;
        }

        //
        // Measure and record the firmware volume that is dispatched by PeiCore
        //
        status = PeiServicesFfsGetVolumeInfo (volumeHandle, &volumeInfo);
        ASSERT_EFI_ERROR (status);
        if (EFI_ERROR(status))
        {
            continue;
        }

        //
        // Locate the corresponding FV_PPI according to founded FV's format guid
        //
        status = PeiServicesLocatePpi (
                   &volumeInfo.FvFormat,
                   0,
                   NULL,
                   (VOID**)&fvPpi
                   );
        if (!EFI_ERROR (status))
        {
            MeasureFvImage ((EFI_PHYSICAL_ADDRESS) (UINTN) volumeInfo.FvStart, volumeInfo.FvSize);
        }
    }
    PERF_END_EX (mFileHandle, "EventRec", "TrEEPei", 0, PERF_ID_TREE_PEI + 1);

    return EFI_SUCCESS;
}

/**
  Measure and record the Firmware Volum Information once FvInfoPPI install.

  @param[in] PeiServices       An indirect pointer to the EFI_PEI_SERVICES table published by the PEI Foundation.
  @param[in] NotifyDescriptor  Address of the notification descriptor data structure.
  @param[in] Ppi               Address of the PPI that was installed.

  @retval EFI_SUCCESS          The FV Info is measured and recorded to TPM.
  @return Others               Fail to measure FV.

**/
EFI_STATUS
EFIAPI
FirmwareVolmeInfoPpiNotifyCallback (
  IN EFI_PEI_SERVICES               **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR      *NotifyDescriptor,
  IN VOID                           *Ppi
  )
{
  EFI_PEI_FIRMWARE_VOLUME_INFO_PPI  *Fv;
  EFI_STATUS                        Status;
  EFI_PEI_FIRMWARE_VOLUME_PPI       *FvPpi;

  Fv = (EFI_PEI_FIRMWARE_VOLUME_INFO_PPI *) Ppi;

  //
  // The PEI Core can not dispatch or load files from memory mapped FVs that do not support FvPpi.
  //
  Status = PeiServicesLocatePpi (
             &Fv->FvFormat,
             0,
             NULL,
             (VOID**)&FvPpi
             );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  //
  // This is an FV from an FFS file, and the parent FV must have already been measured,
  // No need to measure twice, so just record the FV and return
  //
  if (Fv->ParentFvName != NULL || Fv->ParentFileName != NULL ) {

    ASSERT (mMeasuredChildFvIndex < FixedPcdGet32 (PcdPeiCoreMaxFvSupported));
    if (mMeasuredChildFvIndex < FixedPcdGet32 (PcdPeiCoreMaxFvSupported)) {
      mMeasuredChildFvInfo[mMeasuredChildFvIndex].BlobBase   = (EFI_PHYSICAL_ADDRESS) (UINTN) Fv->FvInfo;
      mMeasuredChildFvInfo[mMeasuredChildFvIndex].BlobLength = Fv->FvInfoSize;
      mMeasuredChildFvIndex++;
    }
    return EFI_SUCCESS;
  }

  return MeasureFvImage ((EFI_PHYSICAL_ADDRESS) (UINTN) Fv->FvInfo, Fv->FvInfoSize);
}


EFI_STATUS
SetupTpmMemory (
    IN    EFI_PEI_SERVICES    **PeiServices
    )
/*++

    This routine allocates runtime memory and map Command-Response buffer on host.

Arguments:

    PeiServices -  Describes the list of possible PEI Services.

Return Value:

    EFI_STATUS

--*/
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_PHYSICAL_ADDRESS crBuffer = 0;

    status = (*PeiServices)->AllocatePages (PeiServices, EfiRuntimeServicesData, 2, &crBuffer);
    if (EFI_ERROR (status))
    {
        goto Cleanup;
    }

    if (crBuffer > 0xFFFFFFFFULL)
    {
        // PEI memory was published as - Base at 1MB, size max 64MB.
        // It is guaranteed that physical address is below 4 GB.
        status = EFI_DEVICE_ERROR;
        ASSERT(FALSE);
        goto Cleanup;
    }
    ZeroMem((UINT8*)crBuffer, 2 * EFI_PAGE_SIZE);

    //
    // Send the request to the TPM VDev.
    // Cast of command buffer GPA is safe as it was allocated below 4GB.
    //
    IoWrite32(TpmControlPort, TpmIoMapSharedMemory);
    IoWrite32(TpmDataPort, (UINT32)crBuffer);

    //
    // Query vDev the mapping result
    //
    IoWrite32(TpmControlPort, TpmIoEstablished);
    if (IoRead32(TpmDataPort) == 0)
    {
        //
        // Couldn't establish memory mapping with vDev.
        //
        status = EFI_NO_MAPPING;
        goto Cleanup;
    }

    Tpm2RegisterTpm2DeviceLib((TPM2_DEVICE_INTERFACE *)(UINTN)TPM_BASE_ADDRESS);

Cleanup:

    return status;
}


/**
  Do measurement after memory is ready.

  @param[in]      PeiServices   Describes the list of possible PEI Services.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to log the new event.
  @retval EFI_DEVICE_ERROR      The command was unsuccessful.

**/
EFI_STATUS
PeimEntryMP (
    IN    EFI_PEI_SERVICES    **PeiServices
    )
{
    EFI_STATUS  status = EFI_SUCCESS;

    status = SetupTpmMemory(PeiServices);
    if (EFI_ERROR (status))
    {
        goto Cleanup;
    }

    // Do not check status, because it is optional
    status = PeiServicesLocatePpi (
               &gEfiPeiFirmwareVolumeInfoMeasurementExcludedPpiGuid,
               0,
               NULL,
               (VOID**)&mMeasurementExcludedFvPpi
               );

    status = MeasureCRTMVersion();
    ASSERT_EFI_ERROR (status);

    status = MeasureMainBios();
    ASSERT_EFI_ERROR (status);

    if (mFirmwareDebuggerEnabled)
    {
        status = MeasureLaunchOfFirmwareDebugger();
        ASSERT_EFI_ERROR (status);
    }

    //
    // Post callbacks:
    // for the FvInfoPpi services to measure and record
    // the additional Fvs to TPM
    //
    status = PeiServicesNotifyPpi (&mNotifyList[0]);
    ASSERT_EFI_ERROR (status);

Cleanup:

    return status;
}

/**
  Entry point of this module.

  @param[in] FileHandle   Handle of the file being invoked.
  @param[in] PeiServices  Describes the list of possible PEI Services.

  @return Status.

**/
EFI_STATUS
EFIAPI
PeimEntryMA (
    IN       EFI_PEI_FILE_HANDLE      FileHandle,
    IN CONST EFI_PEI_SERVICES         **PeiServices
    )
{
    EFI_STATUS              status = EFI_SUCCESS;
    void *                  hob;
    BIOS_CONFIG_PAGE_V2 *   configPageV2 = NULL;
    BIOS_CONFIG_PAGE_V3 *   configPageV3 = NULL;
    EFI_BOOT_MODE           bootMode;
    BOOLEAN                 tpmEnabled = FALSE;

    //
    // Get the two relevant configuration settings.
    //
    hob = GetFirstGuidHob(&gMsvmConfigPageV2Guid);
    if (hob != NULL)
    {
        configPageV2 = (BIOS_CONFIG_PAGE_V2 *)GET_GUID_HOB_DATA(hob);
        mFirmwareDebuggerEnabled = configPageV2->Flags.DebuggerEnabled == 1 ? TRUE : FALSE;
        tpmEnabled = FALSE;
    }
    else
    {
        hob = GetFirstGuidHob(&gMsvmConfigPageV3Guid);
        if (hob != NULL)
        {
            configPageV3 = (BIOS_CONFIG_PAGE_V3 *)GET_GUID_HOB_DATA(hob);
            mFirmwareDebuggerEnabled = configPageV3->Flags.DebuggerEnabled == 1 ? TRUE : FALSE;
            tpmEnabled = configPageV3->Flags.TpmEnabled == 1 ? TRUE : FALSE;
        }
        else
        {
            ASSERT(FALSE);
            mFirmwareDebuggerEnabled = tpmEnabled = FALSE;
        }
    }

    //
    // Fail if TPM not enabled.
    //
    if (!tpmEnabled)
    {
        return EFI_UNSUPPORTED;
    }

    //
    // Hyper-V UEFI only implements BOOT_WITH_FULL_CONFIGURATION.
    // Assert that is the case.
    // Future work - S3_RESUME boot skips shadow logic. No Measurement is required.
    //
    status = PeiServicesGetBootMode (&bootMode);
    ASSERT_EFI_ERROR (status);
    ASSERT(bootMode == BOOT_WITH_FULL_CONFIGURATION);

    //
    // Shadow logic.
    // This service registers a file handle so that after memory is available,
    // the PEIM will be re-loaded into permanent memory and re-initialized.
    // The PEIM registered this way will always be initialized twice. The first
    // time it returns EFI_SUCCESS. The secondtime it returns EFI_ALREADY_STARTED.
    //
    status = (**PeiServices).RegisterForShadow(FileHandle);
    if (status == EFI_SUCCESS)
    {
        status = PeiServicesInstallPpi (&mTpmInitializedPpiList);
        ASSERT_EFI_ERROR (status);
        goto Cleanup;
    }

    if (status == EFI_ALREADY_STARTED)
    {
        mFileHandle = FileHandle;
        status = PeimEntryMP ((EFI_PEI_SERVICES**)PeiServices);
        goto Cleanup;
    }

    ASSERT_EFI_ERROR (status);

Cleanup:

    return status;
}
