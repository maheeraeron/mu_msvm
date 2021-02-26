/*++

Copyright (c) Microsoft Corporation

Module Name:

    Config.c

Abstract:

    Gets configuration values and exports them as globals and PCDs.

--*/

#include <PiPei.h>
#include <EfiNt.h>
#include <Platform.h>
#include <BiosInterface.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/Tpm20.h>
#if defined(MDE_CPU_AARCH64)
#include <Library/ArmLib.h>
#endif
#include <Library/CrashDumpAgentLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/ResourcePublicationLib.h>
#include <Ppi/ConfigPpi.h>
#include <IsolationTypes.h>
#include "Config.h"

VOID
TripleFault(
    __in    UINTN   Rax,
    __in    UINTN   Rbx,
    __in    UINTN   Rcx,
    __in    UINTN   Rdx
);

//
// Values and type used with CPUID to get the physical address width.
//
#define CPUID_FUNCTION_EXTENDED_MAX_FUNCTION        0x80000000
#define CPUID_FUNCTION_EXTENDED_ADDRESS_SPACE_SIZES 0x80000008

#define CONFIG 0x434f4e464947 // "CONFIG"

#if defined(MDE_CPU_AARCH64)
#define CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(Info1, Info2) \
    { ASSERT(FALSE); CpuDeadLoop(); }
#elif defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)
#define CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(Info1, Info2) \
    TripleFault(KERNEL_SECURITY_CHECK_FAILURE, FAST_FAIL_UNEXPECTED_HOST_BEHAVIOR, Info1, Info2);
#else
#error Unsupported Architecture
#endif

typedef union _CPUID_ADDRESS_SPACE_SIZES
{
    struct
    {
        UINT8 PhysicalAddressBits;
        UINT8 VirtualAddressBits;
        UINT16 Reserved;
    };

    UINT32 Value;
} CPUID_ADDRESS_SPACE_SIZES;

UINT8
GetPhysicalAddressWidth(
    _In_ CONST EFI_PEI_SERVICES**  PeiServices
    )
/*++

Routine Description:

    Gets the number of bits in the CPU address width.

Arguments:

    PeiServices - An indirect pointer to the PEI Services Table.

Return Value:

    The number of bits in the address width.

--*/
{
    UINT8 physicalAddressWidth = 0;
    static const UINT8 MinimumAddressWidth = 36;
    static const UINT8 MaximumAddressWidth = 48;

#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)

    UINT32 maximumFunction;
    CPUID_ADDRESS_SPACE_SIZES addressSpaceSizes;

    //
    // Query with CPUID
    //
    AsmCpuid(CPUID_FUNCTION_EXTENDED_MAX_FUNCTION, &maximumFunction, NULL, NULL, NULL);
    if (maximumFunction >= CPUID_FUNCTION_EXTENDED_ADDRESS_SPACE_SIZES)
    {
        AsmCpuid(CPUID_FUNCTION_EXTENDED_ADDRESS_SPACE_SIZES,
                 &addressSpaceSizes.Value,
                 NULL,
                 NULL,
                 NULL);

        physicalAddressWidth = addressSpaceSizes.PhysicalAddressBits;
    }
    else
    {
        // It is highly unlikely that the CPUID leaf doesn't exist.
        // Regardless just use the minimum as the default.
        DEBUG((DEBUG_WARN, "Can't query CPUID so defaulting address width to %u bits\n",
            MinimumAddressWidth));
        physicalAddressWidth = MinimumAddressWidth;
    }

#elif defined(MDE_CPU_AARCH64)

    // Read system register ID_AA64MMFR0_EL1
    // ID_AA64MMFR0_EL1.PARange is bits[3:0]
    // Valid values for ARMv8.1 PARange are 0 thru 6 which mean the following address widths.
    static UINT8 aw[7] = { 32, 36, 40, 42, 44, 48, 52 };
    UINT64 regValue = (UINT64)ArmReadIdMmfr0();
    DEBUG((DEBUG_VERBOSE, "ArmReadIdMmfr0 %lx PARange %lx\n", regValue, regValue & 0xF));
    if ((regValue & 0xF) < 7)
    {
        physicalAddressWidth = aw[regValue & 0xF];
    }
    else
    {
        // It is highly unlikely for the register to have an invalid value.
        // Regardless just use 36 as the default.
        DEBUG((DEBUG_WARN, "Invalid D_AA64MMFR0_EL1.PARange so defaulting address width to %u bits\n",
            MinimumAddressWidth));
        physicalAddressWidth = MinimumAddressWidth;
    }

#endif

    if (physicalAddressWidth < MinimumAddressWidth)
    {
        DEBUG((DEBUG_WARN, "Increasing address width from %u to %u\n",
            physicalAddressWidth, MinimumAddressWidth));
        physicalAddressWidth = MinimumAddressWidth;
    }

    if (physicalAddressWidth > MaximumAddressWidth)
    {
        DEBUG((DEBUG_WARN, "Reducing address width from %u to %u\n",
            physicalAddressWidth, MaximumAddressWidth));
        physicalAddressWidth = MaximumAddressWidth;
    }

    DEBUG((DEBUG_VERBOSE, "PhysicalAddressWidth %d\n", physicalAddressWidth));

    return physicalAddressWidth;
}


VOID
DebugDumpMadt(
    _In_ VOID* Madt
)
{
#if !defined(MDEPKG_NDEBUG)
    EFI_ACPI_DESCRIPTION_HEADER  *acpiHdr = (EFI_ACPI_DESCRIPTION_HEADER*)Madt;
    EFI_ACPI_6_2_IO_APIC_STRUCTURE* madtIoApic;
    EFI_ACPI_6_2_LOCAL_APIC_NMI_STRUCTURE* madtApicNmi;
    EFI_ACPI_6_2_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE* madtOverride;
    EFI_ACPI_6_2_PROCESSOR_LOCAL_APIC_STRUCTURE* madtApic;
    EFI_ACPI_6_2_PROCESSOR_LOCAL_X2APIC_STRUCTURE* madtX2Apic;
    EFI_ACPI_6_2_GIC_DISTRIBUTOR_STRUCTURE* madtGicd;
    EFI_ACPI_6_2_GIC_STRUCTURE* madtGicc;

    UINT8 *cursor;

    //
    // Debug dump the MADT entries.
    //
    DEBUG((DEBUG_VERBOSE, "--- MADT data @ %x\n", acpiHdr));
    DEBUG((DEBUG_VERBOSE, "    Header Signature %x\n", acpiHdr->Signature));
    DEBUG((DEBUG_VERBOSE, "    Length %x\n", acpiHdr->Length));

    cursor = (UINT8 *)acpiHdr;
    cursor += sizeof(EFI_ACPI_6_2_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER);

    do
    {
        switch (*cursor)  // UINT8 sized Type always at start of struct
        {

        case EFI_ACPI_6_2_IO_APIC:
            madtIoApic = (EFI_ACPI_6_2_IO_APIC_STRUCTURE *)cursor;

            DEBUG((DEBUG_VERBOSE, "    IOAPIC Type %x Len %02x IoApicId %02x IoApicAddress %02x\n",
                madtIoApic->Type, madtIoApic->Length, madtIoApic->IoApicId, madtIoApic->IoApicAddress));

            cursor += madtIoApic->Length;
            break;

        case EFI_ACPI_6_2_LOCAL_APIC_NMI:
            madtApicNmi = (EFI_ACPI_6_2_LOCAL_APIC_NMI_STRUCTURE *)cursor;

            DEBUG((DEBUG_VERBOSE, "    APIC NMI Type %x Len %02x Flags %02x AcpiProcessorUid %02x LocalApicLint %x\n",
                madtApicNmi->Type, madtApicNmi->Length, madtApicNmi->Flags, madtApicNmi->AcpiProcessorUid,
                madtApicNmi->LocalApicLint));

            cursor += madtApicNmi->Length;
            break;

        case EFI_ACPI_6_2_INTERRUPT_SOURCE_OVERRIDE:
            madtOverride = (EFI_ACPI_6_2_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE *)cursor;

            DEBUG((DEBUG_VERBOSE, "    Interrupt Source Override Type %x Len %02x Flags %02x Source %02x GlobalSystemInterrupt %x\n",
                madtOverride->Type, madtOverride->Length, madtOverride->Flags, madtOverride->Source,
                madtOverride->GlobalSystemInterrupt));

            cursor += madtOverride->Length;
            break;

        case EFI_ACPI_6_2_PROCESSOR_LOCAL_APIC:
            madtApic = (EFI_ACPI_6_2_PROCESSOR_LOCAL_APIC_STRUCTURE *)cursor;

            DEBUG((DEBUG_VERBOSE, "    APIC Type %x Len %02x Flags %02x ApicId %02x\n",
                madtApic->Type, madtApic->Length, madtApic->Flags, madtApic->ApicId));

            cursor += madtApic->Length;
            break;

        case EFI_ACPI_6_2_PROCESSOR_LOCAL_X2APIC:
            madtX2Apic = (EFI_ACPI_6_2_PROCESSOR_LOCAL_X2APIC_STRUCTURE *)cursor;

            DEBUG((DEBUG_VERBOSE, "   X2APIC Type %x Len %02x Flags %02x X2ApicId %02x\n",
                madtX2Apic->Type, madtX2Apic->Length, madtX2Apic->Flags, madtX2Apic->X2ApicId));

            cursor += madtX2Apic->Length;
            break;

        case EFI_ACPI_6_2_GICD:
            madtGicd = (EFI_ACPI_6_2_GIC_DISTRIBUTOR_STRUCTURE *)cursor;

            DEBUG((DEBUG_VERBOSE, "   GICD Type %x Len %02x GicId %02x PhysicalBaseAddress %02x\n",
                madtGicd->Type, madtGicd->Length, madtGicd->GicId, madtGicd->PhysicalBaseAddress));

            cursor += madtGicd->Length;
            break;

        case EFI_ACPI_6_2_GIC:
            madtGicc = (EFI_ACPI_6_2_GIC_STRUCTURE *)cursor;

            DEBUG((DEBUG_VERBOSE, "   GICD Type %x Len %02x Flags %02x AcpiProcessorUid %02x\n",
                madtGicc->Type, madtGicc->Length, madtGicc->Flags, madtGicc->AcpiProcessorUid));

            cursor += madtGicc->Length;
            break;

        default:
            madtApic = (EFI_ACPI_6_2_PROCESSOR_LOCAL_APIC_STRUCTURE *)cursor;

            DEBUG((DEBUG_VERBOSE, "    APIC Type %x Len %02x Flags %02x ApicId %02x\n",
                madtApic->Type, madtApic->Length, madtApic->Flags, madtApic->ApicId));

            cursor += madtApic->Length;
            break;
        }
    } while (cursor < ((UINT8 *)acpiHdr + acpiHdr->Length));

#endif
}


