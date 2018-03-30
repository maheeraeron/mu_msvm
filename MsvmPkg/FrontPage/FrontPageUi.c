/** @file
  User interaction functions for the Surface FrontPage.

  Copyright (c) 2015, Microsoft Corporation. All rights reserved.

**/

#include "FrontPage.h"        // TODO: Perhaps wrap the keys in their own .h file.
#include "FrontPageUi.h"
#include "Language.h"

#include <PiDxe.h>          // This has to be here so Protocol/FirmwareVolume2.h doesn't puke errors.

//#include <MsTpmReservedNvIndices.h>

#include <Guid/FrontPageEventDataStruct.h>
#include <Guid/GlobalVariable.h>
#include <Guid/ImageAuthentication.h>
#include <Guid/SmmVariableCommon.h>
#include <Guid/MdeModuleHii.h>

#include <Protocol/OnScreenKeyboard.h>
#include <Protocol/SimpleWindowManager.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/SmmCommunication.h>
#include <Protocol/SmmVariable.h>
//#include <Protocol/SurfaceTpmProtocol.h>

#include <Library/BaseMemoryLib.h>
#include <Library/SecureMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/HiiLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
//#include <Library/MsSecureBootLib.h>
//#include <Library/MsPasswordLib.h>
//#include <Library/PlatformHstiLib.h>

extern CHAR8                                *mLanguageString;
extern MS_ONSCREEN_KEYBOARD_PROTOCOL        *mOSKProtocol;
extern MS_SIMPLE_WINDOW_MANAGER_PROTOCOL    *mSWMProtocol;
extern EFI_HII_HANDLE                       gStringPackHandle;
extern FRONT_PAGE_CONFIGURATION             mFrontPageConfig;
extern CHAR16                               mConfigEFIVariableName[];
extern BOOLEAN                              mResetRequired;
//extern MS_SYSTEM_SETTING_ACCESS_PROTOCOL      *mSettingAccess;
//extern UINTN                                   mAuthToken;

/**
 * RefreshSecurityForm - Notify browser that the form has changes
 *
 *
 * @return VOID
 */
/*
VOID RefreshSecurityForm ( VOID ) {
    VOID                          *StartOpCodeHandle;
    VOID                          *EndOpCodeHandle;
    EFI_IFR_GUID_LABEL            *StartLabel;
    EFI_IFR_GUID_LABEL            *EndLabel;
    static UINT8                  NullOpCount = 1;

    DEBUG(( DEBUG_VERBOSE, "VERBOSE [SFP] %a - Attempting to force Security form refresh.\n", __FUNCTION__ ));

    //
    // Determine how many NULL opcodes we need.
    // This is so that the number of opcodes will vary from execution to
    // execution and force the page CRC to always change.
    // Can toggle back and forth.
    //
    if (1 != NullOpCount)
    {
        NullOpCount = 1;
    }
    else
    {
        NullOpCount = 2;
    }

    //
    // Init OpCode Handle and Allocate space for creation of UpdateData Buffer
    //
    StartOpCodeHandle = HiiAllocateOpCodeHandle();
    ASSERT(StartOpCodeHandle != NULL);

    EndOpCodeHandle = HiiAllocateOpCodeHandle();
    ASSERT(EndOpCodeHandle != NULL);

    //
    // Create Hii Extend Label OpCode as the start opcode
    //
    StartLabel = (EFI_IFR_GUID_LABEL *)HiiCreateGuidOpCode(StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof(EFI_IFR_GUID_LABEL));
    StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;

    //
    // Create Hii Extend Label OpCode as the end opcode
    //
    EndLabel = (EFI_IFR_GUID_LABEL *)HiiCreateGuidOpCode(EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof(EFI_IFR_GUID_LABEL));
    EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;

    StartLabel->Number     = LABEL_UPDATE_SECURITY_START;
    EndLabel->Number       = LABEL_UPDATE_SECURITY_END;

    HiiCreateTextOpCode (StartOpCodeHandle,STRING_TOKEN(STR_NULL_STRING),0,0);
    if (NullOpCount > 1)
    {
        HiiCreateTextOpCode (StartOpCodeHandle,STRING_TOKEN(STR_NULL_STRING),0,0);
    }

    HiiUpdateForm(
        gFrontPagePrivate.HiiHandle,
        &gMsFrontPageConfigFormSetGuid,
        FRONT_PAGE_FORM_ID_SECURITY,
        StartOpCodeHandle,
        EndOpCodeHandle
        );

    HiiFreeOpCodeHandle(StartOpCodeHandle);
    HiiFreeOpCodeHandle(EndOpCodeHandle);
}
*/

