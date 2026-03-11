; Copyright (c) Microsoft Corporation.
; SPDX-License-Identifier: BSD-2-Clause-Patent

    default rel
    section .text

;------------------------------------------------------------------------------
; Flushing icache on Intel requires a control tranfer, call/ret or branch.
; An assembly function does the job.
global BlArchSweepIcacheRange
BlArchSweepIcacheRange:
    ret
