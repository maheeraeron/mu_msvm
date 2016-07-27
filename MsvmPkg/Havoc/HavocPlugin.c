#include "UefiHavoc.h"


DEFINE_GUID(VmHwVendorVmbusGuid,    0x9b17e5a2, 0x0891, 0x42dd, 0xb6, 0x53, 0x80, 0xb5, 0xc2, 0x28, 0x09, 0xba);
DEFINE_GUID(VmNetworkDeviceGuid,    0xf8615163, 0xdf3e, 0x46c5, 0x91, 0x3f, 0xf2, 0xd2, 0xf9, 0x65, 0xed, 0x0e);
DEFINE_GUID(VmDiskDeviceGuid,       0xba6163d9, 0x04a1, 0x4d29, 0xb6, 0x05, 0x72, 0xe2, 0xff, 0xb1, 0xdc, 0x7f);
DEFINE_GUID(VmSynthKeyDeviceGuid,   0xf912ad6d, 0x2b17, 0x48ea, 0xbd, 0x65, 0xf9, 0x27, 0xa6, 0x1c, 0x76, 0x84);
DEFINE_GUID(VmSynthVideoDeviceGuid, 0xda0a7802, 0xe377, 0x4aac, 0x8e, 0x77, 0x05, 0x58, 0xeb, 0x10, 0x73, 0xf8);
DEFINE_GUID(VmSynthRdpControlGuid,  0xf8e65716, 0x3cb3, 0x4a06, 0x9a, 0x60, 0x18, 0x89, 0xc5, 0xcc, 0xca, 0xb5);
DEFINE_GUID(VmDynamicMemoryGuid,    0x525074dc, 0x8985, 0x46e2, 0x80, 0x57, 0xa3, 0x7, 0xdc, 0x18, 0xa5, 0x2);
DEFINE_GUID(VmHidDeviceGuid,        0xcfa8b69e, 0x5b4a, 0x4cc0, 0xb9, 0x8b, 0x8b, 0xa1, 0xa1, 0xf3, 0xf9, 0x5a);
DEFINE_GUID(VmActivationDeviceGuid, 0x3375baf4, 0x9e15, 0x4b30, 0xb7, 0x65, 0x67, 0xac, 0xb1, 0x0d, 0x60, 0x7b);
DEFINE_GUID(VmHeartbeatDeviceGuid,  0x57164f39, 0x9115, 0x4e78, 0xab, 0x55, 0x38, 0x2f, 0x3b, 0xd5, 0x42, 0x2d);
DEFINE_GUID(VmKvpDeviceGuid,        0xa9a0f4e7, 0x5a45, 0x4d96, 0xb8, 0x27, 0x8a, 0x84, 0x1e, 0x8c, 0x3, 0xe6);
DEFINE_GUID(VmShutdownDeviceGuid,   0x0e0b6031, 0x5213, 0x4934, 0x81, 0x8b, 0x38, 0xd9, 0xc, 0xed, 0x39, 0xdb);
DEFINE_GUID(VmTimeSyncDeviceGuid,   0x9527e630, 0xd0ae, 0x497b, 0xad, 0xce, 0xe8, 0xa, 0xb0, 0x17, 0x5c, 0xaf);
DEFINE_GUID(VmVssDeviceGuid,        0x35fa2e29, 0xea23, 0x4236, 0x96, 0xae, 0x3a, 0x6e, 0xba, 0xcb, 0xa4, 0x40);
DEFINE_GUID(VmRdvDeviceGuid,        0x276aacf4, 0xac15, 0x426c, 0x98, 0xdd, 0x75, 0x21, 0xad, 0x3f, 0x01, 0xfe);


#define DECLARE_PLUGIN(Name) \
    EFI_STATUS \
    EFIAPI \
    Name##Init(); \
    EFI_STATUS \
    EFIAPI \
    Name##Havoc(_In_ HAVOC_PLUGIN *Plugin)


