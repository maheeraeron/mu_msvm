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
#include <BiosConfigPageGuid.h>
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


void
DebugDumpConfigPageV2(
    _In_ CONST BIOS_CONFIG_PAGE_V2* ConfigPage
    )
/*++

Routine Description:

    Outputs the contents of the V2 config page to the debugger.

Arguments:

    ConfigPage - A pointer to a V2 config page.

Return Value:

    n/a

--*/
{
    DEBUG((DEBUG_VERBOSE, "--- V2 Config Data @ %x\n", ConfigPage));
    DEBUG((DEBUG_VERBOSE, "  Size                    % 17x\n", ConfigPage->Size));
    DEBUG((DEBUG_VERBOSE, "  SratSize                % 17x\n", ConfigPage->SratSize));
    DEBUG((DEBUG_VERBOSE, "  BiosSizePages           % 17x\n", ConfigPage->BiosSizePages));
    DEBUG((DEBUG_VERBOSE, "  ProcessorCount          % 17x\n", ConfigPage->ProcessorCount));
    DEBUG((DEBUG_VERBOSE, "  LowMemoryBasePages      % 17lx\n", ConfigPage->LowMemoryBasePages));
    DEBUG((DEBUG_VERBOSE, "  LowMemoryLengthPages    % 17lx\n", ConfigPage->LowMemoryLengthPages));
    DEBUG((DEBUG_VERBOSE, "  MiddleMemoryBasePages   % 17lx\n", ConfigPage->MiddleMemoryBasePages));
    DEBUG((DEBUG_VERBOSE, "  MiddleMemoryLengthPages % 17lx\n", ConfigPage->MiddleMemoryLengthPages));
    DEBUG((DEBUG_VERBOSE, "  HighMemoryBasePages     % 17lx\n", ConfigPage->HighMemoryBasePages));
    DEBUG((DEBUG_VERBOSE, "  HighMemoryLengthPages   % 17lx\n", ConfigPage->HighMemoryLengthPages));
    DEBUG((DEBUG_VERBOSE, "  LowMmioGapBasePages     % 17lx\n", ConfigPage->LowMmioGapBasePages));
    DEBUG((DEBUG_VERBOSE, "  LowMmioGapLengthPages   % 17lx\n", ConfigPage->LowMmioGapLengthPages));
    DEBUG((DEBUG_VERBOSE, "  HighMmioGapBasePages    % 17lx\n", ConfigPage->HighMmioGapBasePages));
    DEBUG((DEBUG_VERBOSE, "  HighMmioGapLengthPages  % 17lx\n", ConfigPage->HighMmioGapLengthPages));
    DEBUG((DEBUG_VERBOSE, "  GUID:                %g\n", (EFI_GUID *)ConfigPage->BiosGuid));
    DEBUG((DEBUG_VERBOSE, "  SystemSerialNumber:  %a\n", ConfigPage->SystemSerialNumber));
    DEBUG((DEBUG_VERBOSE, "  BaseSerialNumber:    %a\n", ConfigPage->BaseSerialNumber));
    DEBUG((DEBUG_VERBOSE, "  ChassisSerialNumber: %a\n", ConfigPage->ChassisSerialNumber));
    DEBUG((DEBUG_VERBOSE, "  ChassisAssetTag:     %a\n", ConfigPage->ChassisAssetTag));
    DEBUG((DEBUG_VERBOSE, "  BiosLockString:      %a\n", ConfigPage->BiosLockString));
    DEBUG((DEBUG_VERBOSE, "  DebuggerEnabled            %a\n",
        ConfigPage->Flags.DebuggerEnabled == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  LoadOempTable              %a\n",
        ConfigPage->Flags.LoadOempTable == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  SerialControllersEnabled   %a\n",
        ConfigPage->Flags2.SerialControllersEnabled == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  PauseAfterBootFailure      %a\n",
        ConfigPage->Flags2.PauseAfterBootFailure == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  PxeIpV6                    %a\n",
        ConfigPage->Flags2.PxeIpV6 == 1 ? "TRUE" : "FALSE"));
}


void
DebugDumpConfigPageV3(
    _In_ CONST BIOS_CONFIG_PAGE_V3* ConfigPage
    )
