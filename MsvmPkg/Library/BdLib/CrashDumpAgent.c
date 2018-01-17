/*++

    Copyright (c) Microsoft Corporation

Module Name:

    CrashDumpAgent.c

Abstract:

    This module contains architecture independent support functions for
    UEFI crash dump support

Author:

    Kris Harper (kharp) 16-Sep-2013

Environment:

    UEFI

--*/

#include <EfiNt.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/CrashDumpAgentLib.h>
#include <Library/HobLib.h>
#include <Library/BaseLib.h>
#include <Library/WatchdogTimerLib.h>
#include <Guid/MemoryAllocationHob.h>
#include "Bd.h"
#include "CrashDump.h"

#define EFI_DUMP_BUFFER_SIZE  (SIZE_32KB)

EFI_DUMP_BUFFER     BdDumpBuffer = {0};
MemoryDumpType      BdDumpType   = MemoryDumpDisabled;

/**
  Called to initialize the crash dump agent.

  @param[in] HobList     Pointer to the HOB list.

**/
VOID
EFIAPI
InitializeCrashDumpAgent(
    IN VOID               *HobList
    )
{
    EFI_PEI_HOB_POINTERS hob;
    UINT8* dumpBuffer;

    //
    // find and save the initial stack info from the HOB list
    // The stack base and limit are stored in the NtTib in the PCR
    // N.B.
    //   The NtTib is overlaid with other fields.  This is fine for
    //   now since the other fields are not used in UEFI.
    //
    hob.Raw = HobList;
    while ((hob.Raw = GetNextHob(EFI_HOB_TYPE_MEMORY_ALLOCATION, hob.Raw)) != NULL)
    {
        if (CompareGuid(&gEfiHobMemoryAllocStackGuid, &(hob.MemoryAllocationStack->AllocDescriptor.Name)))
        {
            BdPcr.NtTib.StackBase  = (PVOID)(UINTN)hob.MemoryAllocationStack->AllocDescriptor.MemoryBaseAddress;
            BdPcr.NtTib.StackLimit = (PVOID)(UINTN)hob.MemoryAllocationStack->AllocDescriptor.MemoryLength;
            // Stack size must be under 4GB to avoid truncation when it is converted
            // to UINT32 as required by the DMP format.
            ASSERT((UINT64)BdPcr.NtTib.StackLimit < SIZE_4GB);
            break;
        }

        hob.Raw = GET_NEXT_HOB(hob);
    }

    // allocate buffer
    dumpBuffer = AllocatePool(EFI_DUMP_BUFFER_SIZE);

    if (dumpBuffer == NULL)
    {
        // This initializes early enough that we should always be able
        // to allocate.
        ASSERT(FALSE);
        return;
    }

    EfiDumpBufferInitialize(&BdDumpBuffer, dumpBuffer, EFI_DUMP_BUFFER_SIZE);

    //
    // This could be configured via the BIOS config hob
    //
    BdDumpType = MemoryDumpTriage;
}


/**
  Called when a fatal error is detected and the system cannot continue.

  @param  BugCheckCode  Reason for the fatal error.
  @param  Param1        Bugcheck code specific parameter.
  @param  Param2        Bugcheck code specific parameter.
  @param  Param3        Bugcheck code specific parameter.
  @param  Param4        Bugcheck code specific parameter.

**/
VOID
EFIAPI
EfiBugCheck(
    IN    UINT32                BugCheckCode,
    IN    UINTN                 Param1,
    IN    UINTN                 Param2,
    IN    UINTN                 Param3,
    IN    UINTN                 Param4
    )
{
    PCONTEXT            context   = &BdPrcb->ProcessorState.ContextFrame;
    PKPROCESSOR_STATE   procState = &BdPrcb->ProcessorState;

    EfiCaptureContext(context);
    KiSaveProcessorControlState(procState);

    EfiBugCheckWithContext(context, NULL, BugCheckCode, Param1, Param2, Param3, Param4);
}


/**
  Common bugcheck implemention for use when the context and exception record are known.

  @param  Context           Saved context.
  @param  ExceptionRecord   Optional exception record
  @param  BugCheckCode      Reason for the fatal error.
  @param  Param1            Bugcheck code specific parameter.
  @param  Param2            Bugcheck code specific parameter.
  @param  Param3            Bugcheck code specific parameter.
  @param  Param4            Bugcheck code specific parameter.

**/
VOID
EfiBugCheckWithContext(
    _In_        CONTEXT              *Context,
    _In_opt_    EXCEPTION_RECORD     *ExceptionRecord,
    _In_        UINT32                BugCheckCode,
    _In_        UINTN                 Param1,
    _In_        UINTN                 Param2,
    _In_        UINTN                 Param3,
    _In_        UINTN                 Param4
    )
{
    EFI_STATUS status;

    //
    // Disable the watchdog timer.
    //
    WatchdogConfigure(0, WatchdogDisabled);

    DEBUG((EFI_D_ERROR, "\nBugcheck\n"));
    DEBUG((EFI_D_ERROR, "    Bugcheck Code: 0x%x, %p, %p, %p, %p\n",
        BugCheckCode, Param1, Param2, Param3, Param4));

    DEBUG((EFI_D_ERROR, "    Context: %p, Exception Record %p\n",
        Context, ExceptionRecord));

    status = EfiBuildCrashDump(Context,
                ExceptionRecord,
                BugCheckCode,
                Param1, Param2, Param3, Param4);

    // give the debugger one last chance to take a look.
    BdBreakPointWithStatus(0);

#if defined(MDE_CPU_IA32)

    if (EFI_ERROR(status))
    {
        TripleFault(GUESTDUMP_TRIPLEFAULT_SIGNATURE_HIGH_DWORD,
                    0,
                    0,
                    GUESTDUMP_TRIPLEFAULT_SIGNATURE_LOW_DWORD);
    }
    else
    {
        TripleFault(GUESTDUMP_TRIPLEFAULT_SIGNATURE_HIGH_DWORD,
                    (UINT64)BdDumpBuffer.Buffer,
                    BdDumpBuffer.Offset,
                    GUESTDUMP_TRIPLEFAULT_SIGNATURE_LOW_DWORD);
    }

#elif defined(MDE_CPU_X64)

    if (EFI_ERROR(status))
    {
        TripleFault(GUESTDUMP_TRIPLEFAULT_SIGNATURE,
                    0,
                    0,
                    0);
    }
    else
    {
        TripleFault(GUESTDUMP_TRIPLEFAULT_SIGNATURE,
                    (UINT64)BdDumpBuffer.Buffer,
                    BdDumpBuffer.Offset,
                    0);
    }

#else
#error "Unsupported architecture"
#endif


}


