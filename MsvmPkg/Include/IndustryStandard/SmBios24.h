/** @file
  Industry Standard Definitions of SMBIOS Table Specification v2.4.0

Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under
the terms and conditions of the BSD License that accompanies this distribution.
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Note: this is trimmed down to just the structs that were expanded after V2.4.0.

**/

#ifndef __SMBIOS24_STANDARD_H__
#define __SMBIOS24_STANDARD_H__

#pragma pack(1)

///
/// BIOS Information (Type 0).
///
typedef struct {
  SMBIOS_STRUCTURE          Hdr;
  SMBIOS_TABLE_STRING       Vendor;
  SMBIOS_TABLE_STRING       BiosVersion;
  UINT16                    BiosSegment;
  SMBIOS_TABLE_STRING       BiosReleaseDate;
  UINT8                     BiosSize;
  MISC_BIOS_CHARACTERISTICS BiosCharacteristics;
  UINT8                     BIOSCharacteristicsExtensionBytes[2];
  UINT8                     SystemBiosMajorRelease;
  UINT8                     SystemBiosMinorRelease;
  UINT8                     EmbeddedControllerFirmwareMajorRelease;
  UINT8                     EmbeddedControllerFirmwareMinorRelease;
} SMBIOS24_TABLE_TYPE0;

///
/// Physical Memory Array (Type 16).
///
/// This structure describes a collection of memory devices that operate
/// together to form a memory address space.
///
typedef struct {
  SMBIOS_STRUCTURE          Hdr;
  UINT8                     Location;                       ///< The enumeration value from MEMORY_ARRAY_LOCATION.
  UINT8                     Use;                            ///< The enumeration value from MEMORY_ARRAY_USE.
  UINT8                     MemoryErrorCorrection;          ///< The enumeration value from MEMORY_ERROR_CORRECTION.
  UINT32                    MaximumCapacity;
  UINT16                    MemoryErrorInformationHandle;
  UINT16                    NumberOfMemoryDevices;
} SMBIOS24_TABLE_TYPE16;

///
/// Memory Device (Type 17).
///
/// This structure describes a single memory device that is part of
/// a larger Physical Memory Array (Type 16).
/// Note:  If a system includes memory-device sockets, the SMBIOS implementation
/// includes a Memory Device structure instance for each slot, whether or not the
/// socket is currently populated.
///
typedef struct {
  SMBIOS_STRUCTURE          Hdr;
  UINT16                    MemoryArrayHandle;
  UINT16                    MemoryErrorInformationHandle;
  UINT16                    TotalWidth;
  UINT16                    DataWidth;
  UINT16                    Size;
  UINT8                     FormFactor;                     ///< The enumeration value from MEMORY_FORM_FACTOR.
  UINT8                     DeviceSet;
  SMBIOS_TABLE_STRING       DeviceLocator;
  SMBIOS_TABLE_STRING       BankLocator;
  UINT8                     MemoryType;                     ///< The enumeration value from MEMORY_DEVICE_TYPE.
  MEMORY_DEVICE_TYPE_DETAIL TypeDetail;
  UINT16                    Speed;
  SMBIOS_TABLE_STRING       Manufacturer;
  SMBIOS_TABLE_STRING       SerialNumber;
  SMBIOS_TABLE_STRING       AssetTag;
  SMBIOS_TABLE_STRING       PartNumber;
} SMBIOS24_TABLE_TYPE17;

///
/// Memory Array Mapped Address (Type 19).
///
/// This structure provides the address mapping for a Physical Memory Array.
/// One structure is present for each contiguous address range described.
///
typedef struct {
  SMBIOS_STRUCTURE      Hdr;
  UINT32                StartingAddress;
  UINT32                EndingAddress;
  UINT16                MemoryArrayHandle;
  UINT8                 PartitionWidth;
} SMBIOS24_TABLE_TYPE19;

///
/// Memory Device Mapped Address (Type 20).
///
/// This structure maps memory address space usually to a device-level granularity.
/// One structure is present for each contiguous address range described.
///
typedef struct {
  SMBIOS_STRUCTURE      Hdr;
  UINT32                StartingAddress;
  UINT32                EndingAddress;
  UINT16                MemoryDeviceHandle;
  UINT16                MemoryArrayMappedAddressHandle;
  UINT8                 PartitionRowPosition;
  UINT8                 InterleavePosition;
  UINT8                 InterleavedDataDepth;
} SMBIOS24_TABLE_TYPE20;

#pragma pack()

#endif
