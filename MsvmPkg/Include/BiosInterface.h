/*++

Copyright (c) Microsoft Corporation

Module Name:

    BiosInterface.h

Abstract:

    This file contains types and constants shared between
    the BiosDevice virtual device and the UEFI firmware.

--*/

#pragma once


//
// BIOS Interface constants
//
enum
{
    BiosInterfaceMaximumProcessorNumber    = 64,
    BiosInterfaceEntropyTableSize          = 64,
    BiosInterfaceGenerationIdSize          = 16,
    BiosInterfaceSmbiosStringMax           = 32
};


//
// BIOS configuration ports.
//
enum
{
    BiosAddressPort     = 0x28,
    BiosDataPort        = 0x2C
};


//
// Values/Selectors for the BIOS configuration ports.
//
// Existing values can not change after Hyper-V is released.
// Only new values can be added if they were previously unused.
//
enum
{
    BiosConfigFirstMemoryBlockSize      = 0x00,
    BiosConfigNumLockEnabled            = 0x01,
    BiosConfigBiosGuid                  = 0x02,
    BiosConfigBiosSystemSerialNumber    = 0x03,
    BiosConfigBiosBaseSerialNumber      = 0x04,
    BiosConfigBiosChassisSerialNumber   = 0x05,
    BiosConfigBiosChassisAssetTag       = 0x06,
    BiosConfigBootDeviceOrder           = 0x07,
    BiosConfigBiosProcessorCount        = 0x08,
    BiosConfigProcessorLocalApicId      = 0x09,
    BiosConfigSratSize                  = 0x0A,
    BiosConfigSratOffset                = 0x0B,
    BiosConfigSratData                  = 0x0C,
    BiosConfigMemoryAmountAbove4Gb      = 0x0D,
    BiosConfigGenerationIdPtrLow        = 0x0E,
    BiosConfigGenerationIdPtrHigh       = 0x0F,
    //
    // intentional gap here - obsolete values
    //
    BiosConfigPciIoGapStart             = 0x12,
    BiosConfigProcessorReplyStatusIndex = 0x13,
    BiosConfigProcessorReplyStatus      = 0x14,
    BiosConfigProcessorMatEnable        = 0x15,
    BiosConfigProcessorStaEnable        = 0x16,
    BiosConfigWaitNano100               = 0x17,
    BiosConfigWait1Millisecond          = 0x18,
    BiosConfigWait10Milliseconds        = 0x19,
    BiosConfigBootFinalize              = 0x1A,
    BiosConfigWait2Millisecond          = 0x1B,
    BiosConfigBiosLockString            = 0x1C,
    BiosConfigProcessorDMTFTable        = 0x1D,
    BiosConfigEntropyTable              = 0x1E,
    BiosConfigMemoryAboveHighMmio       = 0x1F,
    BiosConfigHighMmioGapBaseInMb       = 0x20,
    BiosConfigHighMmioGapLengthInMb     = 0x21,
    BiosConfigE820Entry                 = 0x22,
    BiosConfigInitialMegabytesBelowGap  = 0x23,
    //
    // Values added in Windows Blue
    //
    BiosConfigNvramCommand              = 0x24,
    BiosConfigWriteConfigPage           = 0x25,
    BiosConfigCryptoCommand             = 0x26,
    //
    // Watchdog device (Windows 8.1 MQ)
    //
    BiosConfigWatchdogConfig            = 0x27,
    BiosConfigWatchdogResolution        = 0x28,
    BiosConfigWatchdogCount             = 0x29,
    //
    // Event Logging (Windows 8.1 MQ/M0)
    //
    BiosConfigEventLogFlush             = 0x30,
    //
    // Set MOR bit variable. Triggered by TPM _DSM Memory Clear Interface.
    // In real hardware, _DSM triggers CPU SMM. UEFI SMM driver sets the
    // MOR state via variable service. Hypervisor does not support virtual SMM,
    // so _DSM is not able to trigger SMI in Hyper-V virtualization. The
    // alternative is to send an IO port command to BIOS device and persist the
    // MOR state in UEFI NVRAM via variable service on host.
    //
    BiosConfigMorSetVariable            = 0x31,
    //
    // VDev version. (Windows Threshold)
    //
    BiosConfigVdevVersion               = 0x32,
    //
    // Memory Map. (Windows Threshold)
    //
    BiosConfigMemoryMap                 = 0x33,
    //
    // ARM64 RTC GetTime SetTime (RS2)
    //
    BiosConfigGetTime                   = 0x34,
    BiosConfigSetTime                   = 0x35,
    //
    // Debugger output
    //
    BiosDebugOutputString               = 0x36,
    //
    // vPMem NFIT (RS3)
    //
    BiosConfigNfitSize                  = 0x37,
    BiosConfigNfitPopulate              = 0x38,
    BiosConfigVpmemSetACPIBuffer        = 0x39,
    //
    // This value should be the maximum posible value for the
    // address register with the exception of BiosConfigDebug.
    //
    BiosConfigMaxValue                  = BiosConfigVpmemSetACPIBuffer,
};


