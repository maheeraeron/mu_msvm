/*++

Copyright (c) Microsoft Corporation

Module Name:

    cpPcUart.c

Abstract:

    This module implements a very simply package to do com I/O on machines
    with standard PC 8250/15550 style UART COM ports.

    Adapted from earlier versions of the Windows boot debugger.

--*/
#include <Library/IoLib.h>
#include "cp.h"
#include "Bd.h"

#define FlagOn(_F,_SF) (!!((_F) & (_SF)))

UINT8 CpLastError;

UINT8
CppPortReadRegister8(
    __in PCP_PORT Port,
    __in UINT8 RegisterNumber
    )
/*++

Routine Description:

    This routine reads a byte from a COM port hardware register.

Arguments:

    Port - Supplies a pointer to the COM port object.

    RegisterNumber - Supplies the zero-based number of the COM port register
        to access.

Return Value:

    The 8bit value from the COM port register.

--*/
{
    if (Port->Address.Type == CpPortTypeIoPort)
    {
        return IoRead8(Port->Address.IoPort + RegisterNumber);
    }

    return 0;
}

VOID
CppPortWriteRegister8(
    __in PCP_PORT Port,
    __in UINT8 RegisterNumber,
    __in UINT8 Value
    )
/*++

Routine Description:

    This routine writes a byte to a COM port hardware register.

Arguments:

    Port - Supplies a pointer to the COM port object.

    RegisterNumber - Supplies the zero-based number of the COM port register
        to access.

    Value - Supplies the byte to write to the COM port.

Return Value:

    None.

--*/
{
    if (Port->Address.Type == CpPortTypeIoPort)
    {
        IoWrite8(Port->Address.IoPort + RegisterNumber, Value);
    }
}

VOID
CppCheckPowerButton(
    VOID
    )
/*++

Routine Description:

    This routine checks to see if the machine is trying to be powered down.

Arguments:

    None.

Return Value:

    None.

--*/
{
    //
    // Not relevant in a VM.
    //
    return;
}

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
        CpMcIgnoreRings.

Return Value:

    None.