/**
  Determines the simple struture size for the given
  block type.  This does not include any additional data
  or alignment padding.

  @param  Type    Type of block

  @return Size of the block in bytes

**/
UINT32
EfiDumpBufferBlockSize(
    _In_    MemoryBlockType     Type
    )
{
    switch (Type)
    {
    case BlockTypeInfo:
        return sizeof(MEMORY_DUMP_INFO);

    case BlockTypeContext:
    case BlockTypeException:
    case BlockTypeListEnd:
        return sizeof(MEMORY_DUMP_BLOCK_HEADER);

    case BlockTypeMemoryRegion:
        return sizeof(MEMORY_DUMP_REGION);

    case BlockTypeModuleList:
        return sizeof(MEMORY_DUMP_MODULE_LIST);

    default:
        ASSERT(FALSE);
        return 0;
    }
}


/**
  Initializes a dump buffer with the given
  backing buffer and size.

  @param    DumpBuffer  EFI_DUMP_BUFFER to initialize
  @param    Buffer      Backing store buffer to use
  @param    BufferSize  Size of backing store.

**/
VOID
EfiDumpBufferInitialize(
    _Inout_     EFI_DUMP_BUFFER*    DumpBuffer,
    _In_bytecount_(BufferSize)
                UINT8*              Buffer,
    _In_        UINT32              BufferSize
    )
{
    DumpBuffer->Buffer = Buffer;
    DumpBuffer->BufferSize = BufferSize;
    DumpBuffer->Offset = 0;
    DumpBuffer->BlockCount = 0;

}


/**
  Allocates space for the given block type and initializes the block header

  @param    DumpBuffer  EFI_DUMP_BUFFER to use
  @param    Type        Block type to allocate
  @param    BlockStart  On successfull return contains a pointer to the start
                        of the allocated block.

  @return  TRUE on success, FALSE on failure (not enough space)

**/
BOOLEAN
EfiDumpBufferAllocateBlock(
    _Inout_     EFI_DUMP_BUFFER*    DumpBuffer,
    _In_        MemoryBlockType     Type,
    _Outptr_    VOID**              BlockStart
    )
{
    return EfiDumpBufferAllocateBlockEx(DumpBuffer, Type, 0, BlockStart);
}


/**
  Allocates space for the given block type plus additional requested space.
  The block header is initialized as well.

  @param    DumpBuffer      EFI_DUMP_BUFFER to use
  @param    Type            Block type to allocate
  @param    AdditionalSize  Number of additional bytes to allocate along with the
                            standard block structure.
  @param    BlockStart      On successfull return contains a pointer to the start
                            of the allocated block.

  @return  TRUE on success, FALSE on failure (not enough space)

**/
BOOLEAN
EfiDumpBufferAllocateBlockEx(
    _Inout_     EFI_DUMP_BUFFER*    DumpBuffer,
    _In_        MemoryBlockType     Type,
    _In_        UINT32              AdditionalSize,
    _Outptr_    VOID**              BlockStart
    )
{
    MEMORY_DUMP_BLOCK_HEADER *pHeader;
    UINT32 blockSize = EfiDumpBufferBlockSize(Type) + AdditionalSize;
    UINT32 newOffset = MEMORY_DUMP_ALIGN_BLOCK(DumpBuffer->Offset + blockSize);

    *BlockStart = NULL;

    if ((newOffset < DumpBuffer->Offset) ||     // addition overflow
        (newOffset > DumpBuffer->BufferSize))   // dump buffer overflow
    {
        return FALSE;
    }

    pHeader = (MEMORY_DUMP_BLOCK_HEADER*)(DumpBuffer->Buffer + DumpBuffer->Offset);
    DumpBuffer->Offset = newOffset;
    DumpBuffer->BlockCount++;

    // attempt catch overwrites from the previous block.
    ASSERT((*(UINT64*)pHeader) == 0ll);
    pHeader->Type = Type;
    pHeader->Size = blockSize;
    *BlockStart = pHeader;

    return TRUE;
}
