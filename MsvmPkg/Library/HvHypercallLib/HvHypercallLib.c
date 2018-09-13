/*++

Copyright (c) Microsoft Corporation

Module Name:

    HvHypercallLib.c

Abstract:

    This file implements hypercall support routines.

--*/

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HvHypercallLib.h>


#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)

VOID
HvHypercallConnect(
    _In_ PVOID HypercallPage,
    _Out_ HV_HYPERCALL_CONTEXT *Context
    )
/*++

Routine Description:

    Sets up hypercall context by connecting to the hypervisor.

Arguments:

    HypercallPage - An address at which to place the hypercall page.

    Context - Receives the hypercall context.

Return Value:

    None.

--*/
{
    HV_GUEST_OS_ID_CONTENTS guestOsId;
    HV_X64_MSR_HYPERCALL_CONTENTS hypercallMsr;

    ZeroMem(Context, sizeof(*Context));

    //
    // Set the guest ID.
    //

    guestOsId.AsUINT64 = 0;
    guestOsId.OsId = 1;
    HvHypercallSetVpRegister64Self(HvRegisterGuestOsId, guestOsId.AsUINT64);

    //
    // Enable the hypercall page.
    //

    hypercallMsr.AsUINT64 = HvHypercallGetVpRegister64Self(HvX64RegisterHypercall);
    ASSERT(hypercallMsr.Enable == 0);
    hypercallMsr.Enable = 1;
    hypercallMsr.GpaPageNumber = (UINTN)HypercallPage / HV_PAGE_SIZE;
    HvHypercallSetVpRegister64Self(HvX64RegisterHypercall, hypercallMsr.AsUINT64);

    Context->HypercallPage = HypercallPage;

    Context->Connected = TRUE;
}

#elif defined(MDE_CPU_AARCH64)

VOID
HvHypercallConnect(
    _Out_ HV_HYPERCALL_CONTEXT *Context
    )
/*++

Routine Description:

    Sets up hypercall context by connecting to the hypervisor.

Arguments:

    Context - Receives the hypercall context.

Return Value:

    None.

--*/
{
    HV_GUEST_OS_ID_CONTENTS guestOsId;

    ZeroMem(Context, sizeof(*Context));

    //
    // Set the guest ID.
    //

    guestOsId.AsUINT64 = 0;
    guestOsId.OsId = 4;     // Windows NT
    guestOsId.VendorId = 1; // Microsoft
    HvHypercallSetVpRegister64Self(HvRegisterGuestOsId, guestOsId.AsUINT64);

    guestOsId.AsUINT64 = 0;
    guestOsId.AsUINT64 = HvHypercallGetVpRegister64Self(HvRegisterGuestOsId);
    ASSERT(guestOsId.VendorId == 1 && guestOsId.OsId == 4);

    Context->Connected = TRUE;
}

#endif


VOID
HvHypercallDisconnect(
    _Inout_ HV_HYPERCALL_CONTEXT *Context
    )
/*++

Routine Description:

    Tears down hypercall context by disconnecting from the hypervisor.

Arguments:

    Context - The hypercall context.

Return Value:

    None.

--*/
{
    if (Context->Connected)
    {
#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)

        HV_X64_MSR_HYPERCALL_CONTENTS hypercallMsr;

        hypercallMsr.AsUINT64 = HvHypercallGetVpRegister64Self(HvX64RegisterHypercall);
        hypercallMsr.Enable = 0;
        hypercallMsr.GpaPageNumber = 0;
        HvHypercallSetVpRegister64Self(HvX64RegisterHypercall, hypercallMsr.AsUINT64);

#endif

        HvHypercallSetVpRegister64Self(HvRegisterGuestOsId, 0);

        Context->Connected = FALSE;
    }
}


HV_STATUS
HvHypercallIssue(
    _In_ HV_HYPERCALL_CONTEXT *Context,
    _In_ HV_CALL_CODE CallCode,
    _In_ BOOLEAN Fast,
    _In_ UINT32 CountOfElements,
    _In_ UINT64 FirstRegister,
    _In_ UINT64 SecondRegister,
    _Out_opt_ UINT32 *ElementsProcessed
    )
