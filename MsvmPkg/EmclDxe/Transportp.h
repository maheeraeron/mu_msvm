/** @file
  This file contains the structures that are used internally within
  the packet management library.

  Copyright (c) Microsoft Corporation.
  Licensed under the BSD-2-Clause-Patent license.
**/

#pragma once

#include <vmbuspacketinterface.h>
#include <vmbuspacketformat.h>

NTSTATUS
PkpInitRingBufferControl(
    IN OUT PPACKET_LIB_CONTEXT Context
    );