/**
  This function processes the results of changes in configuration.

  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Action          Specifies the type of action taken by the browser.
  @param QuestionId      A unique value which is sent to the original exporting driver
                         so that it can identify the type of data to expect.
  @param Type            The type of value for the question.
  @param Value           A pointer to the data being sent to the original exporting driver.
  @param ActionRequest   On return, points to the action requested by the callback function.

  @retval  EFI_SUCCESS           The callback successfully handled the action.
  @retval  EFI_OUT_OF_RESOURCES  Not enough storage is available to hold the variable and its data.
  @retval  EFI_DEVICE_ERROR      The variable could not be saved.
  @retval  EFI_UNSUPPORTED       The specified Action is not supported by the callback.

**/
EFI_STATUS
EFIAPI
UiCallback (
           IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
           IN  EFI_BROWSER_ACTION                     Action,
           IN  EFI_QUESTION_ID                        QuestionId,
           IN  UINT8                                  Type,
           IN  EFI_IFR_TYPE_VALUE                     *Value,
           OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
           )
{
    EFI_STATUS    Status = EFI_SUCCESS;

    DEBUG ((DEBUG_INFO, "FrontPage:UiCallback() - Question ID=0x%08x Type=0x%04x Action=0x%04x ShortValue=0x%02x\n", QuestionId, Type, Action, *(UINT8*)Value));

    //
    // Sanitize input values.
    if (Value == NULL || ActionRequest == NULL)
    {
        DEBUG ((DEBUG_INFO, "FrontPage:UiCallback - Bailing from invalid input.\n"));
        return EFI_INVALID_PARAMETER;
    }

    //
    // Filter responses.
    // NOTE: For now, let's only consider elements that have CHANGED.
    if (Action != EFI_BROWSER_ACTION_CHANGED)
    {
        DEBUG ((DEBUG_INFO, "FrontPage:UiCallback - Bailing from unimportant input.\n"));
        return EFI_UNSUPPORTED;
    }

    //
    // Set a default action request.
    *ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;

    //
    // Handle the specific callback.
    // We'll record the callback event as mCallbackKey so that other processes can make decisions
    // on how we exited the run loop (if that occurs).
    mCallbackKey = QuestionId;
    switch (mCallbackKey)
    {
    // FRONT_PAGE_KEY_CONTINUE is the "Exit Menu" option.
    case FRONT_PAGE_ACTION_CONTINUE:
        *ActionRequest = EFI_BROWSER_ACTION_REQUEST_SUBMIT;
        // mCallbackKey set to FRONT_PAGE_KEY_CONTINUE will cause the main run loop to exit
        // once the form browser exits.
        break;

    case FRONT_PAGE_ACTION_SEC_DISPLAY_SB_WHAT_IS:
    case FRONT_PAGE_ACTION_SEC_DISPLAY_TPM_WHAT_IS:
        Status = HandleInfoPopup( Value, ActionRequest );
        break;

    /////////////////////////////////////////////////////////////////////////////
    // SECURITY CALLBACKS
    //
    case FRONT_PAGE_ACTION_SEC_SET_SYSTEM_PASSWORD:
        //Status = SetSystemPassword ( Value, ActionRequest );
        break;

    case FRONT_PAGE_ACTION_SEC_CHANGE_SB_CONFIG:
        //Status = HandleSecureBootChange( Value, ActionRequest );
        break;

        //case FRONT_PAGE_ACTION_LANG_SELECT_LANGUAGE:
        //Status = HandleLanguage( Value, ActionRequest );
        //break;

    case FRONT_PAGE_ACTION_SEC_TPM_ENABLE:
        //Status = HandleTpmChange( Value, ActionRequest );
        break;

    case FRONT_PAGE_ACTION_DEVICE_ENABLE_DOCKINGPORT:
    case FRONT_PAGE_ACTION_DEVICE_ENABLE_FCAMERA:
    case FRONT_PAGE_ACTION_DEVICE_ENABLE_RCAMERA:
    case FRONT_PAGE_ACTION_DEVICE_ENABLE_IRCAMERA:
    case FRONT_PAGE_ACTION_DEVICE_ENABLE_WFOVCAMERA:
    case FRONT_PAGE_ACTION_DEVICE_ENABLE_ACAMERA:
    case FRONT_PAGE_ACTION_DEVICE_ENABLE_ONBOARD_AUDIO:
    case FRONT_PAGE_ACTION_DEVICE_ENABLE_MICROSD:
    case FRONT_PAGE_ACTION_DEVICE_ENABLE_WIFI:
    case FRONT_PAGE_ACTION_DEVICE_ENABLE_BLUETOOTH:
    case FRONT_PAGE_ACTION_DEVICE_ENABLE_WIFI_BLE:
    case FRONT_PAGE_ACTION_DEVICE_ENABLE_WIRED_LAN:
    case FRONT_PAGE_ACTION_DEVICE_ENABLE_ACC_RADIO:
    case FRONT_PAGE_ACTION_DEVICE_ENABLE_LTE_MODEM:
      *ActionRequest = EFI_BROWSER_ACTION_REQUEST_FORM_APPLY;
        break;

    case FRONT_PAGE_ACTION_EXIT_FRONTPAGE:
        *ActionRequest = EFI_BROWSER_ACTION_REQUEST_EXIT;
        break;

    case FRONT_PAGE_ACTION_REBOOT_TO_FRONTPAGE:
        Status = HandleRebootToFrontPage( Value, ActionRequest );
        break;

    default:
        DEBUG ((DEBUG_INFO, "FrontPage:UiCallback - Unknown event passed.\n"));
        Status = EFI_UNSUPPORTED;
        mCallbackKey = 0;
        break;
    }

    return Status;
}

