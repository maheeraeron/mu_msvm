/*++

Copyright (c) Microsoft Corporation

Module Name:

    IgvmConfig.c

Abstract:

    Gets configuration values from IGVM file format parameters and exports
    them as globals and PCDs.

--*/

#include <IndustryStandard/Acpi.h>
#include <PiPei.h>
#include <EfiNt.h>
#include <Platform.h>
#include <BiosInterface.h>
#if defined(MDE_CPU_AARCH64)
#include <Library/ArmLib.h>
#endif
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/ResourcePublicationLib.h>
#include <Ppi/ConfigPpi.h>
#include <Config.h>
#include <KdNet.h>
#include <IsolationTypes.h>
#include <UefiConstants.h>

typedef struct _IGVM_VHS_MEMORY_MAP_ENTRY {
    UINT64 StartingGpaPageNumber;
    UINT64 NumberOfPages;
    UINT16 Type;
    UINT16 Flags;
    UINT32 Reserved;
} IGVM_VHS_MEMORY_MAP_ENTRY;

enum IGVM_VHS_MEMORY_MAP_ENTRY_TYPES
{
    IGVM_VHF_MEMORY_MAP_ENTRY_TYPE_MEMORY            = 0x0,
    IGVM_VHF_MEMORY_MAP_ENTRY_TYPE_PLATFORM_RESERVED = 0x1,
    IGVM_VHF_MEMORY_MAP_ENTRY_TYPE_PERSISTENT        = 0x2,
    IGVM_VHF_MEMORY_MAP_ENTRY_TYPE_VTL2_PROTECTABLE  = 0x3,
};

#define IGVM 0x4947564D // "IGVM"

#define IGVM_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR() \
    PEI_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR(IGVM);
#define IGVM_FAIL_FAST_IF_FAILED(Status, ErrorCode) \
    PEI_FAIL_FAST_IF_FAILED(Status, ErrorCode, IGVM)

PVOID
GetIgvmData(
    _In_ PVOID ParameterAreaBase,
    _In_ UINT32 PageOffset
    )
/*++

Routine Description:

    Obtain the base of an IGVM parameter block.

Arguments:

    ParameterAreaBase - Supplies the base address of the parameter area.

    PageOffset - Supplies the page offset of the desired parameter block.

Return Value:

    The address of the desired parameter block.

--*/
{
    return (PUINT8)ParameterAreaBase + (PageOffset * EFI_PAGE_SIZE);
}


EFI_STATUS
ParseIgvmMemoryMap(
    _In_ UEFI_IGVM_PARAMETER_INFO *ParameterInfo
    )
