/*++

Copyright (c) Microsoft Corporation

Module Name:

    HvHyperCall.h

Abstract:

    Low level hypercalls.

--*/

HV_HYPERCALL_OUTPUT
AsmHyperCall(
    _In_      HV_HYPERCALL_INPUT    InputControl,
    _In_opt_  UINT64                InputPhysicalAddress,
    _In_opt_  UINT64                OutputPhysicalAddress
);

HV_STATUS
AsmGetVpRegister64(
     _In_  UINT32      RegisterIndex,
     _Out_ PUINT64     RegisterBuffer
);

HV_STATUS
AsmSetVpRegister64(
     _In_  UINT32      RegisterIndex,
     _In_  UINT64      RegisterBuffer
);


