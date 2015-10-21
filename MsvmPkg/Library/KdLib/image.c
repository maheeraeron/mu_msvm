/*++

Copyright (c) Microsoft Corporation

Module Name:

    image.c

Abstract:

    Contains module load and unload tracking for the UEFI KD implementation.
    Implements the UEFI PeCoffExtraActionLib interface to get notification of
    module loads and unloads.

Author:

    Kris Harper (kharp) 28-Oct-2013 - Based off of NTOS boot debugger under
        minkernel/boot/environ/lib/bd

Environment:

    UEFI

--*/
#include <PiDxe.h>
#include <Base.h>

#include <Guid/DebugImageInfoTable.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/PeCoffExtraActionLib.h>
#include <Library/TimerLib.h>

#include "EfiKd.h"
#include "EfiNt.h"

UINTN
EfiKdGetModuleName(
    _In_        PE_COFF_LOADER_IMAGE_CONTEXT   *ImageContext,
    _Out_       CHAR8                         **ModuleName
    )
/*++

Routine Description:

    Finds the base module name for an image by looking through the debug data
    present in the module headers.

Arguments:

    ImageContext - A pointer to the loaded image context.

    ModuleName   - Returns a pointer to the module name. The string is NULL
        terminated but includes the .pdb extension; the length without the
        .pdb extension is returned.

Return Value:

    The length of the module base name.
    If no debugging information is present 0 is returned.

--*/
{
    CHAR8 *pdbPath = ImageContext->PdbPointer;
    CHAR8 *pdbName;
    CHAR8 *pdbExt;

    if (pdbPath != NULL)
    {
        //
        // Search backwards from the end of the path string
        // and stop at the beginning of the string or first
        // path separator ('\').
        //
        for (pdbName = pdbPath + AsciiStrLen(pdbPath), pdbExt = pdbName;
             pdbName > pdbPath && pdbName[-1] != '\\';
             pdbName -= 1)
        {
            if (*pdbName == '.')
            {
                pdbExt = pdbName;
            }
        }

        DEBUG ((EFI_D_INFO, "PDB Name: %a\n", pdbName));

        *ModuleName = pdbName;
        return pdbExt - pdbName;
    }
    else
    {
        *ModuleName = NULL;
        return 0;
    }

}


EFI_STATUS
EfiKdAddModuleInfo(
    _Inout_     PLIST_ENTRY                     ModuleList,
    _In_        PE_COFF_LOADER_IMAGE_CONTEXT   *ImageContext,
    _Out_       PLDR_DATA_TABLE_ENTRY          *AddedEntry
    )