/*++

Routine Description:

    Parses the memory map in IGVM format to construct a memory map suitable
    for consumption by the rest of UEFI.

Arguments:

    ParameterInfo - Supplies a pointer to the parameter information block.

Return Value:

    EFI_STATUS.

--*/
{
    UINT64 basePage;
    UINT32 index;
    UINT32 maximumIndex;
    UINT32 maximumRange;
    IGVM_VHS_MEMORY_MAP_ENTRY *memoryMap;
    UINT64 nextPage;
    UINT64 pageCount;
    PVM_MEMORY_RANGE_V5 range;
    UINT32 rangeIndex;
    UINT32 rangeFlags;
    PVOID uefiMemoryMap;
    EFI_STATUS Status;

    memoryMap = GetIgvmData(ParameterInfo, ParameterInfo->MemoryMapOffset);
    maximumIndex = (ParameterInfo->MemoryMapPageCount * EFI_PAGE_SIZE) /
                   sizeof(IGVM_VHS_MEMORY_MAP_ENTRY);

    //
    // Convert the memory map to the format expected by UEFI.
    //

    uefiMemoryMap = GetIgvmData(ParameterInfo, ParameterInfo->UefiMemoryMapOffset);

    range = uefiMemoryMap;
    maximumRange = (ParameterInfo->UefiMemoryMapPageCount * EFI_PAGE_SIZE) /
                   sizeof(VM_MEMORY_RANGE_V5);

    Status = PcdSetBoolS(PcdLegacyMemoryMap, FALSE);
    if (EFI_ERROR(Status))
    {
        DEBUG((DEBUG_ERROR, "Failed to set the PCD PcdLegacyMemoryMap::0x%x \n", Status));
        return Status;
    }

    nextPage = 0;
    index = 0;
    rangeIndex = 0;

    while ((index < maximumIndex) && (rangeIndex < maximumRange))
    {
        basePage = memoryMap[index].StartingGpaPageNumber;
        pageCount = memoryMap[index].NumberOfPages;
        if (pageCount == 0)
        {
            break;
        }

        if (basePage < nextPage)
        {
            return EFI_DEVICE_ERROR;
        }

        nextPage = basePage + pageCount;
        if (nextPage <= basePage)
        {
            return EFI_DEVICE_ERROR;
        }

        rangeFlags = 0;
        switch (memoryMap[index].Type)
        {
        case IGVM_VHF_MEMORY_MAP_ENTRY_TYPE_MEMORY:
            break;

        case IGVM_VHF_MEMORY_MAP_ENTRY_TYPE_PLATFORM_RESERVED:
            rangeFlags |= VM_MEMORY_RANGE_FLAG_PLATFORM_RESERVED;
            break;

        default:
            return EFI_DEVICE_ERROR;
        }

        //
        // Determine whether this range can be consumed in its entirety.  It
        // must be split if it crosses the VP context page.
        //

        if ((rangeFlags & VM_MEMORY_RANGE_FLAG_PLATFORM_RESERVED) == 0)
        {
            if (basePage == ParameterInfo->VpContextPageNumber)
            {
                //
                // Generate a single reserved page and process the remainder
                // of the range (if any) in the next pass.
                //

                memoryMap[index].StartingGpaPageNumber += 1;
                memoryMap[index].NumberOfPages -= 1;
                if (memoryMap[index].NumberOfPages == 0)
                {
                    index += 1;
                }

                pageCount = 1;
                nextPage = basePage + pageCount;
                rangeFlags = VM_MEMORY_RANGE_FLAG_PLATFORM_RESERVED;
            }
            else if ((basePage < ParameterInfo->VpContextPageNumber) &&
                     (nextPage > ParameterInfo->VpContextPageNumber))
            {
                //
                // If this range straddles the VP context page, then split off
                // the portion before the page and process the remainder in
                // the next pass.
                //

                pageCount = ParameterInfo->VpContextPageNumber - basePage;
                memoryMap[index].StartingGpaPageNumber = ParameterInfo->VpContextPageNumber;
                memoryMap[index].NumberOfPages -= pageCount;
                nextPage = basePage + pageCount;
            }
            else
            {
                index += 1;
            }
        }

        range->BaseAddress = basePage * SIZE_4KB;
        range->Length = pageCount * SIZE_4KB;
        range->Flags = rangeFlags;
        range->Reserved = 0;

        range += 1;
    }

    Status = PcdSet64S(PcdMemoryMapPtr, (UINT64)uefiMemoryMap);
    if (EFI_ERROR(Status))
    {
        DEBUG((DEBUG_ERROR, "Failed to set the PCD PcdMemoryMapPtr::0x%x \n", Status));
        return Status;
    }
    Status = PcdSet32S(PcdMemoryMapSize, (UINT32)((UINT64)range - (UINT64)uefiMemoryMap));
    if (EFI_ERROR(Status))
    {
        DEBUG((DEBUG_ERROR, "Failed to set the PCD PcdMemoryMapSize::0x%x \n", Status));
        return Status;
    }

    return EFI_SUCCESS;
}


VOID
ParseIgvmCommandLine(
    _In_ UEFI_IGVM_PARAMETER_INFO *ParameterInfo
    )
