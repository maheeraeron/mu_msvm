/*++

Copyright (c) 2015  Microsoft Corporation

Module Name:

    debug.c

Abstract:

    This module implements the DebugService2 routine for Ia32.

Author:

    Cristian Mocanu (t-crimoc) 14-Sep-2015

--*/

VOID*
DebugService2(
    VOID* Arg1,
    VOID* Arg2,
    UINT32 ServiceClass
    )
/*++
 Routine Description:

    This function calls the kernel debugger to execute a command.

 Arguments:

    Param1 - Supplies the first parameter to the KD fault handler

    Param2 - Supplies the second parameter to the KD fault handler

    Service - Supplies the service code of the debugger request.

 Return Value:

    None.
--*/
{
    VOID * retval;
    _asm {
        mov     eax, ServiceClass
        mov     ecx, Arg1
        mov     edx, Arg2

        int     2dh                 ; Raise exception
        int     3                   ; DO NOT REMOVE (See KiDebugService)

        mov retval, eax
    }

    return retval;
}

