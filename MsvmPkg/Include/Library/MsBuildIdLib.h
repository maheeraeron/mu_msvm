/** @file
Provides helper function when working with MS Build ID  


Copyright (c) 2014, Microsoft Corporation. All rights reserved.<BR>

**/

#ifndef __MS_BUILD_ID_LIB__
#define __MS_BUILD_ID_LIB__

// Minumum buffer size (CHAR16 count) needed for MsBuildIdConvertToString() including termination
#define MS_BUILD_ID_STRING_SIZE      16

// Minumum buffer size (CHAR16 count) needed for MsBuildIdGetBuildDateString() including termination
#define MS_BUILD_DATE_STRING_SIZE    11

//
// GIC definitions
//
typedef enum {
    VERSION_TYPE_0, //10:12:10
    VERSION_TYPE_1  //8:16:8
} MS_BUILD_VERSION_TYPE;


/**
  Function converts a MS formatted BIOS ID value (32bit value) into a UINCODE String.  

  @param[in]  StringBuffer    Caller allocated buffer pointer where string version will be copied.  Buffer needs to be large enough to handle the string length.  15 chars)
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
);


/**
  Function get the date from a MS formatted BIOS ID value (32bit value) into a UINCODE date String MM/DD/YYYY.

  @param[in]  StringBuffer    Caller allocated buffer pointer where string date will be copied.  Must be large enough to handle 11 Unicode characters.  
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
);


#endif