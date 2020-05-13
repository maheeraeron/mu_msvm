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
#include <BiosInterface.h>
#include <Platform.h>
#include <Config.h>
#include <Hob.h>
#include <Hv.h>
#include <Guid/MemoryTypeInformation.h>
#include <IndustryStandard/Acpi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#if defined(MDE_CPU_AARCH64)
#include <Mmu.h>
#endif
#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)
#include <Library/MtrrLib.h>
#endif
#include <Library/PeiServicesLib.h>
#include <Library/ResourcePublicationLib.h>
#include <Ppi/MasterBootMode.h>
#include <Ppi/ConfigPpi.h>

#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)
#define ADDFIRSTMEMORYRANGE AddFirstMemoryRangeIntel
#elif defined(MDE_CPU_AARCH64)
#define ADDFIRSTMEMORYRANGE AddFirstMemoryRangeArm
#else
#error Unsupported Architecture
#endif

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

//
// Read/write Bios Device helper functions.
//
// N.B. Don't use the common library as PEI should not use mutable global
// variables, which only work in our environment because the whole UEFI image is
// located in read/write system memory. In the case of MMIO, the address space
// is identity mapped throughout PEI and does not change.
//
static
VOID
WriteBiosDevice(
    IN UINT32 AddressRegisterValue,
    IN UINT32 DataRegisterValue
    )
{
    UINTN biosBaseAddress = PcdGet32(PcdBiosBaseAddress);
#if defined(MDE_CPU_AARCH64)
    MmioWrite32(biosBaseAddress, AddressRegisterValue);
    MmioWrite32(biosBaseAddress + 4, DataRegisterValue);
#elif defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)
    IoWrite32(biosBaseAddress, AddressRegisterValue);
    IoWrite32(biosBaseAddress + 4, DataRegisterValue);
#endif
}

static
UINT32
ReadBiosDevice(
    IN UINT32 AddressRegisterValue
    )
{
    UINTN biosBaseAddress = PcdGet32(PcdBiosBaseAddress);
#if defined(MDE_CPU_AARCH64)
    MmioWrite32(biosBaseAddress, AddressRegisterValue);
    return MmioRead32(biosBaseAddress + 4);
#elif defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)
    IoWrite32(biosBaseAddress, AddressRegisterValue);
    return IoRead32(biosBaseAddress + 4);
#endif
}

#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)
UINTN
GetPageTableSize(
    _In_ CONST UINT8 PhysicalAddressWidth
    )
/*++

Routine Description:

    Calculates the page table size.

Arguments:

    PhysicalAddressWidth - The number of bits in the address width.

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

    DEBUG((DEBUG_VERBOSE, ">>> GetPageTableSize(%d)\n", PhysicalAddressWidth));

    //
    // If IA32 and PcdDxeIplSwitchToLongMode is false return zero.
    //
#if defined(MDE_CPU_IA32)
    if (!FeaturePcdGet(PcdDxeIplSwitchToLongMode))
    {
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

    if (PhysicalAddressWidth <= 39)
    {
        pml4Entries = 1;
        pdpEntries = 1 << (PhysicalAddressWidth - 30);
        ASSERT(pdpEntries <= 0x200);
    }
    else
    {
        pml4Entries = 1 << (PhysicalAddressWidth - 39);
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
#endif

#if defined(MDE_CPU_AARCH64)
VOID
AddFirstMemoryRangeArm(
    _Inout_ PPLATFORM_INIT_CONTEXT Context,
    _In_ CONST UINT64 Length
)
/*++

Routine Description:

    Utility function to handle special case of memory range zero.
    This function has the side effect of initializing PEI memory
    so the HOB add functions can be used.

Arguments:

    Context - The platform init context.

    Length - The size in bytes of the first memory range.

Return Value:

    None.

--*/
{
    EFI_STATUS status;
    UINT32 configBlobSize = PcdGet32(PcdConfigBlobSize);
    UINT64 configBlobBase = (UINT64) GetStartOfConfigBlob();
    UINT64 peiBase, peiSize;

    //
    // Round config blob size to 4K page increment.
    //
    if (configBlobSize % SIZE_4KB != 0)
    {
        configBlobSize = ((configBlobSize / SIZE_4KB) + 1) * SIZE_4KB;
    }

    //
    // Establish PEI memory first so we can create HOBs in the formal PEI heap.
    // Subtract the size used by the config blob, which starts at the beginning
    // of system memory.
    //
    peiBase = configBlobBase + configBlobSize;
    peiSize = PcdGet32(PcdSystemMemorySize) - configBlobSize;
    status = PublishSystemMemory(peiBase, peiSize);
    ASSERT_EFI_ERROR(status);

    //
    // Declare the whole range as system memory.
    //
    HobAddMemoryRange(Context, 0, Length);

    //
    // Mark the firmware image and config blob as allocated, allowing it to
    // be reclaimed by the guest OS later.
    // TODO-cho: What about pagetable, stack, heap used in PEI? This seems
    // to boot fine.
    //
    HobAddAllocatedMemoryRange(0, PcdGet32(PcdFdSize));
    HobAddAllocatedMemoryRange(configBlobBase, configBlobSize);
}
#endif

