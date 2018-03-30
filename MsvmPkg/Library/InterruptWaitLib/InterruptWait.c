#include <Uefi.h>
#include <EfiNt.h>

#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Protocol/EventStatus.h>

EFI_EVENT_STATUS_PROTOCOL    *mEventStatus = NULL;


EFI_STATUS
EFIAPI
WaitForInterrupt (
  IN UINTN        NumberOfEvents,
  IN EFI_EVENT    *UserEvents,
  OUT UINTN       *UserIndex
  )
{
  EFI_STATUS      Status;
  UINTN           Index;
  UINTN           EventPendingStatus;
  EFI_TPL         CurrentTpl;
  EFI_EVENT       IdleLoopEvent = NULL;

  //
  // get Event Status protocol
  //
  if (mEventStatus == NULL) {
    Status = gBS->LocateProtocol(&gEfiEventStatusProtocolGuid, NULL, (VOID **)&mEventStatus);
    if (EFI_ERROR(Status)) {
        mEventStatus = NULL;
    }
  }

  if (NumberOfEvents == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (UserEvents == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  for(;;) {
    for(Index = 0; Index < NumberOfEvents; Index++) {

      Status = gBS->CheckEvent (UserEvents[Index]);

      //
      // provide index of event that caused problem
      //
      if (Status != EFI_NOT_READY) {
        if (UserIndex != NULL) {
          *UserIndex = Index;
        }
        return Status;
      }
    }

    if (mEventStatus != NULL) {
      DisableInterrupts();

      Status = mEventStatus->GetEventStatus(mEventStatus, 
                                            &EventPendingStatus, 
                                            &CurrentTpl,
                                            &IdleLoopEvent);

      if (EFI_ERROR(Status) || EventPendingStatus != 0) {
        EnableInterrupts();
      }
      else {
        EnableInterruptsAndSleep();
      }
    }
  }
}
