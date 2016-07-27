/*++

ATTENTION - THIS FILE IS DERIVED FROM THIRD PARTY OPEN SOURCE CODE:
           IntelFrameworkModulePackage\Bus\Isa\IsaSerialDxe\Serial.c
IT IS CLEARED ONLY FOR LIMITED USE BY WINDOWS CORE HYPER-V FOR THE HYPER-V ROLE IN THE
WINDOWS PRODUCT.  DO NOT USE OR SHARE THIS CODE WITHOUT APPROVAL PURSUANT TO THE
MICROSOFT OPEN SOURCE  SOFTWARE APPROVAL POLICY.

Module Name:

    Serial.c

Abstract:

    Provides the implementation for Hyper-V serial ports.

Author:

    Larry Cleeton (lcleeton) - 08-Aug-2014

--*/

#include "Serial.h"


//
// The instance of the Driver Binding Protocol for the image handle.
//
EFI_DRIVER_BINDING_PROTOCOL gSerialDriver =
{
    SerialDriverSupported,
    SerialDriverStart,
    SerialDriverStop,
    0xa,
    NULL,
    NULL
};


//
// Starting template for Serial Device objects.
//
SERIAL_DEVICE gSerialDeviceTempate =
{
    SERIAL_DEVICE_SIGNATURE,
    NULL,
    FALSE,
    { // SerialIo
        SERIAL_IO_INTERFACE_REVISION,
        SerialReset,
        SerialSetAttributes,
        SerialSetControl,
        SerialGetControl,
        SerialWrite,
        SerialRead,
        NULL
    },
    { // SerialMode
        SERIAL_PORT_SUPPORT_CONTROL_MASK,
        SERIAL_PORT_DEFAULT_TIMEOUT,
        FixedPcdGet64(PcdUartDefaultBaudRate),     // BaudRate
        SERIAL_PORT_DEFAULT_RECEIVE_FIFO_DEPTH,
        FixedPcdGet8(PcdUartDefaultDataBits),      // DataBits
        FixedPcdGet8(PcdUartDefaultParity),        // Parity
        FixedPcdGet8(PcdUartDefaultStopBits)       // StopBits
    },
    NULL,
    { // UartDevicePath
        {
            MESSAGING_DEVICE_PATH,
            MSG_UART_DP,
            {
                (UINT8)(sizeof(UART_DEVICE_PATH)),
                (UINT8)((sizeof(UART_DEVICE_PATH)) >> 8)
            }
        },
        0,
        FixedPcdGet64(PcdUartDefaultBaudRate),
        FixedPcdGet8(PcdUartDefaultDataBits),
        FixedPcdGet8(PcdUartDefaultParity),
        FixedPcdGet8(PcdUartDefaultStopBits)
    },
    0,    //BaseAddress
    {
        0,
        0,
        SERIAL_MAX_BUFFER_SIZE,
        { 0 }
    },
    {
        0,
        0,
        SERIAL_MAX_BUFFER_SIZE,
        { 0 }
    },
    FALSE,
    FALSE,
    Uart16550A,
    NULL
};


//
// Starting templates for the Serial Port protocol instances.
//
SERIAL_DEVICE_PROPERTIES gSerialProperties[] =
{
    // COM1
    {
        0x3F8,              // BaseAddress
        EISA_PNP_ID(0x501), // HID
        1                   // UID
    },
    // COM2
    {
        0x2F8,              // BaseAddress
        EISA_PNP_ID(0x501), // HID
        2                   // UID
    }
};


//
// The handle of the dummy root device.
//
EFI_HANDLE gRootDevice = NULL;


//
// Configuration state.
//
BOOLEAN gSerialEnabled = FALSE;
BOOLEAN gDebuggerEnabled = FALSE;
UINT32  gConsoleMode = ConfigLibConsoleModeDefault;


BOOLEAN
IsUartFlowControlNode(
    _In_ UART_FLOW_CONTROL_DEVICE_PATH *FlowControl
    )
/*++

Routine Description:

    Check if a device path node is a Flow Control node.

Arguments:

    FlowControl             A pointer to a device path node.

Return Value:

    TRUE                    The node is the flow control node.

    FALSE                   Otherwise.

--*/
{
    return (BOOLEAN)((DevicePathType(FlowControl) == MESSAGING_DEVICE_PATH) &&
                     (DevicePathSubType(FlowControl) == MSG_VENDOR_DP) &&
                     (CompareGuid(&FlowControl->Guid, &gEfiUartDevicePathGuid)));
}


BOOLEAN
ContainsFlowControl(
    _In_ EFI_DEVICE_PATH_PROTOCOL      *DevicePath
    )
/*++

Routine Description:

    Check if a device path contains a Flow Control node.

Arguments:

    DevicePath              A pointer to a device path.

Return Value:

    TRUE                    The path contains a flow control node.

    FALSE                   Otherwise.

--*/
{
    while (!IsDevicePathEnd(DevicePath))
    {
        if (IsUartFlowControlNode((UART_FLOW_CONTROL_DEVICE_PATH *) DevicePath))
        {
            return TRUE;
        }
        DevicePath = NextDevicePathNode(DevicePath);
    }
    return FALSE;
}


VOID
SerialDestroyChildDevice(
    _In_     EFI_DRIVER_BINDING_PROTOCOL *This,
    _In_     EFI_HANDLE                  ParentController,
    _In_opt_ SERIAL_DEVICE               *SerialDevice
    )
/*++

Routine Description:

    Destroys a SERIAL_DEVICE object. The SERIAL_DEVICE doesn't
    need to be fully constructed so this can be used for error cleanup.

Arguments:

    This - Pointer to the driver binding protocol.

    ParentController - Device handle of parent device.

    SerialDevice - Pointer to the SERIAL_DEVICE object to destroy.
                   If null this routine is a no-op.

Return Value:

    none

--*/
{
    EFI_STATUS status;

    if (SerialDevice == NULL)
    {
        //
        // Nothing to do.
        //
        return;
    }

    if (SerialDevice->Handle != NULL)
    {
        //
        // Close the protocol opened BY_CHILD_CONTROLLER.
        //
        if (SerialDevice->BusProtocolOpened)
        {
            status = gBS->CloseProtocol(ParentController,
                                        &gMsvmSerialBusProtocolGuid,
                                        This->DriverBindingHandle,
                                        SerialDevice->Handle);
            DEBUG((DEBUG_INFO, "SerialDriverStop(child): CloseProtocol %r\n", status));
        }
        //
        // Remove the protocols from the child handle. This should delete the handle.
        //
        if (SerialDevice->Handle != NULL)
        {
            status = gBS->UninstallMultipleProtocolInterfaces(SerialDevice->Handle,
                                                              &gEfiDevicePathProtocolGuid,
                                                              SerialDevice->DevicePath,
                                                              &gEfiSerialIoProtocolGuid,
                                                              &SerialDevice->SerialIo,
                                                              NULL);
            DEBUG((DEBUG_INFO, "SerialDriverStop(child): UninstallMPIs %r\n", status));
        }
    }

    if (SerialDevice->DevicePath != NULL)
    {
        gBS->FreePool(SerialDevice->DevicePath);
    }
    FreeUnicodeStringTable(SerialDevice->ControllerNameTable);
    gBS->FreePool(SerialDevice);

}


