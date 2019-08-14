///
/// \copyright  Copyright (c) Microsoft Corporation. All Rights Reserved.
///
/// \file PciBars.h
///
/// \brief Helper definition for PCI BARs defined in the PCI specification.
///
/// \author Chris Oo (cho)
/// \date Aug 9, 2019
///

#pragma once

#include <Base.h>

// Attribute types for BARs. See PCI Local Bus Specification Revision 3.0, section 6.2.5.1
typedef struct _PCI_BAR_FORMAT
{
    union {
        struct
        {
            UINT32 MemorySpaceIndicator:1;
            UINT32 MemoryType:2;
            UINT32 Prefetchable:1;
            UINT32 Address:28;
        } Memory;

        struct
        {
            UINT32 IoSpaceIndicator:1;
            UINT32 Reserved:1;
            UINT32 Address:30;
        };

        UINT32 AsUINT32;
    };
} PCI_BAR_FORMAT;

#define PCI_BAR_MEMORY_SPACE 0

#define PCI_BAR_MEMORY_TYPE_32BIT 0x0
#define PCI_BAR_MEMORY_TYPE_64BIT 0x2