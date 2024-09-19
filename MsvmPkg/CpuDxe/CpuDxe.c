/** @file
  CPU DXE Module to produce CPU ARCH Protocol.

  Copyright (c) 2008 - 2022, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "CpuDxe.h"
#include "CpuMp.h"
#include "CpuPageTable.h"
#include <Library/DeviceStateLib.h> // MU_CHANGE
#define CPU_INTERRUPT_NUM  256

//
// Global Variables
//
BOOLEAN     InterruptState = FALSE;
EFI_HANDLE  mCpuHandle     = NULL;
BOOLEAN     mIsFlushingGCD;

// MS_HYP_CHANGE BEGIN
#include <IsolationTypes.h>
#include <Hv/HvGuestMsr.h>
#include <Library/CrashLib.h>
#include <Library/PrintLib.h>

#if defined(MDE_CPU_X64)

EFI_HV_PROTOCOL *mHv;
EFI_EVENT   mEndOfDxeEvent;

#endif

IA32_IDT_GATE_DESCRIPTOR  gIdtTable[CPU_INTERRUPT_NUM] = { 0 }; // MS_HYP_CHANGE
IA32_IDT_GATE_DESCRIPTOR  mOrigIdtEntry[CPU_INTERRUPT_NUM] = { 0 }; // MS_HYP_CHANGE

EFI_CPU_INTERRUPT_HANDLER ExternalVectorTable[0x100]; // MS_HYP_CHANGE
UINT16                    mOrigIdtEntryCount    = 0;  // MS_HYP_CHANGE

BOOLEAN                   mStrictIsolation;
UINT32                    mIsolationType;
// MS_HYP_CHANGE END

BOOLEAN     mIsAllocatingPageTable = FALSE;
UINT64      mValidMtrrAddressMask;
UINT64      mValidMtrrBitsMask;
UINT64      mTimerPeriod = 0;

FIXED_MTRR  mFixedMtrrTable[] = {
  {
    MSR_IA32_MTRR_FIX64K_00000,
    0,
    0x10000
  },
  {
    MSR_IA32_MTRR_FIX16K_80000,
    0x80000,
    0x4000
  },
  {
    MSR_IA32_MTRR_FIX16K_A0000,
    0xA0000,
    0x4000
  },
  {
    MSR_IA32_MTRR_FIX4K_C0000,
    0xC0000,
    0x1000
  },
  {
    MSR_IA32_MTRR_FIX4K_C8000,
    0xC8000,
    0x1000
  },
  {
    MSR_IA32_MTRR_FIX4K_D0000,
    0xD0000,
    0x1000
  },
  {
    MSR_IA32_MTRR_FIX4K_D8000,
    0xD8000,
    0x1000
  },
  {
    MSR_IA32_MTRR_FIX4K_E0000,
    0xE0000,
    0x1000
  },
  {
    MSR_IA32_MTRR_FIX4K_E8000,
    0xE8000,
    0x1000
  },
  {
    MSR_IA32_MTRR_FIX4K_F0000,
    0xF0000,
    0x1000
  },
  {
    MSR_IA32_MTRR_FIX4K_F8000,
    0xF8000,
    0x1000
  },
};

EFI_CPU_ARCH_PROTOCOL  gCpu = {
  CpuFlushCpuDataCache,
  CpuEnableInterrupt,
  CpuDisableInterrupt,
  CpuGetInterruptState,
  CpuInit,
  CpuRegisterInterruptHandler,
  CpuGetTimerValue,
  CpuSetMemoryAttributes,
  1,                          // NumberOfTimers
  4                           // DmaBufferAlignment
};

// MS_HYP_CHANGE BEGIN
EFI_CPU2_PROTOCOL gCpu2 = {
  CpuWaitForAndEnableInterrupt,
};

//
// Error code flag indicating whether or not an error code will be
// pushed on the stack if an exception occurs.
//
// 1 means an error code will be pushed, otherwise 0
//
// bit 0 - exception 0
// bit 1 - exception 1
// etc.
//
UINT32 mErrorCodeFlag = 0x00027d00;

//
// Local function prototypes
//

/**
  Restore original Interrupt Descriptor Table Handler Address.

  @param Index        The Index of the interrupt descriptor table handle.

**/
VOID
RestoreInterruptDescriptorTableHandlerAddress (
  IN UINTN       Index
  );

/**
  Set Interrupt Descriptor Table Handler Address.

  @param Index        The Index of the interrupt descriptor table handle.
  @param Handler      Handler address.

**/
VOID
SetInterruptDescriptorTableHandlerAddress (
  IN UINTN Index,
  IN VOID  *Handler  OPTIONAL
  );

//
// CPU Arch Protocol Functions
//


/**
  Common exception handler.

  @param  InterruptType  Exception type
  @param  SystemContext  EFI_SYSTEM_CONTEXT

**/
VOID
EFIAPI
CommonExceptionHandlerMsvm (
  IN EFI_EXCEPTION_TYPE   InterruptType,
  IN EFI_SYSTEM_CONTEXT   SystemContext
  )
{
#if defined (MDE_CPU_X64)
  CHAR8 buffer[HV_CRASH_MAXIMUM_MESSAGE_SIZE];
  UINTN cursor = 0;

  cursor += AsciiSPrint(
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "!!!! X64 Exception Type - %016lx !!!!\n",
    (UINT64)InterruptType
    );

  if ((mErrorCodeFlag & (1 << InterruptType)) != 0) {
    cursor += AsciiSPrint (
      &buffer[cursor],
      (sizeof buffer) - (sizeof(buffer[0]) * cursor),
      "ExceptionData - %016lx\n",
      SystemContext.SystemContextX64->ExceptionData
      );
  }

  cursor += AsciiSPrint (
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "RIP - %016lx, RFL - %016lx\n",
    SystemContext.SystemContextX64->Rip,
    SystemContext.SystemContextX64->Rflags
    );
  cursor += AsciiSPrint (
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "ExceptionData - %016lx\n",
    SystemContext.SystemContextX64->ExceptionData
    );
  cursor += AsciiSPrint (
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "RAX - %016lx, RCX - %016lx, RDX - %016lx\n",
    SystemContext.SystemContextX64->Rax,
    SystemContext.SystemContextX64->Rcx,
    SystemContext.SystemContextX64->Rdx
    );
  cursor += AsciiSPrint (
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "RBX - %016lx, RSP - %016lx, RBP - %016lx\n",
    SystemContext.SystemContextX64->Rbx,
    SystemContext.SystemContextX64->Rsp,
    SystemContext.SystemContextX64->Rbp
    );
  cursor += AsciiSPrint (
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "RSI - %016lx, RDI - %016lx\n",
    SystemContext.SystemContextX64->Rsi,
    SystemContext.SystemContextX64->Rdi
    );
  cursor += AsciiSPrint (
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "R8  - %016lx, R9  - %016lx, R10 - %016lx\n",
    SystemContext.SystemContextX64->R8,
    SystemContext.SystemContextX64->R9,
    SystemContext.SystemContextX64->R10
    );
  cursor += AsciiSPrint (
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "R11 - %016lx, R12 - %016lx, R13 - %016lx\n",
    SystemContext.SystemContextX64->R11,
    SystemContext.SystemContextX64->R12,
    SystemContext.SystemContextX64->R13
    );
  cursor += AsciiSPrint (
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "R14 - %016lx, R15 - %016lx\n",
    SystemContext.SystemContextX64->R14,
    SystemContext.SystemContextX64->R15
    );
  cursor += AsciiSPrint (
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "CS  - %04lx, DS  - %04lx, ES  - %04lx, FS  - %04lx, GS  - %04lx, SS  - %04lx\n",
    SystemContext.SystemContextX64->Cs,
    SystemContext.SystemContextX64->Ds,
    SystemContext.SystemContextX64->Es,
    SystemContext.SystemContextX64->Fs,
    SystemContext.SystemContextX64->Gs,
    SystemContext.SystemContextX64->Ss
    );
  cursor += AsciiSPrint(
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "GDT - %016lx; %04lx,                   IDT - %016lx; %04lx\n",
    SystemContext.SystemContextX64->Gdtr[0],
    SystemContext.SystemContextX64->Gdtr[1],
    SystemContext.SystemContextX64->Idtr[0],
    SystemContext.SystemContextX64->Idtr[1]
    );
  cursor += AsciiSPrint(
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "LDT - %016lx, TR  - %016lx\n",
    SystemContext.SystemContextX64->Ldtr,
    SystemContext.SystemContextX64->Tr
    );
  cursor += AsciiSPrint(
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "CR0 - %016lx, CR2 - %016lx, CR3 - %016lx\n",
    SystemContext.SystemContextX64->Cr0,
    SystemContext.SystemContextX64->Cr2,
    SystemContext.SystemContextX64->Cr3
    );
  cursor += AsciiSPrint(
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "CR4 - %016lx, CR8 - %016lx\n",
    SystemContext.SystemContextX64->Cr4,
    SystemContext.SystemContextX64->Cr8
    );
  cursor += AsciiSPrint(
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "DR0 - %016lx, DR1 - %016lx, DR2 - %016lx\n",
    SystemContext.SystemContextX64->Dr0,
    SystemContext.SystemContextX64->Dr1,
    SystemContext.SystemContextX64->Dr2
    );
  cursor += AsciiSPrint(
    &buffer[cursor],
    (sizeof buffer) - (sizeof(buffer[0]) * cursor),
    "DR3 - %016lx, DR6 - %016lx, DR7 - %016lx\n",
    SystemContext.SystemContextX64->Dr3,
    SystemContext.SystemContextX64->Dr6,
    SystemContext.SystemContextX64->Dr7
    );

  for (UINT32 index = 0; index < cursor; index += 0x100) // MAX_DEBUG_MESSAGE_LENGTH
  {
    DEBUG ((EFI_D_ERROR, "%a", &buffer[index]));
  }

#else
#error CPU type not supported for exception information dump!
#endif

  FailFast(
    InterruptType,
    SystemContext.SystemContextX64->ExceptionData,
    0,
    (UINTN)&buffer,
    cursor);
}

