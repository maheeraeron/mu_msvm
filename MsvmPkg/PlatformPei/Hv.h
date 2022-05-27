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
