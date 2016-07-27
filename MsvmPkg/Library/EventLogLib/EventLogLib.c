/*++

Copyright (c) Microsoft Corporation

Module Name:

    EventLogLib.c

Abstract:

    Library wrapper around EFI_EVENTLOG_PROTOCOL

Author:

    Kris Harper (kharp) - 11-Dec-2013

--*/
#include <EfiNt.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/EventLogLib.h>
#include <Library/BootEventLogLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_EVENTLOG_PROTOCOL *mEventLogProtocol = NULL;

static
BOOLEAN
EventLogGetProtocol()
/*++

Routine Description:

    Attempts to locate and cache an instance of the EFI_EVENTLOG_PROTOCOL

Arguments:

    None.


Return Value:

    TRUE if the protocol is available
    FALSE if not.

--*/
{
    EFI_STATUS status;

    if ((mEventLogProtocol == NULL) &&
        (gBS != NULL) && 
        (gBS->LocateProtocol != NULL))
    {
        status = gBS->LocateProtocol(&gEfiEventLogProtocolGuid, NULL, &mEventLogProtocol);

        if (EFI_ERROR (status)) 
        {
            mEventLogProtocol = NULL;
        }
    }

    return (mEventLogProtocol != NULL);
}


EFI_STATUS
EFIAPI
EventLogConstructor(
    _In_    EFI_HANDLE                  ImageHandle,
    _In_    EFI_SYSTEM_TABLE           *SystemTable
    )