/*
STATIC
EFI_STATUS
SetSystemPassword (IN  EFI_IFR_TYPE_VALUE           *Value,
                   OUT EFI_BROWSER_ACTION_REQUEST   *ActionRequest)
{
    EFI_STATUS      Status          = EFI_SUCCESS;
    SWM_MB_RESULT   Result          = 0;
    PW_TEST_BITMAP  PwdValidBitmap  = 0;
    CHAR16          *pErrorMessage  = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_NULL_STRING), NULL);
    CHAR16          *PasswordBuffer = NULL;    // This will be allocated by PasswordDialog(). Needs to be tracked, wiped, and freed.

    DEBUG((DEBUG_INFO, "INFO: [FP] SetSystemPassword: ENTER\r\n"));


    // Present a dialog to the user for setting the password.
    //
    do
    {

        Status = mSWMProtocol->PasswordDialog (mSWMProtocol,
                                               HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_ENTER_PWD_TITLEBARTEXT), NULL),   // Dialog titlebar text.
                                               HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_CAPTION), NULL),                  // Dialog caption text.
                                               HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_SET_BODYTEXT), NULL),             // Dialog body text.
                                               pErrorMessage,
                                               SWM_PWD_TYPE_SET_PASSWORD,
                                               &Result,
                                               &PasswordBuffer
                                              );


        // If the user selects cancel or there's a dialog error, abort.
        //
        if (EFI_ERROR(Status) || SWM_MB_IDCANCEL == Result)
        {
            break;
        }

        // If the user selects OK, make sure both the password and the confirmation match then set it.
        //
        if (SWM_MB_IDOK == Result)
        {
            // If the new password string is invalid *and* it's not a null string (clear password), free the password buffer returned by the password dialog and try again.
            //
            if (FALSE == IsPwStringValid(PasswordBuffer, &PwdValidBitmap) && (PwdValidBitmap & PW_TEST_STRING_NULL) != PW_TEST_STRING_NULL)
            {
                // Select an appropriate error message.
                //
                if (PwdValidBitmap & PW_TEST_STRING_TOO_SHORT)
                {
                    // Password is too short.
                    //
                    pErrorMessage  = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_ERRORMSG_TOOSHORT), NULL);
                }
                else if (PwdValidBitmap & PW_TEST_STRING_TOO_LONG)
                {
                    // Password is too long.
                    //
                    pErrorMessage  = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_ERRORMSG_TOOLONG), NULL);
                }
                else if (PwdValidBitmap & PW_TEST_STRING_INVALID_CHAR)
                {
                    // Password contains invalid characters.
                    //
                    pErrorMessage  = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_ERRORMSG_INVALID_CHAR), NULL);
                }
                else
                {
                    // Some other (non-specific) failure.
                    //
                    pErrorMessage  = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_ERRORMSG_SET_GENFAILURE), NULL);
                }

                // If password buffer was used, make sure it's freed.
                //
                if (NULL != PasswordBuffer)
                {
                    SecureZeroMem ((UINT8*)PasswordBuffer, StrLen (PasswordBuffer) * sizeof(CHAR16) );
                    FreePool (PasswordBuffer);
                    PasswordBuffer = NULL;
                }

                continue;
            }

            // Otherwise, try setting the password.  If it fails, free the password buffer and try again.
            //
            Status = SetPassword (mAuthToken,
                                  PasswordBuffer
                                 );

            if (EFI_ERROR(Status))
            {

                // Select an appropriate error message.
                //
                if (EFI_SECURITY_VIOLATION == Status)
                {
                    // Password authentication error.
                    //
                    pErrorMessage  = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_ERRORMSG_AUTHERROR), NULL);
                }
                else
                {
                    // Some other (non-specific) failure.
                    //
                    pErrorMessage  = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_ERRORMSG_SET_GENFAILURE), NULL);
                }

                // If password buffer was used, make sure it's freed.
                //
                if (NULL != PasswordBuffer)
                {
                    SecureZeroMem ((UINT8*)PasswordBuffer, StrLen (PasswordBuffer) * sizeof(CHAR16) );
                    FreePool (PasswordBuffer);
                    PasswordBuffer = NULL;
                }

                continue;
            }

            break;
        }
    } while (TRUE);

    // If password buffer was used, make sure it's freed.
    //
    if (NULL != PasswordBuffer)
    {
        SecureZeroMem ((UINT8*)PasswordBuffer, StrLen (PasswordBuffer) * sizeof(CHAR16) );
        FreePool (PasswordBuffer);
        PasswordBuffer = NULL;
    }

    DEBUG((DEBUG_INFO, "INFO: [FP] SetSystemPassword: EXIT\r\n"));

    return Status;
}
*/

