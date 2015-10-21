/*++

Copyright (c) Microsoft Corporation

Module Name:

    StatusCode.c

Abstract:

    Status code driver.  Implements the EFI_STATUS_CODE_PROTOCOL and logs
    events to an event log channel.

Author:

    Kris Harper (kharp) - 12-Dec-2013

ATTENTION - THIS FILE CONTAINS THIRD PARTY OPEN SOURCE CODE: 
    IntelFrameworkModulePkg\Universal\StatusCode\RuntimeDxe\StatusCodeRuntimeDxe.c

IT IS CLEARED ONLY FOR LIMITED USE BY WINDOWS CORE HYPER-V FOR THE HYPER-V ROLE
IN THE WINDOWS PRODUCT. DO NOT USE OR SHARE THIS CODE WITHOUT APPROVAL PURSUANT
TO THE MICROSOFT OPEN SOURCE SOFTWARE APPROVAL POLICY.

Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

--*/

#include "EventLogDxe.h"
#include "StatusCode.h"
#include "EventLogger.h"

#include <Guid/MemoryStatusCodeRecord.h>
#include <Protocol/StatusCode.h>
#include <Library/HobLib.h>
#include <Library/UefiLib.h>


EFI_STATUS
EFIAPI
ReportStatusCode(
  _In_      EFI_STATUS_CODE_TYPE     CodeType,
  _In_      EFI_STATUS_CODE_VALUE    Value,
  _In_      UINT32                   Instance,
  _In_opt_  EFI_GUID                 *CallerId,
  _In_opt_  EFI_STATUS_CODE_DATA     *Data
  );

const EFI_STATUS_CODE_PROTOCOL  mEfiStatusCodeProtocol  = 
{
    ReportStatusCode
};

//
// GUID for status code event channel.
//
extern EFI_GUID gStatusCodeEventChannelGuid;

EFI_HANDLE  mEfiStatusCodeEventHandle;

//
// CallerId field is valid.
//
#define EFI_STATUS_EVENT_HAS_CALLER_GUID    0x00000001
//
// Data field is valid.
//
#define EFI_STATUS_EVENT_HAS_DATA           0x00000002

//
// Status code event log entry.
//
typedef struct
{
    UINT32                  Flags;
    EFI_STATUS_CODE_VALUE   Value;
    UINT32                  Instance;
    EFI_GUID                CallerId;
    EFI_STATUS_CODE_DATA    Data;
} EFI_STATUS_CODE_EVENT;


EFI_STATUS
EFIAPI
ReportStatusCode(
    _In_        EFI_STATUS_CODE_TYPE     CodeType,
    _In_        EFI_STATUS_CODE_VALUE    Value,
    _In_        UINT32                   Instance,
    _In_opt_    EFI_GUID                *CallerId,
    _In_opt_    EFI_STATUS_CODE_DATA    *Data
    )
