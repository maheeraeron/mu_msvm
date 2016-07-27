/*++

Copyright (c) Microsoft Corporation

Module Name:

    ExdiSupportDxe.c

Abstract:

    This module is a UEFI DXE driver that provides a DBGKD_GET_VERSION64
    structure at a known offset in memory. This allows the Hyper-V EXDI
    driver to get the UEFI loaded module list.

    To use this, start the EXDI debugger with the VerAddr option to
    specify the location of the version structure. E.g.:

        kd -kx exdi:desc=vmice,kd=VerAddr:0x100000

Author:

    John Starks (jostarks) 1-Jul-2012

--*/

#include <PiDxe.h>

#include <Guid/DebugImageInfoTable.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>

typedef enum _DBGKD_MAJOR_TYPES
{
    DBGKD_MAJOR_NT,
    DBGKD_MAJOR_XBOX,
    DBGKD_MAJOR_BIG,
    DBGKD_MAJOR_EXDI,
    DBGKD_MAJOR_NTBD,
    DBGKD_MAJOR_EFI,
    DBGKD_MAJOR_TNT,
    DBGKD_MAJOR_SINGULARITY,
    DBGKD_MAJOR_HYPERVISOR,
    DBGKD_MAJOR_COUNT
} DBGKD_MAJOR_TYPES;

#define DBGKD_64BIT_PROTOCOL_VERSION1 5
#define DBGKD_64BIT_PROTOCOL_VERSION2 6
#define CURRENT_KD_SECONDARY_VERSION 2

#define DBGKD_VERS_FLAG_MP         0x0001   // kernel is MP built
#define DBGKD_VERS_FLAG_DATA       0x0002   // DebuggerDataList is valid
#define DBGKD_VERS_FLAG_PTR64      0x0004   // native pointers are 64 bits
#define DBGKD_VERS_FLAG_NOMM       0x0008   // No MM - don't decode PTEs
#define DBGKD_VERS_FLAG_HSS        0x0010   // hardware stepping support
#define DBGKD_VERS_FLAG_PARTITIONS 0x0020   // multiple OS partitions exist

typedef struct _DBGKD_GET_VERSION64 {
    UINT16  MajorVersion;
    UINT16  MinorVersion;
    UINT8   ProtocolVersion;
    UINT8   KdSecondaryVersion; // Cannot be 'A' for compat with dump header
    UINT16  Flags;
    UINT16  MachineType;

    //
    // Protocol command support descriptions.
    // These allow the debugger to automatically
    // adapt to different levels of command support
    // in different kernels.
    //

    // One beyond highest packet type understood, zero based.
    UINT8   MaxPacketType;
    // One beyond highest state change understood, zero based.
    UINT8   MaxStateChange;
    // One beyond highest state manipulate message understood, zero based.
    UINT8   MaxManipulate;

    // Kind of execution environment the kernel is running in,
    // such as a real machine or a simulator.  Written back
    // by the simulation if one exists.
    UINT8   Simulation;

    UINT16  Unused[1];

    UINT64 KernBase;
    UINT64 PsLoadedModuleList;

    //
    // Components may register a debug data block for use by
    // debugger extensions.  This is the address of the list head.
    //
    // There will always be an entry for the debugger.
    //

    UINT64 DebuggerDataList;

} DBGKD_GET_VERSION64, *PDBGKD_GET_VERSION64;

typedef LIST_ENTRY LIST_ENTRY64;

typedef struct _UNICODE_STRING
{
    UINT16 Length;
    UINT16 MaximumLength;
    CHAR16 *Buffer;
} UNICODE_STRING;

#pragma warning(disable: 4201)

typedef struct _LDR_DATA_TABLE_ENTRY64 {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    UINT64 DllBase;
    UINT64 EntryPoint;
    UINT32 SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    UINT32   Flags;
    UINT16  LoadCount;
    UINT16 TlsIndex;
    union {
        LIST_ENTRY HashLinks;
        struct {
            UINT64 SectionPointer;
            UINT32 CheckSum;
        };
    };
    union {
        struct {
            UINT32 TimeDateStamp;
        };
        struct {
            UINT64 LoadedImports;
        };
    };

} LDR_DATA_TABLE_ENTRY64, *PLDR_DATA_TABLE_ENTRY64;

EFI_HANDLE mImageLoadEvent;
VOID *mImageLoadRegistration;
DBGKD_GET_VERSION64 *mKdVersion;
LIST_ENTRY mLoadedModuleList;

EFI_DEBUG_IMAGE_INFO_TABLE_HEADER *
ExdiGetImageInfo(
    VOID
    )
/*++

Routine Description:

    Finds the EFI debug image table header in the system configuration table.

Arguments:

    None.

Return Value:

    A pointer to the table header.

--*/
{
    EFI_CONFIGURATION_TABLE *table;
    UINTN index;

    table = gST->ConfigurationTable;
    for (index = 0; index < gST->NumberOfTableEntries; index += 1)
    {
        if (CompareGuid (&table[index].VendorGuid, &gEfiDebugImageInfoTableGuid))
        {
            return table[index].VendorTable;
        }
    }

    return NULL;
}

