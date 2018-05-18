/** @file
  User interaction functions for the Surface FrontPage.

  Copyright (c) 2015, Microsoft Corporation. All rights reserved.

**/

#include "FrontPage.h"        // TODO: Perhaps wrap the keys in their own .h file.
#include "FrontPageUi.h"
#include "Language.h"

#include <PiDxe.h>          // This has to be here so Protocol/FirmwareVolume2.h doesn't puke errors.

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

extern CHAR8                                *mLanguageString;
extern MS_ONSCREEN_KEYBOARD_PROTOCOL        *mOSKProtocol;
extern MS_SIMPLE_WINDOW_MANAGER_PROTOCOL    *mSWMProtocol;
extern EFI_HII_HANDLE                       gStringPackHandle;
extern FRONT_PAGE_CONFIGURATION             mFrontPageConfig;
extern CHAR16                               mConfigEFIVariableName[];
extern BOOLEAN                              mResetRequired;


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
