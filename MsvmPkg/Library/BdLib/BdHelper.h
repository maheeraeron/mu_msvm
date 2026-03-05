/*++

Copyright (c) Microsoft Corporation

Module Name:

    bdhelper.h

Abstract:

    Adaptation based upon ntdbg.h, kd64.h, etc.

--*/
#pragma once
#include "AllowNamelessAggregate.h"
#include "DeclspecAlign.h"

#if !defined(_AMD64_) && !defined(_ARM64_)
#error unsupported architecture
#endif

#define IMAGE_FILE_MACHINE_AMD64            0x8664

#define PAGE_ALIGN(va) (PVOID)((UINT_PTR)(va) & ~(EFI_PAGE_SIZE - 1))
#define BYTE_OFFSET(Va) ((ULONG)((LONG_PTR)(Va) & (EFI_PAGE_SIZE - 1)))

#define HIGH8(_Value16_) (UINT8)((_Value16_) >> 8)
#define LOW8(_Value16_)  (UINT8)((_Value16_) & 0xFF)

#define Add2Ptr(_Ptr, _Value) ((PVOID)((PUCHAR)(_Ptr) + (_Value)))

#define PAGE_SIZE 0x1000
#define PAGE_SHIFT 12L

typedef UINT64 HV_SPA, *PHV_SPA;
typedef UINT8 *PUINT8;

extern UINT32 BdTransportMaxPacketSize;
extern UINT32 BdTrapRoutine;

#define DBGKD_MAXSTREAM 16

typedef struct _X86_DBGKD_CONTROL_REPORT {
    ULONG   Dr6;
    ULONG   Dr7;
    USHORT  InstructionCount;
    USHORT  ReportFlags;
    UCHAR   InstructionStream[DBGKD_MAXSTREAM];
    USHORT  SegCs;
    USHORT  SegDs;
    USHORT  SegEs;
    USHORT  SegFs;
    ULONG   EFlags;
} X86_DBGKD_CONTROL_REPORT, *PX86_DBGKD_CONTROL_REPORT;

#define X86_REPORT_INCLUDES_SEGS    0x0001
#define X86_REPORT_STANDARD_CS      0x0002

typedef struct _AMD64_DBGKD_CONTROL_REPORT {
    ULONG64 Dr6;
    ULONG64 Dr7;
    ULONG EFlags;
    USHORT InstructionCount;
    USHORT ReportFlags;
    UCHAR InstructionStream[DBGKD_MAXSTREAM];
    USHORT SegCs;
    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
} AMD64_DBGKD_CONTROL_REPORT, *PAMD64_DBGKD_CONTROL_REPORT;

#define AMD64_REPORT_INCLUDES_SEGS    0x0001
#define AMD64_REPORT_STANDARD_CS      0x0002

typedef struct _ARM64_DBGKD_CONTROL_REPORT {
    ULONG Cpsr;
    ULONG InstructionCount;
    UCHAR InstructionStream[DBGKD_MAXSTREAM];
} ARM64_DBGKD_CONTROL_REPORT, *PARM64_DBGKD_CONTROL_REPORT;

#pragma pack(push, 4)

typedef struct _X86_DBGKD_CONTROL_SET {
    ULONG   TraceFlag;
    ULONG   Dr7;
    ULONG   CurrentSymbolStart;
    ULONG   CurrentSymbolEnd;
} X86_DBGKD_CONTROL_SET, *PX86_DBGKD_CONTROL_SET;

typedef struct _AMD64_DBGKD_CONTROL_SET {
    ULONG   TraceFlag;
    ULONG64 Dr7;
    ULONG64 CurrentSymbolStart;
    ULONG64 CurrentSymbolEnd;
} AMD64_DBGKD_CONTROL_SET, *PAMD64_DBGKD_CONTROL_SET;

typedef struct _ARM64_DBGKD_CONTROL_SET {
    ULONG   Continue;
    ULONG   TraceFlag;
    ULONG64 CurrentSymbolStart;
    ULONG64 CurrentSymbolEnd;
} ARM64_DBGKD_CONTROL_SET, *PARM64_DBGKD_CONTROL_SET;

#pragma pack(pop)

typedef struct _DBGKD_ANY_CONTROL_REPORT {
    union {
        X86_DBGKD_CONTROL_REPORT X86ControlReport;
        AMD64_DBGKD_CONTROL_REPORT Amd64ControlReport;
        ARM64_DBGKD_CONTROL_REPORT Arm64ControlReport;
    };
} DBGKD_ANY_CONTROL_REPORT, *PDBGKD_ANY_CONTROL_REPORT;


#if defined(_AMD64_)

#define PROGRAM_COUNTER(_context)   ((UINT_PTR)(_context)->Rip)

typedef AMD64_DBGKD_CONTROL_REPORT DBGKD_CONTROL_REPORT;
typedef AMD64_DBGKD_CONTROL_SET DBGKD_CONTROL_SET;

#elif defined(_ARM64_)

#define PROGRAM_COUNTER(_context)   ((ULONG_PTR)(_context)->Pc)

typedef ARM64_DBGKD_CONTROL_REPORT DBGKD_CONTROL_REPORT;
typedef ARM64_DBGKD_CONTROL_SET DBGKD_CONTROL_SET;

#endif

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
    ULONG PacketLeader;
    USHORT PacketType;
    USHORT ByteCount;
    ULONG PacketId;
    ULONG Checksum;
} KD_PACKET, *PKD_PACKET;


#define PACKET_MAX_SIZE   4000
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

// If the state change is from an alternate source
// then this bit is combined with the basic state change code.
#define DbgKdAlternateStateChange     0x00010000L

#define KD_REBOOT    (-1)
#define KD_HIBERNATE (-2)

//
// Pathname Data follows directly
//

typedef struct _DBGKD_LOAD_SYMBOLS32 {
    ULONG PathNameLength;
    ULONG BaseOfDll;
    ULONG ProcessId;
    ULONG CheckSum;
    ULONG SizeOfImage;
    BOOLEAN UnloadSymbols;
} DBGKD_LOAD_SYMBOLS32, *PDBGKD_LOAD_SYMBOLS32;

typedef struct _DBGKD_LOAD_SYMBOLS64
{
    UINT32 PathNameLength;
    UINT64 BaseOfDll;
    UINT64 ProcessId;
    UINT32 CheckSum;
    UINT32 SizeOfImage;
    BOOLEAN UnloadSymbols;
} DBGKD_LOAD_SYMBOLS64, *PDBGKD_LOAD_SYMBOLS64;

typedef struct _DBGKD_GET_VERSION64 {
    UINT16  MajorVersion;
    UINT16  MinorVersion;
    UINT8   ProtocolVersion;
    UINT8   KdSecondaryVersion;
    UINT16  Flags;
    UINT16  MachineType;
    UINT8   MaxPacketType;
    UINT8   MaxStateChange;
    UINT8   MaxManipulate;
    UINT8   Simulation;
    UINT16  Unused[1];
    UINT64 KernBase;
    UINT64 PsLoadedModuleList;
    UINT64 DebuggerDataList;
} DBGKD_GET_VERSION64, *PDBGKD_GET_VERSION64;

typedef struct _DBGKD_DEBUG_DATA_HEADER64 {
    LIST_ENTRY64 List;
    UINT32           OwnerTag;
    UINT32           Size;
} DBGKD_DEBUG_DATA_HEADER64, *PDBGKD_DEBUG_DATA_HEADER64;

typedef struct _KDDEBUGGER_DATA64 {
    DBGKD_DEBUG_DATA_HEADER64 Header;
    ULONG64   KernBase;
    ULONG64   BreakpointWithStatus;
    ULONG64   SavedContext;
    USHORT    ThCallbackStack;
    USHORT    NextCallback;
    USHORT    FramePointer;
    USHORT    PaeEnabled:1;
    ULONG64   KiCallUserMode;
    ULONG64   KeUserCallbackDispatcher;
    ULONG64   PsLoadedModuleList;
    ULONG64   PsActiveProcessHead;
    ULONG64   PspCidTable;
    ULONG64   ExpSystemResourcesList;
    ULONG64   ExpPagedPoolDescriptor;
    ULONG64   ExpNumberOfPagedPools;
    ULONG64   KeTimeIncrement;
    ULONG64   KeBugCheckCallbackListHead;
    ULONG64   KiBugcheckData;
    ULONG64   IopErrorLogListHead;
    ULONG64   ObpRootDirectoryObject;
    ULONG64   ObpTypeObjectType;
    ULONG64   MmSystemCacheStart;
    ULONG64   MmSystemCacheEnd;
    ULONG64   MmSystemCacheWs;
    ULONG64   MmPfnDatabase;
    ULONG64   MmSystemPtesStart;
    ULONG64   MmSystemPtesEnd;
    ULONG64   MmSubsectionBase;
    ULONG64   MmNumberOfPagingFiles;
    ULONG64   MmLowestPhysicalPage;
    ULONG64   MmHighestPhysicalPage;
    ULONG64   MmNumberOfPhysicalPages;
    ULONG64   MmMaximumNonPagedPoolInBytes;
    ULONG64   MmNonPagedSystemStart;
    ULONG64   MmNonPagedPoolStart;
    ULONG64   MmNonPagedPoolEnd;
    ULONG64   MmPagedPoolStart;
    ULONG64   MmPagedPoolEnd;
    ULONG64   MmPagedPoolInformation;
    ULONG64   MmPageSize;
    ULONG64   MmSizeOfPagedPoolInBytes;
    ULONG64   MmTotalCommitLimit;
    ULONG64   MmTotalCommittedPages;
    ULONG64   MmSharedCommit;
    ULONG64   MmDriverCommit;
    ULONG64   MmProcessCommit;
    ULONG64   MmPagedPoolCommit;
    ULONG64   MmExtendedCommit;
    ULONG64   MmZeroedPageListHead;
    ULONG64   MmFreePageListHead;
    ULONG64   MmStandbyPageListHead;
    ULONG64   MmModifiedPageListHead;
    ULONG64   MmModifiedNoWritePageListHead;
    ULONG64   MmAvailablePages;
    ULONG64   MmResidentAvailablePages;
    ULONG64   PoolTrackTable;
    ULONG64   NonPagedPoolDescriptor;
    ULONG64   MmHighestUserAddress;
    ULONG64   MmSystemRangeStart;
    ULONG64   MmUserProbeAddress;
    ULONG64   KdPrintCircularBuffer;
    ULONG64   KdPrintCircularBufferEnd;
    ULONG64   KdPrintWritePointer;
    ULONG64   KdPrintRolloverCount;
    ULONG64   MmLoadedUserImageList;
    ULONG64   NtBuildLab;
    ULONG64   KiNormalSystemCall;
    ULONG64   KiProcessorBlock;
    ULONG64   MmUnloadedDrivers;
    ULONG64   MmLastUnloadedDriver;
    ULONG64   MmTriageActionTaken;
    ULONG64   MmSpecialPoolTag;
    ULONG64   KernelVerifier;
    ULONG64   MmVerifierData;
    ULONG64   MmAllocatedNonPagedPool;
    ULONG64   MmPeakCommitment;
    ULONG64   MmTotalCommitLimitMaximum;
    ULONG64   CmNtCSDVersion;
    ULONG64   MmPhysicalMemoryBlock;
    ULONG64   MmSessionBase;
    ULONG64   MmSessionSize;
    ULONG64   MmSystemParentTablePage;
    ULONG64   MmVirtualTranslationBase;
    USHORT    OffsetKThreadNextProcessor;
    USHORT    OffsetKThreadTeb;
    USHORT    OffsetKThreadKernelStack;
    USHORT    OffsetKThreadInitialStack;
    USHORT    OffsetKThreadApcProcess;
    USHORT    OffsetKThreadState;
    USHORT    OffsetKThreadBStore;
    USHORT    OffsetKThreadBStoreLimit;
    USHORT    SizeEProcess;
    USHORT    OffsetEprocessPeb;
    USHORT    OffsetEprocessParentCID;
    USHORT    OffsetEprocessDirectoryTableBase;
    USHORT    SizePrcb;
    USHORT    OffsetPrcbDpcRoutine;
    USHORT    OffsetPrcbCurrentThread;
    USHORT    OffsetPrcbMhz;
    USHORT    OffsetPrcbCpuType;
    USHORT    OffsetPrcbVendorString;
    USHORT    OffsetPrcbProcStateContext;
    USHORT    OffsetPrcbNumber;
    USHORT    SizeEThread;
    ULONG64   KdPrintCircularBufferPtr;
    ULONG64   KdPrintBufferSize;
    ULONG64   KeLoaderBlock;
    USHORT    SizePcr;
    USHORT    OffsetPcrSelfPcr;
    USHORT    OffsetPcrCurrentPrcb;
    USHORT    OffsetPcrContainedPrcb;
    USHORT    OffsetPcrInitialBStore;
    USHORT    OffsetPcrBStoreLimit;
    USHORT    OffsetPcrInitialStack;
    USHORT    OffsetPcrStackLimit;
    USHORT    OffsetPrcbPcrPage;
    USHORT    OffsetPrcbProcStateSpecialReg;
    USHORT    GdtR0Code;
    USHORT    GdtR0Data;
    USHORT    GdtR0Pcr;
    USHORT    GdtR3Code;
    USHORT    GdtR3Data;
    USHORT    GdtR3Teb;
    USHORT    GdtLdt;
    USHORT    GdtTss;
    USHORT    Gdt64R3CmCode;
    USHORT    Gdt64R3CmTeb;
    ULONG64   IopNumTriageDumpDataBlocks;
    ULONG64   IopTriageDumpDataBlocks;
    ULONG64   VfCrashDataBlock;
    ULONG64   MmBadPagesDetected;
    ULONG64   MmZeroedPageSingleBitErrorsDetected;
    ULONG64   EtwpDebuggerData;
    USHORT    OffsetPrcbContext;
    USHORT    OffsetPrcbMaxBreakpoints;
    USHORT    OffsetPrcbMaxWatchpoints;
    ULONG     OffsetKThreadStackLimit;
    ULONG     OffsetKThreadStackBase;
    ULONG     OffsetKThreadQueueListEntry;
    ULONG     OffsetEThreadIrpList;
    USHORT    OffsetPrcbIdleThread;
    USHORT    OffsetPrcbNormalDpcState;
    USHORT    OffsetPrcbDpcStack;
    USHORT    OffsetPrcbIsrStack;
    USHORT    SizeKDPC_STACK_FRAME;
    USHORT    OffsetKPriQueueThreadListHead;
    USHORT    OffsetKThreadWaitReason;
    USHORT    Padding;
    ULONG64   PteBase;
} KDDEBUGGER_DATA64, *PKDDEBUGGER_DATA64;

