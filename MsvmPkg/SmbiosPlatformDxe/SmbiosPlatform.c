/*++

Copyright (c) Microsoft Corporation

Module Name:

    SmbiosPlatform.c

Abstract:

    This is the Hyper-V specific platform code that creates the SMBIOS table.

    This driver will make a best effort to add all the SMBIOS v2.4 required
    structures. Failure is not fatal and may result in some of the required
    structures to not be installed.  Most operating systems can operate
    without the table or an incomplete table.

    The driver is an "initialization driver" and will return an
    artificial error from the entry point so it doesn't stay resident.

Author:

    Larry Cleeton (lcleeton) 26-Nov-2012

--*/

#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/ConfigLib.h>
#include <Protocol/Smbios.h>
#include <IndustryStandard/SmBios24.h>
#include <IndustryStandard/Acpi.h>

#include <EfiNt.h>

//
// TODO: The release version values and release date should be automated
//       to reflect the current version and date.
//
#define MAJOR_RELEASE_VERSION 2
#define MINOR_RELEASE_VERSION 0
static CHAR8 RELEASE_VERSION_STRING[] = "Hyper-V UEFI Release v2.0";
static CHAR8 RELEASE_DATE_STRING[] = "08/26/2016";

//
// Complying with SMBIOS v2.4 specification.
//
#define TARGETTED_SMBIOS_MAJOR_VERSION 2
#define TARGETTED_SMBIOS_MINOR_VERSION 4

//
// Implementation specific constant strings.
//
static CHAR8 MANUFACTURER_STRING[]    = "Microsoft Corporation";
static CHAR8 VIRTUAL_MACHINE_STRING[] = "Virtual Machine";
static CHAR8 NONE_STRING[]            = "None";

//
// Memory device location string size including null.
// Naming convention is "MXXXX" where XXXX are hex digits.
//
#define LOCATION_STRING_SIZE 6

//
// Maximum SMBIOS memory regions to create.
// 0xFFFF is more than enough for any anticipated memory scale.
// LOCATION_STRING_SIZE above is dependent on this max.
//
static const UINT64 gMaxMemoryRegions = 0xFFFFULL;

//
// Maximum memory size per SMBIOS v2.4 memory device.
// 15 bits in megabyte units, so max 31 gigabytes per device.
//
static const UINT64 gMaxSizePerMemoryDevice = (0x7FFFULL * SIZE_1MB);

//
// Helper macro to init SMBIOS structure header.
//
#define STANDARD_HEADER(type, typeId) { typeId, sizeof(type), SMBIOS_HANDLE_PI_RESERVED }

//
// Context structure for AddMemoryRegionsFromMemoryRange function.
//
typedef struct {
    UINT64                  CurrentRegion;
    EFI_SMBIOS_PROTOCOL*    Smbios;
    EFI_SMBIOS_HANDLE       PhysicalMemoryArrayHandle;
} ADD_MEMORY_REGIONS_CONTEXT;

//
// Callback definition for EnumerateMemoryRanges function.
//
typedef
VOID
(*ENUMERATE_MEMMAP_CALLBACK)(
    VM_MEMORY_RANGE* Range,
    VOID *Context
);


VOID
NumberToMemoryLocationString(
    _In_ UINT16                                     Number,
    _Out_writes_bytes_(LOCATION_STRING_SIZE) CHAR8* Buffer
)
/*++

Routine Description:

    Utility function to create a memory device location string.
    The string is of the form "Mxxxx" where xxxx is 0000 to FFFF.

Arguments:

    Number - A number value between 0 and FFFF (65535).

    String - A pointer to a pre-allocated string buffer of at least 6 bytes.

Return Value:

    n/a

--*/
{
    static const CHAR8* hexdigits = "0123456789ABCDEF";

    *Buffer++ = 'M';
    *Buffer++ = hexdigits[(Number >> 12) & 0xF];
    *Buffer++ = hexdigits[(Number >> 8) & 0xF];
    *Buffer++ = hexdigits[(Number >> 4) & 0xF];
    *Buffer++ = hexdigits[Number & 0xF];
    *Buffer = 0;
}


BOOLEAN
AddStructure(
    _In_       EFI_SMBIOS_PROTOCOL*    Smbios,
    _In_       VOID*                   Structure,
    _In_opt_   CHAR8**                 Strings,
    _Out_opt_  EFI_SMBIOS_HANDLE*      Handle
    )
