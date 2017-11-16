/*++

ATTENTION - THIS FILE IS DERIVED FROM THIRD PARTY OPEN SOURCE CODE:
           IntelFrameworkModulePackage\Bus\Isa\IsaSerialDxe\Serial.h
IT IS CLEARED ONLY FOR LIMITED USE BY WINDOWS CORE HYPER-V FOR THE HYPER-V ROLE IN THE
WINDOWS PRODUCT.  DO NOT USE OR SHARE THIS CODE WITHOUT APPROVAL PURSUANT TO THE
MICROSOFT OPEN SOURCE  SOFTWARE APPROVAL POLICY.

Module Name:

    Serial.h

Abstract:

    Provides the header for the Hyper-V serial ports.

Author:

    Larry Cleeton (lcleeton) - 08-Aug-2014

--*/


#pragma once

#include <EfiNt.h>
#include <Protocol/SerialIo.h>
#include <Protocol/DevicePath.h>

#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <UefiConstants.h>
#include "MsvmSerial.h"

//
// Driver Binding Externs
//
extern EFI_DRIVER_BINDING_PROTOCOL  gSerialDriver;
extern EFI_COMPONENT_NAME_PROTOCOL  gSerialComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL gSerialComponentName2;

//
// Internal Data Structures
//
#define SERIAL_DEVICE_SIGNATURE  SIGNATURE_32('s', 'e', 'r', 'd')
#define SERIAL_MAX_BUFFER_SIZE   16
#define TIMEOUT_STALL_INTERVAL   10

//
//  Name:   SERIAL_DEV_FIFO
//  Purpose:  To define Receive FIFO and Transmit FIFO
//  Context:  Used by serial data transmit and receive
//  Fields:
//      First UINT32: The index of the first data in array Data[]
//      Last  UINT32: The index, which you can put a new data into array Data[]
//      Surplus UINT32: Identify how many data you can put into array Data[]
//      Data[]  UINT8 : An array, which used to store data
//
typedef struct
{
    UINT32  First;
    UINT32  Last;
    UINT32  Surplus;
    UINT8   Data[SERIAL_MAX_BUFFER_SIZE];
} SERIAL_DEV_FIFO;

typedef enum
{
    Uart8250  = 0,
    Uart16450 = 1,
    Uart16550 = 2,
    Uart16550A= 3
} EFI_UART_TYPE;

typedef struct
{
    UINT16 BaseAddress;
    UINT32 HID;
    UINT32 UID;
} SERIAL_DEVICE_PROPERTIES;


//
//  Name:   SERIAL_DEVICE
//  Purpose:  To provide device specific information
//  Context:
//  Fields:
//      Signature UINTN: The identity of the serial device
//      SerialIo  SERIAL_IO_PROTOCOL: Serial I/O protocol interface
//      SerialMode  SERIAL_IO_MODE:
//      DevicePath  EFI_DEVICE_PATH_PROTOCOL *: Device path of the serial device
//      Handle      EFI_HANDLE: The handle instance attached to serial device
//      BaseAddress UINT16: The base address of specific serial device
//      Receive     SERIAL_DEV_FIFO: The FIFO used to store data,
//                  which is received by UART
//      Transmit    SERIAL_DEV_FIFO: The FIFO used to store data,
//                  which you want to transmit by UART
//      SoftwareLoopbackEnable BOOLEAN:
//      Type    EFI_UART_TYPE: Specify the UART type of certain serial device
//
typedef struct
{
    UINTN                                  Signature;
    EFI_HANDLE                             Handle;
    BOOLEAN                                BusProtocolOpened;
    EFI_SERIAL_IO_PROTOCOL                 SerialIo;
    EFI_SERIAL_IO_MODE                     SerialMode;
    EFI_DEVICE_PATH_PROTOCOL               *DevicePath;
    UART_DEVICE_PATH                       UartDevicePath;
    UINT16                                 BaseAddress;
    SERIAL_DEV_FIFO                        Receive;
    SERIAL_DEV_FIFO                        Transmit;
    BOOLEAN                                SoftwareLoopbackEnable;
    BOOLEAN                                HardwareFlowControl;
    EFI_UART_TYPE                          Type;
    EFI_UNICODE_STRING_TABLE               *ControllerNameTable;
} SERIAL_DEVICE;

