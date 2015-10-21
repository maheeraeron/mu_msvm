/*++

Copyright (c) Microsoft Corporation

Module Name:

    EventLogger.h

Abstract:

    Defines types, constants, and function prototypes for event channels 

Author:

    Kris Harper (kharp) - 20-Nov-2013

--*/
#pragma once


EFI_STATUS
EFIAPI
EventLoggerInitialize();


EFI_STATUS
EFIAPI
EventChannelCreate(
    _In_        const EFI_GUID         *Channel,
    _In_opt_    EVENT_CHANNEL_INFO     *Attributes,
    _Out_opt_   EFI_HANDLE             *Handle
    );


EFI_STATUS
EFIAPI
EventChannelFlush(
    _In_    const EFI_HANDLE            Channel
    );


EFI_STATUS
EFIAPI
EventChannelReset(
    _In_    const EFI_HANDLE            Channel
    );


EFI_STATUS
EFIAPI
EventChannelStatistics(
    _In_    const EFI_HANDLE            Channel,
    _Out_   EVENT_CHANNEL_STATISTICS   *Stats
    );


EFI_STATUS
EFIAPI
EventEnumerate(
    _In_    const EFI_HANDLE            Channel,
    _Inout_ EFI_HANDLE                 *Enumerator,
    _Out_   EFI_EVENT_DESCRIPTOR       *Metadata,
    _Outptr_result_bytebuffer_(Metadata->DataSize)
            VOID                      **Event
    );


EFI_STATUS
EFIAPI
EventLog(
    _In_    const EFI_HANDLE            Channel,
    _In_    const EFI_EVENT_DESCRIPTOR *Event,
    _In_opt_bytecount_(Event->DataSize)
            const VOID                 *Data
    );


EFI_STATUS
EFIAPI
EventPendingGet(
    _In_    const EFI_HANDLE            Channel,
    _Out_   EFI_EVENT_DESCRIPTOR       *Metadata,
    _Outptr_result_bytebuffer_(Metadata->DataSize)
            VOID                      **Data
    );


EFI_STATUS
EFIAPI
EventPendingCommit(
    _In_    const EFI_HANDLE            Channel
    );
