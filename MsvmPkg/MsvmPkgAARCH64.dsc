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
  OUTPUT_DIRECTORY               = Build/MsvmAARCH64
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = MsvmPkg/MsvmPkgAARCH64.fdf

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
  ArmDisassemblerLib|ArmPkg/Library/ArmDisassemblerLib/ArmDisassemblerLib.inf
  ArmGicArchLib|ArmPkg/Library/ArmGicArchLib/ArmGicArchLib.inf
  ArmGicLib|ArmPkg/Drivers/ArmGic/ArmGicLib.inf
  ArmLib|ArmPkg/Library/ArmLib/ArmBaseLib.inf
  ArmMmuLib|ArmPkg/Library/ArmMmuLib/ArmMmuBaseLib.inf
  ArmSmcLib|ArmPkg/Library/ArmSmcLib/ArmSmcLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  CacheMaintenanceLib|ArmPkg/Library/ArmCacheMaintenanceLib/ArmCacheMaintenanceLib.inf
  CpuExceptionHandlerLib|ArmPkg/Library/ArmExceptionLib/ArmExceptionLib.inf
  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
  CrashDumpAgentLib|MdeModulePkg/Library/CrashDumpAgentLibNull/CrashDumpAgentLibNull.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  DefaultExceptionHandlerLib|ArmPkg/Library/DefaultExceptionHandlerLib/DefaultExceptionHandlerLibBase.inf
  EfiResetSystemLib|ArmPkg/Library/ArmPsciResetSystemLib/ArmPsciResetSystemLib.inf
  EmclLib|MsvmPkg/Library/EmclLib/EmclLib.inf
  FdtLib|EmbeddedPkg/Library/FdtLib/FdtLib.inf
  HvHypercallLib|MsvmPkg/Library/HvHypercallLib/HvHypercallLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  MemoryInitPeiLib|ArmVirtPkg/Library/ArmVirtMemoryInitPeiLib/ArmVirtMemoryInitPeiLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PL011UartLib|ArmPlatformPkg/Drivers/PL011Uart/PL011Uart.inf
  PlatformPeiLib|ArmPlatformPkg/PlatformPei/PlatformPeiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  Tpm2CommandLib|SecurityPkg/Library/Tpm2CommandLib/Tpm2CommandLib.inf
  TimerLib|MsvmPkg/Library/HvTimerLib/HvTimerLib.inf
  UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  UefiScsiLib|MdePkg/Library/UefiScsiLib/UefiScsiLib.inf

!ifdef DEBUGLIB_SERIAL
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  SerialPortLib|ArmPlatformPkg/Library/PL011SerialPortLib/PL011SerialPortLib.inf
!else
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
!endif

#
# Library instance overrides for SEC and PEI
#
[LibraryClasses.common.SEC, LibraryClasses.common.PEI_CORE, LibraryClasses.common.PEIM]
  ArmMmuLib|ArmPkg/Library/ArmMmuLib/ArmMmuPeiLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/BaseExtractGuidedSectionLib/BaseExtractGuidedSectionLib.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  PeiServicesTablePointerLib|ArmPkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf

#
# Library instance overrides just for SEC
#
[LibraryClasses.common.SEC]

#
# Library instance overrides for PEI
#
[LibraryClasses.common.PEI_CORE, LibraryClasses.common.PEIM]
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
  #DebugAgentLib|MsvmPkg/Library/BdLib/DxeBdLib.inf
  #DebugLib|MsvmPkg/Library/BdDebugLib/BdDebugLib.inf
  #PeCoffExtraActionLib|MsvmPkg/Library/BdLib/DxeBdLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  DpcLib|MdeModulePkg/Library/DxeDpcLib/DxeDpcLib.inf
  DefaultExceptionHandlerLib|ArmPkg/Library/DefaultExceptionHandlerLib/DefaultExceptionHandlerLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  EventLogLib|MsvmPkg/Library/EventLogLib/EventLogLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  IpIoLib|MdeModulePkg/Library/DxeIpIoLib/DxeIpIoLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  NetLib|MdeModulePkg/Library/DxeNetLib/DxeNetLib.inf
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  UdpIoLib|MdeModulePkg/Library/DxeUdpIoLib/DxeUdpIoLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  WatchdogTimerLib|MsvmPkg/Library/WatchdogTimerLib/WatchdogTimerLib.inf

#
# Library instances overrides for just DXE CORE
#
[LibraryClasses.common.DXE_CORE]
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  HobLib|MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  MemoryAllocationLib|MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf

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
  # runtime drivers shouldn't use UEFI debugging, especially after ExitBootServices()
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf

