/*++

Copyright (c) Microsoft Corporation

Module Name:

    VideoDxe.h

Abstract:

    EFI Driver for Synthetic Video Controller.

Author:

    Bhanu Gogineni (bhanug) - 20-Sep-2012

--*/


#pragma once

#include <Uefi.h>
#include <PiDxe.h>
#include <EfiNt.h>

#include <Protocol/Vmbus.h>
#include <Protocol/Emcl.h>
#include <Protocol/GraphicsOutput.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/EmclLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/PcdLib.h>

#include <Guid/EventGroup.h>

#define BITS_PER_BYTE                   (8)
#define DEFAULT_SCREEN_BYTES_PER_PIXEL  (4)
#define DEFAULT_SCREEN_WIDTH            (1024)
#define DEFAULT_SCREEN_HEIGHT           (768)

typedef struct _RECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *PRECT;

#define BYTE UINT8

#include <SynthVidProtocol.h>
#include <VmBusPacketFormat.h>

#define VIDEODXE_VERSION 1
#define VIDEODXE_CONTEXT_SIGNATURE SIGNATURE_32('V','D','X','E')

#define VIDEO_DXE_MAX_PACKET_SIZE (512)

typedef struct _VIDEODXE_CONTEXT VIDEODXE_CONTEXT;

typedef struct _VIDEODXE_CONTEXT
{
    //
    // Device State
    //
    UINTN Signature;
    EFI_HANDLE Handle;
    EFI_EMCL_PROTOCOL *Emcl;
    BOOLEAN ChannelStarted;
    EFI_STATUS InitStatus;
    EFI_EVENT InitCompleteEvent;

    //
    // Produced Protocols
    //
    EFI_GRAPHICS_OUTPUT_PROTOCOL          GraphicsOutput;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE     Mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  ModeInfo;

} VIDEODXE_CONTEXT, *PVIDEODXE_CONTEXT;

#define VIDEODXE_CONTEXT_FROM_GRAPHICS_OUTPUT_THIS(a) \
    CR( \
        a, \
        VIDEODXE_CONTEXT, \
        GraphicsOutput, \
        VIDEODXE_CONTEXT_SIGNATURE \
        )


extern EFI_DRIVER_BINDING_PROTOCOL gVideoDxeDriverBinding;
extern EFI_COMPONENT_NAME2_PROTOCOL gVideoDxeComponentName2;
extern EFI_COMPONENT_NAME_PROTOCOL gVideoDxeComponentName;

EFI_STATUS
EFIAPI
VideoDxeDriverEntryPoint (
    __in EFI_HANDLE ImageHandle,
    __in EFI_SYSTEM_TABLE *SystemTable
    );

//
// EFI_DRIVER_BINDING_PROTOCOL functions, used to determine if this driver supports the controller,
// to start and stop the controller.
//

EFI_STATUS
EFIAPI
VideoDxeDriverBindingSupported(
    __in EFI_DRIVER_BINDING_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in_opt EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
    );

EFI_STATUS
EFIAPI
VideoDxeDriverBindingStart(
    __in EFI_DRIVER_BINDING_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in_opt EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
    );

EFI_STATUS
EFIAPI
VideoDxeDriverBindingStop(
    __in EFI_DRIVER_BINDING_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in UINTN NumberOfChildren,
    __in_ecount(NumberOfChildren) EFI_HANDLE *ChildHandleBuffer
    );

//
// EFI_COMPONENT_NAME_PROTOCOL and EFI_COMPONENT_NAME2_PROTOCOL functions.
// Used to get a user friendly string.
//

EFI_STATUS
EFIAPI
VideoDxeComponentNameGetDriverName(
    __in EFI_COMPONENT_NAME2_PROTOCOL *This,
    __in CHAR8 *Language,
    __out CHAR16 **DriverName
    );

EFI_STATUS
EFIAPI
VideoDxeComponentNameGetControllerName(
    __in EFI_COMPONENT_NAME2_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in_opt EFI_HANDLE ChildHandle,
    __in CHAR8 *Language,
    __out CHAR16 **ControllerName
    );

//
// Video Channel Functions.
//

EFI_STATUS
VideoChannelOpen(
    __in PVIDEODXE_CONTEXT Context
    );

VOID
VideoChannelClose(
    __in PVIDEODXE_CONTEXT Context
    );

EFI_STATUS
VideoChannelStartInitialize(
    __in PVIDEODXE_CONTEXT Context
    );

//
// EFI_GRAPHICS_OUTPUT_PROTOCOL Protocol functions.
//

EFI_STATUS
EFIAPI
VideoGraphicsOutputQueryMode(
  __in  EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
  __in  UINT32 ModeNumber,
  __in  UINTN *SizeOfInfo,
  __out EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info
  );

EFI_STATUS
EFIAPI
VideoGraphicsOutputSetMode(
  __in  EFI_GRAPHICS_OUTPUT_PROTOCOL * This,
  __in  UINT32 ModeNumber
  );

EFI_STATUS
EFIAPI
VideoGraphicsOutputBlt(
  __in  EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
  __in  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer, OPTIONAL
  __in  EFI_GRAPHICS_OUTPUT_BLT_OPERATION  BltOperation,
  __in  UINTN SourceX,
  __in  UINTN SourceY,
  __in  UINTN DestinationX,
  __in  UINTN DestinationY,
  __in  UINTN Width,
  __in  UINTN Height,
  __in  UINTN Delta
  );