#define SERIAL_DEVICE_FROM_THIS(a) CR(a, SERIAL_DEVICE, SerialIo, SERIAL_DEVICE_SIGNATURE)

//
// Serial Driver Defaults
//
#define SERIAL_PORT_DEFAULT_RECEIVE_FIFO_DEPTH  1
#define SERIAL_PORT_DEFAULT_TIMEOUT             1000000
#define SERIAL_PORT_SUPPORT_CONTROL_MASK        (EFI_SERIAL_CLEAR_TO_SEND                | \
                                                 EFI_SERIAL_DATA_SET_READY               | \
                                                 EFI_SERIAL_RING_INDICATE                | \
                                                 EFI_SERIAL_CARRIER_DETECT               | \
                                                 EFI_SERIAL_REQUEST_TO_SEND              | \
                                                 EFI_SERIAL_DATA_TERMINAL_READY          | \
                                                 EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE     | \
                                                 EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE     | \
                                                 EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE | \
                                                 EFI_SERIAL_OUTPUT_BUFFER_EMPTY          | \
                                                 EFI_SERIAL_INPUT_BUFFER_EMPTY)


//
// (24000000/13)MHz input clock
//
#define SERIAL_PORT_INPUT_CLOCK 1843200

//
// 115200 baud with rounding errors
//
#define SERIAL_PORT_MAX_BAUD_RATE           115400
#define SERIAL_PORT_MIN_BAUD_RATE           50

#define SERIAL_PORT_MAX_RECEIVE_FIFO_DEPTH  16
#define SERIAL_PORT_MIN_TIMEOUT             1         // 1 uS
#define SERIAL_PORT_MAX_TIMEOUT             100000000 // 100 seconds
//
// UART Registers
//
#define SERIAL_REGISTER_THR 0 // WO   Transmit Holding Register
#define SERIAL_REGISTER_RBR 0 // RO   Receive Buffer Register
#define SERIAL_REGISTER_DLL 0 // R/W  Divisor Latch LSB
#define SERIAL_REGISTER_DLM 1 // R/W  Divisor Latch MSB
#define SERIAL_REGISTER_IER 1 // R/W  Interrupt Enable Register
#define SERIAL_REGISTER_IIR 2 // RO   Interrupt Identification Register
#define SERIAL_REGISTER_FCR 2 // WO   FIFO Cotrol Register
#define SERIAL_REGISTER_LCR 3 // R/W  Line Control Register
#define SERIAL_REGISTER_MCR 4 // R/W  Modem Control Register
#define SERIAL_REGISTER_LSR 5 // R/W  Line Status Register
#define SERIAL_REGISTER_MSR 6 // R/W  Modem Status Register
#define SERIAL_REGISTER_SCR 7 // R/W  Scratch Pad Register

#pragma pack(1)
//
//  Name:   SERIAL_PORT_IER_BITS
//  Purpose:  Define each bit in Interrupt Enable Register
//  Context:
//  Fields:
//     Ravie  Bit0: Receiver Data Available Interrupt Enable
//     Theie  Bit1: Transmistter Holding Register Empty Interrupt Enable
//     Rie      Bit2: Receiver Interrupt Enable
//     Mie      Bit3: Modem Interrupt Enable
//     Reserved Bit4-Bit7: Reserved
//
typedef struct
{
    UINT8 Ravie : 1;
    UINT8 Theie : 1;
    UINT8 Rie : 1;
    UINT8 Mie : 1;
    UINT8 Reserved : 4;
} SERIAL_PORT_IER_BITS;

//
//  Name:   SERIAL_PORT_IER
//  Purpose:
//  Context:
//  Fields:
//      Bits    SERIAL_PORT_IER_BITS:  Bits of the IER
//      Data    UINT8: the value of the IER
//
typedef union
{
    SERIAL_PORT_IER_BITS  Bits;
    UINT8                 Data;
} SERIAL_PORT_IER;

