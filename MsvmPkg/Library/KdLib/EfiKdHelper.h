/*++

Copyright (c) Microsoft Corporation

Module Name:

    kd64.h

Abstract:

    Header file describing the KD wire protocol.

    This file is based upon the ntdbg.h header in the kernel.

Author:

    Matthew D Hendel (math) 24-May-2005

--*/

#pragma once

//
// General defines.
//

#define IMAGE_FILE_MACHINE_AMD64             0x8664  // AMD64 (K8)
#define PAGE_ALIGN(va) (PVOID)((UINT_PTR)(va) & ~(EFI_PAGE_SIZE - 1))
#define BYTE_OFFSET(Va) ((ULONG)((LONG_PTR)(Va) & (EFI_PAGE_SIZE - 1)))

#define HIGH32(_Value64_) (UINT32)((_Value64_) >> 32)
#define LOW32(_Value64_)  (UINT32)((_Value64_) & 0xFFFFFFFF)

#define HIGH16(_Value32_) (UINT16)((_Value32_) >> 16)
#define LOW16(_Value32_)  (UINT16)((_Value32_) & 0xFFFF)

#define HIGH8(_Value16_) (UINT8)((_Value16_) >> 8)
#define LOW8(_Value16_)  (UINT8)((_Value16_) & 0xFF)

//
// File read stuff.
//

//
// The following are masks for the predefined standard access types
//

#define DELETE                           (0x00010000L)
#define READ_CONTROL                     (0x00020000L)
#define WRITE_DAC                        (0x00040000L)
#define WRITE_OWNER                      (0x00080000L)
#define SYNCHRONIZE                      (0x00100000L)

#define STANDARD_RIGHTS_REQUIRED         (0x000F0000L)

#define STANDARD_RIGHTS_READ             (READ_CONTROL)
#define STANDARD_RIGHTS_WRITE            (READ_CONTROL)
#define STANDARD_RIGHTS_EXECUTE          (READ_CONTROL)

#define STANDARD_RIGHTS_ALL              (0x001F0000L)

#define SPECIFIC_RIGHTS_ALL              (0x0000FFFFL)

#define FILE_READ_DATA            ( 0x0001 )    // file & pipe
#define FILE_LIST_DIRECTORY       ( 0x0001 )    // directory
#define FILE_WRITE_DATA           ( 0x0002 )    // file & pipe
#define FILE_ADD_FILE             ( 0x0002 )    // directory
#define FILE_APPEND_DATA          ( 0x0004 )    // file
#define FILE_ADD_SUBDIRECTORY     ( 0x0004 )    // directory
#define FILE_CREATE_PIPE_INSTANCE ( 0x0004 )    // named pipe

#define FILE_READ_EA              ( 0x0008 )    // file & directory
#define FILE_WRITE_EA             ( 0x0010 )    // file & directory
#define FILE_EXECUTE              ( 0x0020 )    // file
#define FILE_TRAVERSE             ( 0x0020 )    // directory
#define FILE_DELETE_CHILD         ( 0x0040 )    // directory
#define FILE_READ_ATTRIBUTES      ( 0x0080 )    // all
#define FILE_WRITE_ATTRIBUTES     ( 0x0100 )    // all
#define FILE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x1FF)

#define FILE_GENERIC_READ         (STANDARD_RIGHTS_READ | FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_READ_EA | SYNCHRONIZE)
#define FILE_GENERIC_WRITE        (STANDARD_RIGHTS_WRITE | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | FILE_APPEND_DATA | SYNCHRONIZE)
#define FILE_GENERIC_EXECUTE      (STANDARD_RIGHTS_EXECUTE  | FILE_READ_ATTRIBUTES | FILE_EXECUTE | SYNCHRONIZE)

//
// Define share access rights to files and directories
//

#define FILE_SHARE_READ                 0x00000001  // winnt
#define FILE_SHARE_WRITE                0x00000002  // winnt
#define FILE_SHARE_DELETE               0x00000004  // winnt
#define FILE_SHARE_VALID_FLAGS          0x00000007

//
// Define the file attributes values
//
// Note:  0x00000008 is reserved for use for the old DOS VOLID (volume ID)
//        and is therefore not considered valid in NT.
//
// Note:  Note also that the order of these flags is set to allow both the
//        FAT and the Pinball File Systems to directly set the attributes
//        flags in attributes words without having to pick each flag out
//        individually.  The order of these flags should not be changed!
//

#define FILE_ATTRIBUTE_READONLY             0x00000001  // winnt
#define FILE_ATTRIBUTE_HIDDEN               0x00000002  // winnt
#define FILE_ATTRIBUTE_SYSTEM               0x00000004  // winnt
#define FILE_ATTRIBUTE_DIRECTORY            0x00000010  // winnt
#define FILE_ATTRIBUTE_ARCHIVE              0x00000020  // winnt
#define FILE_ATTRIBUTE_DEVICE               0x00000040  // winnt
#define FILE_ATTRIBUTE_NORMAL               0x00000080  // winnt
#define FILE_ATTRIBUTE_TEMPORARY            0x00000100  // winnt
#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200  // winnt
#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400  // winnt
#define FILE_ATTRIBUTE_COMPRESSED           0x00000800  // winnt
#define FILE_ATTRIBUTE_OFFLINE              0x00001000  // winnt
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000  // winnt
#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000  // winnt

//
// Define the create disposition values
//

#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005
#define FILE_MAXIMUM_DISPOSITION        0x00000005

typedef UINT64 HV_SPA, *PHV_SPA;
typedef UINT8 *PUINT8;

extern UINT32 KdTransportMaxPacketSize;

#define _AMD64_

#if defined(_AMD64_)

//
// AMD64 specific portion of KD header
//

//
// DO NOT MODIFY THESE STRUCTURES.
//
// These sizes and offsets of these structures are hard-coded into KD, hence
// they should be considered part of the "wire protocol" for the debugger.
//

#define PROGRAM_COUNTER(_context)   ((UINT_PTR)(_context)->Rip)

#define DBGKD_MAXSTREAM 16

typedef struct _AMD64_DBGKD_CONTROL_REPORT
{
    UINT64 Dr6;
    UINT64 Dr7;
    UINT32 EFlags;
    UINT16 InstructionCount;
    UINT16 ReportFlags;
    UINT8 InstructionStream[DBGKD_MAXSTREAM];
    UINT16 SegCs;
    UINT16 SegDs;
    UINT16 SegEs;
    UINT16 SegFs;
} AMD64_DBGKD_CONTROL_REPORT, *PAMD64_DBGKD_CONTROL_REPORT;

typedef AMD64_DBGKD_CONTROL_REPORT DBGKD_CONTROL_REPORT;

typedef struct _DBGKD_ANY_CONTROL_REPORT
{
    union
    {
        AMD64_DBGKD_CONTROL_REPORT Amd64ControlReport;
    };
} DBGKD_ANY_CONTROL_REPORT, *PDBGKD_ANY_CONTROL_REPORT;

#define AMD64_REPORT_INCLUDES_SEGS    0x0001
// Indicates the current CS is a standard 64-bit flat segment.
// This allows the debugger to avoid retrieving the
// CS descriptor to see if it's 16- or 32-bit code or not.
// Note that the V86 flag in EFlags must also be checked
// when determining the code type.
#define AMD64_REPORT_STANDARD_CS      0x0002

//
// The DBGKD_CONTROL_SET structure is packed with 4 byte packing.
//

#pragma pack(push, 4)

typedef struct _AMD64_DBGKD_CONTROL_SET
{
    UINT32   TraceFlag;
    UINT64 Dr7;
    UINT64 CurrentSymbolStart;
    UINT64 CurrentSymbolEnd;
} AMD64_DBGKD_CONTROL_SET, *PAMD64_DBGKD_CONTROL_SET;

#pragma pack()

typedef AMD64_DBGKD_CONTROL_SET DBGKD_CONTROL_SET;

C_ASSERT (sizeof (AMD64_DBGKD_CONTROL_SET) == 28);


#else // not defined(_AMD64_)

#error "Unknown architecture"

#endif // defined(_AMD64_)

//
// DbgKd APIs are for the portable kernel debugger
//

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


typedef struct _KD_PACKET
{
    UINT32 PacketLeader;
    UINT16 PacketType;
    UINT16 ByteCount;
    UINT32 PacketId;
    UINT32 Checksum;
} KD_PACKET, *PKD_PACKET;


#define PACKET_MAX_SIZE 4000
#define INITIAL_PACKET_ID 0x80800000    // Don't use 0
#define SYNC_PACKET_ID    0x00000800    // Or in with INITIAL_PACKET_ID
                                        // to force a packet ID reset.

//
// BreakIn packet
//

#define BREAKIN_PACKET                  0x62626262
#define BREAKIN_PACKET_BYTE             0x62

//
// Packet lead in sequence
//

#define PACKET_LEADER                   0x30303030 //0x77000077
#define PACKET_LEADER_BYTE              0x30

#define CONTROL_PACKET_LEADER           0x69696969
#define CONTROL_PACKET_LEADER_BYTE      0x69

//
// Packet Trailing Byte
//

#define PACKET_TRAILING_BYTE            0xAA

//
// Packet Types
//