/**
  Presents the user with a (hopefully) helpful dialog
  with more info about a particular subject.

  NOTE: Subject is determined by the state of mCallbackKey.

  @retval   EFI_SUCCESS     Message successfully displayed.
  @retval   EFI_NOT_FOUND   mCallbackKey not recognized or string could not be loaded.
  @retval   Others          Return value of mSWMProtocol->MessageBox().

**/
STATIC
EFI_STATUS
HandleInfoPopup(
              IN  EFI_IFR_TYPE_VALUE                     *Value,
              OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
              )
{
    EFI_STATUS          Status = EFI_SUCCESS;
    EFI_STRING_ID       TitleId = 0, MessageId = 0, CaptionId = 0;
    CHAR16              *TitleBarText = NULL, *InfoMessage = NULL, *CaptionText = NULL;
    SWM_MB_RESULT       SwmResult;

    //
    // First, we need to determine which info message to display.
    switch (mCallbackKey)
    {
        case FRONT_PAGE_ACTION_SEC_DISPLAY_SB_WHAT_IS:
            TitleId = STRING_TOKEN (STR_SEC_SB_WHAT_IS_TITLE);
            CaptionId = STRING_TOKEN (STR_SEC_SB_WHAT_IS_LINK);
            MessageId = STRING_TOKEN (STR_SEC_SB_WHAT_IS_TEXT);
            break;
        case FRONT_PAGE_ACTION_SEC_DISPLAY_TPM_WHAT_IS:
            TitleId = STRING_TOKEN (STR_SEC_TPM_WHAT_IS_TITLE);
            CaptionId = STRING_TOKEN (STR_SEC_TPM_WHAT_IS_LINK);
            MessageId = STRING_TOKEN (STR_SEC_TPM_WHAT_IS_TEXT);
            break;

        default:
            Status = EFI_NOT_FOUND;
            break;
    }

    //
    // Next, attempt to load the string.
    if (!EFI_ERROR( Status ))
    {
        TitleBarText = (CHAR16*)HiiGetString (gStringPackHandle, TitleId, NULL);
        CaptionText = (CHAR16*)HiiGetString (gStringPackHandle, CaptionId, NULL);
        InfoMessage  = (CHAR16*)HiiGetString (gStringPackHandle, MessageId, NULL);
        if (NULL == InfoMessage || NULL == TitleBarText)
        {
            Status = EFI_NOT_FOUND;
        }
    }

    //
    // Finally, display the message to the user.
    if (!EFI_ERROR( Status ))
    {
        Status = mSWMProtocol->MessageBox (mSWMProtocol,
                                           TitleBarText,        // Dialog title bar text.
                                           InfoMessage,         // Dialog body text.
                                           CaptionText,         // Dialog caption text.
                                           SWM_MB_OK,           // Show only Ok button.
                                           0,                   // No timeout
                                           &SwmResult);         // Return result.
    }

    return Status;
}

