/*++

    Copyright (c) Microsoft Corporation

Module Name:

    CrashDump.c

Abstract:

    This module contains code to implement x64 specific crash dump generation

Author:

    Kris Harper (kharp) 16-Sep-2013

Environment:

    UEFI

--*/

#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include "EfiKd.h"
#include "CrashDump.h"

// Based on length of the longest UEFI driver name
// aligned to the nearest power of 2. Most drivers
// are shorter than this so there should be plenty of space
#define STRING_TABLE_ENTRY_LENGTH   (32 * sizeof(UINT16))

/**
  Builds the module list dump request block.
  The module list consists of an array of DUMP_DRIVER_ENTRY64 structures
  followed by a packed list of DUMP_STRINGs.  Each DUMP_DRIVER_ENTRY64
  offsets into the DUMP_STRING list.  DUMP_STRINGs are 8-byte aligned.

  @return TRUE on success, FALSE on failure.
**/
BOOLEAN
EfiBuildCrashDumpModuleList()
{
    MEMORY_DUMP_MODULE_LIST *pModuleBlock;
    DUMP_DRIVER_ENTRY64 *pModuleList;
    DUMP_STRING *pCurString;
    LIST_ENTRY  *pEntry;
    UINT32 tableSize;
    UINT32 stringSpace;
    UINT32 index;

    stringSpace = EfiKdModuleListCount * (sizeof(DUMP_STRING) + STRING_TABLE_ENTRY_LENGTH);
    tableSize   = (EfiKdModuleListCount * sizeof(DUMP_DRIVER_ENTRY64)) + stringSpace;

    if (!EfiDumpBufferAllocateBlockEx(&EfiKdDumpBuffer,
            BlockTypeModuleList,
            tableSize,
            &pModuleBlock))
    {
        return FALSE;
    }

    pModuleBlock->LoadedModuleListPtr = (UINT64)&EfiKdModuleList;
    pModuleBlock->ModuleCount = EfiKdModuleListCount;

    //
    // module list immediately follows the header block
    // and the string table follows the module list
    //
    pModuleList = (DUMP_DRIVER_ENTRY64*)(pModuleBlock + 1);
    pCurString  = (DUMP_STRING*)(pModuleList + EfiKdModuleListCount);

    DEBUG((EFI_D_VERBOSE, "Module List Block Base: %p (%x), List Base: %p, String Base: %p\n",
        pModuleBlock, tableSize, pModuleList, pCurString)); 

    pEntry = EfiKdModuleList.ForwardLink;
    index  = 0;

    while (pEntry != &EfiKdModuleList)
    {
        LDR_DATA_TABLE_ENTRY *pLoadedModule = 
            CONTAINING_RECORD(pEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        UINT32 stringLength;

        pModuleList[index].LdrEntry.DllBase       = pLoadedModule->DllBase;
        pModuleList[index].LdrEntry.EntryPoint    = pLoadedModule->EntryPoint;
        pModuleList[index].LdrEntry.SizeOfImage   = pLoadedModule->SizeOfImage;
        pModuleList[index].LdrEntry.CheckSum      = pLoadedModule->CheckSum;
        pModuleList[index].LdrEntry.TimeDateStamp = pLoadedModule->TimeDateStamp;

        //
        // Copy the name into the string table.
        // If there is not enough space the string will be truncated or not copied
        // at all.
        //
        stringLength = MIN(pLoadedModule->BaseDllName.Length, stringSpace);

        //
        // To write a string we need space for the DUMP_STRING, at lease 1 character
        // and a null terminator.
        //
        if (stringLength > (sizeof(DUMP_STRING) + (sizeof(UINT16) * 2)))
        {
            pModuleList[index].DriverNameOffset = (UINT32)((UINTN)pCurString - (UINTN)pModuleBlock);
            CopyMem(pCurString->Buffer, pLoadedModule->BaseDllName.Buffer, stringLength);

            DEBUG((EFI_D_VERBOSE, " - Module %s, Name Offset %x String Space %x\n",
                pLoadedModule->BaseDllName.Buffer, pModuleList[index].DriverNameOffset, stringSpace));

            //
            // Despite the comment in the DUMP_STRING definition that length
            // is in bytes, both the NT kernel and debugging tools treat the length as
            // length in characters, not bytes.
            //   See IopWriteDriverList in ntos\io\iomgr\dumpctl.c
            //
            pCurString->Length = (stringLength / sizeof(UINT16));

            ASSERT(pCurString->Buffer[pCurString->Length] == L'\0');

            // update to point to the next string
            pCurString = (DUMP_STRING*)((UINT8*)pCurString + sizeof(*pCurString) + 
                                       (pCurString->Length * sizeof(UINT16)) +
                                       sizeof(UINT16));

            pCurString = ALIGN_POINTER(pCurString, 8);

            // update the amount of space left in the string pool
            stringSpace = tableSize - (UINT32)((UINTN)pCurString - (UINTN)pModuleList); 
            
        }

        index++;
        pEntry = pEntry->ForwardLink;
    }

    //
    // All modules are accounted for.
    // FUTURE: 20-sep-2013 kharp - Compact the string pool
    //    Unused string pool could be freed back to the dump buffer
    //    If this is implemented, the string pool size will need to be
    //    added to the dump block.
    //
    return TRUE;
}


/**
  Creates the guest memory dump request structure
  gathering the needed information and memory regions.

  @param  Context           Saved context.
  @param  ExceptionRecord   Optional exception record
  @param  BugCheckCode      Reason for the fatal error.
  @param  Param1            Bugcheck code specific parameter.
  @param  Param2            Bugcheck code specific parameter.
  @param  Param3            Bugcheck code specific parameter.
  @param  Param4            Bugcheck code specific parameter.

  @return EFI_STATUS
**/
EFI_STATUS
EfiBuildCrashDump(
    _In_        CONTEXT              *Context,
    _In_opt_    EXCEPTION_RECORD     *ExceptionRecord,
    _In_        UINT32                BugCheckCode,
    _In_        UINTN                 Param1,
    _In_        UINTN                 Param2,
    _In_        UINTN                 Param3,
    _In_        UINTN                 Param4
    )
{
    MEMORY_DUMP_INFO         *pInfo;
    MEMORY_DUMP_BLOCK_HEADER *pGenericBlock;
    EFI_STATUS                status;

    if (EfiKdDumpType == MemoryDumpDisabled)
    {
        status = RETURN_NOT_READY;
        goto Exit;
    }

    ZeroMem(EfiKdDumpBuffer.Buffer, EfiKdDumpBuffer.BufferSize);

    //
    // Info Block
    //
    if (!EfiDumpBufferAllocateBlock(&EfiKdDumpBuffer, BlockTypeInfo, &pInfo))
    {
        ASSERT(FALSE);
        status = RETURN_OUT_OF_RESOURCES;
        goto Exit;
    }

    pInfo->MajorVersion   = 0x400 | (INT16)((NtBuildNumber >> 28) & 0xFFFFFFF);
    pInfo->MinorVersion   = (INT16)NtBuildNumber;
    pInfo->ProcessorCount = 1;
    pInfo->DumpType       = EfiKdDumpType;

    pInfo->BugCheckCode = BugCheckCode;
    pInfo->BugCheckParams[0] = Param1;
    pInfo->BugCheckParams[1] = Param2;
    pInfo->BugCheckParams[2] = Param3;
    pInfo->BugCheckParams[3] = Param4;

    //
    // Context Block
    //
    if (!EfiDumpBufferAllocateBlockEx(&EfiKdDumpBuffer,
            BlockTypeContext,
            sizeof(*Context),
            &pGenericBlock))
    {
        ASSERT(FALSE);
        status = RETURN_OUT_OF_RESOURCES;
        goto Exit;
    }

    CopyMem((pGenericBlock + 1), Context, sizeof(*Context));

    //
    // Exception Block
    //
    if (ExceptionRecord != NULL)
    {
        if (!EfiDumpBufferAllocateBlockEx(&EfiKdDumpBuffer,
                BlockTypeException,
                sizeof(*ExceptionRecord),
                &pGenericBlock))
        {
            ASSERT(FALSE);
            status = RETURN_OUT_OF_RESOURCES;
            goto Exit;
        }

        CopyMem((pGenericBlock + 1), ExceptionRecord, sizeof(*ExceptionRecord));
    }

    //
    // Triage Dump Specific
    // If other dump types are supported in the future
    // add dump type specific handling here.
    //

    status = EfiCrashDumpAddTriageInfo(Context, ExceptionRecord);

    if (EFI_ERROR(status))
    {
        goto Exit;
    }

    //
    // End of List Block
    //
    if (!EfiDumpBufferAllocateBlock(&EfiKdDumpBuffer, BlockTypeListEnd, &pGenericBlock))
    {
        ASSERT(FALSE);
        status = RETURN_OUT_OF_RESOURCES;
        goto Exit;
    }

    pGenericBlock->Type = BlockTypeListEnd;
    pGenericBlock->Size = sizeof(*pGenericBlock);
    
    //
    // Update header
    //
    pInfo->BlockCount  = EfiKdDumpBuffer.BlockCount;

Exit:

    return status;
}

/**
  Adds triage dump specific blocks to the global dump buffer

  @param  Context           Saved context.
  @param  ExceptionRecord   Optional exception record

  @return EFI_STATUS
**/
EFI_STATUS
EfiCrashDumpAddTriageInfo(
    _In_        CONTEXT              *Context,
    _In_opt_    EXCEPTION_RECORD     *ExceptionRecord
    )
{
    MEMORY_DUMP_REGION       *pMemoryRegion;
    UINTN                     regionStart;
    EFI_STATUS                status;

    //
    // Stack Block
    //
    if (!EfiDumpBufferAllocateBlock(&EfiKdDumpBuffer, BlockTypeMemoryRegion, &pMemoryRegion))
    {
        ASSERT(FALSE);
        status = RETURN_OUT_OF_RESOURCES;
        goto Exit;
    }

    pMemoryRegion->Header.SubType = MEMORY_REGION_TYPE_STACK;
    pMemoryRegion->Address = (UINT64)EfiKdPcr.NtTib.StackBase;
    pMemoryRegion->Size    = (UINT32)EfiKdPcr.NtTib.StackLimit;

    //
    // PRCB Block
    //
    if (!EfiDumpBufferAllocateBlock(&EfiKdDumpBuffer, BlockTypeMemoryRegion, &pMemoryRegion))
    {
        ASSERT(FALSE);
        status = RETURN_OUT_OF_RESOURCES;
        goto Exit;
    }

    pMemoryRegion->Header.SubType = MEMORY_REGION_TYPE_PRCB;
    pMemoryRegion->Address = (UINT64)EfiKdPrcb;
    pMemoryRegion->Size    = sizeof(*EfiKdPrcb);

    //
    // Debugger Data block
    //
    if (!EfiDumpBufferAllocateBlock(&EfiKdDumpBuffer, BlockTypeMemoryRegion, &pMemoryRegion))
    {
        ASSERT(FALSE);
        status = RETURN_OUT_OF_RESOURCES;
        goto Exit;
    }

    pMemoryRegion->Header.SubType = MEMORY_REGION_TYPE_DEBUGDATA;
    pMemoryRegion->Address = (UINT64)&EfiKdDebuggerDataBlock;
    pMemoryRegion->Size = sizeof(EfiKdDebuggerDataBlock);

    //
    // For now the only error that can occur is space allocation
    // ignore it as the module list isn't required to open the dump.
    //
    EfiBuildCrashDumpModuleList();

    //
    // Gather interesting memory regions
    // - current code page from context.
    //

    if (!EfiDumpBufferAllocateBlock(&EfiKdDumpBuffer, BlockTypeMemoryRegion, &pMemoryRegion))
    {
        ASSERT(FALSE);
        status = RETURN_OUT_OF_RESOURCES;
        goto Exit;
    }

    pMemoryRegion->Header.SubType = MEMORY_REGION_TYPE_GENERAL;

    //
    // Set the region to include code before and after the current
    // instruction pointer
    //
    if (Context->Rip < (EFI_PAGE_SIZE / 2))
    {
        regionStart = 0;
    }
    else
    {
        regionStart = (Context->Rip - (EFI_PAGE_SIZE / 2));
    }
    
    pMemoryRegion->Address = regionStart;
    pMemoryRegion->Size = EFI_PAGE_SIZE;

    status = RETURN_SUCCESS;

Exit:

    return status;

}