/*++

Routine Description:

    Adds a structure to the global SMBIOS table.
    Optionally assists with appending the strings.
    Optionally returns the handle to the added structure for use in other structures.

Arguments:

    Smbios - The Smbios dxe protocol.

    Structure - The base or complete structure to add.

    Strings - Optional strings that will be copied at the end of the
              Structure before adding to the SMBIOS table.
              Structure must have sufficient space beyond its Length
              for the strings.

    Handle - Optionally returns the handle of the newly added structure.

Return Value:

    TRUE on success.

--*/
{
    EFI_SMBIOS_HANDLE handle = SMBIOS_HANDLE_PI_RESERVED;
    EFI_SMBIOS_TABLE_HEADER* header = (EFI_SMBIOS_TABLE_HEADER*) Structure;
    UINT32 index;
    CHAR8* destination = (CHAR8*)Structure + header->Length;

    //
    // Optionally copy the strings to end of table.
    // If no strings supplied then caller is expected to already have
    // appended strings and structure terminator.
    //
    if (Strings != NULL)
    {
        //
        // Append each string including its terminating null byte.
        //
        for (index = 0; Strings[index] != NULL; index++)
        {
            CHAR8* source = Strings[index];

            while((*destination++ = *source++) != '\0');
        }

        //
        // Finalize structure terminator.  The last string has a null byte so
        // this addtional one results in two null bytes at the end of the structure.
        //
        *destination++ = '\0';
    }

    //
    // Add the structure to the table.
    //
    if (EFI_ERROR(Smbios->Add(Smbios, NULL, &handle, Structure)))
    {
        return FALSE;
    }

    //
    // Optionally output the structure's handle.
    //
    if (Handle != NULL)
    {
        *Handle = handle;
    }

    return TRUE;
}


VOID
AddBiosInformation(
    _In_ EFI_SMBIOS_PROTOCOL* Smbios
    )
/*++

Routine Description:

    Adds the BIOS Information structure (type 0) to the SMBIOS table.

Arguments:

    Smbios - The smbios dxe protocol.

Return Value:

    n/a

--*/
{
    //
    // Initialized array of pointers to the strings for this structure.
    //
    static CHAR8* strings[] =
    {
        MANUFACTURER_STRING,
        RELEASE_VERSION_STRING,
        RELEASE_DATE_STRING,
        (CHAR8*)NULL
    };

    //
    // Initialized structure with string space appended and extra terminator byte.
    //
    static struct
    {
        SMBIOS_TABLE_TYPE0 Formatted;
        CHAR8 Unformed[sizeof(MANUFACTURER_STRING) +
                       sizeof(RELEASE_VERSION_STRING) +
                       sizeof(RELEASE_DATE_STRING) +
                       1];
    } biosInformation =

    {
        {
            STANDARD_HEADER(SMBIOS_TABLE_TYPE0, EFI_SMBIOS_TYPE_BIOS_INFORMATION),
            1,  // Vendor string index
            2,  // BIOS Version string index
            0,  // BIOS Starting Address Segment (meaningless for UEFI)
            3,  // BIOS Release Date string index
            0,  // BIOS ROM size (meaningless for UEFI)

            //
            // BiosCharacteristics
            //
            {
                0,  // Reserved (2 bits)
                0,  // Unknown
                1,  // BiosCharacteristicsNotSupported - yes
                0,  // IsaIsSupported - no
                0,  // McaIsSupported - no
                0,  // EisaIsSupported - no
                0,  // PciIsSupported - no
                0,  // PcmciaIsSupported - no
                1,  // PlugAndPlayIsSupported - yes
                0,  // ApmIsSupported - no
                0,  // BiosIsUpgradable - no
                0,  // BiosShadowingAllowed - no
                0,  // VlVesaIsSupported - no
                0,  // EscdSupportIsAvailable - no
                1,  // BootFromCdIsSupported - yes
                1,  // SelectableBootIsSupported - yes
                0,  // RomBiosIsSocketed - no
                0,  // BootFromPcmciaIsSupported - no
                1,  // EDDSpecificationIsSupported - no
                0,  // JapaneseNecFloppyIsSupported - no
                0,  // JapaneseToshibaFloppyIsSupported - no
                0,  // Floppy525_360IsSupported - no
                0,  // Floppy525_12IsSupported - no
                0,  // Floppy35_720IsSupported - no
                0,  // Floppy35_288IsSupported - no
                0,  // Floppy35_720IsSupported - no
                0,  // PrintScreenIsSupported - no
                1,  // SerialIsSupported - yes
                0,  // PrinterIsSupported - no
                0,  // CgaMonoIsSupported - no
                0,  // NecPc98 - no
                0,  // ReservedForVendor (32 bits)
            },

            //
            // BIOSCharacteristicsExtensionBytes
            //
            {
                1, // AcpiIsSupported
                4, // TargetContentDistributionEnabled
            },

            MAJOR_RELEASE_VERSION,
            MINOR_RELEASE_VERSION,
            0xFF, // no field upgradeable embedded controller firmware
            0xFF, // no field upgradeable embedded controller firmware
        }
    };

    //
    // Add the structure to the SMBIOS table. Error is not fatal and ignored.
    //
    (VOID)AddStructure(Smbios, &biosInformation, strings, NULL);
}


VOID
AddSystemInformation(
    _In_ EFI_SMBIOS_PROTOCOL* Smbios
    )