#define DBGKD_VERS_FLAG_MP         0x0001
#define DBGKD_VERS_FLAG_DATA       0x0002
#define DBGKD_VERS_FLAG_PTR64      0x0004


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

#define BREAKPOINT_BREAK 0
#define BREAKPOINT_PRINT 1
#define BREAKPOINT_PROMPT 2
#define BREAKPOINT_LOAD_SYMBOLS 3
#define BREAKPOINT_UNLOAD_SYMBOLS 4
#define BREAKPOINT_COMMAND_STRING 5
#define BREAKPOINT_HW_WATCH 6
#define BREAKPOINT_HW_BREAK 7
#define BREAKPOINT_GET_TABLE 8


//
// Define control space.
//

typedef enum _DEBUG_CONTROL_SPACE_ITEM {
    DEBUG_CONTROL_SPACE_PCR,
    DEBUG_CONTROL_SPACE_PRCB,
    DEBUG_CONTROL_SPACE_KSPECIAL,
    DEBUG_CONTROL_SPACE_THREAD,
    DEBUG_CONTROL_SPACE_MAXIMUM
} DEBUG_CONTROL_SPACE_ITEM;


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

#if defined(_AMD64_)

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

typedef struct DECLSPEC_ALIGN(16) AMD64_CONTEXT {

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
} AMD64_CONTEXT, *PAMD64_CONTEXT;

//
//  Define the size of the 80387 save area, which is in the context frame.
//

#define X86_SIZE_OF_80387_REGISTERS      80

typedef struct _X86_FLOATING_SAVE_AREA {
    ULONG   ControlWord;
    ULONG   StatusWord;
    ULONG   TagWord;
    ULONG   ErrorOffset;
    ULONG   ErrorSelector;
    ULONG   DataOffset;
    ULONG   DataSelector;
    UCHAR   RegisterArea[X86_SIZE_OF_80387_REGISTERS];
    ULONG   Cr0NpxState;
} X86_FLOATING_SAVE_AREA;

#define MAXIMUM_SUPPORTED_EXTENSION     512
#define X86_CONTEXT_ALIGN               4

//
// Define the size of FP registers in the FXSAVE format
//
#define X86_SIZE_OF_FX_REGISTERS        128

typedef struct DECLSPEC_ALIGN(16) _X86_FXSAVE_FORMAT {
    USHORT  ControlWord;
    USHORT  StatusWord;
    USHORT  TagWord;
    USHORT  ErrorOpcode;
    ULONG   ErrorOffset;
    ULONG   ErrorSelector;
    ULONG   DataOffset;
    ULONG   DataSelector;
    ULONG   MXCsr;
    ULONG   Reserved2;
    UCHAR   RegisterArea[X86_SIZE_OF_FX_REGISTERS];
    UCHAR   Reserved3[X86_SIZE_OF_FX_REGISTERS];
    UCHAR   Reserved4[224];
} X86_FXSAVE_FORMAT, *PX86_FXSAVE_FORMAT;


//
// Define pseudo descriptor structures for both 64- and 32-bit mode.
//

typedef struct _KDESCRIPTOR {
    USHORT Pad[3];
    USHORT Limit;
    PVOID  Base;
} KDESCRIPTOR, *PKDESCRIPTOR;

typedef struct _KDESCRIPTOR32 {
    USHORT Pad;
    USHORT Limit;
    ULONG  Base;
} KDESCRIPTOR32, *PKDESCRIPTOR32;


#pragma pack(push, 4)

typedef struct _X86_KSPECIAL_REGISTERS {
    ULONG Cr0;
    ULONG Cr2;
    ULONG Cr3;
    ULONG Cr4;
    ULONG KernelDr0;
    ULONG KernelDr1;
    ULONG KernelDr2;
    ULONG KernelDr3;
    ULONG KernelDr6;
    ULONG KernelDr7;
    KDESCRIPTOR Gdtr;
    KDESCRIPTOR Idtr;
    USHORT Tr;
    USHORT Ldtr;
    ULONG64 Xcr0;
    ULONG ExceptionList;
    ULONG Reserved[3];
} X86_KSPECIAL_REGISTERS, *PX86_KSPECIAL_REGISTERS;

#pragma pack(pop)

typedef struct _AMD64_KSPECIAL_REGISTERS {
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
} AMD64_KSPECIAL_REGISTERS, *PAMD64_KSPECIAL_REGISTERS;

//
// Context Frame
//
//  This frame has a several purposes: 1) it is used as an argument to
//  NtContinue, 2) is is used to constuct a call frame for APC delivery,
//  and 3) it is used in the user level thread creation routines.
//
//  The layout of the record conforms to a standard call frame.
//

typedef struct DECLSPEC_ALIGN(16) _X86_NT5_CONTEXT {

    //
    // The flags values within this flag control the contents of
    // a CONTEXT record.
    //
    // If the context record is used as an input parameter, then
    // for each portion of the context record controlled by a flag
    // whose value is set, it is assumed that that portion of the
    // context record contains valid context. If the context record
    // is being used to modify a threads context, then only that
    // portion of the threads context will be modified.
    //
    // If the context record is used as an IN OUT parameter to capture
    // the context of a thread, then only those portions of the thread's
    // context corresponding to set flags will be returned.
    //
    // The context record is never used as an OUT only parameter.
    //

    ULONG ContextFlags;

    //
    // This section is specified/returned if CONTEXT_DEBUG_REGISTERS is
    // set in ContextFlags.  Note that CONTEXT_DEBUG_REGISTERS is NOT
    // included in CONTEXT_FULL.
    //

    ULONG   Dr0;
    ULONG   Dr1;
    ULONG   Dr2;
    ULONG   Dr3;
    ULONG   Dr6;
    ULONG   Dr7;

    //
    // This section is specified/returned if the
    // ContextFlags word contians the flag CONTEXT_FLOATING_POINT.
    //

    X86_FLOATING_SAVE_AREA FloatSave;

    //
    // This section is specified/returned if the
    // ContextFlags word contians the flag CONTEXT_SEGMENTS.
    //

    ULONG   SegGs;
    ULONG   SegFs;
    ULONG   SegEs;
    ULONG   SegDs;

    //
    // This section is specified/returned if the
    // ContextFlags word contians the flag CONTEXT_INTEGER.
    //

    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;

    //
    // This section is specified/returned if the
    // ContextFlags word contians the flag CONTEXT_CONTROL.
    //

    ULONG   Ebp;
    ULONG   Eip;
    ULONG   SegCs;              // MUST BE SANITIZED
    ULONG   EFlags;             // MUST BE SANITIZED
    ULONG   Esp;
    ULONG   SegSs;

    //
    // This section is specified/returned if the ContextFlags word
    // contains the flag CONTEXT_EXTENDED_REGISTERS.
    // The format and contexts are processor specific
    //

    UCHAR   ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];

} X86_NT5_CONTEXT;

#define ARM64_MAX_BREAKPOINTS     8
#define ARM64_MAX_WATCHPOINTS     2

typedef struct _ARM64_KSPECIAL_REGISTERS {

    ULONG64 Elr_El1;
    ULONG32 Spsr_El1;
    ULONG64 Tpidr_El0;
    ULONG64 Tpidrro_El0;
    ULONG64 Tpidr_El1;

    //
    // H/w [break/watch]point support.
    //

    ULONG64 KernelBvr[ARM64_MAX_BREAKPOINTS];
    ULONG KernelBcr[ARM64_MAX_BREAKPOINTS];
    ULONG64 KernelWvr[ARM64_MAX_WATCHPOINTS];
    ULONG KernelWcr[ARM64_MAX_WATCHPOINTS];

} ARM64_KSPECIAL_REGISTERS, *PARM64_KSPECIAL_REGISTERS;

#define MAX_EVENT_COUNTERS 31

typedef struct _KARM64_ARCH_STATE
{
    ULONG64 Midr_El1;
    ULONG64 Sctlr_El1;
    ULONG64 Actlr_El1;
    ULONG64 Cpacr_El1;
    ULONG64 Tcr_El1;
    ULONG64 Ttbr0_El1;
    ULONG64 Ttbr1_El1;
    ULONG64 Esr_El1;
    ULONG64 Far_El1;
    ULONG64 Pmcr_El0;
    ULONG64 Pmcntenset_El0;
    ULONG64 Pmccntr_El0;
    ULONG64 Pmxevcntr_El0[MAX_EVENT_COUNTERS];
    ULONG64 Pmxevtyper_El0[MAX_EVENT_COUNTERS];
    ULONG64 Pmovsclr_El0;
    ULONG64 Pmselr_El0;
    ULONG64 Pmuserenr_El0;
    ULONG64 Mair_El1;
    ULONG64 Vbar_El1;
} KARM64_ARCH_STATE, *PKARM64_ARCH_STATE;

typedef struct _ARM64_KSWITCH_FRAME {
    KIRQL ApcBypass;
    UCHAR Fill[15];
    ULONG64 Fp;
    ULONG64 Return;
} ARM64_KSWITCH_FRAME, *PARM64_KSWITCH_FRAME;

typedef union _ARM64_NEON128 {
    struct {
        ULONGLONG Low;
        LONGLONG High;
    } DUMMYSTRUCTNAME;
    double D[2];
    float S[4];
    USHORT H[8];
    UCHAR B[16];
} ARM64_NEON128, *PARM64_NEON128;

typedef struct _ARM64_CONTEXT {

    //
    // Control flags.
    //

    /* +0x000 */ ULONG ContextFlags;

    //
    // Integer registers
    //

    /* +0x004 */ ULONG Cpsr;       // NZVF + DAIF + CurrentEL + SPSel
    /* +0x008 */ ULONG64 X[29];
    /* +0x0f0 */ ULONG64 Fp;
    /* +0x0f8 */ ULONG64 Lr;
    /* +0x100 */ ULONG64 Sp;
    /* +0x108 */ ULONG64 Pc;

    //
    // Floating Point/NEON Registers
    //

    /* +0x110 */ ARM64_NEON128 V[32];
    /* +0x310 */ ULONG Fpcr;
    /* +0x314 */ ULONG Fpsr;

    //
    // Debug registers
    //

    /* +0x318 */ ULONG Bcr[ARM64_MAX_BREAKPOINTS];
    /* +0x338 */ ULONG64 Bvr[ARM64_MAX_BREAKPOINTS];
    /* +0x378 */ ULONG Wcr[ARM64_MAX_WATCHPOINTS];
    /* +0x380 */ ULONG64 Wvr[ARM64_MAX_WATCHPOINTS];
    /* +0x390 */

} ARM64_CONTEXT, *PARM64_CONTEXT;


typedef struct _X86_KPROCESSOR_STATE {
    X86_KSPECIAL_REGISTERS SpecialRegisters;
    X86_NT5_CONTEXT ContextFrame;
} X86_KPROCESSOR_STATE, *PX86_KPROCESSOR_STATE;

typedef struct _AMD64_KPROCESSOR_STATE {
    AMD64_KSPECIAL_REGISTERS SpecialRegisters;
    AMD64_CONTEXT ContextFrame;
} AMD64_KPROCESSOR_STATE, *PAMD64_KPROCESSOR_STATE;

typedef struct _ARM64_KPROCESSOR_STATE {
    ARM64_KSPECIAL_REGISTERS SpecialRegisters;
    KARM64_ARCH_STATE ArchState;
    ARM64_CONTEXT ContextFrame;
} ARM64_KPROCESSOR_STATE, *PARM64_KPROCESSOR_STATE;


