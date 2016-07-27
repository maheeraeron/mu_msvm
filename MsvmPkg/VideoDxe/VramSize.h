/*++

Copyright (c) Microsoft Corporation

Module Name:

    VramSize.h

Abstract:

    VRAM size definitions.

Author:

    Bhanu Gogineni (bhanug) 10-Oct-2012

--*/

#pragma once

#define DEFAULT_VRAM_SIZE_WIN7 (4 * 1024 * 1024)

//
// In Win8 the synthetic video device upgraded the color depth capabilty from
// 16 to 32 bits per pixel.
//

#define DEFAULT_VRAM_SIZE_WIN8 (2 * DEFAULT_VRAM_SIZE_WIN7)
