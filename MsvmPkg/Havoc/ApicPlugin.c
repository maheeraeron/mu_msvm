#include "UefiHavoc.h"
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/LocalApicLib.h>
#include <Register/LocalApic.h>

#include <Protocol/Cpu.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/ShellLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Register/Cpuid.h>

#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(x[0]))

typedef struct
{
    LOCAL_APIC_ICR_LOW  Icr;
    UINT32              ApicId;
    UINT64              Count;
}APIC_TEST_PARAMS;


EFI_CPU_ARCH_PROTOCOL *mCpu;
LOCAL_APIC_ICR_LOW  mIcr;
UINT32  mApicId = 0;
UINTN   mApicBase = 0;
UINT64  mInterruptCount = 0;


BOOLEAN
HasX2APIC()
{
    UINT32 regEcx;
    AsmCpuid(CPUID_VERSION_INFO, NULL, NULL, &regEcx, NULL);

    // 0x200000 -> Bit 21 is x2APIC support
    return ((regEcx & 0x200000) == 0x200000);
}


UINT32
EFIAPI
ApicRegRead (
  IN UINTN  MmioOffset
  )
{
  ASSERT ((MmioOffset & 0xf) == 0);
  return MmioRead32 (mApicBase + MmioOffset);
}


VOID
EFIAPI
ApicRegWrite (
  IN UINTN  MmioOffset,
  IN UINT32 Value
  )
{
  ASSERT ((MmioOffset & 0xf) == 0);
  MmioWrite32 (mApicBase + MmioOffset, Value);
}


VOID
ApicIpi (
  IN UINT32          IcrLow,
  IN UINT32          ApicId,
     BOOLEAN         WaitForDelivery
  )
{
  LOCAL_APIC_ICR_LOW IcrLowReg;

  ASSERT (ApicId <= 0xff);

  //
  // For xAPIC, the act of writing to the low doubleword of the ICR causes the IPI to be sent.
  //
  ApicRegWrite (XAPIC_ICR_HIGH_OFFSET, ApicId << 24);
  ApicRegWrite (XAPIC_ICR_LOW_OFFSET, IcrLow);

  if (WaitForDelivery)
  {
      do {
        IcrLowReg.Uint32 = ApicRegRead (XAPIC_ICR_LOW_OFFSET);
      } while (IcrLowReg.Bits.DeliveryStatus != 0);
  }
}

#define INTERRUPT_RANGE_START       0x80
#define INTERRUPT_RANGE_END         0xFF

EFI_STATUS
InterruptSetup(
    UINT8                        *Vector,
    EFI_CPU_INTERRUPT_HANDLER     Handler
    )
{
    EFI_STATUS status = EFI_OUT_OF_RESOURCES;
    EFI_TPL tpl;
    UINT8 vector;
    //
    // Disable interrupts while manipulating IDT state.
    //
    tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

    //
    // Find an empty interrupt vector.
    // RegisterInterruptHandler will return EFI_ALREADY_STARTED for
    // vectors that are already hooked.
    //
    for (vector = INTERRUPT_RANGE_START; vector < INTERRUPT_RANGE_END; vector++)
    {
        status = mCpu->RegisterInterruptHandler(mCpu, vector, Handler);

        if (!EFI_ERROR(status))
        {
            *Vector = vector;
            break;
        }
    }

    gBS->RestoreTPL(tpl);

    return status;
}


EFI_STATUS
InterruptCleanup(
    UINT8                         Vector
    )
{
    EFI_STATUS status = EFI_OUT_OF_RESOURCES;
    EFI_TPL tpl;

    //
    // Disable interrupts while manipulating IDT state.
    //
    tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

    status = mCpu->RegisterInterruptHandler(mCpu, Vector, NULL);

    gBS->RestoreTPL(tpl);

    return status;
}


VOID
ApicInterrupt(
  IN CONST  EFI_EXCEPTION_TYPE  InterruptType,
  IN CONST  EFI_SYSTEM_CONTEXT  SystemContext
  )
{
    //Print(L"IPI:%02x\n", InterruptType);
    mInterruptCount++;
    SendApicEoi();
}


EFI_STATUS
EFIAPI
ApicInit(
    )
{
    EFI_STATUS status;
    UINT8 vector = 0;

    status = gBS->LocateProtocol(&gEfiCpuArchProtocolGuid, NULL, (VOID**)&mCpu);
    if (EFI_ERROR(status))
    {
        Print(L"Failed to locate CPU Arch protocol. Status %r\n", status);
        goto Cleanup;
    }

    mApicBase = GetLocalApicBaseAddress();

    //
    // Fill in the defaults
    //
    mIcr.Uint32 = 0;
    mIcr.Bits.DeliveryMode = LOCAL_APIC_DELIVERY_MODE_FIXED;
    mIcr.Bits.Level  = 1;
    mIcr.Bits.Vector = 0;    // auto

    mApicId = GetApicId();

    //
    // Intel docs state that the vector must be zero for SMI
    //
    if (mIcr.Bits.DeliveryMode == LOCAL_APIC_DELIVERY_MODE_SMI)
    {
        mIcr.Bits.Vector = 0;
    }
    else
    {
        if (mIcr.Bits.Vector == 0)
        {
            Print(L" Auto Choosing Vector...\n");
            status = InterruptSetup(&vector, ApicInterrupt);

            if (EFI_ERROR(status))
            {
                Print(L"Failed to setup interrupt handler. Status %r\n", status);
                goto Cleanup;
            }

            mIcr.Bits.Vector = vector;
        }
        else
        {
            Print(L"  User Vector provided.  Interrupt Handler will not be registered\n");
        }
    }

Cleanup:

    return status;
}


EFI_STATUS
EFIAPI
ApicHavoc(
    _In_  HAVOC_PLUGIN   *Plugin
    )
{
    ApicIpi(mIcr.Uint32, mApicId, FALSE);

    return EFI_SUCCESS;
}