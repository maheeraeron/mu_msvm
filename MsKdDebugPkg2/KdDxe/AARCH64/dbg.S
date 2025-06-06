#------------------------------------------------------------------------------
#
# Debug register level support functions
#
# Copyright (c) 2006 - 2009, Intel Corporation. All rights reserved.<BR>
# Portions copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
# Portions copyright (c) 2011 - 2013, ARM Ltd. All rights reserved.<BR>
# Copyright (C) 2019, Micrisoft Corporation.  All rights reserved.<BR>
#
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php.
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#------------------------------------------------------------------------------

.text
.p2align 2
GCC_ASM_EXPORT(DebugService2)
GCC_ASM_EXPORT(KdDxeKdBreakPointWithStatus)

ASM_PFX(DebugService2):
  mov     x16, x2       // set debug service type
  brk     #0xf002       // call debug service
  brk     #0xf000       // required - do not remove
  ret

ASM_PFX(KdDxeKdBreakPointWithStatus):
  brk     #0xf000
  ret