/*++

Routine Description:

    Adds the System Information structure (type 1) to the SMBIOS table.

Arguments:

    Smbios - The smbios dxe protocol.

Return Value:

    n/a

--*/
{
    //
    // Initialized array of pointers to the strings for this structure.
    //
    static CHAR8* strings[] =
    {
        MANUFACTURER_STRING,
        VIRTUAL_MACHINE_STRING,
        RELEASE_VERSION_STRING,
        "",
        NONE_STRING,
        VIRTUAL_MACHINE_STRING,
        NULL
    };

    //
    // Initialized structure with string space appended and extra terminator byte.
    //
    static struct
    {
        SMBIOS_TABLE_TYPE1 Formatted;
        CHAR8 Unformed[sizeof(MANUFACTURER_STRING) +
                       sizeof(VIRTUAL_MACHINE_STRING) +
                       sizeof(RELEASE_VERSION_STRING) +
                       ConfigLibSmbiosStringMax + 1 +
                       sizeof(NONE_STRING) +
                       sizeof(VIRTUAL_MACHINE_STRING) +
                       1];
    } systemInformation =

    {
        {
            STANDARD_HEADER(SMBIOS_TABLE_TYPE1, EFI_SMBIOS_TYPE_SYSTEM_INFORMATION),
            1, // Manufacturer string index
            2, // Product Name string index
            3, // Version string index
            4, // Serial Number string index
            { 0 },
            SystemWakeupTypePowerSwitch,
            5, // SKU Number string index
            6  // Family string index
        }
    };

    //
    // Add the dynamic information to the structure.
    //
    strings[3] = GetSmbiosSystemSerialNumberString();
    CopyMem(&systemInformation.Formatted.Uuid, GetBiosGuid(), sizeof(EFI_GUID));

    //
    // Add the structure to the SMBIOS table. Error is not fatal and ignored.
    //
    (VOID)AddStructure(Smbios, &systemInformation, strings, NULL);
}


BOOLEAN
AddSystemEnclosure(
    _In_  EFI_SMBIOS_PROTOCOL* Smbios,
    _Out_ EFI_SMBIOS_HANDLE*   ChassisHandle
    )
/*++

Routine Description:

    Adds the System Enclosure structure (type 3) to the SMBIOS table.

Arguments:

    Smbios - The smbios dxe protocol.

    ChassisHandle - Returns the handle of the newly added structure.

Return Value:

    TRUE on success.

--*/
{
    //
    // Initialized array of pointers to the strings for this structure.
    //
    static CHAR8* strings[] =
    {
        MANUFACTURER_STRING,
        RELEASE_VERSION_STRING,
        "",
        "",
        NONE_STRING,
        NULL
    };

    //
    // Initialized structure with string space appended and extra terminator byte.
    //
    static struct
    {
        SMBIOS_TABLE_TYPE3 Formatted;
        CHAR8 Unformed[sizeof(MANUFACTURER_STRING) +
                       sizeof(RELEASE_VERSION_STRING) +
                       ConfigLibSmbiosStringMax + 1 +
                       ConfigLibSmbiosStringMax + 1 +
                       sizeof(NONE_STRING) +
                       2];
    } systemEnclosure =

    {
        {
            STANDARD_HEADER(SMBIOS_TABLE_TYPE3, EFI_SMBIOS_TYPE_SYSTEM_ENCLOSURE),
            1, // Manufacturer string index
            MiscChassisTypeDeskTop,
            2, // Version string index
            3, // Serial Number string index
            4, // Asset Tag Number string index
            ChassisStateSafe, // Boot-up State
            ChassisStateSafe, // Power Supply State
            ChassisStateSafe, // Thermal State
            ChassisSecurityStatusUnknown, // Security Status
            { 0 }, // OEM-defined
            0, // Height
            0, // Number of Power Cords
            0, // Contained Element Count
            0, // Contained Element Record Count
            { 5 } // Contained Element dummy string index
        }
    };

    //
    // Add the dynamic information to the structure.
    //
    strings[2] = GetSmbiosChassisSerialNumberString();
    strings[3] = GetSmbiosChassisAssetTagString();

    //
    // Add the structure to the SMBIOS table.
    //
    return AddStructure(Smbios, &systemEnclosure, strings, ChassisHandle);
}


VOID
AddBaseboardInformation(
    _In_ EFI_SMBIOS_PROTOCOL* Smbios,
    _In_ EFI_SMBIOS_HANDLE    ChassisHandle
    )
/*++

Routine Description:

    Adds the Baseboard Information structure (type 2) to the SMBIOS table.

Arguments:

    Smbios - The smbios dxe protocol.

    BiosConfigPage - Contains system information provided by the BiosDevice.

    ChassisHandle - The handle of SystemEnclosure structure in which this
        baseboard resides.

Return Value:

    n/a

--*/
{
    //
    // Initialized array of pointers to the strings for this structure.
    //
    static CHAR8* strings[] =
    {
        MANUFACTURER_STRING,
        VIRTUAL_MACHINE_STRING,
        RELEASE_VERSION_STRING,
        "",
        NONE_STRING,
        VIRTUAL_MACHINE_STRING,
        NULL
    };

    //
    // Initialized structure with string space appended and extra terminator byte.
    //
    static struct
    {
        SMBIOS_TABLE_TYPE2 Formatted;
        CHAR8 Unformed[sizeof(MANUFACTURER_STRING) +
                       sizeof(VIRTUAL_MACHINE_STRING) +
                       sizeof(RELEASE_VERSION_STRING) +
                       ConfigLibSmbiosStringMax + 1 +
                       sizeof(NONE_STRING) +
                       sizeof(VIRTUAL_MACHINE_STRING) +
                       2];
    } baseboardInformation =

    {
        {
            STANDARD_HEADER(SMBIOS_TABLE_TYPE2, EFI_SMBIOS_TYPE_BASEBOARD_INFORMATION),
            1, // Manufacturer string index
            2, // Product string index
            3, // Version string index
            4, // Serial Number string index
            5, // Asset Tag string index
            {
                1   // Feature Flags - Motherboard
            },
            6, // Location in Chassis string index
            SMBIOS_HANDLE_PI_RESERVED, // Chassis Handle
            BaseBoardTypeMotherBoard, // Board Type
            0, // Number of Contained Object Handles
            SMBIOS_HANDLE_PI_RESERVED, // Contained Object Handles
        }
    };

    //
    // Add the dynamic information to the structure.
    //
    baseboardInformation.Formatted.ChassisHandle = ChassisHandle;

    strings[3] = GetSmbiosBaseSerialNumberString();

    //
    // Add the structure to the SMBIOS table. Error is not fatal and ignored.
    //
    (VOID)AddStructure(Smbios, &baseboardInformation, strings, NULL);
}


