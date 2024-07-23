/*++

Copyright (c) Microsoft Corporation

Module Name:

    cp.h

Abstract:

    This module contains the header file for a very simple com
    port package.

    Adapted from earlier versions of the Windows boot debugger.

--*/

#include "EfiNt.h"
#include "Bd.h"

#if defined(_ARM64)
#define TIMEOUT_COUNT (10)
#else
#define TIMEOUT_COUNT (1024 * 50)
#endif

//
// UEFI BD specific defines.
//
#define BD_115200   115200
#define BD_19200    19200

//
// Device address.
//
typedef UINT32 CP_PORT_ADDRESS_TYPE, *PCP_PORTADDRESS_TYPE;

static const CP_PORT_ADDRESS_TYPE CpPortTypeUninitialized   = 0;
static const CP_PORT_ADDRESS_TYPE CpPortTypeIoPort          = 1;
static const CP_PORT_ADDRESS_TYPE CpPortTypeMemoryMapped    = 2;

#pragma warning(disable : 4201)
typedef struct
{
    CP_PORT_ADDRESS_TYPE Type;
    union
    {
        UINT16 IoPort;
        EFI_PHYSICAL_ADDRESS MmioAddress;
    };
} CP_PORT_ADDRESS, *PCP_PORT_ADDRESS;
#pragma warning(default : 4201)

//
// Generic baud rates.
//
typedef UINT32 CP_BAUD_RATE, *PCP_BAUD_RATE;

static const CP_BAUD_RATE CpBrNone    = 0;
static const CP_BAUD_RATE CpBr150     = 150;
static const CP_BAUD_RATE CpBr300     = 300;
static const CP_BAUD_RATE CpBr600     = 600;
static const CP_BAUD_RATE CpBr1200    = 1200;
static const CP_BAUD_RATE CpBr2400    = 2400;
static const CP_BAUD_RATE CpBr4800    = 4800;
static const CP_BAUD_RATE CpBr9600    = 9600;
static const CP_BAUD_RATE CpBr14400   = 14400;
static const CP_BAUD_RATE CpBr19200   = 19200;
static const CP_BAUD_RATE CpBr57600   = 57600;
static const CP_BAUD_RATE CpBr115200  = 115200;

//
// Modem control.
//
typedef UINT32 CP_MODEM_CONTROL, *PCP_MODEM_CONTROL;

static const CP_MODEM_CONTROL CpMcIgnoreRings = 0;
static const CP_MODEM_CONTROL CpMcAnswerRings = 1;

//
// CP API return status.
//
typedef UINT32 CP_STATUS, *PCPSTATUS;

static const CP_STATUS CpStatusSuccess  = 0;
static const CP_STATUS CpStatusNoData   = 1;
static const CP_STATUS CpStatusError    = 2;
static const CP_STATUS CpStatusNotReady = 3;

//
// Traditional/legacy UART I/O adddresses.
//
#define COM1_PORT   0x03f8
#define COM2_PORT   0x02f8

//
// Include device specific header based on architecture
//
#if defined(_AMD64_)

#include "cpPcUart.h"

#elif defined(_ARM64_)

#include "cpPL011.h"

#else
#error unsupported architecture
#endif

//
// The Cp (Com Port) functions
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


