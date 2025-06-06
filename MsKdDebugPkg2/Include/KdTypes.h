/**@file KdTypes.h

This file contains definitions related to the kernel debugger on Windows.

Copyright (c) 2018, Microsoft Corporation

All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#ifndef __KDTYPES_H__
#define __KDTYPES_H__

#if defined (_MSC_EXTENSIONS)
  #pragma warning(disable: 4200 4201 4214 4324)
#endif

#include <Library/UefiLib.h>

//
// These status codes are defined here and are consistent
// with the definitions in the Windows' code base.
//
#define STATUS_SUCCESSFUL         0x0L
#define STATUS_UNSUCCESSFUL       0xC0000001L
#define STATUS_INVALID_PARAMETER  0xC000000DL

//
// The PAGE_ALIGN macro takes a virtual address and returns a page-aligned
// virtual address for that page.
//
#define PAGE_ALIGN(va)  (UINT8 *)((UINT64)(va) & ~(EFI_PAGE_SIZE - 1))

//
// The BYTE_OFFSET macro takes a virtual address and returns the byte offset
// of that address within the page.
//
#define BYTE_OFFSET(Va)  ((UINT32)((UINT64)(Va) & (EFI_PAGE_SIZE - 1)))

typedef struct _KD_STRING {
  UINT16    Length;
  UINT16    MaximumLength;
  UINT8     *Buffer;
} KD_STRING;

//
// Define packet waiting status codes.
//

#define KD_PACKET_RECEIVED  0
#define KD_PACKET_TIMEOUT   1
#define KD_PACKET_RESEND    2

//
// KD_PACKETS are the low level data format used in KD. All packets
// begin with a packet leader, byte count, packet type. The sequence
// for accepting a packet is:
//
//  - read 4 bytes to get packet leader.  If read times out (10 seconds)
//    with a short read, or if packet leader is incorrect, then retry
//    the read.
//
//  - next read 2 byte packet type.  If read times out (10 seconds) with
//    a short read, or if packet type is bad, then start again looking
//    for a packet leader.
//
//  - next read 4 byte packet Id.  If read times out (10 seconds)
//    with a short read, or if packet Id is not what we expect, then
//    ask for resend and restart again looking for a packet leader.
//
//  - next read 2 byte count.  If read times out (10 seconds) with
//    a short read, or if byte count is greater than PACKET_MAX_SIZE,
//    then start again looking for a packet leader.
//
//  - next read 4 byte packet data checksum.
//
//  - The packet data immediately follows the packet.  There should be
//    ByteCount bytes following the packet header.  Read the packet
//    data, if read times out (10 seconds) then start again looking for
//    a packet leader.
//

typedef struct _KD_PACKET {
  UINT32    PacketLeader;
  UINT16    PacketType;
  UINT16    ByteCount;
  UINT32    PacketId;
  UINT32    Checksum;
} KD_PACKET;

#define PACKET_MAX_SIZE  4000           // Windows KD support requires a max
                                        // packet size of 4000 for legacy
                                        // transports.  This value will change
                                        // for other transport types.   See
                                        // minkernel\debuggers\published\ntdbg.w
                                        // for more information.

#define INITIAL_PACKET_ID  0x80800000   // Don't use 0
#define SYNC_PACKET_ID     0x00000800   // Or in with INITIAL_PACKET_ID
                                        // to force a packet ID reset.

//
// BreakIn packet
//

#define BREAKIN_PACKET       0x62626262
#define BREAKIN_PACKET_BYTE  0x62

//
// Packet lead in sequence
//

#define PACKET_LEADER       0x30303030
#define PACKET_LEADER_BYTE  0x30

#define CONTROL_PACKET_LEADER       0x69696969
#define CONTROL_PACKET_LEADER_BYTE  0x69

//
// Packet Trailing Byte
//

#define PACKET_TRAILING_BYTE  0xAA

//
// Packet Types
//

typedef enum _KD_PACKET_TYPE {
  PACKET_TYPE_UNUSED              = 0,
  PACKET_TYPE_KD_STATE_CHANGE32   = 1,
  PACKET_TYPE_KD_STATE_MANIPULATE = 2,
  PACKET_TYPE_KD_DEBUG_IO         = 3,
  PACKET_TYPE_KD_ACKNOWLEDGE      = 4,
  PACKET_TYPE_KD_RESEND           = 5,
  PACKET_TYPE_KD_RESET            = 6,
  PACKET_TYPE_KD_STATE_CHANGE64   = 7,
  PACKET_TYPE_KD_POLL_BREAKIN     = 8,
  PACKET_TYPE_KD_TRACE_IO         = 9,
  PACKET_TYPE_KD_CONTROL_REQUEST  = 10,
  PACKET_TYPE_KD_FILE_IO          = 11,
  PACKET_TYPE_MAX                 = 12,
} KD_PACKET_TYPE;

//
// If the packet type is PACKET_TYPE_KD_STATE_CHANGE64, then
// the format of the packet data is as follows:
//

#define DbgKdMinimumStateChange  0x00003030L

#define DbgKdExceptionStateChange      0x00003030L
#define DbgKdLoadSymbolsStateChange    0x00003031L
#define DbgKdCommandStringStateChange  0x00003032L

#define DbgKdMaximumStateChange  0x00003033L

#define KD_REBOOT     (-1)
#define KD_HIBERNATE  (-2)

//
// Pathname Data follows directly
//

typedef struct _DBGKD_LOAD_SYMBOLS32 {
  UINT32     PathNameLength;
  UINT32     BaseOfDll;
  UINT32     ProcessId;
  UINT32     CheckSum;
  UINT32     SizeOfImage;
  BOOLEAN    UnloadSymbols;
} DBGKD_LOAD_SYMBOLS32;

typedef struct _DBGKD_LOAD_SYMBOLS64 {
  UINT32     PathNameLength;
  UINT64     BaseOfDll;
  UINT64     ProcessId;
  UINT32     CheckSum;
  UINT32     SizeOfImage;
  BOOLEAN    UnloadSymbols;
} DBGKD_LOAD_SYMBOLS64;

#define KD_SECONDARY_VERSION_DEFAULT        0
#define KD_SECONDARY_VERSION_AMD64_CONTEXT  2

#if defined (MDE_CPU_X64)
#define CURRENT_KD_SECONDARY_VERSION  KD_SECONDARY_VERSION_AMD64_CONTEXT
#else
#define CURRENT_KD_SECONDARY_VERSION  KD_SECONDARY_VERSION_DEFAULT
#endif

typedef struct _DBGKD_GET_VERSION64 {
  UINT16    MajorVersion;
  UINT16    MinorVersion;
  UINT8     ProtocolVersion;
  UINT8     KdSecondaryVersion;
  UINT16    Flags;
  UINT16    MachineType;

  //
  // Protocol command support descriptions.
  // These allow the debugger to automatically
  // adapt to different levels of command support
  // in different kernels.
  //

  // One beyond highest packet type understood, zero based.
  UINT8     MaxPacketType;
  // One beyond highest state change understood, zero based.
  UINT8     MaxStateChange;
  // One beyond highest state manipulate message understood, zero based.
  UINT8     MaxManipulate;

  // Kind of execution environment the kernel is running in,
  // such as a real machine or a simulator.  Written back
  // by the simulation if one exists.
  UINT8     Simulation;

  UINT16    Unused[1];

  UINT64    KernBase;
  UINT64    PsLoadedModuleList;

  //
  // Components may register a debug data block for use by
  // debugger extensions.  This is the address of the list head.
  //
  // There will always be an entry for the debugger.
  //

  UINT64    DebuggerDataList;
} DBGKD_GET_VERSION64;

// typedef LIST_ENTRY LIST_ENTRY64;

typedef struct _LIST_ENTRY64 {
  UINT64    ForwardLink;
  UINT64    BackLink;
} LIST_ENTRY64;

//
// This structure is used by the debugger for all targets
// It is the same size as DBGKD_DATA_HEADER on all systems
//
typedef struct _DBGKD_DEBUG_DATA_HEADER64 {
  //
  // Link to other blocks
  //

  LIST_ENTRY64    List;

  //
  // This is a unique tag to identify the owner of the block.
  // If your component only uses one pool tag, use it for this, too.
  //

  UINT32          OwnerTag;

  //
  // This must be initialized to the size of the data block,
  // including this structure.
  //

  UINT32          Size;
} DBGKD_DEBUG_DATA_HEADER64;

//
// This structure is the same size on all systems.  The only field
// which must be translated by the debugger is Header.List.
//

//
// DO NOT ADD OR REMOVE FIELDS FROM THE MIDDLE OF THIS STRUCTURE!!!
//
// If you remove a field, replace it with an "unused" placeholder.
// Do not reuse fields until there has been enough time for old debuggers
// and extensions to age out.
//
typedef struct _KDDEBUGGER_DATA64 {
  DBGKD_DEBUG_DATA_HEADER64    Header;

  //
  // Base address of kernel image
  //

  UINT64                       KernBase;

  //
  // DbgBreakPointWithStatus is a function which takes an argument
  // and hits a breakpoint.  This field contains the address of the
  // breakpoint instruction.  When the debugger sees a breakpoint
  // at this address, it may retrieve the argument from the first
  // argument register, or on x86 the eax register.
  //

  UINT64    BreakpointWithStatus;        // address of breakpoint

  //
  // Address of the saved context record during a bugcheck
  //
  // N.B. This is an automatic in KeBugcheckEx's frame, and
  // is only valid after a bugcheck.
  //

  UINT64    SavedContext;

  //
  // help for walking stacks with user callbacks:
  //

  //
  // The address of the thread structure is provided in the
  // WAIT_STATE_CHANGE packet.  This is the offset from the base of
  // the thread structure to the pointer to the kernel stack frame
  // for the currently active usermode callback.
  //

  UINT16    ThCallbackStack;            // offset in thread data

  //
  // these values are offsets into that frame:
  //

  UINT16    NextCallback;               // saved pointer to next callback frame
  UINT16    FramePointer;               // saved frame pointer

  //
  // pad to a quad boundary
  //
  UINT16    PaeEnabled : 1;

  //
  // Address of the kernel callout routine.
  //

  UINT64    KiCallUserMode;              // kernel routine

  //
  // Address of the usermode entry point for callbacks.
  //

  UINT64    KeUserCallbackDispatcher;    // address in ntdll

  //
  // Addresses of various kernel data structures and lists
  // that are of interest to the kernel debugger.
  //

  UINT64    PsLoadedModuleList;
  UINT64    PsActiveProcessHead;
  UINT64    PspCidTable;

  UINT64    ExpSystemResourcesList;
  UINT64    ExpPagedPoolDescriptor;
  UINT64    ExpNumberOfPagedPools;

  UINT64    KeTimeIncrement;
  UINT64    KeSystemErrorCallbackListHead;
  UINT64    KiSystemErrorData;

  UINT64    IopErrorLogListHead;

  UINT64    ObpRootDirectoryObject;
  UINT64    ObpTypeObjectType;

  UINT64    MmSystemCacheStart;
  UINT64    MmSystemCacheEnd;
  UINT64    MmSystemCacheWs;

  UINT64    MmPfnDatabase;
  UINT64    MmSystemPtesStart;
  UINT64    MmSystemPtesEnd;
  UINT64    MmSubsectionBase;
  UINT64    MmNumberOfPagingFiles;

  UINT64    MmLowestPhysicalPage;
  UINT64    MmHighestPhysicalPage;
  UINT64    MmNumberOfPhysicalPages;

  UINT64    MmMaximumNonPagedPoolInBytes;
  UINT64    MmNonPagedSystemStart;
  UINT64    MmNonPagedPoolStart;
  UINT64    MmNonPagedPoolEnd;

  UINT64    MmPagedPoolStart;
  UINT64    MmPagedPoolEnd;
  UINT64    MmPagedPoolInformation;
  UINT64    MmPageSize;

  UINT64    MmSizeOfPagedPoolInBytes;

  UINT64    MmTotalCommitLimit;
  UINT64    MmTotalCommittedPages;
  UINT64    MmSharedCommit;
  UINT64    MmDriverCommit;
  UINT64    MmProcessCommit;
  UINT64    MmPagedPoolCommit;
  UINT64    MmExtendedCommit;

  UINT64    MmZeroedPageListHead;
  UINT64    MmFreePageListHead;
  UINT64    MmStandbyPageListHead;
  UINT64    MmModifiedPageListHead;
  UINT64    MmModifiedNoWritePageListHead;
  UINT64    MmAvailablePages;
  UINT64    MmResidentAvailablePages;

  UINT64    PoolTrackTable;
  UINT64    NonPagedPoolDescriptor;

  UINT64    MmHighestUserAddress;
  UINT64    MmSystemRangeStart;
  UINT64    MmUserProbeAddress;

  UINT64    KdPrintCircularBuffer;
  UINT64    KdPrintCircularBufferEnd;
  UINT64    KdPrintWritePointer;
  UINT64    KdPrintRolloverCount;

  UINT64    MmLoadedUserImageList;

  // NT 5.1 Addition

  UINT64    NtBuildLab;
  UINT64    KiNormalSystemCall;

  // NT 5.0 QFE addition

  UINT64    KiProcessorBlock;
  UINT64    MmUnloadedDrivers;
  UINT64    MmLastUnloadedDriver;
  UINT64    MmTriageActionTaken;
  UINT64    MmSpecialPoolTag;
  UINT64    KernelVerifier;
  UINT64    MmVerifierData;
  UINT64    MmAllocatedNonPagedPool;
  UINT64    MmPeakCommitment;
  UINT64    MmTotalCommitLimitMaximum;
  UINT64    CmNtCSDVersion;

  // NT 5.1 Addition

  UINT64    MmPhysicalMemoryBlock;
  UINT64    MmSessionBase;
  UINT64    MmSessionSize;
  UINT64    MmSystemParentTablePage;

  // Server 2003 addition

  UINT64    MmVirtualTranslationBase;

  UINT16    OffsetKThreadNextProcessor;
  UINT16    OffsetKThreadTeb;
  UINT16    OffsetKThreadKernelStack;
  UINT16    OffsetKThreadInitialStack;

  UINT16    OffsetKThreadApcProcess;
  UINT16    OffsetKThreadState;
  UINT16    OffsetKThreadBStore;
  UINT16    OffsetKThreadBStoreLimit;

  UINT16    SizeEProcess;
  UINT16    OffsetEprocessPeb;
  UINT16    OffsetEprocessParentCID;
  UINT16    OffsetEprocessDirectoryTableBase;

  UINT16    SizePrcb;
  UINT16    OffsetPrcbDpcRoutine;
  UINT16    OffsetPrcbCurrentThread;
  UINT16    OffsetPrcbMhz;

  UINT16    OffsetPrcbCpuType;
  UINT16    OffsetPrcbVendorString;
  UINT16    OffsetPrcbProcStateContext;
  UINT16    OffsetPrcbNumber;

  UINT16    SizeEThread;

  UINT64    KdPrintCircularBufferPtr;
  UINT64    KdPrintBufferSize;

  UINT64    KeLoaderBlock;

  UINT16    SizePcr;
  UINT16    OffsetPcrSelfPcr;
  UINT16    OffsetPcrCurrentPrcb;
  UINT16    OffsetPcrContainedPrcb;

  UINT16    OffsetPcrInitialBStore;
  UINT16    OffsetPcrBStoreLimit;
  UINT16    OffsetPcrInitialStack;
  UINT16    OffsetPcrStackLimit;

  UINT16    OffsetPrcbPcrPage;
  UINT16    OffsetPrcbProcStateSpecialReg;
  UINT16    GdtR0Code;
  UINT16    GdtR0Data;

  UINT16    GdtR0Pcr;
  UINT16    GdtR3Code;
  UINT16    GdtR3Data;
  UINT16    GdtR3Teb;

  UINT16    GdtLdt;
  UINT16    GdtTss;
  UINT16    Gdt64R3CmCode;
  UINT16    Gdt64R3CmTeb;

  UINT64    IopNumTriageDumpDataBlocks;
  UINT64    IopTriageDumpDataBlocks;

  // Longhorn addition

  UINT64    VfCrashDataBlock;
  UINT64    MmBadPagesDetected;
  UINT64    MmZeroedPageSingleBitErrorsDetected;

  // Windows 7 addition

  UINT64    EtwpDebuggerData;
  UINT16    OffsetPrcbContext;

  // Windows 8 addition

  UINT16    OffsetPrcbMaxBreakpoints;
  UINT16    OffsetPrcbMaxWatchpoints;

  UINT32    OffsetKThreadStackLimit;
  UINT32    OffsetKThreadStackBase;
  UINT32    OffsetKThreadQueueListEntry;
  UINT32    OffsetEThreadIrpList;

  UINT16    OffsetPrcbIdleThread;
  UINT16    OffsetPrcbNormalDpcState;
  UINT16    OffsetPrcbDpcStack;
  UINT16    OffsetPrcbIsrStack;

  UINT16    SizeKDPC_STACK_FRAME;

  // Windows 8.1 Addition

  UINT16    OffsetKPriQueueThreadListHead;
  UINT16    OffsetKThreadWaitReason;

  // Windows 10 RS1 Addition

  UINT16    Padding;
  UINT64    PteBase;
} KDDEBUGGER_DATA64;

//
// If DBGKD_VERS_FLAG_DATA is set in Flags, info should be retrieved from
// the KDDEBUGGER_DATA block rather than from the DBGKD_GET_VERSION
// packet.  The data will remain in the version packet for a while to
// reduce compatibility problems.
//

#define DBGKD_VERS_FLAG_MP          0x0001  // kernel is MP built
#define DBGKD_VERS_FLAG_DATA        0x0002  // DebuggerDataList is valid
#define DBGKD_VERS_FLAG_PTR64       0x0004  // native pointers are 64 bits
#define DBGKD_VERS_FLAG_NOMM        0x0008  // No MM - don't decode PTEs
#define DBGKD_VERS_FLAG_HSS         0x0010  // hardware stepping support
#define DBGKD_VERS_FLAG_PARTITIONS  0x0020  // multiple OS partitions exist

typedef struct _DBGKD_READ_WRITE_IO_EXTENDED64 {
  UINT32    DataSize;                    // 1, 2, 4
  UINT32    InterfaceType;
  UINT32    BusNumber;
  UINT32    AddressSpace;
  UINT64    IoAddress;
  UINT32    DataValue;
} DBGKD_READ_WRITE_IO_EXTENDED64;

//
// This structure is currently all zeroes.
// It just reserves a structure name for future use.
//

typedef struct _DBGKD_COMMAND_STRING {
  UINT32    Flags;
  UINT32    Reserved1;
  UINT64    Reserved2[7];
} DBGKD_COMMAND_STRING;

#define KD_EXCEPTION_NONCONTINUABLE  0x1

//
// If the packet type is PACKET_TYPE_KD_STATE_MANIPULATE, then
// the format of the packet data is as follows:
//
// Api Numbers for state manipulation
//

#define DbgKdMinimumManipulate  0x00003130L

#define DbgKdReadVirtualMemoryApi            0x00003130L
#define DbgKdWriteVirtualMemoryApi           0x00003131L
#define DbgKdGetContextApi                   0x00003132L
#define DbgKdSetContextApi                   0x00003133L
#define DbgKdWriteBreakPointApi              0x00003134L
#define DbgKdRestoreBreakPointApi            0x00003135L
#define DbgKdContinueApi                     0x00003136L
#define DbgKdReadControlSpaceApi             0x00003137L
#define DbgKdWriteControlSpaceApi            0x00003138L
#define DbgKdReadIoSpaceApi                  0x00003139L
#define DbgKdWriteIoSpaceApi                 0x0000313AL
#define DbgKdRebootApi                       0x0000313BL
#define DbgKdContinueApi2                    0x0000313CL
#define DbgKdReadPhysicalMemoryApi           0x0000313DL
#define DbgKdWritePhysicalMemoryApi          0x0000313EL
#define DbgKdSetSpecialCallApi               0x00003140L
#define DbgKdClearSpecialCallsApi            0x00003141L
#define DbgKdSetInternalBreakPointApi        0x00003142L
#define DbgKdGetInternalBreakPointApi        0x00003143L
#define DbgKdReadIoSpaceExtendedApi          0x00003144L
#define DbgKdWriteIoSpaceExtendedApi         0x00003145L
#define DbgKdGetVersionApi                   0x00003146L
#define DbgKdWriteBreakPointExApi            0x00003147L
#define DbgKdRestoreBreakPointExApi          0x00003148L
#define DbgKdCauseBugCheckApi                0x00003149L
#define DbgKdSwitchProcessor                 0x00003150L
#define DbgKdPageInApi                       0x00003151L
#define DbgKdReadMachineSpecificRegister     0x00003152L
#define DbgKdWriteMachineSpecificRegister    0x00003153L
#define OldVlm1                              0x00003154L
#define OldVlm2                              0x00003155L
#define DbgKdSearchMemoryApi                 0x00003156L
#define DbgKdGetBusDataApi                   0x00003157L
#define DbgKdSetBusDataApi                   0x00003158L
#define DbgKdCheckLowMemoryApi               0x00003159L
#define DbgKdClearAllInternalBreakpointsApi  0x0000315AL
#define DbgKdFillMemoryApi                   0x0000315BL
#define DbgKdQueryMemoryApi                  0x0000315CL
#define DbgKdSwitchPartition                 0x0000315DL
#define DbgKdMidoriApi                       0x0000315EL
#define DbgKdGetContextExApi                 0x0000315FL
#define DbgKdSetContextExApi                 0x00003160L

#define DbgKdMaximumManipulate  0x00003161L

//
// Physical memory caching flags.
// These flags can be passed in on physical memory
// access requests in the ActualBytes field.
//

#define DBGKD_CACHING_UNKNOWN         0
#define DBGKD_CACHING_CACHED          1
#define DBGKD_CACHING_UNCACHED        2
#define DBGKD_CACHING_WRITE_COMBINED  3

#define DBGKD_MAXSTREAM  16

//
// DO NOT MODIFY THESE STRUCTURES.
//
// These sizes and offsets of these structures are hard-coded into KD, hence
// they should be considered part of the "wire protocol" for the debugger.
//

// BUGBUG: Verify the packing characteristics of there:
typedef struct _AMD64_DBGKD_CONTROL_REPORT {
  UINT64    Dr6;
  UINT64    Dr7;
  UINT32    EFlags;
  UINT16    InstructionCount;
  UINT16    ReportFlags;
  UINT8     InstructionStream[DBGKD_MAXSTREAM];
  UINT16    SegCs;
  UINT16    SegDs;
  UINT16    SegEs;
  UINT16    SegFs;
} AMD64_DBGKD_CONTROL_REPORT;

#define AMD64_REPORT_INCLUDES_SEGS  0x0001
#define AMD64_REPORT_STANDARD_CS    0x0002

typedef struct _X86_DBGKD_CONTROL_REPORT {
  UINT32    Dr6;
  UINT32    Dr7;
  UINT16    InstructionCount;
  UINT16    ReportFlags;
  UINT8     InstructionStream[DBGKD_MAXSTREAM];
  UINT16    SegCs;
  UINT16    SegDs;
  UINT16    SegEs;
  UINT16    SegFs;
  UINT32    EFlags;
} X86_DBGKD_CONTROL_REPORT;

#define X86_REPORT_INCLUDES_SEGS  0x0001
#define X86_REPORT_STANDARD_CS    0x0002

typedef struct _ARM64_DBGKD_CONTROL_REPORT {
  UINT32    Cpsr;
  UINT32    InstructionCount;
  UINT8     InstructionStream[DBGKD_MAXSTREAM];
} ARM64_DBGKD_CONTROL_REPORT;

typedef struct _DBGKD_ANY_CONTROL_REPORT {
  union {
    AMD64_DBGKD_CONTROL_REPORT    Amd64ControlReport;
    X86_DBGKD_CONTROL_REPORT      X86ControlReport;
    ARM64_DBGKD_CONTROL_REPORT    Arm64ControlReport;
  };
} DBGKD_ANY_CONTROL_REPORT;

//
// The DBGKD_CONTROL_SET structure is packed with 4 byte packing.
//

#pragma pack(push, 4)

typedef struct _AMD64_DBGKD_CONTROL_SET {
  UINT32    TraceFlag;
  UINT64    Dr7;
  UINT64    CurrentSymbolStart;
  UINT64    CurrentSymbolEnd;
} AMD64_DBGKD_CONTROL_SET;

typedef struct _X86_DBGKD_CONTROL_SET {
  UINT32    TraceFlag;
  UINT32    Dr7;
  UINT32    CurrentSymbolStart;
  UINT32    CurrentSymbolEnd;
} X86_DBGKD_CONTROL_SET;

typedef struct _ARM64_DBGKD_CONTROL_SET {
  UINT32    Continue;
  UINT32    TraceFlag;
  UINT64    CurrentSymbolStart;
  UINT64    CurrentSymbolEnd;
} ARM64_DBGKD_CONTROL_SET;

typedef struct _DBGKD_ANY_CONTROL_SET {
  union {
    AMD64_DBGKD_CONTROL_SET    Amd64ControlSet;
    X86_DBGKD_CONTROL_SET      X86ControlSet;
    ARM64_DBGKD_CONTROL_SET    Arm64ControlSet;
  };
} DBGKD_ANY_CONTROL_SET;

#pragma pack(pop)

#if defined (MDE_CPU_X64)

typedef AMD64_DBGKD_CONTROL_REPORT  DBGKD_CONTROL_REPORT;
typedef AMD64_DBGKD_CONTROL_SET     DBGKD_CONTROL_SET;

#elif defined (MDE_CPU_IA32)

typedef X86_DBGKD_CONTROL_REPORT  DBGKD_CONTROL_REPORT;
typedef X86_DBGKD_CONTROL_SET     DBGKD_CONTROL_SET;

#elif defined (MDE_CPU_AARCH64)

typedef ARM64_DBGKD_CONTROL_REPORT  DBGKD_CONTROL_REPORT;
typedef ARM64_DBGKD_CONTROL_SET     DBGKD_CONTROL_SET;

#else

  #error "Unknown architecture"

#endif

#define EXCEPTION_MAXIMUM_PARAMETERS  15// maximum number of exception parameters

typedef struct _EXCEPTION_RECORD {
  UINT32    ExceptionCode;
  UINT32    ExceptionFlags;
  UINT64    ExceptionRecord;
  UINT64    ExceptionAddress;
  UINT32    NumberParameters;
  UINT32    __unusedAlignment;
  UINT64    ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD;

typedef struct _DBGKM_EXCEPTION {
  EXCEPTION_RECORD    ExceptionRecord;
  UINT32              FirstChance;
} DBGKM_EXCEPTION;

//
// Protocol version 6 state change.
//

typedef struct _DBGKD_ANY_WAIT_STATE_CHANGE {
  UINT32    NewState;
  UINT16    ProcessorLevel;
  UINT16    Processor;
  UINT32    NumberProcessors;
  UINT64    Thread;
  UINT64    ProgramCounter;
  union {
    DBGKM_EXCEPTION         Exception;
    DBGKD_LOAD_SYMBOLS64    LoadSymbols;
    DBGKD_COMMAND_STRING    CommandString;
  } u;
  // The ANY control report is unioned here to
  // ensure that this structure is always large
  // enough to hold any possible state change.
  union {
    DBGKD_CONTROL_REPORT        ControlReport;
    DBGKD_ANY_CONTROL_REPORT    AnyControlReport;
  };
} DBGKD_ANY_WAIT_STATE_CHANGE;

//
// Response is a read memory message with data following
//

typedef struct _DBGKD_READ_MEMORY64 {
  UINT64    TargetBaseAddress;
  UINT32    TransferCount;
  UINT32    ActualBytesRead;
} DBGKD_READ_MEMORY64;

//
// Data follows directly
//

typedef struct _DBGKD_WRITE_MEMORY64 {
  UINT64    TargetBaseAddress;
  UINT32    TransferCount;
  UINT32    ActualBytesWritten;
} DBGKD_WRITE_MEMORY64;

//
// Response is a get context message with a full context record following
//

typedef struct _DBGKD_GET_CONTEXT {
  UINT32    Unused;
} DBGKD_GET_CONTEXT;

//
// Full Context record follows
//

typedef struct _DBGKD_SET_CONTEXT {
  UINT32    ContextFlags;
} DBGKD_SET_CONTEXT;

typedef struct _DBGKD_CONTEXT_EX {
  UINT32    Offset;
  UINT32    ByteCount;
  UINT32    BytesCopied;
} DBGKD_CONTEXT_EX;

#define BREAKPOINT_TABLE_SIZE  32       // max number supported by kernel

typedef struct _DBGKD_WRITE_BREAKPOINT64 {
  UINT64    BreakPointAddress;
  UINT32    BreakPointHandle;
} DBGKD_WRITE_BREAKPOINT64;

typedef struct _DBGKD_RESTORE_BREAKPOINT {
  UINT32    BreakPointHandle;
} DBGKD_RESTORE_BREAKPOINT;

typedef struct _DBGKD_BREAKPOINTEX {
  UINT32    BreakPointCount;
  UINT32    ContinueStatus;
} DBGKD_BREAKPOINTEX;

typedef struct _DBGKD_CONTINUE {
  UINT32    ContinueStatus;
} DBGKD_CONTINUE;

//
// DBGKD_CONTINUE2 structure must be 32-bit packed for
// for compatibility with older, processor-specific
// versions of this structure.
//

#pragma pack(push,4)

typedef struct _DBGKD_CONTINUE2 {
  UINT32    ContinueStatus;
  union {
    DBGKD_CONTROL_SET    ControlSet;
    UINT8                PadToSize[28];
  };
} DBGKD_CONTINUE2;

#pragma pack(pop)

typedef struct _DBGKD_READ_WRITE_IO64 {
  UINT64    IoAddress;
  UINT32    DataSize;                    // 1, 2, 4
  UINT32    DataValue;
} DBGKD_READ_WRITE_IO64;

typedef struct _DBGKD_READ_WRITE_MSR {
  UINT32    Msr;
  UINT32    DataValueLow;
  UINT32    DataValueHigh;
} DBGKD_READ_WRITE_MSR;

typedef struct _DBGKD_QUERY_SPECIAL_CALLS {
  UINT32    NumberOfSpecialCalls;
  // UINT64 SpecialCalls[];
} DBGKD_QUERY_SPECIAL_CALLS;

typedef struct _DBGKD_SET_SPECIAL_CALL64 {
  UINT64    SpecialCall;
} DBGKD_SET_SPECIAL_CALL64;

#define DBGKD_MAX_INTERNAL_BREAKPOINTS  20

typedef struct _DBGKD_SET_INTERNAL_BREAKPOINT64 {
  UINT64    BreakpointAddress;
  UINT32    Flags;
} DBGKD_SET_INTERNAL_BREAKPOINT64;

typedef struct _DBGKD_GET_INTERNAL_BREAKPOINT64 {
  UINT64    BreakpointAddress;
  UINT32    Flags;
  UINT32    Calls;
  UINT32    MaxCallsPerPeriod;
  UINT32    MinInstructions;
  UINT32    MaxInstructions;
  UINT32    TotalInstructions;
} DBGKD_GET_INTERNAL_BREAKPOINT64;

#define DBGKD_INTERNAL_BP_FLAG_COUNTONLY  0x00000001 // don't count instructions
#define DBGKD_INTERNAL_BP_FLAG_INVALID    0x00000002 // disabled BP
#define DBGKD_INTERNAL_BP_FLAG_SUSPENDED  0x00000004 // temporarily suspended
#define DBGKD_INTERNAL_BP_FLAG_DYING      0x00000008 // kill on exit

//
// The packet protocol was widened to 64 bits in version 5.
// The PTR64 flag allows the debugger to read the right
// size of pointer when neccessary.
//
// The version packet was changed in the same revision, to remove the
// data that are now available in KDDEBUGGER_DATA.
//
// Version 6 adjusted the structures to use
// cross-platform versions all the time.
//

#define DBGKD_64BIT_PROTOCOL_VERSION2  6

typedef struct _DBGKD_SEARCH_MEMORY {
  union {
    UINT64    SearchAddress;
    UINT64    FoundAddress;
  };

  UINT64    SearchLength;
  UINT32    PatternLength;
} DBGKD_SEARCH_MEMORY;

typedef struct _DBGKD_GET_SET_BUS_DATA {
  UINT32    BusDataType;
  UINT32    BusNumber;
  UINT32    SlotNumber;
  UINT32    Offset;
  UINT32    Length;
} DBGKD_GET_SET_BUS_DATA;

#define DBGKD_FILL_MEMORY_VIRTUAL   0x00000001
#define DBGKD_FILL_MEMORY_PHYSICAL  0x00000002

typedef struct _DBGKD_FILL_MEMORY {
  UINT64    Address;
  UINT32    Length;
  UINT16    Flags;
  UINT16    PatternLength;
} DBGKD_FILL_MEMORY;

// Input AddressSpace values.
#define DBGKD_QUERY_MEMORY_VIRTUAL  0x00000000

// Output AddressSpace values.
#define DBGKD_QUERY_MEMORY_PROCESS  0x00000000
#define DBGKD_QUERY_MEMORY_SESSION  0x00000001
#define DBGKD_QUERY_MEMORY_KERNEL   0x00000002

// Output Flags.
// Currently the kernel always returns rwx.
#define DBGKD_QUERY_MEMORY_READ     0x00000001
#define DBGKD_QUERY_MEMORY_WRITE    0x00000002
#define DBGKD_QUERY_MEMORY_EXECUTE  0x00000004
#define DBGKD_QUERY_MEMORY_FIXED    0x00000008

typedef struct _DBGKD_QUERY_MEMORY {
  UINT64    Address;
  UINT64    Reserved;
  UINT32    AddressSpace;
  UINT32    Flags;
} DBGKD_QUERY_MEMORY;

typedef struct _DBGKD_MANIPULATE_STATE64 {
  UINT32    ApiNumber;
  UINT16    ProcessorLevel;
  UINT16    Processor;
  UINT32    ReturnStatus;
  union {
    DBGKD_READ_MEMORY64                ReadMemory;
    DBGKD_WRITE_MEMORY64               WriteMemory;
    DBGKD_GET_CONTEXT                  GetContext;
    DBGKD_SET_CONTEXT                  SetContext;
    DBGKD_WRITE_BREAKPOINT64           WriteBreakPoint;
    DBGKD_RESTORE_BREAKPOINT           RestoreBreakPoint;
    DBGKD_CONTINUE                     Continue;
    DBGKD_CONTINUE2                    Continue2;
    DBGKD_READ_WRITE_IO64              ReadWriteIo;
    DBGKD_READ_WRITE_IO_EXTENDED64     ReadWriteIoExtended;
    DBGKD_QUERY_SPECIAL_CALLS          QuerySpecialCalls;
    DBGKD_SET_SPECIAL_CALL64           SetSpecialCall;
    DBGKD_SET_INTERNAL_BREAKPOINT64    SetInternalBreakpoint;
    DBGKD_GET_INTERNAL_BREAKPOINT64    GetInternalBreakpoint;
    DBGKD_GET_VERSION64                GetVersion64;
    DBGKD_BREAKPOINTEX                 BreakPointEx;
    DBGKD_READ_WRITE_MSR               ReadWriteMsr;
    DBGKD_SEARCH_MEMORY                SearchMemory;
    DBGKD_GET_SET_BUS_DATA             GetSetBusData;
    DBGKD_FILL_MEMORY                  FillMemory;
    DBGKD_QUERY_MEMORY                 QueryMemory;
    DBGKD_CONTEXT_EX                   GetContextEx;
    DBGKD_CONTEXT_EX                   SetContextEx;
  } u;
} DBGKD_MANIPULATE_STATE64;

//
// Data structure for passing information to KdpReportLoadSymbolsStateChange
// function via the debug trap
//

typedef struct _KD_SYMBOLS_INFO {
  VOID      *BaseOfDll;
  UINTN     ProcessId;
  UINT32    CheckSum;
  UINT32    SizeOfImage;
} KD_SYMBOLS_INFO;

//
// If the packet type is PACKET_TYPE_KD_DEBUG_IO, then
// the format of the packet data is as follows:
//

#define DbgKdPrintStringApi  0x00003230L
#define DbgKdGetStringApi    0x00003231L

//
// For print string, the Null terminated string to print
// immediately follows the message
//
typedef struct _DBGKD_PRINT_STRING {
  UINT32    LengthOfString;
} DBGKD_PRINT_STRING;

//
// For get string, the Null terminated prompt string
// immediately follows the message. The LengthOfStringRead
// field initially contains the maximum number of characters
// to read. Upon reply, this contains the number of bytes actually
// read. The data read immediately follows the message.
//
//
typedef struct _DBGKD_GET_STRING {
  UINT32    LengthOfPromptString;
  UINT32    LengthOfStringRead;
} DBGKD_GET_STRING;

typedef struct _DBGKD_DEBUG_IO {
  UINT32    ApiNumber;
  UINT16    ProcessorLevel;
  UINT16    Processor;
  union {
    DBGKD_PRINT_STRING    PrintString;
    DBGKD_GET_STRING      GetString;
  } u;
} DBGKD_DEBUG_IO;

#endif
