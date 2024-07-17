/** @file
  Provides Set/Get time operations.

Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2018 - 2020, ARM Limited. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DxeServicesTableLib.h>
#include "PcRtc.h"


PC_RTC_MODULE_GLOBALS  mModuleGlobal;

EFI_HANDLE             mHandle = NULL;

STATIC EFI_EVENT       mVirtualAddrChangeEvent;

UINTN                  mRtcIndexRegister;
UINTN                  mRtcTargetRegister;

// MS_HYP_CHANGE BEGIN

#include <IsolationTypes.h>

#if defined(MDE_CPU_AARCH64)

#include <Library/BiosDeviceLib.h>
#include <BiosInterface.h>

//
// The struct used to marshal EFI_TIME to the BiosDevice.
//
#pragma pack(push, 1)

typedef struct _VM_EFI_TIME
{
    EFI_STATUS Status;
    EFI_TIME   Time;
} VM_EFI_TIME, *PVM_EFI_TIME;

#pragma pack(pop)

//
// Descriptor and Data buffers
//
static EFI_PHYSICAL_ADDRESS         mTimeBufferGpa   = 0;
static PVM_EFI_TIME                 mTimeBuffer      = NULL;

#endif

BOOLEAN                mHardwareIsolatedWithNoParavisor = FALSE;

// MS_HYP_CHANGE END


/**
  Returns the current time and date information, and the time-keeping capabilities
  of the hardware platform.

  @param  Time          A pointer to storage to receive a snapshot of the current time.
  @param  Capabilities  An optional pointer to a buffer to receive the real time
                        clock device's capabilities.

  @retval EFI_SUCCESS            The operation completed successfully.
  @retval EFI_INVALID_PARAMETER  Time is NULL.
  @retval EFI_DEVICE_ERROR       The time could not be retrieved due to hardware error.

**/
EFI_STATUS
EFIAPI
PcRtcEfiGetTime (
  OUT EFI_TIME                *Time,
  OUT EFI_TIME_CAPABILITIES   *Capabilities  OPTIONAL
  )
{

  // MS_HYP_CHANGE BEGIN

  if (Time == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  if (mHardwareIsolatedWithNoParavisor)
  {
    //
    // Hardcode a zero value and return success here because the OS Loader will not
    // initialize if an error code is returned here.
    //
    Time->Second = RTC_INIT_SECOND;
    Time->Minute = RTC_INIT_MINUTE;
    Time->Hour   = RTC_INIT_HOUR;
    Time->Day    = RTC_INIT_DAY;
    Time->Month  = RTC_INIT_MONTH;
    Time->Year   = RTC_INIT_YEAR;
    Time->Nanosecond  = 0;
    Time->TimeZone = EFI_UNSPECIFIED_TIMEZONE;
    Time->Daylight = 0;
    return EFI_SUCCESS;
  }

#if defined(MDE_CPU_AARCH64)

  //
  // Send intercept to get the current time.
  //
  WriteBiosDevice(BiosConfigGetTime, (UINT32)mTimeBufferGpa);

  if (mTimeBuffer->Status != EFI_SUCCESS)
  {
    return mTimeBuffer->Status;
  }

  //
  // Copy time from BiosDevice into caller struct.
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

#else

  // MS_HYP_CHANGE END

  return PcRtcGetTime (Time, Capabilities, &mModuleGlobal);

#endif
}

/**
  Sets the current local time and date information.

  @param  Time                   A pointer to the current time.

  @retval EFI_SUCCESS            The operation completed successfully.
  @retval EFI_INVALID_PARAMETER  A time field is out of range.
  @retval EFI_DEVICE_ERROR       The time could not be set due due to hardware error.

**/
EFI_STATUS
EFIAPI
PcRtcEfiSetTime (
  IN EFI_TIME                *Time
  )
{
  // MS_HYP_CHANGE BEGIN

  if (Time == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  if (mHardwareIsolatedWithNoParavisor)
  {
    return EFI_UNSUPPORTED;
  }

#if defined(MDE_CPU_AARCH64)

  //
  // Copy parameters to buffer
  //
  CopyMem(&mTimeBuffer->Time, Time, sizeof(EFI_TIME));

  //
  // Send intercept to BiosDevice to set time.
  //
  WriteBiosDevice(BiosConfigSetTime, (UINT32)mTimeBufferGpa);

  //
  // Return status set by BiosDevice.
  //
  return mTimeBuffer->Status;

#else

  // MS_HYP_CHANGE END

  return PcRtcSetTime (Time, &mModuleGlobal);

#endif
}

/**
  Returns the current wakeup alarm clock setting.

  @param  Enabled  Indicates if the alarm is currently enabled or disabled.
  @param  Pending  Indicates if the alarm signal is pending and requires acknowledgement.
  @param  Time     The current alarm setting.

  @retval EFI_SUCCESS           The alarm settings were returned.
  @retval EFI_INVALID_PARAMETER Enabled is NULL.
  @retval EFI_INVALID_PARAMETER Pending is NULL.
  @retval EFI_INVALID_PARAMETER Time is NULL.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be retrieved due to a hardware error.
  @retval EFI_UNSUPPORTED       A wakeup timer is not supported on this platform.

**/
EFI_STATUS
EFIAPI
PcRtcEfiGetWakeupTime (
  OUT BOOLEAN     *Enabled,
  OUT BOOLEAN     *Pending,
  OUT EFI_TIME    *Time
  )
{
  // MS_HYP_CHANGE BEGIN

  if (mHardwareIsolatedWithNoParavisor)
  {
    return EFI_UNSUPPORTED;
  }

#if defined(MDE_CPU_AARCH64)

  return EFI_UNSUPPORTED;

#else

  // MS_HYP_CHANGE END

  return PcRtcGetWakeupTime (Enabled, Pending, Time, &mModuleGlobal);

#endif
}


/**
  Sets the system wakeup alarm clock time.

  @param  Enabled  Enable or disable the wakeup alarm.
  @param  Time     If Enable is TRUE, the time to set the wakeup alarm for.
                   If Enable is FALSE, then this parameter is optional, and may be NULL.

  @retval EFI_SUCCESS            If Enable is TRUE, then the wakeup alarm was enabled.
                                 If Enable is FALSE, then the wakeup alarm was disabled.
  @retval EFI_INVALID_PARAMETER  A time field is out of range.
  @retval EFI_DEVICE_ERROR       The wakeup time could not be set due to a hardware error.
  @retval EFI_UNSUPPORTED        A wakeup timer is not supported on this platform.

**/
EFI_STATUS
EFIAPI
PcRtcEfiSetWakeupTime (
  IN BOOLEAN      Enabled,
  IN EFI_TIME    *Time       OPTIONAL
  )
{
  // MS_HYP_CHANGE BEGIN

  if (mHardwareIsolatedWithNoParavisor)
  {
    return EFI_UNSUPPORTED;
  }

#if defined(MDE_CPU_AARCH64)

  return EFI_UNSUPPORTED;

#else

  // MS_HYP_CHANGE END

  return PcRtcSetWakeupTime (Enabled, Time, &mModuleGlobal);

#endif
}

/**
  Fixup internal data so that EFI can be called in virtual mode.
  Call the passed in Child Notify event and convert any pointers in
  lib to virtual mode.

  @param[in]    Event   The Event that is being processed
  @param[in]    Context Event Context
**/
VOID
EFIAPI
LibRtcVirtualNotifyEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  // Only needed if you are going to support the OS calling RTC functions in
  // virtual mode. You will need to call EfiConvertPointer (). To convert any
  // stored physical addresses to virtual address. After the OS transitions to
  // calling in virtual mode, all future runtime calls will be made in virtual
  // mode.
  EfiConvertPointer (0x0, (VOID**)&mRtcIndexRegister);
  EfiConvertPointer (0x0, (VOID**)&mRtcTargetRegister);

  // MS_HYP_CHANGE BEGIN

#if defined(MDE_CPU_AARCH64)

  //
  // Physical addresses (GPAs) don't change. Get the new virtual address of
  // the time buffer.
  //
  EfiConvertPointer(0x0, (VOID**)&mTimeBuffer);

#endif

  // MS_HYP_CHANGE END

}

/**
  The user Entry Point for PcRTC module.

  This is the entry point for PcRTC module. It installs the UEFI runtime service
  including GetTime(),SetTime(),GetWakeupTime(),and SetWakeupTime().

  @param  ImageHandle    The firmware allocated handle for the EFI image.
  @param  SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS    The entry point is executed successfully.
  @retval Others         Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
InitializePcRtc (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  EFI_STATUS Status;

  // MS_HYP_CHANGE BEGIN

  BOOLEAN registerAddressChangeHandler = FALSE;

  mHardwareIsolatedWithNoParavisor = IsHardwareIsolatedNoParavisor();

#if defined(MDE_CPU_AARCH64)
  //
  // Allocate memory for Get/SetTime, under the 4GB boundry so 32 bit mmio
  // writes are ok.
  //
  #define BELOW_4GB (0xFFFFFFFFULL)
  mTimeBufferGpa = BELOW_4GB;
  Status = gBS->AllocatePages(AllocateMaxAddress,
                              EfiRuntimeServicesData,
                              EFI_SIZE_TO_PAGES(sizeof(VM_EFI_TIME)),
                              &mTimeBufferGpa);
  if (EFI_ERROR(Status))
  {
    mTimeBufferGpa = 0;
    ASSERT_EFI_ERROR (Status);
    return Status;
  }


  //
  // Addresses are identity mapped until runtime ie GVA == GPA.
  //
  mTimeBuffer = (PVM_EFI_TIME)mTimeBufferGpa;
  registerAddressChangeHandler = TRUE;
#else

  // MS_HYP_CHANGE END

  EFI_EVENT Event;

  EfiInitializeLock (&mModuleGlobal.RtcLock, TPL_CALLBACK);
  mModuleGlobal.CenturyRtcAddress = GetCenturyRtcAddress ();

  if (FeaturePcdGet (PcdRtcUseMmio)) {
    mRtcIndexRegister = (UINTN)PcdGet64 (PcdRtcIndexRegister64);
    mRtcTargetRegister = (UINTN)PcdGet64 (PcdRtcTargetRegister64);
  }

  // Skip RTC device library init as none is present when isolated with no paravisor.
  if (!mHardwareIsolatedWithNoParavisor)
  {
    Status = PcRtcInit(&mModuleGlobal);
    ASSERT_EFI_ERROR(Status);

    Status = gBS->CreateEventEx(
        EVT_NOTIFY_SIGNAL,
        TPL_CALLBACK,
        PcRtcAcpiTableChangeCallback,
        NULL,
        &gEfiAcpi10TableGuid,
        &Event);
    ASSERT_EFI_ERROR(Status);

    Status = gBS->CreateEventEx(
        EVT_NOTIFY_SIGNAL,
        TPL_CALLBACK,
        PcRtcAcpiTableChangeCallback,
        NULL,
        &gEfiAcpiTableGuid,
        &Event);
    ASSERT_EFI_ERROR(Status);
  }


#endif

  gRT->GetTime       = PcRtcEfiGetTime;
  gRT->SetTime       = PcRtcEfiSetTime;
  gRT->GetWakeupTime = PcRtcEfiGetWakeupTime;
  gRT->SetWakeupTime = PcRtcEfiSetWakeupTime;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mHandle,
                  &gEfiRealTimeClockArchProtocolGuid,
                  NULL,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    goto Cleanup;
  }

  if (registerAddressChangeHandler || FeaturePcdGet (PcdRtcUseMmio)) {  // MS_HYP_CHANGE
    // Register for the virtual address change event
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    LibRtcVirtualNotifyEvent,
                    NULL,
                    &gEfiEventVirtualAddressChangeGuid,
                    &mVirtualAddrChangeEvent
                    );
    ASSERT_EFI_ERROR (Status);
  }

Cleanup:

  // MS_HYP_CHANGE BEGIN

#if defined(MDE_CPU_AARCH64)

  if (EFI_ERROR(Status))
  {
    if (mTimeBufferGpa != 0)
    {
      gBS->FreePages(mTimeBufferGpa,
                      EFI_SIZE_TO_PAGES(sizeof(VM_EFI_TIME)));
      mTimeBufferGpa = 0;
    }
  }

#endif

  // MS_HYP_CHANGE END

  return Status;
}