typedef enum _KD_PACKET_TYPE
{
    PACKET_TYPE_UNUSED              = 0,
    PACKET_TYPE_KD_STATE_CHANGE32   = 1,
    PACKET_TYPE_KD_STATE_MANIPULATE = 2,
    PACKET_TYPE_KD_DEBUG_IO         = 3,
    PACKET_TYPE_KD_ACKNOWLEDGE      = 4,    // Packet-control type
    PACKET_TYPE_KD_RESEND           = 5,    // Packet-control type
    PACKET_TYPE_KD_RESET            = 6,    // Packet-control type
    PACKET_TYPE_KD_STATE_CHANGE64   = 7,
    PACKET_TYPE_KD_POLL_BREAKIN     = 8,
    PACKET_TYPE_KD_TRACE_IO         = 9,
    PACKET_TYPE_KD_CONTROL_REQUEST  = 10,
    PACKET_TYPE_KD_FILE_IO          = 11,
    PACKET_TYPE_MAX                 = 12,
} KD_PACKET_TYPE, *PKD_PACKET_TYPE;

//
// If the packet type is PACKET_TYPE_KD_STATE_CHANGE, then
// the format of the packet data is as follows:
//

#define DbgKdMinimumStateChange       0x00003030L

#define DbgKdExceptionStateChange     0x00003030L
#define DbgKdLoadSymbolsStateChange   0x00003031L
#define DbgKdCommandStringStateChange 0x00003032L

#define DbgKdMaximumStateChange       0x00003033L

#define KD_REBOOT    (-1)
#define KD_HIBERNATE (-2)

//
// Pathname Data follows directly
//

typedef struct _DBGKD_LOAD_SYMBOLS64
{
    UINT32 PathNameLength;
    UINT64 BaseOfDll;
    UINT64 ProcessId;
    UINT32 CheckSum;
    UINT32 SizeOfImage;
    BOOLEAN UnloadSymbols;
} DBGKD_LOAD_SYMBOLS64, *PDBGKD_LOAD_SYMBOLS64;


