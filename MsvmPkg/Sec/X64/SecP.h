/*++

Copyright (c) Microsoft Corporation

Module Name:

    SecP.h

Abstract:

    Definitions relating to X64 version of the SEC driver.

--*/

#pragma once

extern HV_HYPERVISOR_ISOLATION_CONFIGURATION mIsolationConfiguration;

typedef struct _TRAP_FRAME {
    UINT64 P1;
    UINT64 P2;
    UINT64 P3;
    UINT64 P4;
    UINT64 XmmRegisters[12];
    UINT64 Rax;
    UINT64 Rcx;
    UINT64 Rdx;
    UINT64 Rbx;
    UINT64 R8;
    UINT64 R9;
    UINT64 R10;
    UINT64 R11;
    UINT64 ErrorCode;
    UINT64 Rip;
    UINT64 SegCs;
    UINT64 Rflags;
    UINT64 Rsp;
    UINT64 SegSs;
} TRAP_FRAME, *PTRAP_FRAME;

BOOLEAN
SecInitializeHardwareIsolation (
    _In_ UINT32 IsolationType,
    _In_ UEFI_IGVM_PARAMETER_INFO *ParameterInfo
    );

#define MSR_GHCB        0xC0010130

VOID
SecVirtualCommunicationExceptionHandler (
    VOID
    );

#define VC_EXIT_CODE_CPUID      0x72
#define VC_EXIT_CODE_MSR        0x7C

VOID
SecVmgexit (
    VOID
    );

typedef struct _HV_PSP_CPUID_LEAF
{
    UINT32 EaxIn;
    UINT32 EcxIn;
    UINT64 XfemIn;
    UINT64 XssIn;
    UINT32 EaxOut;
    UINT32 EbxOut;
    UINT32 EcxOut;
    UINT32 EdxOut;
    UINT64 ReservedZ;
} HV_PSP_CPUID_LEAF, *PHV_PSP_CPUID_LEAF;

#define HV_PSP_CPUID_LEAF_COUNT_MAX     64

typedef struct _HV_PSP_CPUID_PAGE
{
    UINT32 Count;
    UINT32 ReservedZ1;
    UINT64 ReservedZ2;
    HV_PSP_CPUID_LEAF CpuidLeafInfo[HV_PSP_CPUID_LEAF_COUNT_MAX];
} HV_PSP_CPUID_PAGE, *PHV_PSP_CPUID_PAGE;


typedef struct _SEC_CPUID_INFO
{
    UINT64 SupportedLeaves;
    UINT32 MaximumLeafIndex;
} SEC_CPUID_INFO;

VOID
SecVirtualizationExceptionHandler (
    VOID
    );

typedef struct _TDX_VE_INFO {
    UINT32 ExitReason;
    UINT32 Valid;
    UINT64 ExitQualification;
    UINT64 GuestLinearAddress;
    UINT64 GuestPhysicalAddress;
    UINT32 InstructionLength;
    UINT32 InstructionInfo;
} TDX_VE_INFO, *PTDX_VE_INFO;

#define VE_EXIT_CODE_CPUID      10   
#define VE_EXIT_CODE_RDMSR      31
#define VE_EXIT_CODE_WRMSR      32

LONG64
SecGetTdxVeInfo(
    _Out_ PTDX_VE_INFO VeInfo
    );

LONG64
SecGetTdInfo(
    _Out_ PUINT32 GpaWidth
    );

UINT64
SecTdCallRdmsr(
    _In_ UINT64 MsrNumber
    );

VOID
SecTdCallWrmsr(
    _In_ UINT64 MsrNumber,
    _In_ UINT64 MsrValue
    );

UINT64
MulDiv64 (
    _In_ UINT64 Value,
    _In_ UINT64 Multiplier,
    _In_ UINT64 Divisor
    );
