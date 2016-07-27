#include "UefiHavoc.h"
 

const SHELL_PARAM_ITEM mParamList[] =
{
    { L"-h",        TypeFlag  },
    { L"-?",        TypeFlag  },
    { L"-plugin",   TypeValue },
    { L"-all",      TypeFlag  },
    { NULL,         TypeMax   }
};


EFI_STATUS
ParseArguments(
    UINTN               Argc,
    CHAR16             *Argv[],
    HAVOC_PLUGIN      **Plugins,
    UINTN              *PluginCount 
    )
{
    LIST_ENTRY   *paramPackage;
    CHAR16       *badArg;
    const CHAR16 *string;
    EFI_STATUS    status;

    *Plugins = NULL;
    *PluginCount = 0;
    
    status = ShellCommandLineParseEx(mParamList, &paramPackage, &badArg, TRUE, TRUE);

    if (EFI_ERROR(status))
    {
        Print(L"Error parsing arguments, problem arg is %s\n", badArg);
        goto Exit;
    }

    if (ShellCommandLineGetFlag(paramPackage, L"-all"))
    {
        Print(L"Getting all plugins\n");
        *Plugins = GetAllPlugins(PluginCount);
        Print(L"Found %d plugins\n", *PluginCount);
    }
    else
    {
        string = ShellCommandLineGetValue(paramPackage, L"-plugin");
        if (string != NULL)
        {
            if ((*Plugins = PluginFromName(string)) == NULL)
            {
                Print(L"Unknown device name %s\n", string);
                status = EFI_INVALID_PARAMETER;
                goto Exit;
            }

            *PluginCount = 1;
            //Parameters->Type = (MESSAGE_TYPE)type;
        }
    }

    // TODO: process and print help

Exit:

    ShellCommandLineFreeVarList(paramPackage);

    return status;
}


EFI_STATUS
EFIAPI
ListDevicesCallback(
  _In_      VMBUS_DEVICE_INFO          *DeviceInfo,
  _In_opt_  VOID                       *Context
  )
{
    UNREFERENCED_PARAMETER(Context);

    Print(L"   %10s %g   [%s%s]\n",
        VmBusPluginNameFromGuid(&DeviceInfo->DevicePath->InterfaceType),
        DeviceInfo->DevicePath->InterfaceType,
        ((DeviceInfo->EmclInstance == NULL) ? L"Inactive" : L"Active"),
        (DeviceInfo->VmbusInstance->Flags & EFI_VMBUS_PROTOCOL_FLAGS_PIPE_MODE) ? L" Pipe" : L"");
    return EFI_SUCCESS;
}


/**
  UEFI application entry point which has an interface similar to a
  standard C main function.

  The ShellCEntryLib library instance wrappers the actual UEFI application
  entry point and calls this ShellAppMain function.

  @param[in] Argc     The number of items in Argv.
  @param[in] Argv     Array of pointers to strings.

  @retval  0               The application exited normally.
  @retval  Other           An error occurred.

**/
INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
    HAVOC_PLUGIN   *plugins;
    EFI_STATUS      status;
    UINTN           pluginCount;
    UINTN           pluginIndex;

    plugins = NULL;

    //
    // Enumeration all device if command line says so.
    //
    // Otherwise,
    //    get the device GUID from the command line
    //    find the EMCL info
    //
    //    now we can take the other parameters and 
    //    run the attack task.
    //
    status = ParseArguments(Argc, Argv, &plugins, &pluginCount);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    if (plugins == NULL)
    {
        Print(L"Available VmBus Devices\n");
        EnumerateVmbusDevices(ListDevicesCallback, NULL);
        goto Cleanup;
    }

    for (pluginIndex = 0; pluginIndex < pluginCount; pluginIndex++)
    {
        if (plugins[pluginIndex].Initialize != NULL)
        {
            Print(L"Initializing plugin: [%s]\n", plugins[pluginIndex].Name);
            if (plugins[pluginIndex].Guid != NULL)
            {
                Print(L"Locating VmBus device for plugin: [%s]\n", plugins[pluginIndex].Name);
                status = LocateVmBusDevice(&plugins[pluginIndex]);
                if (EFI_ERROR(status))
                {
                    Print(L"Unable to locate device instance for plugin: [%s] - %r\n", plugins[pluginIndex].Name, status);
                    goto Cleanup;
                }
            }

            status = plugins[pluginIndex].Initialize(plugins[pluginIndex]);
            if (EFI_ERROR(status))
            {
                Print(L"Unable to initialize plugin: [%s]\n", plugins[pluginIndex].Name);
                goto Cleanup;
            }
        }
    }

    Print(L"Executing Havoc for %d plugins\n", pluginCount);
    for(;;)
    {
        for (pluginIndex = 0; pluginIndex < pluginCount; pluginIndex++)
        {
            if (plugins[pluginIndex].Havoc != NULL)
            {
                status = plugins[pluginIndex].Havoc(&plugins[pluginIndex]);
                if (EFI_ERROR(status))
                {
                    Print(L"Failed to execute havoc for plugin: [s]\n", plugins[pluginIndex].Name);
                    goto Cleanup;
                }
            }
        }
    }

Cleanup:

    return EFI_ERROR(status);
}
