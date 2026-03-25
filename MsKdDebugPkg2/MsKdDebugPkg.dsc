## @file MsKdDebugPkg.dsc
#
# This Package provides all definitions and library instances, which are
# defined and created by the Microsoft debugger 2 package.
#
# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME                  = MsKdDebugPkg2
  PLATFORM_GUID                  = CC89F5DC-2D94-4C06-B4C2-0ABD6F5224E7
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/MsKdDebugPkg2
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  SourceDebugEnabledLib|SourceLevelDebugPkg/Library/SourceDebugEnabled/SourceDebugEnabledLib.inf
  WatchdogTimerLib|MsKdDebugPkg2/Library/WatchdogTimerLibNull/WatchdogTimerLibNull.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
  RngLib|MdePkg/Library/BaseRngLib/BaseRngLib.inf

# memcpy and memset are difficult to avoid, esp. on ARM64.
# e.g. struct/union/array init/assign, passing va_list by value in PrintLib.
# Therefore link CompilerIntrinsicsLib everywhere (NULL class).
# Current upstream AMD64 CompilerIntrinsicsLib does not compile but future does.
[LibraryClasses.AARCH64]
  NULL|MdePkg/Library/CompilerIntrinsicsLib/CompilerIntrinsicsLib.inf

[LibraryClasses.common.DXE_DRIVER]
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  KdTransportLib|MsKdDebugPkg2/Library/KdTransportLibSerial/KdTransportSerial.inf
  KdProtocolLib|MsKdDebugPkg2/Library/KdProtocolLib/KdProtocolLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf

  # Provide StackCookie support lib so that we can link to /GS exports
  NULL|MdePkg/Library/StackCheckLib/StackCheckLib.inf
  StackCheckFailureLib|MdePkg/Library/StackCheckFailureLibNull/StackCheckFailureLibNull.inf

[Components]
  MsKdDebugPkg2/KdDxe/KdDxe.inf {
    <BuildOptions>
      # Because we're using the NULL SerialPort, we have to ignore unreachable code in the serial processing.
      MSFT:*_*_*_CC_FLAGS = /wd"4702"
  }
  MsKdDebugPkg2/Library/DxeDebugLibRouter/DxeDebugLibRouter.inf
  MsKdDebugPkg2/Library/KdProtocolLib/KdProtocolLib.inf
  MsKdDebugPkg2/Library/KdTransportLibSerial/KdTransportSerial.inf
  MsKdDebugPkg2/Library/WatchdogTimerLibNull/WatchdogTimerLibNull.inf

[BuildOptions]
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES
  MSFT:*_*_*_DLINK_FLAGS = /DEBUG:FULL /PDBALTPATH:$(MODULE_NAME).pdb
  *_CLANGPDB_*_DLINK_FLAGS = /DEBUG:FULL /PDBALTPATH:$(MODULE_NAME).pdb
  MSFT:*_*_*_CC_FLAGS = /Z7
  *_CLANGPDB_*_CC_FLAGS = -g -gcodeview -gcodeview-ghash -gcodeview-command-line

  # Set file alignment and (memory) alignment to 4K.
  # Memory alignment 4K is required for page protection.
  # i.e. So that, text/data/rdata are on different pages,
  # so that data/rdata are not executable and text/rdata are not writable.
  # This is the main reason sections exist and the main feature of the PE format.
  # File==memory for execute in place, or loader perf/simplicity otherwise.
  # Memory alignment defaults to 4K, if not otherwise changed by build system.
  MSFT:*_*_*_DLINK_FLAGS      = -align:4096 -filealign:4096
  *_CLANGPDB_*_DLINK_FLAGS    = -align:4096 -filealign:4096
  *_GCC_*_ASLDLINK_FLAGS = -z common-page-size=0x1000
  *_GCC_*_DLINK_FLAGS    = -z common-page-size=0x1000

  # Workaround https://github.com/tianocore/edk2/pull/11535/changes/38760819a84de03127e0d93abd238eee2ce7a6e8
  # BaseTools/Conf: Make ASLCC_FLAGS independent of CC_FLAGS
  # GenFW:ConvertELF fails otherwise.
  # Ideal, but does not work: *_GCC_AARCH64_ASLCC_FLAGS = $(GCC_AARCH64_CC_FLAGS)
  *_GCC_AARCH64_ASLCC_FLAGS = -mlittle-endian -fno-short-enums -fverbose-asm -funsigned-char -ffunction-sections -fdata-sections -Wno-address -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-pic -fno-pie -ffixed-x18 -mstack-protector-guard=global

# ARM64 has a UEFI spec requirement that RuntimeServiceCode/Data is 64K aligned.
# This applies to in-memory section alignment, and need not apply to file system alignment.
[BuildOptions.common.EDKII.DXE_RUNTIME_DRIVER]
  MSFT:*_*_AARCH64_DLINK_FLAGS      = -align:0x10000
  *_CLANGPDB_AARCH64_DLINK_FLAGS    = -align:0x10000
  *_GCC_AARCH64_ASLDLINK_FLAGS = -z common-page-size=0x10000
  *_GCC_AARCH64_DLINK_FLAGS    = -z common-page-size=0x10000
