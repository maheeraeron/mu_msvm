/** @file
  HiiConfigAccess definitions for Surface FrontPage.

  Copyright (c) 2015, Microsoft Corporation. All rights reserved.

**/

#include "FrontPageConfigAccess.h"
#include "FrontPage.h"
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Library/PrintLib.h>   // Should be temporary.

#include <Guid/DxePhaseVariables.h>

/**
  Quick helper function to see if ReadyToBoot has already been signalled.

  @retval     TRUE    ReadyToBoot has been signalled.
  @retval     FALSE   Otherwise...

**/
STATIC
BOOLEAN
IsPostReadyToBoot (
  VOID
  )
{
  EFI_STATUS        Status;
  UINT32            Attributes;
  PHASE_INDICATOR   Indicator;
  UINTN             Size = sizeof( Indicator );
  static BOOLEAN    Result, Initialized = FALSE;

  if (!Initialized)
  {
    Status = gRT->GetVariable( READY_TO_BOOT_INDICATOR_VAR_NAME,
                               &gMsDxePhaseVariablesGuid,
                               &Attributes,
                               &Size,
                               &Indicator );
    Result = (!EFI_ERROR( Status ) && Attributes == READY_TO_BOOT_INDICATOR_VAR_ATTR);
    Initialized = TRUE;
  }

  return Result;
} // IsPostReadyToBoot()


/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.


  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Request         A null-terminated Unicode string in <ConfigRequest> format.
  @param Progress        On return, points to a character in the Request string.
                         Points to the string's null terminator if request was successful.
                         Points to the most recent '&' before the first failing name/value
                         pair (or the beginning of the string if the failure is in the
                         first name/value pair) if the request was not successful.
  @param Results         A null-terminated Unicode string in <ConfigAltResp> format which
                         has all values filled in for the names in the Request string.
                         String to be allocated by the called function.

  @retval  EFI_SUCCESS            The Results is filled with the requested values.
  @retval  EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
  @retval  EFI_INVALID_PARAMETER  Request is illegal syntax, or unknown name.
  @retval  EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
ExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
/*
  EFI_STATUS                         Status;  
  FRONT_PAGE_HACK                    FrontPageHack;
  FRONT_PAGE_SECURITY_CONFIGURATION  FrontPageSecurity;
  FRONT_PAGE_GRAYOUT_CONFIGURATION   FrontPageGrayOut;
  FRONT_PAGE_DEVICE_CONFIGURATION    FrontPageDeviceSetting;
  MS_SYSTEM_SETTING_FLAGS            Flags;
  UINT8                              *Temp;
  
  DEBUG((DEBUG_INFO, "FrontPage:ExtractConfig() - Request=\n\t%s\n", Request));

  //
  // First, let's sanitize the input...
  if (Progress == NULL || Results == NULL)
  {
    DEBUG((DEBUG_INFO, "FrontPage:ExtractConfig - Invalid parameters.\n"));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Initialize the local variables.
  *Progress         = Request;

  //
  // Set a temporary buffer to refer to the request.
  // NOTE: For now, let's just ignore NULL requests.
  if (Request == NULL)
  {
    DEBUG ((DEBUG_ERROR, "FrontPage:ExtractConfig() - Request == NULL!\n"));
    // NOTE: In theory, we might create a new, complete request here.
    // But NAH!
    return EFI_NOT_FOUND;
  }

  
   if (HiiIsConfigHdrMatch(Request, &gMsFrontPageConfigFormSetGuid, L"FrontPageHack")){
      FrontPageHack.PlatformDeviceDisableSupportedMask = PcdGet64(PcdFrontPageDeviceDisablePlatformSupportMask);

      // NOTE: It's possible this can fail because of a bad value (ie. default not in the expected range).
      // If that occurs, it's conceivable that an attacker could trick one of the drivers publishing
      // setup data to provide a bad value which would cause this to fail which MAY lock the user
      // out of Setup. In all likelihood this is just a nuisance, but should be considered a possible
      // attack vector and planned for. Like, gracefully failing if this fails and requesting whether the
      // user wants to load defaults or something like that.
      // DOUBLE NOTE: Loading defaults should never allow an attacker to bypass a setting. In other words, an
      // attacker should not be able to corrupt a setting in order to force the system to load a default
      // that would reduce the security of the system.

      // Determine whether we are post-ReadyToBoot. This way, the security tab can display accurate results.
      // Is this hack-ish? A bit. But we can iterate on it if it proves too cumbersome.
      FrontPageHack.PostReadyToBoot = IsPostReadyToBoot();
      //
      // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
      //
      Status = mHiiConfigRouting->BlockToConfig(mHiiConfigRouting,
          Request,
          (UINT8*)&FrontPageHack,
          sizeof(FrontPageHack),
          Results,
          Progress);

      //
      // Set Progress string...
      // NOTE: This may need some more figuring out for error handling.
      if (!EFI_ERROR(Status))
      {
          DEBUG((DEBUG_INFO, "FrontPage:ExtractConfig:FrontPageHack() - Result=\n\t%s\n", *Results));
          *Progress = Request + StrLen(Request);
      }
  }   else if (HiiIsConfigHdrMatch(Request, &gMsFrontPageConfigFormSetGuid, L"FrontPageSecurity")){
      //Secure Boot Mode - Nobody is using this in the front page today.
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__SECURE_BOOT_KEYS_ENUM,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_SECUREBOOTKEYENUM,
          &FrontPageSecurity.SecureBootMode,
          &Flags);
      if (EFI_ERROR(Status)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting SecureBootMode - code=%r\n", Status));
          FrontPageSecurity.SecureBootMode = 0;
      }

      //TPM Mode
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__TPM_ENABLE,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageSecurity.TPMMode,
          &Flags);
      if (EFI_ERROR(Status)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting TPMMode - code=%r\n", Status));
          FrontPageSecurity.TPMMode = 0;
      }

      //
      // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
      //
      Status = mHiiConfigRouting->BlockToConfig(mHiiConfigRouting,
          Request,
          (UINT8*)&FrontPageSecurity,
          sizeof(FrontPageSecurity),
          Results,
          Progress);

      //
      // Set Progress string...
      // NOTE: This may need some more figuring out for error handling.
      if (!EFI_ERROR(Status))
      {
          DEBUG((DEBUG_INFO, "FrontPage:ExtractConfig() - Result=\n\t%s\n", *Results));
          *Progress = Request + StrLen(Request);
      }
  } else if (HiiIsConfigHdrMatch(Request, &gMsFrontPageConfigFormSetGuid, L"FrontPageDeviceConfig")){

      //Docking Port
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__DOCKING_USB_PORT,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.DockingPortMode,
          &Flags);
      if (EFI_ERROR(Status)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting DockingPortMode - code=%r\n", Status));
          FrontPageDeviceSetting.DockingPortMode = 0;
      }

      //Front Camera
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__FRONT_CAMERA,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.FCameraMode,
          &Flags);
      if (EFI_ERROR(Status)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting FCameraMode - code=%r\n", Status));
          FrontPageDeviceSetting.FCameraMode = 0;
      }

      //Rear Camera
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__REAR_CAMERA,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.RCameraMode,
          &Flags);
      if (EFI_ERROR(Status)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting RCameraMode - code=%r\n", Status));
          FrontPageDeviceSetting.RCameraMode = 0;
      }

      //IR Camera
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__IR_CAMERA,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.IRCameraMode,
          &Flags);
      if (EFI_ERROR(Status)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting IRCameraMode - code=%r\n", Status));
          FrontPageDeviceSetting.IRCameraMode = 0;
      }

      //WFOV Camera
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__WFOV_CAMERA,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.WFOVCameraMode,
          &Flags);
      if (EFI_ERROR(Status)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting WFOVCameraMode - code=%r\n", Status));
          FrontPageDeviceSetting.WFOVCameraMode = 0;
      }

      //All Cameras
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__ALL_CAMERAS,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.ACameraMode,
          &Flags);
      if (EFI_ERROR(Status)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting ACameraMode - code=%r\n", Status));
          FrontPageDeviceSetting.ACameraMode = 0;
      }

      //On board audio
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__ONBOARD_AUDIO,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.OnBoardAudioMode,
          &Flags);
      if (EFI_ERROR(Status)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting OnBoardAudioMode - code=%r\n", Status));
          FrontPageDeviceSetting.OnBoardAudioMode = 0;
      }

      //Micro SD
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__MICRO_SDCARD,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.MicroSDMode,
          &Flags);
      if (EFI_ERROR(Status)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting MicroSDMode - code=%r\n", Status));
          FrontPageDeviceSetting.MicroSDMode = 0;
      }

      // NOTE: SettingAccess->Get does not alter the setting if the SettingID is not valid for the platform
      FrontPageDeviceSetting.WiFiMode = 0;
      FrontPageDeviceSetting.BluetoothMode = 0;

      //Wifi Mode
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__WIFI_ONLY,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.WiFiMode,
          &Flags);
      if (EFI_ERROR(Status)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting WiFiMode - code=%r\n", Status));
      }

      //Wifi and bluetooth Mode
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__WIFI_AND_BLUETOOTH,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.WiFiMode,
          &Flags);
      if (EFI_ERROR(Status)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting WiFiBleMode - code=%r\n", Status));
      }

      //Bluetooth Mode
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__BLUETOOTH,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.BluetoothMode,
          &Flags);
      if (EFI_ERROR(Status)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting BluetoothMode - code=%r\n", Status));
      }

      //LAN Mode
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__WIRED_LAN,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.LanMode,
          &Flags);
      if (EFI_ERROR(Status)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting LanMode - code=%r\n", Status));
          FrontPageDeviceSetting.LanMode = 0;
      }

      //Blade Port Mode
      Status = mSettingAccess->Get(mSettingAccess,
        MS_SYSTEM_SETTING_ID__BLADE_USB_PORT,
        &mAuthToken,
        MS_SYSTEM_SETTING_TYPE_ENABLE,
        &FrontPageDeviceSetting.BladePort,
        &Flags);
      if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting BladePort - code=%r\n", Status));
        FrontPageDeviceSetting.BladePort = 0;
      }

      //Accessory Radio Mode
      Status = mSettingAccess->Get(mSettingAccess,
        MS_SYSTEM_SETTING_ID__ACCESSORY_RADIO_USB_PORT,
        &mAuthToken,
        MS_SYSTEM_SETTING_TYPE_ENABLE,
        &FrontPageDeviceSetting.AccessoryRadioMode,
        &Flags);
      if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting AccessoryRadioMode - code=%r\n", Status));
        FrontPageDeviceSetting.AccessoryRadioMode = 0;
      }

      //LTE Modem Mode
      Status = mSettingAccess->Get(mSettingAccess,
        MS_SYSTEM_SETTING_ID__LTE_MODEM_USB_PORT,
        &mAuthToken,
        MS_SYSTEM_SETTING_TYPE_ENABLE,
        &FrontPageDeviceSetting.LteModemMode,
        &Flags);
      if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting LteModemMode - code=%r\n", Status));
        FrontPageDeviceSetting.LteModemMode = 0;
      }

      //
      // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
      //
      Status = mHiiConfigRouting->BlockToConfig(mHiiConfigRouting,
          Request,
          (UINT8*)&FrontPageDeviceSetting,
          sizeof(FrontPageDeviceSetting),
          Results,
          Progress);

      //
      // Set Progress string...
      // NOTE: This may need some more figuring out for error handling.
      if (!EFI_ERROR(Status))
      {
          DEBUG((DEBUG_INFO, "FrontPage:ExtractConfig() - Result=\n\t%s\n", *Results));
          *Progress = Request + StrLen(Request);
      }
  } else if (HiiIsConfigHdrMatch(Request, &gMsFrontPageConfigFormSetGuid, L"FrontPageGrayOut")){

      FrontPageGrayOut.DisplayRestrictedStringSecurity = FALSE;
      FrontPageGrayOut.DisplayRestrictedStringDevices = FALSE; //This will be false unless one of the providers has no write access

      //GrayOut for Secure Boot
      FrontPageGrayOut.SecureBootMode =TRUE;
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__SECURE_BOOT_KEYS_ENUM,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_SECUREBOOTKEYENUM,
          &Temp,
          &Flags);

      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.SecureBootMode = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringSecurity = TRUE;
          }
      }
      else {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout SecureBootMode - code=%r\n", Status));
      }

      //GrayOut for TPM Mode
      FrontPageGrayOut.TPMMode = TRUE;
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__TPM_ENABLE,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &Temp,
          &Flags);

      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.TPMMode = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringSecurity = TRUE;
          }
      }
      else {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout TPMMode - code=%r\n", Status));
      }

      //Docking Port
      FrontPageGrayOut.DockingPortMode = TRUE;
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__DOCKING_USB_PORT,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &Temp,
          &Flags);

      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.DockingPortMode = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringDevices = TRUE;
          }
      }
      else {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout DockingPortMode - code=%r\n", Status));
      }

      //Front Camera
      FrontPageGrayOut.FCameraMode = TRUE;
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__FRONT_CAMERA,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.FCameraMode,
          &Flags);

      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.FCameraMode = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringDevices = TRUE;
          }
      }
      else {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout FCameraMode - code=%r\n", Status));
      }

      //Rear Camera
      FrontPageGrayOut.RCameraMode = TRUE;
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__REAR_CAMERA,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &Temp,
          &Flags);

      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.RCameraMode = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringDevices = TRUE;
          }
      }
      else {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout RCameraMode - code=%r\n", Status));
      }

      //IR Camera
      FrontPageGrayOut.IRCameraMode = TRUE;
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__IR_CAMERA,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &Temp,
          &Flags);

      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.IRCameraMode = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringDevices = TRUE;
          }
      }
      else {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout IRCameraMode - code=%r\n", Status));
      }
      
	  //WFOV Camera
      FrontPageGrayOut.WFOVCameraMode = TRUE;
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__WFOV_CAMERA,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &Temp,
          &Flags);

      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.WFOVCameraMode = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringDevices = TRUE;
          }
      }
      else {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout WFOVCameraMode - code=%r\n", Status));
      }

      //All Cameras
      FrontPageGrayOut.ACameraMode = TRUE;
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__ALL_CAMERAS,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &Temp,
          &Flags);

      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.ACameraMode = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringDevices = TRUE;
          }
      }
      else {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout ACameraMode - code=%r\n", Status));
      }

      //On board audio
      FrontPageGrayOut.OnBoardAudioMode = TRUE;
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__ONBOARD_AUDIO,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &Temp,
          &Flags);

      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.OnBoardAudioMode = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringDevices = TRUE;
          }
      }
      else {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout OnBoardAudioMode - code=%r\n", Status));
      }

      //Micro SD
      FrontPageGrayOut.MicroSDMode = TRUE;
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__MICRO_SDCARD,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &Temp,
          &Flags);

      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.MicroSDMode = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringDevices = TRUE;
          }
      }
      else {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout MicroSDMode - code=%r\n", Status));
      }

      FrontPageGrayOut.WiFiMode = TRUE;
      FrontPageGrayOut.BluetoothMode = TRUE;
      //Wifi and bluetooth Mode
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__WIFI_AND_BLUETOOTH,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &Temp,
          &Flags);
      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.WiFiMode = FALSE;
              FrontPageGrayOut.BluetoothMode = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringDevices = TRUE;
          }
      }

      //Wifi Mode
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__WIFI_ONLY,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &Temp,
          &Flags);

      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.WiFiMode = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringDevices = TRUE;
          }
          else{
              FrontPageGrayOut.WiFiMode = TRUE;
          }
      }
      else {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout WiFiMode - code=%r\n", Status));
      }

      //Bluetooth Mode
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__BLUETOOTH,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &Temp,
          &Flags);

      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.BluetoothMode = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringDevices = TRUE;
          }
          else{
              FrontPageGrayOut.BluetoothMode = TRUE;
          }
      }
      else {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout BluetoothMode - code=%r\n", Status));
      }

      //LAN Mode
      FrontPageGrayOut.LanMode = TRUE;
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__WIRED_LAN,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &Temp,
          &Flags);

      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.LanMode = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringDevices = TRUE;
          }
      }
      else {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout LanMode - code=%r\n", Status));
      }

      //Blade USB Port
      FrontPageGrayOut.BladePort = TRUE;
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__BLADE_USB_PORT,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &Temp,
          &Flags);

      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.BladePort = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringDevices = TRUE;
          }
      }
      else {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout BladePort - code=%r\n", Status));
      }

      FrontPageGrayOut.SystemPassword = TRUE;
      Status = mSettingAccess->Get(mSettingAccess,
          MS_SYSTEM_SETTING_ID__PASSWORD,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_PASSWORD,
          &Temp,
          &Flags);
      if (!EFI_ERROR(Status)) {
          if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
              FrontPageGrayOut.SystemPassword = FALSE;
              FrontPageGrayOut.DisplayRestrictedStringSecurity = TRUE;
          }
      }
      else {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout System Password - code=%r\n", Status));
      }

      //Accessory Radio USB Port
      FrontPageGrayOut.AccessoryRadio = TRUE;
      Status = mSettingAccess->Get(mSettingAccess,
        MS_SYSTEM_SETTING_ID__ACCESSORY_RADIO_USB_PORT,
        &mAuthToken,
        MS_SYSTEM_SETTING_TYPE_ENABLE,
        &Temp,
        &Flags);

      if (!EFI_ERROR(Status)) {
        if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
          FrontPageGrayOut.AccessoryRadio = FALSE;
          FrontPageGrayOut.DisplayRestrictedStringDevices = TRUE;
        }
      }
      else {
        DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout AccessoryRadio - code=%r\n", Status));
      }

      //LTE Modem USB Port
      FrontPageGrayOut.LteModem = TRUE;
      Status = mSettingAccess->Get(mSettingAccess,
        MS_SYSTEM_SETTING_ID__LTE_MODEM_USB_PORT,
        &mAuthToken,
        MS_SYSTEM_SETTING_TYPE_ENABLE,
        &Temp,
        &Flags);

      if (!EFI_ERROR(Status)) {
        if ((MS_SYSTEM_SETTING_FLAGS_OUT_WRITE_ACCESS & Flags) == 0) {
          FrontPageGrayOut.LteModem = FALSE;
          FrontPageGrayOut.DisplayRestrictedStringDevices = TRUE;
        }
      }
      else {
        DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Grayout LteModem - code=%r\n", Status));
      }

      //
      // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
      //
      Status = mHiiConfigRouting->BlockToConfig(mHiiConfigRouting,
          Request,
          (UINT8*)&FrontPageGrayOut,
          sizeof(FrontPageGrayOut),
          Results,
          Progress);

      //
      // Set Progress string...
      // NOTE: This may need some more figuring out for error handling.
      if (!EFI_ERROR(Status))
      {
          DEBUG((DEBUG_INFO, "FrontPage:ExtractConfig() - Result=\n\t%s\n", *Results));
          *Progress = Request + StrLen(Request);
      }
  }
  else{
      return EFI_UNSUPPORTED;
  }

  return Status;
*/

  return EFI_NOT_FOUND;
}

