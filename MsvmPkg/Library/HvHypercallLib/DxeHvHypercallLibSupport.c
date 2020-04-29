#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>

#include <HvHypercallLibP.h>

EFI_TPL mTpl;

VOID
HvHypercallpDisableInterrupts(
    VOID
    )
{
    // In DXE, raise TPL to high level that will be restored when enable interrupts
    // is called.
    mTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);
}

VOID
HvHypercallpEnableInterrupts(
    VOID
    )
{
    gBS->RestoreTPL(mTpl);
}