typedef struct _ARM64_VFP_STATE
{
    struct _ARM64_VFP_STATE *Link;          // link to next state entry
    ULONG Fpcr;                             // FPCR register
    ULONG Fpsr;                             // FPSR register
    ARM64_NEON128 V[32];                    // All V registers (0-31)
} ARM64_VFP_STATE, *PARM64_VFP_STATE;

#define ARM64_KTRAP_FRAME_ARGUMENTS (10 * 8)       // up to 16 in-memory syscall args


#if defined(_AMD64_)

typedef AMD64_CONTEXT CONTEXT, *PCONTEXT;

typedef AMD64_KPROCESSOR_STATE KPROCESSOR_STATE, *PKPROCESSOR_STATE;

#elif defined(_ARM64_)

typedef ARM64_CONTEXT CONTEXT, *PCONTEXT;

typedef ARM64_KSPECIAL_REGISTERS KSPECIAL_REGISTERS, *PKSPECIAL_REGISTERS;

typedef ARM64_KPROCESSOR_STATE KPROCESSOR_STATE, *PKPROCESSOR_STATE;

//
// CPSR mode constants
//

#define CPSRM_EL3h 0xd
#define CPSRM_EL3t 0xc
#define CPSRM_EL2h 0x9
#define CPSRM_EL2t 0x8
#define CPSRM_EL1h 0x5
#define CPSRM_EL1t 0x4
#define CPSRM_EL0t 0x0
#define CPSRM_MASK 0x0f

//
// CPSR EL constants
//

#define CPSREL_3 0xc
#define CPSREL_2 0x8
#define CPSREL_1 0x4
#define CPSREL_0 0x0
#define CPSREL_MASK 0x0c

//
// DAIF enable/disables
//

#define DAIF_DEBUG 0x200
#define DAIF_ABORT 0x100
#define DAIF_INT   0x80
#define DAIF_FIQ   0x40

//
// NZCV flags
//

#define NZCV_N 0x80000000
#define NZCV_Z 0x40000000
#define NZCV_C 0x20000000
#define NZCV_V 0x10000000

//
// New ARM64 CSPR flags
//

#define CPSRM_T 0x00000020
#define CPSRM_AA32 0x00000010
#define CPSR_IL 0x00100000
#define CPSR_SS 0x00200000
#define CPSR_IT_MASK 0x0600fc00

//
// FPCR modes
//

#define FPCRM_AHP        0x04000000
#define FPCRM_DN         0x02000000
#define FPCRM_FZ         0x01000000

#define FPCRM_RMODE_MASK 0x00c00000
#define FPCRM_RMODE_RN   0x00000000
#define FPCRM_RMODE_RP   0x00400000
#define FPCRM_RMODE_RM   0x00800000
#define FPCRM_RMODE_RZ   0x00c00000

//
// FPCR exception controls
//

#define FPCR_IDE 0x00008000
#define FPCR_IXE 0x00001000
#define FPCR_UFE 0x00000800
#define FPCR_OFE 0x00000400
#define FPCR_DZE 0x00000200
#define FPCR_IOE 0x00000100

//
// FPSR exception states
//

#define FPSR_IDC 0x00000080
#define FPSR_IXC 0x00000010
#define FPSR_UFC 0x00000008
#define FPSR_OFC 0x00000004
#define FPSR_DZC 0x00000002
#define FPSR_IOC 0x00000001

//
// ARMv8 Status Registers
//
#define ARM64_SYSREG(op0, op1, crn, crm, op2) \
        ( ((op0 & 1) << 14) | \
          ((op1 & 7) << 11) | \
          ((crn & 15) << 7) | \
          ((crm & 15) << 3) | \
          ((op2 & 7) << 0) )

//
// Core registers
//

#define ARM64_TPIDR_EL1         ARM64_SYSREG(3,0,13, 0,4)  // Thread ID Register, Privileged Only [CP15_TPIDRPRW]
#define ARM64_TPIDR_EL2         ARM64_SYSREG(3,4,13, 0,2)  // Thread ID Register, Hypervisor Only [CP15_HTPIDR]

#define ARM64_SPSR_EL1          ARM64_SYSREG(3,0, 4, 0,0)   // Saved processor status register (EL1)
#define ARM64_SPSR_EL2          ARM64_SYSREG(3,4, 4, 0,0)   // Saved processor status register (EL2)
#define ARM64_ELR_EL1           ARM64_SYSREG(3,0, 4, 0,1)   // Exception return address (EL1)
#define ARM64_ELR_EL2           ARM64_SYSREG(3,4, 4, 0,1)   // Exception return address (EL2)
#define ARM64_SP_EL0            ARM64_SYSREG(3,0, 4, 1,0)   // User mode stack pointer (EL1)
#define ARM64_SP_EL1            ARM64_SYSREG(3,4, 4, 1,0)   // Kernel mode stack pointer (EL1)
#define ARM64_SP_EL2            ARM64_SYSREG(3,6, 4, 1,0)   // Hypervisor mode stack pointer (EL2)
#define ARM64_SPSel             ARM64_SYSREG(3,0, 4, 2,0)   // SP select (EL1)
#define ARM64_DAIF              ARM64_SYSREG(3,3, 4, 2,1)   // Interrupt Masks (EL0)
#define ARM64_CurrentEL         ARM64_SYSREG(3,0, 4, 2,2)   // Current Exception Level (ReadOnly, EL1)
#define ARM64_NZCV              ARM64_SYSREG(3,3, 4, 2,0)   // Flags (EL0)
#define ARM64_FPCR              ARM64_SYSREG(3,3, 4, 4,0)   // Floating point control register (EL0)
#define ARM64_FPSR              ARM64_SYSREG(3,3, 4, 4,1)   // Floating point status register (EL0)
#define ARM64_DSPSR             ARM64_SYSREG(3,3, 4, 5,0)   // ??? (EL0)
#define ARM64_DLR               ARM64_SYSREG(3,3, 4, 5,1)   // ??? (EL0)

//
// ID and feature registers
//

#define ARM64_MIDR_EL1          ARM64_SYSREG(3,0, 0, 0,0)   // Main ID Register [CP15_MIDR]
#define ARM64_VPIDR_EL2         ARM64_SYSREG(3,4, 0, 0,0)   // Virtualized Main ID Register [CP15_VPIDR]
#define ARM64_REVIDR_EL1        ARM64_SYSREG(3,0, 0, 0,6)   // Revision ID Register [CP15_REVIDR]
#define ARM64_CTR_EL0           ARM64_SYSREG(3,3, 0, 0,1)   // Cache Type Register [CP15_CTR]
#define ARM64_MPIDR_EL1         ARM64_SYSREG(3,0, 0, 0,5)   // Multiprocessor Affinity Register [CP15_MPIDR]
#define ARM64_VMPIDR_EL2        ARM64_SYSREG(3,4, 0, 0,5)   // Virtualized Multiprocessor Affinity Register [CP15_VMPIDR]
#define ARM64_ID_AA64PFR0_EL1   ARM64_SYSREG(3,0, 0, 4,0)   // Processor Feature Register 0
#define ARM64_ID_AA64PFR1_EL1   ARM64_SYSREG(3,0, 0, 4,1)   // Processor Feature Register 1
#define ARM64_ID_AA64DFR0_EL1   ARM64_SYSREG(3,0, 0, 5,0)   // Debug Feature Register 0
#define ARM64_ID_AA64DFR1_EL1   ARM64_SYSREG(3,0, 0, 5,1)   // Debug Feature Register 1
#define ARM64_ID_AA64AFR0_EL1   ARM64_SYSREG(3,0, 0, 5,4)   //
#define ARM64_ID_AA64AFR1_EL1   ARM64_SYSREG(3,0, 0, 5,5)   //
#define ARM64_ID_AA64ISAR0_EL1  ARM64_SYSREG(3,0, 0, 6,0)   // ISA Feature Register 0
#define ARM64_ID_AA64ISAR1_EL1  ARM64_SYSREG(3,0, 0, 6,1)   // ISA Feature Register 1
#define ARM64_ID_AA64MMFR0_EL1  ARM64_SYSREG(3,0, 0, 7,0)   // Memory Management Feature Register 0
#define ARM64_ID_AA64MMFR1_EL1  ARM64_SYSREG(3,0, 0, 7,1)   // Memory Management Feature Register 1

//
// System control registers
//

#define ARM64_SCTLR_EL1         ARM64_SYSREG(3,0, 1, 0,0)   // System Control Register [CP15_SCTLR]
#define ARM64_SCTLR_EL2         ARM64_SYSREG(3,4, 1, 0,0)   // System Control Register [CP15_HSCTLR]
#define ARM64_HCR_EL2           ARM64_SYSREG(3,4, 1, 1,0)   // Hypervisor Configuration Register (EL2)
#define ARM64_HSTR_EL2          ARM64_SYSREG(3,4, 1, 1,3)   // Hypervisor System Trap Configuration Register (EL2)
#define ARM64_HACR_EL2          ARM64_SYSREG(3,4, 1, 1,7)   // Hypervisor Auxiliary Configuration Register (EL2)
#define ARM64_ACTLR_EL1         ARM64_SYSREG(3,0, 1, 0,1)   // Auxiliary Control Register [CP15_ACTLR]
#define ARM64_ACTLR_EL2         ARM64_SYSREG(3,4, 1, 0,1)   // Auxiliary Control Register [CP15_HACTLR]
#define ARM64_CPACR_EL1         ARM64_SYSREG(3,0, 1, 0,2)   // Coprocessor Access Control Register [CP15_CPACR]
#define ARM64_SCR_EL3           ARM64_SYSREG(3,6, 1, 1,0)   // Secure Configuration Register [CP15_SCR]
#define ARM64_CPTR_EL3          ARM64_SYSREG(3,6, 1, 1,2)   // Non-Secure Access Control Register [CP15_NSACR]
#define ARM64_CPTR_EL2          ARM64_SYSREG(3,4, 1, 1,2)   // Hypervisor Access Control Register [CP15_HCPTR]

//
// Memory protection and control registers
//

#define ARM64_TTBR0_EL1         ARM64_SYSREG(3,0, 2, 0,0)   // Translation Table Base Register 0 [CP15_TTBR0]
#define ARM64_TTBR0_EL2         ARM64_SYSREG(3,4, 2, 0,0)   // Translation Table Base Register 0 [CP15_HTTBR]
#define ARM64_TTBR1_EL1         ARM64_SYSREG(3,0, 2, 0,1)   // Translation Table Base Register 1 [CP15_TTBR1]
#define ARM64_VTTBR_EL2         ARM64_SYSREG(3,4, 2, 1,0)   // Virtualization Translation Table Base Register 1 [CP15_VTTBR]
#define ARM64_TCR_EL1           ARM64_SYSREG(3,0, 2, 0,2)   // Translation Control Register [CP15_TTBCR]
#define ARM64_TCR_EL2           ARM64_SYSREG(3,4, 2, 0,2)   // Translation Control Register [CP15_HTCR]
#define ARM64_VTCR_EL2          ARM64_SYSREG(3,4, 2, 1,2)   // Virtualization Translation Control Register [CP15_VTCR]
#define ARM64_ESR_EL1           ARM64_SYSREG(3,0, 5, 2,0)   // Exception Status Register [CP15_DFSR/CP15_IFSR]
#define ARM64_ESR_EL2           ARM64_SYSREG(3,4, 5, 2,0)   // Exception Status Register [CP15_HSR]
#define ARM64_FAR_EL1           ARM64_SYSREG(3,0, 6, 0,0)   // Fault Address Registers [CP15_DFAR/CP15_IFAR]
#define ARM64_FAR_EL2           ARM64_SYSREG(3,4, 6, 0,0)   // Fault Address Registers [CP15_HDFAR/CP15_HIFAR]
#define ARM64_HPFAR_EL2         ARM64_SYSREG(3,4, 6, 0,4)   // Hypervisor IPA Fault Address Registers [CP15_HPFAR]
#define ARM64_AFSR0_EL1         ARM64_SYSREG(3,0, 5, 1,0)   // Auxiliary Fault Status Register 0 [CP15_ADFSR]
#define ARM64_AFSR0_EL2         ARM64_SYSREG(3,4, 5, 1,0)   // Auxiliary Fault Status Register 0 [CP15_HDFSR]
#define ARM64_AFSR1_EL1         ARM64_SYSREG(3,0, 5, 1,1)   // Auxiliary Fault Status Register 1 [CP15_AIFSR]
#define ARM64_AFSR1_EL2         ARM64_SYSREG(3,4, 5, 1,1)   // Auxiliary Fault Status Register 1 [CP15_HAIFSR]

//
// ARM Cache Operations
//

#define ARM64_DCZID_EL0         ARM64_SYSREG(3,3, 0, 0,7)   // Data Cache Zero ID Register

// IC opcodes, or use with SYS
#define ARM64_IC_IALLUIS        ARM64_SYSINSTR(1,0, 7, 1,0) // Invalidate all instruction caches to PoU Inner Shareable [CP15_ICIALLUIS]
#define ARM64_IC_IALLU          ARM64_SYSINSTR(1,0, 7, 5,0) // Invalidate all instruction caches to PoU [CP15_ICIALLU]
#define ARM64_IC_IVAU           ARM64_SYSINSTR(1,3, 7, 5,1) // Invalidate all instruction caches by MVA to PoU [CP15_ICIMVAU]

