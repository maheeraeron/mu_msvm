/*++

Copyright (c) 1990-2003  Microsoft Corporation

Module Name:

    EFI_KD.h

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

#define _AMD64_

//#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#define UNALIGNED

#include <EfiNt.h>
#include <Library/KdDebugLib.h>

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

typedef struct _STRING
{
    USHORT Length;
    USHORT MaximumLength;
    CHAR *Buffer;
} STRING, *PSTRING;

typedef struct _UNICODE_STRING
{
    UINT16 Length;
    UINT16 MaximumLength;
    CHAR16 *Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

#define UNICODE_NULL ((CHAR16)(0))

#include "cpu.h"
#include "EfiKdHelper.h"

//#include "ntverp.h"
#define VER_PRODUCTBUILD 9600

//#include "kdbgnet.h"
#define KD_NET_KEY_SIZE_DWORDS 8
#define KD_NET_KEY_SIZE (KD_NET_KEY_SIZE_DWORDS * sizeof(ULONG))

//
// Size of unloaded module array
// Must match MI_UNLOADED_DRIVERS in ntos\mm\mi.h
//
#define EFI_KD_MAX_UNLOADED_MODULES 50

//
// Define message buffer size in bytes.
//
// N.B. This must be 0 mod 8.
//

#define EFI_KD_MESSAGE_BUFFER_SIZE 4096
#define EFI_KD_MAXIMUM_FILENAME_SIZE 1024

//
// Define the maximum number of retries for packet sends.
//

#define MAXIMUM_RETRIES 20

//
// Define packet waiting status codes.
//

#define EFI_KD_PACKET_RECEIVED 0
#define EFI_KD_PACKET_TIMEOUT  1
#define EFI_KD_PACKET_RESEND   2

//
// Define break point table entry structure.
//

#define EFI_KD_BREAKPOINT_IN_USE         0x1
#define EFI_KD_BREAKPOINT_NEEDS_WRITE    0x2
#define EFI_KD_BREAKPOINT_SUSPENDED      0x4
#define EFI_KD_BREAKPOINT_NEEDS_REPLACE  0x8

typedef struct _BREAKPOINT_ENTRY
{
    ULONG              Flags;
    ULONG64            Address;
    EFI_KD_BREAKPOINT_TYPE Content;
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
ULONG
(*PEFI_KD_RECEIVE_PACKET) (
    __in UINT32 ExpectedPacketType,
    __out_opt PSTRING MessageHeader,
    __out_opt PSTRING MessageData,
    __out_opt PUINT32 DataLength
    );

typedef
VOID
(*PEFI_KD_SEND_PACKET) (
    __in UINT32 PacketType,
    __in PSTRING MessageHeader,
    __in_opt PSTRING MessageData
    );

extern PEFI_KD_RECEIVE_PACKET EfiKdReceivePacket;
extern PEFI_KD_SEND_PACKET EfiKdSendPacket;

//
// Define debug device connection parameters.
//

typedef enum
{
    EfiKdNone,
    EfiKdSerial,
    EfiKdNet
} EFI_KD_CONNECTION_TYPE;

typedef struct _EFI_KD_CONNECTION_PARAMETERS
{
    EFI_KD_CONNECTION_TYPE Type;

    struct
    {
        ULONG AlternatePortNumber;
        ULONGLONG BaudRate;
    } SerialPort;

    struct
    {
        ULONG TargetIP;
        ULONG HostIP;
        USHORT Port;
        ULONG Bus;
        ULONG Slot;
        BOOLEAN Dhcp;
        BOOLEAN EncryptedLink;
        BOOLEAN Vm;
        UCHAR Key[KD_NET_KEY_SIZE];
    } Net;
} EFI_KD_CONNECTION_PARAMETERS, *PEFI_KD_CONNECTION_PARAMETERS;

extern EFI_KD_CONNECTION_TYPE EfiKdDebuggerType;

//
// Define function prototypes.
//

#define PASS_KERNEL_LARGE_MAPPINGS 1
#define DONT_FREE_LARGE_MAPPINGS 2

// ----------------------------------------------------------------- Prototypes

BOOLEAN
EfiKdDebuggerEnabled (
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
EfiKdInitialize (
    VOID
    );

/*++

Routine Description:

    Initializes the boot debugger for the executing boot application.

Arguments:

    None.

Return Value:

    STATUS_SUCCESS when successful.
    STATUS_NOT_FOUND if the debugger port does not exist.

--*/

