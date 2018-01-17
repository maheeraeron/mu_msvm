/*++

Copyright (c) Microsoft Corporation

Module Name:

    cpPcUart.h

Abstract:

    Device specific header for typical/legacy PC Uart (8250/15550)

--*/

#pragma once

typedef struct _CP_PORT
{
    CP_PORT_ADDRESS Address;
    CP_BAUD_RATE    BaudRate;
} CP_PORT, *PCP_PORT;



