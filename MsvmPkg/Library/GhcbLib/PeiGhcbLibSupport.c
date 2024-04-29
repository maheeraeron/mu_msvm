#include <PiPei.h>

EFI_TPL
GhcbpDisableInterrupts(
    VOID
    )
{
    // In PEI, interrupts are always disabled. This function is a no-op.
    return 0;
}

VOID
GhcbpEnableInterrupts(
    EFI_TPL tpl
    )
{
    // In PEI, interrupts are always disabled. This function is a no-op.
}