//
// INSTRUCTIONS FOR CREATING A NEW PLUGIN
// 
// To create a new Plugin:
// 
// 1.)  Add another DECLARE_PLUGIN(<Name>) macro to the ones below.
//      a.) This will handle declaring the two functions you must implement, <Name>Init and <Name>Havoc.
//
// 2.)  Add another entry to the mPluginList Array.
//      a.) Include the Name (will be parsed from command line), as well as the Init and Havoc functions
//          that were predeclared by step 1.
//      b.) For VmBus devices, to handle some of the intialization, include the device GUID after
//          these two methods.
//
// 3.)  Create a new source file that contains <Name>Init and <Name>Havoc.
//      a.) See NewPlugin.c for a sample.
//
// 4.)  Add the new source file to Havoc.inf, along with any additional EDK libraries that are required for your plugin.
//


DECLARE_PLUGIN(Cpuid);
DECLARE_PLUGIN(Video);
DECLARE_PLUGIN(Nic);
DECLARE_PLUGIN(Apic);
DECLARE_PLUGIN(ScsiInquiry);
DECLARE_PLUGIN(NewPlugin);


HAVOC_PLUGIN    mPluginList[] =
{
    {L"vmbus",      NULL,           NULL,           &VmHwVendorVmbusGuid},
    {L"nic",        NULL,           NULL,           &VmNetworkDeviceGuid},
    {L"scsi",       NULL,           NULL,           &VmDiskDeviceGuid},
    {L"keyboard",   NULL,           NULL,           &VmSynthKeyDeviceGuid},
    {L"video",      VideoInit,      VideoHavoc,     &VmSynthVideoDeviceGuid},
    {L"nic",        NicInit,        NicHavoc,       &VmNetworkDeviceGuid},
    {L"rdp",        NULL,           NULL,           &VmSynthRdpControlGuid},
    {L"dynmem",     NULL,           NULL,           &VmDynamicMemoryGuid},
    {L"hid",        NULL,           NULL,           &VmHidDeviceGuid},
    {L"activation", NULL,           NULL,           &VmActivationDeviceGuid},
    {L"heartbeat",  NULL,           NULL,           &VmHeartbeatDeviceGuid},
    {L"kvp",        NULL,           NULL,           &VmKvpDeviceGuid},
    {L"shutdown",   NULL,           NULL,           &VmShutdownDeviceGuid},
    {L"timesync",   NULL,           NULL,           &VmTimeSyncDeviceGuid},
    {L"vss",        NULL,           NULL,           &VmVssDeviceGuid},
    {L"rdv",        NULL,           NULL,           &VmRdvDeviceGuid},
    {L"apic",       ApicInit,       ApicHavoc},
    {L"cpuid",      CpuidInit,      CpuidHavoc},        
    {L"scsiinquiry",ScsiInquiryInit,ScsiInquiryHavoc},
    {L"newplugin",  NewPluginInit,  NewPluginHavoc},   
    {NULL}
};


HAVOC_PLUGIN*
GetAllPlugins(
    UINTN                  *PluginCount
    )
{
    *PluginCount = sizeof(mPluginList)/sizeof(mPluginList[0]);
    return &mPluginList[0];
}


HAVOC_PLUGIN*
PluginFromName(
    const CHAR16           *String
    )
{
    UINT32 i;

    i = 0;
    while (mPluginList[i].Name != NULL)
    {
        if (StrCmp(String, mPluginList[i].Name) == 0)
        {
            return &mPluginList[i];
        }

        i++;
    }

    return NULL;
}


const CHAR16*
VmBusPluginNameFromGuid(
    const EFI_GUID         *Guid
    )
{
    UINT32 i;

    i = 0;
    while (mPluginList[i].Name != NULL)
    {
        if (CompareGuid(Guid, mPluginList[i].Guid) == TRUE)
        {
            return mPluginList[i].Name;
        }

        i++;
    }

    return NULL;
}