VOID
DebugDumpSrat(
    _In_ VOID* Srat
    )
{
#if !defined(MDEPKG_NDEBUG)
    EFI_ACPI_DESCRIPTION_HEADER  *acpiHdr = (EFI_ACPI_DESCRIPTION_HEADER*) Srat;
    EFI_ACPI_6_2_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY_STRUCTURE *sratApic;
    EFI_ACPI_6_2_PROCESSOR_LOCAL_X2APIC_AFFINITY_STRUCTURE *sratX2Apic;
    EFI_ACPI_6_2_GICC_AFFINITY_STRUCTURE *sratGicc;
    EFI_ACPI_6_2_MEMORY_AFFINITY_STRUCTURE *sratMem;
    UINTN base, size;
    UINT8 *cursor;

    //
    // Debug dump the SRAT entries.
    //
    DEBUG((DEBUG_VERBOSE, "--- SRAT data @ %x\n", acpiHdr));
    DEBUG((DEBUG_VERBOSE, "    Header Signature %x\n", acpiHdr->Signature));
    DEBUG((DEBUG_VERBOSE, "    Length %x\n", acpiHdr->Length));

    cursor = (UINT8 *)acpiHdr;
    cursor += sizeof(EFI_ACPI_6_2_SYSTEM_RESOURCE_AFFINITY_TABLE_HEADER);

    do
    {
        switch(*cursor)  // UINT8 sized Type always at start of struct
        {
            case EFI_ACPI_6_2_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY:
                sratApic = (EFI_ACPI_6_2_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY_STRUCTURE *)cursor;

                DEBUG((DEBUG_VERBOSE, "    APIC Type %x Len %02x Flags %02x ApicId %02x Dom %x\n",
                    sratApic->Type, sratApic->Length, sratApic->Flags, sratApic->ApicId,
                    sratApic->ProximityDomain7To0));

                cursor += sratApic->Length;
                break;

            case EFI_ACPI_6_2_PROCESSOR_LOCAL_X2APIC_AFFINITY:
                sratX2Apic = (EFI_ACPI_6_2_PROCESSOR_LOCAL_X2APIC_AFFINITY_STRUCTURE *)cursor;

                DEBUG((DEBUG_VERBOSE, "   X2APIC Type %x Len %02x Flags %02x X2ApicId %02x Dom %x\n",
                    sratX2Apic->Type, sratX2Apic->Length, sratX2Apic->Flags, sratX2Apic->X2ApicId,
                    sratX2Apic->ProximityDomain));

                cursor += sratX2Apic->Length;
                break;

            case EFI_ACPI_6_2_GICC_AFFINITY:
                sratGicc = (EFI_ACPI_6_2_GICC_AFFINITY_STRUCTURE *)cursor;

                DEBUG((DEBUG_VERBOSE, "   GICC Type %x Len %02x Flags %02x ProcessorUid %02x Dom %x\n",
                    sratGicc->Type, sratGicc->Length, sratGicc->Flags, sratGicc->AcpiProcessorUid,
                    sratGicc->ProximityDomain));

                cursor += sratGicc->Length;
                break;

            case EFI_ACPI_6_2_MEMORY_AFFINITY:
                sratMem = (EFI_ACPI_6_2_MEMORY_AFFINITY_STRUCTURE *)cursor;

                base = (((UINT64)sratMem->AddressBaseHigh) << 32) |
                        (UINT64)sratMem->AddressBaseLow;
                size = (((UINT64)sratMem->LengthHigh) << 32) |
                        (UINT64)sratMem->LengthLow;

                DEBUG((DEBUG_VERBOSE, "    MEM  Type %x Len %02x Flags %02x Base % 14lx Len % 14lx "
                    "Dom %x\n",
                    sratMem->Type, sratMem->Length, sratMem->Flags, base, size,
                    sratMem->ProximityDomain));

                cursor += sratMem->Length;
                break;

            default:
                sratMem = (EFI_ACPI_6_2_MEMORY_AFFINITY_STRUCTURE *)cursor;

                DEBUG((DEBUG_VERBOSE, "    *Skipping* Type %x\n", sratMem->Type));

                cursor += sratMem->Length;
                break;
        }
    } while (cursor < ((UINT8 *)acpiHdr + acpiHdr->Length));
#endif
}


VOID
DebugDumpSlit(
    _In_ VOID* Slit
    )
{
#if !defined(MDEPKG_NDEBUG)
    EFI_ACPI_DESCRIPTION_HEADER  *acpiHdr = (EFI_ACPI_DESCRIPTION_HEADER*) Slit;
    UINT8 *cursor;

    DEBUG((DEBUG_VERBOSE, "--- SLIT data @ %x\n", acpiHdr));
    DEBUG((DEBUG_VERBOSE, "    Header Signature %x\n", acpiHdr->Signature));
    DEBUG((DEBUG_VERBOSE, "    Length %x\n", acpiHdr->Length));

    cursor = (UINT8*)acpiHdr;
    cursor += sizeof(EFI_ACPI_DESCRIPTION_HEADER);

    UINT64 localityCount = *(UINT64*)cursor;
    DEBUG((DEBUG_VERBOSE, "    Number of Localities: %x\n", localityCount));

    cursor += sizeof(UINT64);

    DEBUG((DEBUG_VERBOSE, "    Entries:\n"));
    DEBUG((DEBUG_VERBOSE, "    "));
    for (UINT64 i = 0;i < localityCount;i += 1)
    {
        DEBUG((DEBUG_VERBOSE, "    [%d]", i));
    }
    DEBUG((DEBUG_VERBOSE, "\n"));

    for (UINT64 i = 0;i < localityCount;i += 1)
    {
        DEBUG((DEBUG_VERBOSE, "    [%d]", i));
        for (UINT64 j = 0;j < localityCount;j += 1)
        {
            DEBUG((DEBUG_VERBOSE, "    %d", *cursor));
            cursor += 1;
        }
        DEBUG((DEBUG_VERBOSE, "\n"));
    }
#endif
}

VOID
DebugDumpMemoryMap(
    _In_ VOID* MemMap,
    _In_ UINT32 MemMapSize,
    _In_ BOOLEAN LegacyMemoryMap
    )
{
    //
    // Debug dump the Memory Map entries.
    //
#if !defined(MDEPKG_NDEBUG)
    DEBUG((DEBUG_VERBOSE, "--- Memory Map data @ %x Length %x\n", MemMap, MemMapSize));
    if (!LegacyMemoryMap)
    {
        PVM_MEMORY_RANGE_V5 range = (PVM_MEMORY_RANGE_V5)MemMap;
        do
        {
            DEBUG((DEBUG_VERBOSE, "    Base % 14lx Len % 14lx Flags % 8x\n",
                range->BaseAddress, range->Length, range->Flags));
            range++;
        } while ((UINT8*)range < ((UINT8*)MemMap + MemMapSize));
    }
    else
    {
        PVM_MEMORY_RANGE range = (PVM_MEMORY_RANGE)MemMap;
        do
        {
            DEBUG((DEBUG_VERBOSE, "    Base % 14lx Len % 14lx\n", range->BaseAddress, range->Length));
            range++;
        } while ((UINT8*)range < ((UINT8*)MemMap + MemMapSize));
    }
#endif
}

