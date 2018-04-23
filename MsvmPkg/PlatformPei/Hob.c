/*++

Copyright (c) Microsoft Corporation

Module Name:

    Hob.c

Abstract:

    Hob-building functionality.

Author:

    Rich Yampell (richyam) 8-Jun-2012

--*/

#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <BiosInterface.h>
#include <Platform.h>
#include <Config.h>
#include <Hob.h>
#include <Hv.h>


#define BASIC_FLAGS                                     \
    (EFI_RESOURCE_ATTRIBUTE_PRESENT |                   \
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED)

#define STANDARD_FLAGS                                  \
    (BASIC_FLAGS |                                      \
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |               \
     EFI_RESOURCE_ATTRIBUTE_TESTED)

#define MEMORY_FLAGS                                    \
    (STANDARD_FLAGS |                                   \
     EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |         \
     EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |   \
     EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE)

#define PERSISTENT_MEMORY_FLAGS                         \
    (MEMORY_FLAGS |                                     \
     EFI_RESOURCE_ATTRIBUTE_PERSISTENT)

const char * const gDebugMemoryFormat = "HOB Start % 17lx End %17lx %s\n";
const char * const gDebugCpuFormat    = "HOB MemWidth %d IOWidth %d Cpu\n";
const char * const gDebugGuidFormat   = "HOB Base % 17lx Size %17lx GUID Data\n";


static
VOID
HobpAcceptRamPages(
    _Inout_ PPLATFORM_INIT_CONTEXT Context,
    _In_ HV_GPA_PAGE_NUMBER GpaPageBase,
    _In_ UINT64 PageCount
    )
/*++

Routine Description:

    Accepts a range of RAM GPA pages while ignoring (excluding) pre-accepted pages.

Arguments:

    Context - The platform init context.

    GpaPageBase - Supplies the address of the first target GPA to accept. The
        remaining pages will be modified sequentially from this GPA.

    PageCount - The number of pages to modify.

Return Value:

    None.

--*/
{
    HV_STATUS hvStatus;
    UINT64 pageCountProcessed;
    UINT32 configBlobSize = PcdGet32(PcdConfigBlobSize);
    UINT64 configBlobBase = (UINT64)Context->StartOfConfigBlob;
    HV_GPA_PAGE_NUMBER configBlobPageLimit =
        ((configBlobBase + configBlobSize - 1) / EFI_PAGE_SIZE) + 1;

    //
    // The region from 0 to the end of the config blob is expected to be pre-accepted, so exclude
    // that from the range.
    //
    if (GpaPageBase < configBlobPageLimit)
    {
        if (GpaPageBase + PageCount > configBlobPageLimit)
        {
            PageCount -= configBlobPageLimit - GpaPageBase;
            GpaPageBase = configBlobPageLimit;
        }
        else
        {
            //
            // The region is entirely pre-accepted - there is nothing to do.
            //
            return;
        }
    }

    hvStatus = HvAcceptGpaPages(Context,
                                HvAcceptMemoryTypeRam,
                                HV_MAP_GPA_READABLE | HV_MAP_GPA_WRITABLE,
                                GpaPageBase,
                                PageCount,
                                &pageCountProcessed);

    if (hvStatus != HV_STATUS_SUCCESS)
    {
        //
        // This is a host error, i.e. the memory map does not match the GPA mappings.
        //
        // TODO(wjliu): Report this error in a better way to increase debuggability.
        //
        CpuDeadLoop();
    }

    ASSERT(pageCountProcessed == PageCount);
}


void
HobAddMmioRange(
    __in EFI_PHYSICAL_ADDRESS BaseAddress,
    __in UINT64               Size
    )
/*++

Routine Description:

    Adds an mmio range hob to the current hob list.

Arguments:

    BaseAddress - Base address of the mmio range.

    Size - Size of the mmio range.

Return Value:

    None.

--*/
{
    BuildResourceDescriptorHob(EFI_RESOURCE_MEMORY_MAPPED_IO, 
                               STANDARD_FLAGS, 
                               BaseAddress, 
                               Size);
    DEBUG((DEBUG_VERBOSE, 
           gDebugMemoryFormat, 
           BaseAddress, 
           BaseAddress + Size - 1, 
           L"MMIO"));
}


void
HobAddMemoryRange(
    _Inout_ PPLATFORM_INIT_CONTEXT  Context,
    _In_ EFI_PHYSICAL_ADDRESS       BaseAddress,
    _In_ UINT64                     Size
    )
/*++

Routine Description:

    Adds a memory range hob to the current hob list.

Arguments:

    Context - The platform init context.

    BaseAddress - Base address of the memory range.

    Size - Size of the memory range.

Return Value:

    None.

--*/
{
    ASSERT((BaseAddress % EFI_PAGE_SIZE) == 0);
    ASSERT((Size % EFI_PAGE_SIZE) == 0);

    if (PcdGetBool(PcdSystemIsolated))
    {
        HobpAcceptRamPages(Context, BaseAddress / EFI_PAGE_SIZE, Size / EFI_PAGE_SIZE);
    }

    BuildResourceDescriptorHob(EFI_RESOURCE_SYSTEM_MEMORY, 
                               MEMORY_FLAGS, 
                               BaseAddress, 
                               Size);
    DEBUG((DEBUG_VERBOSE, 
           gDebugMemoryFormat, 
           BaseAddress, 
           BaseAddress + Size - 1, 
           L"Memory"));
}