// MS_HYP_CHANGE END

/**
  Flush CPU data cache. If the instruction cache is fully coherent
  with all DMA operations then function can just return EFI_SUCCESS.

  @param  This              Protocol instance structure
  @param  Start             Physical address to start flushing from.
  @param  Length            Number of bytes to flush. Round up to chipset
                            granularity.
  @param  FlushType         Specifies the type of flush operation to perform.

  @retval EFI_SUCCESS       If cache was flushed
  @retval EFI_UNSUPPORTED   If flush type is not supported.
  @retval EFI_DEVICE_ERROR  If requested range could not be flushed.

**/
EFI_STATUS
EFIAPI
CpuFlushCpuDataCache (
  IN EFI_CPU_ARCH_PROTOCOL  *This,
  IN EFI_PHYSICAL_ADDRESS   Start,
  IN UINT64                 Length,
  IN EFI_CPU_FLUSH_TYPE     FlushType
  )
{
  if (FlushType == EfiCpuFlushTypeWriteBackInvalidate) {
    AsmWbinvd ();
    return EFI_SUCCESS;
  } else if (FlushType == EfiCpuFlushTypeInvalidate) {
    AsmInvd ();
    return EFI_SUCCESS;
  } else {
    return EFI_UNSUPPORTED;
  }
}

/**
  Enables CPU interrupts.

  @param  This              Protocol instance structure

  @retval EFI_SUCCESS       If interrupts were enabled in the CPU
  @retval EFI_DEVICE_ERROR  If interrupts could not be enabled on the CPU.

**/
EFI_STATUS
EFIAPI
CpuEnableInterrupt (
  IN EFI_CPU_ARCH_PROTOCOL  *This
  )
{
  EnableInterrupts ();

  InterruptState = TRUE;
  return EFI_SUCCESS;
}

/**
  Disables CPU interrupts.

  @param  This              Protocol instance structure

  @retval EFI_SUCCESS       If interrupts were disabled in the CPU.
  @retval EFI_DEVICE_ERROR  If interrupts could not be disabled on the CPU.

**/
EFI_STATUS
EFIAPI
CpuDisableInterrupt (
  IN EFI_CPU_ARCH_PROTOCOL  *This
  )
{
  DisableInterrupts ();

  InterruptState = FALSE;
  return EFI_SUCCESS;
}

