## @file
#  EFI/Framework Microsoft Virtual Machine Firmware (MSVM) platform
#
#  Copyright (C) Microsoft. All rights reserved.
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = Msvm
  PLATFORM_GUID                  = 60d3fbae-b4ed-4a10-9145-f8185dd8b1bd
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/MsvmX64
  SUPPORTED_ARCHITECTURES        = X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = MsvmPkg/MsvmPkgX64.fdf

################################################################################
#
# BuildOptions Section - extra build flags
#
################################################################################
[BuildOptions]

################################################################################
#
# SKU Identification section - list of all SKU IDs supported by this Platform.
#
################################################################################
[SkuIds]
  0|DEFAULT

################################################################################
#
# Library Class section
#
# Library class names used by this platform and the implementations of those
# libraries.  <name>|<inf location>
#
################################################################################

#
# Library instances to use by default for all modules and phases unless overridden below
#
[LibraryClasses]
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLibRepStr/BaseMemoryLibRepStr.inf
  CacheMaintenanceLib|MdePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf
  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
  CpuExceptionHandlerLib|MdeModulePkg/Library/CpuExceptionHandlerLibNull/CpuExceptionHandlerLibNull.inf
  CrashDumpAgentLib|MdeModulePkg/Library/CrashDumpAgentLibNull/CrashDumpAgentLibNull.inf
!ifdef DEBUGLIB_SERIAL
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
!else
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
!endif
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  LocalApicLib|UefiCpuPkg/Library/BaseXApicLib/BaseXApicLib.inf
  MtrrLib|UefiCpuPkg/Library/MtrrLib/MtrrLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PCUartLib|MsvmPkg/Library/PCUart/PCUart.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
!ifdef DEBUGLIB_SERIAL
  SerialPortLib|PcAtChipsetPkg\Library\SerialIoLib\SerialIoLib.inf
!else
  SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
!endif
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  TimerLib|MsvmPkg/Library/HvTimerLib/HvTimerLib.inf
  Tpm2CommandLib|SecurityPkg/Library/Tpm2CommandLib/Tpm2CommandLib.inf
  UefiCpuLib|UefiCpuPkg/Library/BaseUefiCpuLib/BaseUefiCpuLib.inf
  UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf

#
# Library instance overrides for SEC and PEI
#
[LibraryClasses.common.SEC, LibraryClasses.common.PEI_CORE, LibraryClasses.common.PEIM]
  BiosDeviceLib|MsvmPkg/Library/BiosDeviceLib/BiosDevicePeiLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/BaseExtractGuidedSectionLib/BaseExtractGuidedSectionLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLibIdt/PeiServicesTablePointerLibIdt.inf

#
# Library instance overrides just for SEC
#
[LibraryClasses.common.SEC]

#
# Library instance overrides for PEI
#
[LibraryClasses.common.PEI_CORE, LibraryClasses.common.PEIM]
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  ReportStatusCodeLib|MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  PcdLib|MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
  WatchdogTimerLib|MsvmPkg/Library/WatchdogTimerLib/WatchdogTimerLib.inf

#
# Library instance overrides just for PEI CORE
#
[LibraryClasses.common.PEI_CORE]
  PeiCoreEntryPoint|MdePkg/Library/PeiCoreEntryPoint/PeiCoreEntryPoint.inf

#
# Library instance overrides just for PEIMs
#
[LibraryClasses.common.PEIM]
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  PeiResourcePublicationLib|MdePkg/Library/PeiResourcePublicationLib/PeiResourcePublicationLib.inf

#
# Library instance overrides for DXE
#
[LibraryClasses.common.DXE_CORE, LibraryClasses.common.DXE_DRIVER, LibraryClasses.common.DXE_RUNTIME_DRIVER, LibraryClasses.common.UEFI_DRIVER, LibraryClasses.common.UEFI_APPLICATION]
  BiosDeviceLib|MsvmPkg/Library/BiosDeviceLib/BiosDeviceBaseLib.inf
  BootEventLogLib|MsvmPkg/Library/BootEventLogLib/BootEventLogLib.inf
  ConfigLib|MsvmPkg/Library/ConfigLib/ConfigLib.inf
  DebugAgentLib|MsvmPkg/Library/BdLib/DxeBdLib.inf
  DebugLib|MsvmPkg/Library/BdDebugLib/BdDebugLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  DpcLib|MdeModulePkg/Library/DxeDpcLib/DxeDpcLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  EmclLib|MsvmPkg/Library/EmclLib/EmclLib.inf
  EventLogLib|MsvmPkg/Library/EventLogLib/EventLogLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  IpIoLib|MdeModulePkg/Library/DxeIpIoLib/DxeIpIoLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  NetLib|MdeModulePkg/Library/DxeNetLib/DxeNetLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  ResetSystemLib|MsvmPkg/Library/ResetSystemLib/ResetSystemLib.inf
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiScsiLib|MdePkg/Library/UefiScsiLib/UefiScsiLib.inf
  UdpIoLib|MdeModulePkg/Library/DxeUdpIoLib/DxeUdpIoLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  WatchdogTimerLib|MsvmPkg/Library/WatchdogTimerLib/WatchdogTimerLib.inf