// DC opcodes, or use with SYS
#define ARM64_DC_ZVA            ARM64_SYSINSTR(1,3, 7, 4,1) // Zero cache line at VA
#define ARM64_DC_IVAC           ARM64_SYSINSTR(1,0, 7, 6,1) // Invalidate data cache line by VA to PoC [CP15_DCIMVAC]
#define ARM64_DC_ISW            ARM64_SYSINSTR(1,0, 7, 6,2) // Invalidate data cache line by set/way [CP15_DCISW]
#define ARM64_DC_CVAC           ARM64_SYSINSTR(1,3, 7,10,1) // Clean data cache line by VA to PoC [CP15_DCCMVAC]
#define ARM64_DC_CSW            ARM64_SYSINSTR(1,0, 7,10,2) // Clean data cache line by set/way [CP15_DCCSW]
#define ARM64_DC_CVAU           ARM64_SYSINSTR(1,3, 7,11,1) // Clean data cache line by VA to PoU [CP15_DCCMVAU]
#define ARM64_DC_CIVAC          ARM64_SYSINSTR(1,3, 7,14,1) // Clean and invalidate data cache line by VA to PoC [CP15_DCCIMVAC]
#define ARM64_DC_CISW           ARM64_SYSINSTR(1,0, 7,14,2) // Clean and invalidate data cache line by set/way [CP15_DCCISW]

//
// ARM Translation Operations
//

#define ARM64_PAR_EL1           ARM64_SYSREG(3,0, 7, 4,0)   // Physical Address Register (Translation Result) [CP15_PAR]

// AT opcodes, or use with SYS
#define ARM64_AT_S1E2R          ARM64_SYSINSTR(1,4, 7, 8,0) // Translate Stage1, EL2, read
#define ARM64_AT_S1E2W          ARM64_SYSINSTR(1,4, 7, 8,1) // Translate Stage1, EL2, write
#define ARM64_AT_S1E1R          ARM64_SYSINSTR(1,0, 7, 8,0) // Translate Stage1, EL1, read
#define ARM64_AT_S1E1W          ARM64_SYSINSTR(1,0, 7, 8,1) // Translate Stage1, EL1, write
#define ARM64_AT_S1E0R          ARM64_SYSINSTR(1,0, 7, 8,2) // Translate Stage1, EL0, read
#define ARM64_AT_S1E0W          ARM64_SYSINSTR(1,0, 7, 8,3) // Translate Stage1, EL0, write
#define ARM64_AT_S12E1R         ARM64_SYSINSTR(1,4, 7, 8,4) // Translate Stage12, EL1, read
#define ARM64_AT_S12E1W         ARM64_SYSINSTR(1,4, 7, 8,5) // Translate Stage12, EL1, write
#define ARM64_AT_S12E0R         ARM64_SYSINSTR(1,4, 7, 8,6) // Translate Stage12, EL0, read
#define ARM64_AT_S12E0W         ARM64_SYSINSTR(1,4, 7, 8,7) // Translate Stage12, EL0, write

//
// ARM Generic Timer registers
//

#define ARM64_CNTFRQ_EL0        ARM64_SYSREG(3,3,14, 0,0)   // Holds the clock frequency of the system counter
#define ARM64_CNTPCT_EL0        ARM64_SYSREG(3,3,14, 0,1)   // Holds the 64bit physical count value
#define ARM64_CNTVCT_EL0        ARM64_SYSREG(3,3,14, 0,2)   // Holds the 64bit virtual count value
#define ARM64_CNTV_OFF_EL2      ARM64_SYSREG(3,4,14, 0,3)   // Holds the 64bit virtual count offset value
#define ARM64_CNTP_TVAL_EL0     ARM64_SYSREG(3,3,14, 2,0)   // Holds the timer value for the El1 physical timer
#define ARM64_CNTP_CTL_EL0      ARM64_SYSREG(3,3,14, 2,1)   // Control register for the El1 physical timer
#define ARM64_CNTP_CVAL_EL0     ARM64_SYSREG(3,3,14, 2,2)   // Holds the compare value for the El1 physical timer
#define ARM64_CNTV_TVAL_EL0     ARM64_SYSREG(3,3,14, 3,0)   // Holds the timer value for the virtual timer
#define ARM64_CNTV_CTL_EL0      ARM64_SYSREG(3,3,14, 3,1)   // Control register for the virtual timer
#define ARM64_CNTV_CVAL_EL0     ARM64_SYSREG(3,3,14, 3,2)   // Holds the compare value for the virtual timer
#define ARM64_CNTK_CTL_EL1      ARM64_SYSREG(3,0,14, 1,0)   // Controls the generation of an event stream from the virtual counter, and access from EL0 to the physical counter, virtual counter, EL1 physical timers, and the virtual timer.
#define ARM64_CNT_HCTL_EL2      ARM64_SYSREG(3,4,14, 1,0)   // Counter-timer Hypervisor Control register
#define ARM64_CNTHP_TVAL_EL2    ARM64_SYSREG(3,4,14, 2,0)   // Counter-timer Hypervisor Physical Timer TimerValue register
#define ARM64_CNTHP_CTL_EL2     ARM64_SYSREG(3,4,14, 2,1)   // Counter-timer Hypervisor Physical Timer Control register
#define ARM64_CNTHP_CVAL_EL2    ARM64_SYSREG(3,4,14, 2,2)   // 64-bit, Counter-timer Hypervisor Physical Timer CompareValue register

#define ARM64_CNT_HCTL_EL1PCTEN    0x00000001
#define ARM64_CNT_HCTL_EL1PCEN     0x00000002
#define ARM64_CNT_HCTL_EVNTEN      0x00000004
#define ARM64_CNT_HCTL_EVNTDIR     0x00000008
#define ARM64_CNT_HCTL_EVNTI_MASK  0x000000F0
#define ARM64_CNT_HCTL_EVNTI_SHIFT 4

#define ARM64_CNT_CTL_ENABLE      0x00000001
#define ARM64_CNT_CTL_IMASK       0x00000002
#define ARM64_CNT_CTL_ISTATUS     0x00000004

#define ARM64_CNTK_CTL_EL0PCTEN   0x00000001 // Controls whether the physical counter, CNTPCT_EL0, and the frequency register CNTFRQ_EL0, are accessible from EL0 modes
#define ARM64_CNTK_CTL_EL0VCTEN   0x00000002 // Controls whether the virtual counter, CNTVCT_EL0, and the frequency register CNTFRQ_EL0, are accessible from EL0 modes
#define ARM64_CNTK_CTL_EVNTEN     0x00000004 // Enables the generation of an event stream from the corresponding counter
#define ARM64_CNTK_CTL_EVNTDIR    0x00000008 // Controls which transition of the counter register (CNTPCT_EL0 or CNTVCT_EL0) trigger bit, defined by EVNTI, generates an event when the event stream is enabled
#define ARM64_CNTK_CTL_EVNTI_MASK 0x000000F0 // Selects which bit (0 to 15) of the corresponding counter register (CNTPCT_EL0 or CNTVCT_EL0) is the trigger for the event stream generated from that counter, when that stream is enabled.
#define ARM64_CNTK_CTL_EL0VTEN    0x00000100 // Controls whether the virtual timer registers are accessible from EL0 modes
#define ARM64_CNTK_CTL_EL0PTEN    0x00000200 // Controls whether the physical timer registers are accessible from EL0 modes

//
// Structure describing the output retrieved from the ARM64_PAR_EL1 register.
//

typedef union _ARM64_PAR {
    ULONG64 Ulong;
    struct {
        ULONG64 Fault         :  1;
        ULONG64 FaultStatus   :  6;
        ULONG64 Shareability  :  2;
        ULONG64 Stage         :  1;
        ULONG64 ImplDefined   :  1;
        ULONG64 Lpae          :  1;
        ULONG64 Pfn           : 36;
        ULONG64 Res0          :  8;
        ULONG64 Mair          :  8;
    };
} ARM64_PAR;

//
// TLB maintenance operations
//

// TLBI opcodes, or use with SYS
#define ARM64_TLBI_VMALLE1      ARM64_SYSINSTR(1,0, 8, 7,0) // Invalidate stage 1 TLB [CP15_TLBIALL]
#define ARM64_TLBI_VAE1         ARM64_SYSINSTR(1,0, 8, 7,1) // Invalidate stage 1 TLB entry by VA and ASID [CP15_TLBIMVA]
#define ARM64_TLBI_ASIDE1       ARM64_SYSINSTR(1,0, 8, 7,2) // Invalidate stage 1 TLB by ASID match [CP15_TLBIASID]
#define ARM64_TLBI_VAAE1        ARM64_SYSINSTR(1,0, 8, 7,3) // Invalidate stage 1 TLB entries by VA all ASID [CP15_TLBIMVAA]
#define ARM64_TLBI_VALE1        ARM64_SYSINSTR(1,0, 8, 7,5) // Invalidate stage 1 TLB entry last level by VA and ASID
#define ARM64_TLBI_VAALE1       ARM64_SYSINSTR(1,0, 8, 7,7) // Invalidate stage 1 TLB entries last level by VA all ASID
#define ARM64_TLBI_VMALLE1IS    ARM64_SYSINSTR(1,0, 8, 3,0) // Invalidate stage 1 TLB Inner Shareable [CP15_TLBIALLIS]
#define ARM64_TLBI_VAE1IS       ARM64_SYSINSTR(1,0, 8, 3,1) // Invalidate stage 1 TLB entry by VA and ASID Inner Shareable [CP15_TLBIMVAIS]
#define ARM64_TLBI_ASIDE1IS     ARM64_SYSINSTR(1,0, 8, 3,2) // Invalidate stage 1 TLB by ASID match Inner Shareable [CP15_TLBIASIDIS]
#define ARM64_TLBI_VAAE1IS      ARM64_SYSINSTR(1,0, 8, 3,3) // Invalidate stage 1 TLB entries by VA all ASID Inner Shareable [CP15_TLBIMVAAIS]
#define ARM64_TLBI_VALE1IS      ARM64_SYSINSTR(1,0, 8, 3,5) // Invalidate stage 1 TLB entry last level by VA and ASID Inner Shareable
#define ARM64_TLBI_VAALE1IS     ARM64_SYSINSTR(1,0, 8, 3,7) // Invalidate stage 1 TLB entries last level by VA all ASID Inner Shareable
#define ARM64_TLBI_ALLE1        ARM64_SYSINSTR(1,4, 8, 7,4) // Invalidate stage 1 and 2 TLB all VMIDs
#define ARM64_TLBI_VMALLS12E1   ARM64_SYSINSTR(1,4, 8, 7,6) // Invalidate stage 1 and 2 TLB
#define ARM64_TLBI_IPAS2E1      ARM64_SYSINSTR(1,4, 8, 4,1) // Invalidate stage 2 TLB entry by IPA
#define ARM64_TLBI_IPAS2LE1     ARM64_SYSINSTR(1,4, 8, 4,5) // Invalidate stage 2 TLB entry last level by IPA
#define ARM64_TLBI_ALLE1IS      ARM64_SYSINSTR(1,4, 8, 3,4) // Invalidate stage 1 and 2 TLB all VMIDs Inner Shareable
#define ARM64_TLBI_VMALLS12E1IS ARM64_SYSINSTR(1,4, 8, 3,6) // Invalidate stage 1 and 2 TLB Inner Shareable
#define ARM64_TLBI_IPAS2E1IS    ARM64_SYSINSTR(1,4, 8, 0,1) // Invalidate stage 2 TLB entry by IPA Inner Shareable
#define ARM64_TLBI_IPAS2LE1IS   ARM64_SYSINSTR(1,4, 8, 0,5) // Invalidate stage 2 TLB entry last level by IPA Inner Shareable
#define ARM64_TLBI_ALLE2        ARM64_SYSINSTR(1,4, 8, 7,0) // Invalidate EL2 TLB
#define ARM64_TLBI_VAE2         ARM64_SYSINSTR(1,4, 8, 7,1) // Invalidate EL2 TLB entry by VA
#define ARM64_TLBI_VALE2        ARM64_SYSINSTR(1,4, 8, 7,5) // Invalidate EL2 TLB entry last level by VA
#define ARM64_TLBI_ALLE2IS      ARM64_SYSINSTR(1,4, 8, 3,0) // Invalidate EL2 TLB Inner Shareable
#define ARM64_TLBI_VAE2IS       ARM64_SYSINSTR(1,4, 8, 3,1) // Invalidate EL2 TLB entry by VA Inner Shareable
#define ARM64_TLBI_VALE2IS      ARM64_SYSINSTR(1,4, 8, 3,5) // Invalidate EL2 TLB entry last level by VA Inner Shareable

//
// TLB maintenance definitions and masks
//

#define ARM64_TLBI_ASID_SHIFT   48
#define ARM64_TLBI_VA_MASK      0x00fffffffffff000ULL

//
// Performance counter registers
//