/*++

Routine Description:

    Issues a hypercall.

Arguments:

    Context - The hypercall context (set up via HvHypercallConnect).

    CallCode - The hypercall code.

    Fast - If TRUE, this is a fast hypercall.

    CountOfElements - The number of elements to process, or 0 if not a rep
        hypercall.

    FirstRegister - The first register value for the hypercall.

    SecondRegister - The second register value for the hypercall.

    ElementsProcessed - Receives the number of elements that were successfully
        processed.

Return Value:

    The hypercall status.

--*/
{
    HV_HYPERCALL_INPUT callInput;
    HV_HYPERCALL_OUTPUT callOutput;

    ASSERT(Context->Connected);
    ASSERT(CountOfElements <= HV_X64_MAX_HYPERCALL_ELEMENTS);

    // DEBUG((DEBUG_VERBOSE, ">>> %a\n", __FUNCTION__));

    callInput.AsUINT64 = 0;
    callInput.CallCode = CallCode;
    callInput.IsFast = (Fast != FALSE);
    callInput.CountOfElements = CountOfElements;

#if defined(MDE_CPU_X64)

    {
        typedef HV_X64_HYPERCALL_OUTPUT HYPERCALL_ROUTINE(
            __in HV_X64_HYPERCALL_INPUT Control,
            __in UINT64                 InputPhysicalAddress,
            __in UINT64                 OutputPhysicalAddress
            );

#pragma warning(disable: 4055)

        HYPERCALL_ROUTINE* hypercallRoutine =
            (HYPERCALL_ROUTINE *)Context->HypercallPage;

        callOutput = hypercallRoutine(callInput,
                                      FirstRegister,
                                      SecondRegister);
    }

#elif defined(MDE_CPU_IA32)

    {
        ULARGE_INTEGER packed;
        ULARGE_INTEGER first;
        ULARGE_INTEGER second;

        packed.QuadPart = callInput.AsUINT64;
        first.QuadPart = FirstRegister;
        second.QuadPart = SecondRegister;

        __asm
        {
            mov edi, second.HighPart;
            mov esi, second.LowPart;
            mov ebx, first.HighPart;
            mov ecx, first.LowPart;
            mov edx, packed.HighPart;
            mov eax, packed.LowPart;

            call Context->HypercallPage;

            mov packed.LowPart, eax;
            mov packed.HighPart, edx;
        }

        callOutput.AsUINT64 = packed.QuadPart;
    }

#elif defined(MDE_CPU_AARCH64)

    callOutput = AsmHyperCall(callInput, FirstRegister, SecondRegister);

#else
#error Unsupported Architecture
#endif
    // DEBUG((DEBUG_VERBOSE, "<<< %a\n", __FUNCTION__));

    if (ElementsProcessed != NULL)
    {
        *ElementsProcessed = callOutput.ElementsProcessed;
    }

    return callOutput.CallStatus;
}


