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

#define BREAKPOINT_BREAK           0
#define BREAKPOINT_PRINT           1
#define BREAKPOINT_PROMPT          2
#define BREAKPOINT_LOAD_SYMBOLS    3
#define BREAKPOINT_UNLOAD_SYMBOLS  4
#define BREAKPOINT_COMMAND_STRING  5
#define BREAKPOINT_HW_WATCH        6
#define BREAKPOINT_HW_BREAK        7

#define EXCEPT_NT_ASSERT         0x2C
#define EXCEPT_NT_DEBUG_SERVICE  0x2D

#endif
