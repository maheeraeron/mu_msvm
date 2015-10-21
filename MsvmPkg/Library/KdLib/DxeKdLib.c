/*++

Copyright (c) Microsoft Corporation

Module Name:

    DxeKdLib.c

Abstract:

    DXE debug agent for UEFI debugging.

Author:

    Shreyas Srivatsan (shreyas) Sep-04-2012

--*/

#include <PiDxe.h>
#include <Base.h>

#include <Guid/DebugImageInfoTable.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/KdDebugLib.h>
#include <Library/ConfigLib.h>
#include "EfiKd.h"


RETURN_STATUS
EFIAPI
DxeDebugAgentLibConstructor(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
    )
/*++

Routine Description:

    Constructor for DXE debug agent.

Paramters:

    ImageHandle - The firmware allocated handle for the EFI image.

    SystemTable - A pointer to the EFI System Table.

Return value:

    None.

--*/
{
    return EFI_SUCCESS;
}


VOID
EFIAPI
InitializeDebugAgent (
    IN UINT32                InitFlag,
    IN VOID                  *Context, OPTIONAL
    IN DEBUG_AGENT_CONTINUE  Function  OPTIONAL
    )
/*++

Routine Description:

    Initialize debug agent.

    This function is used to set up debug environment to support source level
    debugging. If certain Debug Agent Library instance has to save some private
    data in the stack, this function must work on the mode that doesn't return
    to the caller, then the caller needs to wrap up all rest of logic after
    InitializeDebugAgent() into one function and pass it into
    InitializeDebugAgent(). InitializeDebugAgent() is responsible to invoke the
    passing-in function at the end of InitializeDebugAgent().

    If the parameter Function is not NULL, Debug Agent Libary instance will
    invoke it by passing in the Context to be its parameter.

    If Function() is NULL, Debug Agent Library instance will return after setup
    debug environment.

Paramters:

    InitFlag - Supplies a flag used to decide the initialize process.

    Context - Supplies the context needed according to InitFlag; it is optional.

    Function - Supplies a continue function called by debug agent library; it is
    optional.

Return value:

    None.

--*/
{
    if (InitFlag == DEBUG_AGENT_INIT_DXE_CORE)
    {
        EfiKdSubsystemEnabled = GetDebuggerEnabled();
    }

    EfiKdInitialize();

    if (Function != NULL)
    {
        Function (Context);
    }
}


BOOLEAN
EFIAPI
SaveAndSetDebugTimerInterrupt (
    IN BOOLEAN                EnableStatus
    )
/*++

Routine Description:

    Enable/Disable the interrupt of debug timer and return the interrupt state
    prior to the operation.

    If EnableStatus is TRUE, enable the interrupt of debug timer.
    If EnableStatus is FALSE, disable the interrupt of debug timer.

Parameters:

    EnableStatus - Supplies a value whether to enable/disable timer interrupt.

Return value:

    FALSE always.

--*/
{
    return FALSE;
}