//
// Common SMBIOS structure header.
//
#pragma pack(push, 1)

typedef struct _SMBIOS_HEADER
{
    UINT8  Type;
    UINT8  Length;
    UINT16 Handle;
} SMBIOS_HEADER;

#pragma pack(pop)


//
// Other string to use as default string.
//
#define SMBIOS_NONE_STRING "None"
#define SMBIOS_NONE_STRING_SIZE sizeof(SMBIOS_NONE_STRING)


//
// Maximum length of a string in v2.4 SMBIOS structure.
//
#define MAX_SMBIOS_STRING_LENGTH 64


//
// SMBIOS v2.4 CPU Information structure.
//
#pragma pack(push, 1)

typedef struct _SMBIOS_CPU_INFO_FORMATTED
{
    SMBIOS_HEADER     Header;
    UINT8             SocketDesignation;
    UINT8             ProcessorType;
    UINT8             ProcessorFamily;
    UINT8             ProcessorManufacturer;
    UINT64            ProcessorId;
    UINT8             ProcessorVersion;
    UINT8             Voltage;
    UINT16            ExternalClock;
    UINT16            MaxSpeed;
    UINT16            CurrentSpeed;
    UINT8             Status;
    UINT8             Upgrade;
    UINT16            L1Handle;
    UINT16            L2Handle;
    UINT16            L3Handle;
    UINT8             SerialNumber;
    UINT8             AssetTag;
    UINT8             PartNumber;
 } SMBIOS_CPU_INFO_FORMATTED;


typedef struct _SMBIOS_CPU_INFO_STRINGS
{
    //
    // CPU Information structure string table.
    //
    // Sized for:
    //  4 "None" strings.
    //  2 strings obtained from host that are max 64 chars each.
    //  1 empty string to terminate the table.
    //
    char              StringTable[(4 * SMBIOS_NONE_STRING_SIZE) +
                                  ((MAX_SMBIOS_STRING_LENGTH + 1) * 2) +
                                  1];
} SMBIOS_CPU_INFO_STRINGS;


typedef struct _SMBIOS_CPU_INFO_STRINGS_LEGACY
{
    //
    // CPU Information structure string table for legacy BIOS.
    //
    // Sized for:
    //  1 "None" strings.
    //  2 strings obtained from host that are max 64 chars each.
    //  1 empty string to terminate the table.
    //
    char              StringTable[SMBIOS_NONE_STRING_SIZE +
                                  ((MAX_SMBIOS_STRING_LENGTH + 1) * 2)+
                                  1];
} SMBIOS_CPU_INFO_STRINGS_LEGACY;


typedef struct _SMBIOS_CPU_INFORMATION
{
    SMBIOS_CPU_INFO_FORMATTED Formatted;
    SMBIOS_CPU_INFO_STRINGS   Unformatted;
} SMBIOS_CPU_INFORMATION;