[PcdsFixedAtBuild.common]
  # GIC related config
  gArmTokenSpaceGuid.PcdGicDistributorBase|0xFFFF0000        # aka GICD
  gArmTokenSpaceGuid.PcdGicInterruptInterfaceBase|0xFFFEE000 # aka GICC
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0xEFFEE000     # aka GICR

  # Synthetic Timer Config
  gMsvmPkgTokenSpaceGuid.PcdSynicTimerSintIndex|0x1
  gMsvmPkgTokenSpaceGuid.PcdSynicTimerTimerIndex|0x0
  gMsvmPkgTokenSpaceGuid.PcdSynicTimerVector|17             # use PPI for SINTs
  gMsvmPkgTokenSpaceGuid.PcdSynicTimerDefaultPeriod|100000

  # Vmbus Config
  gMsvmPkgTokenSpaceGuid.PcdVmbusSintVector|18              # use PPI for SINTs
  gMsvmPkgTokenSpaceGuid.PcdVmbusSintIndex|0x2
  gMsvmPkgTokenSpaceGuid.PcdVmbusVector|32                  # SPI

  # BIOS Device
  gMsvmPkgTokenSpaceGuid.PcdBiosBaseAddress|0xEFFED000

  # Generation Counter Device
  gMsvmPkgTokenSpaceGuid.PcdGenCountEventVector|35          # SPI

  # Battery Device
  gMsvmPkgTokenSpaceGuid.PcdBatteryBase|0xEFFEA000
  gMsvmPkgTokenSpaceGuid.PcdBatteryEventVector|36           # SPI

  # UART Devices
  gMsvmPkgTokenSpaceGuid.PcdCom1RegisterBase|0xEFFEC000
  gMsvmPkgTokenSpaceGuid.PcdCom1Vector|33                   # SPI
  gMsvmPkgTokenSpaceGuid.PcdCom2RegisterBase|0xEFFEB000
  gMsvmPkgTokenSpaceGuid.PcdCom2Vector|34                   # SPI

  # RTC (clock)
  gMsvmPkgTokenSpaceGuid.PcdRtcRegisterBase|0x70
  gMsvmPkgTokenSpaceGuid.PcdRtcVector|8

  # PMEM (NVDIMM)
  gMsvmPkgTokenSpaceGuid.PcdPmemRegisterBase|0xEFFE9000
  gMsvmPkgTokenSpaceGuid.PcdPmemEventVector|37              # SPI

  #
  # Static initial memory config - presumes minimum 64MB in VM
  # Page table, stack, and heap are hard-coded in host worker process.
  #
  # Firmware:            0x00000000 to 0x00400000 4MB (Pcds from FDF file)
  # PageTable:           0x00400000 to 0x00404000 4KB (starts on 2MB boundary)
  # Stack and Heap:      0x00404000 to 0x00414000 64KB
  # System Memory (PEI): 0x00414000 to 0x04000000 ~59MB
  #
  gMsvmPkgTokenSpaceGuid.PcdSystemMemoryBaseAddress|0x00414000
  gMsvmPkgTokenSpaceGuid.PcdSystemMemorySize|0x03BEC000

  #
  # The runtime state of these two Debug PCDs can be modified in the debugger by
  # modifyting EfiKdDebugPrintGlobalMask and EfiKdDebugPrintComponentMask.
  #
!ifdef DEBUG_NOISY
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x804FEF4B
!else
  # This default turns on errors and warnings
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000002
!endif
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x17

  #
  # See REPORT_STATUS_CODE_PROPERTY_nnnnn in ReportStatusCodeLib.h
  #
  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x07

  # Prevent reboots due to some memory variables being out of sync, seems
  # to only be relevant when supporting S4 (hibernate)
  # FUTURE: figure out what this is all about -- jostarks
  gEfiMdeModulePkgTokenSpaceGuid.PcdResetOnMemoryTypeInformationChange|FALSE

  # Support SMBIOS 3.1
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

  # Default settings for serial port in SerialPortLib (for debug print)
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialRegisterBase|0xEFFEB000 #COM2
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
  #gUefiCpuPkgTokenSpaceGuid.PcdCpuMaxLogicalProcessorNumber|0x00000001

  # Publish UEFI PropertiesTable.
  gEfiMdeModulePkgTokenSpaceGuid.PcdPropertiesTableEnable|TRUE

[PcdsFeatureFlag.common]
  #gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdBootlogoOnlyEnable|TRUE
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
  gMsvmPkgTokenSpaceGuid.PcdIsVmbfsBoot|FALSE

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

  # UEFI_CONFIG_AARCH64_MPIDR
  gMsvmPkgTokenSpaceGuid.PcdProcessorMPIDRValuesPtr|0x0

  # Isolation
  gMsvmPkgTokenSpaceGuid.PcdSystemIsolated|FALSE

  # UEFI_CONFIG_ACPI_TABLE
  gMsvmPkgTokenSpaceGuid.PcdAcpiTablePtr|0x0
  gMsvmPkgTokenSpaceGuid.PcdAcpiTableSize|0x0


################################################################################
#
# Components Section - list of all Modules include for this Platform.
#
################################################################################