void
HobAddPersistentMemoryRange(
    __in EFI_PHYSICAL_ADDRESS BaseAddress,
    __in UINT64               Size
    )
/*++

Routine Description:

    Adds a persistent memory range hob to the current hob list.

Arguments:

    BaseAddress - Base address of the memory range.

    Size - Size of the memory range.

Return Value:

    None.

--*/
{
    BuildResourceDescriptorHob(EFI_RESOURCE_SYSTEM_MEMORY, 
                               PERSISTENT_MEMORY_FLAGS, 
                               BaseAddress, 
                               Size);
    DEBUG((DEBUG_VERBOSE, 
           gDebugMemoryFormat, 
           BaseAddress, 
           BaseAddress + Size - 1, 
           L"Memory"));
}


void
HobAddReservedMemoryRange(
    __in EFI_PHYSICAL_ADDRESS BaseAddress,
    __in UINT64               Size
    )
/*++

Routine Description:

    Adds a reserved memory range hob to the current hob list.    

Arguments:

    BaseAddress - Base address of the reserved memory range.

    Size - Size of the reserved memory range.

Return Value:

    None.

--*/
{
    BuildResourceDescriptorHob(EFI_RESOURCE_MEMORY_RESERVED, 
                               STANDARD_FLAGS, 
                               BaseAddress, 
                               Size);
    DEBUG((DEBUG_VERBOSE, 
           gDebugMemoryFormat, 
           BaseAddress, 
           BaseAddress + Size - 1, 
           L"Reserved Memory"));
}


void
HobAddUntestedMemoryRange(
    __in EFI_PHYSICAL_ADDRESS BaseAddress,
    __in UINT64               Size
    )
/*++

Routine Description:

    Adds an untested memory range hob to the current hob list.

Arguments:

    BaseAddress - Base address of the untested memory range.

    Size - Size of the untested memory range.

Return Value:

    None.

--*/
{
    BuildResourceDescriptorHob(EFI_RESOURCE_SYSTEM_MEMORY,
                               MEMORY_FLAGS & ~EFI_RESOURCE_ATTRIBUTE_TESTED,
                               BaseAddress,
                               Size);
    DEBUG((DEBUG_VERBOSE, 
           gDebugMemoryFormat, 
           BaseAddress, 
           BaseAddress + Size - 1, 
           L"Untested Memory"));
}


void
HobAddAllocatedMemoryRange(
    __in EFI_PHYSICAL_ADDRESS BaseAddress,
    __in UINT64               Size
    )
/*++

Routine Description:

    Adds a allocated memory range hob to the current hob list.

Arguments:

    BaseAddress - Base address of the fv memory range.

    Size - Size of the untested fv range.

Return Value:

    None.

--*/
{
    BuildMemoryAllocationHob(BaseAddress, Size, EfiBootServicesData);
    DEBUG((DEBUG_VERBOSE, 
           gDebugMemoryFormat, 
           BaseAddress, 
           BaseAddress + Size - 1, 
           L"Allocated Memory"));
}


void
HobAddFvMemoryRange(
    __in EFI_PHYSICAL_ADDRESS BaseAddress,
    __in UINT64               Size
    )
/*++

Routine Description:

    Adds a fv memory range hob to the current hob list.

Arguments:

    BaseAddress - Base address of the fv memory range.

    Size - Size of the untested fv range.

Return Value:

    None.

--*/
{
    BuildFvHob(BaseAddress, Size);
    DEBUG((DEBUG_VERBOSE, 
           gDebugMemoryFormat, 
           BaseAddress, 
           BaseAddress + Size - 1, 
           L"Firmware Volume"));
}

void
HobAddIoRange(
    __in EFI_PHYSICAL_ADDRESS BaseAddress,
    __in UINT64               Size
    )
/*++

Routine Description:

    Adds an io port range to the current hob.

Arguments:

    BaseAddress - Base address of the io port range.

    Size - Size of the io port range.

Return Value:

    None.

--*/
{
    BuildResourceDescriptorHob(EFI_RESOURCE_IO, BASIC_FLAGS, BaseAddress, Size);
    DEBUG((DEBUG_VERBOSE, 
           gDebugMemoryFormat, 
           BaseAddress, 
           BaseAddress + Size - 1, 
           L"IO Ports"));
}


void
HobAddCpu(
    __in UINT8 SizeOfMemorySpace,
    __in UINT8 SizeOfIoSpace
    )
/*++

Routine Description:

    Adds a cpu hob to the current hob list.

Arguments:

    SizeOfMemorySpace - The size of the cpu's memory space.

    SizeOfIoSpace - The size of the cpu's io space.

Return Value:

    None.

--*/
{
    BuildCpuHob(SizeOfMemorySpace, SizeOfIoSpace);
    DEBUG((DEBUG_VERBOSE, gDebugCpuFormat, SizeOfMemorySpace, SizeOfIoSpace));
}


void
HobAddGuidData(
    __in EFI_GUID* Guid,
    __in VOID*     Data,
    __in UINTN     DataSize
  )
/*++

Routine Description:

    Adds a guid data hob to the current hob list.  This appears to just
    association some blob of data with a given guid.

Arguments:

    Guid - The guid to which the data will be associated data.

    Data - The data associated with the guid.

    DataSize - The size of Data.

Return Value:

    None.

--*/
{
    BuildGuidDataHob(Guid, Data, DataSize);
    DEBUG((DEBUG_VERBOSE, gDebugGuidFormat, Data, DataSize));
}