#if !defined(MDEPKG_NDEBUG)
PWSTR
HvHypercallpRegisterNameToString(
    _In_ HV_REGISTER_NAME RegisterName
    )
{
    switch (RegisterName)
    {
    case HvRegisterSint0:
        return L"HvRegisterSint0";
    case HvRegisterSint1:
        return L"HvRegisterSint1";
    case HvRegisterSint2:
        return L"HvRegisterSint2";
    case HvRegisterSint3:
        return L"HvRegisterSint3";
    case HvRegisterSint4:
        return L"HvRegisterSint4";
    case HvRegisterSint5:
        return L"HvRegisterSint5";
    case HvRegisterSint6:
        return L"HvRegisterSint6";
    case HvRegisterSint7:
        return L"HvRegisterSint7";
    case HvRegisterSint8:
        return L"HvRegisterSint8";
    case HvRegisterSint9:
        return L"HvRegisterSint9";
    case HvRegisterSint10:
        return L"HvRegisterSint10";
    case HvRegisterSint11:
        return L"HvRegisterSint11";
    case HvRegisterSint12:
        return L"HvRegisterSint12";
    case HvRegisterSint13:
        return L"HvRegisterSint13";
    case HvRegisterSint14:
        return L"HvRegisterSint14";
    case HvRegisterSint15:
        return L"HvRegisterSint15";

    case HvRegisterScontrol:
        return L"HvRegisterScontrol";
    case HvRegisterSversion:
        return L"HvRegisterSversion";
    case HvRegisterSifp:
        return L"HvRegisterSifp";
    case HvRegisterSipp:
        return L"HvRegisterSipp";
    case HvRegisterEom:
        return L"HvRegisterEom";
    case HvRegisterSirbp:
        return L"HvRegisterSirbp";

    case HvRegisterNestedSint0:
        return L"HvRegisterNestedSint0";
    case HvRegisterNestedSint1:
        return L"HvRegisterNestedSint1";
    case HvRegisterNestedSint2:
        return L"HvRegisterNestedSint2";
    case HvRegisterNestedSint3:
        return L"HvRegisterNestedSint3";
    case HvRegisterNestedSint4:
        return L"HvRegisterNestedSint4";
    case HvRegisterNestedSint5:
        return L"HvRegisterNestedSint5";
    case HvRegisterNestedSint6:
        return L"HvRegisterNestedSint6";
    case HvRegisterNestedSint7:
        return L"HvRegisterNestedSint7";
    case HvRegisterNestedSint8:
        return L"HvRegisterNestedSint8";
    case HvRegisterNestedSint9:
        return L"HvRegisterNestedSint9";
    case HvRegisterNestedSint10:
        return L"HvRegisterNestedSint10";
    case HvRegisterNestedSint11:
        return L"HvRegisterNestedSint11";
    case HvRegisterNestedSint12:
        return L"HvRegisterNestedSint12";
    case HvRegisterNestedSint13:
        return L"HvRegisterNestedSint13";
    case HvRegisterNestedSint14:
        return L"HvRegisterNestedSint14";
    case HvRegisterNestedSint15:
        return L"HvRegisterNestedSint15";

    case HvRegisterNestedScontrol:
        return L"HvRegisterNestedScontrol";
    case HvRegisterNestedSversion:
        return L"HvRegisterNestedSversion";
    case HvRegisterNestedSifp:
        return L"HvRegisterNestedSifp";
    case HvRegisterNestedSipp:
        return L"HvRegisterNestedSipp";
    case HvRegisterNestedEom:
        return L"HvRegisterNestedEom";
    case HvRegisterNestedSirbp:
        return L"HvRegisterNestedSirbp";

    case HvRegisterVpIndex:
        return L"HvRegisterVpIndex";
    case HvRegisterGuestOsId:
        return L"HvRegisterGuestOsId";
    case HvRegisterTimeRefCount:
        return L"HvRegisterTimeRefCount";
    case HvRegisterNestedVpIndex:
        return L"HvRegisterNestedVpIndex";

    case HvRegisterStimer0Config:
        return L"HvRegisterStimer0Config";
    case HvRegisterStimer0Count:
        return L"HvRegisterStimer0Count";
    case HvRegisterStimer1Config:
        return L"HvRegisterStimer1Config";
    case HvRegisterStimer1Count:
        return L"HvRegisterStimer1Count";
    case HvRegisterStimer2Config:
        return L"HvRegisterStimer2Config";
    case HvRegisterStimer2Count:
        return L"HvRegisterStimer2Count";
    case HvRegisterStimer3Config:
        return L"HvRegisterStimer3Config";
    case HvRegisterStimer3Count:
        return L"HvRegisterStimer3Count";

#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)
    case HvX64RegisterHypercall:
        return L"HvX64RegisterHypercall";
#endif

    default:
        return L"*** Unknown Register Name ***";
    }
}
#endif


#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)
static
UINT32
HvHypercallpGetMsrNameFromRegisterName(
    _In_ HV_REGISTER_NAME RegisterName
    )
