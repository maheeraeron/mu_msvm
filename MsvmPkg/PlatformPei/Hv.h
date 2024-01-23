/*++

Copyright (c) Microsoft Corporation

Module Name:

    Hv.h

Abstract:

    Hypervisor interactions during PEI.

--*/

#pragma once

#include <EfiNt.h>
#include <hvhdk.h>
#include <hvgdk.h>

extern BOOLEAN mParavisorPresent;
extern UINT32 mIsolationType;
extern UINT32 mSharedGpaBit;

VOID
HvDetectIsolation(
    VOID
    );

VOID
HvDetectSvsm(
    IN PVOID SecretsPage,
    OUT PUINT64 SvsmBase,
    OUT PUINT64 SvsmSize
    );

typedef struct _SNP_SECRETS {
    UINT8 Reserved[0x140];
    UINT64 SvsmBase;
    UINT64 SvsmSize;
    UINT64 SvsmCallingArea;
} SNP_SECRETS, *PSNP_SECRETS;