VOID
AddProcessorInformation(
    _In_ EFI_SMBIOS_PROTOCOL* Smbios
    )
/*++

Routine Description:

    Adds the ProcessorInformation structures (type 4) to the SMBIOS table.
    One per virtual processor.

Arguments:

    Smbios - The smbios dxe protocol.

    BiosConfigPage - Contains system information provided by the BiosDevice.

Return Value:

   n/a

--*/
{
    UINT32 i;
    UINT32 procCount = GetProcessorCount();
    VOID *cpuInfo = GetSmbiosV24CpuInfo();

    //
    // This particular structure is constructed by the BIOS VDev and
    // provided in the config page.  Simply add 1 per configured processor.
    //
    for (i = 0; i < procCount; i++)
    {
        //
        // Add the structure to the SMBIOS table. Error is not fatal and ignored.
        //
        (VOID)AddStructure(Smbios, cpuInfo, NULL, NULL);
    }
}


VOID
AddOEMStrings(
    _In_ EFI_SMBIOS_PROTOCOL* Smbios
    )
/*++

Routine Description:

    Adds the OEM Strings structure (type 11) to the SMBIOS table.

Arguments:

    Smbios - The smbios dxe protocol.

    BiosConfigPage - Contains system information provided by the BiosDevice.

Return Value:

    n/a

--*/
{
    //
    // Prefix and Postfix strings for the BIOS lock string.
    //
    static CHAR8 OEM_STRING_1[] = "[MS_VM_CERT/SHA1/9b80ca0d5dd061ec9da4e494f4c3fd1196270c22]";
    static CHAR8 OEM_STRING_3[] = "To be filled by OEM";

    //
    // Initialized array of pointers to the strings for this structure.
    //
    static CHAR8* strings[] =
    {
        OEM_STRING_1,
        "",
        OEM_STRING_3,
        NULL
    };

    //
    // Initialized structure with string space appended and extra terminator byte.
    //
    static struct
    {
        SMBIOS_TABLE_TYPE11 Formatted;
        CHAR8 Unformed[sizeof(OEM_STRING_1) +
                       ConfigLibSmbiosStringMax + 1 +
                       sizeof(OEM_STRING_3) +
                       1];
    } oemStrings =

    {
        {
            STANDARD_HEADER(SMBIOS_TABLE_TYPE11, EFI_SMBIOS_TYPE_OEM_STRINGS),
            3 // string count
        }
    };

    //
    // Add the dynamic information to the structure.
    //
    strings[1] = GetSmbiosOemBiosLockString();

    //
    // Add the structure to the SMBIOS table. Error is not fatal and ignored.
    //
    (VOID)AddStructure(Smbios, &oemStrings, strings, NULL);
}


BOOLEAN
AddPhysicalMemoryArray(
    _In_ EFI_SMBIOS_PROTOCOL* Smbios,
    _In_ EFI_SMBIOS_HANDLE    MemoryErrorHandle,
    _Out_ EFI_SMBIOS_HANDLE*  PhysicalMemoryArrayHandle,
    _In_ UINT16               PhysicalMemoryArraySize
    )
/*++

Routine Description:

    Adds a Physical Memory Array structure (type 16) to the SMBIOS table SMBIOS table.

Arguments:

    Smbios - The smbios dxe protocol.

    MemoryErrorHandle - The handle of the error information structure for this
        array.

    PhysicalMemoryArrayHandle - Returns the handle of the newly added structure.

    PhysicalMemoryArraySize - The size of the Physical Memory Array.

Return Value:

    TRUE on success.

--*/
{
    //
    // Initialized structure with extra terminator bytes (no strings).
    //
    static struct
    {
        SMBIOS_TABLE_TYPE16 Formatted;
        CHAR8 Unformed[2];
    } physicalMemoryArray =

    {
        {
            STANDARD_HEADER(SMBIOS_TABLE_TYPE16, EFI_SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY),
            MemoryArrayLocationSystemBoard, // Location
            MemoryArrayUseSystemMemory, // Use
            MemoryErrorCorrectionNone, // Memory Error Correction
            0x80000000, // Maximum Capacity - unknown
            SMBIOS_HANDLE_PI_RESERVED, // Memory Error Information Handle
            0, // Number of Memory Devices
        },
        {
            0, // terminator bytes
            0
        }
    };

    //
    // Add the dynamic information to the structure.
    //
    physicalMemoryArray.Formatted.MemoryErrorInformationHandle = MemoryErrorHandle;
    physicalMemoryArray.Formatted.NumberOfMemoryDevices = PhysicalMemoryArraySize;

    //
    // Add the structure to the SMBIOS table.
    //
    return AddStructure(Smbios, &physicalMemoryArray, NULL, PhysicalMemoryArrayHandle);
}