STATIC
EFI_STATUS
HandleLanguage(
              IN  EFI_IFR_TYPE_VALUE                     *Value,
              OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
              )
{
    EFI_STATUS  Status = EFI_SUCCESS;
    CHAR8       *Lang, *LangCode;
    UINTN       Index;

    DEBUG((DEBUG_INFO, "INFO: [SurfaceFrontPage] Language is being changed.\r\n"));

    //
    // Allocate working buffer for RFC 4646 language in supported LanguageString.
    Lang = (CHAR8 *)AllocatePool( AsciiStrSize( mLanguageString ) );
    ASSERT (Lang != NULL);
    if (NULL == Lang)
    {
        return EFI_OUT_OF_RESOURCES;
    }

    //
    // Cycle through the language string until you
    // find the requested index.
    Index = 0;
    LangCode = mLanguageString;
    // Until we hit the end of the language string...
    while (*LangCode != 0)
    {
        // For each index (including 0) load the next language.
        GetNextLanguage (&LangCode, Lang);

        // If we've found the requested index,
        // let's stop here.
        if (Index == Value->u8)
        {
            break;
        }

        // Try all indices.
        Index++;
    }

    //
    // If we've found the selected string...
    if (Index == Value->u8)
    {
        // Set the PlatformLang variable to the newly selected language.
        Status = gRT->SetVariable( (CHAR16 *)L"PlatformLang",
                                   &gEfiGlobalVariableGuid,
                                   EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                   AsciiStrSize( Lang ),
                                   Lang );
    }
    // Otherwise, let's throw up an error because something weird has happened.
    else
    {
        DEBUG((DEBUG_INFO, "INFO: [SurfaceFrontPage] Could not find selected language!\r\n"));
        ASSERT(FALSE);
        Status = EFI_NOT_FOUND;
    }

    //
    // We need to exit the primary loop because we have to redraw the menus.
    *ActionRequest = EFI_BROWSER_ACTION_REQUEST_EXIT;

    //
    // Always put away your toys...
    FreePool( Lang );

    return Status;
}


