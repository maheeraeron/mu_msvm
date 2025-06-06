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