BOOLEAN
AddMemoryArrayMappedAddress(
    _In_ EFI_SMBIOS_PROTOCOL* Smbios,
    _In_ UINT64               BaseAddress,
    _In_ UINT64               Size,
    _In_ EFI_SMBIOS_HANDLE    PhysicalMemoryArrayHandle,
    __out EFI_SMBIOS_HANDLE*  MemoryArrayMappedAddressHandle
    )
/*++

Routine Description:

    Adds a Memory Array Mapped Address structure (type 19) to the SMBIOS table.

Arguments:

    Smbios - The smbios dxe protocol.

    BaseAddress - The address where this memory array is mapped.

    Size - The amount of memory mapped (in bytes).

    PhysicalMemoryArrayHandle - The handle of the PhysicalMemoryArray structure for
        this mapping.

    MemoryArrayMappedAddressHandle - Returns the handle of the newly added structure.

Return Value:

    TRUE on success.

--*/
{
    //
    // Initialized structure with extra terminator bytes (no strings).
    //
    static struct
    {
        SMBIOS_TABLE_TYPE19 Formatted;
        CHAR8 Unformed[2];
    } memoryArrayMappedAddress =

    {
        {
            STANDARD_HEADER(SMBIOS_TABLE_TYPE19, EFI_SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS),
            0, // Starting Address
            0, // Ending Address
            SMBIOS_HANDLE_PI_RESERVED, // Memory Array Handle
            0  // Partition Width
        },
        {
            0, // terminator bytes
            0
        }
    };

    UINT64 endAddress = BaseAddress + Size;
    UINT64 endAddressInKb = (endAddress / 1024) + ((endAddress % 1024) > 0);
    UINT64 baseAddressInKb = BaseAddress / 1024;
    BOOLEAN result = FALSE;

    //
    // The v2.4 type 19 structure only supports 32 bit addresses specified
    // in kilobyte units.  This means we can only declare memory up to
    // 1K below 4 terabytes. Return a failure if the addresses are too big.
    //
    if ((BaseAddress > (BASE_4TB - SIZE_1KB)) || endAddress > (BASE_4TB - SIZE_1KB))
    {
        result = FALSE;
        goto Exit;
    }

    //
    // Add the dynamic information to the structure.
    //
    memoryArrayMappedAddress.Formatted.StartingAddress = (UINT32) baseAddressInKb;
    memoryArrayMappedAddress.Formatted.EndingAddress = (UINT32) endAddressInKb;
    memoryArrayMappedAddress.Formatted.MemoryArrayHandle = PhysicalMemoryArrayHandle;

    //
    // Add the structure to the SMBIOS table.
    //
    result = AddStructure(Smbios, &memoryArrayMappedAddress, NULL, MemoryArrayMappedAddressHandle);

Exit:

    return result;
}


BOOLEAN
AddMemoryDevice(
    _In_ EFI_SMBIOS_PROTOCOL* Smbios,
    _In_ UINT64               Size,
    _In_ EFI_SMBIOS_HANDLE    PhysicalMemoryArrayHandle,
    _In_ EFI_SMBIOS_HANDLE    MemoryErrorHandle,
    _In_ CHAR8*               LocationString,
    _Out_ EFI_SMBIOS_HANDLE*  MemoryDeviceHandle
    )
