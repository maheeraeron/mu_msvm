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
    IN unsigned char *CommandLine
    );

VOID
LoadKdNet(
    IN EFI_PEI_FILE_HANDLE FileHandle
    );
