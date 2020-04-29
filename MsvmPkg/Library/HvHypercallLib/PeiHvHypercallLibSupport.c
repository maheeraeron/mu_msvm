#include <PiPei.h>

#include <HvHypercallLibP.h>

VOID
HvHypercallpDisableInterrupts(
    VOID
    )
{
    // In PEI, interrupts are always disabled. This function is a no-op.
}

VOID
HvHypercallpEnableInterrupts(
    VOID
    )
{
    // In PEI, interrupts are always disabled. This function is a no-op.
}