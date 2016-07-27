#include "UefiHavoc.h"

//
// todo MOVE to header
//

#pragma warning(disable:4214) // nonstandard extension used : bit field types other than int
#define SCSIOP_INQUIRY                  0x12
#define VPD_DEVICE_IDENTIFIERS             0x83

typedef struct _CDB6INQUIRY3 
{
    UCHAR OperationCode;    // 0x12 - SCSIOP_INQUIRY
    UCHAR EnableVitalProductData : 1;
    UCHAR CommandSupportData : 1;
    UCHAR Reserved1 : 6;
    UCHAR PageCode;
    UCHAR Reserved2;
    UCHAR AllocationLength;
    UCHAR Control;
} CDB6INQUIRY3;

typedef struct _VPD_IDENTIFICATION_PAGE {
    UCHAR DeviceType : 5;
    UCHAR DeviceTypeQualifier : 3;
    UCHAR PageCode;
    UCHAR Reserved;
    UCHAR PageLength;

    UCHAR Descriptors[1];
} VPD_IDENTIFICATION_PAGE, *PVPD_IDENTIFICATION_PAGE;


typedef struct _EFI_SCSI_LUN_INFO {
    EFI_EXT_SCSI_PASS_THRU_PROTOCOL*    ExtScsiPassThruProtocol;
    UINT8                               Target[TARGET_MAX_BYTES];
    UINT64                              Lun;
} EFI_SCSI_LUN_INFO;

typedef struct _EFI_VPD_PAGE_BUFFER {
    VPD_IDENTIFICATION_PAGE IdPage;
    UCHAR                   Buffer[128];
} EFI_VPD_PAGE_BUFFER;

typedef struct _EFI_INQUIRY_CALLBACK_CONTEXT {
    EFI_EXT_SCSI_PASS_THRU_SCSI_REQUEST_PACKET  Packet;
    CDB6INQUIRY3                                Cdb;
    EFI_VPD_PAGE_BUFFER                         Buffer;
    EFI_SCSI_LUN_INFO                           *LunInfo;    
    EFI_EVENT                                   Event;
} EFI_INQUIRY_CALLBACK_CONTEXT;



// Number of protocol handles to make buffer space for.
#define DEFAULT_PROTOCOL_HANDLES 4

// How many LUNs to send packets to
#define MAX_LUNS 8

// Default timeout on SCSI requests
#define SCSI_TIMEOUT 200

// How many packets to send concurrently to each LUN
#define PACKETS_PER_LUN 64

EFI_SCSI_LUN_INFO gEfiScsiLunInfoArray[MAX_LUNS];

UINTN NumLUNs;

UINTN gInquiriesIssued;

BOOLEAN gScsiCommandLoopStarted;

VOID
EfiScsiCopyTarget(
    IN OPTIONAL UINT8 InTarget[TARGET_MAX_BYTES],
    OUT UINT8 OutTarget[TARGET_MAX_BYTES]
    )
/*++

Routine Description:

    Copies the value of InTarget into OutTarget. If InTarget is NULL, sets OutTarget to 
    be all 0xff.

Arguments:

    InTarget - The source of the copy. Can be null.

    OutTarget - The target of the copy.

Return Value:

    None.
    
--*/
{
    UINTN byteIndex;

    for (byteIndex = 0; byteIndex < TARGET_MAX_BYTES; byteIndex++)
    {
        OutTarget[byteIndex] = (InTarget == NULL) ? 0xff : InTarget[byteIndex];
    }
}

VOID
EfiScsiInitCdb(
    IN CDB6INQUIRY3 *Cdb
    )
/*++

Routine Description:

    Initializes a SCSI inquiry cdb.

Arguments:

    Cdb - Cdb to initialize.

Return Value:

    None.
    
--*/
{
    Cdb->OperationCode = SCSIOP_INQUIRY;
    Cdb->EnableVitalProductData = 1;
    Cdb->PageCode = VPD_DEVICE_IDENTIFIERS;
    Cdb->AllocationLength = sizeof(EFI_VPD_PAGE_BUFFER);
}