#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)
VOID
AddFirstMemoryRangeIntel(
    _Inout_ PPLATFORM_INIT_CONTEXT Context,
    _In_ CONST UINT64 Length
)
/*++

Routine Description:

    Utility function to handle special case of memory range zero
    that is not fully described by the vm worker process memory range.
    This function has the side effect of initializing PEI memory
    so the HOB add functions can be used.

Arguments:

    Context - The platform init context.

    Length - The size in bytes of the first memory range.

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
    pageTableSize = GetPageTableSize(Context->PhysicalAddressWidth);
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
    HobAddMemoryRange(Context, 0, SIZE_512KB + SIZE_128KB);

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
    HobAddMemoryRange(Context, BASE_1MB, Length - SIZE_1MB);

    //
    // Mark the region occupied by the firmware, along with the page tables, GDT
    // entries, free RW pages and config blob as allocated, which should allow
    // it to be reclaimed by the guest OS.
    //
    UINT64 reservedBlockSize =
        PcdGet32(PcdFdSize) +
        SIZE_4KB * MISC_PAGE_COUNT_TOTAL +
        configBlobSize;
    HobAddAllocatedMemoryRange(PcdGet64(PcdFdBaseAddress), reservedBlockSize);

    DEBUG((DEBUG_VERBOSE, "<<< AddFirstMemoryRange\n"));
}
#endif

VOID
InitializeMemoryMap(
    _Inout_ PPLATFORM_INIT_CONTEXT Context
    )
/*++

Routine Description:

    Initializes the memory map of the vm by creating appropriate HOBs and
    triggering the MTRRs to be initialized.

Arguments:

    Context - The platform init context.

Return Value:

    None.

--*/
{
    BOOLEAN legacyMemoryMap = PcdGetBool(PcdLegacyMemoryMap);
    UINT32 memMapSize = PcdGet32(PcdMemoryMapSize);
    VOID* memMap = (VOID*)(UINTN) PcdGet64(PcdMemoryMapPtr);

    //
    // Process the memory map and create HOBs for memory regions..
    //
    if (legacyMemoryMap)
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
                ADDFIRSTMEMORYRANGE(Context, range->Length);
            }
            else
            {
                HobAddMemoryRange(Context, range->BaseAddress, range->Length);
            }

            //
            // Next memory map range.
            //
            range++;
        } while ((UINT8*)range < ((UINT8*)memMap + memMapSize));
    }
    else
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
                ADDFIRSTMEMORYRANGE(Context, rangeV5->Length);
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
#if defined (MDE_CPU_X64)
                    // On X64, system memory above 4GB can cause UEFI drivers
                    // to explode in bad ways due to UINT32 casts. Just mark
                    // regions above 4GB as untested, and use the null memory
                    // test later in BDS to mark them as tested.
                    if (rangeV5->BaseAddress >= 0x100000000)
                    {
                        HobAddUntestedMemoryRange(Context, rangeV5->BaseAddress, rangeV5->Length);
                    }
                    else
                    {
                        HobAddMemoryRange(Context, rangeV5->BaseAddress, rangeV5->Length);
                    }
#else
                    // On other architectures, just add the memory range like normal.
                    HobAddMemoryRange(Context, rangeV5->BaseAddress, rangeV5->Length);
#endif
                }
            }

            //
            // Next memory map range.
            //
            rangeV5++;
        } while ((UINT8*)rangeV5 < ((UINT8*)memMap + memMapSize));
    }

#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)
    //
    // Initialize the fixed MTRR for low memory.
    // The variable MTRRs are set later in this function with a trigger to
    // the VDev.
    //
    // N.B. This call also has the effect of enabling MTRRs. The default
    // MTRR type remains uncached.
    //
    MtrrSetMemoryAttribute(0, SIZE_512KB + SIZE_128KB, CacheWriteBack);
#endif

    //
    // Low and high MMIO range
    //
#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)
    HobAddMmioRange(
        PcdGet64(PcdLowMmioGapBasePageNumber) * SIZE_4KB,
        PcdGet64(PcdLowMmioGapSizeInPages) * SIZE_4KB
        );
