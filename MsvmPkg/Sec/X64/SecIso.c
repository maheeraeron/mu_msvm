/*++

Copyright (c) Microsoft Corporation

Module Name:

    SecIso.c

Abstract:

    Routines to support hardware isolation of the SEC driver.

--*/

#include <Library/UefiCpuLib.h>
#include <EfiNt.h>
#include "SecP.h"

BOOLEAN
SecProcessVirtualCommunicationException (
    _In_ PTRAP_FRAME TrapFrame
    )
{
    UNREFERENCED_PARAMETER(TrapFrame);

    return FALSE;
}
