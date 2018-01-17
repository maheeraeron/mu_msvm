/*++

Copyright (c) Microsoft Corporation

Module Name:

    cpPL011.c

Abstract:

    This module implements a very simply package to do COM port I/O
    on the Hyper-V emulated ARM PL011 UART.

    Adapted from earlier versions of the Windows boot debugger.

--*/

#include "cp.h"
#include "Bd.h"
#include <Library/SerialPortLib.h>
#include <Drivers/PL011Uart.h>

VOID
CpPortInit(
    __out PCP_PORT Port,
    __in PCP_PORT_ADDRESS Address,
    __in CP_BAUD_RATE BaudRate,
    __in CP_MODEM_CONTROL ModemControl
    )
/*++

Routine Description:

    This routine populates the com port object and initializes the underlying
    port (set baud rate, enable port) as well.

Arguments:

    Port - Supplies a pointer to memory that will be initialized as a com port
        object.

    Address - Supplies the port address of the com port.

    BaudRate - Supplies the baud rate: CpBr57600, etc.

    ModemControl - Supplies modem control options, either CpMcAnswerRings or
        CpMcIgnoreRings.  Ignored in this implementation.

Return Value:

    None.

--*/
{
    UINT64              baudRate;
    UINT32              receiveFifoDepth;
    EFI_PARITY_TYPE     parity;
    UINT8               dataBits;
    EFI_STOP_BITS_TYPE  stopBits;

    //
    // Init the CP_PORT context.
    //

    Port->Address = *Address;
    Port->BaudRate = BaudRate;

    //
    // Init the UART
    //

    baudRate = BaudRate;
    receiveFifoDepth = 0;         // Use default FIFO depth
    parity = 1;
    dataBits = 1;
    stopBits = 1;

    PL011UartInitializePort (
             Port->Address.MmioAddress,
             24000000,
             &baudRate,
             &receiveFifoDepth,
             &parity,
             &dataBits,
             &stopBits
             );

}


CP_STATUS
CpPortWrite(
    __in PCP_PORT Port,
    __in UINT8 Byte,
    __in BOOLEAN BusyWait
    )
/*++

Routine Description:

    This routine writes a byte out the specified port.

Arguments:

    Port - Supplies a pointer to the COM port object.

    Byte - Supplies the value to write out the COM port.

    BusyWait - Supplies a flag to control whether this routine will
               busy wait for COM port to be ready to write.

Return Value:

    CP_STATUS

--*/
{

    if (0 == PL011UartWrite(Port->Address.MmioAddress, &Byte, 1))
    {
        return CpStatusError;
    }
    return CpStatusSuccess;
}


CP_STATUS
CpPortRead(
    __in PCP_PORT Port,
    __out PUINT8 Byte,
    __in BOOLEAN WaitForByte
    )
/*++

Routine Description:

    This routine reads a byte from the COM port.

Arguments:

    Port - Supplies a pointer to the COM port object.

    Byte - Supplies a pointer to the variable that will receive the byte read
        from the COM port.

    WaitForByte - If TRUE, retry getting a byte for an arbitrarily long time.
                  If FALSE, try once to get a byte from the port.

Return Value:

    CpStatusSuccess if data returned.

    CpStatusNoData if no data available, but no error.

    CpStatusError if error (overrun, parity, etc.)

--*/
{
    UINT32 limit;
    UINT32 try;

    if (Port->Address.Type == CpPortTypeUninitialized)
    {
        return CpStatusNoData;
    }

    limit = WaitForByte ? TIMEOUT_COUNT : 1;

    for (try = 1; try <= limit; try++)
    {
        if (PL011UartPoll(Port->Address.MmioAddress))
        {
            PL011UartRead(Port->Address.MmioAddress, Byte, 1);
            if (try > 1)
            {
                BdSerialPrint("CpPortRead: success after %lu tries\n", try);
            }
            return CpStatusSuccess;
        }
    }

    return CpStatusNoData;
}


BOOLEAN
CpPortDataReady(
    __in PCP_PORT Port
    )
/*++

Routine Description:

    This routine returns back if data is available on the serial
    port

Arguments:

    Port - Supplies a pointer to the COM port object.


Return Value:

    BOOLEAN

--*/
{
    return PL011UartPoll(Port->Address.MmioAddress);
}