#
# Library instances overrides for just DXE CORE
#
[LibraryClasses.common.DXE_CORE]
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  HobLib|MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  MemoryAllocationLib|MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
  PeCoffExtraActionLib|MsvmPkg/Library/BdLib/DxeBdLib.inf

#
# Library instance overrides for all DXE Drivers
#
[LibraryClasses.common.DXE_DRIVER, LibraryClasses.common.UEFI_DRIVER, LibraryClasses.common.DXE_RUNTIME_DRIVER]
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf

#
# Library instance overrides for just DXE Runtime Drivers
#
[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf

[PcdsFixedAtBuild.common]
  # Synthetic Timer Config
  gMsvmPkgTokenSpaceGuid.PcdSynicTimerSintIndex|0x1
  gMsvmPkgTokenSpaceGuid.PcdSynicTimerTimerIndex|0x0
  gMsvmPkgTokenSpaceGuid.PcdSynicTimerVector|0x40
  gMsvmPkgTokenSpaceGuid.PcdSynicTimerDefaultPeriod|100000

  # Vmbus Config
  gMsvmPkgTokenSpaceGuid.PcdVmbusSintIndex|0x2
  gMsvmPkgTokenSpaceGuid.PcdVmbusSintVector|0x41
  gMsvmPkgTokenSpaceGuid.PcdVmbusVector|0x5

  # BIOS Device
  gMsvmPkgTokenSpaceGuid.PcdBiosBaseAddress|0x28

  # Battery Device
  gMsvmPkgTokenSpaceGuid.PcdBatteryBase|0xFED3F000

  # UART Devices
  gMsvmPkgTokenSpaceGuid.PcdCom1RegisterBase|0x3F8
  gMsvmPkgTokenSpaceGuid.PcdCom1Vector|4
  gMsvmPkgTokenSpaceGuid.PcdCom2RegisterBase|0x2F8
  gMsvmPkgTokenSpaceGuid.PcdCom2Vector|3

  # RTC (clock)
  gMsvmPkgTokenSpaceGuid.PcdRtcRegisterBase|0x70
  gMsvmPkgTokenSpaceGuid.PcdRtcVector|8

  # PMEM (NVDIMM)
  gMsvmPkgTokenSpaceGuid.PcdPmemRegisterBase|0xFED3D000

  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareRevision|0x00100032
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor|L"Microsoft"

  #
  # The runtime state of these two Debug PCDs can be modified in the debugger by
  # modifyting EfiBdDebugPrintGlobalMask and EfiBdDebugPrintComponentMask.
  #
!ifdef DEBUG_NOISY
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80400042
!else
  # This default turns on errors and warnings
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000002
!endif
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xFF

  #
  # See REPORT_STATUS_CODE_PROPERTY_nnnnn in ReportStatusCodeLib.h
  #
  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x07

  # Prevent reboots due to some memory variables being out of sync, seems
  # to only be relevant when supporting S4 (hibernate)
  # FUTURE: figure out what this is all about -- jostarks
  gEfiMdeModulePkgTokenSpaceGuid.PcdResetOnMemoryTypeInformationChange|FALSE

  # We are only supporting SMBIOS v3.1
  gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosVersion|0x0301

  # Default OEM ID for ACPI table creation, its length must be 0x6 bytes to follow ACPI specification.
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemId|"VRTUAL"

  # Default OEM Table ID for ACPI table creation, "MICROSFT", as a SIGNATURE_64 value.
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemTableId|0x5446534F5243494D

  # Default OEM Revision for ACPI table creation.
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00000001

  # Default Creator ID for ACPI table creation, "MSFT", as a SIGNATURE_32 value.
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultCreatorId|0x5446534D

  # Default Creator Revision for ACPI table creation.
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultCreatorRevision|0x00000001

  # Default settings for serial ports
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultBaudRate|115200
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultDataBits|8
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultParity|1
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultStopBits|1

  # Default setting for serial console terminal type is UTF8
  gEfiMdePkgTokenSpaceGuid.PcdDefaultTerminalType|3

  # Override defaults to indicate only US english support
  gEfiMdePkgTokenSpaceGuid.PcdUefiVariableDefaultLangCodes|"engeng"
  gEfiMdePkgTokenSpaceGuid.PcdUefiVariableDefaultPlatformLangCodes|"en;en-US"

  # Configure max supported number of Logical Processorss
  gUefiCpuPkgTokenSpaceGuid.PcdCpuMaxLogicalProcessorNumber|0x00000001

  # Publish UEFI PropertiesTable.
  gEfiMdeModulePkgTokenSpaceGuid.PcdPropertiesTableEnable|TRUE

  # Base addresses of memory mapped devices in MMIO space.
  gUefiCpuPkgTokenSpaceGuid.PcdCpuLocalApicBaseAddress|0xFEE00000
  gEfiSecurityPkgTokenSpaceGuid.PcdTpmBaseAddress|0xFED40000
  gPcAtChipsetPkgTokenSpaceGuid.PcdIoApicBaseAddress|0xFEC00000

  # Use 1GB page table entries in DXE page table when possible
  gEfiMdeModulePkgTokenSpaceGuid.PcdUse1GPageTable|TRUE

[PcdsFeatureFlag.common]
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdBootlogoOnlyEnable|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdDxeIplBuildPageTables|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseMemory|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseSerial|FALSE

[PcdsDynamicDefault]
  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOut|0xFFFF

  # UEFI Config information from the Bios VDEV
  # UEFI_CONFIG_STRUCTURE_COUNT
  gMsvmPkgTokenSpaceGuid.PcdConfigBlobSize|0x0
  # UEFI_CONFIG_BIOS_INFORMATION
  gMsvmPkgTokenSpaceGuid.PcdBiosVDevVersion|0x0

  # UEFI_CONFIG_SRAT
  gMsvmPkgTokenSpaceGuid.PcdSratPtr|0x0
  gMsvmPkgTokenSpaceGuid.PcdSratSize|0x0

  # UEFI_CONFIG_MEMORY_MAP
  gMsvmPkgTokenSpaceGuid.PcdMemoryMapPtr|0x0
  gMsvmPkgTokenSpaceGuid.PcdMemoryMapSize|0x0

  # UEFI_CONFIG_ENTROPY
  # Points to the actual entropy array, not the containing config structure
  gMsvmPkgTokenSpaceGuid.PcdEntropyPtr|0x0

  # UEFI_CONFIG_BIOS_GUID
  # Points to the actual GUID, not the containing config structure
  gMsvmPkgTokenSpaceGuid.PcdBiosGuidPtr|0x0

  # UEFI_CONFIG_SMBIOS_SYSTEM_SERIAL_NUMBER
  # Points to a null terminated string of the specified size
  gMsvmPkgTokenSpaceGuid.PcdSmbiosSystemSerialNumberStr|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosSystemSerialNumberSize|0x0

  # UEFI_CONFIG_SMBIOS_BASE_SERIAL_NUMBER
  gMsvmPkgTokenSpaceGuid.PcdSmbiosBaseSerialNumberStr|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosBaseSerialNumberSize|0x0

  # UEFI_CONFIG_SMBIOS_CHASSIS_SERIAL_NUMBER
  gMsvmPkgTokenSpaceGuid.PcdSmbiosChassisSerialNumberStr|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosChassisSerialNumberSize|0x0

  # UEFI_CONFIG_SMBIOS_CHASSIS_ASSET_TAG
  gMsvmPkgTokenSpaceGuid.PcdSmbiosChassisAssetTagStr|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosChassisAssetTagSize|0x0

  # UEFI_CONFIG_SMBIOS_BIOS_LOCK_STRING
  gMsvmPkgTokenSpaceGuid.PcdSmbiosBiosLockStringStr|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosBiosLockStringSize|0x0

  # UEFI_CONFIG_SMBIOS_3_1_PROCESSOR_INFORMATION
  # Defaults are set to Unknown unless otherwise noted
  # Processor Type defaults to Central Processor type (CPU)
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorType|0x3
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorID|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorVoltage|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorExternalClock|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorMaxSpeed|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorCurrentSpeed|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorStatus|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorUpgrade|0x1
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorCharacteristics|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorFamily2|0x2

  # UEFI_CONFIG_SMBIOS_SOCKET_DESIGNATION
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorSocketDesignationStr|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorSocketDesignationSize|0x0

  # UEFI_CONFIG_SMBIOS_PROCESSOR_MANUFACTURER
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorManufacturerStr|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorManufacturerSize|0x0

  # UEFI_CONFIG_SMBIOS_PROCESSOR_VERSION
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorVersionStr|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorVersionSize|0x0

  # UEFI_CONFIG_SMBIOS_PROCESSOR_SERIAL_NUMBER
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorSerialNumberStr|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorSerialNumberSize|0x0

  # UEFI_CONFIG_SMBIOS_PROCESSOR_ASSET_TAG
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorAssetTagStr|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorAssetTagSize|0x0

  # UEFI_CONFIG_SMBIOS_PROCESSOR_PART_NUMBER
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorPartNumberStr|0x0
  gMsvmPkgTokenSpaceGuid.PcdSmbiosProcessorPartNumberSize|0x0

  # UEFI_CONFIG_FLAGS
  gMsvmPkgTokenSpaceGuid.PcdSerialControllersEnabled|FALSE
  gMsvmPkgTokenSpaceGuid.PcdPauseAfterBootFailure|FALSE
  gMsvmPkgTokenSpaceGuid.PcdPxeIpV6|FALSE
  gMsvmPkgTokenSpaceGuid.PcdDebuggerEnabled|FALSE
  gMsvmPkgTokenSpaceGuid.PcdLoadOempTable|FALSE
  gMsvmPkgTokenSpaceGuid.PcdTpmEnabled|FALSE
  gMsvmPkgTokenSpaceGuid.PcdHibernateEnabled|FALSE
  gMsvmPkgTokenSpaceGuid.PcdConsoleMode|0x0
  gMsvmPkgTokenSpaceGuid.PcdMemoryAttributesTableEnabled|FALSE
  gMsvmPkgTokenSpaceGuid.PcdVirtualBatteryEnabled|FALSE
  gMsvmPkgTokenSpaceGuid.PcdSgxMemoryEnabled|FALSE

  # UEFI_CONFIG_PROCESSOR_INFORMATION
  gMsvmPkgTokenSpaceGuid.PcdProcessorCount|0x0
  gMsvmPkgTokenSpaceGuid.PcdProcessorsPerVirtualSocket|0x0

  # UEFI_CONFIG_MMIO_DESCRIPTION
  # Currently only two mmio holes, low gap and high gap but we could
  # do more in the future.
  gMsvmPkgTokenSpaceGuid.PcdLowMmioGapBasePageNumber|0x0
  gMsvmPkgTokenSpaceGuid.PcdLowMmioGapSizeInPages|0x0
  gMsvmPkgTokenSpaceGuid.PcdHighMmioGapBasePageNumber|0x0
  gMsvmPkgTokenSpaceGuid.PcdHighMmioGapSizeInPages|0x0

################################################################################
#
# Components Section - list of all Modules include for this Platform.
#
################################################################################

[Components]
  #
  # SEC Phase modules
  #
  MsvmPkg/Sec/SecMain.inf

  #
  # PEI Phase modules
  #
  IntelFrameworkModulePkg/Universal/StatusCode/Pei/StatusCodePei.inf
  MdeModulePkg/Core/DxeIplPeim/DxeIpl.inf
  MdeModulePkg/Core/Pei/PeiMain.inf
  MdeModulePkg/Universal/PCD/Pei/Pcd.inf
  MsvmPkg/PlatformPei/PlatformPei.inf

  #
  # DXE Phase modules
  #
  MdeModulePkg/Core/Dxe/DxeMain.inf
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf {
  <LibraryClasses>
    SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
    NULL|SecurityPkg/Library/DxeImageVerificationLib/DxeImageVerificationLib.inf
    NULL|SecurityPkg/Library/DxeTpm2MeasureBootLib/DxeTpm2MeasureBootLib.inf
  }
  MsvmPkg/CpuDxe/CpuDxe.inf
  MdeModulePkg/Universal/Metronome/Metronome.inf
  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  IntelFrameworkModulePkg/Universal/BdsDxe/BdsDxe.inf {
    <LibraryClasses>
      CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
      GenericBdsLib|IntelFrameworkModulePkg/Library/GenericBdsLib/GenericBdsLib.inf
      HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
      PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
      PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
      PlatformBdsLib|MsvmPkg/Library/PlatformBdsLib/PlatformBdsLib.inf
      UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
    <PcdsPatchableInModule>
      gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1024
      gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|768
  }
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf {
    <LibraryClasses>
      CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
      LockBoxLib|MdeModulePkg/Library/LockBoxNullLib/LockBoxNullLib.inf
  }
  MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf
  MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf
  PcAtChipsetPkg/PcatRealTimeClockRuntimeDxe/PcatRealTimeClockRuntimeDxe.inf
  MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf
  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  FatPkg/EnhancedFatDxe/Fat.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf {
    <LibraryClasses>
      UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
      HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
    <PcdsPatchableInModule>
      gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1024
      gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|768
      gEfiMdeModulePkgTokenSpaceGuid.PcdSetGraphicsConsoleModeOnStart|FALSE
  }
  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
  MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf
  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf

  # Networking components

  MdeModulePkg/Universal/Network/ArpDxe/ArpDxe.inf
  MdeModulePkg/Universal/Network/Dhcp4Dxe/Dhcp4Dxe.inf
  MdeModulePkg/Universal/Network/DpcDxe/DpcDxe.inf
  MdeModulePkg/Universal/Network/Ip4Dxe/Ip4Dxe.inf
  MdeModulePkg/Universal/Network/MnpDxe/MnpDxe.inf
  MdeModulePkg/Universal/Network/Mtftp4Dxe/Mtftp4Dxe.inf
  MdeModulePkg/Universal/Network/Udp4Dxe/Udp4Dxe.inf
  NetworkPkg/Dhcp6Dxe/Dhcp6Dxe.inf
  NetworkPkg/Ip6Dxe/Ip6Dxe.inf
  NetworkPkg/Mtftp6Dxe/Mtftp6Dxe.inf
  NetworkPkg/TcpDxe/TcpDxe.inf
  NetworkPkg/Udp6Dxe/Udp6Dxe.inf
  NetworkPkg/UefiPxeBcDxe/UefiPxeBcDxe.inf

  # Hyper-V platform specific components

  MsvmPkg/AcpiPlatformDxe/AcpiPlatformDxe.inf
  MsvmPkg/AcpiTables/AcpiTables.inf
  MsvmPkg/EfiHvDxe/EfiHvDxe.inf
  MsvmPkg/EmclDxe/EmclDxe.inf
  MsvmPkg/EventLogDxe/EventLogDxe.inf
  MsvmPkg/ExdiSupportDxe/ExdiSupportDxe.inf
  MsvmPkg/NetvscDxe/NetvscDxe.inf
  MsvmPkg/SbCryptDxe/SbCryptDxe.inf
  MsvmPkg/SmbiosPlatformDxe/SmbiosPlatformDxe.inf
  MsvmPkg/StorvscDxe/StorvscDxe.inf
  MsvmPkg/SynicTimerDxe/SynicTimerDxe.inf
  MsvmPkg/SynthKeyDxe/SynthKeyDxe.inf
  MsvmPkg/VariableDxe/VariableDxe.inf
  MsvmPkg/VideoDxe/VideoDxe.inf
  MsvmPkg/VmbusDxe/VmbusDxe.inf
  MsvmPkg/WatchdogTimerDxe/WatchdogTimerDxe.inf
  MsvmPkg/SerialDxe/SerialDxe.inf
  MsvmPkg/VmbfsDxe/VmbfsDxe.inf
  MsvmPkg/ConNullDxe/ConNullDxe.inf

  # TPM related components

  SecurityPkg/Tcg/MemoryOverwriteControl/TcgMor.inf
  SecurityPkg/Tcg/Tcg2Dxe/Tcg2Dxe.inf {
    <LibraryClasses>
      HashLibTpm2|SecurityPkg/Library/HashLibTpm2/HashLibTpm2.inf
      Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibTrEE/Tpm2DeviceLibTrEE.inf
  }
  SecurityPkg/Tcg/Tcg2Pei/Tcg2Pei.inf {
    <LibraryClasses>
      HashLibTpm2|SecurityPkg/Library/HashLibTpm2/HashLibTpm2.inf
      Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibTrEE/Tpm2DeviceLibTrEE.inf
  }

  !ifdef BUILD_HAVOC
  MsvmPkg/Havoc/Havoc.inf {
    <LibraryClasses>
      ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
      ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
      FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
      SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
      UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  }
  !endif