/*++

Routine Description:

    Adds a MemoryDevice structure to the SMBIOS table.

Arguments:

    Smbios - The smbios dxe protocol.

    Size - The amount of memory in the device (in bytes).

    PhysicalMemoryArrayHandle - The handle of the PhysicalMemoryArray structure
        for this device.

    MemoryErrorHandle - The handle of the error information structure for this
        device.

    LocationString - String which describes the "slot" where this memory device
        is located.

    MemoryDeviceHandle - Returns the handle of the newly added structure.

Return Value:

    TRUE on success.

--*/
{
    //
    // Initialized array of pointers to the strings for this structure.
    //
    static CHAR8* strings[] =
    {
        "",
        NONE_STRING,
        MANUFACTURER_STRING,
        NONE_STRING,
        NONE_STRING,
        NONE_STRING,
        NULL
    };

    //
    // Initialized structure with string space appended and extra terminator byte.
    //
    static struct
    {
        SMBIOS_TABLE_TYPE17 Formatted;
        CHAR8 Unformed[LOCATION_STRING_SIZE +
                       sizeof(NONE_STRING) +
                       sizeof(MANUFACTURER_STRING) +
                       sizeof(NONE_STRING) +
                       sizeof(NONE_STRING) +
                       sizeof(NONE_STRING) +
                       1];
    } memoryDevice =

    {
        {
            STANDARD_HEADER(SMBIOS_TABLE_TYPE17, EFI_SMBIOS_TYPE_MEMORY_DEVICE),
            SMBIOS_HANDLE_PI_RESERVED, // Physical Memory Array Handle
            SMBIOS_HANDLE_PI_RESERVED, // Memory Error Information Handle
            0xffff, // Total Width - unknown
            0xffff, // Data Width - unknown
            0xffff, // Size - unknown
            MemoryFormFactorUnknown, // Form Factor - unknown
            0, // Device Set - not part of a set
            1, // Device Locator string index
            2, // Bank Locator string index
            MemoryTypeUnknown, // Memory Type - unknown
            { // Type Detail
                0,  // Reserved
                0,  // Other
                1,  // Unknown
            },
            0, // Speed - unknown
            3, // Manufacturer string index
            4, // Serial Number string index
            5, // Asset Tag string index
            6  // Part Number string index
        }
    };

    BOOLEAN result = FALSE;

    UINT64 sizeInKB = (Size / 1024) + ((Size % 1024) > 0);

    if (sizeInKB <= 0x7fff)
    {
        memoryDevice.Formatted.Size = (UINT16) sizeInKB | 0x8000;
    }
    else
    {
        UINT64 sizeInMB = (sizeInKB / 1024) + ((sizeInKB % 1024) > 0);

        if (sizeInMB <= 0x7fff)
        {
            memoryDevice.Formatted.Size = (UINT16) sizeInMB;
        }
        else
        {
            //
            // Set size to unknown if too big for field.
            //
            memoryDevice.Formatted.Size = (UINT16) 0xffff;
        }
    }

    //
    // Add the dynamic information to the structure.
    //
    memoryDevice.Formatted.MemoryArrayHandle = PhysicalMemoryArrayHandle;
    memoryDevice.Formatted.MemoryErrorInformationHandle = MemoryErrorHandle;
    strings[0] = LocationString;

    //
    // Add the structure to the SMBIOS table.
    //
    result = AddStructure(Smbios, &memoryDevice, strings, MemoryDeviceHandle);

    return result;
}


VOID
AddMemoryDeviceMappedAddress(
    _In_ EFI_SMBIOS_PROTOCOL* Smbios,
    _In_ UINT64               BaseAddress,
    _In_ UINT64               Size,
    _In_ EFI_SMBIOS_HANDLE    MemoryDeviceHandle,
    _In_ EFI_SMBIOS_HANDLE    MemoryArrayMappedAddressHandle
    )
/*++

Routine Description:

    Adds a MemoryDeviceMappedAddress structure to the SMBIOS table.

Arguments:

    Smbios - The smbios dxe protocol.

    BaseAddress - The address where this memory array is mapped.

    Size - The amount of memory mapped (in bytes).

    MemoryDeviceHandle - The handle of the MemoryDevice structure to which this
        mapping applies.

    MemoryArrayMappedAddressHandle - The handle of the address mapping
        structure for the array in which this mapping's device resides.

Return Value:

    n/a

--*/
{
    //
    // Initialized structure with terminator bytes.
    //
    static struct
    {
        SMBIOS_TABLE_TYPE20 Formatted;
        CHAR8 Unformed[2];
    } memoryDeviceMappedAddress =

    {
        {
            STANDARD_HEADER(SMBIOS_TABLE_TYPE20, EFI_SMBIOS_TYPE_MEMORY_DEVICE_MAPPED_ADDRESS),
            0, // Starting Address
            0, // Ending Address
            SMBIOS_HANDLE_PI_RESERVED, // Memory Device Handle
            SMBIOS_HANDLE_PI_RESERVED, // Memory Array Mapped Address Handle
            0xff, // Partition Row Position - unknown
            0, // Interleave Position - not interleaved
            0, // Interleaved Data Depth - not part of interleave
        },
        {
            0, // terminator bytes
            0
        }
    };

    UINT64 endAddress = BaseAddress + Size;
    UINT64 endAddressInKb = (endAddress / 1024) + ((endAddress % 1024) > 0);
    UINT64 baseAddressInKb = BaseAddress / 1024;

    //
    // The v2.4 type 20 structure only supports 32 bit addresses specified
    // in kilobyte units.  This means we can only declare memory up to
    // 1K below 4 terabytes. Just don't add structure if memory is too big.
    //
    if ((BaseAddress < (BASE_4TB - SIZE_1KB)) || endAddress < (BASE_4TB - SIZE_1KB))
    {
        //
        // Add the dynamic information to the structure.
        //
        memoryDeviceMappedAddress.Formatted.StartingAddress = (UINT32) baseAddressInKb;
        memoryDeviceMappedAddress.Formatted.EndingAddress = (UINT32) endAddressInKb;
        memoryDeviceMappedAddress.Formatted.MemoryDeviceHandle = MemoryDeviceHandle;
        memoryDeviceMappedAddress.Formatted.MemoryArrayMappedAddressHandle =
            MemoryArrayMappedAddressHandle;

        //
        // Add the structure to the SMBIOS table. Error not fatal and ignored.
        //
        (VOID)AddStructure(Smbios, &memoryDeviceMappedAddress, NULL, NULL);
    }
}


VOID
AddSystemBootInformation(
    _In_ EFI_SMBIOS_PROTOCOL* Smbios
    )
