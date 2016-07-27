/*++

Copyright (c) Microsoft Corporation

Module Name:

    StatusCode.h

Abstract:

    Status code driver.  Implements the EFI_STATUS_CODE_PROTOCOL and logs
    events to an event log channel.

Author:

    Kris Harper (kharp) - 12-Dec-2013

ATTENTION - THIS FILE CONTAINS THIRD PARTY OPEN SOURCE CODE: 
    IntelFrameworkModulePkg\Include\Framework\StatusCode.h

IT IS CLEARED ONLY FOR LIMITED USE BY WINDOWS CORE HYPER-V FOR THE HYPER-V ROLE
IN THE WINDOWS PRODUCT. DO NOT USE OR SHARE THIS CODE WITHOUT APPROVAL PURSUANT
TO THE MICROSOFT OPEN SOURCE SOFTWARE APPROVAL POLICY.

  Copyright (c) 2006 - 2009, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

--*/

#pragma once

EFI_STATUS
EFIAPI
StatusCodeRuntimeInitialize();
