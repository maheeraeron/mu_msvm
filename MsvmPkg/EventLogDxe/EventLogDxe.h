/** @file
  Internal include file the Event Log Runtime DXE Driver.

  Copyright (c) Microsoft Corporation.
  Licensed under the BSD-2-Clause-Patent license.
**/

#pragma once

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