/*++

Routine Description:

    Adds the SystemBootInformation structure to the SMBIOS table.

Arguments:

    Smbios - The smbios dxe protocol.

Return Value:

    n/a

--*/
{
    //
    // Initialized structure with terminator bytes.
    //
    static struct
    {
        SMBIOS_TABLE_TYPE32 Formatted;
        CHAR8 Unformed[2];
    } systemBootInformation =

    {
        {
            STANDARD_HEADER(SMBIOS_TABLE_TYPE32, EFI_SMBIOS_TYPE_SYSTEM_BOOT_INFORMATION),
            { 0 }, // Reserved
            { BootInformationStatusNoError } // Boot Status
        },
        {
            0, // terminator bytes
            0
        }
    };

    //
    // Add the structure to the SMBIOS table. Error not fatal and ignored.
    //
    (VOID)AddStructure(Smbios, &systemBootInformation, NULL, NULL);
}


VOID
AddMemoryRegion(
    _In_ EFI_SMBIOS_PROTOCOL* Smbios,
    _In_ UINT64 BaseAddress,
    _In_ UINT64 Length,
    _In_ CHAR8* LocationString,
    _In_ EFI_SMBIOS_HANDLE PhyscialMemoryArrayHandle,
    _In_ EFI_SMBIOS_HANDLE MemoryErrorHandle
    )
/*++

Routine Description:

    Adds three memory device/region related structures to the SMBIOS table for a memory region.
    Only adds a zero length Memory Device structure if Length specified as zero.

    Memory Device                (type 17)
    Memory Array Mapped Address  (type 19)
    Memory Device Mapped Address (type 20)

Arguments:

    Smbios - The smbios dxe protocol.

    BaseAddress - the base physical address where the memory device is mapped.

    Length - the length of the memory device.

    LocationString - the identifying string for the device.

    PhysicalMemoryArrayHandle - the handle of the existing Physical Memory Array structure.

    MemoryErrorHandle

Return Value:

    n/a

--*/
{
    EFI_SMBIOS_HANDLE memoryDeviceHandle;
    EFI_SMBIOS_HANDLE memoryArrayMappedAddressHandle;

    //
    // Add Memory Device structure.
    //
    if (AddMemoryDevice(Smbios,
                        Length,
                        PhyscialMemoryArrayHandle,
                        MemoryErrorHandle,
                        LocationString,
                        &memoryDeviceHandle))
    {
        //
        // If length of memory device is zero then done.
        //
        if (Length > 0)
        {
            //
            // Add additional structures for memory.
            //
            if (AddMemoryArrayMappedAddress(Smbios,
                                            BaseAddress,
                                            Length,
                                            PhyscialMemoryArrayHandle,
                                            &memoryArrayMappedAddressHandle))
            {
                AddMemoryDeviceMappedAddress(Smbios,
                                             BaseAddress,
                                             Length,
                                             memoryDeviceHandle,
                                             memoryArrayMappedAddressHandle);
            }
        }
    }
}


VOID
AccumulateMemoryRegionsFromMemoryRange(
    VM_MEMORY_RANGE *Range,
    VOID *Context
    )
/*++

Routine Description:

    Callback function for EnumerateMemoryRanges that counts the number
    of SMBIOS Memory regions required to represent a memory range.

Arguments:

    Range - A pointer to an memory range.

    Context - The context pointer.  Expected to be a UINT64*
              pointer in which to accumulate the regions.

Return Value:

    n/a

--*/
{
    UINT64 *numMemoryRegions = (UINT64 *)Context;

    //
    // Compute the number of SMBIOS Memory regions that will represent
    // the size expressed by the memory map range structure.
    //
    *numMemoryRegions += ((Range->Length + gMaxSizePerMemoryDevice - 1) / gMaxSizePerMemoryDevice);
}


VOID
AddMemoryRegionsFromMemoryRange(
    VM_MEMORY_RANGE *Range,
    VOID *Context
    )
/*++

Routine Description:

    Callback function for EnumerateMemoryRanges to add one or more
    SMBIOS Memory Regions to represent a memory range.

Arguments:

    Range - A pointer to a memory range.

    Context - The context pointer.

Return Value:

    n/a

--*/
{
    ADD_MEMORY_REGIONS_CONTEXT* context = (ADD_MEMORY_REGIONS_CONTEXT*)Context;
    UINT64 base = Range->BaseAddress;
    UINT64 size = Range->Length;
    CHAR8 location[LOCATION_STRING_SIZE];

    //
    // Add memory regions until this memory map entry (range) is consumed or
    // the maximum number of SMBIOS memory regions is reached.
    //
    while ((context->CurrentRegion < gMaxMemoryRegions) && (size > 0))
    {
        context->CurrentRegion++;
        NumberToMemoryLocationString((UINT16)context->CurrentRegion, location);
        AddMemoryRegion(
            context->Smbios,
            base,
            MIN(size, gMaxSizePerMemoryDevice),
            location,
            context->PhysicalMemoryArrayHandle,
            SMBIOS_HANDLE_PI_RESERVED);
        size -= MIN(size, gMaxSizePerMemoryDevice);
        base += MIN(size, gMaxSizePerMemoryDevice);
    }
}


VOID
EnumerateMemoryRanges(
    VM_MEMORY_RANGE             *Memmap,
    UINT32                      MemmapLength,
    ENUMERATE_MEMMAP_CALLBACK   Callback,
    VOID *                      Context
    )