UEFI_CONFIG_HEADER*
GetStartOfConfigBlob(
    VOID
    )
/*++

Routine Description:

    Return the start of the config blob, past the firmware and any additional
    data.

Arguments:

    None.

Return Value:

    A pointer to the start of the config blob.

--*/
{
#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)
    //
    // On X64, the config blob starts after the end of the firmware, and after
    // some misc. pages (including space for the pagetables and GDT entries).
    //
    UINT64 configBlobBase =
        PcdGet64(PcdFdBaseAddress) +
        PcdGet32(PcdFdSize) +
        SIZE_4KB * MISC_PAGE_COUNT_TOTAL;
#elif defined(MDE_CPU_AARCH64)
    //
    // On AARCH64, the config blob starts after the end of the firmware, and
    // after the pagetables and stack/heap, at the start of system ram.
    //
    UINT64 configBlobBase = PcdGet64(PcdSystemMemoryBaseAddress);
#endif

    DEBUG((DEBUG_VERBOSE, "ConfigBlobBase: 0x%lx\n", configBlobBase));

    return (UEFI_CONFIG_HEADER*)(UINTN)configBlobBase;
}

VOID
DebugDumpUefiConfigStruct(
    _In_ UEFI_CONFIG_HEADER* Header
    )
{
#if !defined(MDEPKG_NDEBUG)
    DEBUG((DEBUG_VERBOSE, "Header Type: 0x%x \tHeader Length: 0x%x\n", Header->Type, Header->Length));

    switch(Header->Type)
    {
        case UefiConfigStructureCount:
            UEFI_CONFIG_STRUCTURE_COUNT *count = (UEFI_CONFIG_STRUCTURE_COUNT*) Header;
            DEBUG((DEBUG_VERBOSE, "\tTotalStructureCount: %u\n", count->TotalStructureCount));
            break;

        case UefiConfigBiosInformation:
            UEFI_CONFIG_BIOS_INFORMATION *biosInfo = (UEFI_CONFIG_BIOS_INFORMATION*) Header;
            DEBUG((DEBUG_VERBOSE, "\tBiosSizePages: 0x%x\n\tLegacyMemoryMap:%u\n", biosInfo->BiosSizePages, biosInfo->Flags.LegacyMemoryMap));
            break;

        case UefiConfigMadt:
            UEFI_CONFIG_MADT * madt = (UEFI_CONFIG_MADT*)Header;
            DebugDumpMadt(madt->Madt);
            break;

        case UefiConfigSrat:
            UEFI_CONFIG_SRAT *srat = (UEFI_CONFIG_SRAT*) Header;
            DebugDumpSrat(srat->Srat);
            break;

        case UefiConfigSlit:
            UEFI_CONFIG_SLIT *slit = (UEFI_CONFIG_SLIT*)Header;
            DebugDumpSlit(slit->Slit);
            break;

        case UefiConfigMemoryMap:
            UEFI_CONFIG_MEMORY_MAP *memMap = (UEFI_CONFIG_MEMORY_MAP*) Header;
            DebugDumpMemoryMap(memMap->MemoryMap, Header->Length - sizeof(UEFI_CONFIG_HEADER), PcdGetBool(PcdLegacyMemoryMap));
            break;

        case UefiConfigEntropy:
            DEBUG((DEBUG_VERBOSE, "\tEntropy table found.\n"));
            break;

        case UefiConfigBiosGuid:
            UEFI_CONFIG_BIOS_GUID *biosGuid = (UEFI_CONFIG_BIOS_GUID*) Header;
            DEBUG((DEBUG_VERBOSE, "\tBiosGuid: %g\n", (EFI_GUID*) biosGuid->BiosGuid));
            break;

        case UefiConfigSmbiosSystemManufacturer:
            UEFI_CONFIG_SMBIOS_SYSTEM_MANUFACTURER *systemManufacturer = (UEFI_CONFIG_SMBIOS_SYSTEM_MANUFACTURER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios System Manufacturer: %a\n", systemManufacturer->SystemManufacturer));
            break;

        case UefiConfigSmbiosSystemProductName:
            UEFI_CONFIG_SMBIOS_SYSTEM_PRODUCT_NAME *systemProductName = (UEFI_CONFIG_SMBIOS_SYSTEM_PRODUCT_NAME*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios System Product Name: %a\n", systemProductName->SystemProductName));
            break;

        case UefiConfigSmbiosSystemVersion:
            UEFI_CONFIG_SMBIOS_SYSTEM_VERSION *systemVersion = (UEFI_CONFIG_SMBIOS_SYSTEM_VERSION*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios System Version: %a\n", systemVersion->SystemVersion));
            break;

        case UefiConfigSmbiosSystemSerialNumber:
            UEFI_CONFIG_SMBIOS_SYSTEM_SERIAL_NUMBER *systemSerialNumber = (UEFI_CONFIG_SMBIOS_SYSTEM_SERIAL_NUMBER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios System Serial Number: %a\n", systemSerialNumber->SystemSerialNumber));
            break;

        case UefiConfigSmbiosSystemSKUNumber:
            UEFI_CONFIG_SMBIOS_SYSTEM_SKU_NUMBER *systemSKUNumber = (UEFI_CONFIG_SMBIOS_SYSTEM_SKU_NUMBER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios System SKU Number: %a\n", systemSKUNumber->SystemSKUNumber));
            break;

        case UefiConfigSmbiosSystemFamily:
            UEFI_CONFIG_SMBIOS_SYSTEM_FAMILY *systemFamily = (UEFI_CONFIG_SMBIOS_SYSTEM_FAMILY*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios System Family: %a\n", systemFamily->SystemFamily));
            break;

        case UefiConfigSmbiosBaseSerialNumber:
            UEFI_CONFIG_SMBIOS_BASE_SERIAL_NUMBER *baseSerialNumber = (UEFI_CONFIG_SMBIOS_BASE_SERIAL_NUMBER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Base Serial Number: %a\n", baseSerialNumber->BaseSerialNumber));
            break;

        case UefiConfigSmbiosChassisSerialNumber:
            UEFI_CONFIG_SMBIOS_CHASSIS_SERIAL_NUMBER *chassisSerialNumber = (UEFI_CONFIG_SMBIOS_CHASSIS_SERIAL_NUMBER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Chassis Serial Number: %a\n", chassisSerialNumber->ChassisSerialNumber));
            break;

        case UefiConfigSmbiosChassisAssetTag:
            UEFI_CONFIG_SMBIOS_CHASSIS_ASSET_TAG *chassisAssetTag = (UEFI_CONFIG_SMBIOS_CHASSIS_ASSET_TAG*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Chassis Asset Tag: %a\n", chassisAssetTag->ChassisAssetTag));
            break;

        case UefiConfigSmbiosBiosLockString:
            UEFI_CONFIG_SMBIOS_BIOS_LOCK_STRING *biosLockString = (UEFI_CONFIG_SMBIOS_BIOS_LOCK_STRING*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Bios Lock String: %a\n", biosLockString->BiosLockString));
            break;

        case UefiConfigSmbiosMemoryDeviceSerialNumber:
            UEFI_CONFIG_SMBIOS_MEMORY_DEVICE_SERIAL_NUMBER *memoryDeviceSerialNumber = (UEFI_CONFIG_SMBIOS_MEMORY_DEVICE_SERIAL_NUMBER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Memory Device Serial Number: %a\n", memoryDeviceSerialNumber->MemoryDeviceSerialNumber));
            break;

        case UefiConfigSmbios31ProcessorInformation:
            UEFI_CONFIG_SMBIOS_3_1_PROCESSOR_INFORMATION *procInfo = (UEFI_CONFIG_SMBIOS_3_1_PROCESSOR_INFORMATION*) Header;
            DEBUG((DEBUG_VERBOSE, "\tProcessorType: %u\n", procInfo->ProcessorType));
            DEBUG((DEBUG_VERBOSE, "\tProcessorID: 0x%x\n", procInfo->ProcessorID));
            DEBUG((DEBUG_VERBOSE, "\tVoltage: %u\n", procInfo->Voltage));
            DEBUG((DEBUG_VERBOSE, "\tExternalClock: 0x%x\n", procInfo->ExternalClock));
            DEBUG((DEBUG_VERBOSE, "\tMaxSpeed: 0x%x\n", procInfo->MaxSpeed));
            DEBUG((DEBUG_VERBOSE, "\tCurrentSpeed: 0x%x\n", procInfo->CurrentSpeed));
            DEBUG((DEBUG_VERBOSE, "\tStatus: 0x%x\n", procInfo->Status));
            DEBUG((DEBUG_VERBOSE, "\tProcessorUpgrade: 0x%x\n", procInfo->ProcessorUpgrade));
            DEBUG((DEBUG_VERBOSE, "\tProcessorCharacteristics: 0x%x\n", procInfo->ProcessorCharacteristics));
            DEBUG((DEBUG_VERBOSE, "\tProcessorFamily2: %u\n", procInfo->ProcessorFamily2));
            break;

        case UefiConfigSmbiosSocketDesignation:
            UEFI_CONFIG_SMBIOS_SOCKET_DESIGNATION *socketDesignation = (UEFI_CONFIG_SMBIOS_SOCKET_DESIGNATION*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Socket Designation: %a\n", socketDesignation->SocketDesignation));
            break;

        case UefiConfigSmbiosProcessorManufacturer:
            UEFI_CONFIG_SMBIOS_PROCESSOR_MANUFACTURER *processorManufacturer = (UEFI_CONFIG_SMBIOS_PROCESSOR_MANUFACTURER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Processor Manufacturer: %a\n", processorManufacturer->ProcessorManufacturer));
            break;

        case UefiConfigSmbiosProcessorVersion:
            UEFI_CONFIG_SMBIOS_PROCESSOR_VERSION *processorVersion = (UEFI_CONFIG_SMBIOS_PROCESSOR_VERSION*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Processor Version: %a\n", processorVersion->ProcessorVersion));
            break;

        case UefiConfigSmbiosProcessorSerialNumber:
            UEFI_CONFIG_SMBIOS_PROCESSOR_SERIAL_NUMBER *processorSerialNumber = (UEFI_CONFIG_SMBIOS_PROCESSOR_SERIAL_NUMBER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Processor Serial Number: %a\n", processorSerialNumber->ProcessorSerialNumber));
            break;

        case UefiConfigSmbiosProcessorAssetTag:
            UEFI_CONFIG_SMBIOS_PROCESSOR_ASSET_TAG *processorAssetTag = (UEFI_CONFIG_SMBIOS_PROCESSOR_ASSET_TAG*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Processor Asset Tag: %a\n", processorAssetTag->ProcessorAssetTag));
            break;

        case UefiConfigSmbiosProcessorPartNumber:
            UEFI_CONFIG_SMBIOS_PROCESSOR_PART_NUMBER *processorPartNumber = (UEFI_CONFIG_SMBIOS_PROCESSOR_PART_NUMBER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Processor Part Number: %a\n", processorPartNumber->ProcessorPartNumber));
            break;

        case UefiConfigFlags:
            UEFI_CONFIG_FLAGS *flags = (UEFI_CONFIG_FLAGS*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSerialControllersEnabled: %u\n", flags->Flags.SerialControllersEnabled));
            DEBUG((DEBUG_VERBOSE, "\tPauseAfterBootFailure: %u\n", flags->Flags.PauseAfterBootFailure));
            DEBUG((DEBUG_VERBOSE, "\tPxeIpV6: %u\n", flags->Flags.PxeIpV6));
            DEBUG((DEBUG_VERBOSE, "\tDebuggerEnabled: %u\n", flags->Flags.DebuggerEnabled));
            DEBUG((DEBUG_VERBOSE, "\tLoadOempTable: %u\n", flags->Flags.LoadOempTable));
            DEBUG((DEBUG_VERBOSE, "\tTpmEnabled: %u\n", flags->Flags.TpmEnabled));
            DEBUG((DEBUG_VERBOSE, "\tHibernateEnabled: %u\n", flags->Flags.HibernateEnabled));
            DEBUG((DEBUG_VERBOSE, "\tConsoleMode: %u\n", flags->Flags.ConsoleMode));
            DEBUG((DEBUG_VERBOSE, "\tMemoryAttributesTableEnabled: %u\n", flags->Flags.MemoryAttributesTableEnabled));
            DEBUG((DEBUG_VERBOSE, "\tVirtualBatteryEnabled: %u\n", flags->Flags.VirtualBatteryEnabled));
            DEBUG((DEBUG_VERBOSE, "\tSgxMemoryEnabled: %u\n", flags->Flags.SgxMemoryEnabled));
            DEBUG((DEBUG_VERBOSE, "\tIsVmbfsBoot: %u\n", flags->Flags.IsVmbfsBoot));
            DEBUG((DEBUG_VERBOSE, "\tMeasureAdditionalPcrs: %u\n", flags->Flags.MeasureAdditionalPcrs));
            DEBUG((DEBUG_VERBOSE, "\tShutdownAfterBootFailure: %u\n", flags->Flags.DisableFrontpage));
            DEBUG((DEBUG_VERBOSE, "\tDefaultBootAlwaysAttempt: %u\n", flags->Flags.DefaultBootAlwaysAttempt));
            DEBUG((DEBUG_VERBOSE, "\tLowPowerS0IdleEnabled: %u\n", flags->Flags.LowPowerS0IdleEnabled));
            DEBUG((DEBUG_VERBOSE, "\tVpciBootEnabled: %u\n", flags->Flags.VpciBootEnabled));
            DEBUG((DEBUG_VERBOSE, "\tProcIdleEnabled: %u\n", flags->Flags.ProcIdleEnabled));
            DEBUG((DEBUG_VERBOSE, "\tDisableSha384Pcr: %u\n", flags->Flags.DisableSha384Pcr));
            DEBUG((DEBUG_VERBOSE, "\tMediaPresentEnabledByDefault: %u\n", flags->Flags.MediaPresentEnabledByDefault));
            break;

        case UefiConfigProcessorInformation:
            UEFI_CONFIG_PROCESSOR_INFORMATION *processorInfo = (UEFI_CONFIG_PROCESSOR_INFORMATION*) Header;
            DEBUG((DEBUG_VERBOSE, "\tProcessor Count: %u\n\tProcessorsPerVirtualSocket: %u\n",
                processorInfo->ProcessorCount,
                processorInfo->ProcessorsPerVirtualSocket));
            break;

        case UefiConfigMmioRanges:
            UEFI_CONFIG_MMIO_RANGES *mmioRanges = (UEFI_CONFIG_MMIO_RANGES*) Header;
            DEBUG((DEBUG_VERBOSE, "\tMmio Ranges:\n"));
            DEBUG((DEBUG_VERBOSE, "\tStart:0x%17lx Size:0x%x\n", mmioRanges->Ranges[0].MmioPageNumberStart, mmioRanges->Ranges[0].MmioSizeInPages));
            DEBUG((DEBUG_VERBOSE, "\tStart:0x%17lx Size:0x%x\n", mmioRanges->Ranges[1].MmioPageNumberStart, mmioRanges->Ranges[1].MmioSizeInPages));
            break;

        case UefiConfigAcpiTable:
            UEFI_CONFIG_ACPI_TABLE *acpi = (UEFI_CONFIG_ACPI_TABLE*) Header;
            UINT64 acpiTableSize = acpi->Header.Length - sizeof(UEFI_CONFIG_HEADER);
            EFI_ACPI_DESCRIPTION_HEADER* acpiHeader = (EFI_ACPI_DESCRIPTION_HEADER*) acpi->AcpiTableData;
            DEBUG((DEBUG_VERBOSE, "\tAcpi Data Size:0x%x\n", acpiTableSize));
            DEBUG((DEBUG_VERBOSE, "\tAcpi Header Size:0x%x\n", acpiHeader->Length));
            DEBUG((DEBUG_VERBOSE, "\tAcpi Header Signature:0x%x\n", acpiHeader->Signature));
            break;

        case UefiConfigNvdimmCount:
            UEFI_CONFIG_NVDIMM_COUNT *cfg = (UEFI_CONFIG_NVDIMM_COUNT*) Header;
            DEBUG((DEBUG_VERBOSE, "\tNVDIMM Count:0x%lx\n", cfg->Count));
            break;

        case UefiConfigVpciInstanceFilter:
            UEFI_CONFIG_VPCI_INSTANCE_FILTER *filter = (UEFI_CONFIG_VPCI_INSTANCE_FILTER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tVpci instance filter:%g\n", (EFI_GUID*) filter->InstanceGuid));
            break;

        default:
            DEBUG((DEBUG_VERBOSE, "\t!!! Unrecognized config structure type !!!\n"));
            break;
    }
#endif
}

VOID
GetSmbiosStructureStringLength(
    _In_ UINT32 HeaderLength,
    _In_ UINT8* String,
    _Out_ UINT32* StringLength
    )
/*++

Routine Description:

    Get the length of an SMBIOS string config structure, including the null
    terminator. Will truncate strings if no null terminator is found.

Arguments:

    HeaderLength - The length in the config structure header.

    String - A pointer to the start of the string data past the config header.

    StringLength - Returns the length of the string, including null terminator.

Return Value:

    None

--*/
{
    *StringLength = 0;
    UINT64 remainingStructureSize = HeaderLength - sizeof(UEFI_CONFIG_HEADER);

    UINTN length = AsciiStrnLenS((CHAR8*) String, remainingStructureSize);

    if (length == remainingStructureSize)
    {
        //
        // No NULL found, truncate by adding one at the end.
        //
        String[length - 1] = 0;
        *StringLength = (UINT32)length;

        DEBUG((DEBUG_VERBOSE, "SMBIOS String Structure had no null terminator, truncating to size 0x%x. Truncated string:%a", length, String));
    }
    else
    {
        //
        // Add one to the length for the null character.
        //
        *StringLength = (UINT32) (length + 1);
    }
}

VOID
ConfigSetProcessorInfo(
    UEFI_CONFIG_PROCESSOR_INFORMATION *ProcessorInfo
    )
{
    if (ProcessorInfo->ProcessorCount == 0)
    {
        DEBUG((DEBUG_ERROR, "Processors count was 0.\n"));
        CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(CONFIG, __LINE__);
    }

    if (ProcessorInfo->ProcessorsPerVirtualSocket == 0)
    {
        DEBUG((DEBUG_ERROR, "Processors per virtual socket was 0.\n"));
        CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(CONFIG, __LINE__);
    }

    if (ProcessorInfo->ThreadsPerProcessor == 0)
    {
        DEBUG((DEBUG_ERROR, "Threads per processor was 0.\n"));
        CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(CONFIG, __LINE__);
    }

    PcdSet32(PcdProcessorCount, ProcessorInfo->ProcessorCount);
    PcdSet32(PcdProcessorsPerVirtualSocket, ProcessorInfo->ProcessorsPerVirtualSocket);
    PcdSet32(PcdThreadsPerProcessor, ProcessorInfo->ThreadsPerProcessor);
}


VOID
ConfigSetUefiConfigFlags(
    UEFI_CONFIG_FLAGS *ConfigFlags
    )
{
    PcdSetBool(PcdSerialControllersEnabled, (UINT8) ConfigFlags->Flags.SerialControllersEnabled);
    PcdSetBool(PcdPauseAfterBootFailure, (UINT8) ConfigFlags->Flags.PauseAfterBootFailure);
    PcdSetBool(PcdPxeIpV6, (UINT8) ConfigFlags->Flags.PxeIpV6);
    PcdSetBool(PcdDebuggerEnabled, (UINT8) ConfigFlags->Flags.DebuggerEnabled);
    PcdSetBool(PcdLoadOempTable, (UINT8) ConfigFlags->Flags.LoadOempTable);
    PcdSetBool(PcdTpmEnabled, (UINT8) ConfigFlags->Flags.TpmEnabled);
    PcdSetBool(PcdHibernateEnabled, (UINT8) ConfigFlags->Flags.HibernateEnabled);
    PcdSet8(PcdConsoleMode, (UINT8) ConfigFlags->Flags.ConsoleMode);
    PcdSetBool(PcdMemoryAttributesTableEnabled, (UINT8) ConfigFlags->Flags.MemoryAttributesTableEnabled);
    PcdSetBool(PcdVirtualBatteryEnabled, (UINT8) ConfigFlags->Flags.VirtualBatteryEnabled);
    PcdSetBool(PcdSgxMemoryEnabled, (UINT8) ConfigFlags->Flags.SgxMemoryEnabled);
    PcdSetBool(PcdIsVmbfsBoot, (UINT8) ConfigFlags->Flags.IsVmbfsBoot);
    PcdSetBool(PcdDisableFrontpage, (UINT8) ConfigFlags->Flags.DisableFrontpage);
    PcdSetBool(PcdDefaultBootAlwaysAttempt, (UINT8) ConfigFlags->Flags.DefaultBootAlwaysAttempt);
    PcdSetBool(PcdLowPowerS0IdleEnabled, (UINT8)ConfigFlags->Flags.LowPowerS0IdleEnabled);
    PcdSetBool(PcdVpciBootEnabled, (UINT8)ConfigFlags->Flags.VpciBootEnabled);
    PcdSetBool(PcdProcIdleEnabled, (UINT8) ConfigFlags->Flags.ProcIdleEnabled);
    PcdSetBool(PcdMediaPresentEnabledByDefault, (UINT8) ConfigFlags->Flags.MediaPresentEnabledByDefault);

    //
    // For VM vdev version 8 and above, MeasureAdditionalPcrs will be TRUE.
    // When TRUE, we will perform a more "standard" measured boot
    //
    if (ConfigFlags->Flags.MeasureAdditionalPcrs)
    {
        // TODO-cho: no TPM available for AARCH64 yet.
#if defined (MDE_CPU_X64)
        PcdSetBool(TcgMeasureBootStringsInPcr4, TRUE);
        PcdSetBool(PcdExcludeFvMainFromMeasurements, FALSE);
#endif
    }

    //
    // For VM versions below 9.3, DisableSha384Pcr will be TRUE.
    // When TRUE, we remove SHA-384 from the PCR hash mask.
    //
    if (ConfigFlags->Flags.DisableSha384Pcr)
    {
#if defined (MDE_CPU_X64)
        PcdSet32(PcdTpm2HashMask, (PcdGet32(PcdTpm2HashMask) & ~HASH_ALG_SHA384));
#endif
    }
}


EFI_STATUS
VerifyStructureLength(
    _In_ UEFI_CONFIG_HEADER* Header
    )
/*++

Routine Description:

    Verify that the config structure of a given type is the correct length.

Arguments:

    Header - A pointer to the header of a config structure.

Return Value:

    EFI_SUCCESS on success. EFI_INVALID_PARAMETER if the Header length is invalid.

--*/
{
    //
    // All structures must be aligned to 8 bytes, as AARCH64 does not allow
    // unaligned access like X64.
    // The size must be as big as the header size.
    //
    if (Header->Length % 8 != 0)
    {
        DEBUG((DEBUG_ERROR, "Structure Type 0x%x was length 0x%x, not aligned to 8 bytes.\n",
            Header->Type,
            Header->Length));
        return EFI_INVALID_PARAMETER;
    }

    if (Header->Length <= sizeof(UEFI_CONFIG_HEADER))
    {
        DEBUG((DEBUG_ERROR, "Structure Type 0x%x was length 0x%x, and smaller than the header size and information.\n",
            Header->Type,
            Header->Length));
        return EFI_INVALID_PARAMETER;
    }

    //
    // Size of 0 means the structure has a variable length, and will
    // be verified later on.
    //
    static const UINT32 StructureLengthTable[] =
    {
        sizeof(UEFI_CONFIG_STRUCTURE_COUNT), //UefiConfigStructureCount
        sizeof(UEFI_CONFIG_BIOS_INFORMATION), //UefiConfigBiosInformation
        0, //UefiConfigSrat
        0, //UefiConfigMemoryMap
        sizeof(UEFI_CONFIG_ENTROPY), //UefiConfigEntropy
        sizeof(UEFI_CONFIG_BIOS_GUID), //UefiConfigBiosGuid
        0, //UefiConfigSmbiosSystemSerialNumber
        0, //UefiConfigSmbiosBaseSerialNumber
        0, //UefiConfigSmbiosChassisSerialNumber
        0, //UefiConfigSmbiosChassisAssetTag
        0, //UefiConfigSmbiosBiosLockString
        sizeof(UEFI_CONFIG_SMBIOS_3_1_PROCESSOR_INFORMATION), //UefiConfigSmbios31ProcessorInformation
        0, //UefiConfigSmbiosSocketDesignation
        0, //UefiConfigSmbiosProcessorManufacturer
        0, //UefiConfigSmbiosProcessorVersion
        0, //UefiConfigSmbiosProcessorSerialNumber
        0, //UefiConfigSmbiosProcessorAssetTag
        0, //UefiConfigSmbiosProcessorPartNumber
        sizeof(UEFI_CONFIG_FLAGS), //UefiConfigFlags
        sizeof(UEFI_CONFIG_PROCESSOR_INFORMATION), //UefiConfigProcessorInformation
        0, //UefiConfigMmioRanges
        0, //UefiConfigAARCH64MPIDR - not used
        0, //UefiConfigAcpiTable
        sizeof(UEFI_CONFIG_NVDIMM_COUNT), //UefiConfigNvdimmCount
        0, //UefiConfigMadt
        sizeof(UEFI_CONFIG_VPCI_INSTANCE_FILTER), //UefiConfigVpciInstanceFilter
        0, //UefiConfigSmbiosSystemManufacturer
        0, //UefiConfigSmbiosSystemProductName
        0, //UefiConfigSmbiosSystemVersion
        0, //UefiConfigSmbiosSystemSKUNumber
        0, //UefiConfigSmbiosSystemFamily
        0, //UefiConfigSmbiosMemoryDeviceSerialNumber
        0, //UefiConfigSlit
    };

    //
    // If this is a type that is not currently parsed, ignore it.
    //
    if (Header->Type >= (sizeof(StructureLengthTable)/sizeof(StructureLengthTable[0])))
    {
        return EFI_SUCCESS;
    }

    //
    // Otherwise, check structure length via lookup table. Nonzero values
    // must match.
    //
    UINT32 expectedLength = StructureLengthTable[Header->Type];
    if (expectedLength != 0 && Header->Length != expectedLength)
    {
        DEBUG((DEBUG_ERROR, "Structure Type 0x%x was length 0x%x, expected Length %x\n",
            Header->Type,
            Header->Length,
            expectedLength));
        return EFI_INVALID_PARAMETER;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
GetUefiConfigInfo(
    VOID
    )
/*++

Routine Description:

    Get and parse the config blob that contains information from the Bios VDEV.
    Note that the information received and parsed here can come from the host 
    and cannot be trusted. Validate the config information before using it.

Arguments:

    None.

Return Value:

    EFI_SUCCESS on success. Error if the config blob is malformed.

--*/
{
    UINT32 stringLength = 0;
    UEFI_CONFIG_HEADER *header = NULL;
    UEFI_CONFIG_STRUCTURE_COUNT *configCount = NULL;
    UINT32 calculatedConfigSize = 0;

    //
    // Tracking to see if the config blob has all the required structures.
    //
#if defined(MDE_CPU_X64)
    static const UINT64 AllStructuresFound = 0x1FF;
    union {
        struct {
            UINT64 UefiConfigBiosInformation:1;
            UINT64 UefiConfigMadt:1;
            UINT64 UefiConfigSrat:1;
            UINT64 UefiConfigMemoryMap:1;
            UINT64 UefiConfigEntropy:1;
            UINT64 UefiConfigBiosGuid:1;
            UINT64 UefiConfigFlags:1;
            UINT64 UefiConfigProcessorInformation:1;
            UINT64 UefiConfigMmioRanges:1;
            UINT64 Reserved:55;
        };

        UINT64 AsUINT64;
    } requiredStructures;
#elif defined(MDE_CPU_AARCH64)
    static const UINT64 AllStructuresFound = 0xFF;
    union {
        struct {
            UINT64 UefiConfigBiosInformation:1;
            UINT64 UefiConfigSrat:1;
            UINT64 UefiConfigMemoryMap:1;
            UINT64 UefiConfigEntropy:1;
            UINT64 UefiConfigBiosGuid:1;
            UINT64 UefiConfigFlags:1;
            UINT64 UefiConfigProcessorInformation:1;
            UINT64 UefiConfigMmioRanges:1;
            UINT64 Reserved:56;
        };

        UINT64 AsUINT64;
    } requiredStructures;
#endif
    requiredStructures.AsUINT64 = 0;

    header = GetStartOfConfigBlob();

    //
    // Read the first structure, which must be the structure describing the
    // total number of structures.
    //
    DebugDumpUefiConfigStruct(header);
    configCount = (UEFI_CONFIG_STRUCTURE_COUNT*) header;

    //
    // Sanity checks to make sure that the header is correct.
    //
    if (header->Type != UefiConfigStructureCount ||
        configCount->TotalStructureCount <= 1)
    {
        DEBUG((DEBUG_ERROR, "*** Malformed Header (Structure count) \n"));
        CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(CONFIG, __LINE__);
    }

    if (EFI_ERROR(VerifyStructureLength(header)))
    {
        DEBUG((DEBUG_ERROR, "*** Malformed Header Length (Structure count) \n"));
        CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(CONFIG, __LINE__);
    }

    PcdSet32(PcdConfigBlobSize, configCount->TotalConfigBlobSize);

    //
    // Advance past initial header to other structures.
    //
    calculatedConfigSize += header->Length;
    header = (UEFI_CONFIG_HEADER*) ((UINT64) header + header->Length);

    //
    // Loop through the remaining structures.
    //
    for (UINT64 i = 1; i < configCount->TotalStructureCount; i++)
    {
        if (EFI_ERROR(VerifyStructureLength(header)))
        {
            DEBUG((DEBUG_ERROR, "*** Malformed Header Length\n"));
            CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(CONFIG, __LINE__);
        }

        if (calculatedConfigSize > configCount->TotalConfigBlobSize)
        {
            DEBUG((DEBUG_ERROR, "Config offset of 0x%x is greater than the actual size of 0x%x\n", 
                calculatedConfigSize,
                configCount->TotalConfigBlobSize));
            CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(CONFIG, __LINE__);
        }

        DebugDumpUefiConfigStruct(header);

        switch(header->Type)
        {
            case UefiConfigBiosInformation:
                UEFI_CONFIG_BIOS_INFORMATION *biosInfo = (UEFI_CONFIG_BIOS_INFORMATION*) header;
                PcdSetBool(PcdLegacyMemoryMap, (UINT8)biosInfo->Flags.LegacyMemoryMap);
                requiredStructures.UefiConfigBiosInformation = 1;
                break;

            case UefiConfigMadt:
                UEFI_CONFIG_MADT * madtStructure = (UEFI_CONFIG_MADT*)header;
                EFI_ACPI_DESCRIPTION_HEADER *madtHdr = (EFI_ACPI_DESCRIPTION_HEADER*)madtStructure->Madt;

                if (madtStructure->Header.Length < (sizeof(UEFI_CONFIG_HEADER) + sizeof(EFI_ACPI_DESCRIPTION_HEADER)) ||
                    madtHdr->Signature != EFI_ACPI_6_2_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE ||
                    madtHdr->Length >(madtStructure->Header.Length - sizeof(UEFI_CONFIG_HEADER)))
                {
                    DEBUG((DEBUG_ERROR, "*** Malformed MADT\n"));
                    CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(CONFIG, __LINE__);
                }

                PcdSet64(PcdMadtPtr, (UINT64)madtStructure->Madt);
                PcdSet32(PcdMadtSize, madtHdr->Length);
#if defined(MDE_CPU_X64)
                requiredStructures.UefiConfigMadt = 1;
#endif
                break;

            case UefiConfigSrat:
                UEFI_CONFIG_SRAT *sratStructure = (UEFI_CONFIG_SRAT*) header;
                EFI_ACPI_DESCRIPTION_HEADER *sratHdr = (EFI_ACPI_DESCRIPTION_HEADER*) sratStructure->Srat;

                //
                // NOTE: Because ARM GICC affinity structures are not aligned to 8 bytes,
                // this structure may be padded. Thus, the table described by the ACPI header
                // just needs to be less than the overall length.
                //
                if (sratStructure->Header.Length < (sizeof(UEFI_CONFIG_HEADER) + sizeof(EFI_ACPI_DESCRIPTION_HEADER)) ||
                    sratHdr->Signature != EFI_ACPI_6_2_SYSTEM_RESOURCE_AFFINITY_TABLE_SIGNATURE ||
                    sratHdr->Length > (sratStructure->Header.Length - sizeof(UEFI_CONFIG_HEADER)))
                {
                    DEBUG((DEBUG_ERROR, "*** Malformed SRAT\n"));
                    CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(CONFIG, __LINE__);
                }

                PcdSet64(PcdSratPtr, (UINT64) sratStructure->Srat);
                PcdSet32(PcdSratSize, sratHdr->Length);
                requiredStructures.UefiConfigSrat = 1;
                break;

            case UefiConfigSlit:
                UEFI_CONFIG_SLIT *slitStructure = (UEFI_CONFIG_SLIT*) header;
                EFI_ACPI_DESCRIPTION_HEADER *slitHdr = (EFI_ACPI_DESCRIPTION_HEADER*) slitStructure->Slit;

                if (slitStructure->Header.Length < (sizeof(UEFI_CONFIG_HEADER) + sizeof(EFI_ACPI_DESCRIPTION_HEADER)) ||
                    slitHdr->Signature != EFI_ACPI_6_2_SYSTEM_LOCALITY_INFORMATION_TABLE_SIGNATURE ||
                    slitHdr->Length > (slitStructure->Header.Length - sizeof(UEFI_CONFIG_HEADER)))
                {
                    DEBUG((DEBUG_ERROR, "*** Malformed SLIT\n"));
                    CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(CONFIG, __LINE__);
                }

                PcdSet64(PcdSlitPtr, (UINT64)slitStructure->Slit);
                PcdSet32(PcdSlitSize, slitHdr->Length);
                break;

            case UefiConfigMemoryMap:
                UEFI_CONFIG_MEMORY_MAP *memoryMapStructure = (UEFI_CONFIG_MEMORY_MAP*) header;
                PcdSet64(PcdMemoryMapPtr, (UINT64) memoryMapStructure->MemoryMap);
                PcdSet32(PcdMemoryMapSize, header->Length - sizeof(UEFI_CONFIG_HEADER));
                requiredStructures.UefiConfigMemoryMap = 1;
                break;

            case UefiConfigEntropy:
                UEFI_CONFIG_ENTROPY *entropy = (UEFI_CONFIG_ENTROPY*) header;
                PcdSet64(PcdEntropyPtr, (UINT64) entropy->Entropy);
                requiredStructures.UefiConfigEntropy = 1;
                break;

            case UefiConfigBiosGuid:
                UEFI_CONFIG_BIOS_GUID *biosGuid = (UEFI_CONFIG_BIOS_GUID*) header;
                PcdSet64(PcdBiosGuidPtr, (UINT64) biosGuid->BiosGuid);
                requiredStructures.UefiConfigBiosGuid = 1;
                break;

            case UefiConfigSmbiosSystemManufacturer:
                UEFI_CONFIG_SMBIOS_SYSTEM_MANUFACTURER *systemManufacturer = (UEFI_CONFIG_SMBIOS_SYSTEM_MANUFACTURER*) header;
                GetSmbiosStructureStringLength(header->Length, systemManufacturer->SystemManufacturer, &stringLength);
                PcdSet64(PcdSmbiosSystemManufacturerStr, (UINT64)systemManufacturer->SystemManufacturer);
                PcdSet32(PcdSmbiosSystemManufacturerSize, stringLength);

                break;

            case UefiConfigSmbiosSystemProductName:
                UEFI_CONFIG_SMBIOS_SYSTEM_PRODUCT_NAME *systemProductName = (UEFI_CONFIG_SMBIOS_SYSTEM_PRODUCT_NAME*) header;
                GetSmbiosStructureStringLength(header->Length, systemProductName->SystemProductName, &stringLength);
                PcdSet64(PcdSmbiosSystemProductNameStr, (UINT64)systemProductName->SystemProductName);
                PcdSet32(PcdSmbiosSystemProductNameSize, stringLength);

                break;

            case UefiConfigSmbiosSystemVersion:
                UEFI_CONFIG_SMBIOS_SYSTEM_VERSION *systemVersion = (UEFI_CONFIG_SMBIOS_SYSTEM_VERSION*) header;
                GetSmbiosStructureStringLength(header->Length, systemVersion->SystemVersion, &stringLength);
                PcdSet64(PcdSmbiosSystemVersionStr, (UINT64)systemVersion->SystemVersion);
                PcdSet32(PcdSmbiosSystemVersionSize, stringLength);

                break;

            case UefiConfigSmbiosSystemSerialNumber:
                UEFI_CONFIG_SMBIOS_SYSTEM_SERIAL_NUMBER *systemSerialNumber = (UEFI_CONFIG_SMBIOS_SYSTEM_SERIAL_NUMBER*) header;
                GetSmbiosStructureStringLength(header->Length, systemSerialNumber->SystemSerialNumber, &stringLength);
                PcdSet64(PcdSmbiosSystemSerialNumberStr, (UINT64)systemSerialNumber->SystemSerialNumber);
                PcdSet32(PcdSmbiosSystemSerialNumberSize, stringLength);

                break;

            case UefiConfigSmbiosSystemSKUNumber:
                UEFI_CONFIG_SMBIOS_SYSTEM_SKU_NUMBER *systemSKUNumber = (UEFI_CONFIG_SMBIOS_SYSTEM_SKU_NUMBER*) header;
                GetSmbiosStructureStringLength(header->Length, systemSKUNumber->SystemSKUNumber, &stringLength);
                PcdSet64(PcdSmbiosSystemSKUNumberStr, (UINT64)systemSKUNumber->SystemSKUNumber);
                PcdSet32(PcdSmbiosSystemSKUNumberSize, stringLength);

                break;

            case UefiConfigSmbiosSystemFamily:
                UEFI_CONFIG_SMBIOS_SYSTEM_FAMILY *systemFamily = (UEFI_CONFIG_SMBIOS_SYSTEM_FAMILY*) header;
                GetSmbiosStructureStringLength(header->Length, systemFamily->SystemFamily, &stringLength);
                PcdSet64(PcdSmbiosSystemFamilyStr, (UINT64)systemFamily->SystemFamily);
                PcdSet32(PcdSmbiosSystemFamilySize, stringLength);

                break;

            case UefiConfigSmbiosBaseSerialNumber:
                UEFI_CONFIG_SMBIOS_BASE_SERIAL_NUMBER *baseSerialNumber = (UEFI_CONFIG_SMBIOS_BASE_SERIAL_NUMBER*) header;
                GetSmbiosStructureStringLength(header->Length, baseSerialNumber->BaseSerialNumber, &stringLength);
                PcdSet64(PcdSmbiosBaseSerialNumberStr, (UINT64)baseSerialNumber->BaseSerialNumber);
                PcdSet32(PcdSmbiosBaseSerialNumberSize, stringLength);

                break;

            case UefiConfigSmbiosChassisSerialNumber:
                UEFI_CONFIG_SMBIOS_CHASSIS_SERIAL_NUMBER *chassisSerialNumber = (UEFI_CONFIG_SMBIOS_CHASSIS_SERIAL_NUMBER*) header;
                GetSmbiosStructureStringLength(header->Length, chassisSerialNumber->ChassisSerialNumber, &stringLength);
                PcdSet64(PcdSmbiosChassisSerialNumberStr, (UINT64)chassisSerialNumber->ChassisSerialNumber);
                PcdSet32(PcdSmbiosChassisSerialNumberSize, stringLength);

                break;

            case UefiConfigSmbiosChassisAssetTag:
                UEFI_CONFIG_SMBIOS_CHASSIS_ASSET_TAG *chassisAssetTag = (UEFI_CONFIG_SMBIOS_CHASSIS_ASSET_TAG*) header;
                PcdSet64(PcdSmbiosChassisAssetTagStr, (UINT64) chassisAssetTag->ChassisAssetTag);
                GetSmbiosStructureStringLength(header->Length, chassisAssetTag->ChassisAssetTag, &stringLength);
                PcdSet32(PcdSmbiosChassisAssetTagSize, stringLength);

                break;

            case UefiConfigSmbiosBiosLockString:
                UEFI_CONFIG_SMBIOS_BIOS_LOCK_STRING *biosLockString = (UEFI_CONFIG_SMBIOS_BIOS_LOCK_STRING*) header;
                GetSmbiosStructureStringLength(header->Length, biosLockString->BiosLockString, &stringLength);
                PcdSet64(PcdSmbiosBiosLockStringStr, (UINT64)biosLockString->BiosLockString);
                PcdSet32(PcdSmbiosBiosLockStringSize, stringLength);

                break;

            case UefiConfigSmbiosMemoryDeviceSerialNumber:
                UEFI_CONFIG_SMBIOS_MEMORY_DEVICE_SERIAL_NUMBER *memoryDeviceSerialNumber = (UEFI_CONFIG_SMBIOS_MEMORY_DEVICE_SERIAL_NUMBER*) header;
                GetSmbiosStructureStringLength(header->Length, memoryDeviceSerialNumber->MemoryDeviceSerialNumber, &stringLength);
                PcdSet64(PcdSmbiosMemoryDeviceSerialNumberStr, (UINT64)memoryDeviceSerialNumber->MemoryDeviceSerialNumber);
                PcdSet32(PcdSmbiosMemoryDeviceSerialNumberSize, stringLength);

                break;

            case UefiConfigSmbios31ProcessorInformation:
                UEFI_CONFIG_SMBIOS_3_1_PROCESSOR_INFORMATION *procInfo = (UEFI_CONFIG_SMBIOS_3_1_PROCESSOR_INFORMATION*) header;
                PcdSet8(PcdSmbiosProcessorType, procInfo->ProcessorType);
                PcdSet64(PcdSmbiosProcessorID, procInfo->ProcessorID);
                PcdSet8(PcdSmbiosProcessorVoltage, procInfo->Voltage);
                PcdSet16(PcdSmbiosProcessorExternalClock, procInfo->ExternalClock);
                PcdSet16(PcdSmbiosProcessorMaxSpeed, procInfo->MaxSpeed);
                PcdSet16(PcdSmbiosProcessorCurrentSpeed, procInfo->CurrentSpeed);
                PcdSet8(PcdSmbiosProcessorStatus, procInfo->Status);
                PcdSet8(PcdSmbiosProcessorUpgrade, procInfo->ProcessorUpgrade);
                PcdSet16(PcdSmbiosProcessorCharacteristics, procInfo->ProcessorCharacteristics);
                PcdSet16(PcdSmbiosProcessorFamily2, procInfo->ProcessorFamily2);
                break;

            case UefiConfigSmbiosSocketDesignation:
                UEFI_CONFIG_SMBIOS_SOCKET_DESIGNATION *socketDesignation = (UEFI_CONFIG_SMBIOS_SOCKET_DESIGNATION*) header;
                PcdSet64(PcdSmbiosProcessorSocketDesignationStr, (UINT64) socketDesignation->SocketDesignation);
                GetSmbiosStructureStringLength(header->Length, socketDesignation->SocketDesignation, &stringLength);
                PcdSet32(PcdSmbiosProcessorSocketDesignationSize, stringLength);

                break;

            case UefiConfigSmbiosProcessorManufacturer:
                UEFI_CONFIG_SMBIOS_PROCESSOR_MANUFACTURER *processorManufacturer = (UEFI_CONFIG_SMBIOS_PROCESSOR_MANUFACTURER*) header;
                PcdSet64(PcdSmbiosProcessorManufacturerStr, (UINT64) processorManufacturer->ProcessorManufacturer);
                GetSmbiosStructureStringLength(header->Length, processorManufacturer->ProcessorManufacturer, &stringLength);
                PcdSet32(PcdSmbiosProcessorManufacturerSize, stringLength);

                break;

            case UefiConfigSmbiosProcessorVersion:
                UEFI_CONFIG_SMBIOS_PROCESSOR_VERSION *processorVersion = (UEFI_CONFIG_SMBIOS_PROCESSOR_VERSION*) header;
                PcdSet64(PcdSmbiosProcessorVersionStr, (UINT64) processorVersion->ProcessorVersion);
                GetSmbiosStructureStringLength(header->Length, processorVersion->ProcessorVersion, &stringLength);
                PcdSet32(PcdSmbiosProcessorVersionSize, stringLength);

                break;

            case UefiConfigSmbiosProcessorSerialNumber:
                UEFI_CONFIG_SMBIOS_PROCESSOR_SERIAL_NUMBER *processorSerialNumber = (UEFI_CONFIG_SMBIOS_PROCESSOR_SERIAL_NUMBER*) header;
                PcdSet64(PcdSmbiosProcessorSerialNumberStr, (UINT64) processorSerialNumber->ProcessorSerialNumber);
                GetSmbiosStructureStringLength(header->Length, processorSerialNumber->ProcessorSerialNumber, &stringLength);
                PcdSet32(PcdSmbiosProcessorSerialNumberSize, stringLength);

                break;

            case UefiConfigSmbiosProcessorAssetTag:
                UEFI_CONFIG_SMBIOS_PROCESSOR_ASSET_TAG *processorAssetTag = (UEFI_CONFIG_SMBIOS_PROCESSOR_ASSET_TAG*) header;
                PcdSet64(PcdSmbiosProcessorAssetTagStr, (UINT64) processorAssetTag->ProcessorAssetTag);
                GetSmbiosStructureStringLength(header->Length, processorAssetTag->ProcessorAssetTag, &stringLength);
                PcdSet32(PcdSmbiosProcessorAssetTagSize, stringLength);

                break;

            case UefiConfigSmbiosProcessorPartNumber:
                UEFI_CONFIG_SMBIOS_PROCESSOR_PART_NUMBER *processorPartNumber = (UEFI_CONFIG_SMBIOS_PROCESSOR_PART_NUMBER*) header;
                PcdSet64(PcdSmbiosProcessorAssetTagStr, (UINT64) processorPartNumber->ProcessorPartNumber);
                GetSmbiosStructureStringLength(header->Length, processorPartNumber->ProcessorPartNumber, &stringLength);
                PcdSet32(PcdSmbiosProcessorAssetTagSize, stringLength);

                break;

            case UefiConfigFlags:
                UEFI_CONFIG_FLAGS *flags = (UEFI_CONFIG_FLAGS*) header;
                ConfigSetUefiConfigFlags(flags);
                requiredStructures.UefiConfigFlags = 1;
                break;

            case UefiConfigProcessorInformation:
                UEFI_CONFIG_PROCESSOR_INFORMATION *processorInfo = (UEFI_CONFIG_PROCESSOR_INFORMATION*) header;
                ConfigSetProcessorInfo(processorInfo);
                requiredStructures.UefiConfigProcessorInformation = 1;
                break;

            case UefiConfigMmioRanges:
                UINT64 lowGap, highGap;
                UEFI_CONFIG_MMIO_RANGES *mmioRanges = (UEFI_CONFIG_MMIO_RANGES*) header;

                //
                // Size must be exactly two MMIO entries.
                //
                if (header->Length != (sizeof(UEFI_CONFIG_HEADER) + sizeof(UEFI_CONFIG_MMIO) * 2))
                {
                    DEBUG((DEBUG_ERROR, "***Malformed MMIO range structure\n"));
                    CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(CONFIG, __LINE__);
                }

                //
                // Figure out which entry is the low gap, and which is the high.
                //
                if (mmioRanges->Ranges[0].MmioPageNumberStart < mmioRanges->Ranges[1].MmioPageNumberStart)
                {
                    lowGap = 0;
                    highGap = 1;
                }
                else
                {
                    lowGap = 1;
                    highGap = 0;
                }

                PcdSet64(PcdLowMmioGapBasePageNumber, mmioRanges->Ranges[lowGap].MmioPageNumberStart);
                PcdSet64(PcdLowMmioGapSizeInPages, mmioRanges->Ranges[lowGap].MmioSizeInPages);
                PcdSet64(PcdHighMmioGapBasePageNumber, mmioRanges->Ranges[highGap].MmioPageNumberStart);
                PcdSet64(PcdHighMmioGapSizeInPages, mmioRanges->Ranges[highGap].MmioSizeInPages);
                requiredStructures.UefiConfigMmioRanges = 1;
                break;

            case UefiConfigAcpiTable:
                UEFI_CONFIG_ACPI_TABLE *acpiTable = (UEFI_CONFIG_ACPI_TABLE*) header;
                EFI_ACPI_DESCRIPTION_HEADER *acpiHeader = (EFI_ACPI_DESCRIPTION_HEADER*) acpiTable->AcpiTableData;

                //
                // Verify ACPI table header is completely within the config structure.
                //
                if (acpiTable->Header.Length < (sizeof(UEFI_CONFIG_HEADER) + sizeof(EFI_ACPI_DESCRIPTION_HEADER)) ||
                    acpiHeader->Length > (acpiTable->Header.Length - sizeof(UEFI_CONFIG_HEADER)))
                {
                    DEBUG((DEBUG_ERROR, "***ACPI table is not contained within config structure size.\n"));
                    CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(CONFIG, __LINE__);
                }

                PcdSet64(PcdAcpiTablePtr, (UINT64) acpiTable->AcpiTableData);
                PcdSet32(PcdAcpiTableSize, acpiHeader->Length);
                break;

            case UefiConfigNvdimmCount:
                UEFI_CONFIG_NVDIMM_COUNT *cfg = (UEFI_CONFIG_NVDIMM_COUNT*) header;
                PcdSet16(PcdNvdimmCount, cfg->Count);
                break;

            case UefiConfigVpciInstanceFilter:
                UEFI_CONFIG_VPCI_INSTANCE_FILTER *filter = (UEFI_CONFIG_VPCI_INSTANCE_FILTER*) header;
                PcdSet64(PcdVpciInstanceFilterGuidPtr, (UINT64) filter->InstanceGuid);
                break;
        }

        calculatedConfigSize += header->Length;
        header = (UEFI_CONFIG_HEADER*) ((UINT64) header + header->Length);
    }

    if (requiredStructures.AsUINT64 != AllStructuresFound)
    {
        DEBUG((DEBUG_ERROR, "Missing required structures, found structures: 0x%x\n", requiredStructures.AsUINT64));
        CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(CONFIG, __LINE__);
    }

    if (configCount->TotalConfigBlobSize != calculatedConfigSize)
    {
        DEBUG((DEBUG_ERROR, "Reported config size of 0x%x did not match actual size of 0x%x\n", configCount->TotalConfigBlobSize, calculatedConfigSize));
        CONFIG_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(CONFIG, __LINE__);
    }

    return EFI_SUCCESS;
}

EFI_STATUS
GetConfiguration(
    _In_ CONST EFI_PEI_SERVICES**  PeiServices,
    _Out_ UINT8* PhysicalAddressWidth
    )
/*++

Routine Description:

    Gets the configuraton from the worker process.

Arguments:

    PeiServices  An indirect pointer to the PEI Services Table.

    PhysicalAddressWidth - Returns the number of bits in the address width.

Return Value:

    n/a

--*/
{
    EFI_STATUS status;

    //
    // If this is a hardware-isolated VM running without a paravisor, then no
    // config blob is present.  Instead, the parameters were inserted in IGVM
    // format and must be parsed as such.
    //

    if ((PcdGet32(PcdIsolationArchitecture) >= UefiIsolationTypeSnp) &&
        (PcdGetBool(PcdIsolationParavisorPresent) == FALSE))
    {
        status = GetIgvmConfigInfo();
    }
    else
    {
        status = GetUefiConfigInfo();
    }

    //
    // The config blob was not well formed, do not proceed.
    //
    if (EFI_ERROR(status))
    {
        ASSERT(FALSE);
        return status;
    }

    //
    // Get the address width.
    //
    *PhysicalAddressWidth = GetPhysicalAddressWidth(PeiServices);

    return EFI_SUCCESS;
}
