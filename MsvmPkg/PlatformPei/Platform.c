/*++

Copyright (c) Microsoft Corporation

Module Name:

    platform.c

Abstract:

    This is the Hyper-V "Platform" PEI Module. It initializes in preparation
    for running other PEI Modules and eventually running DXE Core.

--*/

#include <PiPei.h>
#include <EfiNt.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/ResourcePublicationLib.h>
#include <Library/MtrrLib.h>
#include <Library/HobLib.h>
#include <Guid/MemoryTypeInformation.h>
#include <Ppi/MasterBootMode.h>
#include <IndustryStandard/Acpi.h>
#include <Ppi/ConfigPpi.h>
#include <BiosInterface.h>
#include <BiosDeviceAccess.h>
#include <Hob.h>


//
// Values and type used with CPUID to get the physical address width.
//
#define CPUID_FUNCTION_EXTENDED_MAX_FUNCTION        0x80000000
#define CPUID_FUNCTION_EXTENDED_ADDRESS_SPACE_SIZES 0x80000008

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


//
// Initial data for Memory Type Information HOB.
//
// Initial values here are irrelevant.
//
static EFI_MEMORY_TYPE_INFORMATION MsvmDefaultMemoryTypeInformation[] =
{
    { EfiACPIMemoryNVS,       0x004 },
    { EfiACPIReclaimMemory,   0x008 },
    { EfiReservedMemoryType,  0x004 },
    { EfiRuntimeServicesData, 0x024 },
    { EfiRuntimeServicesCode, 0x030 },
    { EfiBootServicesCode,    0x180 },
    { EfiBootServicesData,    0xF00 },
    { EfiMaxMemoryType,       0x000 }
};


//
// Boot Mode PPI.
//
static EFI_PEI_PPI_DESCRIPTOR MsvmBootModePpiDescriptor[] =
{
    {
        EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
        &gEfiPeiMasterBootModePpiGuid,
        NULL
    }
};

VOID
DebugDumpSrat(
    _In_ VOID* Srat
    )
{
#if !defined(MDEPKG_NDEBUG)
    EFI_ACPI_DESCRIPTION_HEADER  *acpiHdr = (EFI_ACPI_DESCRIPTION_HEADER*) Srat;
    EFI_ACPI_5_0_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY_STRUCTURE *sratApic;
    EFI_ACPI_5_0_MEMORY_AFFINITY_STRUCTURE *sratMem;
    UINTN base, size;
    UINT8 *cursor;

    //
    // Debug dump the SRAT entries.
    //
    DEBUG((DEBUG_VERBOSE, "--- SRAT data @ %x\n", acpiHdr));
    DEBUG((DEBUG_VERBOSE, "    Header Signature %x\n", acpiHdr->Signature));
    DEBUG((DEBUG_VERBOSE, "    Length %x\n", acpiHdr->Length));

    cursor = (UINT8 *)acpiHdr;
    cursor += sizeof(EFI_ACPI_4_0_SYSTEM_RESOURCE_AFFINITY_TABLE_HEADER);

    do
    {
        switch(*cursor)  // UINT8 sized Type always at start of struct
        {
            case EFI_ACPI_5_0_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY:
                sratApic = (EFI_ACPI_5_0_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY_STRUCTURE *)cursor;

                DEBUG((DEBUG_VERBOSE, "    APIC Type %x Len %02x Flags %02x ApicId %02x Dom %x\n",
                    sratApic->Type, sratApic->Length, sratApic->Flags, sratApic->ApicId,
                    sratApic->ProximityDomain7To0));

                cursor += sratApic->Length;
                break;

            case EFI_ACPI_5_0_MEMORY_AFFINITY:
                sratMem = (EFI_ACPI_5_0_MEMORY_AFFINITY_STRUCTURE *)cursor;

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
                sratMem = (EFI_ACPI_5_0_MEMORY_AFFINITY_STRUCTURE *)cursor;

                DEBUG((DEBUG_VERBOSE, "    *Skipping* Type %x\n", sratMem->Type));

                cursor += sratMem->Length;
                break;
        }
    } while (cursor < ((UINT8 *)acpiHdr + acpiHdr->Length));
#endif
}

VOID
DebugDumpMemoryMap(
    _In_ VOID* MemMap,
    _In_ UINT32 MemMapSize,
    _In_ UINT32 VDevVersion
    )
{
    //
    // Debug dump the Memory Map entries.
    //
#if !defined(MDEPKG_NDEBUG)
    DEBUG((DEBUG_VERBOSE, "--- Memory Map data @ %x Length %x\n", MemMap, MemMapSize));
    if (VDevVersion >= VDevVersion5)
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
    //
    // On X64, the config blob starts after the end of the firmware, and after
    // the 6 pages for pagetables, and 1 page for GDT entries.
    //
    UINT64 configBlobBase = PcdGet32(PcdMsvmFvBase) + PcdGet32(PcdMsvmFvSize) + SIZE_4KB * 7;
    return (UEFI_CONFIG_HEADER*)(UINTN)configBlobBase;
}

UINTN
GetPageTableSize(
    _In_ CONST UINT8 PhysicalAddressBits
    )
