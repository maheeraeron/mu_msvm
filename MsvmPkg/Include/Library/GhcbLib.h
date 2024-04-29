/*++

Copyright (c) Microsoft Corporation

Module Name:

    GhcbLib.h

Abstract:

    Definitions for functionality available through GHCB calls to the host.

Author:

    Jon Lange (jlange) 7-Feb-2024

--*/

#pragma once

#include <EfiNt.h>

/*++

Routine Description:

    This routine executes the VMGEXIT instruction.

Arguments:

    None.

Return Value:

    None.

--*/
VOID
_sev_vmgexit(
    VOID
    );


/*++

Routine Description:

    This routine initializes the GHCB on an SNP system.

Arguments:

    None.

Return Value:

    Pointer to the GHCB.

--*/
PVOID
GhcbInitializeGhcb(
    VOID
    );


/*++

Routine Description:

    This routine writes an MSR using the GHCB protocol.

Arguments:

    Ghcb - Supplies a pointer to the GHCB.

    MsrNumber - Supplies the MSR number.

    RegisterValue - Supplies the value to write to the MSR.

Return Value:

    None.

--*/
VOID
GhcbWriteMsr(
    _In_ PVOID Ghcb,
    _In_ UINT64 MsrNumber,
    _In_ UINT64 RegisterValue
    );


/*++

Routine Description:

    This routine reads an MSR using the GHCB protocol.

Arguments:

    Ghcb - Supplies a pointer to the GHCB.

    MsrNumber - Supplies the MSR number.

    RegisterValue - Supplies a pointer to a variable that will receive the
        value of the MSR.

Return Value:

    None.

--*/
VOID
GhcbReadMsr(
    _In_ PVOID Ghcb,
    _In_ UINT64 MsrNumber,
    _Out_ UINT64* RegisterValue
    );