/*++

Routine Description:

    Outputs the contents of the V3 config page to the debugger.

Arguments:

    ConfigPage - A pointer to a V3 config page.

Return Value:

    n/a

--*/
{
    DEBUG((DEBUG_VERBOSE, "--- V3 Config Data @ %x\n", ConfigPage));
    DEBUG((DEBUG_VERBOSE, "  Size                    % 17x\n", ConfigPage->Size));
    DEBUG((DEBUG_VERBOSE, "  SratSize                % 17x\n", ConfigPage->SratSize));
    DEBUG((DEBUG_VERBOSE, "  MemoryMapSize           % 17x\n", ConfigPage->MemoryMapSize));
    DEBUG((DEBUG_VERBOSE, "  BiosSizePages           % 17x\n", ConfigPage->BiosSizePages));
    DEBUG((DEBUG_VERBOSE, "  ProcessorCount          % 17x\n", ConfigPage->ProcessorCount));
    DEBUG((DEBUG_VERBOSE, "  LowMmioGapBasePages     % 17lx\n", ConfigPage->LowMmioGapBasePages));
    DEBUG((DEBUG_VERBOSE, "  LowMmioGapLengthPages   % 17lx\n", ConfigPage->LowMmioGapLengthPages));
    DEBUG((DEBUG_VERBOSE, "  HighMmioGapBasePages    % 17lx\n", ConfigPage->HighMmioGapBasePages));
    DEBUG((DEBUG_VERBOSE, "  HighMmioGapLengthPages  % 17lx\n", ConfigPage->HighMmioGapLengthPages));
    DEBUG((DEBUG_VERBOSE, "  GUID:                %g\n", (EFI_GUID *)ConfigPage->BiosGuid));
    DEBUG((DEBUG_VERBOSE, "  SystemSerialNumber:  %a\n", ConfigPage->SystemSerialNumber));
    DEBUG((DEBUG_VERBOSE, "  BaseSerialNumber:    %a\n", ConfigPage->BaseSerialNumber));
    DEBUG((DEBUG_VERBOSE, "  ChassisSerialNumber: %a\n", ConfigPage->ChassisSerialNumber));
    DEBUG((DEBUG_VERBOSE, "  ChassisAssetTag:     %a\n", ConfigPage->ChassisAssetTag));
    DEBUG((DEBUG_VERBOSE, "  BiosLockString:      %a\n", ConfigPage->BiosLockString));
    DEBUG((DEBUG_VERBOSE, "  SerialControllersEnabled   %a\n",
        ConfigPage->Flags.SerialControllersEnabled == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  PauseAfterBootFailure      %a\n",
        ConfigPage->Flags.PauseAfterBootFailure == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  PxeIpV6                    %a\n",
        ConfigPage->Flags.PxeIpV6 == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  DebuggerEnabled            %a\n",
        ConfigPage->Flags.DebuggerEnabled == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  LoadOempTable              %a\n",
        ConfigPage->Flags.LoadOempTable == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  TpmEnabled                 %a\n",
        ConfigPage->Flags.TpmEnabled == 1 ? "TRUE" : "FALSE"));
}


BIOS_CONFIG_PAGE_V2*
GetV2ConfigPage(
    _In_ CONST EFI_PEI_SERVICES**  PeiServices
    )
/*++

Routine Description:

    Gets the version 2 configuraton page from the BIOS VDev.

Arguments:

    PeiServices  An indirect pointer to the PEI Services Table.

Return Value:

    Pointer to a V2 config page.

--*/
{
    EFI_STATUS status;
    BIOS_CONFIG_PAGE_V2 *configPage = NULL;

    //
    // Allocate space for the config data on the heap.
    //
    status = (*PeiServices)->AllocatePool(PeiServices, sizeof(BIOS_CONFIG_PAGE_V2), &configPage);
    if (EFI_ERROR(status))
    {
        ASSERT(FALSE);
        return configPage;
    }

    //
    // Retrieve the configuration page from the worker process.
    //
    configPage->Size = sizeof(*configPage);
    DEBUG((DEBUG_VERBOSE, "--- Config Page Size 0x%x (%d)\n", configPage->Size, configPage->Size));
    DEBUG((DEBUG_VERBOSE, "--- Config Page Address 0x%x\n", configPage));
    configPage->BiosSizePages = 0;
    WriteBiosDevice(BiosConfigWriteConfigPage, (UINT32)(UINTN)configPage);

    if (configPage->BiosSizePages == 0)
    {
        //
        // The memory was not updated.  This can happen if there is a build
        // mismatch between the firmware and worker process, or if the worker
        // process is extremely low on memory.
        //
        DEBUG((DEBUG_ERROR, "*** GetV2ConfigPage failed\n"));
        ASSERT(FALSE);
    }
    else
    {
        DebugDumpConfigPageV2(configPage);
    }

    return configPage;
}