#define ARM64_PMCCFILTR_EL0     ARM64_SYSREG(3,3,14,15,7)  // Performance Monitors Cycle Count Filter Register [CP15_PMCCFILTR]
#define ARM64_MDCR_EL2          ARM64_SYSREG(3,4, 1, 1,1)  // Monitor Debug Configuration Register (EL2) [CP15_HDCR]
#define ARM64_PMCR_EL0          ARM64_SYSREG(3,3, 9,12,0)  // Performance Monitor Control Register [CP15_PMCR]
#define ARM64_PMCNTENSET_EL0    ARM64_SYSREG(3,3, 9,12,1)  // Count Enable Set Register [CP15_PMCNTENSET]
#define ARM64_PMCNTENCLR_EL0    ARM64_SYSREG(3,3, 9,12,2)  // Count Enable Clear Register [CP15_PMCNTENCLR]
#define ARM64_PMOVSCLR_EL0      ARM64_SYSREG(3,3, 9,12,3)  // Overflow Flag Status Register [CP15_PMOVSR]
#define ARM64_PMSWINC_EL0       ARM64_SYSREG(3,3, 9,12,4)  // Software Increment Register [CP15_PSWINC]
#define ARM64_PMSELR_EL0        ARM64_SYSREG(3,3, 9,12,5)  // Event Counter Selection Register [CP15_PMSELR]
#define ARM64_PMCCNTR_EL0       ARM64_SYSREG(3,3, 9,13,0)  // Cycle Count Register [CP15_PMCCNTR]
#define ARM64_PMXEVTYPER_EL0    ARM64_SYSREG(3,3, 9,13,1)  // Event Type Select Register [CP15_PMXEVTYPER]
#define ARM64_PMXEVTYPERn_EL0(n) ARM64_SYSREG(3,3,14, 12+((n)/8), (n)%8)    // Direct Event Type Select Register [n/a]
#define ARM64_PMXEVCNTR_EL0     ARM64_SYSREG(3,3, 9,13,2)  // Event Count Register [CP15_PMXEVCNTR]
#define ARM64_PMXEVCNTRn_EL0(n) ARM64_SYSREG(3,3,14, 8+((n)/8), (n)%8)    // Direct Event Count Register [n/a]
#define ARM64_PMUSERENR_EL0     ARM64_SYSREG(3,3, 9,14,0)  // User Enable Register [CP15_PMUSERENR]
#define ARM64_PMINTENSET_EL1    ARM64_SYSREG(3,0, 9,14,1)  // Interrupt Enable Set Register [CP15_PMINTENSET]
#define ARM64_PMINTENCLR_EL1    ARM64_SYSREG(3,0, 9,14,2)  // Interrupt Enable Clear Register [CP15_PMINTENCLR]

//
// Memory remap registers
//

#define ARM64_MAIR_EL1          ARM64_SYSREG(3,0,10, 2,0)  // Primary Region Remap Register [CP15_PRRR/CP15_NMRR]
#define ARM64_AMAIR_EL1         ARM64_SYSREG(3,0,10, 3,0)  // Auxiliary Region Remap Register [CP15_AMAIR0/CP15_AMAIR1]
#define ARM64_MAIR_EL2          ARM64_SYSREG(3,4,10, 2,0)  // Primary Region Remap Register [CP15_HMAIR0/CP15_HMAIR1]
#define ARM64_AMAIR_EL2         ARM64_SYSREG(3,4,10, 3,0)  // Auxiliary Region Remap Register [CP15_HAMAIR0/CP15_HAMAIR1]

//
// Security extensions registers
//

#define ARM64_VBAR_EL1          ARM64_SYSREG(3,0,12, 0,0)  // Vector Base Address Register [CP15_VBAR]
#define ARM64_VBAR_EL2          ARM64_SYSREG(3,4,12, 0,0)  // Vector Base Address Register [CP15_HVBAR]
#define ARM64_ISR_EL1           ARM64_SYSREG(3,0,12, 1,0)  // Interrupt Status Register [CP15_ISR]

//
// Process, context and thread ID registers
//

#define ARM64_TPIDR_EL0         ARM64_SYSREG(3,3,13, 0,2)  // Thread ID Register, User Read/Write [CP15_TPIDRURW]
#define ARM64_TPIDRRO_EL0       ARM64_SYSREG(3,3,13, 0,3)  // Thread ID Register, User Read Only [CP15_TPIDRURO]
#define ARM64_TPIDR_EL1         ARM64_SYSREG(3,0,13, 0,4)  // Thread ID Register, Privileged Only [CP15_TPIDRPRW]
#define ARM64_TPIDR_EL2         ARM64_SYSREG(3,4,13, 0,2)  // Thread ID Register, Hypervisor Only [CP15_HTPIDR]
#define ARM64_CONTEXTIDR_EL1    ARM64_SYSREG(3,0,13, 0,1)  // Context ID Register

//
// Cache information registers
//

#define ARM64_CCSIDR_EL1        ARM64_SYSREG(3,1, 0, 0,0)  // Cache Size ID Register [CP15_CCSIDR]
#define ARM64_CLIDR_EL1         ARM64_SYSREG(3,1, 0, 0,1)  // Cache Level ID Register [CP15_CLIDR]
#define ARM64_AIDR_EL1          ARM64_SYSREG(3,1, 0, 0,7)  // Auxiliary ID Register [CP15_AIDR]
#define ARM64_CSSELR_EL1        ARM64_SYSREG(3,2, 0, 0,0)  // Cache Size Selection Register [CP15_CSSELR]

//
// Generic Timer Registers
//

#define ARM64_CNTVCT            ARM64_SYSREG(3,3,14, 0,2)
#define ARM64_CNTKCTL           ARM64_SYSREG(3,0,14, 1,0)
#define ARM64_CNTV_CTL          ARM64_SYSREG(3,3,14, 3,1)
#define ARM64_CNTV_CVAL         ARM64_SYSREG(3,3,14, 3,2)

//
// CP14 debugging registers
//

#define ARM64_MDCCSR_EL0        ARM64_SYSREG(2,3, 0, 1,0)  // Debug Comms Channel Status Register (internal view) [CP14_DBGDSCRint]
#define ARM64_DBGDTRRX_EL0      ARM64_SYSREG(2,3, 0, 5,0)  // Host to Target Data Transfer Register (internal view) [CP14_DBGDTRRXint]
#define ARM64_DBGDTRTX_EL0      ARM64_DBGDTRRX_EL0         // Target to Host Data Transfer Register (internal view) [CP14_DBGDTRTXint]
#define ARM64_OSDTRRX_EL1       ARM64_SYSREG(2,0, 0, 0,2)  // Host to Target Data Transfer Register [CP14_DBGDTRRX]
#define ARM64_MDSCR_EL1         ARM64_SYSREG(2,0, 0, 2,2)  // Debug Status and Control Register (external view) [CP14_DBGDSCR]
#define ARM64_OSDTRTX_EL1       ARM64_SYSREG(2,0, 0, 3,2)  // Target to Host Data Transfer Register [CP14_DBGDTRTX]
#define ARM64_DBGBVR0_EL1       ARM64_SYSREG(2,0, 0, 0,4)  // Breakpoint Value Register 0 [CP14_DBGBVRn]
#define ARM64_DBGBVR1_EL1       ARM64_SYSREG(2,0, 0, 1,4)  // ...
#define ARM64_DBGBVR2_EL1       ARM64_SYSREG(2,0, 0, 2,4)  // ...
#define ARM64_DBGBVR3_EL1       ARM64_SYSREG(2,0, 0, 3,4)  // ...
#define ARM64_DBGBVR4_EL1       ARM64_SYSREG(2,0, 0, 4,4)  // ...
#define ARM64_DBGBVR5_EL1       ARM64_SYSREG(2,0, 0, 5,4)  // ...
#define ARM64_DBGBVR6_EL1       ARM64_SYSREG(2,0, 0, 6,4)  // ...
#define ARM64_DBGBVR7_EL1       ARM64_SYSREG(2,0, 0, 7,4)  // ...
#define ARM64_DBGBVR8_EL1       ARM64_SYSREG(2,0, 0, 8,4)  // ...
#define ARM64_DBGBVR9_EL1       ARM64_SYSREG(2,0, 0, 9,4)  // ...
#define ARM64_DBGBVR10_EL1      ARM64_SYSREG(2,0, 0,10,4)  // ...
#define ARM64_DBGBVR11_EL1      ARM64_SYSREG(2,0, 0,11,4)  // ...
#define ARM64_DBGBVR12_EL1      ARM64_SYSREG(2,0, 0,12,4)  // ...
#define ARM64_DBGBVR13_EL1      ARM64_SYSREG(2,0, 0,13,4)  // ...
#define ARM64_DBGBVR14_EL1      ARM64_SYSREG(2,0, 0,14,4)  // ...
#define ARM64_DBGBVR15_EL1      ARM64_SYSREG(2,0, 0,15,4)  // ...
#define ARM64_DBGBCR0_EL1       ARM64_SYSREG(2,0, 0, 0,5)  // Breakpoint Value Register 0 [CP14_DBGBCRn]
#define ARM64_DBGBCR1_EL1       ARM64_SYSREG(2,0, 0, 1,5)  // ...
#define ARM64_DBGBCR2_EL1       ARM64_SYSREG(2,0, 0, 2,5)  // ...
#define ARM64_DBGBCR3_EL1       ARM64_SYSREG(2,0, 0, 3,5)  // ...
#define ARM64_DBGBCR4_EL1       ARM64_SYSREG(2,0, 0, 4,5)  // ...
#define ARM64_DBGBCR5_EL1       ARM64_SYSREG(2,0, 0, 5,5)  // ...
#define ARM64_DBGBCR6_EL1       ARM64_SYSREG(2,0, 0, 6,5)  // ...
#define ARM64_DBGBCR7_EL1       ARM64_SYSREG(2,0, 0, 7,5)  // ...
#define ARM64_DBGBCR8_EL1       ARM64_SYSREG(2,0, 0, 8,5)  // ...
#define ARM64_DBGBCR9_EL1       ARM64_SYSREG(2,0, 0, 9,5)  // ...
#define ARM64_DBGBCR10_EL1      ARM64_SYSREG(2,0, 0,10,5)  // ...
#define ARM64_DBGBCR11_EL1      ARM64_SYSREG(2,0, 0,11,5)  // ...
#define ARM64_DBGBCR12_EL1      ARM64_SYSREG(2,0, 0,12,5)  // ...
#define ARM64_DBGBCR13_EL1      ARM64_SYSREG(2,0, 0,13,5)  // ...
#define ARM64_DBGBCR14_EL1      ARM64_SYSREG(2,0, 0,14,5)  // ...
#define ARM64_DBGBCR15_EL1      ARM64_SYSREG(2,0, 0,15,5)  // ...
#define ARM64_DBGWVR0_EL1       ARM64_SYSREG(2,0, 0, 0,6)  // Watchpoint Value Register 0 [CP14_DBGWVRn]
#define ARM64_DBGWVR1_EL1       ARM64_SYSREG(2,0, 0, 1,6)  // ...
#define ARM64_DBGWVR2_EL1       ARM64_SYSREG(2,0, 0, 2,6)  // ...
#define ARM64_DBGWVR3_EL1       ARM64_SYSREG(2,0, 0, 3,6)  // ...
#define ARM64_DBGWVR4_EL1       ARM64_SYSREG(2,0, 0, 4,6)  // ...
#define ARM64_DBGWVR5_EL1       ARM64_SYSREG(2,0, 0, 5,6)  // ...
#define ARM64_DBGWVR6_EL1       ARM64_SYSREG(2,0, 0, 6,6)  // ...
#define ARM64_DBGWVR7_EL1       ARM64_SYSREG(2,0, 0, 7,6)  // ...
#define ARM64_DBGWVR8_EL1       ARM64_SYSREG(2,0, 0, 8,6)  // ...
#define ARM64_DBGWVR9_EL1       ARM64_SYSREG(2,0, 0, 9,6)  // ...
#define ARM64_DBGWVR10_EL1      ARM64_SYSREG(2,0, 0,10,6)  // ...
#define ARM64_DBGWVR11_EL1      ARM64_SYSREG(2,0, 0,11,6)  // ...
#define ARM64_DBGWVR12_EL1      ARM64_SYSREG(2,0, 0,12,6)  // ...
#define ARM64_DBGWVR13_EL1      ARM64_SYSREG(2,0, 0,13,6)  // ...
#define ARM64_DBGWVR14_EL1      ARM64_SYSREG(2,0, 0,14,6)  // ...
#define ARM64_DBGWVR15_EL1      ARM64_SYSREG(2,0, 0,15,6)  // ...
#define ARM64_DBGWCR0_EL1       ARM64_SYSREG(2,0, 0, 0,7)  // Watchpoint Value Register 0 [CP14_DBGWCRn]
#define ARM64_DBGWCR1_EL1       ARM64_SYSREG(2,0, 0, 1,7)  // ...
#define ARM64_DBGWCR2_EL1       ARM64_SYSREG(2,0, 0, 2,7)  // ...
#define ARM64_DBGWCR3_EL1       ARM64_SYSREG(2,0, 0, 3,7)  // ...
#define ARM64_DBGWCR4_EL1       ARM64_SYSREG(2,0, 0, 4,7)  // ...
#define ARM64_DBGWCR5_EL1       ARM64_SYSREG(2,0, 0, 5,7)  // ...
#define ARM64_DBGWCR6_EL1       ARM64_SYSREG(2,0, 0, 6,7)  // ...
#define ARM64_DBGWCR7_EL1       ARM64_SYSREG(2,0, 0, 7,7)  // ...
#define ARM64_DBGWCR8_EL1       ARM64_SYSREG(2,0, 0, 8,7)  // ...
#define ARM64_DBGWCR9_EL1       ARM64_SYSREG(2,0, 0, 9,7)  // ...
#define ARM64_DBGWCR10_EL1      ARM64_SYSREG(2,0, 0,10,7)  // ...
#define ARM64_DBGWCR11_EL1      ARM64_SYSREG(2,0, 0,11,7)  // ...
#define ARM64_DBGWCR12_EL1      ARM64_SYSREG(2,0, 0,12,7)  // ...
#define ARM64_DBGWCR13_EL1      ARM64_SYSREG(2,0, 0,13,7)  // ...
#define ARM64_DBGWCR14_EL1      ARM64_SYSREG(2,0, 0,14,7)  // ...
#define ARM64_DBGWCR15_EL1      ARM64_SYSREG(2,0, 0,15,7)  // ...
#define ARM64_OSLAR_EL1         ARM64_SYSREG(2,0, 1, 0,4)  // OS Lock Access Register [CP14_DBGOSLAR]
#define ARM64_OSLSR_EL1         ARM64_SYSREG(2,0, 1, 1,4)  // OS Lock Status Register [CP14_DBGOSLSR]
#define ARM64_OSDLR_EL1         ARM64_SYSREG(2,0, 1, 3,4)  // OS Double Lock Register [CP14_DBGOSDLR]
#define ARM64_DBGPRCR_EL1       ARM64_SYSREG(2,0, 1, 4,4)  // Debug Power/Reset Control Register