/*++

Routine Description:

    Utility function to enumerate all the memory ranges in the memory map.
    Calls the passed in callback function for each range.

Arguments:

    Memmap - A pointer to the memory map.

    MemmapLength - The numer of entries (ranges) in the memory map.

    Callback - The function to call with each enumerated entry (range).

    Context - The context pointer to pass to the Callback function.

Return Value:

    n/a

--*/
{
    VM_MEMORY_RANGE *cursor;

    for (cursor = Memmap; cursor < (Memmap + MemmapLength); cursor++)
    {
        Callback(cursor, Context);
    }
}


VOID
AddMemoryStructures(
    _In_ EFI_SMBIOS_PROTOCOL* Smbios
)
/*++

Routine Description:

    Adds all the memory related structures to the SMBIOS table.

    Physical Memory Array        (type 16)
    Memory Device                (type 17)
    Memory Array Mapped Address  (type 19)
    Memory Device Mapped Address (type 20)

    The memory structures on a physical machine typically represent the
    physical memory devices/modules installed.  In a virtual machine this can
    only be simulated.  The most accurate simulation is to create a memory
    device for each non-hot-add region expressed in the SRAT.

Arguments:

    Smbios - The DXE Smbios protocol.

Return Value:

    n/a

--*/
{
    ADD_MEMORY_REGIONS_CONTEXT context;
    VM_MEMORY_RANGE* memmap;
    UINT32 memmapLength;
    UINT64 regions;

    memmap = (VM_MEMORY_RANGE *)GetMemmap();
    memmapLength = GetMemmapSize() / (UINT32)sizeof(VM_MEMORY_RANGE);

    //
    // Calculate the number of SMBIOS memory regions required to represent
    // starting RAM in the machine. This requires a first pass through the
    // memmap entries.
    //
    regions = 0;
    EnumerateMemoryRanges(
        memmap,
        memmapLength,
        AccumulateMemoryRegionsFromMemoryRange,
        (VOID *)&regions);

    //
    // Limit the SMBIOS memory regions to this implementation's maximum.
    //
    if (regions > gMaxMemoryRegions)
    {
        regions = gMaxMemoryRegions;
    }

    //
    // Add the single SMBIOS Physical Memory Array structure (type 16)
    // using the count of require regions from above.
    //
    if (AddPhysicalMemoryArray(
            Smbios,
            SMBIOS_HANDLE_PI_RESERVED,
            &context.PhysicalMemoryArrayHandle,
            (UINT16)regions))
    {
        //
        // Enumerate the memory regions again and add one or more
        // SMBIOS memory regions represent each entry.
        //
        context.Smbios = Smbios;
        context.CurrentRegion = 0;
        EnumerateMemoryRanges(
            memmap,
            memmapLength,
            AddMemoryRegionsFromMemoryRange,
            &context);
    }
}


VOID
AddAllStructures(
    _In_ EFI_SMBIOS_PROTOCOL* Smbios
    )
/*++

Routine Description:

    Adds all the SMBIOS structures to the SMBIOS table.

Arguments:

    Smbios - The smbios dxe protocol.

Return Value:

    n/a

--*/
{
    EFI_SMBIOS_HANDLE chassisHandle;

    AddBiosInformation(Smbios);
    AddSystemInformation(Smbios);
    if (AddSystemEnclosure(Smbios, &chassisHandle))
    {
       AddBaseboardInformation(Smbios, chassisHandle);
    }
    AddProcessorInformation(Smbios);
    AddOEMStrings(Smbios);
    AddMemoryStructures(Smbios);
    AddSystemBootInformation(Smbios);
}


EFI_STATUS
EFIAPI
SmbiosPlatformEntryPoint(
    _In_ EFI_HANDLE        ImageHandle,
    _In_ EFI_SYSTEM_TABLE* SystemTable
    )
/*++

Routine Description:

    Entrypoint of platform SMBIOS driver.

    This entry point adds the SMBIOS structures and returns an error
    so it is immediately unloaded.

Arguments:

    ImageHandle - Driver Image Handle.

    SystemTable - EFI System Table.

Return Value:

    An appropriate EFI_STATUS value.

--*/
{
    EFI_SMBIOS_PROTOCOL* smbios;
    //
    // Get the DXE Smbios protocol to use for adding structures.
    //
    if (EFI_ERROR(gBS->LocateProtocol(&gEfiSmbiosProtocolGuid, NULL, (VOID**) &smbios)))
    {
        return EFI_PROTOCOL_ERROR;
    }

    //
    // Check if verion matches.
    //
    if ((smbios->MajorVersion != TARGETTED_SMBIOS_MAJOR_VERSION) ||
        (smbios->MinorVersion != TARGETTED_SMBIOS_MINOR_VERSION))
    {
        return EFI_INCOMPATIBLE_VERSION;
    }

    //
    // Add all the structures.
    //
    AddAllStructures(smbios);

    //
    // Return success and leave this driver resident even though it is unnecessary.
    // There is currently no graceful way for drivers to exit with success and not stay loaded.
    // A driver failure can confuse anyone debugging the firmware. Since this is a boot services
    // driver the memory will be reclaimed by the OS.
    //
    return EFI_SUCCESS;
}

