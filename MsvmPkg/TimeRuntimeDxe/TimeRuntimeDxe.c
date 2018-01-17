/*++

Copyright (c) Microsoft Corporation

Module Name:

    TimeRuntimeDxe.c

Abstract:

    Implements the UEFI side of the RTC used for AARCH64.

--*/

#include "TimeRuntimeDxe.h"

//
// RTC Protocol Handle
//
EFI_HANDLE mHandle                                   = NULL;

//
// Descriptor and Data buffers
//
static EFI_PHYSICAL_ADDRESS         mTimeBufferGpa   = 0;
static PVM_EFI_TIME                 mTimeBuffer      = NULL;

//
// Events this driver handles
//
static EFI_EVENT mVirtualAddressChangeEvent          = NULL;

EFI_STATUS
VmSetTime(
    _In_ EFI_TIME               *Time
    )
/*++

Routine Description:

    Set the current local time and date information, via MMIO to the Bios Vdev.

Arguments:

    Time - EFI_TIME to pass to the Bios Vdev.

Returns:

    EFI_STATUS:
        EFI_SUCCESS - successful time set.
        EFI_INVALID_PARAMETER - the supplied time was invalid.
        EFI_DEVICE_ERROR - some other error in the Bios Vdev.

--*/
{
    //
    // Copy parameters to buffer
    //
    CopyMem(&mTimeBuffer->Time, Time, sizeof(EFI_TIME));

    //
    // Send intercept to Bios Vdev to set time.
    //
    WriteBiosDevice(BiosConfigSetTime, (UINT32)mTimeBufferGpa);

    //
    // Return status set by Bios Vdev.
    //
    return mTimeBuffer->Status;
}

EFI_STATUS
VmGetTime(
    _Out_     EFI_TIME              *Time,
    _Out_opt_ EFI_TIME_CAPABILITIES *Capabilities
    )
/*++

Routine Description:

    Get the current local time and timezone information, from the Bios Vdev.

Arguments:

    Time - EFI_TIME to return the currrent time.

    Capabilities - Optional information about this RTC device.

Returns:

    EFI_STATUS:
        EFI_SUCCESS - successful time read.
        EFI_INVALID_PARAMETER - Time is NULL.
        EFI_DEVICE_ERROR - other error in the Bios Vdev.

--*/
{
    if (Time == NULL)
    {
        return EFI_INVALID_PARAMETER;
    }

    //
    // Send intercept to get the current time.
    //
    WriteBiosDevice(BiosConfigGetTime, (UINT32)mTimeBufferGpa);

    if (mTimeBuffer->Status != EFI_SUCCESS)
    {
        return mTimeBuffer->Status;
    }

    //
    // Copy time from Bios Vdev into caller struct.
    //
    CopyMem(Time, &mTimeBuffer->Time, sizeof(EFI_TIME));

    //
    // Report capabilities about our RTC device.
    //
    if (Capabilities != NULL)
    {
        //
        // TODO-cho: report same values as PCAT?
        //
        Capabilities->Resolution = 1;
        Capabilities->Accuracy   = 50000000;
        Capabilities->SetsToZero = FALSE;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
VmSetWakeupTime(
  _In_     BOOLEAN                Enable,
  _In_opt_ EFI_TIME               *Time
  )
/*++

Routine Description:

    Sets the system wakeup alarm clock time. Unsupported on AARCH64.

Arguments:

    Enable - Enable or disable the wakeup alarm.

    Time - The time to set the wakeup alarm for if enabled.

    Capabilities - Optional information about this RTC device.

Returns:

    EFI_UNSUPPORTED.

--*/
{
    return EFI_UNSUPPORTED;
}

EFI_STATUS
VmGetWakeupTime(
  _Out_ BOOLEAN               *Enabled,
  _Out_ BOOLEAN               *Pending,
  _Out_ EFI_TIME              *Time
  )
/*++

Routine Description:

    Returns the current system wakeup alarm clock time. Unsupported on AARCH64.

Arguments:

    Enabled - Indicates if the alarm is enabled.

    Pending - Is the alarm signal currently pending.

    Time - The current alarm time.

Returns:

    EFI_UNSUPPORTED.

--*/
{
    return EFI_UNSUPPORTED;
}

VOID
EFIAPI
TimeDxeAddressChangeHandler(
    _In_ EFI_EVENT Event,
    _In_ void*     Context
    )
{
    EFI_STATUS status;

    //
    // Physical addresses (GPAs) don't change. Get the new virtual address of
    // the time buffer.
    //
    status = EfiConvertPointer(0, (void**)&mTimeBuffer);
    ASSERT_EFI_ERROR(status);
}

EFI_STATUS
EFIAPI
InitializeTimeDxe(
  _In_ EFI_HANDLE       ImageHandle,
  _In_ EFI_SYSTEM_TABLE *SystemTable
  )
{
    EFI_STATUS status = EFI_SUCCESS;

    //
    // Allocate memory for Get/SetTime, under the 4GB boundry so 32 bit mmio
    // writes are ok.
    //
    #define BELOW_4GB (0xFFFFFFFFULL)
    mTimeBufferGpa = BELOW_4GB;
    status = gBS->AllocatePages(AllocateMaxAddress,
                                EfiRuntimeServicesData,
                                EFI_SIZE_TO_PAGES(sizeof(VM_EFI_TIME)),
                                &mTimeBufferGpa);
    if (EFI_ERROR(status))
    {
        mTimeBufferGpa = 0;
        goto Cleanup;
    }


    //
    // Addresses are identity mapped until runtime ie GVA == GPA.
    //
    mTimeBuffer = (PVM_EFI_TIME)mTimeBufferGpa;

    //
    // Install the time services into the system table.
    //
    SystemTable->RuntimeServices->SetTime = VmSetTime;
    SystemTable->RuntimeServices->GetTime = VmGetTime;
    SystemTable->RuntimeServices->SetWakeupTime = VmSetWakeupTime;
    SystemTable->RuntimeServices->GetWakeupTime = VmGetWakeupTime;

    //
    // Register a function to update address when page tables are changed.
    //
    status = gBS->CreateEventEx(EVT_NOTIFY_SIGNAL,
                                TPL_NOTIFY,
                                TimeDxeAddressChangeHandler,
                                NULL,
                                &gEfiEventVirtualAddressChangeGuid,
                                &mVirtualAddressChangeEvent);
    ASSERT_EFI_ERROR(status);
    if (EFI_ERROR(status))
    {
        goto Cleanup;
    }

    //
    // Install RTC Protocol on a new handle.
    //
    mHandle = NULL;
    status = gBS->InstallMultipleProtocolInterfaces(&mHandle,
                                                    &gEfiRealTimeClockArchProtocolGuid,
                                                    NULL,
                                                    NULL);
    ASSERT_EFI_ERROR(status);

Cleanup:

    if (EFI_ERROR(status))
    {
        if (mTimeBufferGpa != 0)
        {
            gBS->FreePages(mTimeBufferGpa,
                           EFI_SIZE_TO_PAGES(sizeof(VM_EFI_TIME)));
            mTimeBufferGpa = 0;
        }
    }

    return status;
}
