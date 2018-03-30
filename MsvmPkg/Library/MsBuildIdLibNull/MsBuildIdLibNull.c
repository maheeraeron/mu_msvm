/** @file
 Library provides helper function when working with MS Build ID
 This version just fills the contract.  It is a a NULL instance

Copyright (c) 2014, Microsoft Corporation. All rights reserved.<BR>

**/

#include <Base.h>
#include <Library/PrintLib.h>
#include <Library/MsBuildIdLib.h>

/**
Function converts a MS formatted BIOS ID value (32bit value) into a UINCODE String.

@param[in]  StringBuffer    Caller allocated buffer pointer where string version will be copied. Must be a buffer able to hold at least 15 unicode characters
@param[in]  Id              BIOS ID value
@param[in]  VersionType     Version Format Type , 0 represents 10.12.10 and 1 represents 8.16.8

@retval RETURN_SUCCESS            - StringBuffer filled with version successfully
@retval RETURN_INVALID_PARAMETER  - StringBuffer can't be NULL

**/
RETURN_STATUS
EFIAPI
MsBuildIdConvertToString(
    IN CHAR16                     *StringBuffer,
    IN UINT32                     Id,
    IN MS_BUILD_VERSION_TYPE      VersionType
 )
{
  if (StringBuffer == NULL)
  {
    return RETURN_INVALID_PARAMETER;
  }

  UnicodeSPrint(StringBuffer, 30  /* 15 unicode chars is 30 bytes */, L"%d", Id);

  return RETURN_SUCCESS;
}


/**
Function get the date from a MS formatted BIOS ID value (32bit value) into a UINCODE date String MM/DD/YYYY.

@param[in]  StringBuffer    Caller allocated buffer pointer where string date will be copied. Must be a buffer able to hold at least 11 unicode characters.
@param[in]  Id              BIOS ID value

@retval RETURN_SUCCESS            - StringBuffer filled with date successfully
@retval RETURN_INVALID_PARAMETER  - StringBuffer can't be NULL
@retval RETURN_UNSUPPORTED        - BIOS ID can't be converted to date

**/
RETURN_STATUS
EFIAPI
MsBuildIdGetBuildDateString(
IN CHAR16     *StringBuffer,
IN UINT32     Id
)
{
  if (StringBuffer == NULL)
  {
    return RETURN_INVALID_PARAMETER;
  }

  return RETURN_UNSUPPORTED;
}
