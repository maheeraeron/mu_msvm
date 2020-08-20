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

VOID
HvDetectIsolation(
    VOID
    );