/*++

Routine Description:

    Calculates the page table size.

Arguments:

    PhysicalAddressBits - The physical address width of the CPU in bits.

Return Value:

    The page table size.

--*/
{
    BOOLEAN pcdUse1GPageTable;
    BOOLEAN page1GSupport;
    UINT32  regEax;
    UINT32  regEdx;
    UINT32  pml4Entries;
    UINT32  pdpEntries;
    UINTN   totalPages;

    DEBUG((DEBUG_VERBOSE, ">>> GetPageTableSize(%d)\n", PhysicalAddressBits));

    //
    // If DXE is 32-bit return zero.
    //
#ifdef MDE_CPU_IA32
    if (!FeaturePcdGet(PcdDxeIplSwitchToLongMode)) {
    return 0;
    }
#endif

    //
    // The code below is based on CreateIdentityMappingPageTables() in
    // "MdeModulePkg/Core/DxeIplPeim/X64/VirtualMemory.c".
    //
    page1GSupport = FALSE;
    pcdUse1GPageTable = PcdGetBool(PcdUse1GPageTable);
    DEBUG((DEBUG_VERBOSE, "PcdUse1GPageTable is %a\n", pcdUse1GPageTable ? "TRUE" : "FALSE" ));
    if (pcdUse1GPageTable)
    {
        AsmCpuid(0x80000000, &regEax, NULL, NULL, NULL);
        if (regEax >= 0x80000001)
        {
            AsmCpuid(0x80000001, NULL, NULL, NULL, &regEdx);
            if ((regEdx & BIT26) != 0)
            {
                page1GSupport = TRUE;
            }
        }
    }
    DEBUG((DEBUG_VERBOSE, "page1GSupport is %a\n", page1GSupport ? "TRUE" : "FALSE"));

    if (PhysicalAddressBits <= 39)
    {
        pml4Entries = 1;
        pdpEntries = 1 << (PhysicalAddressBits - 30);
        ASSERT(pdpEntries <= 0x200);
    }
    else
    {
        pml4Entries = 1 << (PhysicalAddressBits - 39);
        ASSERT(pml4Entries <= 0x200);
        pdpEntries = 512;
    }

    totalPages = page1GSupport ? pml4Entries + 1 :
                               (pdpEntries + 1) * pml4Entries + 1;
    ASSERT(totalPages <= 0x40201);

    DEBUG((DEBUG_VERBOSE, "<<< GetPageTableSize returning %lu\n",
        (UINTN)EFI_PAGES_TO_SIZE(totalPages)));

    return (UINTN)(EFI_PAGES_TO_SIZE(totalPages));
}

VOID
AddFirstMemoryRange(
    _In_ CONST UINT8 PhysicalAddressBits,
    _In_ CONST UINT64 Length
)
/*++

Routine Description:

    Utility function to handle special case of memory range zero
    that is not fully described by the vm worker process memory range.
    This function has the side effect of initializing PEI memory
    so the HOB add functions can be used.

Arguments:

    PhysicalAddressBits - The physical address width of the CPU in bits.

    Length - The size in bytes of the first memory range.

    BiosSize - The size in bytes of the firmware image.

Return Value:

    None.

--*/
{
    EFI_STATUS status;
    UINT64 peiBase, peiSize;
    UINT64 pageTableSize;
    UINT32 configBlobSize = PcdGet32(PcdConfigBlobSize);
    UINT64 configBlobBase = (UINT64) GetStartOfConfigBlob();
    DEBUG((DEBUG_VERBOSE, ">>> AddFirstMemoryRange\n"));

    //
    // Round config blob size to 4K page increment.
    //
    if (configBlobSize % SIZE_4KB != 0)
    {
        configBlobSize = ((configBlobSize / SIZE_4KB) + 1) * SIZE_4KB;
    }

    //
    // Establish PEI memory first so we can create HOBs in the formal PEI heap.
    // This first memory range is, by design, the memory below the MMIO range below 4GB.
    // The base and size of PEI memory is constrained by several things:
    // - Avoid the 0-1MB range, so base the PEI memory at 1MB.
    // - Don't consume more than really necessary, 64MB is sufficient for misc pei allocations
    // - Try to include a page table on x64 that can be large when cpu address width is large
    //
    // Insufficient room for a large page table is not fatal as the DXE page table creation
    // code has been updated to fall back to a smaller table.  This will still permit really
    // small VMs on machines with lots of address bits.
    //
    // Exclude the region occupied by the firmware image, along with the the
    // config blob and other data.
    //
    pageTableSize = GetPageTableSize(PhysicalAddressBits);
    peiBase = configBlobBase + configBlobSize;
    peiSize = MIN((Length - peiBase), (pageTableSize + SIZE_64MB));
    DEBUG((DEBUG_VERBOSE, "AddFirstMemoryRange: peiBase %lx peiSize %lx\n", peiBase, peiSize));
    ASSERT((peiBase + peiSize) <= Length);
    status = PublishSystemMemory(peiBase, peiSize);
    ASSERT_EFI_ERROR(status);

    //
    // The first memory range is special in that we have to account for
    // two special cases within it.
    //
    // 1) Even though the host actually puts memory between GPA 640K and 768K
    //    it can't be declared as existing. Linux fails to boot if memory
    //    is declared there. This happens to be the PCAT legacy VGA MMIO range.
    //
    // 2) The memory between 768K and 1MB exists but can't be declared as
    //    regular system memory.  At least one Windows boot driver (Intel
    //    iaStorAV) attempts to access this area with MmMapIoSpace. If this
    //    memory is marked system memory that can apparently trigger a bugcheck.
    //    Therefore this slice is marked reserved. It exists but shouldn't
    //    really be used.
    //
    //
    //           top +---------------------------------------------------
    //               | System Memory
    // 1MB  0x100000 +---------------------------------------------------
    //               | Reserved Memory - legacy device ROM & BIOS
    // 768KB 0xC0000 +---------------------------------------------------
    //                 Empty           - legacy VGA MMIO
    // 640KB 0xA0000 +---------------------------------------------------
    //               | System Memory
    //           0x0 +---------------------------------------------------
    //

    //
    // Declare System Memory from 0 to 640K.
    //
    HobAddMemoryRange(0, SIZE_512KB + SIZE_128KB);

    //
    // Skip the range from 640K to 768K (legacy VGA MMIO range) by not
    // declaring anything in that range.
    //

    //
    // Declare Reserved Memory from 768K to 1MB.
    //
    HobAddReservedMemoryRange(BASE_512KB + SIZE_256KB, SIZE_256KB);

    //
    // Declare System Memory for everything else.
    //
    HobAddMemoryRange(BASE_1MB, Length - SIZE_1MB);

    //
    // Mark the region occupied by the firmware, along with the page tables, GDT
    // entries and config blob as allocated, which should allow it to be
    // reclaimed by the guest OS.
    //
    UINT64 reservedBlockSize = PcdGet32(PcdMsvmFvSize) + SIZE_4KB * 7 + configBlobSize;
    HobAddAllocatedMemoryRange(PcdGet32(PcdMsvmFvBase), reservedBlockSize);

    DEBUG((DEBUG_VERBOSE, "<<< AddFirstMemoryRange\n"));
}