BIOS_CONFIG_PAGE_V3*
GetV3ConfigPage(
    _In_ CONST EFI_PEI_SERVICES**  PeiServices
    )
/*++

Routine Description:

    Gets the version 3 configuraton page from the BIOS VDev.

Arguments:

    PeiServices  An indirect pointer to the PEI Services Table.

Return Value:

    Pointer to a V3 config page.

--*/
{
    EFI_STATUS status;
    BIOS_CONFIG_PAGE_V3 *configPage = NULL;

    //
    // Allocate space for the config data on the heap.
    //
    status = (*PeiServices)->AllocatePool(PeiServices, sizeof(BIOS_CONFIG_PAGE_V3), &configPage);
    if (EFI_ERROR(status))
    {
        ASSERT(FALSE);
        return configPage;
    }

    //
    // Retrieve the configuration page from the worker process.
    //
    configPage->Size = sizeof(BIOS_CONFIG_PAGE_V3);
    configPage->BiosSizePages = 0;
    WriteBiosDevice(BiosConfigWriteConfigPage, (UINT32)(UINTN)configPage);

    if (configPage->BiosSizePages == 0)
    {
        //
        // The memory was not updated.  This can happen if there is a build
        // mismatch between the firmware and worker process, or if the worker
        // process is extremely low on memory.
        //
        DEBUG((DEBUG_ERROR, "*** GetV3ConfigPage failed\n"));
        ASSERT(FALSE);
    }
    else
    {
        DebugDumpConfigPageV3(configPage);
    }
    return configPage;
}


VOID *
GetSRAT(
    _In_ CONST EFI_PEI_SERVICES**  PeiServices,
    _In_ UINT32 SratSize
    )
/*++

Routine Description:

    Gets the SRAT table from the BIOS VDev.

Arguments:

    PeiServices - An indirect pointer to the PEI Services Table.

    SratSize - The size of the SRAT.

Return Value:

    Pointer to the SRAT.  NULL on failure.

--*/
{
    EFI_STATUS status;
    VOID *srat = NULL;
    UINTN base, size;
    UINT8 *cursor;
    EFI_ACPI_DESCRIPTION_HEADER  *acpiHdr;
    EFI_ACPI_5_0_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY_STRUCTURE *sratApic;
    EFI_ACPI_5_0_MEMORY_AFFINITY_STRUCTURE *sratMem;

    //
    // Allocate space for the SRAT on the heap.
    //
    status = (*PeiServices)->AllocatePool(PeiServices, SratSize, &srat);
    if (EFI_ERROR(status))
    {
        DEBUG((DEBUG_ERROR, "*** Failed (%x) to allocate pool for SRAT\n", status));
        ASSERT(FALSE);
        return NULL;
    }
    acpiHdr = (EFI_ACPI_DESCRIPTION_HEADER  *)srat;

    //
    // Retrieve the data from the worker process.
    //
    acpiHdr->Signature = 0;
    WriteBiosDevice(BiosConfigSratData, (UINT32)(UINTN)acpiHdr);

    if (acpiHdr->Signature != EFI_ACPI_4_0_SYSTEM_RESOURCE_AFFINITY_TABLE_SIGNATURE)
    {
        //
        // The memory was not updated.  This can happen if the worker
        // process is extremely low on memory.
        //
        DEBUG((DEBUG_ERROR, "*** Failed to get SRAT from VDEv\n"));
        ASSERT(FALSE);
        return NULL;
    }

    //
    // Debug dump the SRAT entries.
    //
    DEBUG_CODE_BEGIN();
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
    DEBUG_CODE_END();

    return srat;
}


