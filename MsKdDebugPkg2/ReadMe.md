# MsKdDebugPkg

## Copyright

Copyright (c) 2016, Microsoft Corporation

All rights reserved. Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.

## About

MsKdDebugPkg enables the Microsoft Windows kernel debugger to connect to a UEFI
platform and perform common debug operations.

## Supported Architectures

This package is only compatible with ARM64 today, but the code was designed to
be easily expanded to any architecture.

## Components list and overview

1. KdDxe
2. KdProtocolLib
3. KdTransportLib

## KdDxe

KdDxe.efi is a DXE driver that performs the following actions:

1. Registers a synchronous exception handler and processes those callbacks.
2. Registers for periodic timer interrupts and processes those callbacks.
3. Registers for loaded image callbacks, which allows it to support symbols in
   the debugger.
4. Installs a KdDebugPrint protocol, which allows strings to be output on an
   attached kernel debugger.
5. Registers for EXIT_BOOT_SERVICES callbacks, which allows it to cleanup
   debugger configurations.
6. Initializes the KdProtocolLib.
7. Initializes the KdTransportLib.

## KdProtocolLib

KdProtocolLib is a library that provides support for communicating with an
attached kernel debugger.  This library understands the debugging state machine,
the packet formats, and it provides the data that the debugger is requesting.

## KdTransportLib

KdTransportLib is a library that provides the ability to send and receive data
over specific buses.  Today, this is limited to serial connections only.

## Packaging Changes

    ## Sample DSC changes
    Add the component
        MsKdDebugPkg2/KdDxe/KdDxe.inf 
    Add the library classes
        SourceDebugEnabledLib  |SourceLevelDebugPkg/Library/SourceDebugEnabled/SourceDebugEnabledLib.inf
        KdTransportLib         |MsKdDebugPkg2/Library/KdTransportLibSerial/KdTransportSerial.inf
        KdProtocolLib          |MsKdDebugPkg2/Library/KdProtocolLib/KdProtocolLib.inf
        SerialPortLib          |$(SILICON_PACKAGE)/Library/SerialPortLib/SerialPortShLib.inf

    ## Sample FDF/APRIORI_LIST.INC changes
    Add the following
        INF MsKdDebugPkg2/KdDxe/KdDxe.inf

        Note: KdDxe has dependencies on both CpuDxe and TimerDxe.  To maximize
              debugger coverage, ensure that KdDxe loads as soon after these 2 drivers
              as possible.

## Printing to the Debugger

    ## Sample C code:
    #include <Protocol/KdDebugPrint.h>
    KDDXE_PRINT_PROTOCOL *mKdDebugPrint = NULL;

    gBS->LocateProtocol (&gKdDebugPrintGuid, NULL, &mKdDebugPrint);
    mKdDebugPrint->DebugPrint((UINT8 *)Buffer, (UINT16)AsciiStrLen (Buffer));

    ## Sample INF updates
    [Packages]
      MsKdDebugPkg2/MsKdDebugPkg.dec
    [Protocols]
      gKdDebugPrintGuid ## CONSUMES
