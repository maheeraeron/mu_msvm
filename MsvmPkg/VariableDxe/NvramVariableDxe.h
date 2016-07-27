/*++

Copyright (c) Microsoft Corporation

Module Name:

    NvramVariableDxe.h

Abstract:

    Declarations for the Hyper-V NVRAM Variable Services driver. 

Author:

    Larry Cleeton (lcleeton) - 07-Mar-2013

--*/

#pragma once

#include <EfiNt.h>
#include <BiosInterface.h>


extern
EFI_STATUS
NvramInitialize();

extern
VOID
NvramAddressChangeHandler();

extern
VOID
NvramExitBootServicesHandler(
    __in BOOLEAN VsmAware
    );

extern
EFI_STATUS
NvramQueryInfo(
    __in  UINT32  Attributes,
    __out UINT64* MaximumVariableStorageSize,
    __out UINT64* RemainingVariableStorageSize,
    __out UINT64* MaximumVariableSize  
    );

extern
EFI_STATUS
NvramSetVariable(
    __in CHAR16*   VariableName,
    __in EFI_GUID* VendorGuid,
    __in UINT32    Attributes,
    __in UINTN     DataSize,
    __in void*     Data
    );

extern
EFI_STATUS
NvramGetVariable(
    __in           CHAR16*   VariableName,
    __in           EFI_GUID* VendorGuid,
    __out OPTIONAL UINT32*   Attributes,
    __inout        UINTN*    DataSize,
    __out          void*     Data
    );

extern
EFI_STATUS
NvramGetFirstVariableName(
    __out UINTN*    VariableNameSize,
    __out CHAR16*   VariableName,
    __out EFI_GUID* VendorGuid
    );

extern
EFI_STATUS
NvramGetNextVariableName(
    __inout UINTN*    VariableNameSize,
    __inout CHAR16*   VariableName,
    __inout EFI_GUID* VendorGuid    
    );

extern
VOID
NvramDebugLog(
    __in CONST CHAR8 *Format,
    ...
    );