/*++

Routine Description:

    

Arguments:

    ImageHandle     Unused

    SystemTable     Unused


Return Value:

    EFI_SUCCESS

--*/
{
    EventLogGetProtocol();
    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
EventLogChannelCreate(
    _In_        const EFI_GUID         *Channel,
    _In_opt_    EVENT_CHANNEL_INFO     *Attributes,
    _Out_opt_   EFI_HANDLE             *Handle
    )
/*++

Routine Description:

    Wrapper around EFI_EVENTLOG_PROTOCOL.ChannelCreate

Arguments:

    Channel         GUID identifying the channel to create

    Attributes      Optional channel attributes.  If no channel attributes
                    are specified, the channel is opened and must already exist.

    Handle          Optional return handle representing the channel


Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status = EFI_NOT_READY;

    if (EventLogGetProtocol())
    {
        status = mEventLogProtocol->ChannelCreate(Channel, Attributes, Handle);
    }

    return status;
}


EFI_STATUS
EFIAPI
EventLogChannelOpen(
    _In_        const EFI_GUID         *Channel,
    _Out_opt_   EFI_HANDLE             *Handle
    )
/*++

Routine Description:

    Wrapper around EFI_EVENTLOG_PROTOCOL.ChannelCreate. This function will only
    open existing event channels. Use EventLogChannelCreate to create a new channel

Arguments:

    Channel         GUID identifying the channel to create

    Handle          Optional return handle representing the channel


Return Value:

    EFI_STATUS

--*/
{
    return EventLogChannelCreate(Channel, NULL, Handle);
}


EFI_STATUS
EFIAPI
EventLog(
    _In_        const EFI_HANDLE        Channel,
    _In_        UINT32                  Flags,
    _In_        const UINT32            EventId,
    _In_        const UINT32            DataSize,
    _In_opt_    const VOID             *Data
    )
/*++

Routine Description:

    Wrapper around EFI_EVENTLOG_PROTOCOL.EventLog

Arguments:

    Channel         Channel handle

    Flags           See EVENT_FLAG_nnnn

    EventId         Identifier for the event

    DataSize        Size of the optional data

    Data            Optional data


Return Value:

    EFI_STATUS.

--*/
{
    EFI_EVENT_DESCRIPTOR event;
    EFI_STATUS status = EFI_NOT_READY;

    if (EventLogGetProtocol())
    {
        ZeroMem(&event, sizeof(event));
        event.EventId  = EventId;
        event.DataSize = DataSize;
        event.Flags    = Flags;
        status = mEventLogProtocol->EventLog(Channel, &event, Data);
    }

    return status;
}


EFI_STATUS
EFIAPI
EventLogPendingGet(
    _In_        const EFI_HANDLE        Channel,
    _Out_       EFI_EVENT_DESCRIPTOR   *Metadata,
    _Outptr_result_bytebuffer_(Metadata->DataSize)
                VOID                  **Data
    )
/*++

Routine Description:

    Wrapper around EFI_EVENTLOG_PROTOCOL.EventPendingGet

Arguments:

    Channel         Channel handle

    Metadata        Returns the EFI_EVENT_DESCRIPTOR for the currently pending event

    Data            Returns a pointer to the pending event's data.
                    This can be modified in place.

Return Value:

    EFI_SUCCESS
    EFI_NOT_FOUND if no event is currently pending.

--*/
{
    EFI_STATUS status = EFI_NOT_READY;

    if (EventLogGetProtocol())
    {
        status = mEventLogProtocol->EventPendingGet(Channel, Metadata, Data);
    }

    return status;
}


EFI_STATUS
EFIAPI
EventLogPendingCommit(
    _In_        const EFI_HANDLE        Channel
    )
/*++

Routine Description:

    Wrapper around EFI_EVENTLOG_PROTOCOL.EventPendingCommit

Arguments:

    Channel     Channel Handle


Return Value:

    EFI_SUCCESS
    EFI_NOT_FOUND if there is no pending event.

--*/
{
    EFI_STATUS status = EFI_NOT_READY;

    if (EventLogGetProtocol())
    {
        status = mEventLogProtocol->EventPendingCommit(Channel);
    }

    return status;
}


EFI_STATUS
EFIAPI
EventLogFlush(
    _In_        const EFI_HANDLE        Channel
    )
/*++

Routine Description:

    Wrapper around EFI_EVENTLOG_PROTOCOL.ChannelFlush

Arguments:

    Channel     Channel handle


Return Value:

    EFI_STATUS

--*/
{
    EFI_STATUS status = EFI_NOT_READY;

    if (EventLogGetProtocol())
    {
        status = mEventLogProtocol->ChannelFlush(Channel);
    }

    return status;
}


EFI_STATUS
EFIAPI
EventLogReset(
    _In_        const EFI_HANDLE        Channel
    )
/*++

Routine Description:

    Wrapper around EFI_EVENTLOG_PROTOCOL.ChannelReset

Arguments:

    Channel     Channel handle


Return Value:

    EFI_STATUS

--*/
{
    EFI_STATUS status = EFI_NOT_READY;

    if (EventLogGetProtocol())
    {
        status = mEventLogProtocol->ChannelReset(Channel);
    }

    return status;
}


EFI_STATUS
EFIAPI
EventLogStatistics(
    _In_    const EFI_HANDLE            Channel,
    _Out_   EVENT_CHANNEL_STATISTICS   *Stats
    )
/*++

Routine Description:

    Wrapper around EFI_EVENTLOG_PROTOCOL.ChannelStatistics

Arguments:

    Channel     Channel handle

    Stats       On success contains the channel statistics


Return Value:

    EFI_STATUS

--*/
{
    EFI_STATUS status = EFI_NOT_READY;

    if (EventLogGetProtocol())
    {
        status = mEventLogProtocol->ChannelStatistics(Channel, Stats);
    }

    return status;
}


EFI_STATUS
EFIAPI
EventLogEnumerate(
    _In_    const EFI_HANDLE                    Channel,
    _In_    EFI_EVENTLOG_ENUMERATE_CALLBACK     Callback,
    _In_    const VOID                         *Context
    )
/*++

Routine Description:

    Wrapper around EFI_EVENTLOG_PROTOCOL.EventEnumerate
    The caller provided functions is called for each event record in the event channel.

Arguments:

    Channel     Channel handle

    Callback    Enumeration callback

    Context     Caller provided context to be given to the enumeration callback


Return Value:

    EFI_STATUS
    EFI_NOT_FOUND if the event channel is empty

--*/
{
    VOID            *eventData = NULL;
    EFI_EVENT_DESCRIPTOR eventDesc;
    EFI_HANDLE enumerator = NULL;
    EFI_STATUS status     = EFI_NOT_READY;
    BOOLEAN    keepGoing  = TRUE;

    //
    // FUTURE-2014-01-06-kharp  provided filtering capabilities either here on in the
    // protocol implementation.  Currently all callbacks need to perform their own
    // filtering if needed.
    //

    if (EventLogGetProtocol())
    {
        status = EFI_SUCCESS;   

        while ((!EFI_ERROR(status)) && (keepGoing))
        {
            status = mEventLogProtocol->EventEnumerate(Channel, &enumerator, &eventDesc, &eventData);

            if (!EFI_ERROR(status))
            {
                keepGoing = Callback((VOID*)Context, &eventDesc, eventData);
            }
        }

        if (status == EFI_END_OF_FILE)
        {
            status = EFI_SUCCESS;
        }
    }

    gBS->FreePool(enumerator);
    return status;
}