#define ARM64_OSLSR_LOCK_IMP    0x1
#define ARM64_OSLSR_LOCKED      0x2

__int64 _ReadStatusReg(int);
void _WriteStatusReg(int, __int64);

#pragma intrinsic(_WriteStatusReg)
#pragma intrinsic(_ReadStatusReg)

//
// Interrupt Controller Registers
//

#define ARM64_ICC_SRE_EL2       ARM64_SYSREG(3,4,12,9,5)   // System Register Enable Register

//
// ARM Feature ID Codes
//
// These nibble-offsets define which nibble in a control register
// is used to determine the existance of a feature.
//

#define READ_ARM64_FEATURE(_FeatureRegister, _Index) \
        (((ULONG64)_ReadStatusReg(_FeatureRegister) >> ((_Index) * 4)) & 0xF)

#define AFR0_EL0_EXCEPTIONS         0
#define AFR0_EL1_EXCEPTIONS         1
#define AFR0_EL2_EXCEPTIONS         2
#define AFR0_EL3_EXCEPTIONS         3
#define AFR0_ELn_EXCEPTIONS_NI      0
#define AFR0_ELn_EXCEPTIONS_64ONLY  1
#define AFR0_ELn_EXCEPTIONS_64OR32  2

#define AFR0_FLOATING_POINT         4
#define AFR0_FLOATING_POINT_IMP     0
#define AFR0_FLOATING_POINT_NI      15

#define AFR0_ADVANCED_SIMD          5
#define AFR0_ADVANCED_SIMD_IMP      0
#define AFR0_ADVANCED_SIMD_NI       15

#define DFR0_ARCH_VERSION           0
#define DFR0_ARCH_VERSION_V8A       6

#define DFR0_TRACE_VERSION          1
#define DFR0_TRACE_VERSION_NI       0
#define DFR0_TRACE_VERSION_IMP      1

#define DFR0_PERFMON_VERSION        2
#define DFR0_PERFMON_VERSION_NI     0
#define DFR0_PERFMON_VERSION_V3     1
#define DFR0_PERFMON_VERSION_UNK    15

#define DFR0_BREAKPOINT_COUNT       3

#define DFR0_WATCHPOINT_COUNT       5

#define DFR0_CONTEXT_BP_COUNT       7

#define MMFR0_ADDRESS_BITS          0
#define MMFR0_ADDRESS_BITS_32       0
#define MMFR0_ADDRESS_BITS_36       1
#define MMFR0_ADDRESS_BITS_40       2
#define MMFR0_ADDRESS_BITS_42       3
#define MMFR0_ADDRESS_BITS_44       4
#define MMFR0_ADDRESS_BITS_48       5

#define MMFR0_ASID_BITS             1
#define MMFR0_ASID_BITS_8           0
#define MMFR0_ASID_BITS_16          2

#define MMFR0_MIXED_ENDIAN          2
#define MMFR0_MIXED_ENDIAN_NI       0
#define MMFR0_MIXED_ENDIAN_IMP      1

#define MMFR0_SECURE_MEMORY         3
#define MMFR0_SECURE_MEMORY_NI      0
#define MMFR0_SECURE_MEMORY_IMP     1

#define MMFR0_MIXED_ENDIAN_EL0      4
#define MMFR0_MIXED_ENDIAN_EL0_NI   0
#define MMFR0_MIXED_ENDIAN_EL0_IMP  1

#define ISAR0_AES                   1
#define ISAR0_AES_NI                0
#define ISAR0_AES_INSTRUCTIONS      1
#define ISAR0_AES_PLUS_PMULL64      2

#define ISAR0_SHA1                  2
#define ISAR0_SHA1_NI               0
#define ISAR0_SHA1_INSTRUCTIONS     1

#define ISAR0_SHA2                  3
#define ISAR0_SHA2_NI               0
#define ISAR0_SHA2_INSTRUCTIONS     1

#define ISAR0_CRC32                 4
#define ISAR0_CRC32_NI              0
#define ISAR0_CRC32_INSTRUCTIONS    1


//
// VFP sub-architecture is really bits 16-22 (0x007F0000), however, bit 22
// is used to indicate if the subarchitecture was designed by ARM, so all we
// really need to look at are bits 16-21
//
// VFPv3 indicates "The entire floating-point implementation is in
// hardware, and no software support code is required."
//
/*
#define CPVFP_FPSID_SW                  0x00800000      // Software-only floating-point

#define CPVFP_FPEXC_EX                  0x80000000      // Extra state required for context save
#define CPVFP_FPEXC_EN                  0x40000000      // VFP/Advanced SIMD enable
#define CPVFP_FPEXC_DEX                 0x20000000      // synchronous exception flag
#define CPVFP_FPEXC_FP2V                0x10000000      // FPINST2 is valid
*/
#define ARM64_CPACR_VFP_MASK            0x00300000      // Mask for enabling/disabling VFP/NEON access
#define ARM64_CPACR_VFP_MASK_BIT        20              // Index of low bit for TBNZ purposes

#define ARM64_CPTR_VFP_USERMODE         0x00000400      // Enable CP10/CP11 in user mode

#define ARM64_CPTR_TFP                  0x00000400      // Trap Floating Point instructions from EL0/1 to EL2 that are not trapped to EL1.
#define ARM64_CPTR_TTA                  0x00100000      // Trap Trace functionality to EL2 when executed from EL0/1
#define ARM64_CPTR_CPAC                 0x80000000      // Trap direct access to CPACR or CPACR_EL1 from EL1 to EL2
#define ARM64_CPTR_RES1_BITS            0x000033FF      // Bits in CPTR_ELx that are RES1

//
// Coprocessor registers for ARMv7 cache hierarchy information
//

#define ARM_MPIDR_U                   0x40000000
#define ARM_MPIDR_MT                  0x01000000

//
// Constants for flags in system control register.
//

#define ARM64_SCTLR_M                    0x00000001
#define ARM64_SCTLR_A                    0x00000002
#define ARM64_SCTLR_C                    0x00000004
#define ARM64_SCTLR_SA                   0x00000008 /* ARMv7: was W */
#define ARM64_SCTLR_SA0                  0x00000010 /* ARMv7: was P */
#define ARM64_SCTLR_CP15BEN              0x00000020 /* ARMv7: was D */
#define ARM64_SCTLR_THEE                 0x00000040 /* ARMv7: was L */
#define ARM64_SCTLR_ITD                  0x00000080 /* ARMv7: was B */
#define ARM64_SCTLR_SED                  0x00000100 /* ARMv7: was S */
#define ARM64_SCTLR_UMA                  0x00000200 /* ARMv7: was R */
#define ARM64_SCTLR_RES0_10              0x00000400 /* ARMv7: was F */
#define ARM64_SCTLR_RES1_11              0x00000800 /* ARMv7: was Z */
#define ARM64_SCTLR_I                    0x00001000
#define ARM64_SCTLR_RES0_13              0x00002000 /* ARMv7: was V */
#define ARM64_SCTLR_DZE                  0x00004000 /* ARMv7: was RR */
#define ARM64_SCTLR_UCT                  0x00008000 /* ARMv7: was L4 */
#define ARM64_SCTLR_nTWI                 0x00010000 /* ARMv7: was DT */
#define ARM64_SCTLR_RES0_17              0x00020000
#define ARM64_SCTLR_nTWE                 0x00040000 /* ARMv7: was IT */
#define ARM64_SCTLR_WXN                  0x00080000
#define ARM64_SCTLR_RES1_20              0x00100000
#define ARM64_SCTLR_RES0_21              0x00200000 /* ARMv7: was FI */
#define ARM64_SCTLR_RES1_22              0x00400000 /* ARMv7: was U */
#define ARM64_SCTLR_RES1_23              0x00800000 /* ARMv7: was XP */
#define ARM64_SCTLR_E0E                  0x01000000 /* ARMv7: was VE */
#define ARM64_SCTLR_EE                   0x02000000
#define ARM64_SCTLR_UCI                  0x04000000 /* ARMv7: was L2 */
#define ARM64_SCTLR_RES0_27              0x08000000 /* ARMv7: was NM */
#define ARM64_SCTLR_RES1_28              0x10000000 /* ARMv7: was TR */
#define ARM64_SCTLR_RES1_29              0x20000000 /* ARMv7: was AF */
#define ARM64_SCTLR_RES0_30              0x40000000 /* ARMv7: was TE */
#define ARM64_SCTLR_RES0_31              0x80000000 /* ARMv7: was X3 */

#define ARM64_SCTLR_RES1_MASK            ARM64_SCTLR_RES1_11 | \
                                         ARM64_SCTLR_RES1_20 | \
                                         ARM64_SCTLR_RES1_22 | \
                                         ARM64_SCTLR_RES1_23 | \
                                         ARM64_SCTLR_RES1_28 | \
                                         ARM64_SCTLR_RES1_29

//
// Processor feature register definitions and masks
//

#define ARM64_PFR0_EL1_EL0_MASK          0x000000000000000F
#define ARM64_PFR0_EL1_EL1_MASK          0x00000000000000F0
#define ARM64_PFR0_EL1_EL2_MASK          0x0000000000000F00
#define ARM64_PFR0_EL1_EL3_MASK          0x000000000000F000
#define ARM64_PFR0_EL1_FP_MASK           0x00000000000F0000
#define ARM64_PFR0_EL1_ADVSIMD_MASK      0x0000000000F00000
#define ARM64_PFR0_EL1_GIC_MASK          0x000000000F000000


//
// Translation table base register definitions and masks
//

#define ARM64_TTBRx_BADDR_MASK     0x0000ffffffffffffULL
#define ARM64_TTBRx_ASID_MASK      0xffff000000000000ULL
#define ARM64_TTBRx_ASID_SHIFT     48

//
// Constants for flags in the hypervisor control register
//

