/*++

Copyright (c) Microsoft Corporation

Module Name:

    IsolationTypes.h

Abstract:

    This file contains types and constants describing VM isolation.

--*/

#pragma once

enum
{
    UefiIsolationTypeNone       = 0x00,
    UefiIsolationTypeVbs        = 0x01,
    UefiIsolationTypeSnp        = 0x02,
    UefiIsolationTypeTdx        = 0x03,
};

UINT32
GetIsolationType();

BOOLEAN
IsParavisorPresent();

BOOLEAN
IsIsolatedEx(
    UINT32 IsolationType
    );

BOOLEAN 
IsIsolated();

BOOLEAN
IsHardwareIsolatedEx(
    UINT32 IsolationType
    );

BOOLEAN 
IsHardwareIsolated();

BOOLEAN
IsSoftwareIsolatedEx(
    UINT32 IsolationType
    );

BOOLEAN 
IsSoftwareIsolated();

BOOLEAN
IsHardwareIsolatedNoParavisorEx(
    UINT32 IsolationType,
    BOOLEAN IsParavisorPresent
    );

BOOLEAN 
IsHardwareIsolatedNoParavisor();