EFI_STATUS
SerialCreateChildDevice(
    _In_ EFI_DRIVER_BINDING_PROTOCOL    *This,
    _In_ EFI_HANDLE                     ParentController,
    _In_ SERIAL_DEVICE_PROPERTIES       *Properties
    )
/*++

Routine Description:

    Creates a SerialPortDevice object.

Arguments:

    This - Pointer to the driver binding protocol.

    ParentController - Device handle of parent device.

    Properties - The properties of this serial port.

Return Value:

    EFI_STATUS

--*/
{
    EFI_STATUS                  status;
    SERIAL_DEVICE               *serialDevice = NULL;
    EFI_DEVICE_PATH_PROTOCOL    *tempDevicePath = NULL;
    EFI_DEV_PATH                node;
    VOID                        *protocol;

    //
    // Initialize a child serial device instance
    //
    serialDevice = AllocateCopyPool(sizeof(SERIAL_DEVICE), &gSerialDeviceTempate);
    if (serialDevice == NULL)
    {
        status = EFI_OUT_OF_RESOURCES;
        goto Cleanup;
    }

    serialDevice->SerialIo.Mode       = &(serialDevice->SerialMode);
    serialDevice->BaseAddress         = Properties->BaseAddress;
    serialDevice->HardwareFlowControl = FALSE;

    //
    // Construct the child name.
    //
    AddName(serialDevice, Properties);

    //
    // Probe for the actual hardware.
    //
    if (!SerialPortPresent(serialDevice))
    {
        status = EFI_DEVICE_ERROR;
        goto Cleanup;
    }

    //
    // Build a device path and add it to the device structure.
    //
    node.DevPath.Type = ACPI_DEVICE_PATH;
    node.DevPath.SubType = ACPI_DP;
    SetDevicePathNodeLength(&node.DevPath, sizeof(ACPI_HID_DEVICE_PATH));
    node.Acpi.HID = Properties->HID;
    node.Acpi.UID = Properties->UID;

    tempDevicePath = AppendDevicePathNode(NULL, &node.DevPath);
    if (tempDevicePath == NULL)
    {
        status = EFI_OUT_OF_RESOURCES;
        goto Cleanup;
    }
    serialDevice->DevicePath = AppendDevicePathNode(tempDevicePath,
                                    (EFI_DEVICE_PATH_PROTOCOL *)&serialDevice->UartDevicePath);
    if (serialDevice->DevicePath == NULL)
    {
        status = EFI_OUT_OF_RESOURCES;
        goto Cleanup;
    }

    //
    // Fill in Serial I/O Mode structure based on defaults.
    //
    serialDevice->SerialMode.BaudRate = serialDevice->UartDevicePath.BaudRate;
    serialDevice->SerialMode.DataBits = serialDevice->UartDevicePath.DataBits;
    serialDevice->SerialMode.Parity   = serialDevice->UartDevicePath.Parity;
    serialDevice->SerialMode.StopBits = serialDevice->UartDevicePath.StopBits;

    //
    // Issue a reset to initialize the COM port
    //
    status = serialDevice->SerialIo.Reset(&serialDevice->SerialIo);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    //
    // Create child handle and install protocol interfaces for the serial device.
    //
    status = gBS->InstallMultipleProtocolInterfaces(&serialDevice->Handle,
                                                    &gEfiDevicePathProtocolGuid,
                                                    serialDevice->DevicePath,
                                                    &gEfiSerialIoProtocolGuid,
                                                    &serialDevice->SerialIo,
                                                    NULL);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    //
    // Open the bus protocol BY_CHILD_CONTROLLER so the relationship
    // to the parent handle is tracked.
    //
    status = gBS->OpenProtocol(ParentController,
                               &gMsvmSerialBusProtocolGuid,
                               &protocol, // returns null, state tracked in device
                               This->DriverBindingHandle,
                               serialDevice->Handle,
                               EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER);

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }
    serialDevice->BusProtocolOpened = TRUE;

Cleanup:

    if (tempDevicePath != NULL)
    {
        gBS->FreePool(tempDevicePath);
    }

    if (EFI_ERROR(status))
    {
        SerialDestroyChildDevice(This, ParentController, serialDevice);
    }

    return status;
}


EFI_STATUS
EFIAPI
SerialEntryPoint(
    _In_ EFI_HANDLE         ImageHandle,
    _In_ EFI_SYSTEM_TABLE   *SystemTable
    )
