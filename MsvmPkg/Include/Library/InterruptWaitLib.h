#pragma once

EFI_STATUS
EFIAPI
WaitForInterrupt (
  IN UINTN        NumberOfEvents,
  IN EFI_EVENT    *UserEvents,
  OUT UINTN       *UserIndex
  );