typedef struct _DBGKD_GET_VERSION64
{
    UINT16  MajorVersion;
    UINT16  MinorVersion;
    UINT8   ProtocolVersion;
    UINT8   KdSecondaryVersion;
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

//
// This structure is used by the debugger for all targets
// It is the same size as DBGKD_DATA_HEADER on all systems
//
typedef struct _DBGKD_DEBUG_DATA_HEADER64
{

    //
    // Link to other blocks
    //

    LIST_ENTRY64 List;

    //
    // This is a unique tag to identify the owner of the block.
    // If your component only uses one pool tag, use it for this, too.
    //

    UINT32           OwnerTag;

    //
    // This must be initialized to the size of the data block,
    // including this structure.
    //

    UINT32           Size;

} DBGKD_DEBUG_DATA_HEADER64, *PDBGKD_DEBUG_DATA_HEADER64;

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
typedef struct _KDDEBUGGER_DATA64
{

    DBGKD_DEBUG_DATA_HEADER64 Header;

    //
    // Base address of kernel image
    //

    UINT64   KernBase;

    //
    // DbgBreakPointWithStatus is a function which takes an argument
    // and hits a breakpoint.  This field contains the address of the
    // breakpoint instruction.  When the debugger sees a breakpoint
    // at this address, it may retrieve the argument from the first
    // argument register, or on x86 the eax register.
    //

    UINT64   BreakpointWithStatus;       // address of breakpoint

    //
    // Address of the saved context record during a bugcheck
    //
    // N.B. This is an automatic in KeBugcheckEx's frame, and
    // is only valid after a bugcheck.
    //

    UINT64   SavedContext;

    //
    // help for walking stacks with user callbacks:
    //

    //
    // The address of the thread structure is provided in the
    // WAIT_STATE_CHANGE packet.  This is the offset from the base of
    // the thread structure to the pointer to the kernel stack frame
    // for the currently active usermode callback.
    //

    UINT16  ThCallbackStack;            // offset in thread data

    //
    // these values are offsets into that frame:
    //

    UINT16  NextCallback;               // saved pointer to next callback frame
    UINT16  FramePointer;               // saved frame pointer

    //
    // pad to a quad boundary
    //
    UINT16  PaeEnabled:1;

    //
    // Address of the kernel callout routine.
    //

    UINT64   KiCallUserMode;             // kernel routine

    //
    // Address of the usermode entry point for callbacks.
    //

    UINT64   KeUserCallbackDispatcher;   // address in ntdll


    //
    // Addresses of various kernel data structures and lists
    // that are of interest to the kernel debugger.
    //

    UINT64   PsLoadedModuleList;
    UINT64   PsActiveProcessHead;
    UINT64   PspCidTable;

    UINT64   ExpSystemResourcesList;
    UINT64   ExpPagedPoolDescriptor;
    UINT64   ExpNumberOfPagedPools;

    UINT64   KeTimeIncrement;
    UINT64   KeSystemErrorCallbackListHead;
    UINT64   KiSystemErrorData;

    UINT64   IopErrorLogListHead;

    UINT64   ObpRootDirectoryObject;
    UINT64   ObpTypeObjectType;

    UINT64   MmSystemCacheStart;
    UINT64   MmSystemCacheEnd;
    UINT64   MmSystemCacheWs;

    UINT64   MmPfnDatabase;
    UINT64   MmSystemPtesStart;
    UINT64   MmSystemPtesEnd;
    UINT64   MmSubsectionBase;
    UINT64   MmNumberOfPagingFiles;

    UINT64   MmLowestPhysicalPage;
    UINT64   MmHighestPhysicalPage;
    UINT64   MmNumberOfPhysicalPages;

    UINT64   MmMaximumNonPagedPoolInBytes;
    UINT64   MmNonPagedSystemStart;
    UINT64   MmNonPagedPoolStart;
    UINT64   MmNonPagedPoolEnd;

    UINT64   MmPagedPoolStart;
    UINT64   MmPagedPoolEnd;
    UINT64   MmPagedPoolInformation;
    UINT64   MmPageSize;

    UINT64   MmSizeOfPagedPoolInBytes;

    UINT64   MmTotalCommitLimit;
    UINT64   MmTotalCommittedPages;
    UINT64   MmSharedCommit;
    UINT64   MmDriverCommit;
    UINT64   MmProcessCommit;
    UINT64   MmPagedPoolCommit;
    UINT64   MmExtendedCommit;

    UINT64   MmZeroedPageListHead;
    UINT64   MmFreePageListHead;
    UINT64   MmStandbyPageListHead;
    UINT64   MmModifiedPageListHead;
    UINT64   MmModifiedNoWritePageListHead;
    UINT64   MmAvailablePages;
    UINT64   MmResidentAvailablePages;

    UINT64   PoolTrackTable;
    UINT64   NonPagedPoolDescriptor;

    UINT64   MmHighestUserAddress;
    UINT64   MmSystemRangeStart;
    UINT64   MmUserProbeAddress;

    UINT64   KdPrintCircularBuffer;
    UINT64   KdPrintCircularBufferEnd;
    UINT64   KdPrintWritePointer;
    UINT64   KdPrintRolloverCount;

    UINT64   MmLoadedUserImageList;

    // NT 5.1 Addition

    UINT64   NtBuildLab;
    UINT64   KiNormalSystemCall;

    // NT 5.0 QFE addition

    UINT64   KiProcessorBlock;
    UINT64   MmUnloadedDrivers;
    UINT64   MmLastUnloadedDriver;
    UINT64   MmTriageActionTaken;
    UINT64   MmSpecialPoolTag;
    UINT64   KernelVerifier;
    UINT64   MmVerifierData;
    UINT64   MmAllocatedNonPagedPool;
    UINT64   MmPeakCommitment;
    UINT64   MmTotalCommitLimitMaximum;
    UINT64   CmNtCSDVersion;

    // NT 5.1 Addition

    UINT64   MmPhysicalMemoryBlock;
    UINT64   MmSessionBase;
    UINT64   MmSessionSize;
    UINT64   MmSystemParentTablePage;

    // Server 2003 addition

    UINT64   MmVirtualTranslationBase;

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

    UINT64   KdPrintCircularBufferPtr;
    UINT64   KdPrintBufferSize;

    UINT64   KeLoaderBlock;

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

    UINT64   IopNumTriageDumpDataBlocks;
    UINT64   IopTriageDumpDataBlocks;

    // Longhorn addition

    UINT64   VfCrashDataBlock;

} KDDEBUGGER_DATA64, *PKDDEBUGGER_DATA64;

//
// If DBGKD_VERS_FLAG_DATA is set in Flags, info should be retrieved from
// the KDDEBUGGER_DATA block rather than from the DBGKD_GET_VERSION
// packet.  The data will remain in the version packet for a while to
// reduce compatibility problems.
//

#define DBGKD_VERS_FLAG_MP         0x0001   // kernel is MP built
#define DBGKD_VERS_FLAG_DATA       0x0002   // DebuggerDataList is valid
#define DBGKD_VERS_FLAG_PTR64      0x0004   // native pointers are 64 bits


typedef struct _DBGKD_READ_WRITE_IO_EXTENDED64
{
    UINT32 DataSize;                     // 1, 2, 4
    UINT32 InterfaceType;
    UINT32 BusNumber;
    UINT32 AddressSpace;
    UINT64 IoAddress;
    UINT32 DataValue;
} DBGKD_READ_WRITE_IO_EXTENDED64, *PDBGKD_READ_WRITE_IO_EXTENDED64;


//
// This structure is currently all zeroes.
// It just reserves a structure name for future use.
//

typedef struct _DBGKD_COMMAND_STRING
{
    UINT32 Flags;
    UINT32 Reserved1;
    UINT64 Reserved2[7];
} DBGKD_COMMAND_STRING, *PDBGKD_COMMAND_STRING;


//
// The following NT_STATUS values are hardcoded into the debugger.
// These should be used for generating exceptions.
//

#define KD_STATUS_ACCESS_VIOLATION              ((NTSTATUS)0xC0000005L)
#define KD_STATUS_ASSERTION_FAILURE             ((NTSTATUS)0xC0000420L)
#define KD_STATUS_BREAKPOINT                    ((NTSTATUS)0x80000003L)
#define KD_STATUS_CPP_EH_EXCEPTION              ((NTSTATUS)0xE06D7363L)
#define KD_STATUS_DATATYPE_MISALIGNMENT         ((NTSTATUS)0x80000002L)
#define KD_STATUS_GUARD_PAGE_VIOLATION          ((NTSTATUS)0x80000001L)
#define KD_STATUS_ILLEGAL_INSTRUCTION           ((NTSTATUS)0xC000001DL)
#define KD_STATUS_STATUS_IN_PAGE_ERROR          ((NTSTATUS)0xC0000006L)
#define KD_STATUS_INTEGER_DIVIDE_BY_ZERO        ((NTSTATUS)0xC0000094L)
#define KD_STATUS_INTEGER_OVERFLOW              ((NTSTATUS)0xC0000095L)
#define KD_STATUS_INVALID_HANDLE                ((NTSTATUS)0xC0000008L)
#define KD_STATUS_INVALID_LOCK_SEQUENCE         ((NTSTATUS)0xC000001EL)
#define KD_STATUS_INVLAID_SYSTEM_SERVICE        ((NTSTATUS)0xC000001CL)
#define KD_STATUS_PORT_DISCONNECTED             ((NTSTATUS)0xC0000037L)
#define KD_STATUS_SINGLE_STEP                   ((NTSTATUS)0x80000004L)
#define KD_STATUS_STACK_BUFFER_OVERRUN          ((NTSTATUS)0xC0000409L)
#define KD_STATUS_STACK_OVERFLOW                ((NTSTATUS)0xC00000FDL)
#define KD_STATUS_VERIFIER_STOP                 ((NTSTATUS)0xC0000421L)
#define KD_STATUS_VCPP_EXCEPTION                ((NTSTATUS)0x406D1388L)
#define KD_STATUS_WAKE_SYSTEM_DEBUGGER          ((NTSTATUS)0x80000007L)
#define KD_STATUS_WX86_BREAKPOINT               ((NTSTATUS)0x4000001FL)
#define KD_STATUS_WX86_SINGLE_STEP              ((NTSTATUS)0x4000001EL)
#define KD_DBG_CONTROL_C                        ((NTSTATUS)0x40010005L)
#define KD_DBG_CONTROL_BREAK                    ((NTSTATUS)0x40010008L)
#define KD_DBG_COMAND_EXCEPTION                 ((NTSTATUS)0x40010009L)
#define KD_STATUS_ARRAY_BOUNDS_EXCEEDED         ((NTSTATUS)0xC000008CL)
#define KD_STATUS_FLOAT_DENORMAL_OPERAND        ((NTSTATUS)0xC000008DL)
#define KD_STATUS_FLOAT_DIVIDE_BY_ZERO          ((NTSTATUS)0xC000008EL)
#define KD_STATUS_FLOAT_INVALID_OPERATION       ((NTSTATUS)0xC0000090L)
#define KD_STATUS_FLOAT_STACK_CHECK             ((NTSTATUS)0xC0000092L)
#define KD_STATUS_FLOAT_INEXACT_RESULT          ((NTSTATUS)0xC000008FL)
#define KD_STATUS_FLOAT_UNDERFLOW               ((NTSTATUS)0xC0000093L)
#define KD_STATUS_PRIVILEGED_INSTRUCTION        ((NTSTATUS)0xC0000096L)
#define KD_STATUS_NONCONTINUABLE_EXCEPTION      ((NTSTATUS)0xC0000025L)
#define KD_STATUS_INVALID_DISPOSITION           ((NTSTATUS)0xC0000026L)
#define KD_STATUS_FAST_FAIL                     KD_STATUS_STACK_BUFFER_OVERRUN

#define KD_EXCEPTION_NONCONTINUABLE             0x1


// from nti386.h:
//
//  Values put in ExceptionRecord.ExceptionInformation[0]
//  First parameter is always in ExceptionInformation[1],
//  Second parameter is always in ExceptionInformation[2]
//

#define EXCEPTION_MAXIMUM_PARAMETERS 15 // maximum number of exception parameters
//  // Exception record definition.  //
typedef struct _EXCEPTION_RECORD {
    NTSTATUS ExceptionCode;
    ULONG ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    ULONG NumberParameters;
    ULONG_PTR ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
    } EXCEPTION_RECORD;

typedef EXCEPTION_RECORD *PEXCEPTION_RECORD;

typedef struct _EXCEPTION_RECORD32 {
    NTSTATUS ExceptionCode;
    ULONG ExceptionFlags;
    ULONG ExceptionRecord;
    ULONG ExceptionAddress;
    ULONG NumberParameters;
    ULONG ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD32, *PEXCEPTION_RECORD32;

typedef struct _EXCEPTION_RECORD64 {
    NTSTATUS ExceptionCode;
    ULONG ExceptionFlags;
    ULONG64 ExceptionRecord;
    ULONG64 ExceptionAddress;
    ULONG NumberParameters;
    ULONG __unusedAlignment;
    ULONG64 ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD64, *PEXCEPTION_RECORD64;

#define COPYSE(p64,p32,f) p64->f = (ULONG64)(LONG64)(LONG)p32->f

__inline
void
ExceptionRecord32To64(
    IN PEXCEPTION_RECORD32 Ex32,
    OUT PEXCEPTION_RECORD64 Ex64
    )
{
    ULONG i;
    Ex64->ExceptionCode = Ex32->ExceptionCode;
    Ex64->ExceptionFlags = Ex32->ExceptionFlags;
    Ex64->ExceptionRecord = Ex32->ExceptionRecord;
    COPYSE(Ex64,Ex32,ExceptionAddress);
    Ex64->NumberParameters = Ex32->NumberParameters;
    for (i = 0; i < EXCEPTION_MAXIMUM_PARAMETERS; i++) {
        COPYSE(Ex64,Ex32,ExceptionInformation[i]);
    }
}

__inline
void
ExceptionRecord64To32(
    IN PEXCEPTION_RECORD64 Ex64,
    OUT PEXCEPTION_RECORD32 Ex32
    )
{
    ULONG i;
    Ex32->ExceptionCode = Ex64->ExceptionCode;
    Ex32->ExceptionFlags = Ex64->ExceptionFlags;
    Ex32->ExceptionRecord = (ULONG) Ex64->ExceptionRecord;
    Ex32->ExceptionAddress = (ULONG) Ex64->ExceptionAddress;
    Ex32->NumberParameters = Ex64->NumberParameters;
    for (i = 0; i < EXCEPTION_MAXIMUM_PARAMETERS; i++) {
        Ex32->ExceptionInformation[i] = (ULONG) Ex64->ExceptionInformation[i];
    }
}

typedef struct _DBGKM_EXCEPTION64
{
    EXCEPTION_RECORD64 ExceptionRecord;
    UINT32 FirstChance;
} DBGKM_EXCEPTION64, *PDBGKM_EXCEPTION64;


//
// Protocol version 5 64-bit state change.
//

typedef struct _DBGKD_WAIT_STATE_CHANGE64
{
    UINT32 NewState;
    UINT16 ProcessorLevel;
    UINT16 Processor;
    UINT32 NumberProcessors;
    UINT64 Thread;
    UINT64 ProgramCounter;
    union
    {
        DBGKM_EXCEPTION64 Exception;
        DBGKD_LOAD_SYMBOLS64 LoadSymbols;
    } u;
    // A processor-specific control report and context follows.
} DBGKD_WAIT_STATE_CHANGE64, *PDBGKD_WAIT_STATE_CHANGE64;
//
// There are some alignment requirements for this struct for IA64 build.
//

// Protocol version 6 state change.

typedef struct _DBGKD_ANY_WAIT_STATE_CHANGE
{

    UINT32 NewState;
    UINT16 ProcessorLevel;
    UINT16 Processor;
    UINT32 NumberProcessors;
    UINT64 Thread;
    UINT64 ProgramCounter;

    union
    {
        DBGKM_EXCEPTION64 Exception;
        DBGKD_LOAD_SYMBOLS64 LoadSymbols;
        DBGKD_COMMAND_STRING CommandString;
    } u;

    //
    // This packet must be large enough to hold any control report.
    //
    union
    {
        DBGKD_CONTROL_REPORT ControlReport;
        DBGKD_ANY_CONTROL_REPORT AnyControlReport;
    };

} DBGKD_ANY_WAIT_STATE_CHANGE, *PDBGKD_ANY_WAIT_STATE_CHANGE;

C_ASSERT (sizeof (DBGKD_ANY_WAIT_STATE_CHANGE) == 240);

//
// If the packet type is PACKET_TYPE_KD_STATE_MANIPULATE, then
// the format of the packet data is as follows:
//
// Api Numbers for state manipulation
//

#define DbgKdMinimumManipulate              0x00003130L

#define DbgKdReadVirtualMemoryApi           0x00003130L
#define DbgKdWriteVirtualMemoryApi          0x00003131L
#define DbgKdGetContextApi                  0x00003132L
#define DbgKdSetContextApi                  0x00003133L
#define DbgKdWriteBreakPointApi             0x00003134L
#define DbgKdRestoreBreakPointApi           0x00003135L
#define DbgKdContinueApi                    0x00003136L
#define DbgKdReadControlSpaceApi            0x00003137L
#define DbgKdWriteControlSpaceApi           0x00003138L
#define DbgKdReadIoSpaceApi                 0x00003139L
#define DbgKdWriteIoSpaceApi                0x0000313AL
#define DbgKdRebootApi                      0x0000313BL
#define DbgKdContinueApi2                   0x0000313CL
#define DbgKdReadPhysicalMemoryApi          0x0000313DL
#define DbgKdWritePhysicalMemoryApi         0x0000313EL
//#define DbgKdQuerySpecialCallsApi           0x0000313FL
#define DbgKdSetSpecialCallApi              0x00003140L
#define DbgKdClearSpecialCallsApi           0x00003141L
#define DbgKdSetInternalBreakPointApi       0x00003142L
#define DbgKdGetInternalBreakPointApi       0x00003143L
#define DbgKdReadIoSpaceExtendedApi         0x00003144L
#define DbgKdWriteIoSpaceExtendedApi        0x00003145L
#define DbgKdGetVersionApi                  0x00003146L
#define DbgKdWriteBreakPointExApi           0x00003147L
#define DbgKdRestoreBreakPointExApi         0x00003148L
#define DbgKdCauseBugCheckApi               0x00003149L
#define DbgKdSwitchProcessor                0x00003150L
#define DbgKdPageInApi                      0x00003151L // obsolete
#define DbgKdReadMachineSpecificRegister    0x00003152L
#define DbgKdWriteMachineSpecificRegister   0x00003153L
#define OldVlm1                             0x00003154L
#define OldVlm2                             0x00003155L
#define DbgKdSearchMemoryApi                0x00003156L
#define DbgKdGetBusDataApi                  0x00003157L
#define DbgKdSetBusDataApi                  0x00003158L
#define DbgKdCheckLowMemoryApi              0x00003159L
#define DbgKdClearAllInternalBreakpointsApi 0x0000315AL
#define DbgKdFillMemoryApi                  0x0000315BL
#define DbgKdQueryMemoryApi                 0x0000315CL
#define DbgKdSwitchPartition                0x0000315DL
#define DbgKdMidoriApi                      0x0000315EL
#define DbgKdGetContextExApi                0x0000315FL
#define DbgKdSetContextExApi                0x00003160L

#define DbgKdMaximumManipulate              0x00003161L

//
// Physical memory caching flags.
// These flags can be passed in on physical memory
// access requests in the ActualBytes field.
//

#define DBGKD_CACHING_UNKNOWN        0
#define DBGKD_CACHING_CACHED         1
#define DBGKD_CACHING_UNCACHED       2
#define DBGKD_CACHING_WRITE_COMBINED 3

//
// Response is a read memory message with data following
//

typedef struct _DBGKD_READ_MEMORY64
{
    UINT64 TargetBaseAddress;
    UINT32 TransferCount;
    UINT32 ActualBytesRead;
} DBGKD_READ_MEMORY64, *PDBGKD_READ_MEMORY64;

//
// Data follows directly
//

typedef struct _DBGKD_WRITE_MEMORY64
{
    UINT64 TargetBaseAddress;
    UINT32 TransferCount;
    UINT32 ActualBytesWritten;
} DBGKD_WRITE_MEMORY64, *PDBGKD_WRITE_MEMORY64;


//
// Response is a get context message with a full context record following
//

typedef struct _DBGKD_GET_CONTEXT
{
    UINT32 Unused;
} DBGKD_GET_CONTEXT, *PDBGKD_GET_CONTEXT;

//
// Full Context record follows
//

typedef struct _DBGKD_SET_CONTEXT
{
    UINT32 ContextFlags;
} DBGKD_SET_CONTEXT, *PDBGKD_SET_CONTEXT;

typedef struct _DBGKD_CONTEXT_EX {
    UINT32 Offset;
    UINT32 ByteCount;
    UINT32 BytesCopied;
} DBGKD_CONTEXT_EX, *PDBGKD_CONTEXT_EX;


#define BREAKPOINT_TABLE_SIZE   32      // max number supported by kernel

typedef struct _DBGKD_WRITE_BREAKPOINT64
{
    UINT64 BreakPointAddress;
    UINT32 BreakPointHandle;
} DBGKD_WRITE_BREAKPOINT64, *PDBGKD_WRITE_BREAKPOINT64;

typedef struct _DBGKD_RESTORE_BREAKPOINT
{
    UINT32 BreakPointHandle;
} DBGKD_RESTORE_BREAKPOINT, *PDBGKD_RESTORE_BREAKPOINT;

typedef struct _DBGKD_BREAKPOINTEX
{
    UINT32     BreakPointCount;
    NTSTATUS  ContinueStatus;
} DBGKD_BREAKPOINTEX, *PDBGKD_BREAKPOINTEX;

typedef struct _DBGKD_CONTINUE
{
    NTSTATUS ContinueStatus;
} DBGKD_CONTINUE, *PDBGKD_CONTINUE;

//
// DBGKD_CONTINUE2 structure must be 32-bit packed for
// for compatibility with older, processor-specific
// versions of this structure.
//

#pragma pack(push,4)

typedef struct _DBGKD_CONTINUE2
{

    NTSTATUS ContinueStatus;
    union
    {
        DBGKD_CONTROL_SET ControlSet;
        UINT8 PadToSize[28];
    };
} DBGKD_CONTINUE2, *PDBGKD_CONTINUE2;

C_ASSERT(sizeof (DBGKD_CONTINUE2) == 32);

#pragma pack(pop)

typedef struct _DBGKD_READ_WRITE_IO64
{
    UINT64 IoAddress;
    UINT32 DataSize;                     // 1, 2, 4
    UINT32 DataValue;
} DBGKD_READ_WRITE_IO64, *PDBGKD_READ_WRITE_IO64;


typedef struct _DBGKD_READ_WRITE_MSR
{
    UINT32 Msr;
    UINT32 DataValueLow;
    UINT32 DataValueHigh;
} DBGKD_READ_WRITE_MSR, *PDBGKD_READ_WRITE_MSR;


typedef struct _DBGKD_QUERY_SPECIAL_CALLS
{
    UINT32 NumberOfSpecialCalls;
    // UINT64 SpecialCalls[];
} DBGKD_QUERY_SPECIAL_CALLS, *PDBGKD_QUERY_SPECIAL_CALLS;

typedef struct _DBGKD_SET_SPECIAL_CALL64
{
    UINT64 SpecialCall;
} DBGKD_SET_SPECIAL_CALL64, *PDBGKD_SET_SPECIAL_CALL64;

#define DBGKD_MAX_INTERNAL_BREAKPOINTS 20

typedef struct _DBGKD_SET_INTERNAL_BREAKPOINT64
{
    UINT64 BreakpointAddress;
    UINT32 Flags;
} DBGKD_SET_INTERNAL_BREAKPOINT64, *PDBGKD_SET_INTERNAL_BREAKPOINT64;

typedef struct _DBGKD_GET_INTERNAL_BREAKPOINT64
{
    UINT64 BreakpointAddress;
    UINT32 Flags;
    UINT32 Calls;
    UINT32 MaxCallsPerPeriod;
    UINT32 MinInstructions;
    UINT32 MaxInstructions;
    UINT32 TotalInstructions;
} DBGKD_GET_INTERNAL_BREAKPOINT64, *PDBGKD_GET_INTERNAL_BREAKPOINT64;


#define DBGKD_INTERNAL_BP_FLAG_COUNTONLY 0x00000001 // don't count instructions
#define DBGKD_INTERNAL_BP_FLAG_INVALID   0x00000002 // disabled BP
#define DBGKD_INTERNAL_BP_FLAG_SUSPENDED 0x00000004 // temporarily suspended
#define DBGKD_INTERNAL_BP_FLAG_DYING     0x00000008 // kill on exit


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

#define DBGKD_64BIT_PROTOCOL_VERSION1 5
#define DBGKD_64BIT_PROTOCOL_VERSION2 6
#define CURRENT_KD_SECONDARY_VERSION 2

typedef struct _DBGKD_SEARCH_MEMORY
{
    union
    {
        UINT64 SearchAddress;
        UINT64 FoundAddress;
    };
    UINT64 SearchLength;
    UINT32 PatternLength;
} DBGKD_SEARCH_MEMORY, *PDBGKD_SEARCH_MEMORY;


typedef struct _DBGKD_GET_SET_BUS_DATA
{
    UINT32 BusDataType;
    UINT32 BusNumber;
    UINT32 SlotNumber;
    UINT32 Offset;
    UINT32 Length;
} DBGKD_GET_SET_BUS_DATA, *PDBGKD_GET_SET_BUS_DATA;


#define DBGKD_FILL_MEMORY_VIRTUAL  0x00000001
#define DBGKD_FILL_MEMORY_PHYSICAL 0x00000002

typedef struct _DBGKD_FILL_MEMORY
{
    UINT64 Address;
    UINT32 Length;
    UINT16 Flags;
    UINT16 PatternLength;
} DBGKD_FILL_MEMORY, *PDBGKD_FILL_MEMORY;

// Input AddressSpace values.
#define DBGKD_QUERY_MEMORY_VIRTUAL 0x00000000

// Output AddressSpace values.
#define DBGKD_QUERY_MEMORY_PROCESS 0x00000000
#define DBGKD_QUERY_MEMORY_SESSION 0x00000001
#define DBGKD_QUERY_MEMORY_KERNEL  0x00000002

// Output Flags.
// Currently the kernel always returns rwx.
#define DBGKD_QUERY_MEMORY_READ    0x00000001
#define DBGKD_QUERY_MEMORY_WRITE   0x00000002
#define DBGKD_QUERY_MEMORY_EXECUTE 0x00000004
#define DBGKD_QUERY_MEMORY_FIXED   0x00000008

typedef struct _DBGKD_QUERY_MEMORY
{
    UINT64 Address;
    UINT64 Reserved;
    UINT32 AddressSpace;
    UINT32 Flags;
} DBGKD_QUERY_MEMORY, *PDBGKD_QUERY_MEMORY;


typedef struct _DBGKD_MANIPULATE_STATE64
{
    UINT32 ApiNumber;
    UINT16 ProcessorLevel;
    UINT16 Processor;
    NTSTATUS ReturnStatus;
    union
    {
        DBGKD_READ_MEMORY64 ReadMemory;
        DBGKD_WRITE_MEMORY64 WriteMemory;
        DBGKD_GET_CONTEXT GetContext;
        DBGKD_SET_CONTEXT SetContext;
        DBGKD_WRITE_BREAKPOINT64 WriteBreakPoint;
        DBGKD_RESTORE_BREAKPOINT RestoreBreakPoint;
        DBGKD_CONTINUE Continue;
        DBGKD_CONTINUE2 Continue2;
        DBGKD_READ_WRITE_IO64 ReadWriteIo;
        DBGKD_READ_WRITE_IO_EXTENDED64 ReadWriteIoExtended;
        DBGKD_QUERY_SPECIAL_CALLS QuerySpecialCalls;
        DBGKD_SET_SPECIAL_CALL64 SetSpecialCall;
        DBGKD_SET_INTERNAL_BREAKPOINT64 SetInternalBreakpoint;
        DBGKD_GET_INTERNAL_BREAKPOINT64 GetInternalBreakpoint;
        DBGKD_GET_VERSION64 GetVersion64;
        DBGKD_BREAKPOINTEX BreakPointEx;
        DBGKD_READ_WRITE_MSR ReadWriteMsr;
        DBGKD_SEARCH_MEMORY SearchMemory;
        DBGKD_GET_SET_BUS_DATA GetSetBusData;
        DBGKD_FILL_MEMORY FillMemory;
        DBGKD_QUERY_MEMORY QueryMemory;
        DBGKD_CONTEXT_EX GetContextEx;
        DBGKD_CONTEXT_EX SetContextEx;
    } u;
} DBGKD_MANIPULATE_STATE64, *PDBGKD_MANIPULATE_STATE64;

C_ASSERT (sizeof (DBGKD_MANIPULATE_STATE64) == 56);

//
// This is the format for the trace data passed back from the kernel to
// the debugger to describe multiple calls that have returned since the
// last trip back.  The basic format is that there are a bunch of these
// (4 byte) unions stuck together.  Each union is of one of two types: a
// 4 byte unsigned long integer, or a three field struct, describing a
// call (where "call" is delimited by returning or exiting the symbol
// scope).  If the number of instructions executed is too big to fit
// into a UINT16 -1, then the Instructions field has
// TRACE_DATA_INSTRUCTIONS_BIG and the next union is a LongNumber
// containing the real number of instructions executed.
//
// The very first union returned in each callback is a LongNumber
// containing the number of unions returned (including the "size"
// record, so it's always at least 1 even if there's no data to return).
//
// This is all returned to the debugger when one of two things
// happens:
//
//   1) The pc moves out of all defined symbol ranges
//   2) The buffer of trace data entries is filled.
//
// The "trace done" case is hacked around on the debugger side.  It
// guarantees that the pc address that indicates a trace exit never
// winds up in a defined symbol range.
//
// The only other complexity in this system is handling the SymbolNumber
// table.  This table is kept in parallel by the kernel and the
// debugger.  When the PC exits a known symbol range, the Begin and End
// symbol ranges are set by the debugger and are allocated to the next
// symbol slot upon return.  "The next symbol slot" means the numerical
// next slot number, unless we've filled all slots, in which case it is
// #0.  (ie., allocation is cyclic and not LRU or something).  The
// SymbolNumber table is flushed when a SpecialCalls call is made (ie.,
// at the beginning of the WatchTrace).
//

typedef union _DBGKD_TRACE_DATA
{
    struct
    {
        UINT8 SymbolNumber;
        INT8 LevelChange;
        UINT16 Instructions;
    } s;
    UINT32 LongNumber;
} DBGKD_TRACE_DATA, *PDBGKD_TRACE_DATA;

#define TRACE_DATA_INSTRUCTIONS_BIG 0xffff

#define TRACE_DATA_BUFFER_MAX_SIZE 40

//
// If the packet type is PACKET_TYPE_KD_DEBUG_IO, then
// the format of the packet data is as follows:
//

#define DbgKdPrintStringApi     0x00003230L
#define DbgKdGetStringApi       0x00003231L

//
// For print string, the Null terminated string to print
// immediately follows the message
//
typedef struct _DBGKD_PRINT_STRING
{
    UINT32 LengthOfString;
} DBGKD_PRINT_STRING, *PDBGKD_PRINT_STRING;

//
// For get string, the Null terminated prompt string
// immediately follows the message. The LengthOfStringRead
// field initially contains the maximum number of characters
// to read. Upon reply, this contains the number of bytes actually
// read. The data read immediately follows the message.
//
//
typedef struct _DBGKD_GET_STRING
{
    UINT32 LengthOfPromptString;
    UINT32 LengthOfStringRead;
} DBGKD_GET_STRING, *PDBGKD_GET_STRING;

typedef struct _DBGKD_DEBUG_IO
{
    UINT32 ApiNumber;
    UINT16 ProcessorLevel;
    UINT16 Processor;
    union
    {
        DBGKD_PRINT_STRING PrintString;
        DBGKD_GET_STRING GetString;
    } u;
} DBGKD_DEBUG_IO, *PDBGKD_DEBUG_IO;


//
// If the packet type is PACKET_TYPE_KD_TRACE_IO, then
// the format of the packet data is as follows:
//

#define DbgKdPrintTraceApi      0x00003330L

//
// For print trace, the trace buffer data
// immediately follows the message
//
typedef struct _DBGKD_PRINT_TRACE
{
    UINT32 LengthOfData;
} DBGKD_PRINT_TRACE, *PDBGKD_PRINT_TRACE;

typedef struct _DBGKD_TRACE_IO
{
    UINT32 ApiNumber;
    UINT16 ProcessorLevel;
    UINT16 Processor;
    union
    {
        UINT64 ReserveSpace[7];
        DBGKD_PRINT_TRACE PrintTrace;
    } u;
} DBGKD_TRACE_IO, *PDBGKD_TRACE_IO;


//
// If the packet type is PACKET_TYPE_KD_CONTROL_REQUEST, then
// the format of the packet data is as follows:
//

#define DbgKdRequestHardwareBp  0x00004300L
#define DbgKdReleaseHardwareBp  0x00004301L

typedef struct _DBGKD_REQUEST_BREAKPOINT
{
    UINT32 HardwareBreakPointNumber;
    UINT32 Available;
} DBGKD_REQUEST_BREAKPOINT, *PDBGKD_REQUEST_BREAKPOINT;

typedef struct _DBGKD_RELEASE_BREAKPOINT
{
    UINT32 HardwareBreakPointNumber;
    UINT32 Released;
} DBGKD_RELEASE_BREAKPOINT, *PDBGKD_RELEASE_BREAKPOINT;


typedef struct _DBGKD_CONTROL_REQUEST
{
    UINT32 ApiNumber;
    union
    {
        DBGKD_REQUEST_BREAKPOINT RequestBreakpoint;
        DBGKD_RELEASE_BREAKPOINT ReleaseBreakpoint;
    } u;
} DBGKD_CONTROL_REQUEST, *PDBGKD_CONTROL_REQUEST;


//
// If the packet type is PACKET_TYPE_KD_FILE_IO, then
// the format of the packet data is as follows:
//

#define DbgKdCreateFileApi      0x00003430L
#define DbgKdReadFileApi        0x00003431L
#define DbgKdWriteFileApi       0x00003432L
#define DbgKdCloseFileApi       0x00003433L

//
// Unicode filename follows as additional data.
//
typedef struct _DBGKD_CREATE_FILE
{
    UINT32 DesiredAccess;
    UINT32 FileAttributes;
    UINT32 ShareAccess;
    UINT32 CreateDisposition;
    UINT32 CreateOptions;
    // Return values.
    UINT64 Handle;
    UINT64 Length;
} DBGKD_CREATE_FILE, *PDBGKD_CREATE_FILE;

//
// Data is returned as additional data in the response.
//
typedef struct _DBGKD_READ_FILE
{
    UINT64 Handle;
    UINT64 Offset;
    UINT32 Length;
} DBGKD_READ_FILE, *PDBGKD_READ_FILE;

//
// Data is given as additional data.
//
typedef struct _DBGKD_WRITE_FILE
{
    UINT64 Handle;
    UINT64 Offset;
    UINT32 Length;
} DBGKD_WRITE_FILE, *PDBGKD_WRITE_FILE;

typedef struct _DBGKD_CLOSE_FILE
{
    UINT64 Handle;
} DBGKD_CLOSE_FILE, *PDBGKD_CLOSE_FILE;

typedef struct _DBGKD_FILE_IO
{
    UINT32 ApiNumber;
    NTSTATUS Status;
    union
    {
        UINT64 ReserveSpace[7];
        DBGKD_CREATE_FILE CreateFile;
        DBGKD_READ_FILE ReadFile;
        DBGKD_WRITE_FILE WriteFile;
        DBGKD_CLOSE_FILE CloseFile;
    } u;
} DBGKD_FILE_IO, *PDBGKD_FILE_IO;


//
// Portable debugger structures.
//
// The debugger assumes several structures exist (thread, process, APC state,
// etc.) and that these structures are linked together in specific ways.
// To allow the debugger to continue to work, we define portable versions
// of these structures below. In an NT system, these would correspond to
// real structures that the OS maintains.
//
// Note that the offsets of the interesting fields within these structures
// is passed into the debugger, so we do not need to maintain the exact
// layout of NT's KTHREAD or EPROCESS.
//

typedef struct _NON_PAGED_DEBUG_INFO {
    USHORT      Signature;
    USHORT      Flags;
    ULONG       Size;
    USHORT      Machine;
    USHORT      Characteristics;
    ULONG       TimeDateStamp;
    ULONG       CheckSum;
    ULONG       SizeOfImage;
    ULONGLONG   ImageBase;
    //DebugDirectorySize      //IMAGE_DEBUG_DIRECTORY
} NON_PAGED_DEBUG_INFO, *PNON_PAGED_DEBUG_INFO;

//
// LDR_DATA_TABLE_ENTRY is taken from KLDR_DATA_TABLE_ENTRY
// minkernel/published/base/ntldr.w with the unused fields renamed
// to reduce confusion.
//
// The size and layout of this structure must match the one in ntldr.w exactly!!!
//
typedef struct _LDR_DATA_TABLE_ENTRY
{
    LIST_ENTRY InLoadOrderLinks;
    UINTN __Undefined1;
    UINTN __Undefined2;
    UINTN __Undefined3;
    PNON_PAGED_DEBUG_INFO NonPagedDebugInfo;
    UINTN DllBase;
    UINTN EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG   Flags;
    USHORT  LoadCount;
    USHORT  __Undefined5;
    UINTN   __Undefined6;
    ULONG   CheckSum;
    ULONG   __padding1;
    ULONG   TimeDateStamp;
    ULONG   __padding2;
}LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

//
// DUMP_DRIVER_ENTRY64 and DUMP_STRING are take from
// minkernel/debuggers/published/ntiodump.w
//
typedef struct _DUMP_DRIVER_ENTRY64 {
    ULONG DriverNameOffset;
    ULONG __alignment;
    LDR_DATA_TABLE_ENTRY LdrEntry;
} DUMP_DRIVER_ENTRY64, * PDUMP_DRIVER_ENTRY64;

//
// The DUMP_STRING is guaranteed to be both NULL terminated and length prefixed
// (prefix does not include the NULL).
//
typedef struct _DUMP_STRING {
    ULONG Length;                   // Length IN BYTES of the string.
    WCHAR Buffer [0];               // Buffer.
} DUMP_STRING, * PDUMP_STRING;

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

typedef enum _INTERFACE_TYPE {
    InterfaceTypeUndefined = -1,
    Internal,
    Isa,
    Eisa,
    MicroChannel,
    TurboChannel,
    PCIBus,
    VMEBus,
    NuBus,
    PCMCIABus,
    CBus,
    MPIBus,
    MPSABus,
    ProcessorInternal,
    InternalPowerBus,
    PNPISABus,
    PNPBus,
    Vmcs,
    MaximumInterfaceType
} INTERFACE_TYPE, *PINTERFACE_TYPE;

//
// Define AMD64 specific control space.
//

typedef enum _DEBUG_CONTROL_SPACE_ITEM {
    DEBUG_CONTROL_SPACE_PCR,
    DEBUG_CONTROL_SPACE_PRCB,
    DEBUG_CONTROL_SPACE_KSPECIAL,
    DEBUG_CONTROL_SPACE_THREAD,
    DEBUG_CONTROL_SPACE_MAXIMUM
} DEBUG_CONTROL_SPACE_ITEM;

//
//  Net Protocol constants
//

#define NET_PACKET_MAX_SIZE 1408        // 1024 + 256 + 128.  < 1500 for Net!
#define NET_TARGET_INITIAL_TX_PACKET_ID 0
#define NET_PACKET_ID_INCREMENT 2

//  // Define 128-bit 16-byte aligned xmm register type.  //
typedef struct DECLSPEC_ALIGN(16) _M128A {
    ULONGLONG Low;
    LONGLONG High;
} M128A, *PM128A;

//  // Format of data for (F)XSAVE/(F)XRSTOR instruction  //
typedef struct DECLSPEC_ALIGN(16) _XSAVE_FORMAT {
    USHORT ControlWord;
    USHORT StatusWord;
    UCHAR TagWord;
    UCHAR Reserved1;
    USHORT ErrorOpcode;
    ULONG ErrorOffset;
    USHORT ErrorSelector;
    USHORT Reserved2;
    ULONG DataOffset;
    USHORT DataSelector;
    USHORT Reserved3;
    ULONG MxCsr;
    ULONG MxCsr_Mask;
    M128A FloatRegisters[8];

#if defined(_WIN64)

    M128A XmmRegisters[16];
    UCHAR Reserved4[96];

#else

    M128A XmmRegisters[8];
    UCHAR Reserved4[220];

    //      // Cr0NpxState is not a part of XSAVE/XRSTOR format. The OS is relying on      // a fact that neither (FX)SAVE nor (F)XSTOR uses this area.      //
    ULONG   Cr0NpxState;

#endif

} XSAVE_FORMAT, *PXSAVE_FORMAT;

typedef XSAVE_FORMAT XMM_SAVE_AREA32, *PXMM_SAVE_AREA32;

//
// Context Frame
//
//  This frame has a several purposes: 1) it is used as an argument to
//  NtContinue, 2) it is used to constuct a call frame for APC delivery,
//  and 3) it is used in the user level thread creation routines.
//
//
// The flags field within this record controls the contents of a CONTEXT
// record.
//
// If the context record is used as an input parameter, then for each
// portion of the context record controlled by a flag whose value is
// set, it is assumed that that portion of the context record contains
// valid context. If the context record is being used to modify a threads
// context, then only that portion of the threads context is modified.
//
// If the context record is used as an output parameter to capture the
// context of a thread, then only those portions of the thread's context
// corresponding to set flags will be returned.
//
// CONTEXT_CONTROL specifies SegSs, Rsp, SegCs, Rip, and EFlags.
//
// CONTEXT_INTEGER specifies Rax, Rcx, Rdx, Rbx, Rbp, Rsi, Rdi, and R8-R15.
//
// CONTEXT_SEGMENTS specifies SegDs, SegEs, SegFs, and SegGs.
//
// CONTEXT_FLOATING_POINT specifies Xmm0-Xmm15.
//
// CONTEXT_DEBUG_REGISTERS specifies Dr0-Dr3 and Dr6-Dr7.
//

typedef struct DECLSPEC_ALIGN(16) _CONTEXT {

    //
    // Register parameter home addresses.
    //
    // N.B. These fields are for convience - they could be used to extend the
    //      context record in the future.
    //

    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5Home;
    ULONG64 P6Home;

    //
    // Control flags.
    //

    ULONG ContextFlags;
    ULONG MxCsr;

    //
    // Segment Registers and processor flags.
    //

    USHORT SegCs;
    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
    USHORT SegGs;
    USHORT SegSs;
    ULONG EFlags;

    //
    // Debug registers
    //

    ULONG64 Dr0;
    ULONG64 Dr1;
    ULONG64 Dr2;
    ULONG64 Dr3;
    ULONG64 Dr6;
    ULONG64 Dr7;

    //
    // Integer registers.
    //

    ULONG64 Rax;
    ULONG64 Rcx;
    ULONG64 Rdx;
    ULONG64 Rbx;
    ULONG64 Rsp;
    ULONG64 Rbp;
    ULONG64 Rsi;
    ULONG64 Rdi;
    ULONG64 R8;
    ULONG64 R9;
    ULONG64 R10;
    ULONG64 R11;
    ULONG64 R12;
    ULONG64 R13;
    ULONG64 R14;
    ULONG64 R15;

    //
    // Program counter.
    //

    ULONG64 Rip;

    //
    // Floating point state.
    //

    union {
        XMM_SAVE_AREA32 FltSave;
        struct {
            M128A Header[2];
            M128A Legacy[8];
            M128A Xmm0;
            M128A Xmm1;
            M128A Xmm2;
            M128A Xmm3;
            M128A Xmm4;
            M128A Xmm5;
            M128A Xmm6;
            M128A Xmm7;
            M128A Xmm8;
            M128A Xmm9;
            M128A Xmm10;
            M128A Xmm11;
            M128A Xmm12;
            M128A Xmm13;
            M128A Xmm14;
            M128A Xmm15;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;

    //
    // Vector registers.
    //

    M128A VectorRegister[26];
    ULONG64 VectorControl;

    //
    // Special debug control registers.
    //

    ULONG64 DebugControl;
    ULONG64 LastBranchToRip;
    ULONG64 LastBranchFromRip;
    ULONG64 LastExceptionToRip;
    ULONG64 LastExceptionFromRip;
} CONTEXT, *PCONTEXT;

//
// Data structure for passing information to KdpReportLoadSymbolsStateChange
// function via the debug trap
//

typedef struct _KD_SYMBOLS_INFO {
    PVOID BaseOfDll;
    UINT_PTR ProcessId;
    UINT32 CheckSum;
    UINT32 SizeOfImage;
} KD_SYMBOLS_INFO, *PKD_SYMBOLS_INFO;

//
// Define pseudo descriptor structures for both 64- and 32-bit mode.
//

typedef struct _KDESCRIPTOR {
    USHORT Pad[3];
    USHORT Limit;
    PVOID Base;
} KDESCRIPTOR, *PKDESCRIPTOR;

typedef struct _KDESCRIPTOR32 {
    USHORT Pad[3];
    USHORT Limit;
    ULONG Base;
} KDESCRIPTOR32, *PKDESCRIPTOR32;

//
// Define special kernel registers and the initial MXCSR value.
//

typedef struct _KSPECIAL_REGISTERS {
    ULONG64 Cr0;
    ULONG64 Cr2;
    ULONG64 Cr3;
    ULONG64 Cr4;
    ULONG64 KernelDr0;
    ULONG64 KernelDr1;
    ULONG64 KernelDr2;
    ULONG64 KernelDr3;
    ULONG64 KernelDr6;
    ULONG64 KernelDr7;
    KDESCRIPTOR Gdtr;
    KDESCRIPTOR Idtr;
    USHORT Tr;
    USHORT Ldtr;
    ULONG MxCsr;
    ULONG64 DebugControl;
    ULONG64 LastBranchToRip;
    ULONG64 LastBranchFromRip;
    ULONG64 LastExceptionToRip;
    ULONG64 LastExceptionFromRip;
    ULONG64 Cr8;
    ULONG64 MsrGsBase;
    ULONG64 MsrGsSwap;
    ULONG64 MsrStar;
    ULONG64 MsrLStar;
    ULONG64 MsrCStar;
    ULONG64 MsrSyscallMask;
    ULONG64 Xcr0;
} KSPECIAL_REGISTERS, *PKSPECIAL_REGISTERS;

//
// Define processor state structure.
//

typedef struct _KPROCESSOR_STATE {
    KSPECIAL_REGISTERS SpecialRegisters;
    CONTEXT ContextFrame;
} KPROCESSOR_STATE, *PKPROCESSOR_STATE;

typedef CCHAR KPROCESSOR_MODE;

//
// Exception frame
//
//  This frame is established when handling an exception. It provides a place
//  to save all nonvolatile registers. The volatile registers will already
//  have been saved in a trap frame.
//
// N.B. The exception frame has a built in exception record capable of
//      storing information for four parameter values. This exception
//      record is used exclusively within the trap handling code.
//

typedef struct _KEXCEPTION_FRAME {

//
// Home address for the parameter registers.
//

    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5;
    ULONG64 Spare1;

//
// Saved nonvolatile floating registers.
//

    M128A Xmm6;
    M128A Xmm7;
    M128A Xmm8;
    M128A Xmm9;
    M128A Xmm10;
    M128A Xmm11;
    M128A Xmm12;
    M128A Xmm13;
    M128A Xmm14;
    M128A Xmm15;

//
// Kernel callout frame variables.
//

    ULONG64 TrapFrame;
    ULONG64 OutputBuffer;
    ULONG64 OutputLength;
    ULONG64 Spare2;

//
// Saved MXCSR when a thread is interrupted in kernel mode via a dispatch
// interrupt.
//

    ULONG64 MxCsr;

//
// Saved nonvolatile register - not always saved.
//

    ULONG64 Rbp;

//
// Saved nonvolatile registers.
//

    ULONG64 Rbx;
    ULONG64 Rdi;
    ULONG64 Rsi;
    ULONG64 R12;
    ULONG64 R13;
    ULONG64 R14;
    ULONG64 R15;

//
// EFLAGS and return address.
//

    ULONG64 Return;
} KEXCEPTION_FRAME, *PKEXCEPTION_FRAME;

//
// Trap frame
//
// This frame is established when handling a trap. It provides a place to
// save all volatile registers. The nonvolatile registers are saved in an
// exception frame or through the normal C calling conventions for saved
// registers.
//

typedef struct _KTRAP_FRAME {

//
// Home address for the parameter registers.
//

    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5;

//
// Previous processor mode (system services only) and previous IRQL
// (interrupts only).
//

    KPROCESSOR_MODE PreviousMode;
    KIRQL PreviousIrql;

//
// Page fault load/store indicator.
//

    UCHAR FaultIndicator;

//
// Exception active indicator.
//
//    0 - interrupt frame.
//    1 - exception frame.
//    2 - service frame.
//

    UCHAR ExceptionActive;

//
// Floating point state.
//

    ULONG MxCsr;

//
//  Volatile registers.
//
// N.B. These registers are only saved on exceptions and interrupts. They
//      are not saved for system calls.
//

    ULONG64 Rax;
    ULONG64 Rcx;
    ULONG64 Rdx;
    ULONG64 R8;
    ULONG64 R9;
    ULONG64 R10;
    ULONG64 R11;

//
// Gsbase is only used if the previous mode was kernel.
//
// GsSwap is only used if the previous mode was user.
//

    union {
        ULONG64 GsBase;
        ULONG64 GsSwap;
    };

//
// Volatile floating registers.
//
// N.B. These registers are only saved on exceptions and interrupts. They
//      are not saved for system calls.
//

    M128A Xmm0;
    M128A Xmm1;
    M128A Xmm2;
    M128A Xmm3;
    M128A Xmm4;
    M128A Xmm5;

//
// First parameter, page fault address, context record address if user APC
// bypass, or time stamp value.
//

    union {
        ULONG64 FaultAddress;
        ULONG64 ContextRecord;
        ULONG64 TimeStampCKCL;
    };

//
//  Debug registers.
//

    ULONG64 Dr0;
    ULONG64 Dr1;
    ULONG64 Dr2;
    ULONG64 Dr3;
    ULONG64 Dr6;
    ULONG64 Dr7;

//
// Special debug registers.
//
// N.B. Either AMD64 or EM64T information is stored in the following locations.
//

    union {
        struct {
            ULONG64 DebugControl;
            ULONG64 LastBranchToRip;
            ULONG64 LastBranchFromRip;
            ULONG64 LastExceptionToRip;
            ULONG64 LastExceptionFromRip;
        };

        struct {
            ULONG64 LastBranchControl;
            ULONG LastBranchMSR;
        };
    };

//
//  Segment registers
//

    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
    USHORT SegGs;

//
// Previous trap frame address.
//

    ULONG64 TrapFrame;

//
// Saved nonvolatile registers RBX, RDI and RSI. These registers are only
// saved in system service trap frames.
//

    ULONG64 Rbx;
    ULONG64 Rdi;
    ULONG64 Rsi;

//
// Saved nonvolatile register RBP. This register is used as a frame
// pointer during trap processing and is saved in all trap frames.
//

    ULONG64 Rbp;

//
// Information pushed by hardware.
//
// N.B. The error code is not always pushed by hardware. For those cases
//      where it is not pushed by hardware a dummy error code is allocated
//      on the stack.
//

    union {
        ULONG64 ErrorCode;
        ULONG64 ExceptionFrame;
        ULONG64 TimeStampKlog;
    };

    ULONG64 Rip;
    USHORT SegCs;
    UCHAR Fill0;
    UCHAR Logging;
    USHORT Fill1[2];
    ULONG EFlags;
    ULONG Fill2;
    ULONG64 Rsp;
    USHORT SegSs;
    USHORT Fill3;

//
// Copy of the global patch cycle at the time of the fault. Filled in by the
// invalid opcode and general protection fault routines.
//

    LONG CodePatchCycle;
} KTRAP_FRAME, *PKTRAP_FRAME;


#if 1
/*typedef
_IRQL_requires_same_
_Function_class_(EXCEPTION_ROUTINE)
EXCEPTION_DISPOSITION
EXCEPTION_ROUTINE (
    _Inout_ struct _EXCEPTION_RECORD *ExceptionRecord,
    _In_ PVOID EstablisherFrame,
    _Inout_ struct _CONTEXT *ContextRecord,
    _In_ PVOID DispatcherContext
    );
typedef EXCEPTION_ROUTINE *PEXCEPTION_ROUTINE;
*/
typedef PVOID PEXCEPTION_ROUTINE;

typedef struct _EXCEPTION_REGISTRATION_RECORD {
    struct _EXCEPTION_REGISTRATION_RECORD *Next;
    PEXCEPTION_ROUTINE Handler;
} EXCEPTION_REGISTRATION_RECORD;

typedef EXCEPTION_REGISTRATION_RECORD *PEXCEPTION_REGISTRATION_RECORD;

typedef struct _NT_TIB {
    struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID SubSystemTib;
#if defined(_MSC_EXTENSIONS)
    union {
        PVOID FiberData;
        ULONG Version;
    };
#else
    PVOID FiberData;
#endif
    PVOID ArbitraryUserPointer;
    struct _NT_TIB *Self;
} NT_TIB;
typedef NT_TIB *PNT_TIB;

typedef union _KGDTENTRY64 {
    struct {
        USHORT LimitLow;
        USHORT BaseLow;
        union {
            struct {
                UCHAR BaseMiddle;
                UCHAR Flags1;
                UCHAR Flags2;
                UCHAR BaseHigh;
            } Bytes;

            struct {
                ULONG BaseMiddle : 8;
                ULONG Type : 5;
                ULONG Dpl : 2;
                ULONG Present : 1;
                ULONG LimitHigh : 4;
                ULONG System : 1;
                ULONG LongMode : 1;
                ULONG DefaultBig : 1;
                ULONG Granularity : 1;
                ULONG BaseHigh : 8;
            } Bits;
        };

        ULONG BaseUpper;
        ULONG MustBeZero;
    };

    struct {
        LONG64 DataLow;
        LONG64 DataHigh;
    };

} KGDTENTRY64, *PKGDTENTRY64;

typedef PVOID PKSPIN_LOCK_QUEUE;
//
// Define Task State Segment (TSS) structure and constants.
//
// Task switches are not supported by the AMD64, but a task state segment
// must be present to define the kernel stack pointer and I/O map base.
//
// N.B. This structure is misaligned as per the AMD64 specification.
//
// N.B. The size of TSS must be <= 0xDFFF.
//

#pragma pack(push, 4)
typedef struct _KTSS64 {
    ULONG Reserved0;
    ULONG64 Rsp0;
    ULONG64 Rsp1;
    ULONG64 Rsp2;

    //
    // Element 0 of the Ist is reserved.
    //

    ULONG64 Ist[8];
    ULONG64 Reserved1;
    USHORT Reserved2;
    USHORT IoMapBase;
} KTSS64, *PKTSS64;
#pragma pack(pop)

typedef ULONG_PTR KSPIN_LOCK;
typedef KSPIN_LOCK *PKSPIN_LOCK;

typedef struct _KPRCB
{

    ULONG MxCsr;
    UCHAR LegacyNumber;
    UCHAR ReservedMustBeZero;
    BOOLEAN InterruptRequest;
    BOOLEAN IdleHalt;
    struct _KTHREAD *CurrentThread;
    struct _KTHREAD *NextThread;
    struct _KTHREAD *IdleThread;
    UCHAR NestingLevel;
    BOOLEAN ClockOwner;
    BOOLEAN PendingTick;
    UCHAR PrcbPad00[1];
    ULONG Number;
    ULONG64 RspBase;
    KSPIN_LOCK PrcbLock;
    ULONG64 PrcbPad01;
    KPROCESSOR_STATE ProcessorState;

} KPRCB, *PKPRCB;

typedef struct _KPCR {

//
// Start of the architecturally defined section of the PCR. This section
// may be directly addressed by vendor/platform specific HAL code and will
// not change from version to version of NT.
//
// Certain fields in the TIB are not used in kernel mode. These include the
// exception list, stack base, stack limit, subsystem TIB, fiber data, and
// the arbitrary user pointer. Therefore, these fields are overlaid with
// other data to get better cache locality.
//
// N.B. The offset to the PRCB in the PCR is fixed for all time.
//

    union {
        NT_TIB NtTib;
        struct {
            union _KGDTENTRY64 *GdtBase;
            struct _KTSS64 *TssBase;
            ULONG64 UserRsp;
            struct _KPCR *Self;
            struct _KPRCB *CurrentPrcb;
            PKSPIN_LOCK_QUEUE LockArray;
            PVOID Used_Self;
        };
    };

    union _KIDTENTRY64 *IdtBase;
    ULONG64 Unused[2];
    KIRQL Irql;
    UCHAR SecondLevelCacheAssociativity;
    UCHAR ObsoleteNumber;
    UCHAR Fill0;
    ULONG Unused0[3];
    USHORT MajorVersion;
    USHORT MinorVersion;
    ULONG StallScaleFactor;
    PVOID Unused1[3];

    ULONG KernelReserved[15];
    ULONG SecondLevelCacheSize;
    ULONG HalReserved[16];
    ULONG Unused2;
    PVOID KdVersionBlock;
    PVOID Unused3;
    ULONG PcrAlign1[24];

// end_ntddk

    KPRCB Prcb;

//
// End of the architecturally defined section of the PCR.
//
// end_ntosp
//
// N.B. This is the start of the architecturally defined part of the PRCB.
//      The preceeding PCR layout cannot change for all time. The initial
//      architecturally defined part of the PRCB cannot change for all time
//      either.
//
//      Since KPRCB is a variable sized structure, Prcb must be the last
//      field in KPCR.
//

// begin_ntddk begin_ntosp

} KPCR, *PKPCR;
#endif

typedef enum {
    ContinueError = FALSE,
    ContinueSuccess = TRUE,
    ContinueProcessorReselected,
    ContinueNextProcessor
} KCONTINUE_STATUS;

#define CONTEXT_TO_PROGRAM_COUNTER(Context) ((Context)->Rip)

#define CONTEXT_AMD64           0x00100000L

// end_wx86
#define CONTEXT_CONTROL         (CONTEXT_AMD64 | 0x00000001L)
#define CONTEXT_INTEGER         (CONTEXT_AMD64 | 0x00000002L)
#define CONTEXT_SEGMENTS        (CONTEXT_AMD64 | 0x00000004L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_AMD64 | 0x00000008L)
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_AMD64 | 0x00000010L)

#define CONTEXT_FULL            (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT)

#define BAGDT_DATA_SELECTOR         0x30
#define X86_REPORT_INCLUDES_SEGS    0x0001

#define EFLAGS_CF_MASK 0x00000001       // carry flag
#define EFLAGS_PF_MASK 0x00000004       // parity flag
#define EFLAGS_AF_MASK 0x00000010       // auxiliary carry flag
#define EFLAGS_ZF_MASK 0x00000040       // zero flag
#define EFLAGS_SF_MASK 0x00000080       // sign flag
#define EFLAGS_TF_MASK 0x00000100       // trap flag
#define EFLAGS_IF_MASK 0x00000200       // interrupt flag
#define EFLAGS_DF_MASK 0x00000400       // direction flag
#define EFLAGS_OF_MASK 0x00000800       // overflow flag
#define EFLAGS_IOPL_MASK 0x00003000     // I/O privilege level
#define EFLAGS_NT_MASK 0x00004000       // nested task
#define EFLAGS_RF_MASK 0x00010000       // resume flag
#define EFLAGS_VM_MASK 0x00020000       // virtual 8086 mode
#define EFLAGS_AC_MASK 0x00040000       // alignment check
#define EFLAGS_VIF_MASK 0x00080000      // virtual interrupt flag
#define EFLAGS_VIP_MASK 0x00100000      // virtual interrupt pending
#define EFLAGS_ID_MASK 0x00200000       // identification flag
#define EFLAGS_TF_SHIFT 8               // trap
#define EFLAGS_IF_SHIFT 9               // interrupt enable
#define EFLAGS_SYSCALL_CLEAR (EFLAGS_IF_MASK | EFLAGS_DF_MASK | EFLAGS_TF_MASK | EFLAGS_NT_MASK)



typedef struct {
    UCHAR     Type;  //CmResourceType      BOOLEAN   Valid;
    UCHAR     Reserved[2];
    PUCHAR    TranslatedAddress;
    ULONG     Length;
} DEBUG_DEVICE_ADDRESS, *PDEBUG_DEVICE_ADDRESS;

typedef struct {
    PHYSICAL_ADDRESS  Start;
    PHYSICAL_ADDRESS  MaxEnd;
    PVOID             VirtualAddress;
    ULONG             Length;
    BOOLEAN           Cached;
    BOOLEAN           Aligned;
} DEBUG_MEMORY_REQUIREMENTS, *PDEBUG_MEMORY_REQUIREMENTS;

typedef enum {
    KdNameSpacePCI,
    KdNameSpaceACPI,
    KdNameSpaceAny,

    //      // Maxmimum namespace enumerator.      //
    KdNameSpaceMax,
} KD_NAMESPACE_ENUM, *PKD_NAMESPACE_ENUM;

typedef enum {
    KdConfigureDeviceAndContinue,
    KdSkipDeviceAndContinue,
    KdConfigureDeviceAndStop,
    KdSkipDeviceAndStop,
} KD_CALLBACK_ACTION, *PKD_CALLBACK_ACTION;

#define MAXIMUM_DEBUG_BARS 6

typedef struct {
    ULONG     Bus;
    ULONG     Slot;
    USHORT    Segment;
    USHORT    VendorID;
    USHORT    DeviceID;
    UCHAR     BaseClass;
    UCHAR     SubClass;
    UCHAR     ProgIf;
    UCHAR     Flags;
    BOOLEAN   Initialized;
    BOOLEAN   Configured;
    DEBUG_DEVICE_ADDRESS BaseAddress[MAXIMUM_DEBUG_BARS];
    DEBUG_MEMORY_REQUIREMENTS Memory;
    USHORT    PortType;
    USHORT    PortSubtype;
    PVOID     OemData;
    ULONG     OemDataLength;
    KD_NAMESPACE_ENUM NameSpace;
    PVOID     NameSpacePath;
    ULONG     NameSpacePathLength;
} DEBUG_DEVICE_DESCRIPTOR, *PDEBUG_DEVICE_DESCRIPTOR;

typedef struct _KD_CONTEXT {
    ULONG KdpDefaultRetries;
    BOOLEAN KdpControlCPending;
} KD_CONTEXT, *PKD_CONTEXT;

typedef enum _KD_EXCEPTION_CODE
{
    KdExceptionSingleStep =             0x1001, // Single step
    KdExceptionBreakpoint =             0x1002, // Breakpoint
    KdExceptionInvalidOpcodeFault =     0x1003, // Invalid opcode fault
    KdExceptionGeneralProtectionFault = 0x1004, // General protection fault
    KdExceptionPageFault =              0x1005, // Page fault
    KdExceptionDebugService =           0x1006, // Debug service
    KdExceptionDoubleFault =            0x1007, // Double fault
    KdExceptionSx =                     0x1008, // #SX Exception (SVM-only)
    KdExceptionAssertionFailure =       0x1009, // Assertion failure
    KdExceptionDivideError =            0x100A, // Divide Error
    KdExceptionOverflowTrap =           0x100B, // Overflow trap
    KdExceptionBoundFault =             0x100C, // Bound Fault
    KdExceptionAlignmentFault =         0x100D, // Alignment Fault
    KdExceptionFloatingPointFault =     0x100E, // Legacy Floating Point Fault
    KdExceptionFastFail =               0x100F  // Fast fail

} KD_EXCEPTION_CODE, *PKD_EXCEPTION_CODE;

//
// _UNLOADED_DRIVERS from ntos\inc\mm.h
// Used to track unloaded UEFI modules.
//
typedef struct _EFI_UNLOADED_MODULE {
    UNICODE_STRING Name;
    PVOID StartAddress;
    PVOID EndAddress;
    UINT64 CurrentTime;
} EFI_UNLOADED_MODULE, *PEFI_UNLOADED_MODULE;