/*++

Routine Description:

    Maps a register name to an msr index.

Arguments:

    RegisterName - Supplies the register name to be mapped.

Return Value:

    Returns the msr index.

--*/
{
    UINT32 msrIndex;

    switch (RegisterName)
    {
    case HvRegisterSint0:
    case HvRegisterSint1:
    case HvRegisterSint2:
    case HvRegisterSint3:
    case HvRegisterSint4:
    case HvRegisterSint5:
    case HvRegisterSint6:
    case HvRegisterSint7:
    case HvRegisterSint8:
    case HvRegisterSint9:
    case HvRegisterSint10:
    case HvRegisterSint11:
    case HvRegisterSint12:
    case HvRegisterSint13:
    case HvRegisterSint14:
    case HvRegisterSint15:
        msrIndex = HV_X64_MSR_SINT0 + (RegisterName - HvRegisterSint0);
        break;

    case HvRegisterScontrol:
    case HvRegisterSversion:
    case HvRegisterSifp:
    case HvRegisterSipp:
    case HvRegisterEom:
    case HvRegisterSirbp:
        msrIndex = HV_X64_MSR_SCONTROL + (RegisterName - HvRegisterScontrol);
        break;

    case HvRegisterNestedSint0:
    case HvRegisterNestedSint1:
    case HvRegisterNestedSint2:
    case HvRegisterNestedSint3:
    case HvRegisterNestedSint4:
    case HvRegisterNestedSint5:
    case HvRegisterNestedSint6:
    case HvRegisterNestedSint7:
    case HvRegisterNestedSint8:
    case HvRegisterNestedSint9:
    case HvRegisterNestedSint10:
    case HvRegisterNestedSint11:
    case HvRegisterNestedSint12:
    case HvRegisterNestedSint13:
    case HvRegisterNestedSint14:
    case HvRegisterNestedSint15:
        msrIndex = HV_X64_MSR_NESTED_SINT0 +
            (RegisterName - HvRegisterNestedSint0);

        break;

    case HvRegisterNestedScontrol:
    case HvRegisterNestedSversion:
    case HvRegisterNestedSifp:
    case HvRegisterNestedSipp:
    case HvRegisterNestedEom:
    case HvRegisterNestedSirbp:
        msrIndex = HV_X64_MSR_NESTED_SCONTROL +
            (RegisterName - HvRegisterNestedScontrol);

        break;

    case HvRegisterVpIndex:
        msrIndex = HV_X64_MSR_VP_INDEX;
        break;

    case HvRegisterGuestOsId:
        msrIndex = HV_X64_MSR_GUEST_OS_ID;
        break;

    case HvRegisterTimeRefCount:
        msrIndex = HV_X64_MSR_TIME_REF_COUNT;
        break;

    case HvRegisterNestedVpIndex:
        msrIndex = HV_X64_MSR_NESTED_VP_INDEX;
        break;

    case HvRegisterStimer0Config:
    case HvRegisterStimer0Count:
    case HvRegisterStimer1Config:
    case HvRegisterStimer1Count:
    case HvRegisterStimer2Config:
    case HvRegisterStimer2Count:
    case HvRegisterStimer3Config:
    case HvRegisterStimer3Count:
        msrIndex = HV_X64_MSR_STIMER0_CONFIG +
            (RegisterName - HvRegisterStimer0Config);
        break;

    case HvX64RegisterHypercall:
        msrIndex = HV_X64_MSR_HYPERCALL;
        break;

    default:
        ASSERT(FALSE);
        __assume(0);
    }

    return msrIndex;
}
#endif


UINT64
HvHypercallGetVpRegister64Self(
    _In_ HV_REGISTER_NAME RegisterName
    )
/*++

Routine Description:

    Gets a 64 bit register value on the current virtual processor.

Arguments:

    RegisterName - Supplies the register name to be mapped.

Return Value:

    The register value.

--*/
{
    UINT64 registerValue;

#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)

    UINT32 msr = HvHypercallpGetMsrNameFromRegisterName(RegisterName);

    // DEBUG((DEBUG_VERBOSE, ">>> %a: Name 0x%x %s MSR 0x%x\n", __FUNCTION__,
    //     RegisterName, HvHypercallpRegisterNameToString(RegisterName), msr));

    registerValue = AsmReadMsr64(msr);

#elif defined(MDE_CPU_AARCH64)

    HV_STATUS status;

    status = AsmGetVpRegister64(RegisterName, &registerValue);
    ASSERT(status == 0);

#else
#error Unsupported Architecture
#endif

    // DEBUG((DEBUG_VERBOSE, "<<< %a: Value 0x%lx\n", __FUNCTION__, registerValue));
    return registerValue;
}


VOID
HvHypercallSetVpRegister64Self(
    _In_ HV_REGISTER_NAME RegisterName,
    _In_ UINT64 RegisterValue
    )
/*++

Routine Description:

    Sets a 64 bit register on the current virtual processor.

Arguments:

    RegisterName - Supplies the register name to be mapped.

    RegisterValue - Supplies the register value.

Return Value:

    None.

--*/
{

#if defined(MDE_CPU_IA32) || defined(MDE_CPU_X64)

    UINT32 msr = HvHypercallpGetMsrNameFromRegisterName(RegisterName);

    DEBUG((DEBUG_VERBOSE, ">>> %a: Name 0x%x %s MSR 0x%x Value 0x%lx\n", __FUNCTION__,
        RegisterName, HvHypercallpRegisterNameToString(RegisterName), msr, RegisterValue));

    AsmWriteMsr64(msr, RegisterValue);

#elif defined(MDE_CPU_AARCH64)

    DEBUG((DEBUG_VERBOSE, ">>> %a: Name 0x%x %s Value 0x%lx\n", __FUNCTION__,
        RegisterName, HvHypercallpRegisterNameToString(RegisterName),
        RegisterValue));

    AsmSetVpRegister64(RegisterName, RegisterValue);

#else
#error Unsupported Architecture
#endif

    DEBUG((DEBUG_VERBOSE, "<<< %a\n", __FUNCTION__));
}