EFI_STATUS
EfiScsiLoadProtocol()
/*++

Routine Description:

    Creates a VMBUS_PIPE instance if it can find a channel with matching interface type.

Arguments:

    None.
    
Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;
    EFI_HANDLE handleArray[DEFAULT_PROTOCOL_HANDLES];
    UINTN handleArraySize;
    UINTN numHandles;
    UINTN handleIndex;
    UINTN lunCount;
    UINT8 *targetPointer;

    gInquiriesIssued = 0;

    handleArraySize = sizeof(EFI_HANDLE) * DEFAULT_PROTOCOL_HANDLES;

    status = gBS->LocateHandle(ByProtocol,
                               &gEfiExtScsiPassThruProtocolGuid,
                               NULL,
                               &handleArraySize,
                               handleArray);

    if (EFI_ERROR(status))
    {
        Print(L"Failed to locate handle\n");
        goto Cleanup;
    }

    numHandles = handleArraySize / sizeof(EFI_HANDLE);
    lunCount = 0;                         

    for (handleIndex = 0; handleIndex < numHandles; handleIndex++)
    {
        status = gBS->HandleProtocol(handleArray[handleIndex],
                                     &gEfiExtScsiPassThruProtocolGuid,
                                     (VOID **)&gEfiScsiLunInfoArray[lunCount].ExtScsiPassThruProtocol);

        if (EFI_ERROR(status))
        {
            Print(L"Failed to handle protocol\n");
            goto Cleanup;
        }

        EfiScsiCopyTarget(NULL, gEfiScsiLunInfoArray[lunCount].Target);

        while (lunCount < MAX_LUNS)
        {
            targetPointer = gEfiScsiLunInfoArray[lunCount].Target;
            status = gEfiScsiLunInfoArray[lunCount].ExtScsiPassThruProtocol->GetNextTargetLun(
                         gEfiScsiLunInfoArray[lunCount].ExtScsiPassThruProtocol,
                         &targetPointer,
                         &gEfiScsiLunInfoArray[lunCount].Lun);

            if (status == EFI_NOT_FOUND)
            {
                Print(L"LUN not found\n");
                status = EFI_SUCCESS;
                break;
            }
            else if (EFI_ERROR(status))
            {
                Print(L"Failed to get target LUN\n");
                goto Cleanup;
            }
            else
            {
                Print(L"LUN found\n");
                EfiScsiCopyTarget(gEfiScsiLunInfoArray[lunCount].Target,
                                  gEfiScsiLunInfoArray[lunCount+1].Target);

                gEfiScsiLunInfoArray[lunCount+1].Lun = gEfiScsiLunInfoArray[lunCount].Lun;
                gEfiScsiLunInfoArray[lunCount+1].ExtScsiPassThruProtocol = 
                    gEfiScsiLunInfoArray[lunCount].ExtScsiPassThruProtocol;
                lunCount++;
            }
        }
    }

    NumLUNs = lunCount;

Cleanup:
    
    return status;
}


EFI_STATUS
EfiScsiSendScsiInquiryCommand(
    IN EFI_SCSI_LUN_INFO                            *ScsiLunInfo,    
    IN EFI_EXT_SCSI_PASS_THRU_SCSI_REQUEST_PACKET   *Packet,
    IN EFI_EVENT                                    *Event,
    IN CDB6INQUIRY3                                 *Cdb,
    IN EFI_VPD_PAGE_BUFFER                          *DataBuffer,
    IN UINT32                                       DataBufferLength
    )
/*++

Routine Description:

    Sends a SCSI Inquiry command to the designated LUN. If Event is not NULL, then the
    command is asynchronous.

Arguments:

    ScsiLunInfo - Info about the LUN to which to send the inquiry command.

    PacketEvent - Structure containing the pre-allocated ScsiPacket and EFI Event.

    Cdb - Previously filled out CDB.

    DataBuffer - Previously allocated data buffer.

    DataBufferLength - Length of the data buffer.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status;

    Packet->Cdb = Cdb;
    Packet->CdbLength = sizeof(*Cdb);
    Packet->DataDirection = EFI_EXT_SCSI_DATA_DIRECTION_READ;
    Packet->HostAdapterStatus = 0;
    Packet->InDataBuffer = DataBuffer;
    Packet->InTransferLength = DataBufferLength;
    Packet->OutDataBuffer = NULL;
    Packet->OutTransferLength = 0;
    Packet->SenseData = NULL;
    Packet->SenseDataLength = 0;
    Packet->TargetStatus = 0;
    Packet->Timeout = SCSI_TIMEOUT;

    gInquiriesIssued++;

    status = ScsiLunInfo->ExtScsiPassThruProtocol->PassThru(ScsiLunInfo->ExtScsiPassThruProtocol,
                                                            ScsiLunInfo->Target,
                                                            ScsiLunInfo->Lun,
                                                            Packet,
                                                            *Event);

    return status;
}


//EFI_EVENT_NOTIFY EfiScsiReIssueInquiry;

VOID
EFIAPI 
EfiScsiReIssueInquiry(
    IN EFI_EVENT Event,
    IN VOID *Context
)
/*++

--*/
{
    EFI_INQUIRY_CALLBACK_CONTEXT *context = Context;
    EFI_STATUS status;

    UNREFERENCED_PARAMETER(Event);

    EfiScsiInitCdb(&context->Cdb);

    status = EfiScsiSendScsiInquiryCommand(context->LunInfo,
                                           &context->Packet,
                                           &context->Event,
                                           &context->Cdb,
                                           &context->Buffer,
                                           sizeof(EFI_VPD_PAGE_BUFFER));

    if (EFI_ERROR(status))
    {
        Print(L"Error issuing IO: ");
        Print(L"\n");
    }
}


