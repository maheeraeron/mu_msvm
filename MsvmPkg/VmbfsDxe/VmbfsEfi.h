/** @file

  EFI simple file system protocol over vmbus.

  Copyright (c) Microsoft Corporation.
  Licensed under the BSD-2-Clause-Patent license.

**/

#include <Uefi.h>
#include <EfiNt.h>

#include <Protocol/Emcl.h>
#include <Protocol/Vmbus.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/FileInfo.h>

#include <Library/BaseMemoryLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SynchronizationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/EmclLib.h>

#include <VmbusFileSystem.h>

#define VMBFS_BAD_HOST ASSERT(FALSE)

#define GetPacketBuffer(fileInformation, Type) ((Type*)((fileInformation)->FileSystem->FileSystemInformation.PacketBuffer))
#define GetPacketSize(fileInformation) (((fileInformation)->FileSystem->FileSystemInformation.PacketSize))
#define GetFileSystemInformation(fileInformation) (&((fileInformation)->FileSystem->FileSystemInformation))
#define GetThisFileSystemInformation(SimpleFSProtocol) (&(((PVMBFS_SIMPLE_FILE_SYSTEM_PROTOCOL)(SimpleFSProtocol))->FileSystemInformation))
#define GetThisEfiFileSystemInfo(SimpleFSProtocol) (&(((PVMBFS_SIMPLE_FILE_SYSTEM_PROTOCOL)(SimpleFSProtocol))->EfiFileSystemInfo))

#define GetThisFileInformation(EfiFileProtocol) (&(((PVMBFS_FILE)(EfiFileProtocol))->FileInformation))
#define GetThisEfiFileInfo(EfiFileProtocol) (&(((PVMBFS_FILE)(EfiFileProtocol))->EfiFileInfo))

//
// Choose a maximum size that is known to fit in a VMBus pipe message.
//
#define VMBFS_MAXIMUM_RDMA_SIZE (7 * 1024 * 1024)

typedef struct _FILESYSTEM_INFORMATION {
    EFI_DEVICE_PATH_PROTOCOL *DevicePathProtocol;
    EFI_EMCL_PROTOCOL *EmclProtocol;
    INTN ReferenceCount;
    EFI_EVENT ReceivePacketEvent;
    UINT8 *PacketBuffer;
    UINT32 PacketSize;
    SPIN_LOCK VmbusIoLock;
} FILESYSTEM_INFORMATION, *PFILESYSTEM_INFORMATION;

typedef struct _VMBFS_SIMPLE_FILE_SYSTEM_PROTOCOL {
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL EfiSimpleFileSystemProtocol;
    FILESYSTEM_INFORMATION FileSystemInformation;
    EFI_FILE_SYSTEM_INFO EfiFileSystemInfo;
} VMBFS_SIMPLE_FILE_SYSTEM_PROTOCOL, *PVMBFS_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef struct _FILE_INFORMATION {
    BOOLEAN IsDirectory;
    BOOLEAN RdmaCapable;
    PVMBFS_SIMPLE_FILE_SYSTEM_PROTOCOL FileSystem;
    UINT64 FileOffset;
    SIZE_T FilePathLength;
} FILE_INFORMATION, *PFILE_INFORMATION;

typedef struct _VMBFS_FILE {
    EFI_FILE_PROTOCOL EfiFileProtocol;
    FILE_INFORMATION FileInformation;
    EFI_FILE_INFO EfiFileInfo;
} VMBFS_FILE, *PVMBFS_FILE;


extern const EFI_FILE_PROTOCOL gVmbFsEfiFileProtocol;
extern const EFI_SIMPLE_FILE_SYSTEM_PROTOCOL gVmbFsSimpleFileSystemProtocol;
extern const EFI_FILE_SYSTEM_INFO gVmbFsEfiFileSystemInfoPrototype;
extern const EFI_FILE_INFO gVmbFsEfiFileInfoPrototype;
UINTN gEventIndexDiscarded;

//
// Driver protocol implementation.
//
EFI_STATUS
EFIAPI
VmbfsStart (
    _In_ EFI_DRIVER_BINDING_PROTOCOL *This,
    _In_ EFI_HANDLE ControllerHandle,
    _In_opt_ EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
    );

//
// File System protocol implementation.
//
EFI_STATUS
EFIAPI
VmbfsOpenVolume (
    _In_ EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,
    _Out_ EFI_FILE_PROTOCOL **Root
    );

VOID
VmbfsCloseVolume (
    _In_ PVMBFS_SIMPLE_FILE_SYSTEM_PROTOCOL VmbfsSimpleFileSystemProtocol,
    _In_ BOOLEAN ChannelOpened
    );

//
// File protocol implementation.
//
VOID
VmbfsReceivePacketCallback (
    __in VOID *ReceiveContext,
    __in VOID *PacketContext,
    __in_bcount_opt(BufferLength) VOID *Buffer,
    __in UINT32 BufferLength,
    __in UINT16 TransferPageSetId,
    __in UINT32 RangeCount,
    __in_ecount(RangeCount) EFI_TRANSFER_RANGE *Ranges
    );

EFI_STATUS
VmbfsSendReceivePacket (
    __in PFILESYSTEM_INFORMATION FileSystemInformation,
    __in_bcount_opt(BufferLength) VOID *Buffer,
    __in UINTN BufferLength,
    __in UINT32 GpaRangeHandle,
    __in_bcount(ExternalBufferLength) VOID *ExternalBuffer,
    __in UINTN ExternalBufferLength,
    __in BOOLEAN IsWritable
    );

EFI_STATUS
EFIAPI
VmbfsOpen (
    _In_ EFI_FILE_PROTOCOL *This,
    _Out_ EFI_FILE_PROTOCOL **NewHandle,
    _In_ CHAR16 *FileName,
    _In_ UINT64 OpenMode,
    _In_ UINT64 Attributes
    );

EFI_STATUS
EFIAPI
VmbfsClose (
    _In_ EFI_FILE_PROTOCOL *This
    );

EFI_STATUS
EFIAPI
VmbfsRead (
    _In_ EFI_FILE_PROTOCOL *This,
    _Inout_ UINTN *BufferSize,
    _Out_writes_bytes_(BufferSize) VOID *Buffer
    );

EFI_STATUS
EFIAPI
VmbfsWrite (
    _In_ EFI_FILE_PROTOCOL *This,
    _Inout_ UINTN *BufferSize,
    _In_reads_bytes_(BufferSize) PVOID Buffer
    );

EFI_STATUS
EFIAPI
VmbfsGetPosition (
    _In_ EFI_FILE_PROTOCOL *This,
    _Out_ UINT64 *Position
    );

EFI_STATUS
EFIAPI
VmbfsSetPosition (
    _In_ EFI_FILE_PROTOCOL *This,
    _In_ UINT64 Position
    );

EFI_STATUS
EFIAPI
VmbfsGetInfo (
    _In_ EFI_FILE_PROTOCOL *This,
    _In_ EFI_GUID *InformationType,
    _Inout_ UINTN *BufferSize,
    _Out_ VOID *Buffer
    );

EFI_STATUS
EFIAPI
VmbfsSetInfo (
    _In_ EFI_FILE_PROTOCOL *This,
    _In_ EFI_GUID *InformationType,
    _Inout_ UINTN *BufferSize,
    _In_ VOID *Buffer
    );

EFI_STATUS
EFIAPI
VmbfsFlush (
  _In_ EFI_FILE_PROTOCOL *This
  );

EFI_STATUS
EFIAPI
VmbfsDelete (
  _In_ EFI_FILE_PROTOCOL *This
  );

