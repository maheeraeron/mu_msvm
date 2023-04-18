/*++

Copyright (c) 1990-2003  Microsoft Corporation

Module Name:

    bd.h

Abstract:

    This module contains the data structures and function prototypes for the
    boot debugger.

Author:

    Jamie Schwartz (jamschw) 10-Dec-2003 - Mostly a copy of the original
        implementation by DaveC.

Environment:

    Boot

--*/

#pragma once

//#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#define UNALIGNED

#include <EfiNt.h>
#include <Library/BdDebugLib.h>

//
// Necessary type definitions.
//

typedef LIST_ENTRY LIST_ENTRY64;

typedef UINT8  volatile* PVUINT8;
typedef UINT8 BYTE, *PBYTE;

typedef char CCHAR;
typedef UCHAR KIRQL;
typedef KIRQL *PKIRQL;

typedef unsigned long DWORD;

typedef ULONG LOGICAL;
typedef LONG HRESULT;
#define S_OK    ((HRESULT)0L)

typedef PVOID HANDLE;
typedef HANDLE *PHANDLE;

typedef DWORD ACCESS_MASK, *PACCESS_MASK;

typedef struct _UNICODE_STRING
{
    UINT16 Length;
    UINT16 MaximumLength;
    CHAR16 *Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

#define UNICODE_NULL ((CHAR16)(0))

#include "cpu.h"
#include "BdHelper.h"

//#include "ntverp.h"
#define VER_PRODUCTBUILD 9600

//#include "kdbgnet.h"
#define KD_NET_KEY_SIZE_DWORDS 8
#define KD_NET_KEY_SIZE (KD_NET_KEY_SIZE_DWORDS * sizeof(ULONG))

//
// Size of unloaded module array
// Must match MI_UNLOADED_DRIVERS in ntos\mm\mi.h
//
#define BD_MAX_UNLOADED_MODULES 50

//
// Define message buffer size in bytes.
//
// N.B. This must be 0 mod 8.
//

#define BD_MESSAGE_BUFFER_SIZE 4096
#define BD_MAXIMUM_FILENAME_SIZE 1024

//
// Define the maximum number of retries for packet sends.
//

#define MAXIMUM_RETRIES 20

//
// Define packet waiting status codes.
//

#define BD_PACKET_RECEIVED 0
#define BD_PACKET_TIMEOUT  1
#define BD_PACKET_RESEND   2

//
// Define break point table entry structure.
//

#define BD_BREAKPOINT_IN_USE         0x1
#define BD_BREAKPOINT_NEEDS_WRITE    0x2
#define BD_BREAKPOINT_SUSPENDED      0x4
#define BD_BREAKPOINT_NEEDS_REPLACE  0x8

typedef struct _BREAKPOINT_ENTRY
{
    ULONG              Flags;
    ULONG64            Address;
    BD_BREAKPOINT_TYPE Content;
} BREAKPOINT_ENTRY, *PBREAKPOINT_ENTRY;

//
// Maximum packet size.
//

#define LEGACY_PACKET_MAX_SIZE 4088


//
// KD file support.
//

#define TRANSFER_LENGTH 8192
#define KD_MAX_REMOTE_FILES 16

typedef struct _KD_REMOTE_FILE
{
    ULONG64 RemoteHandle;
} KD_REMOTE_FILE, *PKD_REMOTE_FILE;

//
// Define packet send and receive function prototypes.  These will be set to
// appropriate routines based on the debugger transport in use.
//

typedef
UINT32
(*PBD_RECEIVE_PACKET) (
    __in UINT32 ExpectedPacketType,
    __out_opt PSTRING MessageHeader,
    __out_opt PSTRING MessageData,
    __out_opt PUINT32 DataLength
    );

typedef
VOID
(*PBD_SEND_PACKET) (
    __in UINT32 PacketType,
    __in PSTRING MessageHeader,
    __in_opt PSTRING MessageData
    );

extern PBD_RECEIVE_PACKET BdReceivePacket;
extern PBD_SEND_PACKET BdSendPacket;

//
// Define debug device connection parameters.
//

typedef enum
{
    BdNone,
    BdSerial,
    BdNet
} BD_CONNECTION_TYPE;

typedef struct _BD_CONNECTION_PARAMETERS
{
    BD_CONNECTION_TYPE Type;

    struct
    {
        ULONG AlternatePortNumber;
        ULONG BaudRate;
    } SerialPort;

    struct
    {
        PVOID TransportHob;
    } Net;

} BD_CONNECTION_PARAMETERS, *PBD_CONNECTION_PARAMETERS;

extern BD_CONNECTION_TYPE BdDebuggerType;

//
// Define function prototypes.
//

#define PASS_KERNEL_LARGE_MAPPINGS 1
#define DONT_FREE_LARGE_MAPPINGS 2

// ----------------------------------------------------------------- Prototypes

BOOLEAN
BdDebuggerEnabled (
    VOID
    );

/*++

Routine Description:

    This routine returns a BOOLEAN indicating the status of the debugger
    connection.

Arguments:

    None.

Return Value:

    TRUE if a debugger connection is established.
    FALSE otherwise.

--*/

NTSTATUS
BdInitialize (
    _In_opt_ VOID *Hob
    );

/*++

Routine Description:

    Initializes the boot debugger for the executing boot application.

Arguments:

    Hob - Optionally supplies a pointer to a HOB describing the loaded KDNET
          transport module.

Return Value:

    STATUS_SUCCESS when successful.
    STATUS_NOT_FOUND if the debugger port does not exist.

--*/

VOID
BdStart (
    VOID
    );

/*++

Routine Description:

    This routine will load the image symbols and initiate a debugger session.

Arguments:

    None.

Return Value:

    None.

--*/

VOID
BdStop (
    VOID
    );

/*++

Routine Description:

    This routine will unload the image symbols and temporarily stop debugging.

    It will not close the debugger port or reset any global state information.
    A call to BdStart will resume the debugger session.

Arguments:

    None.

Return Value:

    None.

--*/

VOID
BdPatchIdt (
    _Inout_bytecount_(IdtLength) PVOID Idt,
    _In_    UINT32      IdtLength,
    _In_    UINT16      CodeSegment
    );

/*++

Routine Description:

    This routine will install trap vectors in the given Interrupt Descriptor
    Table.

Arguments:

    Idt - Supplies a pointer to the IDT to patch.

    IdtLength - Supplies the size of the IDT in bytes.

    CodeSegment - Supplies a pointer to the code segment to use when patching
        the IDT.

Return Value:

    None.

--*/

UINT32
BdPollBreakIn (
    VOID
    );

VOID
BdBreakPointWithStatus(
    __in UINT32 Status
    );

VOID
TripleFault(
    __in    UINTN   Rax,
    __in    UINTN   Rbx,
    __in    UINTN   Rcx,
    __in    UINTN   Rdx
    );

NTSTATUS
BdArchInitialize (
    VOID
    );

/*++

Routine Description:

    This function carries out architecture specific boot debugger
    initialization.  Trap handlers may be installed either in this routine or
    when the debugger is started.

Arguments:

    None.

Return Value:

    STATUS_SUCCESS when initialization is successful.

    An appropriate error status is returned if initialization is not
        successful.

--*/

VOID
BdArchDestroy (
    VOID
    );

/*++

Routine Description:

    This function frees any resources allocated during architecture specific
    boot debugger initialization.

Arguments:

    None.

Return Value:

    None.

--*/

VOID
BdArchStart (
    VOID
    );

/*++

Routine Description:

    This function performs any architecture specific actions required before a
    debugger connection attempt can take place.  This may include installing
    trap handlers that cannot be used unless a debugger connection is active.

Arguments:

    None.

Return Value:

    None.

--*/

VOID
BdArchStop (
    VOID
    );

/*++

Routine Description:

    This function performs any architecture specific actions required before
    an active debugger connection can be disconnected.  This may include
    removing trap handlers that only function correctly while executing this
    boot application under an active debugger connection.  Connections are
    most often disconnected when passing control to a component (such as
    another boot application) that will possibly return to this application
    but will use its own debugging subsystem while running.

Arguments:

    None.

Return Value:

    None.

--*/

BOOLEAN
BdDebuggerInitialized (
    VOID
    );

/*++

Routine Description:

    This routine returns a BOOLEAN indicating whether the debugger subsystem
    is initialized.

Arguments:

    None.

Return Value:

    TRUE if a debugger subsystem is initialized.
    FALSE otherwise.

--*/


//
// Serial communication functions (comio.c)
//

VOID
BdComCloseDebuggerDevice (
    VOID
    );

NTSTATUS
BdComConfigureDebuggerDevice (
    __in PBD_CONNECTION_PARAMETERS Parameters
    );

NTSTATUS
BdComGetConnectionParameters (
    __out PBD_CONNECTION_PARAMETERS Parameters
    );

UINT32
BdComReceivePacket (
    __in UINT32 ExpectedPacketType,
    __out_opt PSTRING MessageHeader,
    __out_opt PSTRING MessageData,
    __out_opt PUINT32 DataLength
    );

VOID
BdComSendPacket (
    __in UINT32 PacketType,
    __in PSTRING MessageHeader,
    __in_opt PSTRING MessageData
    );

UINT64
BdComSentReceivedPacketCount(
    VOID
    );

//
// Network communication functions (netio.c).
//

NTSTATUS
BdNetConfigureDebuggerDevice (
    __in PBD_CONNECTION_PARAMETERS Parameters
    );

UINT32
BdNetReceivePacket (
    __in UINT32 ExpectedPacketType,
    __out_opt PSTRING MessageHeader,
    __out_opt PSTRING MessageData,
    __out_opt PUINT32 DataLength
    );

VOID
BdNetSendPacket (
    __in UINT32 PacketType,
    __in PSTRING MessageHeader,
    __in_opt PSTRING MessageData
    );

UINT64
BdNetSentReceivedPacketCount(
    VOID
    );

//
// Breakpoint functions (break.c).
//

UINT32
BdAddBreakpoint (
    __in ULONG64 Address
    );

LOGICAL
BdDeleteBreakpoint (
    __in UINT32 Handle
    );

LOGICAL
BdDeleteBreakpointRange (
    __in UINT64 Lower,
    __in UINT64 Upper
    );

VOID
BdRestoreBreakpoint (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
BdWriteBreakpoint (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
BdSuspendBreakpoint (
    UINT32 Handle
    );

VOID
BdSuspendAllBreakpoints (
    VOID
    );

LOGICAL
BdLowRestoreBreakpoint (
    __in UINT32 Index
    );

VOID
BdRestoreAllBreakpoints (
    VOID
    );

//
// Memory check functions (check.c)
//

PVOID
BdReadCheck (
    __in PVOID Address
    );

PVOID
BdWriteCheck (
    __in PVOID Address
    );

PVOID
BdTranslatePhysicalAddress (
    __in PHYSICAL_ADDRESS Address
    );

VOID
BdUnmapVirtualAddress(
    __in PVOID VirtualAddress
    );

//
// KD file support (file.c).
//

NTSTATUS
BdCloseRemoteFile (
    __in HANDLE Handle
    );

NTSTATUS
BdCreateRemoteFile (
    __out PHANDLE Handle,
    __out_opt PULONG64 Length,
    __in PUNICODE_STRING FileName,
    __in ACCESS_MASK DesiredAccess,
    __in ULONG FileAttributes,
    __in ULONG ShareAccess,
    __in ULONG CreateDisposition,
    __in ULONG CreateOptions
    );

NTSTATUS
BdReadRemoteFile (
    __in HANDLE Handle,
    __in ULONG64 Offset,
    __out_bcount(Length) PVOID Buffer,
    __in ULONG Length,
    __out PULONG Completed
    );

//
// State change message functions (message.c)
//

LOGICAL
BdReportExceptionStateChange (
    __in PEXCEPTION_RECORD ExceptionRecord,
    __inout PCONTEXT ContextRecord,
    __in BOOLEAN FirstChance
    );

LOGICAL
BdReportLoadSymbolsStateChange (
    __in PSTRING PathName,
    __in PKD_SYMBOLS_INFO SymbolInfo,
    __in LOGICAL UnloadSymbols,
    __inout PCONTEXT ContextRecord
    );

//
// Platform independent debugger APIs (xxapi.c)
//

VOID
BdGetVersion (
    __in PDBGKD_MANIPULATE_STATE64 m
    );

VOID
BdRestoreBreakPointEx (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

NTSTATUS
BdWriteBreakPointEx (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
BdReadPhysicalMemory (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
BdWritePhysicalMemory (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
BdReadVirtualMemory (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
BdWriteVirtualMemory (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
BdGetContext (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
BdGetContextEx (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
BdSetContext (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __out PCONTEXT Context
    );

VOID
BdSetContextEx (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __out PCONTEXT Context
    );

//
// Move memory functions (move.c)
//

ULONG
BdMoveMemory (
    __out_bcount_part(Length, return) volatile PCHAR Destination,
    __in_bcount(Length) volatile PCHAR Source,
    __in UINT32 Length
    );

VOID
BdCopyMemory (
    __out_bcount(Length) volatile PCHAR Destination,
    __in_bcount(Length) volatile PCHAR Source,
    __in UINT32 Length
    );

//
// CPU specific interfaces (cpuapi.c)
//

VOID
BdSetContextState (
    __in PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    __in PCONTEXT ContextRecord
    );

VOID
BdGetStateChange (
    __in PDBGKD_MANIPULATE_STATE64 ManipulateState,
    __in PCONTEXT ContextRecord
    );

VOID
BdSetStateChange (
    __in PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    __in PEXCEPTION_RECORD ExceptionRecord,
    __in PCONTEXT ContextRecord
    );

VOID
BdReadControlSpace (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
BdWriteControlSpace (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
BdReadIoSpace (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
BdWriteIoSpace (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
BdReadMachineSpecificRegister (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
BdWriteMachineSpecificRegister (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

//
// CPU specific trap routines.
//

VOID
BdInstallTrapVectors (
    VOID
    );

VOID
BdUnhandledException(
    VOID
    );

VOID
BdDivideError(
    VOID
    );

VOID
BdDebugTrapOrFault(
    VOID
    );

VOID
BdNmiInterrupt(
    VOID
    );

VOID
BdBreakpointTrap(
    VOID
    );

VOID
BdOverflowTrap(
    VOID
    );

VOID
BdBoundFault(
    VOID
    );

VOID
BdInvalidOpcodeFault(
    VOID
    );

VOID
BdNpxNotAvailableFault(
    VOID
);

VOID
BdGeneralProtectionFault(
    VOID
    );

VOID
BdPageFault(
    VOID
    );

VOID
BdFloatingPointFault(
    VOID
    );

VOID
BdFastFailTrap(
    VOID
    );

VOID
BdAssertionFailureTrap(
    VOID
    );

VOID
BdDebugServiceTrap(
    VOID
    );

VOID
BdAlignmentFault(
    VOID
    );

VOID
BdMachineCheckAbort(
    VOID
    );

VOID
BdXmmException(
    VOID
    );

VOID
BdDoubleFault(
    VOID
    );

VOID
BdInvalidTss(
    VOID
    );

//
// Print and prompt functions (dbgio.c)
//

LOGICAL
BdPrintString (
    __in PSTRING Output
    );

LOGICAL
BdPromptString (
    __in PSTRING Output,
    __inout PSTRING Input
    );

//
// image.c defines.
//

typedef LIST_ENTRY *PLIST_ENTRY;

VOID
BdImageNotification(
    _In_        PLDR_DATA_TABLE_ENTRY   Image,
    _In_        UINT8                   Operation
    );

//
// Processor state functions (copied from ke).
//

VOID
KiRestoreProcessorControlState (
    __in PKPROCESSOR_STATE ProcessorState
    );

VOID
KiSaveProcessorControlState (
    __in PKPROCESSOR_STATE ProcessorState
    );

VOID
EfiCaptureContext (
    __out PCONTEXT ContextRecord
    );

//
// Define debug routine prototypes.
//

typedef
LOGICAL
(*PBD_DEBUG_ROUTINE) (
    __in PEXCEPTION_RECORD ExceptionRecord,
    __in PKEXCEPTION_FRAME ExceptionFrame,
    __in PKTRAP_FRAME TrapFrame
    );

VOID
EfiDispatchException (
    _In_    PEXCEPTION_RECORD   ExceptionRecord,
    _In_    PKEXCEPTION_FRAME   ExceptionFrame,
    _In_    PKTRAP_FRAME        TrapFrame
    );

LOGICAL
BdDispatchException (
    _In_    PEXCEPTION_RECORD   ExceptionRecord,
    _In_    PKEXCEPTION_FRAME   ExceptionFrame,
    _In_    PKTRAP_FRAME        TrapFrame
    );

LOGICAL
BdDispatchStub (
    _In_    PEXCEPTION_RECORD   ExceptionRecord,
    _In_    PKEXCEPTION_FRAME   ExceptionFrame,
    _In_    PKTRAP_FRAME        TrapFrame
    );

VOID
BdRestoreKframe (
    _Inout_ PKTRAP_FRAME        TrapFrame,
    _Inout_ PKEXCEPTION_FRAME   ExceptionFrame,
    _In_    PCONTEXT            ContextRecord
    );

VOID
BdSaveKframe (
    _In_    PKTRAP_FRAME        TrapFrame,
    _In_    PKEXCEPTION_FRAME   ExceptionFrame,
    _Inout_ PCONTEXT            ContextRecord
    );

//
// Define external data.
//

extern LOGICAL BdArchBlockDebuggerOperation;
extern BD_BREAKPOINT_TYPE BdBreakpointInstruction;
extern BREAKPOINT_ENTRY BdBreakpointTable[];
extern LOGICAL BdControlCPending;
extern LOGICAL BdControlCPressed;
extern volatile UINT32 BdDebuggerNotPresent;
extern PBD_DEBUG_ROUTINE BdDebugRoutine;
extern UINT64 BdMessageBuffer[];
extern UINT32 BdNextPacketIdToSend;
extern UINT32 BdNumberRetries;
extern UINT32 BdPacketIdExpected;
extern KPCR BdPcr;
extern PKPRCB BdPrcb;
extern UINT32 BdRetryCount;
extern const UINT32 NtBuildNumber;
extern LIST_ENTRY BdModuleList;
extern UINT32 BdModuleListCount;
extern volatile UINT32 BdDebugPrintGlobalMask;
extern PLDR_DATA_TABLE_ENTRY BdModuleDataTableEntry;
extern KDDEBUGGER_DATA64 BdDebuggerDataBlock;
extern LIST_ENTRY BdDebuggerDataListHead;
extern BOOLEAN BdSubsystemInitialized;
extern BOOLEAN BdSubsystemEnabled;
extern BOOLEAN BdpContextSent;
extern EFI_UNLOADED_MODULE BdUnloadedModules[];
extern ULONG BdLastUnloadedModule;

VOID
BdSerialPrint (
  IN  CONST CHAR8  *Format,
  ...
  );

#if defined(MDE_CPU_AARCH64)
VOID
BlArchSweepIcacheRange (
  __in PVOID BaseAddress,
  __in SIZE_T Length
  );
#endif

