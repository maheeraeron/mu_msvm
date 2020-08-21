/*++

Copyright (c) Microsoft Corporation

Module Name:

    IgvmConfig.c

Abstract:

    Gets configuration values from IGVM file format parameters and exports
    them as globals and PCDs.

--*/

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
#include <IsolationTypes.h>

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
    UINT64 contextPage;
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

    memoryMap = GetIgvmData(ParameterInfo, ParameterInfo->MemoryMapOffset);
    maximumIndex = (ParameterInfo->MemoryMapPageCount * EFI_PAGE_SIZE) /
                   sizeof(IGVM_VHS_MEMORY_MAP_ENTRY);

    //
    // Determine which page holds the VP context, since this page will need to
    // be marked permanently reserved.
    //

    contextPage = (UINTN)GetIgvmData(ParameterInfo, ParameterInfo->VpContextPageOffset) / EFI_PAGE_SIZE;

    //
    // Convert the memory map to the format expected by UEFI.
    //

    uefiMemoryMap = GetIgvmData(ParameterInfo, ParameterInfo->UefiMemoryMapOffset);

    range = uefiMemoryMap;
    maximumRange = (ParameterInfo->UefiMemoryMapPageCount * EFI_PAGE_SIZE) /
                   sizeof(VM_MEMORY_RANGE_V5);

    PcdSetBool(PcdLegacyMemoryMap, FALSE);

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
            if (basePage == contextPage)
            {
                //
                // Generate a single reserved page and process the remainder
                // of the range in the next pass.
                //

                memoryMap[index].StartingGpaPageNumber += 1;
                memoryMap[index].NumberOfPages -= 1;

                pageCount = 1;
                nextPage = basePage + pageCount;
                rangeFlags = VM_MEMORY_RANGE_FLAG_PLATFORM_RESERVED;
            }
            else if ((basePage < contextPage) && (nextPage > contextPage))
            {
                //
                // If this page straddles the architectural reset page, then
                // split it into the portion before the page and the
                // remainder.
                //

                pageCount = contextPage - basePage;
                memoryMap[index].StartingGpaPageNumber = contextPage;
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

    PcdSet64(PcdMemoryMapPtr, (UINT64)uefiMemoryMap);
    PcdSet32(PcdMemoryMapSize, (UINT32)((UINT64)range - (UINT64)uefiMemoryMap));

    return EFI_SUCCESS;
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

    //
    // Locate the parameter layout description at the base of the parameter
    // area.
    //

    parameterInfo = (UEFI_IGVM_PARAMETER_INFO *)GetStartOfConfigBlob();

    //
    // Capture the total size of config information.
    //

    PcdSet32(PcdConfigBlobSize, parameterInfo->ParameterPageCount * EFI_PAGE_SIZE);

    //
    // Assume a single processor until VPR/VPS information can be configured
    // in the IGVM file.
    //

    processorInfo.ProcessorCount = 1;
    processorInfo.ProcessorsPerVirtualSocket = 1;
    processorInfo.ThreadsPerProcessor = 1;
    if (!ConfigSetProcessorInfo(&processorInfo))
    {
        ASSERT(FALSE);
    }

    //
    // Build a config structure with a statically defined configuration.
    //

    ZeroMem(&configFlags, sizeof(configFlags));
    configFlags.Flags.DebuggerEnabled = 1;
    if (!ConfigSetUefiConfigFlags(&configFlags))
    {
        ASSERT(FALSE);
    }

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
