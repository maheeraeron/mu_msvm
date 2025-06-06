#------------------------------------------------------------------------------
#
# Debug related direct register handling support functions.
#
# Copyright (C) 2019, Microsoft Corporation. All rights reserved.<BR>
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

GCC_ASM_EXPORT(ReadStatusRegFpcr)
GCC_ASM_EXPORT(WriteStatusRegFpcr)
GCC_ASM_EXPORT(ReadStatusRegMdscrEl1)
GCC_ASM_EXPORT(WriteStatusRegMdscrEl1)
GCC_ASM_EXPORT(WriteStatusRegOslarEl1)

ASM_PFX(ReadStatusRegFpcr):
    mrs x0, fpcr
    ret

ASM_PFX(WriteStatusRegFpcr):
    msr fpcr, x0
    ret

ASM_PFX(ReadStatusRegMdscrEl1):
    mrs x0, mdscr_el1
    ret

ASM_PFX(WriteStatusRegMdscrEl1):
    msr mdscr_el1, x0
    ret

ASM_PFX(WriteStatusRegOslarEl1):
    msr oslar_el1, x0
    ret