//
//  Name:   SERIAL_PORT_FCR_BITS
//  Purpose:  Define each bit in FIFO Control Register
//  Context:
//  Fields:
//      TrFIFOE    Bit0: Transmit and Receive FIFO Enable
//      ResetRF    Bit1: Reset Reciever FIFO
//      ResetTF    Bit2: Reset Transmistter FIFO
//      Dms        Bit3: DMA Mode Select
//      Reserved   Bit4-Bit5: Reserved
//      Rtb        Bit6-Bit7: Receive Trigger Bits
//
typedef struct
{
    UINT8 TrFIFOE : 1;
    UINT8 ResetRF : 1;
    UINT8 ResetTF : 1;
    UINT8 Dms : 1;
    UINT8 Reserved : 2;
    UINT8 Rtb : 2;
} SERIAL_PORT_FCR_BITS;

//
//  Name:   SERIAL_PORT_FCR
//  Purpose:
//  Context:
//  Fields:
//      Bits    SERIAL_PORT_FCR_BITS:  Bits of the FCR
//      Data    UINT8: the value of the FCR
//
typedef union
{
    SERIAL_PORT_FCR_BITS  Bits;
    UINT8                 Data;
} SERIAL_PORT_FCR;

//
//  Name:   SERIAL_PORT_LCR_BITS
//  Purpose:  Define each bit in Line Control Register
//  Context:
//  Fields:
//      SerialDB  Bit0-Bit1: Number of Serial Data Bits
//      StopB     Bit2: Number of Stop Bits
//      ParEn     Bit3: Parity Enable
//      EvenPar   Bit4: Even Parity Select
//      SticPar   Bit5: Sticky Parity
//      BrCon     Bit6: Break Control
//      DLab      Bit7: Divisor Latch Access Bit
//
typedef struct
{
    UINT8 SerialDB : 2;
    UINT8 StopB : 1;
    UINT8 ParEn : 1;
    UINT8 EvenPar : 1;
    UINT8 SticPar : 1;
    UINT8 BrCon : 1;
    UINT8 DLab : 1;
} SERIAL_PORT_LCR_BITS;

//
//  Name:   SERIAL_PORT_LCR
//  Purpose:
//  Context:
//  Fields:
//      Bits    SERIAL_PORT_LCR_BITS:  Bits of the LCR
//      Data    UINT8: the value of the LCR
//
typedef union
{
    SERIAL_PORT_LCR_BITS  Bits;
    UINT8                 Data;
} SERIAL_PORT_LCR;

//
//  Name:   SERIAL_PORT_MCR_BITS
//  Purpose:  Define each bit in Modem Control Register
//  Context:
//  Fields:
//      DtrC     Bit0: Data Terminal Ready Control
//      Rts      Bit1: Request To Send Control
//      Out1     Bit2: Output1
//      Out2     Bit3: Output2, used to disable interrupt
//      Lme;     Bit4: Loopback Mode Enable
//      Reserved Bit5-Bit7: Reserved
//
typedef struct
{
    UINT8 DtrC : 1;
    UINT8 Rts : 1;
    UINT8 Out1 : 1;
    UINT8 Out2 : 1;
    UINT8 Lme : 1;
    UINT8 Reserved : 3;
} SERIAL_PORT_MCR_BITS;

//
//  Name:   SERIAL_PORT_MCR
//  Purpose:
//  Context:
//  Fields:
//      Bits    SERIAL_PORT_MCR_BITS:  Bits of the MCR
//      Data    UINT8: the value of the MCR
//
typedef union
{
    SERIAL_PORT_MCR_BITS  Bits;
    UINT8                 Data;
} SERIAL_PORT_MCR;

//
//  Name:   SERIAL_PORT_LSR_BITS
//  Purpose:  Define each bit in Line Status Register
//  Context:
//  Fields:
//      Dr    Bit0: Receiver Data Ready Status
//      Oe    Bit1: Overrun Error Status
//      Pe    Bit2: Parity Error Status
//      Fe    Bit3: Framing Error Status
//      Bi    Bit4: Break Interrupt Status
//      Thre  Bit5: Transmistter Holding Register Status
//      Temt  Bit6: Transmitter Empty Status
//      FIFOe Bit7: FIFO Error Status
//
typedef struct
{
    UINT8 Dr : 1;
    UINT8 Oe : 1;
    UINT8 Pe : 1;
    UINT8 Fe : 1;
    UINT8 Bi : 1;
    UINT8 Thre : 1;
    UINT8 Temt : 1;
    UINT8 FIFOe : 1;
} SERIAL_PORT_LSR_BITS;