/**
  Handle a request to change the SecureBoot configuration.

  @retval EFI_SUCCESS         Successfully updated SecureBoot default variables or user cancelled.
  @retval Others              Failed to update. SecureBoot state remains unchanged.

**/
/*
STATIC
EFI_STATUS
HandleSecureBootChange (
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
    EFI_STATUS      Status;
    CHAR16          *DialogTitleBarText, *DialogCaptionText, *DialogBodyText;
    // CHAR16          *FormattedRecommendation = NULL, *RecommendText;
    CHAR16          *Options[MS_SB_CONFIG_COUNT];
    // CHAR16          *RecommendedFormat = L"%s (%s)";
    SWM_MB_RESULT   SwmResult;
    // UINTN           SelectedIndex, RecommendedOptionIndex, TextBufferSize;
    UINTN           SelectedIndex;
    MS_SYSTEM_SETTING_FLAGS Flags = 0;

    //
    // Load UI dialog strings.
    DialogTitleBarText  = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_SB_CONFIG_TITLEBARTEXT), NULL);
    DialogCaptionText   = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_SB_CONFIG_CAPTION), NULL);
    DialogBodyText      = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_SB_CONFIG_BODY), NULL);

    //
    // Load UI option strings.
    // TODO: Do we want to recommend any of these options??
    Options[MS_SB_CONFIG_MS_ONLY] = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_SEC_SB_MS_ONLY_CONFIG_TEXT), NULL);
    Options[MS_SB_CONFIG_MS_3P]   = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_SEC_SB_MS_PLUS_CONFIG_TEXT), NULL);
    Options[MS_SB_CONFIG_NONE]    = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_GENERIC_TEXT_NONE), NULL);

    //
    // Add "recommended" to the recommended option.
    // NOTE: For now we're going to remove this, because it is at odds with the factory defaults (MS+3P).
    // RecommendedOptionIndex  = MS_SB_CONFIG_MS_ONLY;      // Recommend the "Microsoft only" option.
    // RecommendText           = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_GENERIC_TEXT_RECOMMEND), NULL);
    // Determine the size of the final buffer.
    // TextBufferSize          = (StrLen( Options[RecommendedOptionIndex] ) + StrLen( RecommendText ) + StrLen( RecommendedFormat )) * sizeof( CHAR16 );
    // FormattedRecommendation = AllocatePool( TextBufferSize );
    // If the buffer could be allocated, use it.
    // if (NULL != FormattedRecommendation)
    // {
    //     // Fill the buffer with the formatted string.
    //     UnicodeSPrint( FormattedRecommendation, TextBufferSize, RecommendedFormat, Options[RecommendedOptionIndex], RecommendText );
    //     // Replace the unformatted option with the formatted option.
    //     Options[RecommendedOptionIndex] = FormattedRecommendation;
    // }

    //
    // Display the dialog to the user.
    Status = mSWMProtocol->SingleSelectDialog (mSWMProtocol,
                                               DialogTitleBarText,
                                               DialogCaptionText,
                                               DialogBodyText,
                                               Options,
                                               MS_SB_CONFIG_COUNT,
                                               &SwmResult,
                                               &SelectedIndex);
    DEBUG(( DEBUG_INFO, "INFO [SFP] %a - SingleSelectDialog returning: Status = %r, SwmResult = 0x%X, SelectedIndex = %d\n", __FUNCTION__, Status, (UINTN)SwmResult, SelectedIndex ));

    //
    // If the form was submitted, process the update.
    if (!EFI_ERROR( Status ) && SWM_MB_IDOK == SwmResult)
    {
        Status = mSettingAccess->Set(mSettingAccess,
            MS_SYSTEM_SETTING_ID__SECURE_BOOT_KEYS_ENUM,
            &mAuthToken,
            MS_SYSTEM_SETTING_TYPE_SECUREBOOTKEYENUM,
            &SelectedIndex,
            &Flags);
        //
        // If successful, update the display.
        if (!EFI_ERROR( Status ))
        {
            DEBUG(( DEBUG_INFO, "%a - Successfully changed keys. Updating form browser and requesting restart.\n", __FUNCTION__ ));
            
            *ActionRequest = EFI_BROWSER_ACTION_REQUEST_FORM_APPLY;

            //
            // Indicate that this change should also trigger a reboot.
            mResetRequired = TRUE;

            //
            // Update the display strings.
            UpdateSecureBootStatusStrings( TRUE );

            //Delete the HSTI data from the variable. The tests have to be re-run if the secure boot state changes. The status of the HSTI variable should not affect the Front Page Ui Behavior.
            ClearCachedHsti();

        }
        else
        {
            DEBUG(( DEBUG_ERROR, "ERROR [SFP] %a - Failed to update SecureBoot config! %r\n", __FUNCTION__, Status ));
            DialogTitleBarText = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_SB_UPDATE_FAILURE_TITLE), NULL);
            DialogBodyText = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_SB_UPDATE_FAILURE), NULL);
            mSWMProtocol->MessageBox (mSWMProtocol,
                                       DialogTitleBarText,  // Dialog title bar text.
                                       DialogBodyText,      // Dialog body text.
                                       L"",                 // Dialog caption text.
                                       SWM_MB_OK,           // Show Ok button.
                                       0,                   // No timeout
                                       &SwmResult);         // Return result.
        }
    }

    //
    // Always put away your toys.
    // NOTE: See above.
    // NOTE: For now we're going to remove this, because it is at odds with the factory defaults (MS+3P).
    // if (NULL != FormattedRecommendation)
    // {
    //     FreePool( FormattedRecommendation );
    // }

    return Status;
}
*/