typedef struct _SMBIOS_CPU_INFORMATION_LEGACY
{
    SMBIOS_CPU_INFO_FORMATTED      Formatted;
    SMBIOS_CPU_INFO_STRINGS_LEGACY Unformatted;
} SMBIOS_CPU_INFORMATION_LEGACY;

#pragma pack(pop)


#define MAKE_VDEV_VERSION(_major_, _minor_) ((_major_) << 8 | (_minor_))


enum
{
    VDevVersion2   = MAKE_VDEV_VERSION(2, 0),
    VDevVersion3   = MAKE_VDEV_VERSION(3, 0),
    VDevVersion4   = MAKE_VDEV_VERSION(4, 0),
    VDevVersion5   = MAKE_VDEV_VERSION(5, 0)
};


//
// Memory map for VDev versions 2-4
//
typedef struct _VM_MEMORY_RANGE
{
    UINT64  BaseAddress;
    UINT64  Length;
} VM_MEMORY_RANGE, *PVM_MEMORY_RANGE;


//
// Memory map beginning with VDev version 5
//
#define VM_MEMORY_RANGE_FLAG_PLATFORM_RESERVED  0x1

typedef struct _VM_MEMORY_RANGE_V5
{
    UINT64  BaseAddress;
    UINT64  Length;
    UINT32  Flags;
    UINT32  Reserved;
} VM_MEMORY_RANGE_V5, *PVM_MEMORY_RANGE_V5;


// Configuration "page" that is requested by the UEFI firmware.
// This is the struct used by Windows Blue and subsequent versions
// of the firmware when the VM configuration is Windows Blue (5)
// and the BIOS VDev version is therefore 2.
//
typedef struct _BIOS_CONFIG_PAGE_V2
{
    UINT32 Size;
    UINT32 SratSize;
    UINT32 BiosSizePages;
    UINT32 ProcessorCount;
    UINT64 LowMemoryBasePages;
    UINT64 LowMemoryLengthPages;
    UINT64 MiddleMemoryBasePages;
    UINT64 MiddleMemoryLengthPages;
    UINT64 HighMemoryBasePages;
    UINT64 HighMemoryLengthPages;
    UINT64 LowMmioGapBasePages;
    UINT64 LowMmioGapLengthPages;
    UINT64 HighMmioGapBasePages;
    UINT64 HighMmioGapLengthPages;
    UINT8 Entropy[BiosInterfaceEntropyTableSize];
    UINT8 BiosGuid[sizeof(GUID)];
    UINT8 SystemSerialNumber[BiosInterfaceSmbiosStringMax + 1];
    UINT8 BaseSerialNumber[BiosInterfaceSmbiosStringMax + 1];
    UINT8 ChassisSerialNumber[BiosInterfaceSmbiosStringMax + 1];
    UINT8 ChassisAssetTag[BiosInterfaceSmbiosStringMax + 1];
    UINT8 BiosLockString[BiosInterfaceSmbiosStringMax + 1];
    SMBIOS_CPU_INFORMATION ProcessorInformation;
    struct {
        // Enables the UEFI debugger.  COM1 must also be configured.
        UINT32 DebuggerEnabled:1;
        // Triggers ACPI to load a table called "OEMP" which will is used by
        // the KPG test team to get a virtual PCIe root complex instantiated.
        UINT32 LoadOempTable:1;
        UINT32 HibernateEnabled:1;
        UINT32 Reserved:29;
    } Flags;
    struct {
        UINT32 SerialControllersEnabled:1;
        UINT32 PauseAfterBootFailure:1;
        UINT32 PxeIpV6:1;
        UINT32 Reserved:29;
    } Flags2;
} BIOS_CONFIG_PAGE_V2;