EFI_IMAGE_NT_HEADERS64 *
ExdiGetNtHeader(
    IN VOID *ImageBase
    )
/*++

Routine Description:

    Returns the NT headers for the image at ImageBase.

Arguments:    

    ImageBase - Supplies the base address of the image.

Return Value:

    Pointer to the NT headers.

--*/
{
    EFI_IMAGE_DOS_HEADER *dosHeader;
    EFI_IMAGE_NT_HEADERS64 *ntHeaders;

    dosHeader = ImageBase;
    if (dosHeader->e_magic == EFI_IMAGE_DOS_SIGNATURE)
    {
        ntHeaders = (EFI_IMAGE_NT_HEADERS64 *)
            ((UINTN)ImageBase + (dosHeader->e_lfanew & 0xffff));
    }
    else
    {
        ntHeaders = ImageBase;
    }

    ASSERT (ntHeaders->Signature == EFI_IMAGE_NT_SIGNATURE);
    ASSERT (ntHeaders->OptionalHeader.Magic == EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC);

    return ntHeaders;
}


UINTN
ExdiGetModuleName(
    VOID *ImageBase,
    EFI_IMAGE_NT_HEADERS64 *NtHeaders,
    CHAR8 **ModuleName
    )
/*++

Routine Description:

    Finds the base module name for an image by looking through the debug data
    present in the module headers.

Arguments:

    ImageBase - A pointer to the image base.

    NtHeaders - A pointer to the NT headers for the image.

    ModuleName - Returns a pointer to the module name. The string is NULL
        terminated but includes the .pdb extension; the length without the
        .pdb extension is returned.

Return Value:

    The length of the module base name.

--*/
{
    UINT8 *base;
    UINT8 *debugDirectory;
    EFI_IMAGE_DEBUG_DIRECTORY_ENTRY *entry;
    CHAR8 *pdbPath;
    CHAR8 *pdbName;
    CHAR8 *pdbExt;

    *ModuleName = NULL;
    base = ImageBase;
    if (NtHeaders->OptionalHeader.NumberOfRvaAndSizes <= EFI_IMAGE_DIRECTORY_ENTRY_DEBUG) 
    {
        return 0;
    }

    entry = (EFI_IMAGE_DEBUG_DIRECTORY_ENTRY *)(base +
        NtHeaders->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress);

    debugDirectory = base + entry->RVA;
    switch (*(UINT32 *)debugDirectory) 
    {
    case CODEVIEW_SIGNATURE_NB10:
        pdbPath = (CHAR8 *)debugDirectory + sizeof(EFI_IMAGE_DEBUG_CODEVIEW_NB10_ENTRY);
        break;

    case CODEVIEW_SIGNATURE_RSDS:
        pdbPath = (CHAR8 *)debugDirectory + sizeof(EFI_IMAGE_DEBUG_CODEVIEW_RSDS_ENTRY);
        break;

    default:
        return 0;
    }

    for (pdbName = pdbPath + AsciiStrLen(pdbPath), pdbExt = pdbName;
         pdbName > pdbPath && pdbName[-1] != '\\'; 
         pdbName -= 1)
    {
        if (*pdbName == '.')
        {
            pdbExt = pdbName;
        }
    }

    *ModuleName = pdbName;
    return pdbExt - pdbName;
}


EFI_STATUS
ExdiAddModuleInfo(
    VOID *ImageBase
    )
/*++

Routine Description:

    Adds a module to the loaded module list by image base.

Arguments:

    ImageBase - A pointer to the load address of the module.

Return Value:

    EFI status.

--*/
{
    LDR_DATA_TABLE_ENTRY64 *entry;
    CHAR8 *moduleName;
    UINTN moduleNameLength;
    UINTN nameByteCount;
    EFI_IMAGE_NT_HEADERS64 *ntHeader;

    ntHeader = ExdiGetNtHeader(ImageBase);
    moduleNameLength = ExdiGetModuleName(ImageBase, ntHeader, &moduleName);

    nameByteCount = moduleNameLength * sizeof(CHAR16) + sizeof(L".efi");
    entry = AllocateZeroPool(sizeof(*entry) + nameByteCount);
    if (entry == NULL)
    {
        return EFI_OUT_OF_RESOURCES;
    }

    entry->DllBase = (UINT64)(UINTN)ImageBase;
    entry->SizeOfImage = ntHeader->OptionalHeader.SizeOfImage;
    entry->CheckSum = ntHeader->OptionalHeader.CheckSum;

    if (moduleNameLength != 0)
    {
        entry->BaseDllName.Buffer = (CHAR16 *)(entry + 1);
        UnicodeSPrint(entry->BaseDllName.Buffer, 
                      nameByteCount,
                      L"%.*a.efi",
                      moduleNameLength,
                      moduleName);

        entry->BaseDllName.Length = (UINT16)nameByteCount - sizeof(CHAR16);
        entry->BaseDllName.MaximumLength = (UINT16)nameByteCount;
    }

    entry->LoadCount = 1;
    InsertTailList(&mLoadedModuleList, &entry->InLoadOrderLinks);
    return EFI_SUCCESS;
}


