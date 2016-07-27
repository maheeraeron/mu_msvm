/** @file
  The internal header file includes the common header files, defines
  internal structure and functions used by ImageVerificationLib.

Copyright (c) 2009 - 2013, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials 
are licensed and made available under the terms and conditions of the BSD License 
which accompanies this distribution.  The full text of the license may be found at 
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, 
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __IMAGEVERIFICATIONLIB_H__
#define __IMAGEVERIFICATIONLIB_H__

#include <Library/UefiDriverEntryPoint.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/PcdLib.h>
#include <Library/DevicePathLib.h>
#include <Library/SecurityManagementLib.h>
#include <Library/PeCoffLib.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/DevicePath.h>
#include <Protocol/BlockIo.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/VariableWrite.h>
#include <Guid/ImageAuthentication.h>
#include <Guid/AuthenticatedVariableFormat.h>
#include <IndustryStandard/PeImage.h>
#include <EfiNt.h>
#include <Protocol/SbCrypt.h>

#define EFI_CERT_TYPE_RSA2048_SHA256_SIZE 256
#define EFI_CERT_TYPE_RSA2048_SIZE        256
#define MAX_NOTIFY_STRING_LEN             64
#define TWO_BYTE_ENCODE                   0x82

#define ALIGNMENT_SIZE                    8
#define ALIGN_SIZE(a) (((a) % ALIGNMENT_SIZE) ? ALIGNMENT_SIZE - ((a) % ALIGNMENT_SIZE) : 0)

//
// Image type definitions
//
#define IMAGE_UNKNOWN                         0x00000000
#define IMAGE_FROM_FV                         0x00000001
#define IMAGE_FROM_OPTION_ROM                 0x00000002
#define IMAGE_FROM_REMOVABLE_MEDIA            0x00000003
#define IMAGE_FROM_FIXED_MEDIA                0x00000004

//
// Authorization policy bit definition
//
#define ALWAYS_EXECUTE                         0x00000000
#define NEVER_EXECUTE                          0x00000001
#define ALLOW_EXECUTE_ON_SECURITY_VIOLATION    0x00000002
#define DEFER_EXECUTE_ON_SECURITY_VIOLATION    0x00000003
#define DENY_EXECUTE_ON_SECURITY_VIOLATION     0x00000004
#define QUERY_USER_ON_SECURITY_VIOLATION       0x00000005

//
// Support hash types
//
#define HASHALG_SHA1                           0x00000000
#define HASHALG_SHA256                         0x00000001
#define HASHALG_MAX                            0x00000002

//
// Set max digest size as SHA256 Output (32 bytes) by far
//
#define MAX_DIGEST_SIZE    SHA256_DIGEST_SIZE      
//
//
// PKCS7 Certificate definition
//
typedef struct {
  WIN_CERTIFICATE Hdr;
  UINT8           CertData[1];
} WIN_CERTIFICATE_EFI_PKCS;

//
// Hash Algorithm Table
//
typedef struct {
  //
  // Name for Hash Algorithm
  //
  CHAR16                   *Name;
  //
  // Digest Length
  //
  UINTN                    DigestLength;
  //
  // Hash Algorithm OID ASN.1 Value
  //
  UINT8                    *OidValue;
  //
  // Length of Hash OID Value
  //
  UINTN                    OidLength;
  //
  //    Algorithm ID
  //
  HASH_ALG_ID              HashAlgorithmId;
 
} HASH_TABLE;

#endif
