/**@file Exception.h

This file contains definitions related to exception handling from within KdDxe.efi.


Copyright (c) 2018, Microsoft Corporation

All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#define KI_EXCEPTION_INVALID_OP          0x10000002
#define STATUS_INSTRUCTION_MISALIGNMENT  0xc00000aa
#define STATUS_DATATYPE_MISALIGNMENT     0x80000002
#define STATUS_BREAKPOINT                0x80000003
#define STATUS_SINGLE_STEP               0x80000004
#define STATUS_ACCESS_VIOLATION          0xc0000005
#define STATUS_ASSERTION_FAILURE         0xC0000420
#define STATUS_STACK_BUFFER_OVERRUN      0xC0000409
#define STATUS_ILLEGAL_INSTRUCTION       0xC000001D
#define STATUS_DEVICE_NOT_CONNECTED      0xC000009D

#define ARM64_BREAK_DEBUG_BASE  0xf000
#define ARM64_BREAKPOINT        (ARM64_BREAK_DEBUG_BASE + 0)
#define ARM64_ASSERT            (ARM64_BREAK_DEBUG_BASE + 1)
#define ARM64_DEBUG_SERVICE     (ARM64_BREAK_DEBUG_BASE + 2)
#define ARM64_FASTFAIL          (ARM64_BREAK_DEBUG_BASE + 3)
#define ARM64_DIVIDE_BY_0       (ARM64_BREAK_DEBUG_BASE + 4)

//
// DAIF enable/disables
//

#define DAIF_DEBUG  0x200
#define DAIF_ABORT  0x100
#define DAIF_INT    0x80
#define DAIF_FIQ    0x40

#define ARM64_SYSREG(op0, op1, crn, crm, op2) \
        ( ((op0 & 1) << 14) | \
          ((op1 & 7) << 11) | \
          ((crn & 15) << 7) | \
          ((crm & 15) << 3) | \
          ((op2 & 7) << 0) )

//
// FPCR exception control
//

#define ARM64_FPCR  ARM64_SYSREG(3,3, 4, 4,0)   // Floating point control register (EL0)
#define FPCR_IDE    0x00008000
#define FPCR_IXE    0x00001000
#define FPCR_UFE    0x00000800
#define FPCR_OFE    0x00000400
#define FPCR_DZE    0x00000200
#define FPCR_IOE    0x00000100

//
// Debug registers.
//
#define ARM64_OSLAR_EL1     ARM64_SYSREG(2,0, 1, 0,4)      // OS lock access register
#define ARM64_MDSCR_EL1     ARM64_SYSREG(2,0, 0, 2,2)      // Debug Status and Control Register (external view) [CP14_DBGDSCR]
#define ARM64_MDSCR_RXfull  0x40000000
#define ARM64_MDSCR_TXfull  0x20000000
#define ARM64_MDSCR_MDE     0x00008000
#define ARM64_MDSCR_HDE     0x00004000
#define ARM64_MDSCR_KDE     0x00002000
#define ARM64_MDSCR_TDCC    0x00001000
#define ARM64_MDSCR_SS      0x00000001

UINT64
ReadStatusRegFpcr (
  );

VOID  WriteStatusRegFpcr (UINT64);
UINT64
ReadStatusRegMdscrEl1 (
  );

VOID  WriteStatusRegMdscrEl1 (UINT64);
VOID  WriteStatusRegOslarEl1 (UINT64);

#define BREAKPOINT_BREAK           0
#define BREAKPOINT_PRINT           1
#define BREAKPOINT_PROMPT          2
#define BREAKPOINT_LOAD_SYMBOLS    3
#define BREAKPOINT_UNLOAD_SYMBOLS  4
#define BREAKPOINT_COMMAND_STRING  5
#define BREAKPOINT_HW_WATCH        6
#define BREAKPOINT_HW_BREAK        7

// ARM64_WORKITEM: Update these to match new behaviour
#define SWFS_WRITE        0x01
#define SWFS_EXECUTE      0x08
#define SWFS_PAGE_FAULT   0x10
#define SWFS_ALIGN_FAULT  0x20
#define SWFS_HWERR_FAULT  0x40
#define SWFS_DEBUG_FAULT  0x80

#endif
