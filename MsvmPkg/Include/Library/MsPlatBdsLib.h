/** @file
  Platform BDS library definition. A platform can implement 
  instances to support platform-specific behavior.

Copyright (c) 2008 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under 
the terms and conditions of the BSD License that accompanies this distribution.  
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.                                          
    
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __MS_PLAT_BDS_LIB_H_
#define __MS_PLAT_BDS_LIB_H_

/**
  Platform Bds initialization. Includes the platform firmware vendor, revision
  and so crc check.

**/
VOID
EFIAPI
PlatformBdsInit (
  VOID
  );

#endif