EFI_STATUS
ExdiAddLoadedModule(
    EFI_HANDLE Handle
    )
/*++

Routine Description:

    Adds a module to the loaded module list by image handle.

Arguments:

    Handle - An EFI handle referencing the module.

Return Value:

    EFI status.

--*/
{
    EFI_STATUS status;
    EFI_LOADED_IMAGE_PROTOCOL *image;
  
    status = gBS->HandleProtocol(
        Handle,
        &gEfiLoadedImageProtocolGuid,
        (VOID **)&image);

    if (EFI_ERROR(status)) {
        return status;
    }

    status = ExdiAddModuleInfo(image->ImageBase);
    return EFI_SUCCESS;
}


VOID
ExdiBuildModuleList(
    VOID
    )
/*++

Routine Description:

    Builds a loaded module list containing the currently loaded modules.

Arguments:

    None.

Return Value:

    None.

--*/
{
    EFI_DEBUG_IMAGE_INFO_TABLE_HEADER *imageInfo;
    EFI_DEBUG_IMAGE_INFO_NORMAL *image;
    UINT32 index;
    EFI_STATUS status;

    InitializeListHead(&mLoadedModuleList);
    imageInfo = ExdiGetImageInfo();
    for (index = 0; index < imageInfo->TableSize; index += 1)
    {
        image = imageInfo->EfiDebugImageInfoTable[index].NormalImage;
        if (image->ImageInfoType == EFI_DEBUG_IMAGE_INFO_TYPE_NORMAL)
        {
            status = ExdiAddLoadedModule(image->ImageHandle);
            EFI_ERROR(status);
        }
    }
}

VOID
ExdiOnImageLoad(
    EFI_EVENT *Event,
    VOID *Context
    )
/*++

Routine Description:

    A notify routine called whenever a new module is loaded.

Arguments:

    Event - A pointer to the notify event.

    Context - Unused.

Return Value:

    None.

--*/
{
    EFI_HANDLE handle;
    UINTN size;
    EFI_STATUS status;

    for (;;)
    {
        size = sizeof(handle);
        status = gBS->LocateHandle(ByRegisterNotify,
                                   NULL,
                                   mImageLoadRegistration,
                                   &size,
                                   &handle);

        if (EFI_ERROR(status))
        {
            return;
        }

        ExdiAddLoadedModule(handle);
    }    
}

EFI_STATUS
EFIAPI
ExdiSupportInitialize (
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
    )
/*++

Routine Description:

    The initialization routine for this driver.

Arguments:

    ImageHandle - An EFI handle referencing this driver's image.

    SystemTable - A pointer to the EFI system table.

Return Value:

    EFI status.

--*/
{
    EFI_PHYSICAL_ADDRESS address;
    LDR_DATA_TABLE_ENTRY64 *entry;
    EFI_STATUS status;
    DBGKD_GET_VERSION64 versionTemplate = {
        DBGKD_MAJOR_NTBD << 8, // kd is most forgiving of the boot debugger
        0,
        DBGKD_64BIT_PROTOCOL_VERSION2,
        CURRENT_KD_SECONDARY_VERSION,
        DBGKD_VERS_FLAG_PTR64,
        EFI_IMAGE_MACHINE_X64,
    };

    address = PcdGet64(PcdExdiSupportVersionAddress);
    status = gBS->AllocatePages(AllocateAddress,
                                EfiBootServicesData,
                                1,
                                &address);

    if (EFI_ERROR(status))
    {
        DEBUG ((EFI_D_ERROR, "Failed to get memory for EXDI version tag at %p.\n", (UINTN)address));
        return status;
    }

    mKdVersion = (VOID *)address;
    CopyMem(mKdVersion, &versionTemplate, sizeof(versionTemplate));
    DEBUG ((EFI_D_INFO, "Version at %p\n", mKdVersion));

    //
    // Construct the initial module list.
    //
    
    ExdiBuildModuleList();
    entry = (LDR_DATA_TABLE_ENTRY64 *)mLoadedModuleList.ForwardLink;
    mKdVersion->KernBase = entry->DllBase;
    mKdVersion->PsLoadedModuleList = (UINT64)(UINTN)&mLoadedModuleList;

    //
    // Register an event to be notified of additional module loads.
    //

    status = gBS->CreateEvent(EVT_NOTIFY_SIGNAL,
                              TPL_NOTIFY,
                              ExdiOnImageLoad,
                              NULL,
                              &mImageLoadEvent);

    if (!EFI_ERROR(status))
    {
        status = gBS->RegisterProtocolNotify(&gEfiLoadedImageProtocolGuid,
                                             mImageLoadEvent,
                                             &mImageLoadRegistration);
    }    

    return status;
}

