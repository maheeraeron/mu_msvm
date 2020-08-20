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
SecInitializeSnp (
    UEFI_IGVM_PARAMETER_INFO *ParameterInfo
    );

#define MSR_GHCB        0xC0010130

VOID
SecVirtualCommunicationExceptionHandler (
    VOID
    );

VOID
SecVmgexit (
    VOID
    );