[Components]

  #
  # PEI Phase modules
  #
  MsvmPkg/Sec/SecMain.inf
  MdeModulePkg/Core/DxeIplPeim/DxeIpl.inf
  MdeModulePkg/Core/Pei/PeiMain.inf
  MdeModulePkg/Universal/PCD/Pei/Pcd.inf
  MsvmPkg/PlatformPei/PlatformPei.inf

  SecurityPkg/Tcg/Tcg2Pei/Tcg2Pei.inf {
    <LibraryClasses>
      HashLibTpm2|SecurityPkg/Library/HashLibTpm2/HashLibTpm2.inf
      Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibTrEE/Tpm2DeviceLibTrEE.inf
  }

  #
  # DXE Phase modules
  #
  ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  ArmPkg/Drivers/ArmGic/ArmGicDxe.inf

  EmbeddedPkg/ResetRuntimeDxe/ResetRuntimeDxe.inf

  FatPkg/EnhancedFatDxe/Fat.inf

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

  MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf
  MdeModulePkg/Core/Dxe/DxeMain.inf
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
  MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf {
    <LibraryClasses>
      CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
      LockBoxLib|MdeModulePkg/Library/LockBoxNullLib/LockBoxNullLib.inf
  }
  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf {
    <LibraryClasses>
      UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
      HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
    <PcdsPatchableInModule>
      gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1024
      gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|768
      gEfiMdeModulePkgTokenSpaceGuid.PcdSetGraphicsConsoleModeOnStart|FALSE
  }
  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  MdeModulePkg/Universal/Metronome/Metronome.inf
  MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf
  MdeModulePkg/Universal/Network/ArpDxe/ArpDxe.inf
  MdeModulePkg/Universal/Network/Dhcp4Dxe/Dhcp4Dxe.inf
  MdeModulePkg/Universal/Network/DpcDxe/DpcDxe.inf
  MdeModulePkg/Universal/Network/Ip4Dxe/Ip4Dxe.inf
  MdeModulePkg/Universal/Network/MnpDxe/MnpDxe.inf
  MdeModulePkg/Universal/Network/Mtftp4Dxe/Mtftp4Dxe.inf
  MdeModulePkg/Universal/Network/Udp4Dxe/Udp4Dxe.inf
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf {
  <LibraryClasses>
    SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
    NULL|SecurityPkg/Library/DxeImageVerificationLib/DxeImageVerificationLib.inf
    NULL|SecurityPkg/Library/DxeTpm2MeasureBootLib/DxeTpm2MeasureBootLib.inf
  }
  MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf

  MsvmPkg/AcpiPlatformDxe/AcpiPlatformDxe.inf
  MsvmPkg/AcpiTables/AcpiTables.inf
  MsvmPkg/ConNullDxe/ConNullDxe.inf
  MsvmPkg/EfiHvDxe/EfiHvDxe.inf
  MsvmPkg/EmclDxe/EmclDxe.inf
  MsvmPkg/EventLogDxe/EventLogDxe.inf
  MsvmPkg/NetvscDxe/NetvscDxe.inf
  MsvmPkg/SbCryptDxe/SbCryptDxe.inf
  MsvmPkg/SerialDxe/SerialDxe.inf
  MsvmPkg/SmbiosPlatformDxe/SmbiosPlatformDxe.inf
  MsvmPkg/StorvscDxe/StorvscDxe.inf
  MsvmPkg/SynthKeyDxe/SynthKeyDxe.inf
  MsvmPkg/SynicTimerDxe/SynicTimerDxe.inf
  MsvmPkg/TimeRuntimeDxe/TimeRuntimeDxe.inf
  MsvmPkg/VariableDxe/VariableDxe.inf
  MsvmPkg/VideoDxe/VideoDxe.inf
  MsvmPkg/VmbfsDxe/VmbfsDxe.inf
  MsvmPkg/VmbusDxe/VmbusDxe.inf
  MsvmPkg/WatchdogTimerDxe/WatchdogTimerDxe.inf

  NetworkPkg/Dhcp6Dxe/Dhcp6Dxe.inf
  NetworkPkg/Ip6Dxe/Ip6Dxe.inf
  NetworkPkg/Mtftp6Dxe/Mtftp6Dxe.inf
  NetworkPkg/TcpDxe/TcpDxe.inf
  NetworkPkg/Udp6Dxe/Udp6Dxe.inf
  NetworkPkg/UefiPxeBcDxe/UefiPxeBcDxe.inf

  SecurityPkg/Tcg/Tcg2Dxe/Tcg2Dxe.inf {
    <LibraryClasses>
      HashLibTpm2|SecurityPkg/Library/HashLibTpm2/HashLibTpm2.inf
      Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibTrEE/Tpm2DeviceLibTrEE.inf
  }
  SecurityPkg/Tcg/MemoryOverwriteControl/TcgMor.inf