EFI_STATUS
EFIAPI
LocateVmbusDeviceCallback(
    _In_      VMBUS_DEVICE_INFO      *DeviceInfo,
    _In_opt_  VOID                   *Context
    )
{
    HAVOC_PLUGIN *plugin = (HAVOC_PLUGIN*)Context;
    EFI_STATUS    status = EFI_SUCCESS;

    if (CompareGuid(plugin->Guid, &DeviceInfo->DevicePath->InterfaceType) == TRUE)
    {
        //TODO need to do something here?
        //status = device->Locate(DeviceInfo, device);

        if (!EFI_ERROR(status))
        {
            //
            // We found the device instance we're looking for.
            // Copy the instance info, then flip the return status to stop enumeration.
            //
            CopyMem(&plugin->DeviceInfo, DeviceInfo, sizeof(plugin->DeviceInfo));
            status = EFI_ABORTED;
        }
    }

    return status;
}


EFI_STATUS
EFIAPI
LocateVmBusDevice(
    _In_    HAVOC_PLUGIN       *Plugin
    )
{
    EFI_STATUS status = EnumerateVmbusDevices(LocateVmbusDeviceCallback, Plugin);

    if (status == EFI_ABORTED)
    {
        status = EFI_SUCCESS;
    }
    else
    {
        status = EFI_NOT_FOUND;
    }

    return status;
}


EFI_STATUS
EFIAPI
EnumerateVmbusDevices(
    VMBUS_DEVICE_CALLBACK   Callback,
    VOID                   *Context
    )
{
    EFI_STATUS  status;

    UINTN       curHandle;
    UINTN       vmbusHandleCount;
    EFI_HANDLE *vmbusHandles = NULL;
    VMBUS_DEVICE_INFO deviceInfo;

    deviceInfo.DevicePath    = NULL;
    deviceInfo.EmclInstance  = NULL;
    deviceInfo.VmbusInstance = NULL;

    // locate all VMBUS instances
    status = gBS->LocateHandleBuffer(ByProtocol,
                  &gEfiVmbusProtocolGuid,
                  NULL,
                  &vmbusHandleCount,
                  &vmbusHandles
                  );

    for (curHandle = 0; curHandle < vmbusHandleCount; curHandle++)
    {
        EFI_DEVICE_PATH_PROTOCOL *devicePathNode = NULL;

        status = gBS->HandleProtocol(vmbusHandles[curHandle],
            &gEfiVmbusProtocolGuid,
            &deviceInfo.VmbusInstance);

        if (EFI_ERROR(status))
        {
            continue;
        }

        //
        // See if this device already has an EMCL channel context
        // (meaning it has a channel opened).
        //

        status = gBS->OpenProtocol(vmbusHandles[curHandle],
                    &gEfiEmclProtocolGuid,
                    &deviceInfo.EmclInstance,
                    gImageHandle,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL);

        //
        // Now what? we need a way to identify each vmbus channel
        //   get the device path, then find the vmbus node?
        //
        //
        devicePathNode = DevicePathFromHandle(vmbusHandles[curHandle]);

        if (devicePathNode == NULL)
        {
            // TODO: This shouldn't happen
            continue;
        }

        deviceInfo.DevicePath = NULL;

        // find the vmbus node so we can get the device type
        while (!IsDevicePathEnd(devicePathNode))
        {
            if ((DevicePathType(devicePathNode) == HARDWARE_DEVICE_PATH) &&
                (DevicePathSubType(devicePathNode) == HW_VENDOR_DP))
            {
                VENDOR_DEVICE_PATH *vendorDevicePath = (VENDOR_DEVICE_PATH*) devicePathNode;

                if (CompareGuid(&vendorDevicePath->Guid, &gEfiVmbusChannelDevicePathGuid))
                {
                    deviceInfo.DevicePath = (VMBUS_DEVICE_PATH*) devicePathNode;
                    break;
                }
            }
            devicePathNode = NextDevicePathNode(devicePathNode);
        }

        //
        // no vmbus device path means this is not a vmbus device
        // which is weird because it does implement VMBUS protocol.
        // TODO: ASSERT or flag an error?
        if (deviceInfo.DevicePath == NULL)
        {
            continue;
        }

        //
        // Dispatch to callback and quit looping if the callback
        // requests it.
        //
        status = Callback(&deviceInfo, Context);

        if (EFI_ERROR(status))
        {
            break;
        }

    }

    if (vmbusHandles != NULL)
    {
        FreePool(vmbusHandles);
    }

    return status;
}