--*/
{
    Port->Address = *Address;
    Port->BaudRate = CpBrNone;
    Port->RingHistory = 0;
    Port->LastLsr = 0;
    Port->LastMsr = 0;

    if (ModemControl == CpMcAnswerRings)
    {
        Port->Flags = PORT_ANSWER_MODEM;
    }
    else
    {
        Port->Flags = 0;
    }

    CppPortSetBaud(Port, BaudRate);
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
    UINT8 lsr;
    UINT8 msr;

    //
    // If modem control, make sure DSR, CTS and CD are all set before
    // sending any data.
    //

    while (Port->Flags & PORT_MODEMCONTROL)
    {
        msr = CppPortReadRegister8(Port, COM_MSR);

        if ((msr & MS_DSRCTSCD) == MS_DSRCTSCD)
        {
            break;
        }

        //
        // If no CD, and there's a charactor ready, eat it
        //
        lsr = CppPortReadLsr(Port, 0);

        if (((msr & MS_CD) == 0) && ((lsr & COM_DATRDY) == COM_DATRDY))
        {
            CppPortReadLsr(Port, COM_DAT);
        }
    }

    //
    //  Wait for port to not be busy
    //

    if (BusyWait)
    {
        while (!(CppPortReadLsr(Port, COM_OUTRDY) & COM_OUTRDY))
        {
//            YieldProcessor();
        }
    }
    else
    {
        if (!(CppPortReadLsr(Port, COM_OUTRDY) & COM_OUTRDY))
        {
            return CpStatusNotReady;
        }
    }

    //
    // Send the byte
    //

    CppPortWriteRegister8(Port, COM_DAT, Byte);
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

    WaitTimeUs - Supplies the amount of time to wait in 100 nanosecs, or zero
        if the routine should merely probe for a byte and return.

Return Value:

    CpStatusSuccess if data returned.

    CpStatusNoData if no data available, but no error.

    CpStatusError if error (overrun, parity, etc.)

--*/
{
    UINT8 lsr;
    UINT8 value;
    UINT32 count;

    //
    //  Make sure DTR and CTS are set
    //

    //
    // Check to make sure the CPPORT we were passed has been initialized.
    // (The only time it won't be initialized is when the kernel debugger
    // is disabled, in which case we just return.)
    //

    if (Port->Address.Type == CpPortTypeUninitialized)
    {
        CppCheckPowerButton();
        return CpStatusNoData;
    }

    count = WaitForByte ? TIMEOUT_COUNT : 1;
    while (count != 0)
    {
        count -= 1;
        lsr = CppPortReadLsr(Port, COM_DATRDY);

        if (lsr == SERIAL_LSR_NOT_PRESENT)
        {
            return CpStatusNoData;
        }

        if ((lsr & COM_DATRDY) == COM_DATRDY)
        {

            //
            // Check for errors
            //

            if (lsr & (COM_FE | COM_PE | COM_OE))
            {
                CpLastError = lsr;
                *Byte = 0;

                CppPortReadRegister8(Port, COM_DAT);

                return CpStatusError;
            }

            //
            // Fetch the byte
            //

            value = CppPortReadRegister8(Port, COM_DAT);

            if (Port->Flags & PORT_MODEMCONTROL)
            {
                //
                // Using modem control.  If no CD, then skip this byte.
                //

                if ((CppPortReadRegister8(Port, COM_MSR) & MS_CD) == 0)
                {
                    continue;
                }
            }

            *Byte = value & (UCHAR)0xff;
            return CpStatusSuccess;
        }
    }

    Port->LastLsr = 0;
    CppPortReadLsr(Port, 0);
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
    UINT8 lsr = CppPortReadLsr(Port, COM_DATRDY);

    return (lsr != SERIAL_LSR_NOT_PRESENT) && FlagOn(lsr, COM_DATRDY);
}

// ------------------------------------------------------------ Local Functions

VOID
CppPortSetBaud(
    __inout PCP_PORT Port,
    __in CP_BAUD_RATE BaudRate
    )
/*++

Routine Description:

    This routine updates the baud rate for the COM port specified.

Arguments:

    Port - Supplies a pointer to the COM port object.

    BaudRate - Supplies the baud rate: CpBr57600, etc.

Return Value:

    None.

--*/
{
    UINT32 divisorlatch;
    UINT8 lcr;

    //
    // compute the divsor
    //

    divisorlatch = CLOCK_RATE / BaudRate;

    //
    // set the divisor latch access bit (DLAB) in the line control reg
    //

    lcr = CppPortReadRegister8(Port, COM_LCR);
    lcr |= LC_DLAB;
    CppPortWriteRegister8(Port, COM_LCR, lcr);

    //
    // set the divisor latch value.
    //

    CppPortWriteRegister8(Port, COM_DLM, HIGH8(divisorlatch));
    CppPortWriteRegister8(Port, COM_DLL, LOW8(divisorlatch));

    //
    // Set LCR to 8N1 (No parity, 8 data bits, 1 stop bit)
    //

    CppPortWriteRegister8(Port, COM_LCR, 3);

    //
    // Remember the baud rate
    //

    Port->BaudRate = BaudRate;
}

UINT8
CppPortReadLsr(
    __in PCP_PORT Port,
    __in UINT8 PolledField
    )
/*++

Routine Description:

    This routine reads the LSR byte from specified port.

Arguments:

    Port - Supplies a pointer to the COM port object.

    PolledField - Supplies the field which is being polled, or zero if no
        polling is underway. In the latter case, the modem status regiser
        will be monitored for rings as well.

Return Value:

    Byte read from the port.

--*/
{
    UINT8 lsr;
    UINT8 msr;

    lsr = CppPortReadRegister8(Port, COM_LSR);

    //
    // Check to see if the port still exists.
    //

    if (lsr == SERIAL_LSR_NOT_PRESENT)
    {
        return SERIAL_LSR_NOT_PRESENT;
    }

    if (lsr & PolledField)
    {
        Port->LastLsr = ~COM_DATRDY | (lsr & COM_DATRDY);
        return lsr;
    }

    msr = CppPortReadRegister8(Port, COM_MSR);

    if (!(Port->Flags & PORT_ANSWER_MODEM))
    {
        return lsr;
    }

    CppCheckPowerButton();

    if ((lsr == Port->LastLsr) && (msr == Port->LastMsr))
    {
        return lsr;
    }

    Port->RingHistory |= (msr & SERIAL_MSR_RI) ? 1 : 2;
    if (Port->RingHistory == 3)
    {
        //
        // The ring indicate line has toggled between both values at some
        // point. Switch to modem control from now on.
        //

        Port->RingHistory = 0;
        Port->Flags |= PORT_MODEMCONTROL | PORT_NOCDLTIME;
        Port->Flags &= ~PORT_MDM_CD;
    }

    Port->LastLsr = lsr;
    Port->LastMsr = msr;
    return lsr;
}