//
// Configuration "page" that is requested by the UEFI firmware.
// This is the struct used by Windows Threshold version of the
// firmware when the VM configuration is Windows Threshold (6).
// and the BIOS VDev version is therefore 3.
//
typedef struct _BIOS_CONFIG_PAGE_V3
{
    UINT32 Size;
    UINT32 SratSize;
    UINT32 MemoryMapSize;
    UINT32 BiosSizePages;
    UINT32 ProcessorCount;
    UINT64 LowMmioGapBasePages;
    UINT64 LowMmioGapLengthPages;
    UINT64 HighMmioGapBasePages;
    UINT64 HighMmioGapLengthPages;
    UINT8 Entropy[BiosInterfaceEntropyTableSize];
    UINT8 BiosGuid[sizeof(GUID)];
    UINT8 SystemSerialNumber[BiosInterfaceSmbiosStringMax + 1];
    UINT8 BaseSerialNumber[BiosInterfaceSmbiosStringMax + 1];
    UINT8 ChassisSerialNumber[BiosInterfaceSmbiosStringMax + 1];
    UINT8 ChassisAssetTag[BiosInterfaceSmbiosStringMax + 1];
    UINT8 BiosLockString[BiosInterfaceSmbiosStringMax + 1];
    SMBIOS_CPU_INFORMATION ProcessorInformation;
    struct {
        UINT32 SerialControllersEnabled:1;
        UINT32 PauseAfterBootFailure:1;
        UINT32 PxeIpV6:1;
        UINT32 DebuggerEnabled:1;
        UINT32 LoadOempTable:1;
        UINT32 TpmEnabled:1;
        UINT32 HibernateEnabled:1;
        UINT32 ConsoleMode:2;
        UINT32 MemoryAttributesTableEnabled:1;
        UINT32 Reserved:22;
    } Flags;
} BIOS_CONFIG_PAGE_V3;

//
// Command types for NVRAM_COMMAND_DESCRIPTOR.
//
// These correlate with the semantics of the UEFI runtime variable services.
//
typedef enum _NVRAM_COMMAND
{
    NvramGetVariableCommand,
    NvramSetVariableCommand,
    NvramGetFirstVariableNameCommand,
    NvramGetNextVariableNameCommand,
    NvramQueryInfoCommand,
    NvramSignalRuntimeCommand,
    NvramDebugStringCommand
} NVRAM_COMMAND;


//
// The maximum sizes in bytes for EFI Variable Name and Data.
//
// The Data size must be minimum 32K for secure boot databases.
//
#define EFI_MAX_VARIABLE_NAME_SIZE  (2 * 1024)
#define EFI_MAX_VARIABLE_DATA_SIZE  (32 * 1024)

//
// In-memory descriptor used to pass NVRAM variable requests
// from the UEFI firmware to the BIOS VDev.
//
#pragma pack(push, 1)

typedef struct _NVRAM_COMMAND_DESCRIPTOR
{
    NVRAM_COMMAND   Command;

    //
    // Status of the processed command.
    //
    UINT64          Status;

    union
    {
        struct
        {
            //
            // UEFI variable attributes associated with
            // the variable: access rights (RT/BS).
            // Used as input for the SetVariable command.
            // Used as output for the GetVariable command.
            //
            UINT32 VariableAttributes;

            //
            // GPA of the buffer containing a 16-bit
            // unicode variable name.
            // Memory at this location is read for the
            // GetVariable, SetVariable, GetNextVariable command.
            // Memory at this location is written to for
            // the GetNextVariable command.
            //
            UINT64 VariableNameAddress;

            //
            // Size in bytes of the buffer at
            // VariableNameAddress.
            // Used as input for GetVariable, SetVariable,
            // and GetNextVariable commands.
            // Used as output for the GetNextVariable command.
            //
            UINT32 VariableNameBytes;

            //
            // A GUID comprising the other half of the variable name.
            // Used as input for GetVariable, SetVariable, and
            // GetNextVariable commands.
            // Used as output for the GetNextVariable command.
            //
            GUID VariableVendorGuid;

            //
            // GPA of the buffer containing variable data.
            // Memory at this location is written to for the
            // GetVariable command.
            // Memory at this location is read for the SetVariable
            // command.
            //
            UINT64 VariableDataAddress;

            //
            // Size of the buffer at VariableDataAddress.
            // Used as input for the GetVariable command.
            // Used as output for the GetVariable and
            // SetVariable commands.
            //
            UINT32 VariableDataBytes;

        } VariableCommand;

        struct
        {
            //
            // Attribute mask, controls variable type for which
            // the information is returned.
            // Used as an input for the QueryInfo command.
            //
            UINT32 Attributes;

            //
            // These are outputs for the QueryInfo command.
            //
            UINT64 MaximumVariableStorage;
            UINT64 RemainingVariableStorage;
            UINT64 MaximumVariableSize;
        } QueryInfo;

        union
        {
            struct
            {
                UINT64 VsmAware : 1;
                UINT64 Unused   : 63;
            } S;
            UINT64 AsUINT64;
        }SignalRuntimeCommand;
    } U;
} NVRAM_COMMAND_DESCRIPTOR, *PNVRAM_COMMAND_DESCRIPTOR;