/**
  Return the state of interrupts.

  @param  This                   Protocol instance structure
  @param  State                  Pointer to the CPU's current interrupt state

  @retval EFI_SUCCESS            If interrupts were disabled in the CPU.
  @retval EFI_INVALID_PARAMETER  State is NULL.

**/
EFI_STATUS
EFIAPI
CpuGetInterruptState (
  IN  EFI_CPU_ARCH_PROTOCOL  *This,
  OUT BOOLEAN                *State
  )
{
  if (State == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *State = GetInterruptState ();  // MS_HYP_CHANGE
  return EFI_SUCCESS;
}

/**
  Generates an INIT to the CPU.

  @param  This              Protocol instance structure
  @param  InitType          Type of CPU INIT to perform

  @retval EFI_SUCCESS       If CPU INIT occurred. This value should never be
                            seen.
  @retval EFI_DEVICE_ERROR  If CPU INIT failed.
  @retval EFI_UNSUPPORTED   Requested type of CPU INIT not supported.

**/
EFI_STATUS
EFIAPI
CpuInit (
  IN EFI_CPU_ARCH_PROTOCOL  *This,
  IN EFI_CPU_INIT_TYPE      InitType
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Registers a function to be called from the CPU interrupt handler.

  @param  This                   Protocol instance structure
  @param  InterruptType          Defines which interrupt to hook. IA-32
                                 valid range is 0x00 through 0xFF
  @param  InterruptHandler       A pointer to a function of type
                                 EFI_CPU_INTERRUPT_HANDLER that is called
                                 when a processor interrupt occurs.  A null
                                 pointer is an error condition.

  @retval EFI_SUCCESS            If handler installed or uninstalled.
  @retval EFI_ALREADY_STARTED    InterruptHandler is not NULL, and a handler
                                 for InterruptType was previously installed.
  @retval EFI_INVALID_PARAMETER  InterruptHandler is NULL, and a handler for
                                 InterruptType was not previously installed.
  @retval EFI_UNSUPPORTED        The interrupt specified by InterruptType
                                 is not supported.

**/
EFI_STATUS
EFIAPI
CpuRegisterInterruptHandler (
  IN EFI_CPU_ARCH_PROTOCOL      *This,
  IN EFI_EXCEPTION_TYPE         InterruptType,
  IN EFI_CPU_INTERRUPT_HANDLER  InterruptHandler
  )
{
  // MS_HYP_CHANGE BEGIN
#if defined(DEBUGLIB_ADVANCED)
  return RegisterCpuInterruptHandler (InterruptType, InterruptHandler);
#else
  if (InterruptType < 0 || InterruptType > 0xff) {
    return EFI_UNSUPPORTED;
  }

  if (InterruptHandler == NULL && ExternalVectorTable[InterruptType] == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (InterruptHandler != NULL && ExternalVectorTable[InterruptType] != NULL) {
    return EFI_ALREADY_STARTED;
  }

  if (InterruptHandler != NULL) {
    SetInterruptDescriptorTableHandlerAddress ((UINTN)InterruptType, NULL);
  } else {
    //
    // Restore the original IDT handler address if InterruptHandler is NULL.
    //
    RestoreInterruptDescriptorTableHandlerAddress ((UINTN)InterruptType);
  }

  ExternalVectorTable[InterruptType] = InterruptHandler;
  return EFI_SUCCESS;
#endif
  // MS_HYP_CHANGE END
}


/**
  Returns a timer value from one of the CPU's internal timers. There is no
  inherent time interval between ticks but is a function of the CPU frequency.

  @param  This                - Protocol instance structure.
  @param  TimerIndex          - Specifies which CPU timer is requested.
  @param  TimerValue          - Pointer to the returned timer value.
  @param  TimerPeriod         - A pointer to the amount of time that passes
                                in femtoseconds (10-15) for each increment
                                of TimerValue. If TimerValue does not
                                increment at a predictable rate, then 0 is
                                returned.  The amount of time that has
                                passed between two calls to GetTimerValue()
                                can be calculated with the formula
                                (TimerValue2 - TimerValue1) * TimerPeriod.
                                This parameter is optional and may be NULL.

  @retval EFI_SUCCESS           - If the CPU timer count was returned.
  @retval EFI_UNSUPPORTED       - If the CPU does not have any readable timers.
  @retval EFI_DEVICE_ERROR      - If an error occurred while reading the timer.
  @retval EFI_INVALID_PARAMETER - TimerIndex is not valid or TimerValue is NULL.

**/
EFI_STATUS
EFIAPI
CpuGetTimerValue (
  IN  EFI_CPU_ARCH_PROTOCOL  *This,
  IN  UINT32                 TimerIndex,
  OUT UINT64                 *TimerValue,
  OUT UINT64                 *TimerPeriod OPTIONAL
  )
{
  if (TimerValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (TimerIndex != 0) {
    return EFI_INVALID_PARAMETER;
  }

  *TimerValue = AsmReadTsc ();

  if (TimerPeriod != NULL) {
      //
      // BugBug: Hard coded. Don't know how to do this generically
      //
      *TimerPeriod = 1000000000;  // MS_HYP_CHANGE
  }

  return EFI_SUCCESS;
}

/**
  A minimal wrapper function that allows MtrrSetAllMtrrs() to be passed to
  EFI_MP_SERVICES_PROTOCOL.StartupAllAPs() as Procedure.

  @param[in] Buffer  Pointer to an MTRR_SETTINGS object, to be passed to
                     MtrrSetAllMtrrs().
**/
VOID
EFIAPI
SetMtrrsFromBuffer (
  IN VOID  *Buffer
  )
{
  MtrrSetAllMtrrs (Buffer);
}

/**
  Implementation of SetMemoryAttributes() service of CPU Architecture Protocol.

  This function modifies the attributes for the memory region specified by BaseAddress and
  Length from their current attributes to the attributes specified by Attributes.

  @param  This             The EFI_CPU_ARCH_PROTOCOL instance.
  @param  BaseAddress      The physical address that is the start address of a memory region.
  @param  Length           The size in bytes of the memory region.
  @param  Attributes       The bit mask of attributes to set for the memory region.

  @retval EFI_SUCCESS           The attributes were set for the memory region.
  @retval EFI_ACCESS_DENIED     The attributes for the memory resource range specified by
                                BaseAddress and Length cannot be modified.
  @retval EFI_INVALID_PARAMETER Length is zero.
                                Attributes specified an illegal combination of attributes that
                                cannot be set together.
  @retval EFI_OUT_OF_RESOURCES  There are not enough system resources to modify the attributes of
                                the memory resource range.
  @retval EFI_UNSUPPORTED       The processor does not support one or more bytes of the memory
                                resource range specified by BaseAddress and Length.
                                The bit mask of attributes is not support for the memory resource
                                range specified by BaseAddress and Length.

**/
EFI_STATUS
EFIAPI
CpuSetMemoryAttributes (
  IN EFI_CPU_ARCH_PROTOCOL  *This,
  IN EFI_PHYSICAL_ADDRESS   BaseAddress,
  IN UINT64                 Length,
  IN UINT64                 Attributes
  )
{
  RETURN_STATUS             Status;
  MTRR_MEMORY_CACHE_TYPE    CacheType;
  UINT64                    CacheAttributes;
  UINT64                    MemoryAttributes;
  MTRR_MEMORY_CACHE_TYPE    CurrentCacheType;

  //
  // If this function is called because GCD SetMemorySpaceAttributes () is called
  // by RefreshGcdMemoryAttributes (), then we are just synchronizing GCD memory
  // map with MTRR values. So there is no need to modify MTRRs, just return immediately
  // to avoid unnecessary computing.
  //
  if (mIsFlushingGCD) {
    DEBUG ((DEBUG_VERBOSE, "  Flushing GCD\n"));
    return EFI_SUCCESS;
  }

  //
  // During memory attributes updating, new pages may be allocated to setup
  // smaller granularity of page table. Page allocation action might then cause
  // another calling of CpuSetMemoryAttributes() recursively, due to memory
  // protection policy configured (such as the DXE NX Protection Policy). // MU_CHANGE
  // Since this driver will always protect memory used as page table by itself,
  // there's no need to apply protection policy requested from memory service.
  // So it's safe to just return EFI_SUCCESS if this time of calling is caused
  // by page table memory allocation.
  //
  if (mIsAllocatingPageTable) {
    DEBUG ((DEBUG_VERBOSE, "  Allocating page table memory\n"));
    return EFI_SUCCESS;
  }

  CacheAttributes  = Attributes & EFI_CACHE_ATTRIBUTE_MASK;
  MemoryAttributes = Attributes & EFI_MEMORY_ATTRIBUTE_MASK;

  if (Attributes != (CacheAttributes | MemoryAttributes)) {
    return EFI_INVALID_PARAMETER;
  }

  if (CacheAttributes != 0) {
    switch (CacheAttributes) {
      case EFI_MEMORY_UC:
        CacheType = CacheUncacheable;
        break;

      case EFI_MEMORY_WC:
        CacheType = CacheWriteCombining;
        break;

      case EFI_MEMORY_WT:
        CacheType = CacheWriteThrough;
        break;

      case EFI_MEMORY_WP:
        CacheType = CacheWriteProtected;
        break;

      case EFI_MEMORY_WB:
        CacheType = CacheWriteBack;
        break;

      default:
        return EFI_INVALID_PARAMETER;
    }

    // MS_HYP_CHANGE BEGIN
    //
    // If this system enforces hardware isolation with no paravisor, then
    // cache attribute changes are not possible.  However, this routine may
    // still be called to adjust memory permissions for addresses that have
    // writeback attributes.  If the cache type is writeback, then ignore any
    // attribute changes.
    //
    CurrentCacheType = MtrrGetMemoryAttribute(BaseAddress);

    if (!mStrictIsolation && (CacheType != CacheWriteBack) && (CurrentCacheType != CacheType)) {

      if (!IsMtrrSupported ()) {
          return EFI_INVALID_PARAMETER;
      }
    // MS_HYP_CHANGE BEGIN

      //
      // call MTRR library function
      //
      Status = MtrrSetMemoryAttribute (
                 BaseAddress,
                 Length,
                 CacheType
                 );

      if (EFI_ERROR (Status)) {
        return Status;
      }
    }
  }

  //
  // Set memory attribute by page table
  //
  return AssignMemoryPageAttributes (NULL, BaseAddress, Length, MemoryAttributes, NULL);
}

// MS_HYP_CHANGE BEGIN
/**
  Waits for an interrupt to arrive, then enables CPU interrupts.

  @param  This              Protocol instance structure

  @retval EFI_SUCCESS       If interrupts were enabled in the CPU

**/
EFI_STATUS
EFIAPI
CpuWaitForAndEnableInterrupt (
  IN EFI_CPU2_PROTOCOL          *This
  )
{
  EnableInterruptsAndSleep ();

  return EFI_SUCCESS;
}
// MS_HYP_CHANGE END

/**
  Initializes the valid bits mask and valid address mask for MTRRs.

  This function initializes the valid bits mask and valid address mask for MTRRs.

**/
VOID
InitializeMtrrMask (
  VOID
  )
{
  UINT32  RegEax;
  UINT8   PhysicalAddressBits;

  AsmCpuid (0x80000000, &RegEax, NULL, NULL, NULL);

  if (RegEax >= 0x80000008) {
    AsmCpuid (0x80000008, &RegEax, NULL, NULL, NULL);

    PhysicalAddressBits = (UINT8)RegEax;
  } else {
    PhysicalAddressBits = 36;
  }

  mValidMtrrBitsMask    = LShiftU64 (1, PhysicalAddressBits) - 1;
  mValidMtrrAddressMask = mValidMtrrBitsMask & 0xfffffffffffff000ULL;
}

/**
  Gets GCD Mem Space type from MTRR Type.

  This function gets GCD Mem Space type from MTRR Type.

  @param  MtrrAttributes  MTRR memory type

  @return GCD Mem Space type

**/
UINT64
GetMemorySpaceAttributeFromMtrrType (
  IN UINT8  MtrrAttributes
  )
{
  switch (MtrrAttributes) {
    case MTRR_CACHE_UNCACHEABLE:
      return EFI_MEMORY_UC;
    case MTRR_CACHE_WRITE_COMBINING:
      return EFI_MEMORY_WC;
    case MTRR_CACHE_WRITE_THROUGH:
      return EFI_MEMORY_WT;
    case MTRR_CACHE_WRITE_PROTECTED:
      return EFI_MEMORY_WP;
    case MTRR_CACHE_WRITE_BACK:
      return EFI_MEMORY_WB;
    default:
      return 0;
  }
}

/**
  Searches memory descriptors covered by given memory range.

  This function searches into the Gcd Memory Space for descriptors
  (from StartIndex to EndIndex) that contains the memory range
  specified by BaseAddress and Length.

  @param  MemorySpaceMap       Gcd Memory Space Map as array.
  @param  NumberOfDescriptors  Number of descriptors in map.
  @param  BaseAddress          BaseAddress for the requested range.
  @param  Length               Length for the requested range.
  @param  StartIndex           Start index into the Gcd Memory Space Map.
  @param  EndIndex             End index into the Gcd Memory Space Map.

  @retval EFI_SUCCESS          Search successfully.
  @retval EFI_NOT_FOUND        The requested descriptors does not exist.

**/
EFI_STATUS
SearchGcdMemorySpaces (
  IN EFI_GCD_MEMORY_SPACE_DESCRIPTOR  *MemorySpaceMap,
  IN UINTN                            NumberOfDescriptors,
  IN EFI_PHYSICAL_ADDRESS             BaseAddress,
  IN UINT64                           Length,
  OUT UINTN                           *StartIndex,
  OUT UINTN                           *EndIndex
  )
{
  UINTN  Index;

  *StartIndex = 0;
  *EndIndex   = 0;
  for (Index = 0; Index < NumberOfDescriptors; Index++) {
    if ((BaseAddress >= MemorySpaceMap[Index].BaseAddress) &&
        (BaseAddress < MemorySpaceMap[Index].BaseAddress + MemorySpaceMap[Index].Length))
    {
      *StartIndex = Index;
    }

    if ((BaseAddress + Length - 1 >= MemorySpaceMap[Index].BaseAddress) &&
        (BaseAddress + Length - 1 < MemorySpaceMap[Index].BaseAddress + MemorySpaceMap[Index].Length))
    {
      *EndIndex = Index;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

/**
  Sets the attributes for a specified range in Gcd Memory Space Map.

  This function sets the attributes for a specified range in
  Gcd Memory Space Map.

  @param  MemorySpaceMap       Gcd Memory Space Map as array
  @param  NumberOfDescriptors  Number of descriptors in map
  @param  BaseAddress          BaseAddress for the range
  @param  Length               Length for the range
  @param  Attributes           Attributes to set

  @retval EFI_SUCCESS          Memory attributes set successfully
  @retval EFI_NOT_FOUND        The specified range does not exist in Gcd Memory Space

**/
EFI_STATUS
SetGcdMemorySpaceAttributes (
  IN EFI_GCD_MEMORY_SPACE_DESCRIPTOR  *MemorySpaceMap,
  IN UINTN                            NumberOfDescriptors,
  IN EFI_PHYSICAL_ADDRESS             BaseAddress,
  IN UINT64                           Length,
  IN UINT64                           Attributes
  )
{
  EFI_STATUS            Status;
  UINTN                 Index;
  UINTN                 StartIndex;
  UINTN                 EndIndex;
  EFI_PHYSICAL_ADDRESS  RegionStart;
  UINT64                RegionLength;

  //
  // Get all memory descriptors covered by the memory range
  //
  Status = SearchGcdMemorySpaces (
             MemorySpaceMap,
             NumberOfDescriptors,
             BaseAddress,
             Length,
             &StartIndex,
             &EndIndex
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Go through all related descriptors and set attributes accordingly
  //
  for (Index = StartIndex; Index <= EndIndex; Index++) {
    if (MemorySpaceMap[Index].GcdMemoryType == EfiGcdMemoryTypeNonExistent) {
      continue;
    }

    //
    // Calculate the start and end address of the overlapping range
    //
    if (BaseAddress >= MemorySpaceMap[Index].BaseAddress) {
      RegionStart = BaseAddress;
    } else {
      RegionStart = MemorySpaceMap[Index].BaseAddress;
    }

    if (BaseAddress + Length - 1 < MemorySpaceMap[Index].BaseAddress + MemorySpaceMap[Index].Length) {
      RegionLength = BaseAddress + Length - RegionStart;
    } else {
      RegionLength = MemorySpaceMap[Index].BaseAddress + MemorySpaceMap[Index].Length - RegionStart;
    }

    //
    // Set memory attributes according to MTRR attribute and the original attribute of descriptor
    //
    gDS->SetMemorySpaceAttributes (
           RegionStart,
           RegionLength,
           (MemorySpaceMap[Index].Attributes & ~EFI_MEMORY_CACHETYPE_MASK) | (MemorySpaceMap[Index].Capabilities & Attributes)
           );   // MS_HYP_CHANGE
  }

  return EFI_SUCCESS;
}

/**
  Refreshes the GCD Memory Space attributes according to MTRRs.

  This function refreshes the GCD Memory Space attributes according to MTRRs.

**/
VOID
RefreshMemoryAttributesFromMtrr (
  VOID
  )
{
  EFI_STATUS                       Status;
  UINTN                            Index;
  UINTN                            SubIndex;
  UINT64                           RegValue;
  EFI_PHYSICAL_ADDRESS             BaseAddress;
  UINT64                           Length;
  UINT64                           Attributes;
  UINT64                           CurrentAttributes;
  UINT8                            MtrrType;
  UINTN                            NumberOfDescriptors;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR  *MemorySpaceMap;
  UINT64                           DefaultAttributes;
  VARIABLE_MTRR                    VariableMtrr[MTRR_NUMBER_OF_VARIABLE_MTRR];
  MTRR_FIXED_SETTINGS              MtrrFixedSettings;
  UINT32                           FirmwareVariableMtrrCount;
  UINT8                            DefaultMemoryType;

  FirmwareVariableMtrrCount = GetFirmwareVariableMtrrCount ();
  ASSERT (FirmwareVariableMtrrCount <= MTRR_NUMBER_OF_VARIABLE_MTRR);

  MemorySpaceMap = NULL;

  //
  // Initialize the valid bits mask and valid address mask for MTRRs
  //
  InitializeMtrrMask ();

  //
  // Get the memory attribute of variable MTRRs
  //
  MtrrGetMemoryAttributeInVariableMtrr (
    mValidMtrrBitsMask,
    mValidMtrrAddressMask,
    VariableMtrr
    );

  //
  // Get the memory space map from GCD
  //
  Status = gDS->GetMemorySpaceMap (
                  &NumberOfDescriptors,
                  &MemorySpaceMap
                  );
  ASSERT_EFI_ERROR (Status);

  DefaultMemoryType = (UINT8)MtrrGetDefaultMemoryType ();
  DefaultAttributes = GetMemorySpaceAttributeFromMtrrType (DefaultMemoryType);

  //
  // Set default attributes to all spaces.
  //
  for (Index = 0; Index < NumberOfDescriptors; Index++) {
    if (MemorySpaceMap[Index].GcdMemoryType == EfiGcdMemoryTypeNonExistent) {
      continue;
    }

    gDS->SetMemorySpaceAttributes (
           MemorySpaceMap[Index].BaseAddress,
           MemorySpaceMap[Index].Length,
           (MemorySpaceMap[Index].Attributes & ~EFI_MEMORY_CACHETYPE_MASK) |
           (MemorySpaceMap[Index].Capabilities & DefaultAttributes)
           );   // MS_HYP_CHANGE
  }

  //
  // Go for variable MTRRs with WB attribute
  //
  for (Index = 0; Index < FirmwareVariableMtrrCount; Index++) {
    if (VariableMtrr[Index].Valid &&
        (VariableMtrr[Index].Type == MTRR_CACHE_WRITE_BACK))
    {
      SetGcdMemorySpaceAttributes (
        MemorySpaceMap,
        NumberOfDescriptors,
        VariableMtrr[Index].BaseAddress,
        VariableMtrr[Index].Length,
        EFI_MEMORY_WB
        );
    }
  }

  //
  // Go for variable MTRRs with the attribute except for WB and UC attributes
  //
  for (Index = 0; Index < FirmwareVariableMtrrCount; Index++) {
    if (VariableMtrr[Index].Valid &&
        (VariableMtrr[Index].Type != MTRR_CACHE_WRITE_BACK) &&
        (VariableMtrr[Index].Type != MTRR_CACHE_UNCACHEABLE))
    {
      Attributes = GetMemorySpaceAttributeFromMtrrType ((UINT8)VariableMtrr[Index].Type);
      SetGcdMemorySpaceAttributes (
        MemorySpaceMap,
        NumberOfDescriptors,
        VariableMtrr[Index].BaseAddress,
        VariableMtrr[Index].Length,
        Attributes
        );
    }
  }

  //
  // Go for variable MTRRs with UC attribute
  //
  for (Index = 0; Index < FirmwareVariableMtrrCount; Index++) {
    if (VariableMtrr[Index].Valid &&
        (VariableMtrr[Index].Type == MTRR_CACHE_UNCACHEABLE))
    {
      SetGcdMemorySpaceAttributes (
        MemorySpaceMap,
        NumberOfDescriptors,
        VariableMtrr[Index].BaseAddress,
        VariableMtrr[Index].Length,
        EFI_MEMORY_UC
        );
    }
  }

  //
  // Go for fixed MTRRs
  //
  Attributes  = 0;
  BaseAddress = 0;
  Length      = 0;
  MtrrGetFixedMtrr (&MtrrFixedSettings);
  for (Index = 0; Index < MTRR_NUMBER_OF_FIXED_MTRR; Index++) {
    RegValue = MtrrFixedSettings.Mtrr[Index];
    //
    // Check for continuous fixed MTRR sections
    //
    for (SubIndex = 0; SubIndex < 8; SubIndex++) {
      MtrrType          = (UINT8)RShiftU64 (RegValue, SubIndex * 8);
      CurrentAttributes = GetMemorySpaceAttributeFromMtrrType (MtrrType);
      if (Length == 0) {
        //
        // A new MTRR attribute begins
        //
        Attributes = CurrentAttributes;
      } else {
        //
        // If fixed MTRR attribute changed, then set memory attribute for previous attribute
        //
        if (CurrentAttributes != Attributes) {
          SetGcdMemorySpaceAttributes (
            MemorySpaceMap,
            NumberOfDescriptors,
            BaseAddress,
            Length,
            Attributes
            );
          BaseAddress = mFixedMtrrTable[Index].BaseAddress + mFixedMtrrTable[Index].Length * SubIndex;
          Length      = 0;
          Attributes  = CurrentAttributes;
        }
      }

      Length += mFixedMtrrTable[Index].Length;
    }
  }

  //
  // Handle the last fixed MTRR region
  //
  SetGcdMemorySpaceAttributes (
    MemorySpaceMap,
    NumberOfDescriptors,
    BaseAddress,
    Length,
    Attributes
    );

  //
  // Free memory space map allocated by GCD service GetMemorySpaceMap ()
  //
  if (MemorySpaceMap != NULL) {
    FreePool (MemorySpaceMap);
  }
}

// MS_HYP_CHANGE BEGIN
/**
  Set Interrupt Descriptor Table Handler Address.

  @param Index        The Index of the interrupt descriptor table handle.
  @param Handler      Handler address.

**/
VOID
SetInterruptDescriptorTableHandlerAddress (
  IN UINTN Index,
  IN VOID  *Handler  OPTIONAL
  )
{
  UINTN                     UintnHandler;

  if (Handler != NULL) {
    UintnHandler = (UINTN) Handler;
  } else {
    UintnHandler = ((UINTN) AsmIdtVector00) + (8 * Index);
  }

  gIdtTable[Index].Bits.Selector    = AsmReadCs();
  gIdtTable[Index].Bits.OffsetLow   = (UINT16)UintnHandler;
  gIdtTable[Index].Bits.Reserved_0  = 0;
  gIdtTable[Index].Bits.GateType    = IA32_IDT_GATE_TYPE_INTERRUPT_32;
  gIdtTable[Index].Bits.OffsetHigh  = (UINT16)(UintnHandler >> 16);
#if defined (MDE_CPU_X64)
  gIdtTable[Index].Bits.OffsetUpper = (UINT32)(UintnHandler >> 32);
  gIdtTable[Index].Bits.Reserved_1  = 0;
#endif
}

/**
  Restore original Interrupt Descriptor Table Handler Address.

  @param Index        The Index of the interrupt descriptor table handle.

**/
VOID
RestoreInterruptDescriptorTableHandlerAddress (
  IN UINTN       Index
  )
{
  if (Index < mOrigIdtEntryCount) {
    gIdtTable[Index].Bits.OffsetLow   = mOrigIdtEntry[Index].Bits.OffsetLow;
    gIdtTable[Index].Bits.OffsetHigh  = mOrigIdtEntry[Index].Bits.OffsetHigh;
#if defined (MDE_CPU_X64)
    gIdtTable[Index].Bits.OffsetUpper = mOrigIdtEntry[Index].Bits.OffsetUpper;
#endif
  }
}
// MS_HYP_CHANGE END

/**
 Check if paging is enabled or not.
**/
BOOLEAN
IsPagingAndPageAddressExtensionsEnabled (
  VOID
  )
{
  IA32_CR0  Cr0;
  IA32_CR4  Cr4;

  Cr0.UintN = AsmReadCr0 ();
  Cr4.UintN = AsmReadCr4 ();

  return ((Cr0.Bits.PG != 0) && (Cr4.Bits.PAE != 0));
}

/**
  Refreshes the GCD Memory Space attributes according to MTRRs and Paging.

  This function refreshes the GCD Memory Space attributes according to MTRRs
  and page tables.

**/
VOID
RefreshGcdMemoryAttributes (
  VOID
  )
{
  mIsFlushingGCD = TRUE;

  if (IsMtrrSupported ()) {
    RefreshMemoryAttributesFromMtrr ();
  }

  // MS_HYP_CHANGE BEGIN
  //if (IsPagingAndPageAddressExtensionsEnabled ()) {
  //  DEBUG ((DEBUG_INFO, "Syncing GCD...\n")); // MU_CHANGE
  //  RefreshGcdMemoryAttributesFromPaging ();
  //}
  // MS_HYP_CHANGE END

  mIsFlushingGCD = FALSE;
}

/**
  Initialize Interrupt Descriptor Table for interrupt handling.

**/
VOID
InitInterruptDescriptorTable (
  VOID
  )
{
#if defined(DEBUGLIB_ADVANCED)
  EFI_STATUS                Status;
  EFI_VECTOR_HANDOFF_INFO   *VectorInfoList;
  EFI_VECTOR_HANDOFF_INFO   *VectorInfo;
  IA32_IDT_GATE_DESCRIPTOR  *IdtTable;
  IA32_DESCRIPTOR           IdtDescriptor;
  UINTN                     IdtEntryCount;

  VectorInfo = NULL;
  Status     = EfiGetSystemConfigurationTable (&gEfiVectorHandoffTableGuid, (VOID **)&VectorInfoList);
  if ((Status == EFI_SUCCESS) && (VectorInfoList != NULL)) {
    VectorInfo = VectorInfoList;
  }

  AsmReadIdtr (&IdtDescriptor);
  IdtEntryCount = (IdtDescriptor.Limit + 1) / sizeof (IA32_IDT_GATE_DESCRIPTOR);
  if (IdtEntryCount < CPU_INTERRUPT_NUM) {
    //
    // Increase Interrupt Descriptor Table and Copy the old IDT table in
    //
    IdtTable = gIdtTable; // MS_HYP_CHANGE
    ASSERT (IdtTable != NULL);
    CopyMem (IdtTable, (VOID *)IdtDescriptor.Base, sizeof (IA32_IDT_GATE_DESCRIPTOR) * IdtEntryCount);

    //
    // Load Interrupt Descriptor Table
    //
    IdtDescriptor.Base  = (UINTN)IdtTable;
    IdtDescriptor.Limit = (UINT16)(sizeof (IA32_IDT_GATE_DESCRIPTOR) * CPU_INTERRUPT_NUM - 1);
    AsmWriteIdtr (&IdtDescriptor);
  }

  Status = InitializeCpuExceptionHandlers (VectorInfo);
  ASSERT_EFI_ERROR (Status);

#else

  EFI_STATUS                Status;
  IA32_DESCRIPTOR           OldIdtPtr;
  IA32_IDT_GATE_DESCRIPTOR  *OldIdt;
  UINTN                     OldIdtSize;
  __declspec(align(16)) IA32_DESCRIPTOR IdtPtr;
  UINTN                     Index;
  UINT16                    CurrentCs;
  VOID                      *IntHandler;

  SetMem (ExternalVectorTable, sizeof(ExternalVectorTable), 0);

  //
  // Get original IDT address and size.
  //
  AsmReadIdtr ((IA32_DESCRIPTOR *) &OldIdtPtr);

  if ((OldIdtPtr.Base != 0) && ((OldIdtPtr.Limit & 7) == 7)) {
    OldIdt = (IA32_IDT_GATE_DESCRIPTOR*) OldIdtPtr.Base;
    OldIdtSize = (OldIdtPtr.Limit + 1) / sizeof (IA32_IDT_GATE_DESCRIPTOR);
    //
    // Save original IDT entry and IDT entry count.
    //
    CopyMem(mOrigIdtEntry, (VOID *) OldIdtPtr.Base, OldIdtPtr.Limit + 1);
    mOrigIdtEntryCount = (UINT16) OldIdtSize;
  } else {
    OldIdt = NULL;
    OldIdtSize = 0;
  }

  //
  // Intialize IDT
  //
  CurrentCs = AsmReadCs();
  for (Index = 0; Index < CPU_INTERRUPT_NUM; Index ++) {

    //
    // If the old IDT had a handler for this interrupt, then
    // preserve it.
    //
    if ((Index < OldIdtSize) &&
        (OldIdt[Index].Bits.GateType != 0)) {

      IntHandler =
        (VOID*) (
          OldIdt[Index].Bits.OffsetLow +
          (((UINTN) OldIdt[Index].Bits.OffsetHigh) << 16)
#if defined (MDE_CPU_X64)
            + (((UINTN) OldIdt[Index].Bits.OffsetUpper) << 32)
#endif
          );

        SetInterruptDescriptorTableHandlerAddress (Index, IntHandler);
    }
  }

  //
  // Load IDT Pointer
  //
  IdtPtr.Base = (UINT32)(((UINTN)(VOID*) gIdtTable) & (BASE_4GB-1));
  IdtPtr.Limit = (UINT16) (sizeof (gIdtTable) - 1);

  AsmWriteIdtr (&IdtPtr);

  //
  // Initialize Exception Handlers
  //
  for (Index = 0; Index < 32; Index++) {
    if (gIdtTable[Index].Bits.GateType == 0) {
      Status = CpuRegisterInterruptHandler (&gCpu, Index, CommonExceptionHandlerMsvm);
      ASSERT_EFI_ERROR (Status);
    }
  }

#endif
}

#if defined(MDE_CPU_X64)

/**
  Callback function for end of DXE.

  @param  Event                 Event whose notification function is being invoked.
  @param  Context               The pointer to the notification function's context,
                                which is implementation-dependent.

**/
VOID
EFIAPI
EndOfDxeCallback (
  IN EFI_EVENT                Event,
  IN VOID                     *Context
  )
{

  MP_WAKEUP_MAILBOX *ApMailbox;
  UINT32 ProcessorCount;
  UINT32 VpIndex;
  HV_INITIAL_VP_CONTEXT  VpContext;
  EFI_STATUS  Status;
  EFI_PHYSICAL_ADDRESS PageTableBase = 0;
  TDX_CONTEXT *TdxApStartContext;
  UINT32 ApWaitInMailboxFunctionSize = 0;

  ApMailbox = (MP_WAKEUP_MAILBOX *) PcdGet64(PcdAcpiMadtMpMailBoxAddress);
  ProcessorCount = PcdGet32(PcdProcessorCount);

  ASSERT(ApMailbox != NULL);
  ASSERT (mIsolationType == UefiIsolationTypeTdx);
  ASSERT (ProcessorCount > 1);

  ZeroMem(&VpContext, sizeof(HV_INITIAL_VP_CONTEXT));
  ApMailbox->HasVcpuEnteredMailboxWait = 0;

  Status = gBS->LocateProtocol(&gEfiHvProtocolGuid, NULL, (VOID **)&mHv);
  if (EFI_ERROR(Status))
  {
      DEBUG((EFI_D_ERROR, "%a: Failed to locate the protocol.\n", __FUNCTION__));
      FAIL_FAST_INITIALIZATION_FAILURE(Status);
  }

  //
  // Setup the wake up code
  //
  ApWaitInMailboxFunctionSize = (UINT32)((UINT8 *)ApWaitInMailboxEnd - (UINT8 *)ApWaitInMailbox);
  ASSERT(ApWaitInMailboxFunctionSize <= AP_WAIT_IN_MAILBOX_CODE_MAX_SIZE);

  //
  // Set up the pagetables, reset page and the execution environment.
  //
  PageTableBase = InitializeMpPageTables((UINT64)ApMailbox);
  if (PageTableBase == 0)
  {
      DEBUG((EFI_D_ERROR, "%a: Failed to initialize the page tables\n", __FUNCTION__));
      FAIL_FAST_INITIALIZATION_FAILURE(EFI_OUT_OF_RESOURCES);
  }

  CopyMem (
    ApMailbox->ApWaitInMailboxCode,
    (UINT8 *)ApWaitInMailbox,
    ApWaitInMailboxFunctionSize
    );

  TdxApStartContext = (TDX_CONTEXT *)((EFI_PHYSICAL_ADDRESS)(0xFFFFF000));
  TdxApStartContext->gdtrLimit = 0;
  TdxApStartContext->idtrLimit = 0;
  TdxApStartContext->taskSelector = 0;
  TdxApStartContext->codeSelector = 0;

  TdxApStartContext->cr3 = AsmReadCr3();
  TdxApStartContext->initialRip = (EFI_PHYSICAL_ADDRESS)ApMailbox->ApWaitInMailboxCode;

  TdxApStartContext->r8 = (EFI_PHYSICAL_ADDRESS)ApMailbox;
  TdxApStartContext->r10 = PageTableBase;

  //
  // Setup and start all the APs. VCPU0 is the BSP.
  //
  for (VpIndex = 1; VpIndex < ProcessorCount; VpIndex++)
  {

    //
    // Once startGate is setup, the Hypervisor could start the VP. All the context setup
    // should be completed before setting the startGate. After setting the startGate, the
    // context should not be modified until the AP has entered the mailbox wait.
    //
    TdxApStartContext->r9 = VpIndex;
    TdxApStartContext->startGate = VpIndex;


    //
    // Wake up the processor so that it can start executing the AP wait loop.
    //

    Status = mHv->StartApplicationProcessor(
                    mHv,
                    VpIndex,
                    &VpContext);

    if (EFI_ERROR(Status))
    {
      DEBUG((EFI_D_ERROR, "%a: Failed to wakeup AP : %u\n", __FUNCTION__, VpIndex));
      FAIL_FAST_INITIALIZATION_FAILURE(Status);
    }

    //
    // Wait for this AP to enter the wait loop before moving on to the next AP.
    //

    DEBUG((EFI_D_INFO, "Waiting for AP(%u) to wait the mailbox. \n", VpIndex));

    while (ApMailbox->HasVcpuEnteredMailboxWait != 1) {
      CpuPause();
    }
    DEBUG((EFI_D_INFO, "AP(%u) is waiting in the mailbox\n", VpIndex));
    ApMailbox->HasVcpuEnteredMailboxWait = 0;
  }

  gBS->CloseEvent(mEndOfDxeEvent);
}

#endif

// MS_HYP_CHANGE END

/**
  Callback function for idle events.

  @param  Event                 Event whose notification function is being invoked.
  @param  Context               The pointer to the notification function's context,
                                which is implementation-dependent.

**/
VOID
EFIAPI
IdleLoopEventCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  CpuSleep ();
}

/**
  Ensure the compatibility of a memory space descriptor with the MMIO aperture.

  The memory space descriptor can come from the GCD memory space map, or it can
  represent a gap between two neighboring memory space descriptors. In the
  latter case, the GcdMemoryType field is expected to be
  EfiGcdMemoryTypeNonExistent.

  If the memory space descriptor already has type
  EfiGcdMemoryTypeMemoryMappedIo, and its capabilities are a superset of the
  required capabilities, then no action is taken -- it is by definition
  compatible with the aperture.

  Otherwise, the intersection of the memory space descriptor is calculated with
  the aperture. If the intersection is the empty set (no overlap), no action is
  taken; the memory space descriptor is compatible with the aperture.

  Otherwise, the type of the descriptor is investigated again. If the type is
  EfiGcdMemoryTypeNonExistent (representing a gap, or a genuine descriptor with
  such a type), then an attempt is made to add the intersection as MMIO space
  to the GCD memory space map, with the specified capabilities. This ensures
  continuity for the aperture, and the descriptor is deemed compatible with the
  aperture.

  Otherwise, the memory space descriptor is incompatible with the MMIO
  aperture.

  @param[in] Base         Base address of the aperture.
  @param[in] Length       Length of the aperture.
  @param[in] Capabilities Capabilities required by the aperture.
  @param[in] Descriptor   The descriptor to ensure compatibility with the
                          aperture for.

  @retval EFI_SUCCESS            The descriptor is compatible. The GCD memory
                                 space map may have been updated, for
                                 continuity within the aperture.
  @retval EFI_INVALID_PARAMETER  The descriptor is incompatible.
  @return                        Error codes from gDS->AddMemorySpace().
**/
EFI_STATUS
IntersectMemoryDescriptor (
  IN  UINT64                                 Base,
  IN  UINT64                                 Length,
  IN  UINT64                                 Capabilities,
  IN  CONST EFI_GCD_MEMORY_SPACE_DESCRIPTOR  *Descriptor
  )
{
  UINT64      IntersectionBase;
  UINT64      IntersectionEnd;
  EFI_STATUS  Status;

  if ((Descriptor->GcdMemoryType == EfiGcdMemoryTypeMemoryMappedIo) &&
      ((Descriptor->Capabilities & Capabilities) == Capabilities))
  {
    return EFI_SUCCESS;
  }

  IntersectionBase = MAX (Base, Descriptor->BaseAddress);
  IntersectionEnd  = MIN (
                       Base + Length,
                       Descriptor->BaseAddress + Descriptor->Length
                       );
  if (IntersectionBase >= IntersectionEnd) {
    //
    // The descriptor and the aperture don't overlap.
    //
    return EFI_SUCCESS;
  }

  if (Descriptor->GcdMemoryType == EfiGcdMemoryTypeNonExistent) {
    Status = gDS->AddMemorySpace (
                    EfiGcdMemoryTypeMemoryMappedIo,
                    IntersectionBase,
                    IntersectionEnd - IntersectionBase,
                    Capabilities
                    );

    DEBUG ((
      EFI_ERROR (Status) ? DEBUG_ERROR : DEBUG_VERBOSE,
      "%a: %a: add [%Lx, %Lx): %r\n",
      gEfiCallerBaseName,
      __FUNCTION__,
      IntersectionBase,
      IntersectionEnd,
      Status
      ));
    return Status;
  }

  DEBUG ((
    DEBUG_ERROR,
    "%a: %a: desc [%Lx, %Lx) type %u cap %Lx conflicts "
    "with aperture [%Lx, %Lx) cap %Lx\n",
    gEfiCallerBaseName,
    __FUNCTION__,
    Descriptor->BaseAddress,
    Descriptor->BaseAddress + Descriptor->Length,
    (UINT32)Descriptor->GcdMemoryType,
    Descriptor->Capabilities,
    Base,
    Base + Length,
    Capabilities
    ));
  return EFI_INVALID_PARAMETER;
}

/**
  Add MMIO space to GCD.
  The routine checks the GCD database and only adds those which are
  not added in the specified range to GCD.

  @param Base         Base address of the MMIO space.
  @param Length       Length of the MMIO space.
  @param Capabilities Capabilities of the MMIO space.

  @retval EFI_SUCCESS The MMIO space was added successfully.
**/
EFI_STATUS
AddMemoryMappedIoSpace (
  IN  UINT64  Base,
  IN  UINT64  Length,
  IN  UINT64  Capabilities
  )
{
  EFI_STATUS                       Status;
  UINTN                            Index;
  UINTN                            NumberOfDescriptors;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR  *MemorySpaceMap;

  Status = gDS->GetMemorySpaceMap (&NumberOfDescriptors, &MemorySpaceMap);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: %a: GetMemorySpaceMap(): %r\n",
      gEfiCallerBaseName,
      __FUNCTION__,
      Status
      ));
    return Status;
  }

  for (Index = 0; Index < NumberOfDescriptors; Index++) {
    Status = IntersectMemoryDescriptor (
               Base,
               Length,
               Capabilities,
               &MemorySpaceMap[Index]
               );
    if (EFI_ERROR (Status)) {
      goto FreeMemorySpaceMap;
    }
  }

  DEBUG_CODE_BEGIN ();
  //
  // Make sure there are adjacent descriptors covering [Base, Base + Length).
  // It is possible that they have not been merged; merging can be prevented
  // by allocation and different capabilities.
  //
  UINT64                           CheckBase;
  EFI_STATUS                       CheckStatus;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR  Descriptor;

  for (CheckBase = Base;
       CheckBase < Base + Length;
       CheckBase = Descriptor.BaseAddress + Descriptor.Length)
  {
    CheckStatus = gDS->GetMemorySpaceDescriptor (CheckBase, &Descriptor);
    ASSERT_EFI_ERROR (CheckStatus);
    ASSERT (Descriptor.GcdMemoryType == EfiGcdMemoryTypeMemoryMappedIo);
    ASSERT ((Descriptor.Capabilities & Capabilities) == Capabilities);
  }

  DEBUG_CODE_END ();

FreeMemorySpaceMap:
  FreePool (MemorySpaceMap);

  return Status;
}

/**
  Add and allocate CPU local APIC memory mapped space.

  @param[in]ImageHandle     Image handle this driver.

**/
VOID
AddLocalApicMemorySpace (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  BaseAddress;

  BaseAddress = (EFI_PHYSICAL_ADDRESS)GetLocalApicBaseAddress ();
  Status      = AddMemoryMappedIoSpace (BaseAddress, SIZE_4KB, EFI_MEMORY_UC);
  ASSERT_EFI_ERROR (Status);

  //
  // Try to allocate APIC memory mapped space, does not check return
  // status because it may be allocated by other driver, or DXE Core if
  // this range is built into Memory Allocation HOB.
  //
  Status = gDS->AllocateMemorySpace (
                  EfiGcdAllocateAddress,
                  EfiGcdMemoryTypeMemoryMappedIo,
                  0,
                  SIZE_4KB,
                  &BaseAddress,
                  ImageHandle,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_INFO,
      "%a: %a: AllocateMemorySpace() Status - %r\n",
      gEfiCallerBaseName,
      __FUNCTION__,
      Status
      ));
  }
}

/**
  Initialize the state information for the CPU Architectural Protocol.

  @param ImageHandle     Image handle this driver.
  @param SystemTable     Pointer to the System Table.

  @retval EFI_SUCCESS           Thread can be successfully created
  @retval EFI_OUT_OF_RESOURCES  Cannot allocate protocol data structure
  @retval EFI_DEVICE_ERROR      Cannot create the thread

**/
EFI_STATUS
EFIAPI
InitializeCpu (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   IdleLoopEvent;

  // MS_HYP_CHANGE BEGIN
  //
  // Determine whether hardware isolation is being enforced.  If so, then
  // certain aspects of hardware initialization are not supported when no
  // paravisor is present to handle them.
  //
  if (IsHardwareIsolatedNoParavisor())

  {
      mStrictIsolation = TRUE;
  }

  mIsolationType = GetIsolationType();
  // MS_HYP_CHANGE END

  InitializePageTableLib ();

  InitializeFloatingPointUnits ();

  //
  // Make sure interrupts are disabled
  //
  DisableInterrupts ();

  //
  // Init GDT for DXE
  //
  InitGlobalDescriptorTable ();

  //
  // Setup IDT pointer, IDT and interrupt entry points
  //
  InitInterruptDescriptorTable ();

  //
  // Enable the local APIC for Virtual Wire Mode.
  //
  if (!mStrictIsolation)
  {
    ProgramVirtualWireMode ();  // MS_HYP_CHANGE
  }

  //
  // Install CPU Architectural Protocol
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mCpuHandle,
                  &gEfiCpuArchProtocolGuid,
                  &gCpu,
                  // MS_HYP_CHANGE BEGIN
                  &gEfiCpu2ProtocolGuid,
                  &gCpu2,
                  // MS_HYP_CHANGE ENG
                  NULL
                  );
  // MS_HYP_CHANGE BEGIN
  if (EFI_ERROR(Status))
  {
    DEBUG((EFI_D_ERROR, "%a: Failed to install protocol.\n", __FUNCTION__));
    goto Cleanup;
  }
  // MS_HYP_CHANGE END

  //
  // Refresh GCD memory space map according to MTRR value.
  //
  RefreshGcdMemoryAttributes ();

  //
  // Setup a callback for idle events
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  IdleLoopEventCallback,
                  NULL,
                  &gIdleLoopEventGuid,
                  &IdleLoopEvent
                  );

  // MS_HYP_CHANGE BEGIN
  if (EFI_ERROR(Status))
  {
    DEBUG((EFI_D_ERROR, "%a: Failed to create the idle events callback.\n", __FUNCTION__));
    goto Cleanup;
  }

#if defined(MDE_CPU_X64)
  //
  // Setup a callback for end of DXE if this is a TDX guest with no paravisor.
  //
  if (mIsolationType == UefiIsolationTypeTdx && !IsParavisorPresent())
  {
    if (PcdGet32(PcdProcessorCount) > 1)
    {
      Status = gBS->CreateEventEx(
                      EVT_NOTIFY_SIGNAL,
                      TPL_CALLBACK,
                      EndOfDxeCallback,
                      NULL,
                      &gEfiEndOfDxeEventGroupGuid,
                      &mEndOfDxeEvent);
      if (EFI_ERROR(Status))
      {
          DEBUG((EFI_D_ERROR, "%a: Failed to create the end of DXE callback.\n", __FUNCTION__));
          goto Cleanup;
      }
    }
  }
#endif
  // MS_HYP_CHANGE END

Cleanup:

  return Status;
}
