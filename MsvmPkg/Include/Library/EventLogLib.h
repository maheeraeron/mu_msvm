/*++

Copyright (c) Microsoft Corporation

Module Name:

    EventLogLib.h

Abstract:

    Library wrapper around EFI_EVENTLOG_PROTOCOL

Author:

    Kris Harper (kharp) - 11-Dec-2013

--*/

#pragma once
#include <Protocol/EventLog.h>

EFI_STATUS
EFIAPI
EventLogChannelCreate(
    _In_        const EFI_GUID         *Channel,
    _In_opt_    EVENT_CHANNEL_INFO     *Attributes,
    _Out_opt_   EFI_HANDLE             *Handle
    );


EFI_STATUS
EFIAPI
EventLogChannelOpen(
    _In_        const EFI_GUID         *Channel,
    _Out_opt_   EFI_HANDLE             *Handle
    );


EFI_STATUS
EFIAPI
EventLog(
    _In_        const EFI_HANDLE        Channel,
    _In_        UINT32                  Flags,
    _In_        const UINT32            EventId,
    _In_        const UINT32            DataSize,
    _In_opt_    const VOID             *Data
    );


EFI_STATUS
EFIAPI
EventLogPendingGet(
    _In_        const EFI_HANDLE        Channel,
    _Out_       EFI_EVENT_DESCRIPTOR   *Metadata,
    _Outptr_result_bytebuffer_(Metadata->DataSize)
                VOID                  **Data
    );


EFI_STATUS
EFIAPI
EventLogPendingCommit(
    _In_        const EFI_HANDLE        Channel
    );


EFI_STATUS
EFIAPI
EventLogFlush(
    _In_        const EFI_HANDLE        Channel
    );


EFI_STATUS
EFIAPI
EventLogReset(
    _In_        const EFI_HANDLE        Channel
    );


EFI_STATUS
EFIAPI
EventLogStatistics(
    _In_        const EFI_HANDLE            Channel,
    _Out_       EVENT_CHANNEL_STATISTICS   *Stats
    );


typedef
BOOLEAN
(EFIAPI *EFI_EVENTLOG_ENUMERATE_CALLBACK)(
    _In_        VOID                               *Context,
    _In_        const EFI_EVENT_DESCRIPTOR         *Metadata,
    _In_bytecount_(Metadata->DataSize)
                const VOID                         *Event
    );


EFI_STATUS
EFIAPI
EventLogEnumerate(
    _In_        const EFI_HANDLE                    Channel,
    _In_        EFI_EVENTLOG_ENUMERATE_CALLBACK     Callback,
    _In_        const VOID                         *Context
    );