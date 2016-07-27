/** @file
  Industry Standard Definitions of SMBIOS Table Specification v2.4.0

Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under 
the terms and conditions of the BSD License that accompanies this distribution.  
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.                                          
    
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef __SMBIOS24_STANDARD_H__
#define __SMBIOS24_STANDARD_H__

///
/// Reference SMBIOS 2.4, chapter 3.1.2.
/// For v2.1 and later, handle values in the range 0FF00h to 0FFFFh are reserved for
/// use by this specification.
///
#define SMBIOS_HANDLE_RESERVED_BEGIN 0xFF00

///
/// Reference SMBIOS 2.4, chapter 3.1.2.
/// The UEFI Platform Initialization Specification reserves handle number FFFEh for its
/// EFI_SMBIOS_PROTOCOL.Add() function to mean "assign an unused handle number automatically."
/// This number is not used for any other purpose by the SMBIOS specification.
///
#define SMBIOS_HANDLE_PI_RESERVED 0xFFFE

///
/// Reference SMBIOS 2.4, chapter 3.1.3.
/// Each text string is limited to 64 significant characters due to system MIF limitations.
///
#define SMBIOS_STRING_MAX_LENGTH     64

///
/// Inactive type is added from SMBIOS 2.2. Reference SMBIOS 2.4, chapter 3.3.41.
/// Upper-level software that interprets the SMBIOS structure-table should bypass an 
/// Inactive structure just like a structure type that the software does not recognize.
///
#define SMBIOS_TYPE_INACTIVE         0x007E    

///
/// End-of-table type is added from SMBIOS 2.2. Reference SMBIOS 2.4, chapter 3.3.42.
/// The end-of-table indicator is used in the last physical structure in a table
///
#define SMBIOS_TYPE_END_OF_TABLE     0x007F

///
/// Smbios Table Entry Point Structure.
///
#pragma pack(1)
typedef struct {
  UINT8   AnchorString[4];
  UINT8   EntryPointStructureChecksum;
  UINT8   EntryPointLength;
  UINT8   MajorVersion;
  UINT8   MinorVersion;
  UINT16  MaxStructureSize;
  UINT8   EntryPointRevision;
  UINT8   FormattedArea[5];
  UINT8   IntermediateAnchorString[5];
  UINT8   IntermediateChecksum;
  UINT16  TableLength;
  UINT32  TableAddress;
  UINT16  NumberOfSmbiosStructures;
  UINT8   SmbiosBcdRevision;
} SMBIOS_TABLE_ENTRY_POINT;

///
/// The Smbios structure header.
///
typedef struct {
  UINT8   Type;
  UINT8   Length;
  UINT16  Handle;
} SMBIOS_STRUCTURE;

///
/// String Number for a Null terminated string, 00h stands for no string available.
///
typedef UINT8 SMBIOS_TABLE_STRING;

///
/// BIOS Characteristics
/// Defines which functions the BIOS supports. PCI, PCMCIA, Flash, etc.
///
typedef struct {
  UINT32  Reserved                          :2;  ///< Bits 0-1.
  UINT32  Unknown                           :1;
  UINT32  BiosCharacteristicsNotSupported   :1;
  UINT32  IsaIsSupported                    :1;
  UINT32  McaIsSupported                    :1;
  UINT32  EisaIsSupported                   :1;
  UINT32  PciIsSupported                    :1;
  UINT32  PcmciaIsSupported                 :1;
  UINT32  PlugAndPlayIsSupported            :1;
  UINT32  ApmIsSupported                    :1;
  UINT32  BiosIsUpgradable                  :1;
  UINT32  BiosShadowingAllowed              :1;
  UINT32  VlVesaIsSupported                 :1;
  UINT32  EscdSupportIsAvailable            :1;
  UINT32  BootFromCdIsSupported             :1;
  UINT32  SelectableBootIsSupported         :1;
  UINT32  RomBiosIsSocketed                 :1;
  UINT32  BootFromPcmciaIsSupported         :1;
  UINT32  EDDSpecificationIsSupported       :1;
  UINT32  JapaneseNecFloppyIsSupported      :1;
  UINT32  JapaneseToshibaFloppyIsSupported  :1;
  UINT32  Floppy525_360IsSupported          :1;
  UINT32  Floppy525_12IsSupported           :1;
  UINT32  Floppy35_720IsSupported           :1;
  UINT32  Floppy35_288IsSupported           :1;
  UINT32  PrintScreenIsSupported            :1;
  UINT32  Keyboard8042IsSupported           :1;
  UINT32  SerialIsSupported                 :1;
  UINT32  PrinterIsSupported                :1;
  UINT32  CgaMonoIsSupported                :1;
  UINT32  NecPc98                           :1;
  UINT32  ReservedForVendor                 :32; ///< Bits 32-63. Bits 32-47 reserved for BIOS vendor 
                                                 ///< and bits 48-63 reserved for System Vendor. 
} MISC_BIOS_CHARACTERISTICS;

///
/// BIOS Characteristics Extension Byte 1.
/// This information, available for SMBIOS version 2.1 and later, appears at offset 12h
/// within the BIOS Information structure.
///
typedef struct {
  UINT8  AcpiIsSupported                   :1;
  UINT8  UsbLegacyIsSupported              :1;
  UINT8  AgpIsSupported                    :1;
  UINT8  I2OBootIsSupported                :1;
  UINT8  Ls120BootIsSupported              :1;
  UINT8  AtapiZipDriveBootIsSupported      :1;
  UINT8  Boot1394IsSupported               :1;
  UINT8  SmartBatteryIsSupported           :1;
} MBCE_BIOS_RESERVED;

///
/// BIOS Characteristics Extension Byte 2.
/// This information, available for SMBIOS version 2.3 and later, appears at offset 13h
/// within the BIOS Information structure.
///
typedef struct {
  UINT8  BiosBootSpecIsSupported              :1;
  UINT8  FunctionKeyNetworkBootIsSupported    :1;
  UINT8  TargetContentDistributionEnabled     :1;
  UINT8  Reserved                             :5;
} MBCE_SYSTEM_RESERVED;

///
/// BIOS Characteristics Extension Bytes.
///
typedef struct {
  MBCE_BIOS_RESERVED    BiosReserved;
  MBCE_SYSTEM_RESERVED  SystemReserved;
} MISC_BIOS_CHARACTERISTICS_EXTENSION;

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
} SMBIOS_TABLE_TYPE0;

///
///  System Wake-up Type.
///
typedef enum {  
  SystemWakeupTypeReserved         = 0x00,
  SystemWakeupTypeOther            = 0x01,
  SystemWakeupTypeUnknown          = 0x02,
  SystemWakeupTypeApmTimer         = 0x03,
  SystemWakeupTypeModemRing        = 0x04,
  SystemWakeupTypeLanRemote        = 0x05,
  SystemWakeupTypePowerSwitch      = 0x06,
  SystemWakeupTypePciPme           = 0x07,
  SystemWakeupTypeAcPowerRestored  = 0x08
} MISC_SYSTEM_WAKEUP_TYPE;

///
/// System Information (Type 1).
/// 
/// The information in this structure defines attributes of the overall system and is 
/// intended to be associated with the Component ID group of the system's MIF.
/// An SMBIOS implementation is associated with a single system instance and contains 
/// one and only one System Information (Type 1) structure.
///
typedef struct {
  SMBIOS_STRUCTURE        Hdr;
  SMBIOS_TABLE_STRING     Manufacturer;
  SMBIOS_TABLE_STRING     ProductName;
  SMBIOS_TABLE_STRING     Version;
  SMBIOS_TABLE_STRING     SerialNumber;
  GUID                    Uuid;
  UINT8                   WakeUpType;           ///< The enumeration value from MISC_SYSTEM_WAKEUP_TYPE.
  SMBIOS_TABLE_STRING     SKUNumber;
  SMBIOS_TABLE_STRING     Family;
} SMBIOS_TABLE_TYPE1;

///
///  Base Board - Feature Flags. 
///
typedef struct {
  UINT8  Motherboard           :1;
  UINT8  RequiresDaughterCard  :1;
  UINT8  Removable             :1;
  UINT8  Replaceable           :1;
  UINT8  HotSwappable          :1;
  UINT8  Reserved              :3;
} BASE_BOARD_FEATURE_FLAGS;

///
///  Base Board - Board Type.
///
typedef enum {  
  BaseBoardTypeUnknown                  = 0x1,
  BaseBoardTypeOther                    = 0x2,
  BaseBoardTypeServerBlade              = 0x3,
  BaseBoardTypeConnectivitySwitch       = 0x4,
  BaseBoardTypeSystemManagementModule   = 0x5,
  BaseBoardTypeProcessorModule          = 0x6,
  BaseBoardTypeIOModule                 = 0x7,
  BaseBoardTypeMemoryModule             = 0x8,
  BaseBoardTypeDaughterBoard            = 0x9,
  BaseBoardTypeMotherBoard              = 0xA,
  BaseBoardTypeProcessorMemoryModule    = 0xB,
  BaseBoardTypeProcessorIOModule        = 0xC,
  BaseBoardTypeInterconnectBoard        = 0xD
} BASE_BOARD_TYPE;

///
/// Base Board (or Module) Information (Type 2).
///
/// The information in this structure defines attributes of a system baseboard - 
/// for example a motherboard, planar, or server blade or other standard system module.
///
typedef struct {
  SMBIOS_STRUCTURE          Hdr;
  SMBIOS_TABLE_STRING       Manufacturer;
  SMBIOS_TABLE_STRING       ProductName;
  SMBIOS_TABLE_STRING       Version;
  SMBIOS_TABLE_STRING       SerialNumber;
  SMBIOS_TABLE_STRING       AssetTag;
  BASE_BOARD_FEATURE_FLAGS  FeatureFlag;
  SMBIOS_TABLE_STRING       LocationInChassis;
  UINT16                    ChassisHandle;
  UINT8                     BoardType;              ///< The enumeration value from BASE_BOARD_TYPE.
  UINT8                     NumberOfContainedObjectHandles;
  UINT16                    ContainedObjectHandles[1];
} SMBIOS_TABLE_TYPE2;

///
/// System Enclosure or Chassis Types
///
typedef enum {  
  MiscChassisTypeOther                = 0x01,
  MiscChassisTypeUnknown              = 0x02,
  MiscChassisTypeDeskTop              = 0x03,
  MiscChassisTypeLowProfileDesktop    = 0x04,
  MiscChassisTypePizzaBox             = 0x05,
  MiscChassisTypeMiniTower            = 0x06,
  MiscChassisTypeTower                = 0x07,
  MiscChassisTypePortable             = 0x08,
  MiscChassisTypeLapTop               = 0x09,
  MiscChassisTypeNotebook             = 0x0A,
  MiscChassisTypeHandHeld             = 0x0B,
  MiscChassisTypeDockingStation       = 0x0C,
  MiscChassisTypeAllInOne             = 0x0D,
  MiscChassisTypeSubNotebook          = 0x0E,
  MiscChassisTypeSpaceSaving          = 0x0F,
  MiscChassisTypeLunchBox             = 0x10,
  MiscChassisTypeMainServerChassis    = 0x11,
  MiscChassisTypeExpansionChassis     = 0x12,
  MiscChassisTypeSubChassis           = 0x13,
  MiscChassisTypeBusExpansionChassis  = 0x14,
  MiscChassisTypePeripheralChassis    = 0x15,
  MiscChassisTypeRaidChassis          = 0x16,
  MiscChassisTypeRackMountChassis     = 0x17,
  MiscChassisTypeSealedCasePc         = 0x18,
  MiscChassisMultiSystemChassis       = 0x19
} MISC_CHASSIS_TYPE;

///
/// System Enclosure or Chassis States .
///
typedef enum {  
  ChassisStateOther           = 0x01,
  ChassisStateUnknown         = 0x02,
  ChassisStateSafe            = 0x03,
  ChassisStateWarning         = 0x04,
  ChassisStateCritical        = 0x05,
  ChassisStateNonRecoverable  = 0x06
} MISC_CHASSIS_STATE;

///
/// System Enclosure or Chassis Security Status.
///
typedef enum {  
  ChassisSecurityStatusOther                          = 0x01,
  ChassisSecurityStatusUnknown                        = 0x02,
  ChassisSecurityStatusNone                           = 0x03,
  ChassisSecurityStatusExternalInterfaceLockedOut     = 0x04,
  ChassisSecurityStatusExternalInterfaceLockedEnabled = 0x05
} MISC_CHASSIS_SECURITY_STATE;

///
/// Contained Element record
///
typedef struct {
  UINT8                 ContainedElementType;
  UINT8                 ContainedElementMinimum;
  UINT8                 ContainedElementMaximum;
} CONTAINED_ELEMENT;


///
/// System Enclosure or Chassis (Type 3).
///
/// The information in this structure defines attributes of the system's mechanical enclosure(s).  
/// For example, if a system included a separate enclosure for its peripheral devices, 
/// two structures would be returned: one for the main, system enclosure and the second for
/// the peripheral device enclosure.  The additions to this structure in v2.1 of this specification
/// support the population of the CIM_Chassis class. 
///
typedef struct {
  SMBIOS_STRUCTURE            Hdr;
  SMBIOS_TABLE_STRING         Manufacturer;
  UINT8                       Type;
  SMBIOS_TABLE_STRING         Version;
  SMBIOS_TABLE_STRING         SerialNumber;
  SMBIOS_TABLE_STRING         AssetTag;
  UINT8                       BootupState;            ///< The enumeration value from MISC_CHASSIS_STATE.
  UINT8                       PowerSupplyState;       ///< The enumeration value from MISC_CHASSIS_STATE.
  UINT8                       ThermalState;           ///< The enumeration value from MISC_CHASSIS_STATE.
  UINT8                       SecurityStatus;         ///< The enumeration value from MISC_CHASSIS_SECURITY_STATE.
  UINT8                       OemDefined[4];
  UINT8                       Height;
  UINT8                       NumberofPowerCords;
  UINT8                       ContainedElementCount;
  UINT8                       ContainedElementRecordLength;
  CONTAINED_ELEMENT           ContainedElements[1];
} SMBIOS_TABLE_TYPE3;

///
/// Processor Information - Processor Type.
///
typedef enum {
  ProcessorOther   = 0x01,
  ProcessorUnknown = 0x02,
  CentralProcessor = 0x03,
  MathProcessor    = 0x04,
  DspProcessor     = 0x05,
  VideoProcessor   = 0x06
} PROCESSOR_TYPE_DATA;

///
/// Processor Information - Processor Family.
///
typedef enum {
  ProcessorFamilyOther                  = 0x01, 
  ProcessorFamilyUnknown                = 0x02,
  ProcessorFamily8086                   = 0x03, 
  ProcessorFamily80286                  = 0x04,
  ProcessorFamilyIntel386               = 0x05, 
  ProcessorFamilyIntel486               = 0x06,
  ProcessorFamily8087                   = 0x07,
  ProcessorFamily80287                  = 0x08,
  ProcessorFamily80387                  = 0x09, 
  ProcessorFamily80487                  = 0x0A,
  ProcessorFamilyPentium                = 0x0B, 
  ProcessorFamilyPentiumPro             = 0x0C,
  ProcessorFamilyPentiumII              = 0x0D,
  ProcessorFamilyPentiumMMX             = 0x0E,
  ProcessorFamilyCeleron                = 0x0F,
  ProcessorFamilyPentiumIIXeon          = 0x10,
  ProcessorFamilyPentiumIII             = 0x11, 
  ProcessorFamilyM1                     = 0x12,
  ProcessorFamilyM2                     = 0x13,
  // gap
  ProcessorFamilyAmdDuron               = 0x18,
  ProcessorFamilyK5                     = 0x19, 
  ProcessorFamilyK6                     = 0x1A,
  ProcessorFamilyK6_2                   = 0x1B,
  ProcessorFamilyK6_3                   = 0x1C,
  ProcessorFamilyAmdAthlon              = 0x1D,
  ProcessorFamilyAmd29000               = 0x1E,
  ProcessorFamilyK6_2Plus               = 0x1F,
  ProcessorFamilyPowerPC                = 0x20,
  ProcessorFamilyPowerPC601             = 0x21,
  ProcessorFamilyPowerPC603             = 0x22,
  ProcessorFamilyPowerPC603Plus         = 0x23,
  ProcessorFamilyPowerPC604             = 0x24,
  ProcessorFamilyPowerPC620             = 0x25,
  ProcessorFamilyPowerPCx704            = 0x26,
  ProcessorFamilyPowerPC750             = 0x27,
  // gap
  ProcessorFamilyAlpha3                 = 0x30,
  ProcessorFamilyAlpha21064             = 0x31,
  ProcessorFamilyAlpha21066             = 0x32,
  ProcessorFamilyAlpha21164             = 0x33,
  ProcessorFamilyAlpha21164PC           = 0x34,
  ProcessorFamilyAlpha21164a            = 0x35,
  ProcessorFamilyAlpha21264             = 0x36,
  ProcessorFamilyAlpha21364             = 0x37,
  // gap
  ProcessorFamilyMips                   = 0x40,
  ProcessorFamilyMIPSR4000              = 0x41,
  ProcessorFamilyMIPSR4200              = 0x42,
  ProcessorFamilyMIPSR4400              = 0x43,
  ProcessorFamilyMIPSR4600              = 0x44,
  ProcessorFamilyMIPSR10000             = 0x45,
  // gap
  ProcessorFamilySparc                  = 0x50,
  ProcessorFamilySuperSparc             = 0x51,
  ProcessorFamilymicroSparcII           = 0x52,
  ProcessorFamilymicroSparcIIep         = 0x53,
  ProcessorFamilyUltraSparc             = 0x54,
  ProcessorFamilyUltraSparcII           = 0x55,
  ProcessorFamilyUltraSparcIIi          = 0x56,
  ProcessorFamilyUltraSparcIII          = 0x57,
  ProcessorFamilyUltraSparcIIIi         = 0x58,
  // gap
  ProcessorFamily68040                  = 0x60,
  ProcessorFamily68xxx                  = 0x61,
  ProcessorFamily68000                  = 0x62,
  ProcessorFamily68010                  = 0x63,
  ProcessorFamily68020                  = 0x64,
  ProcessorFamily68030                  = 0x65,
  // gap
  ProcessorFamilyHobbit                 = 0x70,
  // gap
  ProcessorFamilyCrusoeTM5000           = 0x78,
  ProcessorFamilyCrusoeTM3000           = 0x79,
  ProcessorFamilyEfficeonTM8000         = 0x7A,
  // gap
  ProcessorFamilyWeitek                 = 0x80,
  // gap
  ProcessorFamilyItanium                = 0x82,
  ProcessorFamilyAmdAthlon64            = 0x83,
  ProcessorFamilyAmdOpteron             = 0x84,
  // gap
  ProcessorFamilyPARISC                 = 0x90,
  ProcessorFamilyPaRisc8500             = 0x91,
  ProcessorFamilyPaRisc8000             = 0x92,
  ProcessorFamilyPaRisc7300LC           = 0x93,
  ProcessorFamilyPaRisc7200             = 0x94,
  ProcessorFamilyPaRisc7100LC           = 0x95,
  ProcessorFamilyPaRisc7100             = 0x96,
  // gap
  ProcessorFamilyV30                    = 0xA0,
  // gap
  ProcessorFamilyPentiumIIIXeon         = 0xB0,
  ProcessorFamilyPentiumIIISpeedStep    = 0xB1,
  ProcessorFamilyPentium4               = 0xB2,
  ProcessorFamilyIntelXeon              = 0xB3,
  ProcessorFamilyAS400                  = 0xB4,
  ProcessorFamilyIntelXeonMP            = 0xB5,
  ProcessorFamilyAMDAthlonXP            = 0xB6,
  ProcessorFamilyAMDAthlonMP            = 0xB7,
  ProcessorFamilyIntelItanium2          = 0xB8,
  ProcessorFamilyIntelPentiumM          = 0xB9,
  // gap
  ProcessorFamilyIBM390                 = 0xC8,
  ProcessorFamilyG4                     = 0xC9,
  ProcessorFamilyG5                     = 0xCA,
  // gap
  ProcessorFamilyi860                   = 0xFA,
  ProcessorFamilyi960                   = 0xFB,
  // gap
  ProcessorFamilyReserved1              = 0xFF
} PROCESSOR_FAMILY_DATA;

///
/// Processor Information - Voltage. 
///
typedef struct {
  UINT8  ProcessorVoltageCapability5V        :1; 
  UINT8  ProcessorVoltageCapability3_3V      :1;  
  UINT8  ProcessorVoltageCapability2_9V      :1;  
  UINT8  ProcessorVoltageCapabilityReserved  :1; ///< Bit 3, must be zero.
  UINT8  ProcessorVoltageReserved            :3; ///< Bits 4-6, must be zero.
  UINT8  ProcessorVoltageIndicateLegacy      :1;
} PROCESSOR_VOLTAGE;

///
/// Processor Information - Processor Upgrade.
///
typedef enum {
  ProcessorUpgradeOther         = 0x01,
  ProcessorUpgradeUnknown       = 0x02,
  ProcessorUpgradeDaughterBoard = 0x03,
  ProcessorUpgradeZIFSocket     = 0x04,
  ProcessorUpgradePiggyBack     = 0x05, ///< Replaceable.
  ProcessorUpgradeNone          = 0x06,
  ProcessorUpgradeLIFSocket     = 0x07,
  ProcessorUpgradeSlot1         = 0x08,
  ProcessorUpgradeSlot2         = 0x09,
  ProcessorUpgrade370PinSocket  = 0x0A,
  ProcessorUpgradeSlotA         = 0x0B,
  ProcessorUpgradeSlotM         = 0x0C,
  ProcessorUpgradeSocket423     = 0x0D,
  ProcessorUpgradeSocketA       = 0x0E, ///< Socket 462.
  ProcessorUpgradeSocket478     = 0x0F,
  ProcessorUpgradeSocket754     = 0x10,
  ProcessorUpgradeSocket940     = 0x11,
} PROCESSOR_UPGRADE;

///
/// Processor ID Field Description
///
typedef struct {
  UINT32  ProcessorSteppingId:4;
  UINT32  ProcessorModel:     4;
  UINT32  ProcessorFamily:    4;
  UINT32  ProcessorType:      2;
  UINT32  ProcessorReserved1: 2;
  UINT32  ProcessorXModel:    4;
  UINT32  ProcessorXFamily:   8;
  UINT32  ProcessorReserved2: 4;
} PROCESSOR_SIGNATURE;

typedef struct {
  UINT32  ProcessorFpu       :1;
  UINT32  ProcessorVme       :1;
  UINT32  ProcessorDe        :1;
  UINT32  ProcessorPse       :1;
  UINT32  ProcessorTsc       :1;
  UINT32  ProcessorMsr       :1;
  UINT32  ProcessorPae       :1;
  UINT32  ProcessorMce       :1;
  UINT32  ProcessorCx8       :1;
  UINT32  ProcessorApic      :1;
  UINT32  ProcessorReserved1 :1;
  UINT32  ProcessorSep       :1;
  UINT32  ProcessorMtrr      :1;
  UINT32  ProcessorPge       :1;
  UINT32  ProcessorMca       :1;
  UINT32  ProcessorCmov      :1;
  UINT32  ProcessorPat       :1;
  UINT32  ProcessorPse36     :1;
  UINT32  ProcessorPsn       :1;
  UINT32  ProcessorClfsh     :1;
  UINT32  ProcessorReserved2 :1;
  UINT32  ProcessorDs        :1;
  UINT32  ProcessorAcpi      :1;
  UINT32  ProcessorMmx       :1;
  UINT32  ProcessorFxsr      :1;
  UINT32  ProcessorSse       :1;
  UINT32  ProcessorSse2      :1;
  UINT32  ProcessorSs        :1;
  UINT32  ProcessorReserved3 :1;
  UINT32  ProcessorTm        :1;
  UINT32  ProcessorReserved4 :2;
} PROCESSOR_FEATURE_FLAGS;

typedef struct {
  PROCESSOR_SIGNATURE     Signature;
  PROCESSOR_FEATURE_FLAGS FeatureFlags;
} PROCESSOR_ID_DATA;

///
/// Processor Information (Type 4).
///
/// The information in this structure defines the attributes of a single processor; 
/// a separate structure instance is provided for each system processor socket/slot. 
/// For example, a system with an IntelDX2 processor would have a single 
/// structure instance, while a system with an IntelSX2 processor would have a structure
/// to describe the main CPU, and a second structure to describe the 80487 co-processor. 
///
typedef struct { 
  SMBIOS_STRUCTURE      Hdr;
  SMBIOS_TABLE_STRING   Socket;
  UINT8                 ProcessorType;          ///< The enumeration value from PROCESSOR_TYPE_DATA.
  UINT8                 ProcessorFamily;        ///< The enumeration value from PROCESSOR_FAMILY_DATA.
  SMBIOS_TABLE_STRING   ProcessorManufacture;
  PROCESSOR_ID_DATA     ProcessorId;
  SMBIOS_TABLE_STRING   ProcessorVersion;
  PROCESSOR_VOLTAGE     Voltage;
  UINT16                ExternalClock;
  UINT16                MaxSpeed;
  UINT16                CurrentSpeed;
  UINT8                 Status;
  UINT8                 ProcessorUpgrade;      ///< The enumeration value from PROCESSOR_UPGRADE.
  UINT16                L1CacheHandle;
  UINT16                L2CacheHandle;
  UINT16                L3CacheHandle;
  SMBIOS_TABLE_STRING   SerialNumber;
  SMBIOS_TABLE_STRING   AssetTag;
  SMBIOS_TABLE_STRING   PartNumber;
} SMBIOS_TABLE_TYPE4;

///
/// OEM Strings (Type 11).
/// This structure contains free form strings defined by the OEM. Examples of this are: 
/// Part Numbers for Reference Documents for the system, contact information for the manufacturer, etc. 
///
typedef struct {
  SMBIOS_STRUCTURE      Hdr;
  UINT8                 StringCount;
} SMBIOS_TABLE_TYPE11;

///
/// Physical Memory Array - Location.
///
typedef enum {
  MemoryArrayLocationOther                 = 0x01,
  MemoryArrayLocationUnknown               = 0x02,
  MemoryArrayLocationSystemBoard           = 0x03,
  MemoryArrayLocationIsaAddonCard          = 0x04,
  MemoryArrayLocationEisaAddonCard         = 0x05,
  MemoryArrayLocationPciAddonCard          = 0x06,
  MemoryArrayLocationMcaAddonCard          = 0x07,
  MemoryArrayLocationPcmciaAddonCard       = 0x08,
  MemoryArrayLocationProprietaryAddonCard  = 0x09,
  MemoryArrayLocationNuBus                 = 0x0A,
  MemoryArrayLocationPc98C20AddonCard      = 0xA0,
  MemoryArrayLocationPc98C24AddonCard      = 0xA1,
  MemoryArrayLocationPc98EAddonCard        = 0xA2,
  MemoryArrayLocationPc98LocalBusAddonCard = 0xA3
} MEMORY_ARRAY_LOCATION;

///
/// Physical Memory Array - Use.
///
typedef enum {
  MemoryArrayUseOther                      = 0x01,
  MemoryArrayUseUnknown                    = 0x02,
  MemoryArrayUseSystemMemory               = 0x03,
  MemoryArrayUseVideoMemory                = 0x04,
  MemoryArrayUseFlashMemory                = 0x05,
  MemoryArrayUseNonVolatileRam             = 0x06,
  MemoryArrayUseCacheMemory                = 0x07
} MEMORY_ARRAY_USE;

///
/// Physical Memory Array - Error Correction Types. 
///
typedef enum {
  MemoryErrorCorrectionOther               = 0x01,
  MemoryErrorCorrectionUnknown             = 0x02,
  MemoryErrorCorrectionNone                = 0x03,
  MemoryErrorCorrectionParity              = 0x04,
  MemoryErrorCorrectionSingleBitEcc        = 0x05,
  MemoryErrorCorrectionMultiBitEcc         = 0x06,
  MemoryErrorCorrectionCrc                 = 0x07
} MEMORY_ERROR_CORRECTION;

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
} SMBIOS_TABLE_TYPE16;

///
/// Memory Device - Form Factor.
///
typedef enum {
  MemoryFormFactorOther                    = 0x01,
  MemoryFormFactorUnknown                  = 0x02,
  MemoryFormFactorSimm                     = 0x03,
  MemoryFormFactorSip                      = 0x04,
  MemoryFormFactorChip                     = 0x05,
  MemoryFormFactorDip                      = 0x06,
  MemoryFormFactorZip                      = 0x07,
  MemoryFormFactorProprietaryCard          = 0x08,
  MemoryFormFactorDimm                     = 0x09,
  MemoryFormFactorTsop                     = 0x0A,
  MemoryFormFactorRowOfChips               = 0x0B,
  MemoryFormFactorRimm                     = 0x0C,
  MemoryFormFactorSodimm                   = 0x0D,
  MemoryFormFactorSrimm                    = 0x0E,
} MEMORY_FORM_FACTOR;

///
/// Memory Device - Type
///
typedef enum {
  MemoryTypeOther                          = 0x01,
  MemoryTypeUnknown                        = 0x02,
  MemoryTypeDram                           = 0x03,
  MemoryTypeEdram                          = 0x04,
  MemoryTypeVram                           = 0x05,
  MemoryTypeSram                           = 0x06,
  MemoryTypeRam                            = 0x07,
  MemoryTypeRom                            = 0x08,
  MemoryTypeFlash                          = 0x09,
  MemoryTypeEeprom                         = 0x0A,
  MemoryTypeFeprom                         = 0x0B,
  MemoryTypeEprom                          = 0x0C,
  MemoryTypeCdram                          = 0x0D,
  MemoryType3Dram                          = 0x0E,
  MemoryTypeSdram                          = 0x0F,
  MemoryTypeSgram                          = 0x10,
  MemoryTypeRdram                          = 0x11,
  MemoryTypeDdr                            = 0x12,
  MemoryTypeDdr2                           = 0x13,
} MEMORY_DEVICE_TYPE;

typedef struct {
  UINT16    Reserved        :1;
  UINT16    Other           :1;
  UINT16    Unknown         :1;
  UINT16    FastPaged       :1;
  UINT16    StaticColumn    :1;
  UINT16    PseudoStatic    :1;
  UINT16    Rambus          :1;
  UINT16    Synchronous     :1;
  UINT16    Cmos            :1;
  UINT16    Edo             :1;
  UINT16    WindowDram      :1;
  UINT16    CacheDram       :1;
  UINT16    Nonvolatile     :1;
  UINT16    Reserved1       :3;
} MEMORY_DEVICE_TYPE_DETAIL;

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
} SMBIOS_TABLE_TYPE17;

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
} SMBIOS_TABLE_TYPE19;

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
} SMBIOS_TABLE_TYPE20;

///
/// System Boot Information - System Boot Status.
///
typedef enum {
  BootInformationStatusNoError                  = 0x00,
  BootInformationStatusNoBootableMedia          = 0x01,
  BootInformationStatusNormalOSFailedLoading    = 0x02,
  BootInformationStatusFirmwareDetectedFailure  = 0x03,
  BootInformationStatusOSDetectedFailure        = 0x04,
  BootInformationStatusUserRequestedBoot        = 0x05,
  BootInformationStatusSystemSecurityViolation  = 0x06,
  BootInformationStatusPreviousRequestedImage   = 0x07,
  BootInformationStatusWatchdogTimerExpired     = 0x08,
  BootInformationStatusStartReserved            = 0x09,
  BootInformationStatusStartOemSpecific         = 0x80,
  BootInformationStatusStartProductSpecific     = 0xC0
} MISC_BOOT_INFORMATION_STATUS_DATA_TYPE;

///
/// System Boot Information (Type 32).
///
/// The client system firmware, e.g. BIOS, communicates the System Boot Status to the 
/// client's Pre-boot Execution Environment (PXE) boot image or OS-present management 
/// application via this structure. When used in the PXE environment, for example, 
/// this code identifies the reason the PXE was initiated and can be used by boot-image 
/// software to further automate an enterprise's PXE sessions.  For example, an enterprise  
/// could choose to automatically download a hardware-diagnostic image to a client whose 
/// reason code indicated either a firmware- or operating system-detected hardware failure.
///
typedef struct {
  SMBIOS_STRUCTURE                        Hdr;
  UINT8                                   Reserved[6];
  UINT8                                   BootStatus;     ///< The enumeration value from MISC_BOOT_INFORMATION_STATUS_DATA_TYPE.
} SMBIOS_TABLE_TYPE32;

///
/// Inactive (Type 126)
///
typedef struct {
  SMBIOS_STRUCTURE   Hdr;
} SMBIOS_TABLE_TYPE126;

///
/// End-of-Table (Type 127)
///
typedef struct {
  SMBIOS_STRUCTURE   Hdr;
} SMBIOS_TABLE_TYPE127;

#pragma pack()

#endif
