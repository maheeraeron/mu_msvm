/*++

Copyright (c) Microsoft Corporation

Module Name:

    DxeBdLib.c

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
#include <Library/CrashDumpAgentLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/BdDebugLib.h>
#include <Library/HobLib.h>
#include "Bd.h"


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
    VOID *kdnetHob;

    kdnetHob = NULL;

    BdSerialPrint(">>> %a (%lx, %p, %p)\n", __FUNCTION__, InitFlag, Context, Function);
    if (InitFlag == DEBUG_AGENT_INIT_DXE_CORE)
    {
        //
        // InitializeDebugAgent is called very early on in DXE Core, before any
        // drivers are dispatched. Thus, we can't use the PcdDebuggerEnabled to
        // determine if debuggers are enabled, rather we search for the special
        // HOB containing just this flag to determine if the debug system should
        // be enabled or not.
        //
        void* hob = GetFirstGuidHob(&gMsvmDebuggerEnabledGuid);
        if (hob != NULL)
        {
            BdSubsystemEnabled = *((BOOLEAN *)GET_GUID_HOB_DATA(hob));
        }
        else
        {
            // We should always be passing this HOB.
            ASSERT(FALSE);
        }
        BdSerialPrint("--- %a: DebuggerEnabled %a\n", __FUNCTION__,
            BdSubsystemEnabled ? "TRUE" : "FALSE");

        //
        // Get the hob that specifies where the KDNET binary was loaded to in
        // PEI.
        //
        if (BdSubsystemEnabled)
        {
            hob = GetFirstGuidHob(&gMsvmDebuggerKdnetBinaryGuid);
            if (hob != NULL)
            {
                kdnetHob = GET_GUID_HOB_DATA(hob);
            }
        }

        //
        // Initialize crashdump, only once when INIT called during DXE Core.
        // Context is the HOB list when called from DXE Core.
        //
        InitializeCrashDumpAgent(Context);
    }

    BdInitialize(kdnetHob);

    if (Function != NULL)
    {
        BdSerialPrint("--- %a: Calling function %p\n", __FUNCTION__, Function);
        Function (Context);
    }
    BdSerialPrint("<<< %a\n", __FUNCTION__);
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