/*++

Routine Description:

    Adds a module to the loaded module list by image base.

Arguments:

    ModuleList   - List to add the module info to.
    
    ImageContext - A pointer to the load address of the module.

    AddedEntry   - Returns a pointer to the newly added entry.

Return Value:

    EFI status.

--*/
{
    LDR_DATA_TABLE_ENTRY *entry;
    CHAR8 *moduleName;
    UINTN moduleNameLength = 0;
    UINTN nameByteCount = 0;
    EFI_IMAGE_OPTIONAL_HEADER_UNION *ntHeader;

    moduleNameLength = EfiKdGetModuleName(ImageContext, &moduleName);

    if (moduleNameLength > 0)
    {
        nameByteCount = moduleNameLength * sizeof(CHAR16) + sizeof(L".efi");
    }

    //
    // N.B.
    //   The string buffer for BaseDllName is directly after the LDR_DATA_TABLE_ENTRY
    //   struct.  If this changes EfiKdAddUnloadedModuleInfo will also need to change.
    //
    entry = AllocateZeroPool(sizeof(*entry) + nameByteCount);
    if (entry == NULL)
    {
        *AddedEntry = NULL;
        return EFI_OUT_OF_RESOURCES;
    }

    //
    // N.B.
    //      The initial image load for DxeCore the ImageContext is very minimal.
    //      Make sure the needed fields are initialized in InitializeDxeCoreImageContext.
    //
    ASSERT(ImageContext->ImageSize <= MAX_UINT32);
    entry->SizeOfImage = (UINT32)ImageContext->ImageSize;
    entry->DllBase     = ImageContext->ImageAddress;
    entry->EntryPoint  = ImageContext->EntryPoint;

    //
    // Checksum is not provided in the EFI image context
    // get it from the PE header directly.
    //
    ntHeader = (EFI_IMAGE_OPTIONAL_HEADER_UNION*)((UINT8*)ImageContext->ImageAddress +
                ImageContext->PeCoffHeaderOffset);

    if (ntHeader->Pe32.Signature == EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    {
        entry->TimeDateStamp = ntHeader->Pe32Plus.FileHeader.TimeDateStamp;
        entry->CheckSum      = ntHeader->Pe32Plus.OptionalHeader.CheckSum;
    }
    else if (ntHeader->Pe32.Signature == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC)
    {
        entry->TimeDateStamp = ntHeader->Pe32.FileHeader.TimeDateStamp;
        entry->CheckSum      = ntHeader->Pe32.OptionalHeader.CheckSum;
    }
    else
    {
        //
        // Unknown or unsupported image format (could be a TE image)
        // Checksum and timestamp are already zero.
        //
    }

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
    InsertTailList(ModuleList, &entry->InLoadOrderLinks);
    EfiKdModuleListCount++;
    *AddedEntry = entry;

    return EFI_SUCCESS;
}


EFI_STATUS
EfiKdFindAndRemoveModuleInfo(
    _Inout_     PLIST_ENTRY             ModuleListHead,
    _In_        UINTN                   ImageBase,
    _Out_       PLDR_DATA_TABLE_ENTRY  *Module
    )
/*++

Routine Description:

    Attempts to find a module in the loaded module list by image base.
    If the module is found it will be removed from the loaded list and returned
    to the caller.  The caller can free the memory associated with the module if
    it is no longer needed.

Arguments:

    ModuleListHead - Head of the module list.

    ImageBase - A pointer to the load address of the module to be found and removed.

    Module - Returns the found module entry on success
             NULL on failure.

Return Value:

    EFI status.

--*/
{
    LIST_ENTRY *entry = ModuleListHead->ForwardLink;
    EFI_STATUS status = EFI_NOT_FOUND;

    *Module = NULL;

    while (entry != ModuleListHead)
    {
        LDR_DATA_TABLE_ENTRY *module = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        if (module->DllBase == ImageBase)
        {
            //
            // This indicates an attempt to remove the first module loaded, DxeCore
            // (see call to PeCoffLoaderRelocateImageExtraAction in DxeMain) and should never
            // happen.
            // ASSERT and fail just in case as removing the entry will invalidate
            // EfiKdModuleDataTableEntry
            //
            if (entry == (PLIST_ENTRY)EfiKdModuleDataTableEntry)
            {
                ASSERT(FALSE);
                status = EFI_LOAD_ERROR;
                break;
            }

            // module found, remove it from the list and return it.
            RemoveEntryList(entry);
            EfiKdModuleListCount--;
            *Module = module;
            status = EFI_SUCCESS;
            break;
        }

        entry = entry->ForwardLink;
    }

    return status;
}


VOID
EfiKdAddUnloadedModuleInfo(
    _In_        PLDR_DATA_TABLE_ENTRY   Module
    )
/*++

Routine Description:

    Adds the given module to the unloaded module list.

Arguments:

    Module - Module being unloaded.

Return Value:

    Nothing.

--*/
{
    PEFI_UNLOADED_MODULE pUnloadEntry;

    if (EfiKdLastUnloadedModule >= EFI_KD_MAX_UNLOADED_MODULES)
    {
        EfiKdLastUnloadedModule = 0;
    }

    pUnloadEntry = &EfiKdUnloadedModules[EfiKdLastUnloadedModule];

    //
    // Free current entry if needed.
    //
    if (pUnloadEntry->Name.Buffer != NULL)
    {
        //
        // N.B.
        //   The string buffer is allocated directly after the LDR_DATA_TABLE_ENTRY
        //   struct.  Subtract to get to the start of LDR_DATA_TABLE_ENTRY.
        //
        LDR_DATA_TABLE_ENTRY* oldEntry = 
            (PLDR_DATA_TABLE_ENTRY)((PUCHAR)pUnloadEntry->Name.Buffer - sizeof(LDR_DATA_TABLE_ENTRY));

        FreePool(oldEntry);
    }

    pUnloadEntry->Name.Buffer = Module->BaseDllName.Buffer;
    pUnloadEntry->Name.Length = Module->BaseDllName.Length;
    pUnloadEntry->Name.MaximumLength = Module->BaseDllName.MaximumLength;
    
    pUnloadEntry->StartAddress = (PVOID)Module->DllBase;
    pUnloadEntry->EndAddress   = (PUCHAR)Module->DllBase + Module->SizeOfImage;
    pUnloadEntry->CurrentTime  = GetPerformanceCounter();

    EfiKdLastUnloadedModule++;
}


VOID
EfiKdImageNotification(
    _In_        PLDR_DATA_TABLE_ENTRY   Image,
    _In_        UINT8                   Operation
    )
/*++

Routine Description:

    Notifies the debugger about an image load or unload.

Arguments:

    Image       Image entry to signal symbol load or unload

    Operation   BREAKPOINT_LOAD_SYMBOLS or BREAKPOINT_UNLOAD_SYMBOLS

Return Value:

    None.

--*/
{
    CHAR8 buffer[EFI_KD_MAXIMUM_FILENAME_SIZE];
    KD_SYMBOLS_INFO symbolInfo;
    PUNICODE_STRING baseDllName;
    UINTN length;
    STRING string;

    ASSERT((Operation == BREAKPOINT_LOAD_SYMBOLS) ||
           (Operation == BREAKPOINT_UNLOAD_SYMBOLS));

    if (!EfiKdSubsystemInitialized)
    {
        return;
    }

    string.Buffer = buffer;
    string.MaximumLength = ARRAY_SIZE(buffer);
    string.Length = 0;

    //
    // The module entry stores the PDB name as Unicode
    // If it's valid, convert the name to ASCII for the debug service API.
    //
    baseDllName = &Image->BaseDllName;
    if (baseDllName->Length > 0)
    {
        length = (baseDllName->Length / sizeof(WCHAR)) + 1;

        if (length > string.MaximumLength)
        {
            length = string.MaximumLength;
        }

        string.Buffer = buffer;
        AsciiSPrintUnicodeFormat(string.Buffer, length, L"%s", baseDllName->Buffer);
        string.Length = (USHORT)AsciiStrLen(string.Buffer);
        string.MaximumLength = (USHORT)length;
    }

    symbolInfo.SizeOfImage = Image->SizeOfImage;
    symbolInfo.CheckSum    = Image->CheckSum;
    symbolInfo.BaseOfDll   = (PVOID)Image->DllBase;
    symbolInfo.ProcessId   = (UINT_PTR)-1;

    DebugService2(&string, &symbolInfo, BREAKPOINT_LOAD_SYMBOLS);
    DebugPollDebugger();
}


VOID
EFIAPI
PeCoffLoaderRelocateImageExtraAction (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
/*++

Routine Description:

    Performs additional actions after a PE/COFF image has been loaded and 
    relocated.

    If ImageContext is NULL, then ASSERT().

    Note that for the initial image load for DxeCore the ImageContext is
    very minimal.  Make sure the needed fields are initialized in DxeMain.

Arguments:

    ImageContext  The pointer to the image context structure that describes the
        PE/COFF image that has already been loaded and relocated.

Return Value:

    None.

--*/
{
    LDR_DATA_TABLE_ENTRY *entry;
    EFI_STATUS status;

    ASSERT (ImageContext != NULL);

    status = EfiKdAddModuleInfo(&EfiKdModuleList, 
                                ImageContext,
                                &entry);
    if (EFI_ERROR(status))
    {
        return;
    }

    //
    // Setup value of EfiKdModuleTableEntry.
    // This only needs to be done if not set and will most likely point to the module 
    // info for DxeCore (see first call to PeCoffLoaderRelocateImageExtraAction
    // in DxeMain).
    // 
    if (EfiKdModuleDataTableEntry == NULL)
    {
        EfiKdModuleDataTableEntry = (PLDR_DATA_TABLE_ENTRY)EfiKdModuleList.ForwardLink;
        EfiKdDebuggerDataBlock.KernBase = ImageContext->ImageAddress;
    }

    EfiKdImageNotification(entry, BREAKPOINT_LOAD_SYMBOLS);
}


VOID
EFIAPI
PeCoffLoaderUnloadImageExtraAction (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
/*++

Routine Description:

    Performs additional actions just before a PE/COFF image is unloaded.  Any 
    resources that were allocated by PeCoffLoaderRelocateImageExtraAction() 
    must be freed.

    If ImageContext is NULL, then ASSERT().

Arguments:

    ImageContext  The pointer to the image context structure that describes the
        PE/COFF image that is being unloaded.

Return Value:

    None.

--*/
{
    LDR_DATA_TABLE_ENTRY *module;
    EFI_STATUS status;

    ASSERT (ImageContext != NULL);

    status = EfiKdFindAndRemoveModuleInfo(&EfiKdModuleList,
        ImageContext->ImageAddress,
        &module);

    if (EFI_ERROR(status))
    {
        //
        // Didn't find the module.  Typically because it was never loaded and
        // this function is called for LoadImage() failure cleanup.  
        // Therefore don't try to create and send the unload message to the debugger.
        //
        return;
    }

    EfiKdAddUnloadedModuleInfo(module);
    EfiKdImageNotification(module, BREAKPOINT_UNLOAD_SYMBOLS);
}