/*++

Routine Description:

    Entry point into this driver.

Arguments:

    ImageHandle - Handle of the driver image.

    SystemTable - EFI system table.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;
    EFI_DEVICE_PATH_PROTOCOL *devicePath;

    //
    // Install the driver model protocol(s) on the image handle.
    //
    status = EfiLibInstallDriverBindingComponentName2(ImageHandle,
                                                      SystemTable,
                                                      &gSerialDriver,
                                                      ImageHandle,
                                                      &gSerialComponentName,
                                                      &gSerialComponentName2);
    if (EFI_ERROR(status))
    {
        return status;
    }

    //
    // Get the serial port and UEFI debugger configuration.
    //
    gSerialEnabled = GetSerialControllersEnabled();
    gDebuggerEnabled = GetDebuggerEnabled();
    gConsoleMode = GetConsoleMode();

    //
    // Do nothing and return success if the serial ports are not configured.
    //
    if (!gSerialEnabled)
    {
        return EFI_SUCCESS;
    }

    //
    // Create a root handle with the device path protocol and a tag protocol
    //
    devicePath = AppendDevicePathNode(NULL, NULL); // empty device path
    if (devicePath == NULL)
    {
        return EFI_OUT_OF_RESOURCES;
    }
    status = gBS->InstallMultipleProtocolInterfaces(&gRootDevice,
                                                    &gEfiDevicePathProtocolGuid,
                                                    devicePath,
                                                    &gMsvmSerialBusProtocolGuid,
                                                    NULL,
                                                    NULL);

    return status;
}


EFI_STATUS
EFIAPI
SerialDriverSupported(
    _In_ EFI_DRIVER_BINDING_PROTOCOL    *This,
    _In_ EFI_HANDLE                     ControllerHandle,
    _In_ EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
    )
/*++

Routine Description:

    Check to see if this driver supports the given controller

Arguments:

    This                    A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.

    ControllerHandle        The handle of the controller to test.

    RemainingDevicePath     A pointer to the remaining portion of a device path.


Return Value:

    EFI_SUCCESS             This driver can support the given controller

    EFI_ALREADY_STARTED     A driver already is managing this controller.

    EFI_UNSUPPORTED         This driver cannot support the given controller.

--*/
{
    EFI_STATUS                status;
    EFI_DEVICE_PATH_PROTOCOL  *parentDevicePath = NULL;
    VOID                      *protocol;

    //
    // Briefly open (BY_DRIVER) the private serial bus protocol as a
    // simple way to determine if the ControllerHandle is our device and
    // that it is already started.
    //
    status = gBS->OpenProtocol(ControllerHandle,
                               &gMsvmSerialBusProtocolGuid,
                               &protocol, // required but returns NULL
                               This->DriverBindingHandle,
                               ControllerHandle,
                               EFI_OPEN_PROTOCOL_BY_DRIVER);
    if (EFI_ERROR(status))
    {
        return status;
    }
    gBS->CloseProtocol(ControllerHandle,
                       &gMsvmSerialBusProtocolGuid,
                       This->DriverBindingHandle,
                       ControllerHandle);

    //
    // Test if the Device Path protocol is available. It is required.
    //
    status = gBS->OpenProtocol(ControllerHandle,
                               &gEfiDevicePathProtocolGuid,
                               (VOID **)&parentDevicePath,
                               This->DriverBindingHandle,
                               ControllerHandle,
                               EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
    if (EFI_ERROR(status))
    {
        return status;
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
SerialDriverStart(
    _In_ EFI_DRIVER_BINDING_PROTOCOL    *This,
    _In_ EFI_HANDLE                     Controller,
    _In_ EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
    )
/*++

Routine Description:

    Start managing a controller.

Arguments:

    This                    A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.

    Controller              The handle of the controller to manage.

    RemainingDevicePath     A pointer to the remaining portion of a device path.

Return Value:

    EFI_SUCCESS             Driver started successfully.

    EFI_ALREADY_STARTED     A driver already is managing this controller.

--*/
{
    EFI_STATUS                  status;
    VOID                        *protocol;
    UINT32                      index;

    //
    // Open the bus tag protocol to indicate the driver is now managing
    // the root device handle.
    //
    status = gBS->OpenProtocol(Controller,
                               &gMsvmSerialBusProtocolGuid,
                               (VOID **) &protocol,
                               This->DriverBindingHandle,
                               Controller,
                               EFI_OPEN_PROTOCOL_BY_DRIVER
                               );
    if (EFI_ERROR(status))
    {
        goto Exit;
    }

    //
    // Create the child handles.
    //
    for (index = 0; index < ARRAY_SIZE(gSerialProperties); index++)
    {
        //
        // Don't create the first child handle (COM1) if the UEFI debugger is enabled
        // or the port is not configured as the console.
        //
        if ((index == 0) && (gDebuggerEnabled || (gConsoleMode != ConfigLibConsoleModeCOM1)))
        {
            continue;
        }
        //
        // Don't create the second child handle (COM2) if the port is not
        // configured as the console.
        //
        if ((index == 1) && (gConsoleMode != ConfigLibConsoleModeCOM2))
        {
            continue;
        }
        SerialCreateChildDevice(This, Controller, &gSerialProperties[index]);
    }

Exit:

    return status;
}


EFI_STATUS
EFIAPI
SerialDriverStop(
    _In_  EFI_DRIVER_BINDING_PROTOCOL   *This,
    _In_  EFI_HANDLE                    Controller,
    _In_  UINTN                         NumberOfChildren,
    _In_  EFI_HANDLE                    *ChildHandleBuffer
    )
/*++

Routine Description:

    Disconnect this driver from a controller and uninstall related protocol instances.

Arguments:

    This                    A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.

    Controller              The handle of the controller to stop.

    NumberOfChildren        The Number of child devices. Can be zero to indicate
                            stop managing the Controller.

    ChildHandleBuffer       An array of child device handles. Null if NumberOfChildren
                            is zero.

Return Value:

    EFI_SUCCESS             Driver stopped successfully.

--*/
{
    EFI_STATUS              status;
    UINTN                   index;
    BOOLEAN                 allChildrenStopped;
    EFI_SERIAL_IO_PROTOCOL  *serialIo;
    SERIAL_DEVICE           *serialDevice;

    DEBUG((DEBUG_VERBOSE, "SerialDriverStop: ControllerHandle %x\n", Controller));
    DEBUG((DEBUG_VERBOSE, "                  NumberOfChildren %x\n", NumberOfChildren));
    for (index = 0; index < NumberOfChildren; index++)
    {
    DEBUG((DEBUG_VERBOSE, "                  ChildHandle      %x\n", ChildHandleBuffer[index]));
    }

    //
    // Check if stopping child device handles or the main controller handle.
    //
    if (NumberOfChildren == 0)
    {
        //
        // Close the tag protocol on the controller handle.
        //
        status = gBS->CloseProtocol(Controller,
                                    &gMsvmSerialBusProtocolGuid,
                                    This->DriverBindingHandle,
                                    Controller);
        return status;
    }

    allChildrenStopped = TRUE;

    for (index = 0; index < NumberOfChildren; index++)
    {
        //
        // Get a pointer to the Serial IO protocol in order to offset to the device structure.
        //
        status = gBS->OpenProtocol(ChildHandleBuffer[index],
                                   &gEfiSerialIoProtocolGuid,
                                   (VOID **) &serialIo,
                                   This->DriverBindingHandle,
                                   Controller,
                                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
                                   );

        if (!EFI_ERROR(status))
        {
            //
            // Destroy the child device.
            //
            serialDevice = SERIAL_DEVICE_FROM_THIS(serialIo);
            SerialDestroyChildDevice(This, Controller, serialDevice);
        }
        else
        {
            allChildrenStopped = FALSE;
        }
    }

    if (!allChildrenStopped)
    {
        return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;
}


BOOLEAN
SerialFifoFull(
    _In_ SERIAL_DEV_FIFO *Fifo
    )
/*++

Routine Description:

    Detect whether a specific FIFO is full or not.

Arguments:

    Fifo    A pointer to the Data Structure SERIAL_DEV_FIFO

Return Value:

    Whether specific FIFO is full or not

--*/
{
    if (Fifo->Surplus == 0)
    {
        return TRUE;
    }
    return FALSE;
}


BOOLEAN
SerialFifoEmpty(
    _In_ SERIAL_DEV_FIFO *Fifo
    )
/*++

Routine Description:

    Detect whether a specific FIFO is empty or not.

Arguments:

    Fifo    A pointer to the Data Structure SERIAL_DEV_FIFO

Return Value:

    Whether specific FIFO is empty or not

--*/
{
    if (Fifo->Surplus == SERIAL_MAX_BUFFER_SIZE)
    {
        return TRUE;
    }
    return FALSE;
}


EFI_STATUS
SerialFifoAdd(
    _In_ SERIAL_DEV_FIFO *Fifo,
    _In_ UINT8           Data
    )
/*++

Routine Description:

    Add data to a specific FIFO.

Arguments:

    Fifo    A pointer to the Data Structure SERIAL_DEV_FIFO

    Data    the data tp be added to FIFO

Return Value:

    EFI_SUCCESS           Added data to the FIFO successfully

    EFI_OUT_OF_RESOURCE   Failed to add data because FIFO is already full

--*/
{
    //
    // if FIFO full can not add data
    //
    if (SerialFifoFull(Fifo))
    {
        return EFI_OUT_OF_RESOURCES;
    }
    //
    // FIFO is not full can add data
    //
    Fifo->Data[Fifo->Last] = Data;
    Fifo->Surplus--;
    Fifo->Last++;
    if (Fifo->Last == SERIAL_MAX_BUFFER_SIZE)
    {
        Fifo->Last = 0;
    }

    return EFI_SUCCESS;
}


EFI_STATUS
SerialFifoRemove(
    _In_  SERIAL_DEV_FIFO *Fifo,
    _Out_ UINT8           *Data
    )
/*++

Routine Description:

    Remove data from a specific FIFO.

Arguments:

    Fifo    A pointer to the Data Structure SERIAL_DEV_FIFO

    Data    the data removed from the FIFO

Return Value:

    EFI_SUCCESS           Removed data from the FIFO successfully

    EFI_OUT_OF_RESOURCE   Failed to remove data because the FIFO is empty

--*/
{
    //
    // if FIFO is empty, no data to remove
    //
    if (SerialFifoEmpty(Fifo))
    {
        return EFI_OUT_OF_RESOURCES;
    }
    //
    // FIFO is not empty, can remove data
    //
    *Data = Fifo->Data[Fifo->First];
    Fifo->Surplus++;
    Fifo->First++;
    if (Fifo->First == SERIAL_MAX_BUFFER_SIZE)
    {
        Fifo->First = 0;
    }

    return EFI_SUCCESS;
}


EFI_STATUS
SerialReceiveTransmit(
      _In_ SERIAL_DEVICE *SerialDevice
      )
/*++

Routine Description:

    Reads and writes all available data.

Arguments:

    SerialDevice        The device to flush

    Data                The data removed from the FIFO

Return Value:

    EFI_SUCCESS           Data was read/written successfully.

    EFI_OUT_OF_RESOURCE   Failed because software receive FIFO is full.  Note, when
                          this happens, pending writes are not done.

--*/
{
    SERIAL_PORT_LSR lsr;
    UINT8           data;
    BOOLEAN         receiveFifoFull;
    SERIAL_PORT_MSR msr;
    SERIAL_PORT_MCR mcr;
    UINTN           timeOut;

    data = 0;

    //
    // Begin the read or write
    //
    if (SerialDevice->SoftwareLoopbackEnable)
    {
        do
        {
            receiveFifoFull = SerialFifoFull(&SerialDevice->Receive);
            if (!SerialFifoEmpty(&SerialDevice->Transmit))
            {
                SerialFifoRemove(&SerialDevice->Transmit, &data);
                if (receiveFifoFull)
                {
                    return EFI_OUT_OF_RESOURCES;
                }
                SerialFifoAdd(&SerialDevice->Receive, data);
            }
        } while (!SerialFifoEmpty(&SerialDevice->Transmit));
    }
    else
    {
        receiveFifoFull = SerialFifoFull(&SerialDevice->Receive);
        //
        // For full handshake flow control, tell the peer to send data
        // if receive buffer is available.
        //
        if (SerialDevice->HardwareFlowControl &&
            !FeaturePcdGet(PcdIsaBusSerialUseHalfHandshake)&&
            !receiveFifoFull
            )
        {
            mcr.Data     = READ_MCR(SerialDevice->BaseAddress);
            mcr.Bits.Rts = 1;
            WRITE_MCR(SerialDevice->BaseAddress, mcr.Data);
        }
        do
        {
            lsr.Data = READ_LSR(SerialDevice->BaseAddress);

            //
            // Flush incomming data to prevent a an overrun during a long write
            //
            if ((lsr.Bits.Dr == 1) && !receiveFifoFull)
            {
                receiveFifoFull = SerialFifoFull(&SerialDevice->Receive);
                if (!receiveFifoFull)
                {
                    if (lsr.Bits.FIFOe == 1 || lsr.Bits.Oe == 1 || lsr.Bits.Pe == 1 ||
                        lsr.Bits.Fe == 1 || lsr.Bits.Bi == 1)
                    {
                        REPORT_STATUS_CODE_WITH_DEVICE_PATH(
                            EFI_ERROR_CODE,
                            EFI_P_EC_INPUT_ERROR | EFI_PERIPHERAL_SERIAL_PORT,
                            SerialDevice->DevicePath);
                        if (lsr.Bits.FIFOe == 1 || lsr.Bits.Pe == 1|| lsr.Bits.Fe == 1 ||
                            lsr.Bits.Bi == 1)
                        {
                            data = READ_RBR(SerialDevice->BaseAddress);
                            continue;
                        }
                    }

                    data = READ_RBR(SerialDevice->BaseAddress);

                    SerialFifoAdd(&SerialDevice->Receive, data);

                    //
                    // For full handshake flow control, if receive buffer full
                    // tell the peer to stop sending data.
                    //
                    if (SerialDevice->HardwareFlowControl &&
                        !FeaturePcdGet(PcdIsaBusSerialUseHalfHandshake)   &&
                        SerialFifoFull(&SerialDevice->Receive)
                        )
                    {
                        mcr.Data = READ_MCR(SerialDevice->BaseAddress);
                        mcr.Bits.Rts = 0;
                        WRITE_MCR(SerialDevice->BaseAddress, mcr.Data);
                    }

                    continue;
                }
                else
                {
                    REPORT_STATUS_CODE_WITH_DEVICE_PATH(
                        EFI_PROGRESS_CODE,
                        EFI_P_SERIAL_PORT_PC_CLEAR_BUFFER | EFI_PERIPHERAL_SERIAL_PORT,
                        SerialDevice->DevicePath);
                }
            }
            //
            // Do the write
            //
            if (lsr.Bits.Thre == 1 && !SerialFifoEmpty(&SerialDevice->Transmit))
            {
                //
                // Make sure the transmit data will not be missed
                //
                if (SerialDevice->HardwareFlowControl)
                {
                    //
                    // For half handshake flow control assert RTS before sending.
                    //
                    if (FeaturePcdGet(PcdIsaBusSerialUseHalfHandshake))
                    {
                        mcr.Data     = READ_MCR(SerialDevice->BaseAddress);
                        mcr.Bits.Rts= 0;
                        WRITE_MCR(SerialDevice->BaseAddress, mcr.Data);
                    }
                    //
                    // Wait for CTS
                    //
                    timeOut   = 0;
                    msr.Data  = READ_MSR(SerialDevice->BaseAddress);
                    while ((msr.Bits.Dcd == 1) &&
                          ((msr.Bits.Cts == 0) ^ FeaturePcdGet(PcdIsaBusSerialUseHalfHandshake)))
                    {
                        gBS->Stall(TIMEOUT_STALL_INTERVAL);
                        timeOut++;
                        if (timeOut > 5)
                        {
                          break;
                        }

                        msr.Data = READ_MSR(SerialDevice->BaseAddress);
                    }

                    if ((msr.Bits.Dcd == 0) ||
                    ((msr.Bits.Cts == 1) ^ FeaturePcdGet(PcdIsaBusSerialUseHalfHandshake)))
                    {
                        SerialFifoRemove(&SerialDevice->Transmit, &data);
                        WRITE_THR(SerialDevice->BaseAddress, data);
                    }

                    //
                    // For half handshake flow control, tell DCE we are done.
                    //
                    if (FeaturePcdGet(PcdIsaBusSerialUseHalfHandshake))
                    {
                        mcr.Data = READ_MCR(SerialDevice->BaseAddress);
                        mcr.Bits.Rts = 1;
                        WRITE_MCR(SerialDevice->BaseAddress, mcr.Data);
                    }
                }
                else
                {
                    SerialFifoRemove(&SerialDevice->Transmit, &data);
                    WRITE_THR(SerialDevice->BaseAddress, data);
                }
            }
        } while (lsr.Bits.Thre == 1 && !SerialFifoEmpty(&SerialDevice->Transmit));
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
SerialReset(
      _In_ EFI_SERIAL_IO_PROTOCOL  *This
      )
/*++

Routine Description:

    Resets a serial device.

Arguments:

    This                A pointer to an EFI_SERIAL_IO_PROTOCOL

Return Value:

    EFI_SUCCESS         Reset successfully.

    EFI_DEVICE_ERROR    Failed to reset.

--*/
{
    EFI_STATUS      status;
    SERIAL_DEVICE   *serialDevice;
    SERIAL_PORT_LCR lcr;
    SERIAL_PORT_IER ier;
    SERIAL_PORT_MCR mcr;
    SERIAL_PORT_FCR fcr;
    EFI_TPL         tpl;
    UINT32          control;

    serialDevice = SERIAL_DEVICE_FROM_THIS(This);

    //
    // Report the status code reset the serial
    //
    REPORT_STATUS_CODE_WITH_DEVICE_PATH(
        EFI_PROGRESS_CODE,
        EFI_P_PC_RESET | EFI_PERIPHERAL_SERIAL_PORT,
        serialDevice->DevicePath);

    tpl = gBS->RaiseTPL(TPL_NOTIFY);

    //
    // Make sure DLAB is 0.
    //
    lcr.Data = READ_LCR(serialDevice->BaseAddress);
    lcr.Bits.DLab = 0;
    WRITE_LCR(serialDevice->BaseAddress, lcr.Data);

    //
    // Turn off all interrupts
    //
    ier.Data        = READ_IER(serialDevice->BaseAddress);
    ier.Bits.Ravie  = 0;
    ier.Bits.Theie  = 0;
    ier.Bits.Rie    = 0;
    ier.Bits.Mie    = 0;
    WRITE_IER(serialDevice->BaseAddress, ier.Data);

    //
    // Disable the FIFO.
    //
    fcr.Bits.TrFIFOE = 0;
    WRITE_FCR(serialDevice->BaseAddress, fcr.Data);

    //
    // Turn off loopback and disable device interrupt.
    //
    mcr.Data      = READ_MCR(serialDevice->BaseAddress);
    mcr.Bits.Out1 = 0;
    mcr.Bits.Out2 = 0;
    mcr.Bits.Lme  = 0;
    WRITE_MCR(serialDevice->BaseAddress, mcr.Data);

    //
    // Clear the scratch pad register
    //
    WRITE_SCR(serialDevice->BaseAddress, 0);

    //
    // Go set the current attributes
    //
    status = This->SetAttributes(This,
                                 This->Mode->BaudRate,
                                 This->Mode->ReceiveFifoDepth,
                                 This->Mode->Timeout,
                                 (EFI_PARITY_TYPE) This->Mode->Parity,
                                 (UINT8) This->Mode->DataBits,
                                 (EFI_STOP_BITS_TYPE) This->Mode->StopBits);

    if (EFI_ERROR(status))
    {
        gBS->RestoreTPL(tpl);
        return EFI_DEVICE_ERROR;
    }
    //
    // Go set the current control bits
    //
    control = 0;
    if (serialDevice->HardwareFlowControl)
    {
        control |= EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE;
    }
    if (serialDevice->SoftwareLoopbackEnable)
    {
        control |= EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE;
    }
    status = This->SetControl(This, control);

    if (EFI_ERROR(status))
    {
        gBS->RestoreTPL(tpl);
        return EFI_DEVICE_ERROR;
    }
    //
    // for 16550A enable FIFO, 16550 disable FIFO
    //
    fcr.Bits.TrFIFOE  = 1;
    fcr.Bits.ResetRF  = 1;
    fcr.Bits.ResetTF  = 1;
    WRITE_FCR(serialDevice->BaseAddress, fcr.Data);

    //
    // Reset the software FIFO
    //
    serialDevice->Receive.First     = 0;
    serialDevice->Receive.Last      = 0;
    serialDevice->Receive.Surplus   = SERIAL_MAX_BUFFER_SIZE;
    serialDevice->Transmit.First    = 0;
    serialDevice->Transmit.Last     = 0;
    serialDevice->Transmit.Surplus  = SERIAL_MAX_BUFFER_SIZE;

    gBS->RestoreTPL(tpl);

    //
    // Device reset is complete
    //
    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
SerialSetAttributes(
    _In_ EFI_SERIAL_IO_PROTOCOL  *This,
    _In_ UINT64                  BaudRate,
    _In_ UINT32                  ReceiveFifoDepth,
    _In_ UINT32                  Timeout,
    _In_ EFI_PARITY_TYPE         Parity,
    _In_ UINT8                   DataBits,
    _In_ EFI_STOP_BITS_TYPE      StopBits
    )
/*++

Routine Description:

      Set new attributes to a serial device.

Arguments:

    This                    Pointer to EFI_SERIAL_IO_PROTOCOL

    BaudRate                The baudrate of the serial device

    ReceiveFifoDepth        The depth of receive FIFO buffer

    Timeout                 The request timeout for a single char

    Parity                  The type of parity used in serial device

    DataBits                Number of databits used in serial device

    StopBits                Number of stopbits used in serial device


Return Value:

    EFI_SUCCESS             The new attributes were set

    EFI_INVALID_PARAMETERS  One or more attributes have an unsupported value

    EFI_UNSUPPORTED         Data Bits can not set to 5 or 6

    EFI_DEVICE_ERROR        The serial device is not functioning correctly (no return)

--*/
{
    EFI_STATUS                status;
    SERIAL_DEVICE             *serialDevice;
    UINT32                    divisor;
    UINT32                    remained;
    SERIAL_PORT_LCR           lcr;
    UART_DEVICE_PATH          *uart;
    EFI_TPL                   tpl;

    serialDevice = SERIAL_DEVICE_FROM_THIS(This);

    //
    // Check for default settings and fill in actual values.
    //
    if (BaudRate == 0)
    {
        BaudRate = PcdGet64(PcdUartDefaultBaudRate);
    }

    if (ReceiveFifoDepth == 0)
    {
        ReceiveFifoDepth = SERIAL_PORT_DEFAULT_RECEIVE_FIFO_DEPTH;
    }

    if (Timeout == 0)
    {
        Timeout = SERIAL_PORT_DEFAULT_TIMEOUT;
    }

    if (Parity == DefaultParity)
    {
        Parity = (EFI_PARITY_TYPE)PcdGet8(PcdUartDefaultParity);
    }

    if (DataBits == 0)
    {
        DataBits = PcdGet8(PcdUartDefaultDataBits);
    }

    if (StopBits == DefaultStopBits)
    {
        StopBits = (EFI_STOP_BITS_TYPE) PcdGet8(PcdUartDefaultStopBits);
    }
    //
    // 5 and 6 data bits can not be verified on a 16550A UART
    // Return EFI_INVALID_PARAMETER if an attempt is made to use these settings.
    //
    if ((DataBits == 5) || (DataBits == 6))
    {
        return EFI_INVALID_PARAMETER;
    }
    //
    // Make sure all parameters are valid
    //
    if ((BaudRate > SERIAL_PORT_MAX_BAUD_RATE) || (BaudRate < SERIAL_PORT_MIN_BAUD_RATE))
    {
        return EFI_INVALID_PARAMETER;
    }
    //
    // 50,75,110,134,150,300,600,1200,1800,2000,2400,3600,4800,7200,9600,19200,
    // 38400,57600,115200
    //
    if (BaudRate < 75)
    {
        BaudRate = 50;
    }
    else if (BaudRate < 110)
    {
        BaudRate = 75;
    }
    else if (BaudRate < 134)
    {
        BaudRate = 110;
    }
    else if (BaudRate < 150)
    {
        BaudRate = 134;
    }
    else if (BaudRate < 300)
    {
        BaudRate = 150;
    }
    else if (BaudRate < 600)
    {
        BaudRate = 300;
    }
    else if (BaudRate < 1200)
    {
        BaudRate = 600;
    }
    else if (BaudRate < 1800)
    {
        BaudRate = 1200;
    }
    else if (BaudRate < 2000)
    {
        BaudRate = 1800;
    }
    else if (BaudRate < 2400)
    {
        BaudRate = 2000;
    }
    else if (BaudRate < 3600)
    {
        BaudRate = 2400;
    } else if (BaudRate < 4800)
    {
        BaudRate = 3600;
    }
    else if (BaudRate < 7200)
    {
        BaudRate = 4800;
    }
    else if (BaudRate < 9600)
    {
        BaudRate = 7200;
    }
    else if (BaudRate < 19200)
    {
        BaudRate = 9600;
    }
    else if (BaudRate < 38400)
    {
        BaudRate = 19200;
    }
    else if (BaudRate < 57600)
    {
        BaudRate = 38400;
    }
    else if (BaudRate < 115200)
    {
        BaudRate = 57600;
    }
    else if (BaudRate <= SERIAL_PORT_MAX_BAUD_RATE)
    {
        BaudRate = 115200;
    }

    if ((ReceiveFifoDepth < 1) || (ReceiveFifoDepth > SERIAL_PORT_MAX_RECEIVE_FIFO_DEPTH))
    {
        return EFI_INVALID_PARAMETER;
    }

    if ((Timeout < SERIAL_PORT_MIN_TIMEOUT) || (Timeout > SERIAL_PORT_MAX_TIMEOUT))
    {
        return EFI_INVALID_PARAMETER;
    }

    if ((Parity < NoParity) || (Parity > SpaceParity))
    {
        return EFI_INVALID_PARAMETER;
    }

    if ((DataBits < 5) || (DataBits > 8))
    {
        return EFI_INVALID_PARAMETER;
    }

    if ((StopBits < OneStopBit) || (StopBits > TwoStopBits))
    {
        return EFI_INVALID_PARAMETER;
    }

    //
    // for DataBits = 6,7,8, StopBits can not set OneFiveStopBits
    //
    if ((DataBits >= 6) && (DataBits <= 8) && (StopBits == OneFiveStopBits))
    {
        return EFI_INVALID_PARAMETER;
    }

    //
    // Compute divisor use to program the baud rate using a round determination
    //
    divisor = (UINT32)DivU64x32Remainder(SERIAL_PORT_INPUT_CLOCK,
                                          ((UINT32) BaudRate * 16),
                                          &remained);
    if (remained != 0)
    {
        divisor += 1;
    }

    if ((divisor == 0) || ((divisor & 0xffff0000) != 0))
    {
        return EFI_INVALID_PARAMETER;
    }

    tpl = gBS->RaiseTPL(TPL_NOTIFY);

    //
    // Compute the actual baud rate that the serial port will be programmed for.
    //
    BaudRate = SERIAL_PORT_INPUT_CLOCK / divisor / 16;

    //
    // Put serial port on divisor Latch Mode
    //
    lcr.Data = READ_LCR(serialDevice->BaseAddress);
    lcr.Bits.DLab = 1;
    WRITE_LCR(serialDevice->BaseAddress, lcr.Data);

    //
    // Write the divisor to the serial port
    //
    WRITE_DLL(serialDevice->BaseAddress,(UINT8)(divisor & 0xff));
    WRITE_DLM(serialDevice->BaseAddress, (UINT8)((divisor >> 8) & 0xff));

    //
    // Put serial port back in normal mode and set remaining attributes.
    //
    lcr.Bits.DLab = 0;

    switch (Parity)
    {
        case NoParity:
            lcr.Bits.ParEn    = 0;
            lcr.Bits.EvenPar  = 0;
            lcr.Bits.SticPar  = 0;
            break;

        case EvenParity:
            lcr.Bits.ParEn    = 1;
            lcr.Bits.EvenPar  = 1;
            lcr.Bits.SticPar  = 0;
            break;

        case OddParity:
            lcr.Bits.ParEn    = 1;
            lcr.Bits.EvenPar  = 0;
            lcr.Bits.SticPar  = 0;
            break;

        case SpaceParity:
            lcr.Bits.ParEn    = 1;
            lcr.Bits.EvenPar  = 1;
            lcr.Bits.SticPar  = 1;
            break;

        case MarkParity:
            lcr.Bits.ParEn    = 1;
            lcr.Bits.EvenPar  = 0;
            lcr.Bits.SticPar  = 1;
            break;

        default:
            break;
    }

    switch (StopBits)
    {
        case OneStopBit:
            lcr.Bits.StopB = 0;
            break;

        case OneFiveStopBits:
        case TwoStopBits:
            lcr.Bits.StopB = 1;
            break;

        default:
            break;
    }
    //
    // DataBits
    //
    lcr.Bits.SerialDB = (UINT8)((DataBits - 5) & 0x03);
    WRITE_LCR(serialDevice->BaseAddress, lcr.Data);

    //
    // Set the Serial I/O mode
    //
    This->Mode->BaudRate          = BaudRate;
    This->Mode->ReceiveFifoDepth  = ReceiveFifoDepth;
    This->Mode->Timeout           = Timeout;
    This->Mode->Parity            = Parity;
    This->Mode->DataBits          = DataBits;
    This->Mode->StopBits          = StopBits;

    //
    // See if Device Path Node has actually changed
    //
    if (serialDevice->UartDevicePath.BaudRate == BaudRate &&
        serialDevice->UartDevicePath.DataBits == DataBits &&
        serialDevice->UartDevicePath.Parity == Parity &&
        serialDevice->UartDevicePath.StopBits == StopBits)
    {
        gBS->RestoreTPL(tpl);
        return EFI_SUCCESS;
    }
    //
    // Update the device path
    //
    serialDevice->UartDevicePath.BaudRate = BaudRate;
    serialDevice->UartDevicePath.DataBits = DataBits;
    serialDevice->UartDevicePath.Parity   = (UINT8) Parity;
    serialDevice->UartDevicePath.StopBits = (UINT8) StopBits;

    status = EFI_SUCCESS;
    if (serialDevice->Handle != NULL)
    {
        uart = (UART_DEVICE_PATH *)((UINTN) serialDevice->DevicePath - END_DEVICE_PATH_LENGTH);
        CopyMem(uart, &serialDevice->UartDevicePath, sizeof(UART_DEVICE_PATH));
        status = gBS->ReinstallProtocolInterface(serialDevice->Handle,
                                                 &gEfiDevicePathProtocolGuid,
                                                 serialDevice->DevicePath,
                                                 serialDevice->DevicePath);
    }

    gBS->RestoreTPL(tpl);

    return status;
}


EFI_STATUS
EFIAPI
SerialSetControl(
    _In_ EFI_SERIAL_IO_PROTOCOL  *This,
    _In_ UINT32                  Control
    )
/*++

Routine Description:

    Set Control Bits.

Arguments:

    This                    Pointer to EFI_SERIAL_IO_PROTOCOL

    Control                 Control bits that can be settable

Return Value:

    EFI_SUCCESS       New Control bits were set successfully

    EFI_UNSUPPORTED   The Control bits to set are not supported

--*/
{
    SERIAL_DEVICE                 *serialDevice;
    SERIAL_PORT_MCR               mcr;
    EFI_TPL                       tpl;
    UART_FLOW_CONTROL_DEVICE_PATH *flowControl;
    EFI_STATUS                    status;

    //
    // The control bits that can be set are :
    //     EFI_SERIAL_DATA_TERMINAL_READY: 0x0001  // WO
    //     EFI_SERIAL_REQUEST_TO_SEND: 0x0002  // WO
    //     EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE: 0x1000  // RW
    //     EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE: 0x2000  // RW
    //     EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE: 0x4000 // RW
    //
    serialDevice = SERIAL_DEVICE_FROM_THIS(This);

    //
    // first determine the parameter is invalid
    //
    if ((Control & (~(EFI_SERIAL_REQUEST_TO_SEND | EFI_SERIAL_DATA_TERMINAL_READY |
                       EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE | EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE |
                       EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE))) != 0)
    {
        return EFI_UNSUPPORTED;
    }

    tpl = gBS->RaiseTPL(TPL_NOTIFY);

    mcr.Data = READ_MCR(serialDevice->BaseAddress);
    mcr.Bits.DtrC = 0;
    mcr.Bits.Rts = 0;
    mcr.Bits.Lme = 0;
    serialDevice->SoftwareLoopbackEnable = FALSE;
    serialDevice->HardwareFlowControl = FALSE;

    if ((Control & EFI_SERIAL_DATA_TERMINAL_READY) == EFI_SERIAL_DATA_TERMINAL_READY)
    {
        mcr.Bits.DtrC = 1;
    }

    if ((Control & EFI_SERIAL_REQUEST_TO_SEND) == EFI_SERIAL_REQUEST_TO_SEND)
    {
        mcr.Bits.Rts = 1;
    }

    if ((Control & EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE) == EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE)
    {
        mcr.Bits.Lme = 1;
    }

    if ((Control & EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE) ==
         EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE)
    {
        serialDevice->HardwareFlowControl = TRUE;
    }

    WRITE_MCR(serialDevice->BaseAddress, mcr.Data);

    if ((Control & EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE) == EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE)
    {
        serialDevice->SoftwareLoopbackEnable = TRUE;
    }

    status = EFI_SUCCESS;
    if (serialDevice->Handle != NULL)
    {
        flowControl = (UART_FLOW_CONTROL_DEVICE_PATH *)(
                        (UINTN) serialDevice->DevicePath
                        - END_DEVICE_PATH_LENGTH
                        + sizeof(UART_DEVICE_PATH)
                        );
        if (IsUartFlowControlNode(flowControl) &&
            ((ReadUnaligned32(&flowControl->FlowControlMap) == UART_FLOW_CONTROL_HARDWARE)
                ^ serialDevice->HardwareFlowControl))
        {
            //
            // Flow Control setting is changed, need to reinstall device path protocol
            //
            WriteUnaligned32(&flowControl->FlowControlMap,
                             serialDevice->HardwareFlowControl ? UART_FLOW_CONTROL_HARDWARE : 0);
            status = gBS->ReinstallProtocolInterface(serialDevice->Handle,
                                                     &gEfiDevicePathProtocolGuid,
                                                     serialDevice->DevicePath,
                                                     serialDevice->DevicePath);
        }
    }

    gBS->RestoreTPL(tpl);

    return status;
}


EFI_STATUS
EFIAPI
SerialGetControl(
    _In_ EFI_SERIAL_IO_PROTOCOL  *This,
    _Out_ UINT32                 *Control
    )
/*++

Routine Description:

    Get Control Bits.

Arguments:

    This            Pointer to EFI_SERIAL_IO_PROTOCOL

    Control         Control bits of the serial device

Return Value:

    EFI_SUCCESS     Control bits were gotten successfully

--*/
{
    SERIAL_DEVICE   *serialDevice;
    SERIAL_PORT_MSR msr;
    SERIAL_PORT_MCR mcr;
    EFI_TPL         tpl;

    tpl= gBS->RaiseTPL(TPL_NOTIFY);

    serialDevice = SERIAL_DEVICE_FROM_THIS(This);

    *Control= 0;

    //
    // Read the Modem status Register
    //
    msr.Data = READ_MSR(serialDevice->BaseAddress);

    if (msr.Bits.Cts == 1)
    {
        *Control |= EFI_SERIAL_CLEAR_TO_SEND;
    }

    if (msr.Bits.Dsr == 1)
    {
        *Control |= EFI_SERIAL_DATA_SET_READY;
    }

    if (msr.Bits.Ri == 1)
    {
        *Control |= EFI_SERIAL_RING_INDICATE;
    }

    if (msr.Bits.Dcd == 1)
    {
        *Control |= EFI_SERIAL_CARRIER_DETECT;
    }
    //
    // Read the Modem Control Register
    //
    mcr.Data = READ_MCR(serialDevice->BaseAddress);

    if (mcr.Bits.DtrC == 1)
    {
        *Control |= EFI_SERIAL_DATA_TERMINAL_READY;
    }

    if (mcr.Bits.Rts == 1)
    {
        *Control |= EFI_SERIAL_REQUEST_TO_SEND;
    }

    if (mcr.Bits.Lme == 1)
    {
        *Control |= EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE;
    }

    if (serialDevice->HardwareFlowControl)
    {
        *Control |= EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE;
    }
    //
    // See if the Transmit FIFO is empty
    //
    SerialReceiveTransmit(serialDevice);

    if (SerialFifoEmpty(&serialDevice->Transmit))
    {
        *Control |= EFI_SERIAL_OUTPUT_BUFFER_EMPTY;
    }
    //
    // See if the Receive FIFO is empty.
    //
    SerialReceiveTransmit(serialDevice);

    if (SerialFifoEmpty(&serialDevice->Receive))
    {
        *Control |= EFI_SERIAL_INPUT_BUFFER_EMPTY;
    }

    if (serialDevice->SoftwareLoopbackEnable)
    {
        *Control |= EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE;
    }

    gBS->RestoreTPL(tpl);

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
SerialWrite(
    _In_ EFI_SERIAL_IO_PROTOCOL  *This,
    _In_ OUT UINTN               *BufferSize,
    _In_ VOID                    *Buffer
    )
/*++

Routine Description:

    Write the specified number of bytes to serial device.

Arguments:

    This            Pointer to EFI_SERIAL_IO_PROTOCOL

    BufferSize      On input the size of Buffer, on output the amount of
                    data actually written.

    Buffer          The buffer of data to write

Return Value:

    EFI_SUCCESS        The data bytes were written successfully

    EFI_DEVICE_ERROR   The device reported an error

    EFI_TIMEOUT        The write operation was stopped due to timeout

--*/
{
    SERIAL_DEVICE  *serialDevice;
    UINT8          *charBuffer;
    UINT32         index;
    UINTN          elapsed;
    UINTN          actualWrite;
    EFI_TPL        tpl;
    UINTN          timeout;
    UINTN          bitsPerCharacter;

    serialDevice  = SERIAL_DEVICE_FROM_THIS(This);
    elapsed       = 0;
    actualWrite   = 0;

    if (*BufferSize == 0)
    {
        return EFI_SUCCESS;
    }

    if (Buffer == NULL)
    {
        REPORT_STATUS_CODE_WITH_DEVICE_PATH(
            EFI_ERROR_CODE,
            EFI_P_EC_OUTPUT_ERROR | EFI_PERIPHERAL_SERIAL_PORT,
            serialDevice->DevicePath);

        return EFI_DEVICE_ERROR;
    }

    tpl = gBS->RaiseTPL(TPL_NOTIFY);

    charBuffer  = (UINT8 *) Buffer;

    //
    // Compute the number of bits in a single character.  This is a start bit,
    // followed by the number of data bits, followed by the number of stop bits.
    // The number of stop bits is specified by an enumeration that includes
    // support for 1.5 stop bits.  Treat 1.5 stop bits as 2 stop bits.
    //
    bitsPerCharacter =
        1 +
        This->Mode->DataBits +
        ((This->Mode->StopBits == TwoStopBits) ? 2 : This->Mode->StopBits);

    //
    // Compute the timeout in microseconds to wait for a single byte to be
    // transmitted.  The Mode structure contans a Timeout field that is the
    // maximum time to transmit or receive a character.  However, many UARTs
    // have a FIFO for transmits, so the time required to add one new character
    // to the transmit FIFO may be the time required to flush a full FIFO.  If
    // the Timeout in the Mode structure is smaller than the time required to
    // flush a full FIFO at the current baud rate, then use a timeout value that
    // is required to flush a full transmit FIFO.
    //
    timeout = MAX(This->Mode->Timeout,
                  (UINTN)DivU64x64Remainder(
                      bitsPerCharacter * (SERIAL_PORT_MAX_RECEIVE_FIFO_DEPTH + 1) * 1000000,
                      This->Mode->BaudRate,
                      NULL
                      )
                  );

    for (index = 0; index < *BufferSize; index++)
    {
        SerialFifoAdd(&serialDevice->Transmit, charBuffer[index]);

        while (SerialReceiveTransmit(serialDevice) != EFI_SUCCESS ||
            !SerialFifoEmpty(&serialDevice->Transmit))
        {
            //
            //  Unsuccessful write so check if timeout has expired, if not,
            //  stall for a bit, increment time elapsed, and try again
            //
            if (elapsed >= timeout)
            {
                *BufferSize = actualWrite;
                gBS->RestoreTPL(tpl);
                return EFI_TIMEOUT;
            }

            gBS->Stall(TIMEOUT_STALL_INTERVAL);

            elapsed += TIMEOUT_STALL_INTERVAL;
        }

        actualWrite++;
        //
        //  Successful write so reset timeout
        //
        elapsed = 0;
    }

    gBS->RestoreTPL(tpl);

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
SerialRead(
    _In_    EFI_SERIAL_IO_PROTOCOL  *This,
    _Inout_ UINTN                   *BufferSize,
    _Out_   VOID                    *Buffer
    )
/*++

Routine Description:

    Read the specified number of bytes from serial device.

Arguments:

    This            Pointer to EFI_SERIAL_IO_PROTOCOL

    BufferSize      On input the size of Buffer, on output the amount of
                    data returned in the buffer.

    Buffer          The buffer to return the data into

Return Value:

    EFI_SUCCESS        The data bytes were read successfully

    EFI_DEVICE_ERROR   The device reported an error

    EFI_TIMEOUT        The read operation was stopped due to timeout

--*/
{
      SERIAL_DEVICE  *serialDevice;
      UINT32         index;
      UINT8          *charBuffer;
      UINTN          elapsed;
      EFI_STATUS     status;
      EFI_TPL        tpl;

      serialDevice = SERIAL_DEVICE_FROM_THIS(This);
      elapsed = 0;

    if (*BufferSize == 0)
    {
        return EFI_SUCCESS;
    }

    if (Buffer == NULL)
    {
        return EFI_DEVICE_ERROR;
    }

    tpl = gBS->RaiseTPL(TPL_NOTIFY);

    status = SerialReceiveTransmit(serialDevice);

    if (EFI_ERROR(status))
    {
        *BufferSize = 0;

        REPORT_STATUS_CODE_WITH_DEVICE_PATH(
            EFI_ERROR_CODE,
            EFI_P_EC_INPUT_ERROR | EFI_PERIPHERAL_SERIAL_PORT,
            serialDevice->DevicePath);

        gBS->RestoreTPL(tpl);

        return EFI_DEVICE_ERROR;
    }

    charBuffer = (UINT8 *) Buffer;
    for (index = 0; index < *BufferSize; index++)
    {
        while (SerialFifoRemove(&serialDevice->Receive, &(charBuffer[index])) != EFI_SUCCESS)
        {
            //
            //  Unsuccessful read so check if timeout has expired, if not,
            //  stall for a bit, increment time elapsed, and try again
            //  Need this time out to get conspliter to work.
            //
            if (elapsed >= This->Mode->Timeout)
            {
                *BufferSize = index;
                gBS->RestoreTPL(tpl);
                return EFI_TIMEOUT;
            }

            gBS->Stall(TIMEOUT_STALL_INTERVAL);
            elapsed += TIMEOUT_STALL_INTERVAL;

            status = SerialReceiveTransmit(serialDevice);
            if (status == EFI_DEVICE_ERROR)
            {
                *BufferSize = index;
                gBS->RestoreTPL(tpl);
                return EFI_DEVICE_ERROR;
            }
        }

        //
        //  Successful read so reset timeout
        //
        elapsed = 0;
    }

    SerialReceiveTransmit(serialDevice);

    gBS->RestoreTPL(tpl);

    return EFI_SUCCESS;
}


BOOLEAN
SerialPortPresent(
      _In_ SERIAL_DEVICE *serialDevice
      )
/*++

Routine Description:

    Probe the scratchpad register to test if this serial port is present.

Arguments:

    SerialDevice   Pointer to serial device structure

Return Value:

    If the serial port is present

--*/
{
    UINT8   temp;
    BOOLEAN status;

    status = TRUE;

    //
    // Save SCR reg
    //
    temp = READ_SCR(serialDevice->BaseAddress);

    //
    // Try writing two patterns to SCR and see if they stick.
    //
    WRITE_SCR(serialDevice->BaseAddress, 0xAA);
    if (READ_SCR(serialDevice->BaseAddress) != 0xAA)
    {
        status = FALSE;
    }
    WRITE_SCR(serialDevice->BaseAddress, 0x55);
    if (READ_SCR(serialDevice->BaseAddress) != 0x55)
    {
        status = FALSE;
    }
    //
    // Restore SCR
    //
    WRITE_SCR(serialDevice->BaseAddress, temp);
    return status;
}


UINT8
SerialReadPort(
    _In_ UINT16 BaseAddress,
    _In_ UINT32 Offset
    )
/*++

Routine Description:

    Read a serial port register.

Arguments:

    BaseAddress   Serial port register group base address

    Offset        Offset in register group

Return Value:

    Data read from serial port

--*/
{
    return IoRead8(BaseAddress + Offset);
}


VOID
SerialWritePort(
    _In_ UINT16 BaseAddress,
    _In_ UINT32 Offset,
    _In_ UINT8  Data
    )
/*++

Routine Description:

    Write a serial port register.

Arguments:

    BaseAddress     Serial port register group base address

    Offset          Offset in register group

    Data            Data to write to register.

Return Value:

    n/a

--*/
{
    IoWrite8(BaseAddress + Offset, Data);
}