//
//  Name:   SERIAL_PORT_LSR
//  Purpose:
//  Context:
//  Fields:
//      Bits    SERIAL_PORT_LSR_BITS:  Bits of the LSR
//      Data    UINT8: the value of the LSR
//
typedef union
{
    SERIAL_PORT_LSR_BITS  Bits;
    UINT8                 Data;
} SERIAL_PORT_LSR;

//
//  Name:   SERIAL_PORT_MSR_BITS
//  Purpose:  Define each bit in Modem Status Register
//  Context:
//  Fields:
//      DeltaCTS      Bit0: Delta Clear To Send Status
//      DeltaDSR        Bit1: Delta Data Set Ready Status
//      TrailingEdgeRI  Bit2: Trailing Edge of Ring Indicator Status
//      DeltaDCD        Bit3: Delta Data Carrier Detect Status
//      Cts             Bit4: Clear To Send Status
//      Dsr             Bit5: Data Set Ready Status
//      Ri              Bit6: Ring Indicator Status
//      Dcd             Bit7: Data Carrier Detect Status
//
typedef struct
{
    UINT8 DeltaCTS : 1;
    UINT8 DeltaDSR : 1;
    UINT8 TrailingEdgeRI : 1;
    UINT8 DeltaDCD : 1;
    UINT8 Cts : 1;
    UINT8 Dsr : 1;
    UINT8 Ri : 1;
    UINT8 Dcd : 1;
} SERIAL_PORT_MSR_BITS;

//
//  Name:   SERIAL_PORT_MSR
//  Purpose:
//  Context:
//  Fields:
//      Bits    SERIAL_PORT_MSR_BITS:  Bits of the MSR
//      Data    UINT8: the value of the MSR
//
typedef union
{
    SERIAL_PORT_MSR_BITS  Bits;
    UINT8                 Data;
} SERIAL_PORT_MSR;

#pragma pack()
//
// Define serial register I/O macros
//
#define READ_RBR(B)     SerialReadPort(B, SERIAL_REGISTER_RBR)
#define READ_DLL(B)     SerialReadPort(B, SERIAL_REGISTER_DLL)
#define READ_DLM(B)     SerialReadPort(B, SERIAL_REGISTER_DLM)
#define READ_IER(B)     SerialReadPort(B, SERIAL_REGISTER_IER)
#define READ_IIR(B)     SerialReadPort(B, SERIAL_REGISTER_IIR)
#define READ_LCR(B)     SerialReadPort(B, SERIAL_REGISTER_LCR)
#define READ_MCR(B)     SerialReadPort(B, SERIAL_REGISTER_MCR)
#define READ_LSR(B)     SerialReadPort(B, SERIAL_REGISTER_LSR)
#define READ_MSR(B)     SerialReadPort(B, SERIAL_REGISTER_MSR)
#define READ_SCR(B)     SerialReadPort(B, SERIAL_REGISTER_SCR)

#define WRITE_THR(B, D) SerialWritePort(B, SERIAL_REGISTER_THR, D)
#define WRITE_DLL(B, D) SerialWritePort(B, SERIAL_REGISTER_DLL, D)
#define WRITE_DLM(B, D) SerialWritePort(B, SERIAL_REGISTER_DLM, D)
#define WRITE_IER(B, D) SerialWritePort(B, SERIAL_REGISTER_IER, D)
#define WRITE_FCR(B, D) SerialWritePort(B, SERIAL_REGISTER_FCR, D)
#define WRITE_LCR(B, D) SerialWritePort(B, SERIAL_REGISTER_LCR, D)
#define WRITE_MCR(B, D) SerialWritePort(B, SERIAL_REGISTER_MCR, D)
#define WRITE_LSR(B, D) SerialWritePort(B, SERIAL_REGISTER_LSR, D)
#define WRITE_MSR(B, D) SerialWritePort(B, SERIAL_REGISTER_MSR, D)
#define WRITE_SCR(B, D) SerialWritePort(B, SERIAL_REGISTER_SCR, D)

