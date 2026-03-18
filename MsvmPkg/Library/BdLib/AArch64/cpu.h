/*++

Copyright (c) 2004  Microsoft Corporation

Module Name:

    cpu.h

Abstract:

    Machine specific kernel debugger data types and constants.

Author:

    Aaron Giles (aarongi) 11-Jan-2013

--*/

#pragma once

#define BD_BREAKPOINT_TYPE  ULONG
#define BD_BREAKPOINT_SIZE  4
#define BD_BREAKPOINT_ALIGN 1
#define BD_BREAKPOINT_VALUE (0xd4200000 | (0xf000 << 5))