VOID *
GetMemoryMap(
    _In_ CONST UINT32 VDevVersion,
    _In_ CONST EFI_PEI_SERVICES**  PeiServices,
    _In_ UINT32 MemMapSize
    )
/*++

Routine Description:

    Gets the Memory Map table from the BIOS VDev.

Arguments:

    VDevVersion - The VDev version from the host.

    PeiServices - An indirect pointer to the PEI Services Table.

    MemMapSize - The size of the Memory Map in bytes.

Return Value:

    Pointer to the Memory Map. NULL on failure.

--*/
{
    EFI_STATUS status;
    VOID *memMap = NULL;

    //
    // If size is zero just return NULL.
    //
    if (MemMapSize == 0)
    {
        return NULL;
    }

    //
    // Allocate space for the Memory Map on the heap.
    //
    status = (*PeiServices)->AllocatePool(PeiServices, MemMapSize, &memMap);
    if (EFI_ERROR(status))
    {
        DEBUG((DEBUG_ERROR, "*** Failed (%x) to allocate pool for Memory Map\n", status));
        ASSERT(FALSE);
        return NULL;
    }

    //
    // Init the first range length to zero. Nonzero length value after intercept
    // to the VDev will indicate success.
    //
    if (VDevVersion >= VDevVersion5)
    {
        ((PVM_MEMORY_RANGE_V5)memMap)->Length = 0;
    }
    else
    {
        ((PVM_MEMORY_RANGE)memMap)->Length = 0;
    }

    //
    // Retrieve the data from the worker process.
    //
    WriteBiosDevice(BiosConfigMemoryMap, (UINT32)(UINTN)memMap);

    if ((VDevVersion >= VDevVersion5 && ((PVM_MEMORY_RANGE_V5)memMap)->Length == 0) ||
        (((PVM_MEMORY_RANGE)memMap)->Length == 0))
    {
        //
        // The first range length was not updated.  This can happen if the worker
        // process is extremely low on memory.
        //
        DEBUG((DEBUG_ERROR, "*** Failed to get Memory Map from VDEv\n"));
        ASSERT(FALSE);
        return NULL;
    }

    //
    // Debug dump the Memory Map entries.
    //
    DEBUG_CODE_BEGIN();
    DEBUG((DEBUG_VERBOSE, "--- Memory Map data @ %x Length %x\n", memMap, MemMapSize));
    if (VDevVersion >= VDevVersion5)
    {
        PVM_MEMORY_RANGE_V5 range = (PVM_MEMORY_RANGE_V5)memMap;
        do
        {
            DEBUG((DEBUG_VERBOSE, "    Base % 14lx Len % 14lx Flags % 8x\n",
                range->BaseAddress, range->Length, range->Flags));
            range++;
        } while ((UINT8*)range < ((UINT8*)memMap + MemMapSize));
    }
    else
    {
        PVM_MEMORY_RANGE range = (PVM_MEMORY_RANGE)memMap;
        do
        {
            DEBUG((DEBUG_VERBOSE, "    Base % 14lx Len % 14lx\n", range->BaseAddress, range->Length));
            range++;
        } while ((UINT8*)range < ((UINT8*)memMap + MemMapSize));
    }
    DEBUG_CODE_END();

    return memMap;
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

    DEBUG((DEBUG_VERBOSE, "<<< GetPageTableSize returning %ull\n",
        (UINTN)EFI_PAGES_TO_SIZE(totalPages)));

    return (UINTN)(EFI_PAGES_TO_SIZE(totalPages));
}


VOID
AddFirstMemoryRange(
    _In_ CONST UINT8 PhysicalAddressBits,
    _In_ CONST UINT64 Length,
    _In_ CONST UINT64 BiosSize
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
    DEBUG((DEBUG_VERBOSE, ">>> AddFirstMemoryRange\n"));

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
    pageTableSize = GetPageTableSize(PhysicalAddressBits);
    peiBase = BASE_1MB;
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
    //    reguler system memory.  At least one Windows boot driver (Intel
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
    // Declare System Memory from 1MB to the top.
    //
    HobAddMemoryRange(BASE_1MB, Length - SIZE_1MB);

    DEBUG((DEBUG_VERBOSE, "<<< AddFirstMemoryRange\n"));
}