EFI_STATUS
EfiScsiInquiryLoop()
/*++

Routine Description:

    Sends SCSI inquiry commands in a loop to every to up to four different LUNs. Does not
    wait for responses.

Arguments:

    None.

Return Value:

    EFI_STATUS.

--*/
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_INQUIRY_CALLBACK_CONTEXT *context;
    UINTN lunIndex;
    UINTN packetIndex;
    UINTN contextIndex;

    status = gBS->AllocatePool(EfiLoaderData, 
                               sizeof(*context) * MAX_LUNS * PACKETS_PER_LUN,
                               &context);

    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }


    for (packetIndex = 0; packetIndex < PACKETS_PER_LUN; packetIndex++)
    {
        for (lunIndex = 0; lunIndex < NumLUNs; lunIndex++)
        {
            contextIndex = packetIndex * NumLUNs + lunIndex;
            EfiScsiInitCdb(&context[contextIndex].Cdb);
            context[contextIndex].LunInfo = &gEfiScsiLunInfoArray[lunIndex];
            status = gBS->CreateEvent(EVT_NOTIFY_SIGNAL,
                                      TPL_CALLBACK,
                                      EfiScsiReIssueInquiry,
                                      &context[contextIndex],
                                      &context[contextIndex].Event);

            if (EFI_ERROR(status))
            {
                goto Cleanup;
            }
        }
    }
    
    for (packetIndex = 0; packetIndex < PACKETS_PER_LUN; packetIndex++)
    {
        for (lunIndex = 0; lunIndex < NumLUNs; lunIndex++)
        {
            contextIndex = packetIndex * NumLUNs + lunIndex;
            status = EfiScsiSendScsiInquiryCommand(context[contextIndex].LunInfo,
                                                   &context[contextIndex].Packet,
                                                   &context[contextIndex].Event,
                                                   &context[contextIndex].Cdb,
                                                   &context[contextIndex].Buffer,
                                                   sizeof(EFI_VPD_PAGE_BUFFER));

            if (EFI_ERROR(status))
            {
                goto Cleanup;
            }
        }
    }

Cleanup:
    
    return status;
}



EFI_SCSI_LUN_INFO gEfiScsiLunInfoArray[MAX_LUNS];

EFI_STATUS
EFIAPI
ScsiInquiryInit(
    )
{
    EFI_STATUS status;
    
    status = EfiScsiLoadProtocol();
    if (EFI_ERROR(status))
    {
        Print(L"Failed to load protocol");
        goto Cleanup;
    }

    gScsiCommandLoopStarted = FALSE;

Cleanup:

    return status;
}


EFI_STATUS
EFIAPI
ScsiInquiryHavoc(
    _In_  HAVOC_PLUGIN   *Plugin
    )
{
    EFI_STATUS status = EFI_SUCCESS;

    //
    // This Havoc Plugin sets up SCSI Inquiry Command operations that will
    // continually requeue themselves, so only spawn them on the first time
    // this function is called.
    //
    
    if (!gScsiCommandLoopStarted)
    {
        status = EfiScsiInquiryLoop();
        if (EFI_ERROR(status))
        {
            Print(L"Failed to spawn scsi inquiries");
            goto Cleanup;
        }

        gScsiCommandLoopStarted = TRUE;
    }

Cleanup:

    return status;
}
