#include "UefiHavoc.h"

int mData[4];

EFI_STATUS
EFIAPI
CpuidInit(
    )
{
    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
CpuidHavoc(
    _In_  HAVOC_PLUGIN   *Plugin
    )
{
    __cpuid(mData, 0);
    return EFI_SUCCESS;
}