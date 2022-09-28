/*++

Copyright (c) Microsoft Corporation

Module Name:

    EventLogDxe.h

Abstract:

    Internal include file the Event Log Runtime DXE Driver.

Author:

    Kris Harper (kharp) - 20-Nov-2013

--*/

#pragma once

#include <EfiNt.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BootEventLogLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/EfiHv.h>
#include <Protocol/EventLog.h>