/**
  This function processes the results of changes in configuration.


  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Configuration   A null-terminated Unicode string in <ConfigResp> format.
  @param Progress        A pointer to a string filled in with the offset of the most
                         recent '&' before the first failing name/value pair (or the
                         beginning of the string if the failure is in the first
                         name/value pair) or the terminating NULL if all was successful.

  @retval  EFI_SUCCESS            The Results is processed successfully.
  @retval  EFI_INVALID_PARAMETER  Configuration is NULL.
  @retval  EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
RouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
/*
  EFI_STATUS                         Status;
  UINTN                              BufferSize;
  FRONT_PAGE_DEVICE_CONFIGURATION    FrontPageDeviceSetting;
  MS_SYSTEM_SETTING_FLAGS            Flags = 0;
  EFI_STATUS                         SetStatus;

  DEBUG((DEBUG_INFO, "FrontPage:RouteConfig() - Configuration=\n\t%s\n", Configuration));

  //
  // First, let's sanitize the input...
  if (Configuration == NULL || Progress == NULL)
  {
    DEBUG((DEBUG_INFO, "FrontPage:RouteConfig - Invalid parameters.\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (HiiIsConfigHdrMatch(Configuration, &gMsFrontPageConfigFormSetGuid, L"FrontPageDeviceConfig")) {
      //
      // Initialize the local variables.
      *Progress = Configuration;

      //
      // Convert <ConfigResp> to buffer data by helper function ConfigToBlock()
      BufferSize = sizeof(FrontPageDeviceSetting);
      Status = mHiiConfigRouting->ConfigToBlock(mHiiConfigRouting,
          Configuration,
          (UINT8*)&FrontPageDeviceSetting,
          &BufferSize,
          Progress
          );

      if (EFI_ERROR(Status))
      {
          return Status;
      }

      DEBUG((DEBUG_INFO, __FUNCTION__"Saving Front Page Device Setting \n"));

      //Docking Port
      SetStatus = mSettingAccess->Set(mSettingAccess,
          MS_SYSTEM_SETTING_ID__DOCKING_USB_PORT,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.DockingPortMode,
          &Flags);
      if (EFI_ERROR(SetStatus)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting DockingPortMode - code=%r\n", SetStatus));
      }

      //Front Camera
      SetStatus = mSettingAccess->Set(mSettingAccess,
          MS_SYSTEM_SETTING_ID__FRONT_CAMERA,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.FCameraMode,
          &Flags);
      if (EFI_ERROR(SetStatus)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting FCameraMode - code=%r\n", SetStatus));
      }

      //Rear Camera
      SetStatus = mSettingAccess->Set(mSettingAccess,
          MS_SYSTEM_SETTING_ID__REAR_CAMERA,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.RCameraMode,
          &Flags);
      if (EFI_ERROR(SetStatus)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting RCameraMode - code=%r\n", SetStatus));
      }

      //IR Camera
      SetStatus = mSettingAccess->Set(mSettingAccess,
          MS_SYSTEM_SETTING_ID__IR_CAMERA,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.IRCameraMode,
          &Flags);
      if (EFI_ERROR(SetStatus)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting IRCameraMode - code=%r\n", SetStatus));
      }

      //WFOV Camera
      SetStatus = mSettingAccess->Set(mSettingAccess,
          MS_SYSTEM_SETTING_ID__WFOV_CAMERA,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.WFOVCameraMode,
          &Flags);
      if (EFI_ERROR(SetStatus)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting WFOVCameraMode - code=%r\n", SetStatus));
      }
	  
      //All Cameras
      SetStatus = mSettingAccess->Set(mSettingAccess,
          MS_SYSTEM_SETTING_ID__ALL_CAMERAS,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.ACameraMode,
          &Flags);
      if (EFI_ERROR(SetStatus)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting ACameraMode - code=%r\n", SetStatus));
      }

      //On board audio
      SetStatus = mSettingAccess->Set(mSettingAccess,
          MS_SYSTEM_SETTING_ID__ONBOARD_AUDIO,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.OnBoardAudioMode,
          &Flags);
      if (EFI_ERROR(SetStatus)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting OnBoardAudioMode - code=%r\n", SetStatus));
      }

      //Micro SD
      SetStatus = mSettingAccess->Set(mSettingAccess,
          MS_SYSTEM_SETTING_ID__MICRO_SDCARD,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.MicroSDMode,
          &Flags);
      if (EFI_ERROR(SetStatus)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting MicroSDMode - code=%r\n", SetStatus));
      }

      if (PcdGet64(PcdFrontPageDeviceDisablePlatformSupportMask) & FP_DD_WIFI_BLE){
          //Wifi Mode
          SetStatus = mSettingAccess->Set(mSettingAccess,
              MS_SYSTEM_SETTING_ID__WIFI_AND_BLUETOOTH,
              &mAuthToken,
              MS_SYSTEM_SETTING_TYPE_ENABLE,
              &FrontPageDeviceSetting.WiFiMode,
              &Flags);
          if (EFI_ERROR(SetStatus)) {
              DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting Wifi in Wifi and Bluetooth - code=%r\n", SetStatus));
          }
          //Bluetooth Mode
          SetStatus = mSettingAccess->Set(mSettingAccess,
              MS_SYSTEM_SETTING_ID__BLUETOOTH,
              &mAuthToken,
              MS_SYSTEM_SETTING_TYPE_ENABLE,
              &FrontPageDeviceSetting.BluetoothMode,
              &Flags);
          if (EFI_ERROR(SetStatus)) {
              DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting BluetoothMode in wifi and blue tooth- code=%r\n", SetStatus));
          }
      } else{
          //Wifi Mode
          SetStatus = mSettingAccess->Set(mSettingAccess,
              MS_SYSTEM_SETTING_ID__WIFI_ONLY,
              &mAuthToken,
              MS_SYSTEM_SETTING_TYPE_ENABLE,
              &FrontPageDeviceSetting.WiFiMode,
              &Flags);
          if (EFI_ERROR(SetStatus)) {
              DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting Wifi only - code=%r\n", SetStatus));
          }
      }

      //Bluetooth Mode
      SetStatus = mSettingAccess->Set(mSettingAccess,
          MS_SYSTEM_SETTING_ID__BLUETOOTH,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.BluetoothMode,
          &Flags);
      if (EFI_ERROR(SetStatus)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting BluetoothMode - code=%r\n", SetStatus));
      }

      //LAN Mode
      SetStatus = mSettingAccess->Set(mSettingAccess,
          MS_SYSTEM_SETTING_ID__WIRED_LAN,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.LanMode,
          &Flags);
      if (EFI_ERROR(SetStatus)) {
          DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting LanMode - code=%r\n", SetStatus));
      }

        //Blade USB Mode
        SetStatus = mSettingAccess->Set(mSettingAccess,
          MS_SYSTEM_SETTING_ID__BLADE_USB_PORT,
          &mAuthToken,
          MS_SYSTEM_SETTING_TYPE_ENABLE,
          &FrontPageDeviceSetting.BladePort,
          &Flags);
      if (EFI_ERROR(SetStatus)) {
        DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting BladePort - code=%r\n", SetStatus));
      }

      //AccessoryRadio USB Mode
      SetStatus = mSettingAccess->Set(mSettingAccess,
        MS_SYSTEM_SETTING_ID__ACCESSORY_RADIO_USB_PORT,
        &mAuthToken,
        MS_SYSTEM_SETTING_TYPE_ENABLE,
        &FrontPageDeviceSetting.AccessoryRadioMode,
        &Flags);
      if (EFI_ERROR(SetStatus)) {
        DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting AccessoryRadioMode - code=%r\n", SetStatus));
      }

      //LTE Modem USB Mode
      SetStatus = mSettingAccess->Set(mSettingAccess,
        MS_SYSTEM_SETTING_ID__LTE_MODEM_USB_PORT,
        &mAuthToken,
        MS_SYSTEM_SETTING_TYPE_ENABLE,
        &FrontPageDeviceSetting.LteModemMode,
        &Flags);
      if (EFI_ERROR(SetStatus)) {
        DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error setting LteModemMode - code=%r\n", SetStatus));
      }
  }
  else if (HiiIsConfigHdrMatch(Configuration, &gMsFrontPageConfigFormSetGuid, L"FrontPageSecurity")) {
    // Nothing needs to be done for routing the TPM data and the secureboot here.
    // Setting provider set is called from HandleTpmChange and HandleSecureBootChange.
    //This is Phase1 of moving to setting provider and introducing placeholders.
      DEBUG((DEBUG_INFO, __FUNCTION__" FrontPageSecurity Variable will be ignored \n"));
      Status = EFI_SUCCESS;
  }
  else if (HiiIsConfigHdrMatch(Configuration, &gMsFrontPageConfigFormSetGuid, L"FrontPageGrayOut")) {
      // Nothing needs to be done for routing the TPM data and the secureboot here.
      // Setting provider set is called from HandleTpmChange and HandleSecureBootChange.
      //This is Phase1 of moving to setting provider and introducing placeholders.
      DEBUG((DEBUG_INFO, __FUNCTION__" FrontPageGrayOut Variable will be ignored \n"));
      Status = EFI_SUCCESS;
  }
  else if (HiiIsConfigHdrMatch(Configuration, &gMsFrontPageConfigFormSetGuid, L"FrontPageHack")) {
      // Nothing needs to be done for routing the TPM data and the secureboot here.
      // Setting provider set is called from HandleTpmChange and HandleSecureBootChange.
      //This is Phase1 of moving to setting provider and introducing placeholders.
      DEBUG((DEBUG_INFO, __FUNCTION__" FrontPageHack Variable will be ignored \n"));
      Status = EFI_SUCCESS;
  }
  else {
  // we shouldnt be here. there should not be any other var stores.
      DEBUG((DEBUG_ERROR, __FUNCTION__" UNEXPECTED Front Page Variable \n"));
      Status = EFI_UNSUPPORTED;
  }

  return Status;
*/

  return EFI_NOT_FOUND;
}