#define ARM64_HCR_EL2_VM                0x000000000000001ULL
#define ARM64_HCR_EL2_SWIO              0x000000000000002ULL
#define ARM64_HCR_EL2_PTW               0x000000000000004ULL
#define ARM64_HCR_EL2_FMO               0x000000000000008ULL
#define ARM64_HCR_EL2_IMO               0x000000000000010ULL
#define ARM64_HCR_EL2_AMO               0x000000000000020ULL
#define ARM64_HCR_EL2_VF                0x000000000000040ULL
#define ARM64_HCR_EL2_VI                0x000000000000080ULL
#define ARM64_HCR_EL2_VSE               0x000000000000100ULL
#define ARM64_HCR_EL2_FB                0x000000000000200ULL
#define ARM64_HCR_EL2_BSU_NONE          0x000000000000000ULL
#define ARM64_HCR_EL2_BSU_ISH           0x000000000000400ULL
#define ARM64_HCR_EL2_BSU_OSH           0x000000000000800ULL
#define ARM64_HCR_EL2_BSU_FS            0x000000000000C00ULL
#define ARM64_HCR_EL2_BSU_MASK          0x000000000000C00ULL
#define ARM64_HCR_EL2_DC                0x000000000001000ULL
#define ARM64_HCR_EL2_TWI               0x000000000002000ULL
#define ARM64_HCR_EL2_TWE               0x000000000004000ULL
#define ARM64_HCR_EL2_TID0              0x000000000008000ULL
#define ARM64_HCR_EL2_TID1              0x000000000010000ULL
#define ARM64_HCR_EL2_TID2              0x000000000020000ULL
#define ARM64_HCR_EL2_TID3              0x000000000040000ULL
#define ARM64_HCR_EL2_TSC               0x000000000080000ULL
#define ARM64_HCR_EL2_TIDCP             0x000000000100000ULL
#define ARM64_HCR_EL2_TACR              0x000000000200000ULL
#define ARM64_HCR_EL2_TSW               0x000000000400000ULL
#define ARM64_HCR_EL2_TPC               0x000000000800000ULL
#define ARM64_HCR_EL2_TPU               0x000000001000000ULL
#define ARM64_HCR_EL2_TTLB              0x000000002000000ULL
#define ARM64_HCR_EL2_TVM               0x000000004000000ULL
#define ARM64_HCR_EL2_TGE               0x000000008000000ULL
#define ARM64_HCR_EL2_TDZ               0x000000010000000ULL
#define ARM64_HCR_EL2_HCD               0x000000020000000ULL
#define ARM64_HCR_EL2_TRVM              0x000000040000000ULL
#define ARM64_HCR_EL2_RW                0x000000080000000ULL
#define ARM64_HCR_EL2_CD                0x000000100000000ULL
#define ARM64_HCR_EL2_ID                0x000000200000000ULL


//
// Constants for flags in translation control register.
// (configured so that hardware page table walks are cached)
//

#define ARM64_TCR_T0SZ_MASK             0x000000000000003fULL
#define ARM64_TCR_RES0_1                0x0000000000000040ULL
#define ARM64_TCR_EPD0                  0x0000000000000080ULL
#define ARM64_TCR_IRGN0_NC              0x0000000000000000ULL
#define ARM64_TCR_IRGN0_WBWA            0x0000000000000100ULL
#define ARM64_TCR_IRGN0_WT              0x0000000000000200ULL
#define ARM64_TCR_IRGN0_WB              0x0000000000000300ULL
#define ARM64_TCR_IRGN0_MASK            0x0000000000000300ULL
#define ARM64_TCR_ORGN0_NC              0x0000000000000000ULL
#define ARM64_TCR_ORGN0_WBWA            0x0000000000000400ULL
#define ARM64_TCR_ORGN0_WT              0x0000000000000800ULL
#define ARM64_TCR_ORGN0_WB              0x0000000000000c00ULL
#define ARM64_TCR_ORGN0_MASK            0x0000000000000c00ULL
#define ARM64_TCR_SH0_NON_SHARED        0x0000000000000000ULL
#define ARM64_TCR_SH0_OUTER_SHARED      0x0000000000002000ULL
#define ARM64_TCR_SH0_INNER_SHARED      0x0000000000003000ULL
#define ARM64_TCR_SH0_MASK              0x0000000000003000ULL
#define ARM64_TCR_TG0_4K                0x0000000000000000ULL
#define ARM64_TCR_TG0_16K               0x0000000000008000ULL
#define ARM64_TCR_TG0_64K               0x0000000000004000ULL
#define ARM64_TCR_TG0_RESERVED          0x000000000000c000ULL
#define ARM64_TCR_TG0_MASK              0x000000000000c000ULL

#define ARM64_TCR_T1SZ_MASK             0x00000000003f0000ULL
#define ARM64_TCR_A1                    0x0000000000400000ULL
#define ARM64_TCR_EPD1                  0x0000000000800000ULL
#define ARM64_TCR_IRGN1_NC              0x0000000000000000ULL
#define ARM64_TCR_IRGN1_WBWA            0x0000000001000000ULL
#define ARM64_TCR_IRGN1_WT              0x0000000002000000ULL
#define ARM64_TCR_IRGN1_WB              0x0000000003000000ULL
#define ARM64_TCR_IRGN1_MASK            0x0000000003000000ULL
#define ARM64_TCR_ORGN1_NC              0x0000000000000000ULL
#define ARM64_TCR_ORGN1_WBWA            0x0000000004000000ULL
#define ARM64_TCR_ORGN1_WT              0x0000000008000000ULL
#define ARM64_TCR_ORGN1_WB              0x000000000c000000ULL
#define ARM64_TCR_ORGN1_MASK            0x000000000c000000ULL
#define ARM64_TCR_SH1_NON_SHARED        0x0000000000000000ULL
#define ARM64_TCR_SH1_OUTER_SHARED      0x0000000020000000ULL
#define ARM64_TCR_SH1_INNER_SHARED      0x0000000030000000ULL
#define ARM64_TCR_SH1_MASK              0x0000000030000000ULL
#define ARM64_TCR_TG1_4K                0x0000000080000000ULL
#define ARM64_TCR_TG1_16K               0x0000000040000000ULL
#define ARM64_TCR_TG1_64K               0x00000000c0000000ULL
#define ARM64_TCR_TG1_RESERVED          0x0000000000000000ULL
#define ARM64_TCR_TG1_MASK              0x00000000c0000000ULL

#define ARM64_TCR_IPASize_4G            0x0000000000000000ULL
#define ARM64_TCR_IPASize_64G           0x0000000100000000ULL
#define ARM64_TCR_IPASize_1T            0x0000000200000000ULL
#define ARM64_TCR_IPASize_4T            0x0000000300000000ULL
#define ARM64_TCR_IPASize_16T           0x0000000400000000ULL
#define ARM64_TCR_IPASize_256T          0x0000000500000000ULL
#define ARM64_TCR_IPASize_MASK          0x0000000700000000ULL
#define ARM64_TCR_RES0_35               0x0000000800000000ULL
#define ARM64_TCR_AS                    0x0000001000000000ULL
#define ARM64_TCR_TBI0                  0x0000002000000000ULL
#define ARM64_TCR_TBI1                  0x0000004000000000ULL

#define ARM64_TCR_T0SZ_SHIFT            0
#define ARM64_TCR_T1SZ_SHIFT            16
#define ARM64_TCR_IPASize_SHIFT         32


//
// Constants for flags in translation control register for EL2.
// (HTCR) that are different from the EL1 version of this register.
//

#define ARM64_TCR_PASize_4G             0x00000000
#define ARM64_TCR_PASize_64G            0x00010000
#define ARM64_TCR_PASize_1T             0x00020000
#define ARM64_TCR_PASize_4T             0x00030000
#define ARM64_TCR_PASize_16T            0x00040000
#define ARM64_TCR_PASize_256T           0x00050000
#define ARM64_TCR_PASize_MASK           0x00070000

#define ARM64_TCR_EL2_TBI               0x00100000
#define ARM64_TCR_EL2_PASize_SHIFT      16

/*
//
// Fault status syndrome masks
//

#define CP15_xFSR_FS_LOW                0x0000000f
#define CP15_xFSR_FS_HIGH               0x00000400
#define CP15_DFSR_WnR                   0x00000800
*/
//
// Performance counter register bits
//

#define ARM64_PMCR_N_MASK               0x0000f800
#define ARM64_PMCR_N_SHIFT              11
#define ARM64_PMCR_LC                   0x00000040
#define ARM64_PMCR_DP                   0x00000020
#define ARM64_PMCR_X                    0x00000010
#define ARM64_PMCR_D                    0x00000008
#define ARM64_PMCR_C                    0x00000004
#define ARM64_PMCR_P                    0x00000002
#define ARM64_PMCR_E                    0x00000001


#define ARM64_PMCNTEN_PMCCNT            0x80000000

//
// Performance counter user-enable register bits
//

#define ARM64_PMUSERENR_EN              0x00000001
#define ARM64_PMUSERENR_SW              0x00000002
#define ARM64_PMUSERENR_CR              0x00000004
#define ARM64_PMUSERENR_ER              0x00000008

//
// Performance filter register bits
//

#define ARM64_PMCCFILTR_M               0x04000000
#define ARM64_PMCCFILTR_NSH             0x08000000
#define ARM64_PMCCFILTR_NSU             0x10000000
#define ARM64_PMCCFILTR_NSK             0x20000000
#define ARM64_PMCCFILTR_U               0x40000000
#define ARM64_PMCCFILTR_P               0x80000000

//
// Monitor Debug Configuration Register bits
//

#define ARM64_MDCR_HPMN_MASK            0x0000001F
#define ARM64_MDCR_TPMCR                0x00000020
#define ARM64_MDCR_TPM                  0x00000040
#define ARM64_MDCR_HPME                 0x00000080
#define ARM64_MDCR_TDE                  0x00000100
#define ARM64_MDCR_TDA                  0x00000200
#define ARM64_MDCR_TDOSA                0x00000400
#define ARM64_MDCR_TDRA                 0x00000800

//
// The hardware fault status registers DFSR and IFSR contain fields
// that can describe up to 64 different fault sources/causes.  To
// address the large number and the inconvenient encoding, we map
// these sources to an internal software fault status representation.
//

// ARM64_WORKITEM: Update these to match new behavior
#define SWFS_WRITE                      0x01
#define SWFS_EXECUTE                    0x08
#define SWFS_PAGE_FAULT                 0x10
#define SWFS_ALIGN_FAULT                0x20
#define SWFS_HWERR_FAULT                0x40
#define SWFS_DEBUG_FAULT                0x80


//
// Remap register settings.  Goal is to remap the 3 attribute bits
// down to 2 attributes plus one OS-managed bit.
//

// ARM64: double-check these -- are they right?

//
// Memory attribute indirection register settings. Goal is to provide
// memory attribute encodings corresponding to the possible AttrIndx values in
// Long-descriptor format translation table entry for stage 1 translations.
//

//
// The memory types are encoded as follows:
//         Attr0           Cached normal memory
//         Attr1           Device-nGnRnE memory (uncached, Strongly ordered)
//         Attr2           UNUSED
//         Attr3           Non-Cacheable normal memory (similar to WC)
//

#define ARM64_MAIR_CACHE_WBWA       0xff
#define ARM64_MAIR_CACHE_NC         0x00
#define ARM64_MAIR_CACHE_WTNA       0xbb
#define ARM64_MAIR_CACHE_WC         0x44

#define ARM64_MAIR_DEFAULT \
    (((ULONG64) ARM64_MAIR_CACHE_WBWA <<  0) | \
     ((ULONG64) ARM64_MAIR_CACHE_NC   <<  8) | \
     ((ULONG64) ARM64_MAIR_CACHE_WTNA << 16) | \
     ((ULONG64) ARM64_MAIR_CACHE_WC   << 24) | \
     ((ULONG64) ARM64_MAIR_CACHE_WBWA << 32) | \
     ((ULONG64) ARM64_MAIR_CACHE_NC   << 40) | \
     ((ULONG64) ARM64_MAIR_CACHE_WTNA << 48) | \
     ((ULONG64) ARM64_MAIR_CACHE_WC   << 56))

//
// Debug register bits.
//


#define ARM64_DBGBCR_MISMATCH_BIT        0x00400000
#define ARM64_DBGBCR_SECURITY_BITS       0x0000c000
#define   ARM64_DBGBCR_ANY_SECURE        0x00000000
#define   ARM64_DBGBCR_NONSECURE_ONLY    0x00004000
#define   ARM64_DBGBCR_SECURE_ONLY       0x00008000
#define ARM64_DBGBCR_PRIVILEGE_BITS      0x00000006
#define   ARM64_DBGBCR_USER_SYS_SUPER    0x00000000
#define   ARM64_DBGBCR_PRIVILEGED_ONLY   0x00000002
#define   ARM64_DBGBCR_USER_ONLY         0x00000004
#define   ARM64_DBGBCR_ANY_PRIVILEGE     0x00000006
#define ARM64_DBGBCR_ENABLE_BIT          0x00000001

#define ARM64_MDSCR_RXfull               0x40000000
#define ARM64_MDSCR_TXfull               0x20000000
#define ARM64_MDSCR_MDE                  0x00008000
#define ARM64_MDSCR_HDE                  0x00004000
#define ARM64_MDSCR_KDE                  0x00002000
#define ARM64_MDSCR_TDCC                 0x00001000
#define ARM64_MDSCR_SS                   0x00000001

typedef enum _tag_ARMINTR_BARRIER_TYPE
{
    _ARM_BARRIER_SY    = 0xF,
    _ARM_BARRIER_ST    = 0xE,
    _ARM_BARRIER_ISH   = 0xB,
    _ARM_BARRIER_ISHST = 0xA,
    _ARM_BARRIER_NSH   = 0x7,
    _ARM_BARRIER_NSHST = 0x6,
    _ARM_BARRIER_OSH   = 0x3,
    _ARM_BARRIER_OSHST = 0x2
}
_ARMINTR_BARRIER_TYPE;

void __dmb(unsigned int _Type);
void __dsb(unsigned int _Type);
void __isb(unsigned int _Type);

#pragma intrinsic(__dsb)
#pragma intrinsic(__isb)

#define _DataSynchronizationBarrier()        __dsb(_ARM_BARRIER_SY)
#define _InstructionSynchronizationBarrier() __isb(_ARM_BARRIER_SY)

