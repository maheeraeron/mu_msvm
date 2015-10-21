/*----------------------------------------------------------------------------
 $Microsoft Confidential$
 $Copyright (C) 2004 Microsoft Corporation.  All Rights Reserved.$

 $File: Transportp.h $

 Abstract:

     This file contains the structures that are used internally within
     the packet management library.

----------------------------------------------------------------------------*/

#pragma once

#ifndef VMBUS_RING_BUFFER_SINGLE_MAPPED
#include <ntddk.h>
#endif

#include <vmbuspacketinterface.h>
#include <vmbuspacketformat.h>
#include <hvgdk.h>

NTSTATUS
PkpInitRingBufferControl(
    __inout PPACKET_LIB_CONTEXT Context
    );