/**
  Determines the current SecureBoot state and updates the status strings accordingly.

  @param[in]  RefreshScreen     BOOLEAN indicating whether to force a screen refresh after updating the strings.

**/
/*
VOID
UpdateSecureBootStatusStrings (
  IN BOOLEAN        RefreshScreen
  )
{
  BOOLEAN       IsEnabled;
  UINTN         CurrentConfig;
  CHAR16        StateString[256];           // This is a somewhat arbitrary limit. Just needs to be large enough to encompass the largest possible string.
  CHAR16        *PreambleSubstring = NULL;
  CHAR16        *StateSubstring = NULL;
  CHAR16        *ConfigSubstring = NULL;
  CHAR16        *SuffixSubstring = NULL;

  //
  // No matter what the mode is, we need the preamble.
  PreambleSubstring = (CHAR16*)HiiGetString (gFrontPagePrivate.HiiHandle, STRING_TOKEN (STR_SEC_SB_STATE_PREAMBLE), NULL);

  //
  // Determine whether SecureBoot is enabled.
  IsEnabled = IsSecureBootEnable();

  //
  // If enabled, determine the current config.
  if (IsEnabled)
  {
    // Use the "Enabled" substring.
    StateSubstring = (CHAR16*)HiiGetString (gFrontPagePrivate.HiiHandle, STRING_TOKEN (STR_SEC_SB_STATE_ENABLED), NULL);
    SuffixSubstring = (CHAR16*)HiiGetString (gFrontPagePrivate.HiiHandle, STRING_TOKEN (STR_SEC_SB_KEY_CONFIG_TEXT), NULL);

    // Determine the ConfigSubstring.
    CurrentConfig = GetCurrentSecureBootConfig();
    if (MS_SB_CONFIG_MS_ONLY == CurrentConfig)
    {
        ConfigSubstring = (CHAR16*)HiiGetString (gFrontPagePrivate.HiiHandle, STRING_TOKEN (STR_SEC_SB_MS_ONLY_CONFIG_TEXT), NULL);
    }
    else if (MS_SB_CONFIG_MS_3P == CurrentConfig)
    {
        ConfigSubstring = (CHAR16*)HiiGetString (gFrontPagePrivate.HiiHandle, STRING_TOKEN (STR_SEC_SB_MS_PLUS_CONFIG_TEXT), NULL);
    }
    else
    {
        ConfigSubstring = (CHAR16*)HiiGetString (gFrontPagePrivate.HiiHandle, STRING_TOKEN (STR_SEC_SB_CUSTOM_CONFIG_TEXT), NULL);
    }

    UnicodeSPrint( StateString, sizeof( StateString ), L"%s %s %s %s",
                 PreambleSubstring,
                 StateSubstring,
                 ConfigSubstring,
                 SuffixSubstring );
  }
  //
  // If disabled, just present the state string.
  else
  {
    StateSubstring = (CHAR16*)HiiGetString (gFrontPagePrivate.HiiHandle, STRING_TOKEN (STR_SEC_SB_STATE_DISABLED), NULL);
    UnicodeSPrint( StateString, sizeof( StateString ), L"%s %s", PreambleSubstring, StateSubstring );
  }

  //
  // Finally, push the updates into the HiiDatabase.
  HiiSetString (gFrontPagePrivate.HiiHandle, STRING_TOKEN (STR_SEC_SB_STATE_TEXT), StateString, NULL);

  //
  // And notify the form that it needs to refresh.
  if (RefreshScreen)
  {
      RefreshSecurityForm();
  }
} // UpdateSecureBootStatusStrings()
*/