#define ARM64_BREAK_DEBUG_BASE          0xf000
#define ARM64_BREAKPOINT                (ARM64_BREAK_DEBUG_BASE + 0)
#define ARM64_ASSERT                    (ARM64_BREAK_DEBUG_BASE + 1)
#define ARM64_DEBUG_SERVICE             (ARM64_BREAK_DEBUG_BASE + 2)
#define ARM64_FASTFAIL                  (ARM64_BREAK_DEBUG_BASE + 3)
#define ARM64_DIVIDE_BY_0               (ARM64_BREAK_DEBUG_BASE + 4)

#endif // defined(_ARM64_)

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

typedef struct _AMD64_KEXCEPTION_FRAME {

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
} AMD64_KEXCEPTION_FRAME, *AMD64_PKEXCEPTION_FRAME;


typedef struct X86_KEXCEPTION_FRAME {

//
// Home address for the parameter registers.
//

    ULONG P1Home;
    ULONG P2Home;
    ULONG P3Home;
    ULONG P4Home;
    ULONG P5;
    ULONG Spare1;

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

    ULONG TrapFrame;
    ULONG OutputBuffer;
    ULONG OutputLength;
    ULONG Spare2;

//
// Saved MXCSR when a thread is interrupted in kernel mode via a dispatch
// interrupt.
//

    ULONG MxCsr;

//
// Saved nonvolatile register - not always saved.
//

    ULONG Ebp;

//
// Saved nonvolatile registers.
//

    ULONG Ebx;
    ULONG Edi;
    ULONG Esi;

//
// EFLAGS and return address.
//

    ULONG Return;
} X86_KEXCEPTION_FRAME, *PX86_KEXCEPTION_FRAME;

typedef struct _ARM64_KEXCEPTION_FRAME {

//
// Saved nonvolatile registers.
//

    ULONG64 X19;
    ULONG64 X20;
    ULONG64 X21;
    ULONG64 X22;
    ULONG64 X23;
    ULONG64 X24;
    ULONG64 X25;
    ULONG64 X26;
    ULONG64 X27;
    ULONG64 X28;
    ULONG64 Fp;

    ULONG64 Return;

} ARM64_KEXCEPTION_FRAME, *PARM64_KEXCEPTION_FRAME;

//
// Trap frame
//
// This frame is established when handling a trap. It provides a place to
// save all volatile registers. The nonvolatile registers are saved in an
// exception frame or through the normal C calling conventions for saved
// registers.
//

typedef struct _AMD64_KTRAP_FRAME {

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
} AMD64_KTRAP_FRAME, *PAMD64_PKTRAP_FRAME;

//
// Define the format of the x86 trap frame for BLUE and later OSes.
//

typedef struct _X86_KTRAP_FRAME_BLUE {
    ULONG   DbgEbp;         // Copy of User EBP set up so KB will work.
    ULONG   DbgEip;         // EIP of caller to system call, again, for KB.
    ULONG   DbgArgMark;     // Marker to show no args here.

//
//  Temporary values used when frames are edited.
//

    USHORT  TempSegCs;
    UCHAR   Logging;
    UCHAR   FrameType;
    ULONG   TempEsp;

//
//  Debug registers.
//

    ULONG   Dr0;
    ULONG   Dr1;
    ULONG   Dr2;
    ULONG   Dr3;
    ULONG   Dr6;
    ULONG   Dr7;

//
//  Segment registers
//

    ULONG   SegGs;
    ULONG   SegEs;
    ULONG   SegDs;
    ULONG   SegSs;

//
//  Volatile registers
//

    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;

//
//  Nesting state, not part of context record
//

    UCHAR   PreviousPreviousMode;
    UCHAR   EntropyQueueDpc;                // decision whether to queue an entropy DPC
    UCHAR   Reserved[2];

    ULONG   MxCsr;                          // saved SSE control register

    ULONG ExceptionList;
                                            // Trash if caller was user mode.
                                            // Saved exception list if caller
                                            // was kernel mode or we're in
                                            // an interrupt.

//
//  FS is TIB/PCR pointer, is here to make save sequence easy
//

    ULONG   SegFs;

//
//  Non-volatile registers
//

    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Ebp;
    ULONG   Esp;

//
//  Control registers
//

    ULONG   ErrCode;
    ULONG   Eip;
    ULONG   SegCs;
    ULONG   EFlags;

    ULONG   HardwareEsp;    // WARNING - segSS:esp are only here for stacks
    ULONG   HardwareSegSs;  // that involve a ring transition.

    ULONG   V86Es;          // these will be present for all transitions from
    ULONG   V86Ds;          // V86 mode
    ULONG   V86Fs;
    ULONG   V86Gs;
} X86_KTRAP_FRAME_BLUE, *PX86_KTRAP_FRAME_BLUE;


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


//
// Trap frame
//
//  NOTE - We deal only with 32bit registers, so the assembler equivalents
//         are always the extended forms.
//
//  NOTE - Unless you want to run like slow molasses everywhere in the
//         the system, this structure must be of DWORD length, DWORD
//         aligned, and its elements must all be DWORD aligned.
//
//  NOTE WELL   -
//
//      The i386 does not build stack frames in a consistent format, the
//      frames vary depending on whether or not a privilege transition
//      was involved.
//
//      In order to make NtContinue work for both user mode and kernel
//      mode callers, we must force a canonical stack.
//
//      If we're called from kernel mode, this structure is 8 bytes longer
//      than the actual frame!
//
//  WARNING:
//
//      KTRAP_FRAME_LENGTH needs to be 16byte integral (at present.)
//

typedef struct _X86_KTRAP_FRAME {


//
// The following 3 fields are placed to make debugger backtraces work through
// a trap frame.
//

    ULONG   DbgEbp;         // Copy of User EBP set up so KB will work.
    ULONG   DbgEip;         // EIP of caller to system call, again, for KB.
    ULONG   DbgArgMark;     // Marker to show no args here.

//
//  Temporary values used when frames are edited.
//
//
//  NOTE:   Any code that wants ESP must materialize it, since it
//          is not stored in the frame for kernel mode callers.
//
//          And code that sets ESP in a KERNEL mode frame, must put
//          the new value in TempEsp, make sure that TempSegCs holds
//          the real SegCs value, and put a special marker value into SegCs.
//

    USHORT  TempSegCs;
    UCHAR   Logging;
    UCHAR   FrameType;
    ULONG   TempEsp;

//
//  Debug registers.
//

    ULONG   Dr0;
    ULONG   Dr1;
    ULONG   Dr2;
    ULONG   Dr3;
    ULONG   Dr6;
    ULONG   Dr7;

//
//  Segment registers
//

    ULONG   SegGs;
    ULONG   SegEs;
    ULONG   SegDs;

//
//  Volatile registers
//

    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;

//
//  Nesting state, not part of context record
//

    UCHAR   PreviousPreviousMode;
    UCHAR   EntropyQueueDpc;                // decision whether to queue an entropy DPC
    UCHAR   Reserved[2];

    ULONG   MxCsr;                          // saved SSE control register

    PEXCEPTION_REGISTRATION_RECORD ExceptionList;
                                            // Trash if caller was user mode.
                                            // Saved exception list if caller
                                            // was kernel mode or we're in
                                            // an interrupt.

//
//  FS is TIB/PCR pointer, is here to make save sequence easy
//

    ULONG   SegFs;

//
//  Non-volatile registers
//

    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Ebp;

//
//  Control registers
//

    ULONG   ErrCode;
    ULONG   Eip;
    ULONG   SegCs;
    ULONG   EFlags;

    ULONG   HardwareEsp;    // WARNING - segSS:esp are only here for stacks
    ULONG   HardwareSegSs;  // that involve a ring transition.

    ULONG   V86Es;          // these will be present for all transitions from
    ULONG   V86Ds;          // V86 mode
    ULONG   V86Fs;
    ULONG   V86Gs;
} X86_KTRAP_FRAME;

typedef struct _ARM64_KTRAP_FRAME {

//
// Exception active indicator.
//
//    0 - interrupt frame.
//    1 - exception frame.
//    2 - service frame.
//

    /* +0x000 */ UCHAR ExceptionActive;              // always valid
    /* +0x001 */ UCHAR ContextFromKFramesUnwound;    // set if KeContextFromKFrames created this frame
    /* +0x002 */ UCHAR DebugRegistersValid;          // always valid
    /* +0x003 */ union {
                     KPROCESSOR_MODE PreviousMode;   // system services only
                     KIRQL PreviousIrql;             // interrupts only
                 };

//
// Page fault information (page faults only)
// Previous trap frame address (system services only)
//
// Organized this way to allow first couple words to be used
// for scratch space in the general case
//

    /* +0x004 */ ULONG FaultStatus;                      // page faults only
    /* +0x008 */ union {
                     ULONG64 FaultAddress;             // page faults only
                     ULONG64 TrapFrame;                // system services only
                 };

//
// The ARM architecture does not have an architectural trap frame.  On
// an exception or interrupt, the processor switches to an
// exception-specific processor mode in which at least the LR and SP
// registers are banked.  Software is responsible for preserving
// registers which reflect the processor state in which the
// exception occurred rather than any intermediate processor modes.
//

//
// Volatile floating point state is dynamically allocated; this
// pointer may be NULL if the FPU was not enabled at the time the
// trap was taken.
//

    /* +0x010 */ PVOID VfpState;

//
// Debug registers
//

    /* +0x018 */ ULONG Bcr[ARM64_MAX_BREAKPOINTS];
    /* +0x038 */ ULONG64 Bvr[ARM64_MAX_BREAKPOINTS];
    /* +0x078 */ ULONG Wcr[ARM64_MAX_WATCHPOINTS];
    /* +0x080 */ ULONG64 Wvr[ARM64_MAX_WATCHPOINTS];

//
// Volatile registers X0-X17, and the FP, SP, LR
//

    /* +0x090 */ ULONG Spsr;
    /* +0x094 */ ULONG Esr;
    /* +0x098 */ ULONG64 Sp;
    /* +0x0A0 */ ULONG64 X[19];
    /* +0x138 */ ULONG64 Lr;
    /* +0x140 */ ULONG64 Fp;
    /* +0x148 */ ULONG64 Pc;
    /* +0x150 */

} ARM64_KTRAP_FRAME, *PARM64_KTRAP_FRAME;



#if defined(_AMD64_)

typedef AMD64_CONTEXT CONTEXT, *PCONTEXT;
typedef AMD64_KPROCESSOR_STATE KPROCESSOR_STATE, *PKPROCESSOR_STATE;
typedef AMD64_KTRAP_FRAME _KTRAP_FRAME, *PKTRAP_FRAME;
typedef AMD64_KEXCEPTION_FRAME KEXCEPTION_FRAME, *PKEXCEPTION_FRAME;
typedef AMD64_KSPECIAL_REGISTERS KSPECIAL_REGISTERS;

#elif defined(_ARM64_)

typedef ARM64_CONTEXT CONTEXT, *PCONTEXT;
typedef ARM64_KSPECIAL_REGISTERS KSPECIAL_REGISTERS, *PKSPECIAL_REGISTERS;
typedef ARM64_KPROCESSOR_STATE KPROCESSOR_STATE, *PKPROCESSOR_STATE;
typedef ARM64_KTRAP_FRAME _KTRAP_FRAME, *PKTRAP_FRAME;
typedef ARM64_KEXCEPTION_FRAME KEXCEPTION_FRAME, *PKEXCEPTION_FRAME;
typedef ARM64_KSPECIAL_REGISTERS KSPECIAL_REGISTERS;

#endif


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

typedef enum {
    ContinueError = FALSE,
    ContinueSuccess = TRUE,
    ContinueProcessorReselected,
    ContinueNextProcessor
} KCONTINUE_STATUS;

#if defined(_AMD64_)

#define CONTEXT_TO_PROGRAM_COUNTER(Context) ((Context)->Rip)

#define CONTEXT_AMD64           0x00100000L

#define CONTEXT_CONTROL         (CONTEXT_AMD64 | 0x00000001L)
#define CONTEXT_INTEGER         (CONTEXT_AMD64 | 0x00000002L)
#define CONTEXT_SEGMENTS        (CONTEXT_AMD64 | 0x00000004L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_AMD64 | 0x00000008L)
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_AMD64 | 0x00000010L)

#define CONTEXT_FULL            (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT)

#elif defined(_ARM64_)

#define CONTEXT_TO_PROGRAM_COUNTER(Context) ((Context)->Pc)

#define CONTEXT_ARM64           0x00400000L

#define CONTEXT_CONTROL         (CONTEXT_ARM64 | 0x1L)
#define CONTEXT_INTEGER         (CONTEXT_ARM64 | 0x2L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_ARM64 | 0x4L)
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_ARM64 | 0x8L)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT)
#define CONTEXT_ALL  (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS)

#endif


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

//
// SEGMENT_MASK is used to throw away trash part of segment.  Part always
// pushes or pops 32 bits to/from stack, but if it's a segment value,
// high order 16 bits are trash.
//

#define SEGMENT_MASK    0xffff


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