/*++

Routine Description:

    This function implements EFI_STATUS_CODE_PROTOCOL.ReportStatusCode().
    It logs the status code and associated data to the status code event channel.

Arguments:

    CodeType        Indicates the type of status code being reported.

    Value           Describes the current status of a hardware or software entity.
                    This includes information about the class and subclass that is used to
                    classify the entity as well as an operation.

    Instance        The enumeration of a hardware or software entity within
                    the system. Valid instance numbers start with 1.

    CallerId        This optional parameter may be used to identify the caller.
                    This parameter allows the status code driver to apply different rules to
                    different callers.

    Data            This optional parameter may be used to pass additional data.

Return Value:

    EFI_SUCCESS      The function completed successfully
    EFI_DEVICE_ERROR The function should not be completed due to a device error.


--*/
{
    EFI_STATUS_CODE_EVENT localEvent;
    EFI_STATUS_CODE_EVENT *eventData = NULL;
    EFI_EVENT_DESCRIPTOR eventDesc;
    UINT32 size = 0;

    ASSERT(EfiGetCurrentTpl() <= TPL_NOTIFY);

    if ((Data != NULL) &&
        (Data->HeaderSize >= sizeof(EFI_STATUS_CODE_DATA)))
    {
        //
        // Subract out the size of the embedded EFI_STATUS_CODE_DATA
        // to avoid over-allocating. The HeaderSize field will be set to the
        // needed size.
        //
        size = (sizeof(EFI_STATUS_CODE_EVENT) - sizeof(EFI_STATUS_CODE_DATA)) +
               Data->HeaderSize +
               Data->Size;

        eventData = AllocateZeroPool(size);

        if (eventData != NULL)
        {
            CopyMem(&eventData->Data, Data, (Data->HeaderSize + Data->Size));
            eventData->Flags |= EFI_STATUS_EVENT_HAS_DATA;
        }
    }

    if (eventData == NULL)
    {
        //
        // No data was provided or allocating memory failed.
        // Fallback to the local event and drop the data.
        //
        ZeroMem(&localEvent, sizeof(localEvent));
        size  = sizeof(EFI_STATUS_CODE_EVENT);
        eventData = &localEvent;
    }

    if (CallerId != NULL)
    {
        CopyGuid(&eventData->CallerId, CallerId);
        eventData->Flags |= EFI_STATUS_EVENT_HAS_CALLER_GUID;
    }

    eventData->Instance = Instance;
    eventData->Value    = Value;

    ZeroMem(&eventDesc, sizeof(eventDesc));
    eventDesc.EventId  = CodeType;  // Use as EventId
    eventDesc.DataSize = size;

    EventLog(mEfiStatusCodeEventHandle,
        &eventDesc,
        eventData);

    if (eventData != &localEvent)
    {
        gBS->FreePool(eventData);
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
StatusCodeRuntimeInitialize()
/*++

Routine Description:

    Initializes the implementation of EFI_STATUS_CODE_PROTOCOL and creates an event channel
    for collecting status code events.
    If enabled, status codes saved during the PEI phase will be retrieved and logged.

Arguments:

    None.

Return Value:

    EFI_SUCCESS on success

--*/
{
    EVENT_CHANNEL_INFO                Attributes;
    EFI_PEI_HOB_POINTERS              Hob;
    MEMORY_STATUSCODE_PACKET_HEADER  *PacketHeader;
    MEMORY_STATUSCODE_RECORD         *Record;
    UINTN                             Index;
    UINTN                             MaxRecordNumber;
    EFI_HANDLE                        Handle;
    EFI_STATUS                        Status;

    DEBUG((DEBUG_INIT, "Initializing Status Code Event Channel\n"));
    //
    // Create the event channel for logging UEFI status codes.
    //
    Attributes.Flags      = EVENT_CHANNEL_OVERWRITE_RECORDS;
    Attributes.BufferSize = PcdGet32(PcdStatusCodeEventLogSize);
    Attributes.RecordSize = 0;
    Attributes.Tpl        = TPL_NOTIFY;

    Status = EventChannelCreate(&gStatusCodeEventChannelGuid, &Attributes, &mEfiStatusCodeEventHandle);

    if (EFI_ERROR(Status))
    {
        DEBUG((DEBUG_ERROR, "Failed to Create Status Code Event Channel. Error %08x\n", Status));
        ASSERT(FALSE);
        goto Exit;
    }

    //
    // Replay Status code entries which were logged during the PEI phase.
    // They are saved in a GUID HOB.
    //
    if (FeaturePcdGet(PcdStatusCodeReplayIn)) 
    {
        Hob.Raw = GetFirstGuidHob(&gMemoryStatusCodeRecordGuid);

        if (Hob.Raw != NULL) 
        {
            PacketHeader    = (MEMORY_STATUSCODE_PACKET_HEADER *) GET_GUID_HOB_DATA (Hob.Guid);
            MaxRecordNumber = (UINTN) PacketHeader->RecordIndex;
            Record          = (MEMORY_STATUSCODE_RECORD *) (PacketHeader + 1);

            if (PacketHeader->PacketIndex > 0) 
            {
                //
                // RecordIndex has wrapped around. The record count is
                // the maximum.
                //
                MaxRecordNumber = (UINTN) PacketHeader->MaxRecordsNumber;
            }

            //
            // FUTURE-2014-1-6-kharp If the PEI status code ring buffer overflowed,
            //  the buffer is not processed in order.
            //   start at index RecordIndex,
            //   go up and mask Index by the max size.
            //   stop after processing MaxRecordNumber
            //
            for (Index = 0; Index < MaxRecordNumber; Index++) 
            {
                ReportStatusCode(Record[Index].CodeType,
                    Record[Index].Value,
                    Record[Index].Instance,
                    NULL,  // PEI Phase events don't have a caller ID or extra data.
                    NULL);
            }
        }
    }

    //
    // Install Status Code Runtime Protocol implementation
    //
    Handle = NULL;

    Status = gBS->InstallMultipleProtocolInterfaces(
                  &Handle,
                  &gEfiStatusCodeRuntimeProtocolGuid,
                  &mEfiStatusCodeProtocol,
                  NULL);
    if (EFI_ERROR(Status))
    {
        DEBUG((DEBUG_ERROR, "Failed to Register Status Code Runtime Protocol. Error %08x\n", Status));
        ASSERT(FALSE);
    }

Exit:

    return Status;
}