VOID
EfiKdStart (
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
EfiKdStop (
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
EfiKdPatchIdt (
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

LOGICAL
EfiKdPollBreakIn (
    VOID
    );

VOID
EfiKdBreakPointWithStatus(
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
EfiKdArchInitialize (
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
EfiKdArchDestroy (
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
EfiKdArchStart (
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
EfiKdArchStop (
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
EfiKdDebuggerInitialized (
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
EfiKdComCloseDebuggerDevice (
    VOID
    );

NTSTATUS
EfiKdComConfigureDebuggerDevice (
    __in PEFI_KD_CONNECTION_PARAMETERS Parameters
    );

NTSTATUS
EfiKdComGetConnectionParameters (
    __out PEFI_KD_CONNECTION_PARAMETERS Parameters
    );

UINT32
EfiKdComReceivePacket (
    __in UINT32 ExpectedPacketType,
    __out_opt PSTRING MessageHeader,
    __out_opt PSTRING MessageData,
    __out_opt PUINT32 DataLength
    );

VOID
EfiKdComSendPacket (
    __in UINT32 PacketType,
    __in PSTRING MessageHeader,
    __in_opt PSTRING MessageData
    );

UINT64
EfiKdComSentReceivedPacketCount(
    VOID
    );


//
// Breakpoint functions (break.c).
//

UINT32
EfiKdAddBreakpoint (
    __in ULONG64 Address
    );

LOGICAL
EfiKdDeleteBreakpoint (
    __in UINT32 Handle
    );

LOGICAL
EfiKdDeleteBreakpointRange (
    __in UINT64 Lower,
    __in UINT64 Upper
    );

VOID
EfiKdRestoreBreakpoint (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
EfiKdWriteBreakpoint (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
EfiKdSuspendBreakpoint (
    UINT32 Handle
    );

VOID
EfiKdSuspendAllBreakpoints (
    VOID
    );

LOGICAL
EfiKdLowRestoreBreakpoint (
    __in UINT32 Index
    );

VOID
EfiKdRestoreAllBreakpoints (
    VOID
    );

//
// Memory check functions (check.c)
//

PVOID
EfiKdReadCheck (
    __in PVOID Address
    );

PVOID
EfiKdWriteCheck (
    __in PVOID Address
    );

PVOID
EfiKdTranslatePhysicalAddress (
    __in PHYSICAL_ADDRESS Address
    );

VOID
EfiKdUnmapVirtualAddress(
    __in PVOID VirtualAddress
    );

//
// KD file support (file.c).
//

NTSTATUS
EfiKdCloseRemoteFile (
    __in HANDLE Handle
    );

NTSTATUS
EfiKdCreateRemoteFile (
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
EfiKdReadRemoteFile (
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
EfiKdReportExceptionStateChange (
    __in PEXCEPTION_RECORD ExceptionRecord,
    __inout PCONTEXT ContextRecord,
    __in BOOLEAN FirstChance
    );

LOGICAL
EfiKdReportLoadSymbolsStateChange (
    __in PSTRING PathName,
    __in PKD_SYMBOLS_INFO SymbolInfo,
    __in LOGICAL UnloadSymbols,
    __inout PCONTEXT ContextRecord
    );

//
// Platform independent debugger APIs (xxapi.c)
//

VOID
EfiKdGetVersion (
    __in PDBGKD_MANIPULATE_STATE64 m
    );

VOID
EfiKdRestoreBreakPointEx (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

NTSTATUS
EfiKdWriteBreakPointEx (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
EfiKdReadPhysicalMemory (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
EfiKdWritePhysicalMemory (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
EfiKdReadVirtualMemory (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
EfiKdWriteVirtualMemory (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
EfiKdGetContext (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
EfiKdSetContext (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __out PCONTEXT Context
    );

//
// Move memory functions (move.c)
//

ULONG
EfiKdMoveMemory (
    __out_bcount_part(Length, return) volatile PCHAR Destination,
    __in_bcount(Length) volatile PCHAR Source,
    __in UINT32 Length
    );

VOID
EfiKdCopyMemory (
    __out_bcount(Length) volatile PCHAR Destination,
    __in_bcount(Length) volatile PCHAR Source,
    __in UINT32 Length
    );

//
// CPU specific interfaces (cpuapi.c)
//

VOID
EfiKdSetContextState (
    __in PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    __in PCONTEXT ContextRecord
    );

VOID
EfiKdGetStateChange (
    __in PDBGKD_MANIPULATE_STATE64 ManipulateState,
    __in PCONTEXT ContextRecord
    );

VOID
EfiKdSetStateChange (
    __in PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    __in PEXCEPTION_RECORD ExceptionRecord,
    __in PCONTEXT ContextRecord
    );

VOID
EfiKdReadControlSpace (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __out PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
EfiKdWriteControlSpace (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
EfiKdReadIoSpace (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
EfiKdWriteIoSpace (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
EfiKdReadMachineSpecificRegister (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

VOID
EfiKdWriteMachineSpecificRegister (
    __in PDBGKD_MANIPULATE_STATE64 m,
    __in PSTRING AdditionalData,
    __in PCONTEXT Context
    );

//
// CPU specific trap routines.
//

VOID
EfiKdInstallTrapVectors (
    VOID
    );

VOID
EfiKdUnhandledException(
    VOID
    );

VOID
EfiKdDivideError(
    VOID
    );

VOID
EfiKdDebugTrapOrFault(
    VOID
    );

VOID
EfiKdNmiInterrupt(
    VOID
    );

VOID
EfiKdBreakpointTrap(
    VOID
    );

VOID
EfiKdOverflowTrap(
    VOID
    );

VOID
EfiKdBoundFault(
    VOID
    );

VOID
EfiKdInvalidOpcodeFault(
    VOID
    );

VOID
EfiKdNpxNotAvailableFault(
    VOID
);

VOID
EfiKdGeneralProtectionFault(
    VOID
    );

VOID
EfiKdPageFault(
    VOID
    );

VOID
EfiKdFloatingPointFault(
    VOID
    );

VOID
EfiKdFastFailTrap(
    VOID
    );

VOID
EfiKdAssertionFailureTrap(
    VOID
    );

VOID
EfiKdDebugServiceTrap(
    VOID
    );

VOID
EfiKdAlignmentFault(
    VOID
    );

VOID
EfiKdMachineCheckAbort(
    VOID
    );

VOID
EfiKdXmmException(
    VOID
    );

VOID
EfiKdDoubleFault(
    VOID
    );

VOID
EfiKdInvalidTss(
    VOID
    );

//
// Print and prompt functions (dbgio.c)
//

LOGICAL
EfiKdPrintString (
    __in PSTRING Output
    );

LOGICAL
EfiKdPromptString (
    __in PSTRING Output,
    __inout PSTRING Input
    );

//
// image.c defines.
//

typedef LIST_ENTRY *PLIST_ENTRY;

VOID
EfiKdImageNotification(
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
(*PEFI_KD_DEBUG_ROUTINE) (
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
EfiKdDispatchException (
    _In_    PEXCEPTION_RECORD   ExceptionRecord,
    _In_    PKEXCEPTION_FRAME   ExceptionFrame,
    _In_    PKTRAP_FRAME        TrapFrame
    );

LOGICAL
EfiKdDispatchStub (
    _In_    PEXCEPTION_RECORD   ExceptionRecord,
    _In_    PKEXCEPTION_FRAME   ExceptionFrame,
    _In_    PKTRAP_FRAME        TrapFrame
    );

VOID
EfiKdRestoreKframe (
    _Inout_ PKTRAP_FRAME        TrapFrame,
    _Inout_ PKEXCEPTION_FRAME   ExceptionFrame,
    _In_    PCONTEXT            ContextRecord
    );

VOID
EfiKdSaveKframe (
    _In_    PKTRAP_FRAME        TrapFrame,
    _In_    PKEXCEPTION_FRAME   ExceptionFrame,
    _Inout_ PCONTEXT            ContextRecord
    );

//
// Define external data.
//

extern LOGICAL EfiKdArchBlockDebuggerOperation;
extern EFI_KD_BREAKPOINT_TYPE EfiKdBreakpointInstruction;
extern BREAKPOINT_ENTRY EfiKdBreakpointTable[];
extern LOGICAL EfiKdControlCPending;
extern LOGICAL EfiKdControlCPressed;
extern volatile LOGICAL EfiKdDebuggerNotPresent;
extern PEFI_KD_DEBUG_ROUTINE EfiKdDebugRoutine;
extern UINT64 EfiKdMessageBuffer[];
extern UINT32 EfiKdNextPacketIdToSend;
extern UINT32 EfiKdNumberRetries;
extern UINT32 EfiKdPacketIdExpected;
extern KPCR EfiKdPcr;
extern PKPRCB EfiKdPrcb;
extern UINT32 EfiKdRetryCount;
extern const UINT32 NtBuildNumber;
extern LIST_ENTRY EfiKdModuleList;
extern UINT32 EfiKdModuleListCount;
extern UINT32 EfiKdDebugPrintGlobalMask;
extern PLDR_DATA_TABLE_ENTRY EfiKdModuleDataTableEntry;
extern KDDEBUGGER_DATA64 EfiKdDebuggerDataBlock;
extern LIST_ENTRY EfiKdDebuggerDataListHead;
extern BOOLEAN EfiKdSubsystemInitialized;
extern BOOLEAN EfiKdSubsystemEnabled;
extern EFI_UNLOADED_MODULE EfiKdUnloadedModules[];
extern ULONG EfiKdLastUnloadedModule;

#if defined(MDE_CPU_IA32)

// This points to the EfiKdTrap procedure and is being called from trapa.asm for Ia32.
extern UINT32 EfiKdTrapRoutine;

#endif