//
// Prototypes
// Driver model protocol interface
//
EFI_STATUS
EFIAPI
SerialDriverSupported(
    _In_ EFI_DRIVER_BINDING_PROTOCOL    *This,
    _In_ EFI_HANDLE                     Controller,
    _In_ EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
    );

EFI_STATUS
EFIAPI
SerialDriverStart(
    _In_ EFI_DRIVER_BINDING_PROTOCOL    *This,
    _In_ EFI_HANDLE                     Controller,
    _In_ EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
    );

EFI_STATUS
EFIAPI
SerialDriverStop(
    _In_  EFI_DRIVER_BINDING_PROTOCOL   *This,
    _In_  EFI_HANDLE                    Controller,
    _In_  UINTN                         NumberOfChildren,
    _In_  EFI_HANDLE                    *ChildHandleBuffer
    );

//
// Serial I/O Protocol Interface
//
EFI_STATUS
EFIAPI
SerialReset(
    _In_ EFI_SERIAL_IO_PROTOCOL         *This
    );

EFI_STATUS
EFIAPI
SerialSetAttributes(
    _In_ EFI_SERIAL_IO_PROTOCOL         *This,
    _In_ UINT64                         BaudRate,
    _In_ UINT32                         ReceiveFifoDepth,
    _In_ UINT32                         Timeout,
    _In_ EFI_PARITY_TYPE                Parity,
    _In_ UINT8                          DataBits,
    _In_ EFI_STOP_BITS_TYPE             StopBits
    );

EFI_STATUS
EFIAPI
SerialSetControl(
    _In_ EFI_SERIAL_IO_PROTOCOL         *This,
    _In_ UINT32                         Control
    );

EFI_STATUS
EFIAPI
SerialGetControl(
    _In_ EFI_SERIAL_IO_PROTOCOL         *This,
    _Out_ UINT32                        *Control
    );

EFI_STATUS
EFIAPI
SerialWrite(
    _In_ EFI_SERIAL_IO_PROTOCOL         *This,
    _Inout_ UINTN                       *BufferSize,
    _In_ VOID                           *Buffer
    );

EFI_STATUS
EFIAPI
SerialRead(
    _In_ EFI_SERIAL_IO_PROTOCOL         *This,
    _Inout_ UINTN                       *BufferSize,
    _Out_ VOID                          *Buffer
    );

//
// Internal Functions
//
BOOLEAN
SerialPortPresent(
    _In_ SERIAL_DEVICE                  *SerialDevice
    );

BOOLEAN
SerialFifoFull(
    _In_ SERIAL_DEV_FIFO                *Fifo
    );

BOOLEAN
SerialFifoEmpty(
    _In_ SERIAL_DEV_FIFO                *Fifo
    );

EFI_STATUS
SerialFifoAdd(
    _In_ SERIAL_DEV_FIFO                *Fifo,
    _In_ UINT8                          Data
    );

EFI_STATUS
SerialFifoRemove(
    _In_  SERIAL_DEV_FIFO               *Fifo,
    _Out_ UINT8                         *Data
    );

EFI_STATUS
SerialReceiveTransmit(
    _In_ SERIAL_DEVICE                  *SerialDevice
    );

UINT8
SerialReadPort(
    _In_ UINT16                         BaseAddress,
    _In_ UINT32                         Offset
    );

VOID
SerialWritePort(
    _In_ UINT16                         BaseAddress,
    _In_ UINT32                         Offset,
    _In_ UINT8                          Data
    );


//
// EFI Component Name Functions
//
EFI_STATUS
EFIAPI
SerialComponentNameGetDriverName(
    _In_  EFI_COMPONENT_NAME_PROTOCOL   *This,
    _In_  CHAR8                         *Language,
    _Out_ CHAR16                        **DriverName
    );


EFI_STATUS
EFIAPI
SerialComponentNameGetControllerName(
    _In_  EFI_COMPONENT_NAME_PROTOCOL   *This,
    _In_  EFI_HANDLE                    ControllerHandle,
    _In_opt_ EFI_HANDLE                 ChildHandle,
    _In_  CHAR8                         *Language,
    _Out_ CHAR16                        **ControllerName
    );

VOID
AddName(
    _In_  SERIAL_DEVICE                 *SerialDevice,
    _In_  SERIAL_DEVICE_PROPERTIES      *SerialProperties
    );