/**
  Handle a request to change the TPM enablement.

  @retval EFI_SUCCESS         Successfully updated TPM state.
  @retval Others              Failed to update. TPM state remains unchanged.

**/
/*
STATIC
EFI_STATUS
HandleTpmChange (
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
    EFI_STATUS              Status = EFI_SUCCESS;
    BOOLEAN    NewValue;
    CHAR16                  *DialogTitleBarText;
    CHAR16                  *DialogBodyText;
    SWM_MB_RESULT           SwmResult;
    static BOOLEAN          KnownReentry = FALSE;
    MS_SYSTEM_SETTING_FLAGS Flags = 0;

    DEBUG(( DEBUG_INFO, "INFO [%a] %a()\n", gEfiCallerBaseName, __FUNCTION__ ));

    NewValue = (*(UINT8*)Value == 0x1) ? TRUE : FALSE;

    //
    // REENTRY NOTE
    // Here's the deal...
    // See that 'EFI_BROWSER_ACTION_REQUEST_FORM_DISCARD' down there?
    // That's going to cause this function to re-enter as soon as we exit.
    // We only want to display the error message once, so let's go ahead and stop here.
    if (KnownReentry)
    {
        DEBUG(( DEBUG_INFO, "INFO [%a] %a - Exiting early from known-reentry.\n", gEfiCallerBaseName, __FUNCTION__ ));
        // Clear the known reentry.
        KnownReentry = FALSE;
        // Update the view.
        RefreshSecurityForm();
        // Exit.
        return EFI_SUCCESS;
    }

        DEBUG(( DEBUG_INFO, "%a - Call the settings provider to set the new TPM value.\n", __FUNCTION__ ));
        Status = mSettingAccess->Set(mSettingAccess,
            MS_SYSTEM_SETTING_ID__TPM_ENABLE,
            &mAuthToken,
            MS_SYSTEM_SETTING_TYPE_ENABLE,
            &NewValue,
            &Flags);

    if (!EFI_ERROR(Status)) {
        DEBUG((DEBUG_INFO, "%a - Successfully updated TPM. Updating form browser and requesting restart.\n", __FUNCTION__));   
        // Indicate that the change should ALSO be pushed to the permanent varstore.
        *ActionRequest = EFI_BROWSER_ACTION_REQUEST_FORM_APPLY;

        //
        // Indicate that this change should also trigger a reboot.
        mResetRequired = TRUE;

        //Delete the HSTI data from the variable. The tests have to be re-run if the tpm state changes. The status of the HSTI variable should not affect the Front Page Ui Behavior.
        ClearCachedHsti();

    }
    //
    // If there WAS an error, inform the user and roll back the change.
    else
    {
        DEBUG(( DEBUG_ERROR, "ERROR [SFP] %a - Failed to update TPM setting! %r\n", __FUNCTION__, Status ));
        DialogTitleBarText = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_SEC_TPM_UPDATE_FAILURE_TITLE), NULL);
        DialogBodyText = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_SEC_TPM_UPDATE_FAILURE), NULL);
        mSWMProtocol->MessageBox (mSWMProtocol,
                                   DialogTitleBarText,  // Dialog Title Bar text
                                   DialogBodyText,      // Dialog body text.
                                   L"",                 // Dialog caption text.
                                   SWM_MB_OK,           // Show Ok button.
                                   0,                   // No timeout
                                   &SwmResult);         // Return result.

        // Indicate that the change should be rejected.
        *ActionRequest = EFI_BROWSER_ACTION_REQUEST_FORM_DISCARD;
        // Indicate that we know a function reentry is coming. (See 'REENTRY NOTE' above).
        KnownReentry = TRUE;
        // Set the status to EFI_SUCCESS so the FormsBrowser will accurately discard the change.
        Status = EFI_SUCCESS;
    }

    return Status;
}
*/

/**
  Handle a request to reboot back into FrontPage.

  @retval EFI_SUCCESS

**/
STATIC
EFI_STATUS
HandleRebootToFrontPage (
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
    EFI_STATUS  Status;
    UINT32      Attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE;
    UINTN       DataSize;
    UINT8       OsIndications[sizeof( UINT64 )] = {0};      // For UINT32 Compatibility

    DEBUG(( DEBUG_INFO, "INFO [SFP] %a()\n", __FUNCTION__ ));

    //
    // Step 1: Read the current OS indications variable.
    DataSize = sizeof( OsIndications );
    Status = gRT->GetVariable( L"OsIndications",
                               &gEfiGlobalVariableGuid,
                               &Attributes,
                               &DataSize,
                               OsIndications );
    DEBUG(( DEBUG_VERBOSE, "VERBOSE [SFP] %a - GetVariable(OsIndications) = %r\n", __FUNCTION__, Status ));

    //
    // Step 2: Update OS indications variable to enable the boot to FrontPage.
    if (!EFI_ERROR( Status ) || Status == EFI_NOT_FOUND)
    {
        OsIndications[0] |= EFI_OS_INDICATIONS_BOOT_TO_FW_UI;   // Flag is located in the lowest byte.
        Status = gRT->SetVariable( L"OsIndications",
                                   &gEfiGlobalVariableGuid,
                                   Attributes,
                                   DataSize,
                                   OsIndications );
        DEBUG(( DEBUG_VERBOSE, "VERBOSE [SFP] %a - SetVariable(OsIndications) = %r\n", __FUNCTION__, Status ));
    }

    //
    // Step 3: Reboot!
    if (!EFI_ERROR( Status ))
    {
        DEBUG(( DEBUG_INFO, "INFO [SFP] %a - Requesting reboot...\n", __FUNCTION__ ));
        *ActionRequest = EFI_BROWSER_ACTION_REQUEST_EXIT;
        mResetRequired = TRUE;
    }

    return Status;
}
