/*++

Copyright (c) Microsoft Corporation

Module Name:

    KdNet.h

Abstract:

    Definitions for loading and configuring the KDNET debugger transport.

--*/

#pragma once

extern BOOLEAN UseKdNetDebugger;

VOID
ParseKdNetParameters(
    _In_z_ PUCHAR CommandLine
    );

VOID
LoadKdNet(
    _In_ EFI_PEI_FILE_HANDLE FileHandle
    );
