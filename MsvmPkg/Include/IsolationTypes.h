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
};