#elif defined(MDE_CPU_AARCH64)
    //
    // For ARM64 we are still using the BIOS psuedo-device for runtime services.
    // However the registers are now in MMIO space instead of IO space. Therefore the
    // addresses need to be translated after the guest calls SetVirtualAddressMap.
    // To have the address range included with the guest's call to SetVirtualAddressMap
    // the range has to be declared as DXE runtime memory. That has to be done in DXE phase
    // by a driver so the range can't be declared as MMIO here.  Therefore leave that page
    // out of this early general platform declaration.
    //
    UINT64 GapBase = PcdGet32(PcdBiosBaseAddress);
    UINT64 GapSize = SIZE_4KB;
    UINT64 FirstRangeBase = PcdGet64(PcdLowMmioGapBasePageNumber) * SIZE_4KB;
    UINT64 FirstRangeSize = GapBase - FirstRangeBase;
    UINT64 SecondRangeBase = FirstRangeBase + FirstRangeSize + GapSize;
    UINT64 SecondRangeSize = (PcdGet64(PcdLowMmioGapSizeInPages) * SIZE_4KB) -
                             (FirstRangeSize + GapSize);

    HobAddMmioRange(
        FirstRangeBase,
        FirstRangeSize
        );

    HobAddMmioRange(
        SecondRangeBase,
        SecondRangeSize
        );
#endif
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
    HobAddCpu(Context->PhysicalAddressWidth, 16);

#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)
    //
    // Tell the BiosDevice to set up the variable MTRRs.
    //
    WriteBiosDevice(BiosConfigBootFinalize, Context->PhysicalAddressWidth);
#endif

#if defined(MDE_CPU_AARCH64)
    //
    // Configure the MMU.
    //
    ConfigureMmu(
        (1ULL << Context->PhysicalAddressWidth) - 1
        );
#endif

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
    BOOLEAN connectedToHypervisor = FALSE;
    PLATFORM_INIT_CONTEXT context;
    EFI_STATUS status;

    DEBUG((DEBUG_VERBOSE, ">>> *** Platform PEIM InitializePlatform@%p\n", InitializePlatform));

    ZeroMem(&context, sizeof(context));

    //
    // Get the configuration from the worker process.
    //
    status = GetConfiguration(PeiServices, &context.PhysicalAddressWidth);
    if (EFI_ERROR(status))
    {
        ASSERT(FALSE);
        return status;
    }

    context.StartOfConfigBlob = GetStartOfConfigBlob();

    //
    // DxeBdLib.c InitializeDebugAgent is called very early on in DXE Core,
    // before any drivers are dispatched. Thus, we need to send this boolean
    // flag via a HOB since the Pcd module isn't yet available.
    //
    BOOLEAN debuggerEnabled = PcdGetBool(PcdDebuggerEnabled);
    HobAddGuidData(&gMsvmDebuggerEnabledGuid,
        &debuggerEnabled,
        sizeof(BOOLEAN));

    //
    // Set the boot mode and installs the boot mode tag PPI.
    //
    status = PeiServicesSetBootMode(BOOT_WITH_FULL_CONFIGURATION);
    ASSERT_EFI_ERROR(status);

    status = PeiServicesInstallPpi(MsvmBootModePpiDescriptor);
    ASSERT_EFI_ERROR(status);

    if (HvInitialize())
    {
        DEBUG((DEBUG_INFO, "System detected as isolated, connecting to hypervisor\n"));
        status = HvConnectToHypervisor(&context);
        if (EFI_ERROR(status))
        {
            //
            // TODO-19259739: Have a better way of reporting UEFI errors.
            //
            ASSERT(FALSE);
            CpuDeadLoop();
        }

        connectedToHypervisor = TRUE;
    }

    //
    // Init memory map before publishing any other HOBs.
    //
    InitializeMemoryMap(&context);

    //
    // Publish the FV HOB.
    //
    DEBUG((DEBUG_VERBOSE, "--- InitializePlatform FV Base %p Size %lx\n",
        PcdGet64(PcdFvBaseAddress), PcdGet32(PcdFvSize)));
    HobAddFvMemoryRange(PcdGet64(PcdFvBaseAddress), PcdGet32(PcdFvSize));

    //
    // Init the watchdog (available starting with Threshold VDev)
    //
    InitializeWatchdog();

    if (connectedToHypervisor)
    {
        HvDisconnectFromHypervisor(&context);
    }

    DEBUG((DEBUG_VERBOSE, "<<< *** Platform PEIM InitializePlatform@%p\n", InitializePlatform));

    return EFI_SUCCESS;
}