VOID
InitializeMemoryMap(
    _In_ CONST UINT32 VDevVersion
    )
/*++

Routine Description:

    Initializes the memory map of the vm by creating appropriate HOBs and
    triggering the MTRRs to be initialized.

Arguments:

    VDevVersion - The version of the BIOS VDev.

Return Value:

    None.

--*/
{
    CPUID_ADDRESS_SPACE_SIZES addressSpaceSizes;
    UINT8 physicalAddressBits;
    UINT32 maximumFunction;
    UINT32 memMapSize = PcdGet32(PcdMemoryMapSize);
    VOID* memMap = (VOID*)(UINTN) PcdGet64(PcdMemoryMapPtr);

    DEBUG((DEBUG_VERBOSE, ">>> InitializeMemoryMap\n"));
    DEBUG((DEBUG_VERBOSE, "VDevVersion is %d.%d\n", VDevVersion >> 8, VDevVersion & 0xFF));
    //
    // Determine the number of physical address bits.
    //
    AsmCpuid(CPUID_FUNCTION_EXTENDED_MAX_FUNCTION, &maximumFunction, NULL, NULL, NULL);
    if (maximumFunction >= CPUID_FUNCTION_EXTENDED_ADDRESS_SPACE_SIZES)
    {
        AsmCpuid(CPUID_FUNCTION_EXTENDED_ADDRESS_SPACE_SIZES,
                 &addressSpaceSizes.Value,
                 NULL,
                 NULL,
                 NULL);

        physicalAddressBits = addressSpaceSizes.PhysicalAddressBits;

        //
        // Limit to max 48 bits of address width as that is the design in DXE.
        //
        if (physicalAddressBits > 48)
        {
            DEBUG((DEBUG_WARN, "--- InitializeMemoryMap limiting address width to 48bits\n"));
            physicalAddressBits = 48;
        }
    }
    else
    {
        //
        // The processor does not support the required query, which means
        // the processor only supports 36 physical address bits.
        //
        DEBUG((DEBUG_WARN, "Can't query CPUID so defaulting address width to 36 bits\n"));
        physicalAddressBits = 36;
    }
    ASSERT(physicalAddressBits >= 36 && physicalAddressBits <= 48);
    DEBUG((DEBUG_VERBOSE, "PhysicalAddressbits %d\n", physicalAddressBits));

    //
    // Process the memory map and create HOBs for memory regions..
    //
    switch (VDevVersion)
    {
        case VDevVersion2:
        case VDevVersion3:
        case VDevVersion4:
        {
            //
            // VDev versions 3 & 4
            //
            // A memory map range contains only base address and length.
            //
            // Loop through the Memory Map and create HOBs for RAM regions.
            //
            ASSERT(memMap != NULL);
            PVM_MEMORY_RANGE range = (PVM_MEMORY_RANGE)memMap;
            do
            {
                DEBUG((DEBUG_VERBOSE, "Range BaseAddress %lx \n", range->BaseAddress));
                DEBUG((DEBUG_VERBOSE, "Range Length      %lx \n", range->Length));
                //
                // First memory region is a special case that isn't fully
                // described in the Memory Map.
                //
                if (range->BaseAddress == 0)
                {
                    AddFirstMemoryRange(
                        physicalAddressBits,
                        range->Length);
                }
                else
                {
                    HobAddMemoryRange(range->BaseAddress, range->Length);
                }

                //
                // Next memory map range.
                //
                range++;
            } while ((UINT8*)range < ((UINT8*)memMap + memMapSize));
        }
        break;

        case VDevVersion5:
        default:
        {
            //
            // VDev version 5 and up
            //
            // A memory map range now contains base address, length, and attribute flags.
            // The reserved bit allows for support of Intel SGX memory.
            //
            // Loop through the Memory Map and create HOBs for RAM regions.
            //
            ASSERT(memMap != NULL);
            ASSERT(memMapSize % sizeof(VM_MEMORY_RANGE_V5) == 0);
            PVM_MEMORY_RANGE_V5 rangeV5 = (PVM_MEMORY_RANGE_V5)memMap;
            do
            {
                DEBUG((DEBUG_VERBOSE, "BaseAddress %lx \n", rangeV5->BaseAddress));
                DEBUG((DEBUG_VERBOSE, "Length      %lx \n", rangeV5->Length));
                DEBUG((DEBUG_VERBOSE, "Flags       %x \n", rangeV5->Flags));

                //
                // First memory region is a special case that isn't fully
                // described in the Memory Map.
                //
                if (rangeV5->BaseAddress == 0)
                {
                    AddFirstMemoryRange(
                        physicalAddressBits,
                        rangeV5->Length);
                }
                else
                {
                    //
                    // Report subsequent memory regions directly.
                    //
                    if (rangeV5->Flags & VM_MEMORY_RANGE_FLAG_PLATFORM_RESERVED)
                    {
                        HobAddReservedMemoryRange(rangeV5->BaseAddress, rangeV5->Length);
                    }
                    else if (rangeV5->Flags & VM_MEMORY_RANGE_FLAG_PERSISTENT_MEMORY)
                    {
                        HobAddPersistentMemoryRange(rangeV5->BaseAddress, rangeV5->Length);
                    }
                    else
                    {
                        HobAddMemoryRange(rangeV5->BaseAddress, rangeV5->Length);
                    }
                }

                //
                // Next memory map range.
                //
                rangeV5++;
            } while ((UINT8*)rangeV5 < ((UINT8*)memMap + memMapSize));
        }
        break;
    }

    //
    // Initialize the fixed MTRR for low memory.
    // The variable MTRRs are set later in this function with a trigger to
    // the VDev.
    //
    // N.B. This call also has the effect of enabling MTRRs. The default
    // MTRR type remains uncached.
    //
    MtrrSetMemoryAttribute(0, SIZE_512KB + SIZE_128KB, CacheWriteBack);

    //
    // Low and high MMIO range
    //
    HobAddMmioRange(
        PcdGet64(PcdLowMmioGapBasePageNumber) * SIZE_4KB,
        PcdGet64(PcdLowMmioGapSizeInPages) * SIZE_4KB
        );

    HobAddMmioRange(
        PcdGet64(PcdHighMmioGapBasePageNumber) * SIZE_4KB,
        PcdGet64(PcdHighMmioGapSizeInPages) * SIZE_4KB
        );

    //
    // Memory Type Information HOB
    //
    HobAddGuidData(
        &gEfiMemoryTypeInformationGuid,
        MsvmDefaultMemoryTypeInformation,
        sizeof(MsvmDefaultMemoryTypeInformation)
        );

    //
    // Add CPU HOB with resultant address width and 16-bits of IO space.
    //
    HobAddCpu(physicalAddressBits, 16);

    //
    // Tell the BiosDevice to set up the variable MTRRs.
    //
    WriteBiosDevice(BiosConfigBootFinalize, physicalAddressBits);

    DEBUG((DEBUG_VERBOSE, "<<< InitializeMemoryMap\n"));
}


