/*++

Copyright (c) 2004  Microsoft Corporation

Module Name:

    state.c

Abstract:

    Machine specific kernel debugger data types and constants.

Author:

    Aaron Giles (aarongi) 30-Jan-2013

--*/

#include "bd.h"

VOID
KiSaveProcessorControlState (
     __out PKPROCESSOR_STATE ProcessorState
     )
/*++

 Routine Description:

   This routine saves the control state of the current processor.

 Arguments:

   ProcessorState  - Supplies a pointer to a processor state structure.

 Return Value:

    None.

--*/
{
    PKSPECIAL_REGISTERS SpecialRegisters;
    PKARM64_ARCH_STATE ArchState;

    SpecialRegisters = &ProcessorState->SpecialRegisters;
    ArchState = &ProcessorState->ArchState;

    ArchState->Midr_El1 = _ReadStatusReg(ARM64_MIDR_EL1);
    ArchState->Sctlr_El1 = _ReadStatusReg(ARM64_SCTLR_EL1);
    ArchState->Actlr_El1 = _ReadStatusReg(ARM64_ACTLR_EL1);
    ArchState->Cpacr_El1 = _ReadStatusReg(ARM64_CPACR_EL1);
    ArchState->Tcr_El1 = _ReadStatusReg(ARM64_TCR_EL1);
    ArchState->Ttbr0_El1 = _ReadStatusReg(ARM64_TTBR0_EL1);
    ArchState->Ttbr1_El1 = _ReadStatusReg(ARM64_TTBR1_EL1);
    ArchState->Esr_El1 = _ReadStatusReg(ARM64_ESR_EL1);
    ArchState->Far_El1 = _ReadStatusReg(ARM64_FAR_EL1);

    ArchState->Mair_El1 = _ReadStatusReg(ARM64_MAIR_EL1);
    ArchState->Vbar_El1 = _ReadStatusReg(ARM64_VBAR_EL1);
    
    //
    // Save thread registers.
    //

    SpecialRegisters->Tpidr_El0 = _ReadStatusReg(ARM64_TPIDR_EL0);
    SpecialRegisters->Tpidrro_El0 = _ReadStatusReg(ARM64_TPIDRRO_EL0);
    SpecialRegisters->Tpidr_El1 = _ReadStatusReg(ARM64_TPIDR_EL1);

}
