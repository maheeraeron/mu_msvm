/*++

Copyright (c) Microsoft Corporation

Module Name:

    SecIso.c

Abstract:

    Routines to support hardware isolation of the SEC driver.

--*/

#include <Library/UefiCpuLib.h>
#include <EfiNt.h>
#include <hvgdk_mini.h>
#include <BiosInterface.h>
#include "SecP.h"

#define SetGhcbField16(Ghcb, Field, Value) \
    (*(PUINT16)((PUCHAR)(Ghcb) + (Field)) = (Value))
#define SetGhcbField32(Ghcb, Field, Value) \
    (*(PUINT32)((PUCHAR)(Ghcb) + (Field)) = (Value))
#define SetGhcbField64(Ghcb, Field, Value) \
    (*(PUINT64)((PUCHAR)(Ghcb) + (Field)) = (Value))
#define GetGhcbField64(Ghcb, Field) \
    (*(PUINT64)((PUCHAR)(Ghcb) + (Field)))

#define GHCB_INFO_MAP_GHCB              6

#define GHCB_EXITCODE_MSR               0x7C

#define GHCB_FIELD64_RAX                0x1F8
#define GHCB_FIELD64_RBX                0x318
#define GHCB_FIELD64_RCX                0x308
#define GHCB_FIELD64_RDX                0x310
#define GHCB_FIELD64_EXITCODE           0x390
#define GHCB_FIELD64_EXITINFO1          0x398
#define GHCB_FIELD16_VERSION            0xFFA
#define GHCB_FIELD32_FORMAT             0xFFC

PVOID Ghcb;

BOOLEAN
SecInitializeSnp (
    UEFI_IGVM_PARAMETER_INFO *ParameterInfo
    )
{
    UINT64 ghcbAddress;
    UINT64 ghcbMsr;
    UINT64 sharedGpaBoundary;

    //
    // Select a GHCB address as the first page before the parameter info block.
    //

    if (mIsolationConfiguration.SharedGpaBoundaryActive)
    {
        sharedGpaBoundary = (1UI64 << mIsolationConfiguration.SharedGpaBoundaryBits);
    }
    else
    {
        sharedGpaBoundary = 0;
    }

    ghcbAddress = (UINTN)ParameterInfo - EFI_PAGE_SIZE + sharedGpaBoundary;

    //
    // Check to see whether the hypervisor has configured a GHCB.  If not,
    // configure a GHCB page at the selected address.
    //

    ghcbMsr = AsmReadMsr64(MSR_GHCB);
    if (ghcbMsr == 0)
    {
        AsmWriteMsr64(MSR_GHCB, ghcbAddress | GHCB_INFO_MAP_GHCB);
        SecVmgexit();
        ghcbMsr = AsmReadMsr64(MSR_GHCB);
    }
    
    //
    // The selected GHCB is usable as long as it is either the selected
    // address or it is beyond the parameter block and above the shared GPA
    // boundary.
    //

    if ((ghcbMsr & 0xFFF) != 0)
    {
        return FALSE;
    }

    if (ghcbMsr != ghcbAddress)
    {
        ghcbAddress = (UINTN)ParameterInfo +
                      (ParameterInfo->ParameterPageCount * EFI_PAGE_SIZE) +
                      sharedGpaBoundary;

        if (ghcbMsr < ghcbAddress)
        {
            return FALSE;
        }

        ghcbAddress = ghcbMsr;
    }

    Ghcb = (PVOID)ghcbAddress;

    return TRUE;
}

BOOLEAN
SecProcessVirtualCommunicationException (
    _In_ PTRAP_FRAME TrapFrame
    )
{
    UNREFERENCED_PARAMETER(TrapFrame);

    return FALSE;
}
