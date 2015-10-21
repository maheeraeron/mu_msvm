#pragma once

// TODO: This should be in a VMBUS header.
extern EFI_GUID gEfiVmbusChannelDevicePathGuid;

typedef struct
{
    EFI_VMBUS_PROTOCOL  *VmbusInstance;
    EFI_EMCL_PROTOCOL   *EmclInstance;
    VMBUS_DEVICE_PATH   *DevicePath;
}VMBUS_DEVICE_INFO;

typedef struct _HAVOC_PLUGIN    HAVOC_PLUGIN;

typedef
EFI_STATUS
(EFIAPI *VMBUS_DEVICE_CALLBACK)(
    _In_        VMBUS_DEVICE_INFO  *DeviceInfo,
    _In_opt_    VOID               *Context
    );

typedef
EFI_STATUS
(EFIAPI *HAVOC_PLUGIN_INITIALIZE)(
    );

typedef
EFI_STATUS
(EFIAPI *HAVOC_PLUGIN_RUN)(
    _In_        HAVOC_PLUGIN       *Plugin
    );

typedef struct _HAVOC_PLUGIN
{
    const CHAR16*                   Name;

    // dispatch functions
    HAVOC_PLUGIN_INITIALIZE         Initialize;
    HAVOC_PLUGIN_RUN                Havoc;

    // device info
    const EFI_GUID*                 Guid;
    VMBUS_DEVICE_INFO               DeviceInfo;
}HAVOC_PLUGIN;


HAVOC_PLUGIN*
GetAllPlugins(
    _Out_   UINTN                  *PluginCount
    );

HAVOC_PLUGIN*
PluginFromName(
    _In_z_  const CHAR16           *String
    );

const CHAR16*
VmBusPluginNameFromGuid(
    _In_    const EFI_GUID         *Guid
    );

EFI_STATUS
EFIAPI
EnumerateVmbusDevices(
    _In_    VMBUS_DEVICE_CALLBACK   Callback,
    _In_    VOID                   *Context
    );

EFI_STATUS
EFIAPI
LocateVmBusDevice(
    _In_    HAVOC_PLUGIN           *Plugin
    );