/*++

Routine Description:

    Parses the command line in IGVM format to determine additional parameters
    (e.g. debug parameters).

Arguments:

    ParameterInfo - Supplies a pointer to the parameter information block.

Return Value:

    None.

--*/
{
    PUCHAR commandString;
    UINT32 maximumSize;
    UINT32 size;

    //
    // Verify command line is within parameter page.
    //

    commandString = GetIgvmData(ParameterInfo, ParameterInfo->CommandLineOffset);
    size = 0;
    maximumSize = ParameterInfo->CommandLinePageCount * EFI_PAGE_SIZE;

    while (commandString[size] != '\0')
    {
        size++;

        //
        // No null terminator found, can't be valid.
        //

        if (size >= maximumSize)
        {
            return;
        }
    }

    //
    // Extract the KDNET parameters.
    //

    ParseKdNetParameters(commandString);
}


EFI_STATUS
GetIgvmConfigInfo(
    VOID
    )
/*++

Routine Description:

    Get and parse the config information in IGVM format.

Arguments:

    None.

Return Value:

    EFI_SUCCESS on success. Error if the config information is corrupt.

--*/
{
    UEFI_CONFIG_FLAGS configFlags;
    UEFI_IGVM_PARAMETER_INFO *parameterInfo;
    UEFI_CONFIG_PROCESSOR_INFORMATION processorInfo;
    EFI_STATUS status;
    PUINT64 freeParameterMemory;


    //
    // Locate the parameter layout description at the base of the parameter
    // area.
    //

    parameterInfo = (UEFI_IGVM_PARAMETER_INFO *)GetStartOfConfigBlob();

    //
    // Capture the total size of config information.
    //

    IGVM_FAIL_FAST_IF_FAILED(PcdSet32S(PcdConfigBlobSize, parameterInfo->ParameterPageCount * EFI_PAGE_SIZE), CRITICAL_INITIALIZATION_FAILURE);

    if (parameterInfo->UefiIgvmConfigurationFlags & UEFI_IGVM_CONFIGURATION_ENABLE_HOST_EMULATORS)
    {
        IGVM_FAIL_FAST_IF_FAILED(PcdSetBoolS(PcdHostEmulatorsWhenHardwareIsolated, TRUE), CRITICAL_INITIALIZATION_FAILURE);
    }

    //
    // TODO: Find some way of avoiding hardcode of necessary host information
    //
    freeParameterMemory = (PUINT64)(parameterInfo) + sizeof(UEFI_IGVM_PARAMETER_INFO);

    // set BIOS GUID
    freeParameterMemory[0] = 0x7464782d7464782d;
    freeParameterMemory[1] = 0x7464782d7464782d;
    // set chassis asset tag
    freeParameterMemory[2] = 0x736168632d786474;
    freeParameterMemory[3] = 0x736168632d786474;
    freeParameterMemory[4] = 0x00;
    IGVM_FAIL_FAST_IF_FAILED(PcdSet64S(PcdBiosGuidPtr, (UINT64)freeParameterMemory), CRITICAL_INITIALIZATION_FAILURE);
    IGVM_FAIL_FAST_IF_FAILED(PcdSet64S(PcdSmbiosChassisAssetTagStr, (UINT64)freeParameterMemory + sizeof(GUID)), CRITICAL_INITIALIZATION_FAILURE);
    IGVM_FAIL_FAST_IF_FAILED(PcdSet32S(PcdSmbiosChassisAssetTagSize, 17), CRITICAL_INITIALIZATION_FAILURE);

    //
    // TODO: use parameters for this
    // Assume a single processor until VPR/VPS information can be configured
    // in the IGVM file.
    //

    processorInfo.ProcessorCount = 1;
    processorInfo.ProcessorsPerVirtualSocket = 1;
    processorInfo.ThreadsPerProcessor = 1;
    ConfigSetProcessorInfo(&processorInfo);

    //
    // Update the processor count.
    //
    UEFI_IGVM_LOADER_BLOCK *loaderBlock = (UEFI_IGVM_LOADER_BLOCK*)GetIgvmData(parameterInfo, parameterInfo->LoaderBlockOffset);
    if (loaderBlock->NumberOfProcessors == 0 || loaderBlock->NumberOfProcessors > HV_MAXIMUM_PROCESSORS)
    {
        DEBUG((DEBUG_ERROR, "Invalid processor count %u.\n", loaderBlock->NumberOfProcessors));
        IGVM_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR();
    }
    IGVM_FAIL_FAST_IF_FAILED(PcdSet32S(PcdProcessorCount, loaderBlock->NumberOfProcessors), CRITICAL_INITIALIZATION_FAILURE);

    //
    // Enable ACPI tables
    //
    if (parameterInfo->MadtPageCount == 0)
    {
        DEBUG((DEBUG_ERROR, "MadtPageCount was 0.\n"));
        IGVM_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR();
    }

    EFI_ACPI_DESCRIPTION_HEADER *madtHdr = (EFI_ACPI_DESCRIPTION_HEADER*)GetIgvmData(parameterInfo, parameterInfo->MadtOffset);
    
    if (madtHdr->Signature != EFI_ACPI_6_2_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE ||
        madtHdr->Length > (parameterInfo->MadtPageCount * EFI_PAGE_SIZE))
    {
        DEBUG((DEBUG_ERROR, "*** Malformed MADT\n"));
        IGVM_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR();
    }

    IGVM_FAIL_FAST_IF_FAILED(PcdSet64S(PcdMadtPtr, (UINT64)madtHdr), CRITICAL_INITIALIZATION_FAILURE);
    IGVM_FAIL_FAST_IF_FAILED(PcdSet32S(PcdMadtSize, madtHdr->Length), CRITICAL_INITIALIZATION_FAILURE);

    if (parameterInfo->SratPageCount == 0)
    {
        DEBUG((DEBUG_ERROR, "SratPageCount was 0.\n"));
        IGVM_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR();
    }

    EFI_ACPI_DESCRIPTION_HEADER *sratHdr = (EFI_ACPI_DESCRIPTION_HEADER*)GetIgvmData(parameterInfo, parameterInfo->SratOffset);
   
    if (sratHdr->Signature != EFI_ACPI_6_2_SYSTEM_RESOURCE_AFFINITY_TABLE_SIGNATURE ||
        sratHdr->Length > (parameterInfo->SratPageCount * EFI_PAGE_SIZE))
    {
        DEBUG((DEBUG_ERROR, "*** Malformed SRAT\n"));
        IGVM_FAIL_FAST_UNEXPECTED_HOST_BEHAVIOR();
    }

    IGVM_FAIL_FAST_IF_FAILED(PcdSet64S(PcdSratPtr, (UINT64)sratHdr), CRITICAL_INITIALIZATION_FAILURE);
    IGVM_FAIL_FAST_IF_FAILED(PcdSet32S(PcdSratSize, sratHdr->Length), CRITICAL_INITIALIZATION_FAILURE);
    
    //
    // Parse the command line to obtain debug parameters.
    //

    ParseIgvmCommandLine(parameterInfo);

    //
    // Build a config structure with a statically defined configuration.
    //

    ZeroMem(&configFlags, sizeof(configFlags));
    configFlags.Flags.MeasureAdditionalPcrs = 1;
    configFlags.Flags.DefaultBootAlwaysAttempt = 1;
    // TODO: allow and harden vpci before ship
    configFlags.Flags.VpciBootEnabled = 1;
    // TODO: Address before ship
    configFlags.Flags.MemoryProtectionMode = ConfigLibMemoryProtectionModeDisabled;

    //
    // IGVM configurations support only KDNET debugging, so only enable
    // debugging if KDNET was configured.
    //

    if (UseKdNetDebugger)
    {
        configFlags.Flags.DebuggerEnabled = 1;
    }

    ConfigSetUefiConfigFlags(&configFlags);

    //
    // Convert the memory map to UEFI format.
    //

    status = ParseIgvmMemoryMap(parameterInfo);
    if (EFI_ERROR(status))
    {
        return status;
    }

    return EFI_SUCCESS;
}
