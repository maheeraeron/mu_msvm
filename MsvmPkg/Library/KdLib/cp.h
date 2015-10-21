/*++

Copyright (c) Microsoft Corporation

Module Name:

    cp.h

Abstract:

    This module contains the header file for a very simple com 
    port package.

Author:

    Bryan M. Willman (bryanwi) 24-Sep-1990
    John Vert (jvert) 19-Jul-1991
    Adrian J. Oney (adriao) 06-Oct-2005

--*/

#include "EfiNt.h"
#include "EfiKd.h"

#define TIMEOUT_COUNT (1024 * 50) 

//
// EFI KD specific defines.
//
#define EFI_KD_115200   115200
#define EFI_KD_19200    19200

//
// Private fields for the Flags member of the com port object.
//
#define PORT_ANSWER_MODEM   0x1000      // answer incoming modem rings
#define PORT_MODEMCONTROL   0x2000      // using modem controls
#define PORT_NOCDLTIME      0x4000      // 'Carrier detect lost' time not set
#define PORT_MDM_CD         0x8000      // CD while in modem control mode

#define COM_DAT     0x00
#define COM_IEN     0x01            // interrupt enable register
#define COM_IDN     0x02            // interrupt ident register
#define COM_FCR     0x02            // interrupt FIFO register
#define COM_LCR     0x03            // line control registers
#define COM_MCR     0x04            // modem control reg
#define COM_LSR     0x05            // line status register
#define COM_MSR     0x06            // modem status register
#define COM_DLL     0x00            // divisor latch least sig
#define COM_DLM     0x01            // divisor latch most sig

#define COM_BI      0x10
#define COM_FE      0x08
#define COM_PE      0x04
#define COM_OE      0x02

#define LC_DLAB     0x80            // divisor latch access bit

#define CLOCK_RATE  0x1C200         // USART clock rate

#define MC_DTRRTS   0x03            // Control bits to assert DTR and RTS
#define MS_DSRCTSCD 0xB0            // Status bits for DSR, CTS and CD
#define MS_CD       0x80

#define COM_OUTRDY  0x20
#define COM_DATRDY  0x01

//
// This bit controls the loopback testing mode of the device.  Basically
// the outputs are connected to the inputs (and vice versa).
//
#define SERIAL_MCR_LOOP     0x10

//
// This bit is used for general purpose output.
//
#define SERIAL_MCR_OUT1     0x04

//
// This bit contains the (complemented) state of the clear to send
// (CTS) line.
//
#define SERIAL_MSR_CTS      0x10

//
// This bit contains the (complemented) state of the data set ready
// (DSR) line.
//
#define SERIAL_MSR_DSR      0x20

//
// This bit contains the (complemented) state of the ring indicator
// (RI) line.
//
#define SERIAL_MSR_RI       0x40

//
// This bit contains the (complemented) state of the data carrier detect
// (DCD) line.
//
#define SERIAL_MSR_DCD      0x80

#define SERIAL_LSR_NOT_PRESENT  0xff

//
// Bits for enabling FIFO in COM_FCR and for reading back FIFO enabled status in 
// COM_IDN
//
#define SERIAL_ENABLE_FIFO  0x1
#define SERIAL_FIFO_ENABLED_BIT 0xC

typedef enum
{
    CpPortTypeUninitialized = 0,
    CpPortTypeIoPort,
    CpPortTypeMemoryMapped
} CP_PORT_ADDRESS_TYPE, *PCP_PORT_ADDRESS_TYPE;

typedef struct
{
    CP_PORT_ADDRESS_TYPE Type;
    union
    {
        UINT16 IoPort;
        HV_SPA RegistersSpa;
    };
} CP_PORT_ADDRESS, *PCP_PORT_ADDRESS;

typedef enum
{
    CpBrNone    = 0,
    CpBr150     = 150,
    CpBr300     = 300,
    CpBr600     = 600,
    CpBr1200    = 1200,
    CpBr2400    = 2400,
    CpBr4800    = 4800,
    CpBr9600    = 9600,
    CpBr14400   = 14400,
    CpBr19200   = 19200,
    CpBr57600   = 57600,
    CpBr115200  = 115200

} CP_BAUD_RATE, *PCP_BAUD_RATE;

typedef enum _CP_MODEM_CONTROL
{
    CpMcIgnoreRings = 0,
    CpMcAnswerRings

} CP_MODEM_CONTROL, *PCP_MODEM_CONTROL;

typedef enum
{
    //
    // Status returned if data was available.
    //
    CpStatusSuccess,

    //
    // Status returns if no data is available, but not due to error.
    //
    CpStatusNoData,

    //
    // Status returned on overrun, parity, etc.
    //
    CpStatusError,

    //
    // Status returned on not ready.
    //
    CpStatusNotReady

} CP_STATUS, *PCP_STATUS;

typedef struct _CP_PORT
{
    CP_PORT_ADDRESS Address;
    CP_BAUD_RATE    BaudRate;
    PUINT8          VaForPhysicalAddress;
    UINT16          Flags;
    UINT8           LastLsr;
    UINT8           LastMsr;
    UINT8           RingHistory; // bit0 set if ever on, bit1 set if ever off.
    UINT8           IrqIden;
} CP_PORT, *PCP_PORT;

#define CP_GET_SUCCESS  0  
#define CP_GET_NODATA   1  
#define CP_GET_ERROR    2  

#define COM1_PORT   0x03f8  
#define COM2_PORT   0x02f8  
#define COM3_PORT   0x03e8  
#define COM4_PORT   0x02e8  

//
// Standard com port support routines.
//
// NOTICE-2006/03/22-bartn -- Address is passed as a pointer to prevent
//     code generation that uses XMM registers.
//
VOID
CpPortInit(
    __out PCP_PORT Port,
    __in PCP_PORT_ADDRESS Address,
    __in CP_BAUD_RATE BaudRate,
    __in CP_MODEM_CONTROL ModemControl
    );

CP_STATUS
CpPortWrite(
    __in PCP_PORT Port,
    __in UINT8 Byte,
    __in BOOLEAN BusyWait
    );

CP_STATUS
CpPortRead(
    __in PCP_PORT Port,
    __out PUINT8 Byte,
    __in BOOLEAN WaitForByte
    );

BOOLEAN
CpPortDataReady(
    __in PCP_PORT Port
    );

VOID
CppPortMapRegisters(
    __inout PCP_PORT Port
    );

UINT8
CppPortReadLsr(
    __in PCP_PORT Port,
    __in UINT8 PolledField
    );

VOID
CppPortSetBaud(
    __inout PCP_PORT Port,
    __in CP_BAUD_RATE BaudRate
    );

UINT8
CppPortReadRegister8(
    __in PCP_PORT Port,
    __in UINT8 RegisterNumber
    );

VOID
CppPortWriteRegister8(
    __in PCP_PORT Port,
    __in UINT8 RegisterNumber,
    __in UINT8 Value
    );

VOID
CppCheckPowerButton(
    VOID
    );

VOID
CppDumpPortString(
    __in PSTR String
    );