#pragma pack(pop)


#define CRYPT_HASH_CONTEXT_SIZE (2 * sizeof(UINT64))


typedef enum _HASH_ALG_ID
{
     HashAlgSha1,
     HashAlgSha256
} HASH_ALG_ID;


typedef enum _CRYPTO_COMMAND
{
    CryptoComputeHash,
    CryptoVerifyRsaPkcs1,
    CryptoVerifyPkcs7,
    CryptoVerifyAuthenticode,
    CryptoLogEvent,
    CryptoGetRandomNumber
} CRYPTO_COMMAND;


typedef enum _SECUREBOOT_EVENT_INFO
{
    ImageFailedVerification,
    ImageFailedVerificationUnsignedAndHashNotInDb,
    ImageFailedVerificationHashInDbx,
    ImageFailedVerificationNeitherCertNorHashInDb,
    ImageFailedVerificationCertInDbx,
    ImageFailedVerificationNotValidPeOrCoff
} SECUREBOOT_EVENT_INFO;


#pragma pack(push, 1)

typedef struct _CRYPTO_COMMAND_DESCRIPTOR
{
    CRYPTO_COMMAND    Command;
    UINT64            Status;
    union
    {
        struct
        {
            HASH_ALG_ID HashAlgorithm;
            UINT64      DataAddress;
            UINT32      DataLength;
            UINT64      ValueAddress;
            UINT32      ValueLength;
        }
        ComputeHashParams;
        struct
        {
            UINT64      RsaContextAddress;
            UINT32      RsaContextLength;
            UINT64      MessageHashAddress;
            UINT32      MessageHashLength;
            UINT64      SignatureAddress;
            UINT32      SignatureLength;
        }
        RsaPkcs1Params;
        struct
        {
            UINT64      AuthDataAddress;
            UINT32      AuthDataSize;
            UINT64      TrustedCertAddress;
            UINT32      TrustedCertSize;
            UINT64      HashOrPkcsDataAddress;
            UINT32      HashOrPkcsDataSize;
        }AuthenticodeOrPkcs7Params;
        struct
        {
            SECUREBOOT_EVENT_INFO      EventInfo;
        }
        LogEventParams;
        struct
        {
            UINT64      BufferAddress;
            UINT32      BufferSize;
        }
        GetRandomNumberParams;
    } U;
} CRYPTO_COMMAND_DESCRIPTOR, *PCRYPTO_COMMAND_DESCRIPTOR;

#pragma pack(pop)

//
// Value returned to for any watchdog register reads if the
// BIOS watchdog timer is disabled.
//
#define BIOS_WATCHDOG_NOT_ENABLED   ((UINT32)0xFFFFFFFF)

//
// Values for the BiosConfigWatchdogConfig DWORD.
// Update BIOS_WATCHDOG_VALID_CONFIG_BITS as new values are added
//
#define BIOS_WATCHDOG_CONFIGURED    0x00000001
#define BIOS_WATCHDOG_ENABLED       0x00000002
#define BIOS_WATCHDOG_ONE_SHOT      0x00000010

#define BIOS_WATCHDOG_RUNNING       (BIOS_WATCHDOG_CONFIGURED | BIOS_WATCHDOG_ENABLED)
