/*++

Copyright (c) Microsoft Corporation

Module Name:

    EmclLib.h

Abstract:

    Utility functions for EMCL.

Author:

    Marius Buleandra (mariub) - 31 Jul 2012

--*/

#pragma once


EFI_STATUS
EFIAPI
EmclInstallProtocol (
    __in EFI_HANDLE ControllerHandle
    );

VOID
EFIAPI
EmclUninstallProtocol (
    __in EFI_HANDLE ControllerHandle
    );

EFI_STATUS
EFIAPI
EmclSendPacketSync (
    __in EFI_EMCL_PROTOCOL *This,
    __in_bcount(InlineBufferLength) VOID *InlineBuffer,
    __in UINT32 InlineBufferLength,
    __in_ecount(ExternalBufferCount) EFI_EXTERNAL_BUFFER *ExternalBuffers,
    __in UINT32 ExternalBufferCount
    );

EFI_STATUS
EmclChannelTypeSupported (
    __in EFI_HANDLE ControllerHandle,
    __in const EFI_GUID *ChannelType,
    __in EFI_HANDLE AgentHandle
    );

EFI_STATUS
EmclChannelTypeAndInstanceSupported (
    __in EFI_HANDLE ControllerHandle,
    __in const EFI_GUID *ChannelType,
    __in EFI_HANDLE AgentHandle,
    __in_opt const EFI_GUID *ChannelInstance
    );