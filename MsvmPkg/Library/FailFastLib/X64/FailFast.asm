;++
;
;  This file contains architecture specific functions for debugging a failure.
;
;  Copyright (c) Microsoft Corporation.
;  Licensed under the BSD-2-Clause-Patent license.
;--
include macamd64.inc
;++
;
; VOID
; GenerateDumpAndFailFast(
;    __in    UINT64 ErrorCode,
;    __in    UINT64 Param1,
;    __in    UINT64 Param2,
;    __in    UINT64 Param3,
;    __in    UINT64 Param4
;    );
;
; Routine Description:
;
;   Called when a fatal error is detected and the system cannot continue.
;   It places the four parameters in the respective registers prior to the crash.
;   The Hyper-V worker process logs the VP state which includes these registers.
;
;
; Arguments:
;
;   This function assumes x64 calling conventions and the
;   four arguments are expected in rcx, rdx, r8, and r9 respectively.
;
;   ErrorCode     Bugcheck error code
;   Param1        Bugcheck code specific parameter.
;   Param2        Bugcheck code specific parameter.
;   Param3        Bugcheck code specific parameter.
;   Param4        Bugcheck code specific parameter.
;
; Return Value:
;
;    None.
;
;--
LEAF_ENTRY      GenerateDumpAndFailFast, _TEXT$00
                int     12H
LEAF_END        GenerateDumpAndFailFast, _TEXT$00
end
