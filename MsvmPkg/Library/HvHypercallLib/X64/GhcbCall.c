/*++

Copyright (c) Microsoft Corporation

Module Name:

    GhcbCall.c

Abstract:

    This file implements support routines for GHCB-based calls.

--*/

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HvHypercallLib.h>

#include <HvHypercallLibP.h>

#define SetGhcbField16(Ghcb, Field, Value) \
    (*(PUINT16)((PUCHAR)(Ghcb) + (Field)) = (Value))
#define SetGhcbField32(Ghcb, Field, Value) \
    (*(PUINT32)((PUCHAR)(Ghcb) + (Field)) = (Value))
#define SetGhcbField64(Ghcb, Field, Value) \
    (*(PUINT64)((PUCHAR)(Ghcb) + (Field)) = (Value))
#define GetGhcbField64(Ghcb, Field) \
    (*(PUINT64)((PUCHAR)(Ghcb) + (Field)))

#define GHCB_EXITCODE_MSR               0x7C

#define GHCB_FIELD64_RAX                0x1F8
#define GHCB_FIELD64_RCX                0x308
#define GHCB_FIELD64_RDX                0x310
#define GHCB_FIELD64_EXITCODE           0x390
#define GHCB_FIELD64_EXITINFO1          0x398
#define GHCB_FIELD16_VERSION            0xFFA
#define GHCB_FIELD32_FORMAT             0xFFC
#define GHCB_FIELD64_HYPERCALL_CODE     0xFF0
#define GHCB_FIELD64_HYPERCALL_OUTPUT   0xFE8

#define HV_STATUS_INVALID_HYPERCALL_CODE ((HV_STATUS)0x0002)
#define HV_STATUS_INVALID_PARAMETER      ((HV_STATUS)0x0005)
#define HV_STATUS_TIMEOUT                ((HV_STATUS)0x0078)

VOID
_sev_vmgexit(
    VOID
    );

VOID
HvHypercallpSetMsrWithGhcb(
    _In_ HV_HYPERCALL_CONTEXT *Context,
    _In_ UINT64 MsrNumber,
    _In_ UINT64 RegisterValue
    )
{
    //
    // Initialize the GHCB page to indicate a request to set the specified
    // MSR.
    //

    SetGhcbField64(Context->Ghcb, GHCB_FIELD64_EXITCODE, GHCB_EXITCODE_MSR);
    SetGhcbField64(Context->Ghcb, GHCB_FIELD64_EXITINFO1, 1);
    SetGhcbField64(Context->Ghcb, GHCB_FIELD64_RCX, MsrNumber);
    SetGhcbField64(Context->Ghcb, GHCB_FIELD64_RAX, (UINT32)RegisterValue);
    SetGhcbField64(Context->Ghcb, GHCB_FIELD64_RDX, (RegisterValue >> 32));
    SetGhcbField32(Context->Ghcb, GHCB_FIELD32_FORMAT, 0);
    SetGhcbField16(Context->Ghcb, GHCB_FIELD16_VERSION, 1);
    _sev_vmgexit();
}


HV_STATUS
HvHypercallpIssueGhcbHypercall(
    _In_ HV_HYPERCALL_CONTEXT *Context,
    _In_ HV_CALL_CODE CallCode,
    _In_opt_ VOID *InputPage,
    _In_ UINT32 CountOfElements,
    _Out_opt_ PUINT32 ElementsProcessed
    )
{
    UINT32 headerSize;
    HV_HYPERCALL_INPUT hypercallInput;
    HV_HYPERCALL_OUTPUT hypercallOutput;
    UINT32 inputSize;
    UINT32 repSize;
    HV_STATUS status;

    //
    // Copy the input page if required.  In order to minimize the amount of
    // data exposed, only the amount of input specified by the call code and
    // rep count are copied to the GHCB.  This means that only specifically
    // approved hypercalls can be made, so the calculation can be done
    // correctly.
    //

    if (InputPage != NULL)
    {
        switch (CallCode)
        {
        case HvCallPostMessage:
            {
                PHV_INPUT_POST_MESSAGE input;
                input = InputPage;
                headerSize = sizeof(*input) + input->PayloadSize;
                repSize = 0;
            }
            break;

        default:
            ASSERT(FALSE);
            return HV_STATUS_INVALID_HYPERCALL_CODE;
        }

        inputSize = headerSize + (repSize * CountOfElements);
        if (inputSize > GHCB_FIELD64_HYPERCALL_OUTPUT)
        {
            ASSERT(FALSE);
            return HV_STATUS_INVALID_PARAMETER;
        }
        CopyMem(Context->Ghcb, InputPage, inputSize);
    }

    SetGhcbField32(Context->Ghcb, GHCB_FIELD32_FORMAT, 1);
    SetGhcbField64(Context->Ghcb, GHCB_FIELD64_HYPERCALL_OUTPUT, (UINTN)Context->OutputPage);

    hypercallInput.AsUINT64 = 0;
    hypercallInput.CallCode = CallCode;
    hypercallInput.CountOfElements = CountOfElements;

    while (TRUE)
    {
        SetGhcbField64(Context->Ghcb, GHCB_FIELD64_HYPERCALL_CODE, hypercallInput.AsUINT64);

        _sev_vmgexit();

        //
        // If this was not a rep hypercall, or if the call failed, then no
        // further processing is required.
        //

        hypercallOutput.AsUINT64 = GetGhcbField64(Context->Ghcb, GHCB_FIELD64_HYPERCALL_CODE);

        if ((CountOfElements == 0) ||
            (hypercallOutput.CallStatus != HV_STATUS_TIMEOUT))
        {
            break;
        }

        //
        // Continue processing from wherever the hypervisor left off.  The
        // rep start index is not checked for validity, since it is only being
        // used as an input to the untrusted hypervisor.
        //

        hypercallInput.RepStartIndex = hypercallOutput.ElementsProcessed;
    }

    status = (HV_STATUS)hypercallOutput.CallStatus;

    //
    // Ensure that the completed rep count is reasonable.  If not, indicate
    // that the fall failed.
    //

    if ((status == HV_STATUS_SUCCESS) && (hypercallOutput.ElementsProcessed == CountOfElements))
    {
        // NOTHING
    }
    else if ((status != HV_STATUS_SUCCESS) && (hypercallOutput.ElementsProcessed < CountOfElements))
    {
        // NOTHING
    }
    else
    {
        ASSERT(FALSE);
        hypercallOutput.ElementsProcessed = 0;
        status = 0xFFFF;
    }

    if (ElementsProcessed != NULL)
    {
        *ElementsProcessed = hypercallOutput.ElementsProcessed;
    }

    return status;
}