VOID
InitializeWatchdog()
/*++

Routine Description:

    Initializes and starts the watchdog timer

    Note that until the Watchdog DXE driver is loaded, there is no entity
    to reset the watchdog count. This should not be an issue since the initial
    watchdog count is in minutes and the DXE driver should load within milliseconds

Arguments:

    None.

Return Value:

    None.

--*/
{
    UINT32 hwResolution;

    hwResolution = ReadBiosDevice(BiosConfigWatchdogResolution);

    if ((hwResolution != 0) && (hwResolution != BIOS_WATCHDOG_NOT_ENABLED))
    {
        //
        // Use one-shot mode and the default count for the watchdog device.
        // Directly program the watchdog registers since the WatchdogTimerLib
        // is only available for DXE drivers
        //
        WriteBiosDevice(BiosConfigWatchdogConfig, BIOS_WATCHDOG_RUNNING | BIOS_WATCHDOG_ONE_SHOT);
    }

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
            DEBUG((DEBUG_VERBOSE, "\tBiosSizePages: 0x%x\n\tBiosVdevVersion:0x%x\n", biosInfo->BiosSizePages, biosInfo->BiosVDevVersion));
            break;

        case UefiConfigSrat:
            UEFI_CONFIG_SRAT *srat = (UEFI_CONFIG_SRAT*) Header;
            DebugDumpSrat(srat->Srat);
            break;

        case UefiConfigMemoryMap:
            UEFI_CONFIG_MEMORY_MAP *memMap = (UEFI_CONFIG_MEMORY_MAP*) Header;
            DebugDumpMemoryMap(memMap->MemoryMap, Header->Length - sizeof(UEFI_CONFIG_HEADER), PcdGet32(PcdBiosVDevVersion));
            break;

        case UefiConfigEntropy:
            DEBUG((DEBUG_VERBOSE, "\tEntropy table found.\n"));
            break;

        case UefiConfigBiosGuid:
            UEFI_CONFIG_BIOS_GUID *biosGuid = (UEFI_CONFIG_BIOS_GUID*) Header;
            DEBUG((DEBUG_VERBOSE, "\tBiosGuid: %g\n", (EFI_GUID*) biosGuid->BiosGuid));
            break;

        case UefiConfigSmbiosSystemSerialNumber:
            UEFI_CONFIG_SMBIOS_SYSTEM_SERIAL_NUMBER *systemSerialNumber = (UEFI_CONFIG_SMBIOS_SYSTEM_SERIAL_NUMBER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios System Serial Number: %s\n", systemSerialNumber->SystemSerialNumber));
            break;

        case UefiConfigSmbiosBaseSerialNumber:
            UEFI_CONFIG_SMBIOS_BASE_SERIAL_NUMBER *baseSerialNumber = (UEFI_CONFIG_SMBIOS_BASE_SERIAL_NUMBER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Base Serial Number: %s\n", baseSerialNumber->BaseSerialNumber));
            break;

        case UefiConfigSmbiosChassisSerialNumber:
            UEFI_CONFIG_SMBIOS_CHASSIS_SERIAL_NUMBER *chassisSerialNumber = (UEFI_CONFIG_SMBIOS_CHASSIS_SERIAL_NUMBER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Chassis Serial Number: %s\n", chassisSerialNumber->ChassisSerialNumber));
            break;

        case UefiConfigSmbiosChassisAssetTag:
            UEFI_CONFIG_SMBIOS_CHASSIS_ASSET_TAG *chassisAssetTag = (UEFI_CONFIG_SMBIOS_CHASSIS_ASSET_TAG*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Chassis Asset Tag: %s\n", chassisAssetTag->ChassisAssetTag));
            break;

        case UefiConfigSmbiosBiosLockString:
            UEFI_CONFIG_SMBIOS_BIOS_LOCK_STRING *biosLockString = (UEFI_CONFIG_SMBIOS_BIOS_LOCK_STRING*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Bios Lock String: %s\n", biosLockString->BiosLockString));
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
            DEBUG((DEBUG_VERBOSE, "\tSmbios Socket Designation: %s\n", socketDesignation->SocketDesignation));
            break;

        case UefiConfigSmbiosProcessorManufacturer:
            UEFI_CONFIG_SMBIOS_PROCESSOR_MANUFACTURER *processorManufacturer = (UEFI_CONFIG_SMBIOS_PROCESSOR_MANUFACTURER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Processor Manufacturer: %s\n", processorManufacturer->ProcessorManufacturer));
            break;

        case UefiConfigSmbiosProcessorVersion:
            UEFI_CONFIG_SMBIOS_PROCESSOR_VERSION *processorVersion = (UEFI_CONFIG_SMBIOS_PROCESSOR_VERSION*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Processor Version: %s\n", processorVersion->ProcessorVersion));
            break;

        case UefiConfigSmbiosProcessorSerialNumber:
            UEFI_CONFIG_SMBIOS_PROCESSOR_SERIAL_NUMBER *processorSerialNumber = (UEFI_CONFIG_SMBIOS_PROCESSOR_SERIAL_NUMBER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Processor Serial Number: %s\n", processorSerialNumber->ProcessorSerialNumber));
            break;

        case UefiConfigSmbiosProcessorAssetTag:
            UEFI_CONFIG_SMBIOS_PROCESSOR_ASSET_TAG *processorAssetTag = (UEFI_CONFIG_SMBIOS_PROCESSOR_ASSET_TAG*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Processor Asset Tag: %s\n", processorAssetTag->ProcessorAssetTag));
            break;

        case UefiConfigSmbiosProcessorPartNumber:
            UEFI_CONFIG_SMBIOS_PROCESSOR_PART_NUMBER *processorPartNumber = (UEFI_CONFIG_SMBIOS_PROCESSOR_PART_NUMBER*) Header;
            DEBUG((DEBUG_VERBOSE, "\tSmbios Processor Part Number: %s\n", processorPartNumber->ProcessorPartNumber));
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

        default:
            DEBUG((DEBUG_VERBOSE, "\t!!! Unrecognized config structure type !!!\n"));
            break;
    }
#endif
}

EFI_STATUS
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

    EFI_SUCCESS on success. EFI_INVALID_PARAMETER if HeaderLength is not long
    enough to contain a valid string.

--*/
{
    if (HeaderLength <= sizeof(UEFI_CONFIG_HEADER))
    {
        return EFI_INVALID_PARAMETER;
    }

    *StringLength = 0;
    UINT64 remainingStructureSize = HeaderLength - sizeof(UEFI_CONFIG_HEADER);

    UINTN length = AsciiStrnLenS(String, remainingStructureSize);

    if (length == remainingStructureSize)
    {
        //
        // No NULL found, truncate by adding one at the end.
        //
        String[length - 1] = 0;
        *StringLength = (UINT32) length;

        DEBUG((DEBUG_VERBOSE, "SMBIOS String Structure had no null terminator, truncating to size 0x%x. Truncated string:%s", length, String));
    }
    else
    {
        //
        // Add one to the length for the null character.
        //
        *StringLength = (UINT32) (length + 1);
    }

    return EFI_SUCCESS;
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
    //
    if (Header->Length % 8 != 0)
    {
        DEBUG((DEBUG_ERROR, "Structure Type 0x%x was length 0x%x, not aligned to 8 bytes.\n",
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
        0  //UefiConfigAARCH64MPIDR
    };

    //
    // If this is a type that is not currently parsed, ignore it.
    //
    if (Header->Type > UefiConfigAARCH64MPIDR)
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
        DEBUG((DEBUG_VERBOSE, "Structure Type 0x%x was length 0x%x, expected Length %x\n",
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

Arguments:

    None.

Return Value:

    EFI_SUCCESS on success. Error if the config blob is malformed.

--*/
{
    EFI_STATUS status;
    UINT32 stringLength = 0;
    UEFI_CONFIG_HEADER *header = NULL;
    UEFI_CONFIG_STRUCTURE_COUNT *configCount = NULL;
    UINT32 calculatedConfigSize = 0;

    //
    // Tracking to see if the config blob has all the required structures.
    //
    static const UINT64 AllStructuresFound = 0xFF;
    static union {
        struct {
            UINT64 UefiConfigBiosInformation:1;
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

    header = GetStartOfConfigBlob();

    //
    // Read the first structure, which must be the structure describing the
    // total number of structures.
    //
    DebugDumpUefiConfigStruct(header);
    configCount = (UEFI_CONFIG_STRUCTURE_COUNT*) header;

    //
    // Anything less than 1 structure the Bios VDEV failed to create the config.
    // Also sanity check the header.
    //
    if (header->Type != UefiConfigStructureCount ||
        configCount->TotalStructureCount <= 1 ||
        header->Length != sizeof(UEFI_CONFIG_STRUCTURE_COUNT))
    {
        ASSERT(FALSE);
        return EFI_DEVICE_ERROR;
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
        status = VerifyStructureLength(header);

        if (EFI_ERROR(status))
        {
            goto Failure;
        }

        DebugDumpUefiConfigStruct(header);

        switch(header->Type)
        {
            case UefiConfigBiosInformation:
                UEFI_CONFIG_BIOS_INFORMATION *biosInfo = (UEFI_CONFIG_BIOS_INFORMATION*) header;
                PcdSet32(PcdBiosVDevVersion, biosInfo->BiosVDevVersion);
                requiredStructures.UefiConfigBiosInformation = 1;
                break;

            case UefiConfigSrat:
                UEFI_CONFIG_SRAT *sratStructure = (UEFI_CONFIG_SRAT*) header;
                EFI_ACPI_DESCRIPTION_HEADER *acpiHdr = (EFI_ACPI_DESCRIPTION_HEADER*) sratStructure->Srat;

                if (sratStructure->Header.Length < (sizeof(UEFI_CONFIG_HEADER) + sizeof(EFI_ACPI_DESCRIPTION_HEADER)) ||
                    acpiHdr->Signature != EFI_ACPI_4_0_SYSTEM_RESOURCE_AFFINITY_TABLE_SIGNATURE ||
                    acpiHdr->Length != (sratStructure->Header.Length - sizeof(UEFI_CONFIG_HEADER)))
                {
                    DEBUG((DEBUG_ERROR, "*** Malformed SRAT\n"));
                    goto Failure;
                }

                PcdSet64(PcdSratPtr, (UINT64) sratStructure->Srat);
                PcdSet32(PcdSratSize, header->Length - sizeof(UEFI_CONFIG_HEADER));
                requiredStructures.UefiConfigSrat = 1;
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

            case UefiConfigSmbiosSystemSerialNumber:
                UEFI_CONFIG_SMBIOS_SYSTEM_SERIAL_NUMBER *systemSerialNumber = (UEFI_CONFIG_SMBIOS_SYSTEM_SERIAL_NUMBER*) header;
                PcdSet64(PcdSmbiosSystemSerialNumberStr, (UINT64) systemSerialNumber->SystemSerialNumber);
                status = GetSmbiosStructureStringLength(header->Length, systemSerialNumber->SystemSerialNumber, &stringLength);

                if (EFI_ERROR(status))
                {
                    goto Failure;
                }

                PcdSet32(PcdSmbiosSystemSerialNumberSize, stringLength);

                break;

            case UefiConfigSmbiosBaseSerialNumber:
                UEFI_CONFIG_SMBIOS_BASE_SERIAL_NUMBER *baseSerialNumber = (UEFI_CONFIG_SMBIOS_BASE_SERIAL_NUMBER*) header;
                PcdSet64(PcdSmbiosBaseSerialNumberStr, (UINT64) baseSerialNumber->BaseSerialNumber);
                status = GetSmbiosStructureStringLength(header->Length, baseSerialNumber->BaseSerialNumber, &stringLength);

                if (EFI_ERROR(status))
                {
                    goto Failure;
                }

                PcdSet32(PcdSmbiosBaseSerialNumberSize, stringLength);

                break;

            case UefiConfigSmbiosChassisSerialNumber:
                UEFI_CONFIG_SMBIOS_CHASSIS_SERIAL_NUMBER *chassisSerialNumber = (UEFI_CONFIG_SMBIOS_CHASSIS_SERIAL_NUMBER*) header;
                PcdSet64(PcdSmbiosChassisSerialNumberStr, (UINT64) chassisSerialNumber->ChassisSerialNumber);
                status = GetSmbiosStructureStringLength(header->Length, chassisSerialNumber->ChassisSerialNumber, &stringLength);

                if (EFI_ERROR(status))
                {
                    goto Failure;
                }

                PcdSet32(PcdSmbiosChassisSerialNumberSize, stringLength);

                break;

            case UefiConfigSmbiosChassisAssetTag:
                UEFI_CONFIG_SMBIOS_CHASSIS_ASSET_TAG *chassisAssetTag = (UEFI_CONFIG_SMBIOS_CHASSIS_ASSET_TAG*) header;
                PcdSet64(PcdSmbiosChassisAssetTagStr, (UINT64) chassisAssetTag->ChassisAssetTag);
                status = GetSmbiosStructureStringLength(header->Length, chassisAssetTag->ChassisAssetTag, &stringLength);

                if (EFI_ERROR(status))
                {
                    goto Failure;
                }

                PcdSet32(PcdSmbiosChassisAssetTagSize, stringLength);

                break;

            case UefiConfigSmbiosBiosLockString:
                UEFI_CONFIG_SMBIOS_BIOS_LOCK_STRING *biosLockString = (UEFI_CONFIG_SMBIOS_BIOS_LOCK_STRING*) header;
                PcdSet64(PcdSmbiosBiosLockStringStr, (UINT64) biosLockString->BiosLockString);
                status = GetSmbiosStructureStringLength(header->Length, biosLockString->BiosLockString, &stringLength);

                if (EFI_ERROR(status))
                {
                    goto Failure;
                }

                PcdSet32(PcdSmbiosBiosLockStringSize, stringLength);

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
                status = GetSmbiosStructureStringLength(header->Length, socketDesignation->SocketDesignation, &stringLength);

                if (EFI_ERROR(status))
                {
                    goto Failure;
                }

                PcdSet32(PcdSmbiosProcessorSocketDesignationSize, stringLength);

                break;

            case UefiConfigSmbiosProcessorManufacturer:
                UEFI_CONFIG_SMBIOS_PROCESSOR_MANUFACTURER *processorManufacturer = (UEFI_CONFIG_SMBIOS_PROCESSOR_MANUFACTURER*) header;
                PcdSet64(PcdSmbiosProcessorManufacturerStr, (UINT64) processorManufacturer->ProcessorManufacturer);
                status = GetSmbiosStructureStringLength(header->Length, processorManufacturer->ProcessorManufacturer, &stringLength);

                if (EFI_ERROR(status))
                {
                    goto Failure;
                }

                PcdSet32(PcdSmbiosProcessorManufacturerSize, stringLength);

                break;

            case UefiConfigSmbiosProcessorVersion:
                UEFI_CONFIG_SMBIOS_PROCESSOR_VERSION *processorVersion = (UEFI_CONFIG_SMBIOS_PROCESSOR_VERSION*) header;
                PcdSet64(PcdSmbiosProcessorVersionStr, (UINT64) processorVersion->ProcessorVersion);
                status = GetSmbiosStructureStringLength(header->Length, processorVersion->ProcessorVersion, &stringLength);

                if (EFI_ERROR(status))
                {
                    goto Failure;
                }

                PcdSet32(PcdSmbiosProcessorVersionSize, stringLength);

                break;

            case UefiConfigSmbiosProcessorSerialNumber:
                UEFI_CONFIG_SMBIOS_PROCESSOR_SERIAL_NUMBER *processorSerialNumber = (UEFI_CONFIG_SMBIOS_PROCESSOR_SERIAL_NUMBER*) header;
                PcdSet64(PcdSmbiosProcessorSerialNumberStr, (UINT64) processorSerialNumber->ProcessorSerialNumber);
                status = GetSmbiosStructureStringLength(header->Length, processorSerialNumber->ProcessorSerialNumber, &stringLength);

                if (EFI_ERROR(status))
                {
                    goto Failure;
                }

                PcdSet32(PcdSmbiosProcessorSerialNumberSize, stringLength);

                break;

            case UefiConfigSmbiosProcessorAssetTag:
                UEFI_CONFIG_SMBIOS_PROCESSOR_ASSET_TAG *processorAssetTag = (UEFI_CONFIG_SMBIOS_PROCESSOR_ASSET_TAG*) header;
                PcdSet64(PcdSmbiosProcessorAssetTagStr, (UINT64) processorAssetTag->ProcessorAssetTag);
                status = GetSmbiosStructureStringLength(header->Length, processorAssetTag->ProcessorAssetTag, &stringLength);

                if (EFI_ERROR(status))
                {
                    goto Failure;
                }

                PcdSet32(PcdSmbiosProcessorAssetTagSize, stringLength);

                break;

            case UefiConfigSmbiosProcessorPartNumber:
                UEFI_CONFIG_SMBIOS_PROCESSOR_PART_NUMBER *processorPartNumber = (UEFI_CONFIG_SMBIOS_PROCESSOR_PART_NUMBER*) header;
                PcdSet64(PcdSmbiosProcessorAssetTagStr, (UINT64) processorPartNumber->ProcessorPartNumber);
                status = GetSmbiosStructureStringLength(header->Length, processorPartNumber->ProcessorPartNumber, &stringLength);

                if (EFI_ERROR(status))
                {
                    goto Failure;
                }

                PcdSet32(PcdSmbiosProcessorAssetTagSize, stringLength);

                break;

            case UefiConfigFlags:
                UEFI_CONFIG_FLAGS *flags = (UEFI_CONFIG_FLAGS*) header;
                PcdSetBool(PcdSerialControllersEnabled, (UINT8) flags->Flags.SerialControllersEnabled);
                PcdSetBool(PcdPauseAfterBootFailure, (UINT8) flags->Flags.PauseAfterBootFailure);
                PcdSetBool(PcdPxeIpV6, (UINT8) flags->Flags.PxeIpV6);
                PcdSetBool(PcdDebuggerEnabled, (UINT8) flags->Flags.DebuggerEnabled);
                PcdSetBool(PcdLoadOempTable, (UINT8) flags->Flags.LoadOempTable);
                PcdSetBool(PcdTpmEnabled, (UINT8) flags->Flags.TpmEnabled);
                PcdSetBool(PcdHibernateEnabled, (UINT8) flags->Flags.HibernateEnabled);
                PcdSet8(PcdConsoleMode, (UINT8) flags->Flags.ConsoleMode);
                PcdSetBool(PcdMemoryAttributesTableEnabled, (UINT8) flags->Flags.MemoryAttributesTableEnabled);
                PcdSetBool(PcdVirtualBatteryEnabled, (UINT8) flags->Flags.VirtualBatteryEnabled);
                PcdSetBool(PcdSgxMemoryEnabled, (UINT8) flags->Flags.SgxMemoryEnabled);
                requiredStructures.UefiConfigFlags = 1;
                break;

            case UefiConfigProcessorInformation:
                UEFI_CONFIG_PROCESSOR_INFORMATION *processorInfo = (UEFI_CONFIG_PROCESSOR_INFORMATION*) header;
                PcdSet32(PcdProcessorCount, processorInfo->ProcessorCount);
                PcdSet32(PcdProcessorsPerVirtualSocket, processorInfo->ProcessorsPerVirtualSocket);

                if (processorInfo->ProcessorCount == 0)
                {
                    DEBUG((DEBUG_ERROR, "Processors count was 0.\n"));
                    goto Failure;
                }

                if (processorInfo->ProcessorsPerVirtualSocket == 0)
                {
                    DEBUG((DEBUG_ERROR, "Processors per virtual socket was 0.\n"));
                    goto Failure;
                }

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
                    goto Failure;
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
        }

        calculatedConfigSize += header->Length;
        header = (UEFI_CONFIG_HEADER*) ((UINT64) header + header->Length);
    }

    if (requiredStructures.AsUINT64 != AllStructuresFound)
    {
        DEBUG((DEBUG_ERROR, "Missing required structures, found structures: 0x%x\n", requiredStructures.AsUINT64));
        ASSERT(FALSE);
        return EFI_DEVICE_ERROR;
    }

    if (configCount->TotalConfigBlobSize != calculatedConfigSize)
    {
        DEBUG((DEBUG_ERROR, "Reported config size of 0x%x did not match actual size of 0x%x\n", configCount->TotalConfigBlobSize, calculatedConfigSize));
        ASSERT(FALSE);
        return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;

Failure:

    //
    // Some structure is malformed, stop boot.
    //
    DEBUG((DEBUG_ERROR, "Config Structure of type 0x%x, with length 0x%x was malformed\n", header->Type, header->Length));
    ASSERT(FALSE);

    return EFI_DEVICE_ERROR;
}

EFI_STATUS
EFIAPI
InitializePlatform(
    _In_ EFI_PEI_FILE_HANDLE       FileHandle,
    _In_ CONST EFI_PEI_SERVICES**  PeiServices
  )
/*++

Routine Description:

    Entry point of the Platform PEIM.  Initializes the platform.

Arguments:

    FileHandle - Handle of the file being invoked.

    PeiServices  An indirect pointer to the PEI Services Table.

Return Value:

    EFI_SUCCESS on success, otherwise an error status.

--*/
{
    EFI_STATUS status;
    UINT32 vDevVersion = (UINT32)-1;

    DEBUG((DEBUG_VERBOSE, "Platform PEIM InitializePlatform entered\n"));

    status = GetUefiConfigInfo();

    //
    // The config blob was not well formed, do not proceed.
    //
    if (EFI_ERROR(status))
    {
        ASSERT(FALSE);
        return status;
    }

    //
    // DxeKDLib.c InitializeDebugAgent is called very early on in DXE Core,
    // before any drivers are dispatched. Thus, we need to send this boolean
    // flag via a HOB since the Pcd module isn't yet available.
    //
    BOOLEAN debuggerEnabled = PcdGetBool(PcdDebuggerEnabled);
    HobAddGuidData(&gMsvmDebuggerEnabledGuid,
        &debuggerEnabled,
        sizeof(BOOLEAN));

    vDevVersion = PcdGet32(PcdBiosVDevVersion);

    //
    // Init memory map before publishing any other HOBs.
    //
    InitializeMemoryMap(vDevVersion);

    //
    // Set the boot mode and installs the boot mode tag PPI.
    //
    status = PeiServicesSetBootMode(BOOT_WITH_FULL_CONFIGURATION);
    ASSERT_EFI_ERROR(status);

    status = PeiServicesInstallPpi(MsvmBootModePpiDescriptor);
    ASSERT_EFI_ERROR(status);

    //
    // Init the watchdog.
    //
    if (vDevVersion > VDevVersion2)
    {
        //
        // Watchdog only available starting with Threshold VDev.
        //
        InitializeWatchdog();
    }

    DEBUG((DEBUG_VERBOSE, "Platform PEIM InitializePlatform completed\n"));

    return EFI_SUCCESS;
}

