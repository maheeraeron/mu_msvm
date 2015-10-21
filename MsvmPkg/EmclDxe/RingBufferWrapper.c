/*++

Copyright (c) Microsoft Corporation

Module Name:

    RingBufferWrapper.c

Abstract:

    This module wraps the VMBus packet library C files with definitions
    to allow it to compile in the UEFI environment.

Author:

    John Starks (jostarks) - 31-Jul-2012

--*/

#include <PiDxe.h>
#include <EfiNt.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#define PAGED_CODE()
#define PrefetchForWrite(x)

#define NT_ASSERT ASSERT
#define NT_VERIFY(x) x

#define RtlZeroMemory(x, y) ZeroMem(x, y)
#define RtlCopyMemory(x, y, z) CopyMem(x, y, z)

#define ALIGN_UP(x, y) ALIGN_VALUE((x), sizeof(y))
#define PAGE_SIZE EFI_PAGE_SIZE

#define VMBUS_RING_BUFFER_SINGLE_MAPPED 1

#include "Init.c"
#include "RingBuffer.c"

