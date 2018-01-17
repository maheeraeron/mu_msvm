/*++

Copyright (c) Microsoft Corporation

Module Name:

    MemoryDumpRequest.h

Abstract:

    Structure and type definitions for guest requested memory dumps.

Author:

    Kris Harper (kharp) 12-Sep-2013

--*/
#pragma once

//
// Value present in RAX upon triple fault
// signals that this is a guest requested memory dump
// 32 bit modes will split the signature across eax & edx
// with the high DWORD in eax and the low DWORD in edx.
//
#define GUESTDUMP_TRIPLEFAULT_SIGNATURE 0x504d445453455547  // "GUESTDMP"
#define GUESTDUMP_TRIPLEFAULT_SIGNATURE_LOW_DWORD 0x53455547    // put in edx
#define GUESTDUMP_TRIPLEFAULT_SIGNATURE_HIGH_DWORD 0x504d4454   // put in eax


/**
 * Type of dump to generate
 */
typedef enum
{
    MemoryDumpDisabled,
    MemoryDumpTriage    // a.k.a. minidump
}MemoryDumpType;


typedef enum
{
    // General dump request information
    // Structure - MEMORY_DUMP_INFO
    BlockTypeInfo,

    // Processor Context
    // Structure - MEMORY_DUMP_BLOCK_HEADER directly followed by
    // context data in the form of the NT kernel CONTEXT structure
    BlockTypeContext,

    // Exception Record
    // Structure - MEMORY_DUMP_BLOCK_HEADER directly followed by
    // an EXCEPTION_RECORD structure.
    BlockTypeException,

    // Generic memory region
    // Structure - MEMORY_DUMP_REGION
    // Subtype may indicate a specific type of region.
    BlockTypeMemoryRegion,

    // Loaded module list
    // Structure - MEMORY_DUMP_MODULE_LIST
    //
    BlockTypeModuleList,
    BlockTypeListEnd = 0xffff
}MemoryBlockType;


//
// All blocks are 8 byte aligned.
// MEMORY_DUMP_ALIGN_BLOCK can be used when creating or traversing
// a packed list of memory blocks to ensure correct alignment
// and traversal from one block to the next.
//
#define MEMORY_DUMP_BLOCK_ALIGNMENT 8
#define MEMORY_DUMP_ALIGN_BLOCK(Block)  \
    (((Block) + MEMORY_DUMP_BLOCK_ALIGNMENT - 1) & (~(MEMORY_DUMP_BLOCK_ALIGNMENT - 1)))


/**
 * Common header for each dump block
 */
typedef struct
{
    MemoryBlockType             Type;
    UINT16                      SubType;
    //
    // Total size of the block including header
    // The size *does not* include any alignment padding
    //
    UINT32                      Size;
}MEMORY_DUMP_BLOCK_HEADER;


/**
 * Basic information for a dump request
 * Header.Type = BlockTypeInfo
 */
typedef struct
{
    MEMORY_DUMP_BLOCK_HEADER    Header;

    // total number of blocks in the request
    UINT32                      BlockCount;

    UINT32                      MajorVersion;
    UINT32                      MinorVersion;
    UINT32                      ProcessorCount;

    UINT32                      BugCheckCode;
    UINT64                      BugCheckParams[4];
    MemoryDumpType              DumpType;

}MEMORY_DUMP_INFO;


/**
 *  Describes a region of memory to be saved in a memory dump
 *  Header subtype can describe a specific region.
 *  The address is a guest physical address.
 */
typedef struct
{
    MEMORY_DUMP_BLOCK_HEADER    Header;
    UINT64                      Address;
    UINT32                      Size;
}MEMORY_DUMP_REGION;

#define MEMORY_REGION_TYPE_GENERAL       0x00000000
#define MEMORY_REGION_TYPE_PRCB          0x00000001
#define MEMORY_REGION_TYPE_DEBUGDATA     0x00000002
#define MEMORY_REGION_TYPE_STACK         0x00000003


/**
 * Defines a list of loaded modules and names
 * Header.Type = BlockTypeModuleList
 * The module list is represented by two concatenated regions
 *
 * The first region is an array made up of DUMP_DRIVER_ENTRY64 structures.
 * ModuleCount describes the size of the array. Each DUMP_DRIVER_ENTRY64
 * entry contains an offset in to the string table.  The offset is from
 * the start of the MEMORY_DUMP_MODULE_LIST block.
 *
 * The second region is a string table made of up variable length
 * string entries represented by DUMP_STRING structures.  The start
 * of this region can be computed as block + (ModuleCount * ModuleBlobSize)
 *
 * Each DUMP_STRING is 8-byte aligned, although the offset should already
 * reflect this and entities reading a module list should not be concerned
 * about this detail.
 *
 */
typedef struct
{
    MEMORY_DUMP_BLOCK_HEADER    Header;
    UINT64                      LoadedModuleListPtr;
    UINT32                      ModuleCount;

    //
    // Module array follows directly
    // String array follows the module array
    //
}MEMORY_DUMP_MODULE_LIST;
