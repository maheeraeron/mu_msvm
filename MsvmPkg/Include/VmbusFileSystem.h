/**
\copyright Copyright (c) Microsoft Corporation

\file VmbusFileSystem.h

\brief Implements the VMBus file system protocol.

\author Arseney Romanenko (arseneyr) 2014-10-10
*/
#pragma once
#pragma warning(push)
#pragma warning(disable:4200)

//
// Make sure padding works correctly for the flexible array members.
//

#pragma warning(error:4820)

#define GUID_VMBFS_INTERFACE_TYPE_DEFINE {0xc376c1c3, 0xd276, 0x48d2, \
    {0x90, 0xa9, 0xc0, 0x47, 0x48, 0x07, 0x2c, 0x60}}

#define GUID_VMBFS_IMC_INSTANCE_DEFINE {0xc4e5e7d1, 0xd748, 0x4afc, \
    {0x97, 0x9d, 0x68, 0x31, 0x67, 0x91, 0x0a, 0x55}}

#define GUID_VMBFS_BOOT_INSTANCE_DEFINE {0xc63c9bdf, 0x5fa5, 0x4208, \
    {0xb0, 0x3f, 0x6b, 0x45, 0x8b, 0x36, 0x55, 0x92}}

/* c376c1c3-d276-48d2-90a9-c04748072c60 */
DEFINE_GUID(GUID_VMBFS_INTERFACE_TYPE, 0xc376c1c3, 0xd276, 0x48d2, 0x90, 0xa9, 0xc0, 0x47, 0x48, 0x07, 0x2c, 0x60);

/* c4e5e7d1-d748-4afc-979d-683167910a55 */
DEFINE_GUID(GUID_VMBFS_IMC_INSTANCE, 0xc4e5e7d1, 0xd748, 0x4afc, 0x97, 0x9d, 0x68, 0x31, 0x67, 0x91, 0x0a, 0x55);

/* c63c9bdf-5fa5-4208-b03f-6b458b365592 */
DEFINE_GUID(GUID_VMBFS_BOOT_INSTANCE, 0xc63c9bdf, 0x5fa5, 0x4208, 0xb0, 0x3f, 0x6b, 0x45, 0x8b, 0x36, 0x55, 0x92);


#define VMBFS_MAXIMUM_MESSAGE_SIZE 12288
#define VMBFS_MAXIMUM_PAYLOAD_SIZE(_Header_) (VMBFS_MAXIMUM_MESSAGE_SIZE - sizeof(_Header_))

#define VMBFS_MAKE_VERSION(Major, Minor) ((UINT32)((Major) << 16) | (Minor))

#define VMBFS_VERSION_WIN10         VMBFS_MAKE_VERSION(1, 0)


typedef enum _VMBFS_MESSAGE_TYPE
{
    VmbfsMessageTypeInvalid = 0,
    VmbfsMessageTypeVersionRequest,
    VmbfsMessageTypeVersionResponse,
    VmbfsMessageTypeGetFileInfo,
    VmbfsMessageTypeGetFileInfoResponse,
    VmbfsMessageTypeReadFile,
    VmbfsMessageTypeReadFileResponse,
    VmbfsMessageTypeReadFileRdma,
    VmbfsMessageTypeReadFileRdmaResponse,

    VmbfsMessageTypeMax

} VMBFS_MESSAGE_TYPE, *PVMBFS_MESSAGE_TYPE;


#define VMBFS_GET_FILE_INFO_FLAG_DIRECTORY    0x1
#define VMBFS_GET_FILE_INFO_FLAG_RDMA_CAPABLE 0x2

#define VMBFS_GET_FILE_INFO_FLAGS (VMBFS_GET_FILE_INFO_FLAG_DIRECTORY | VMBFS_GET_FILE_INFO_FLAG_RDMA_CAPABLE)

#pragma pack(push)
#pragma pack(1)

typedef struct _VMBFS_MESSAGE_HEADER
{
    VMBFS_MESSAGE_TYPE Type;
    UINT32 Reserved;

} VMBFS_MESSAGE_HEADER, *PVMBFS_MESSAGE_HEADER;


typedef struct _VMBFS_MESSAGE_VERSION_REQUEST
{
    VMBFS_MESSAGE_HEADER Header;
    UINT32 RequestedVersion;

} VMBFS_MESSAGE_VERSION_REQUEST, *PVMBFS_MESSAGE_VERSION_REQUEST;


typedef enum _VMBFS_STATUS_VERSION_RESPONSE
{
    VmbfsVersionSupported = 0,
    VmbfsVersionUnsupported = 1

} VMBFS_STATUS_VERSION_RESPONSE, *PVMBFS_STATUS_VERSION_RESPONSE;


typedef struct _VMBFS_MESSAGE_VERSION_RESPONSE
{
    VMBFS_MESSAGE_HEADER Header;
    UINT32 Status;

} VMBFS_MESSAGE_VERSION_RESPONSE, *PVMBFS_MESSAGE_VERSION_RESPONSE;


typedef struct _VMBFS_MESSAGE_GET_FILE_INFO
{
    VMBFS_MESSAGE_HEADER Header;
    WCHAR FilePath[];

} VMBFS_MESSAGE_GET_FILE_INFO, *PVMBFS_MESSAGE_GET_FILE_INFO;


typedef enum _VMBFS_STATUS_FILE_RESPONSE
{
    VmbfsFileSuccess = 0,
    VmbfsFileNotFound = 1,
    VmbfsFileEndOfFile = 2,
    VmbfsFileError = 3

} VMBFS_STATUS_FILE_RESPONSE, *PVMBFS_STATUS_FILE_RESPONSE;


typedef struct _VMBFS_MESSAGE_GET_FILE_INFO_RESPONSE
{
    VMBFS_MESSAGE_HEADER Header;
    UINT32 Status;

    UINT32 Flags;
    UINT64 FileSize;

} VMBFS_MESSAGE_GET_FILE_INFO_RESPONSE, *PVMBFS_MESSAGE_GET_FILE_INFO_RESPONSE;


typedef struct _VMBFS_MESSAGE_READ_FILE
{
    VMBFS_MESSAGE_HEADER Header;
    UINT32 ByteCount;
    UINT64 Offset;
    WCHAR FilePath[];

} VMBFS_MESSAGE_READ_FILE, *PVMBFS_MESSAGE_READ_FILE;


typedef struct _VMBFS_MESSAGE_READ_FILE_RESPONSE
{
    VMBFS_MESSAGE_HEADER Header;
    UINT32 Status;

    UINT8 Payload[];

} VMBFS_MESSAGE_READ_FILE_RESPONSE, *PVMBFS_MESSAGE_READ_FILE_RESPONSE;

typedef struct _VMBFS_MESSAGE_READ_FILE_RDMA
{
    VMBFS_MESSAGE_HEADER Header;
    UINT32 Handle;
    UINT32 ByteCount;
    UINT64 FileOffset;
    UINT64 TokenOffset;
    WCHAR FilePath[];

} VMBFS_MESSAGE_READ_FILE_RDMA, *PVMBFS_MESSAGE_READ_FILE_RDMA;

typedef struct _VMBFS_MESSAGE_READ_FILE_RDMA_RESPONSE
{
    VMBFS_MESSAGE_HEADER Header;
    UINT32 Status;
    UINT32 ByteCount;

} VMBFS_MESSAGE_READ_FILE_RDMA_RESPONSE, *PVMBFS_MESSAGE_READ_FILE_RDMA_RESPONSE;

#pragma pack(pop)
#pragma warning(pop)

