#include "UefiHavoc.h"

EFI_STATUS
EFIAPI
NewPluginInit(
    )
/*++

Routine Description:

    Callback used to initialize any required plugin state.

Arguments:

    None.

Return Value:

    None.
    
--*/
{
    //
    // Use this function to set up anything that's required for your havoc operation
    // to execute iterations. It will be executed a single time before the Havoc
    // method below is called.
    //

    //
    // Returning a non-EFI_SUCCESS error code will halt the Havoc initializations and print an error message.
    // 

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
NewPluginHavoc(
    _In_  HAVOC_PLUGIN   *Plugin
    )
/*++

Routine Description:

    Callback that is called to execute a single plugin iteration.

Arguments:

    Plugin - Contains device info for VmBus plugins.

Return Value:

    None.
    
--*/
{
    //
    // Use this function to perform a single iteration of your Havoc Plugin's operation.
    // It is called repeatedly and there is no need to set up your own loop around multiple
    // operations here.
    //

    //
    // Returning a non-EFI_SUCCESS error code will halt the Havoc operations and print an error message.
    // 

    return EFI_SUCCESS;
}