VOID
InitializeMemoryMap(
    _In_ CONST UINT32 VDevVersion,
    _In_ CONST BIOS_CONFIG_PAGE_V2* ConfigPageV2,
    _In_ CONST BIOS_CONFIG_PAGE_V3* ConfigPageV3,
    _In_ CONST VOID *Srat,
    _In_ CONST VOID *MemMap,
    _In_ CONST UINT32 MemMapSize
    )
/*++

Routine Description:

    Initializes the memory map of the vm by creating appropriate HOBs and
    triggering the MTRRs to be initialized.

Arguments:

    VDevVersion - The version of the BIOS VDev.

    ConfigPageV2 - A pointer to the V2 config page.

    ConfigPageV3 - A pointer to the V3 config page.

    Srat - A pointer to the SRAT.

    MemMap - A pointer to the Memory Map.

    MemMapSize - The length of the Memory Map in bytes.

Return Value:

    None.

--*/
{
    CPUID_ADDRESS_SPACE_SIZES addressSpaceSizes;
    UINT8 physicalAddressBits;
    UINT32 maximumFunction;

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
        {
            //
            // VDev version 2
            //
            // No separate memory map exists so derive memory map from the SRAT.
            //
            EFI_ACPI_DESCRIPTION_HEADER  *acpiHdr;
            UINT8 *cursor;
            EFI_ACPI_5_0_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY_STRUCTURE *sratApic;
            EFI_ACPI_5_0_MEMORY_AFFINITY_STRUCTURE *sratMem;
            UINTN base, size;

            //
            // Loop through the SRAT entries and create HOBs for RAM regions.
            //
            acpiHdr = (EFI_ACPI_DESCRIPTION_HEADER  *)Srat;
            cursor = (UINT8 *)Srat;
            cursor += sizeof(EFI_ACPI_4_0_SYSTEM_RESOURCE_AFFINITY_TABLE_HEADER);

            do
            {
                switch(*cursor)  // UINT8 Type always at start of struct
                {
                    case EFI_ACPI_5_0_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY:
                        sratApic = (EFI_ACPI_5_0_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY_STRUCTURE *)cursor;
                        cursor += sratApic->Length;
                        break;

                    case EFI_ACPI_5_0_MEMORY_AFFINITY:
                        sratMem = (EFI_ACPI_5_0_MEMORY_AFFINITY_STRUCTURE *)cursor;

                        base = (((UINT64)sratMem->AddressBaseHigh) << 32) |
                                (UINT64)sratMem->AddressBaseLow;
                        size = (((UINT64)sratMem->LengthHigh) << 32) |
                                (UINT64)sratMem->LengthLow;

                        //
                        // First memory region is a special case that isn't fully
                        // described in the SRAT.
                        //
                        if (base == 0)
                        {
                            AddFirstMemoryRange(
                                physicalAddressBits,
                                size,
                                ConfigPageV2->BiosSizePages * SIZE_4KB);
                        }
                        else
                        {
                            //
                            // Report subsequent memory regions directly *unless* they are hot-add.
                            //
                            if ((sratMem->Flags & EFI_ACPI_5_0_MEMORY_HOT_PLUGGABLE) !=
                                EFI_ACPI_5_0_MEMORY_HOT_PLUGGABLE)
                            {
                                HobAddMemoryRange(base, size);
                            }
                        }

                        cursor += sratMem->Length;
                        break;

                    default:
                        sratMem = (EFI_ACPI_5_0_MEMORY_AFFINITY_STRUCTURE *)cursor;
                        cursor += sratMem->Length;
                        break;

                }
            } while (cursor < ((UINT8 *)acpiHdr + acpiHdr->Length));
        }
        break;

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
            ASSERT(MemMap != NULL);
            PVM_MEMORY_RANGE range = (PVM_MEMORY_RANGE)MemMap;
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
                        range->Length,
                        ConfigPageV3->BiosSizePages * SIZE_4KB);
                }
                else
                {
                    HobAddMemoryRange(range->BaseAddress, range->Length);
                }

                //
                // Next memory map range.
                //
                range++;
            } while ((UINT8*)range < ((UINT8*)MemMap + MemMapSize));
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
            ASSERT(MemMap != NULL);
            PVM_MEMORY_RANGE_V5 rangeV5 = (PVM_MEMORY_RANGE_V5)MemMap;
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
                        rangeV5->Length,
                        ConfigPageV3->BiosSizePages * SIZE_4KB);
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
            } while ((UINT8*)rangeV5 < ((UINT8*)MemMap + MemMapSize));
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
    if (VDevVersion >= VDevVersion3)
    {
        HobAddMmioRange(
            ConfigPageV3->LowMmioGapBasePages * SIZE_4KB,
            ConfigPageV3->LowMmioGapLengthPages * SIZE_4KB
            );
        HobAddMmioRange(
            ConfigPageV3->HighMmioGapBasePages * SIZE_4KB,
            ConfigPageV3->HighMmioGapLengthPages * SIZE_4KB
            );
    }
    else
    {
        HobAddMmioRange(
            ConfigPageV2->LowMmioGapBasePages * SIZE_4KB,
            ConfigPageV2->LowMmioGapLengthPages * SIZE_4KB
            );
        HobAddMmioRange(
            ConfigPageV2->HighMmioGapBasePages * SIZE_4KB,
            ConfigPageV2->HighMmioGapLengthPages * SIZE_4KB
            );
    }

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
    BIOS_CONFIG_PAGE_V2 *configPageV2 = NULL;
    BIOS_CONFIG_PAGE_V3 *configPageV3 = NULL;
    UINT32 sratSize = 0;
    VOID *srat = NULL;
    UINT32 memMapSize = 0;
    VOID *memMap = NULL;

    DEBUG((DEBUG_VERBOSE, "Platform PEIM InitializePlatform entered\n"));

    //
    // Try to get the VDev version from the worker process.
    //
    vDevVersion = ReadBiosDevice(BiosConfigVdevVersion);
    if (vDevVersion == 0)
    {
        //
        // If the version comes back zero it must be the Windows Blue
        // VDev that didn't support getting the version.
        // Therefore it is implicitly version 2.
        //
        vDevVersion = VDevVersion2;
        DEBUG((DEBUG_VERBOSE, "*** VDev version returned as 0. Defaulting to V2 (512).\n"));
    }
    DEBUG((DEBUG_VERBOSE, "--- VDev version is %d.%d\n", vDevVersion >> 8, vDevVersion & 0xFF));

    //
    // Validate the version number, get the config page, and then some data from it.
    //
    switch(vDevVersion)
    {
        case VDevVersion2:
            configPageV2 = GetV2ConfigPage(PeiServices);
            sratSize = configPageV2->SratSize;
            memMapSize = 0; // not in V2 config page
            break;

        case VDevVersion3:
        case VDevVersion4:
        case VDevVersion5:
            configPageV3 = GetV3ConfigPage(PeiServices);
            sratSize = configPageV3->SratSize;
            memMapSize = configPageV3->MemoryMapSize;
            break;

        default:
            DEBUG((DEBUG_WARN, "** VDev version 0x%x not recognized\n", vDevVersion));
            // assume latest config page version
            configPageV3 = GetV3ConfigPage(PeiServices);
            sratSize = configPageV3->SratSize;
            memMapSize = configPageV3->MemoryMapSize;
            break;
    }

    //
    // Get the SRAT.
    //
    srat = GetSRAT(PeiServices, sratSize);
    if (srat == NULL)
    {
        return EFI_DEVICE_ERROR;
    }

    //
    // Get the Memory Map.
    //
    memMap = GetMemoryMap(vDevVersion, PeiServices, memMapSize);
    if (memMapSize > 0 && memMap == NULL)
    {
        return EFI_DEVICE_ERROR;
    }

    //
    // Init memory map before publishing any other HOBs.
    //
    InitializeMemoryMap(vDevVersion, configPageV2, configPageV3, srat, memMap, memMapSize);

    //
    // Publish the config page HOBs.
    //
    switch (vDevVersion)
    {
        case VDevVersion2:
            HobAddGuidData(
                &gMsvmConfigPageV2Guid,
                configPageV2,
                sizeof(BIOS_CONFIG_PAGE_V2));
            break;

        case VDevVersion3:
        case VDevVersion4:
        case VDevVersion5:
        default: // assume latest config page version
            HobAddGuidData(
                &gMsvmConfigPageV3Guid,
                configPageV3,
                sizeof(BIOS_CONFIG_PAGE_V3));
            break;
    }

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

