/** @file

  Implements the Surface UEFI Front Page (Settings Menu).

  Copyright (c) 2015, Microsoft Corporation. All rights reserved.<BR>

**/


#include "FrontPage.h"
#include "Language.h"
#include "FrontPageUi.h"
#include "FrontPageConfigAccess.h"

//#include <MsTpmReservedNvIndices.h>

#include <IndustryStandard/SmBios.h>

#include <Guid/FrontPageEventDataStruct.h>
#include <Guid/GlobalVariable.h>
//#include <Guid/MsNVRAMVariableGuid.h>
//#include <Guid/MsBootMenuGuid.h>
//#include <Guid/MsSemmMenuGuid.h>
//#include <Guid/MsSetTimeMenuGuid.h>
#include <Guid/MdeModuleHii.h>
#include <Guid/DebugImageInfoTable.h>

#include <Pi/PiFirmwareFile.h>

#include <Protocol/GraphicsOutput.h>
#include <Protocol/Smbios.h>
#include <Protocol/OnScreenKeyboard.h>
#include <Protocol/SimpleWindowManager.h>
#include <Protocol/FirmwareManagement.h>

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SecureMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/HiiLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootManagerLib.h>
//#include <Library/MsSecureBootLib.h>
//#include <Library/MsPasswordLib.h>
//#include <Library/FmpHelperLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/BmpSupportLib.h>
#include <Library/MsUiThemeLib.h>
#include <Library/ResetHelperLib.h>
#include <Library/MsLogoLib.h>
#include <Library/BootEventLogLib.h>

#include <MsDisplayEngine.h>
#include <UIToolKit/SimpleUIToolKit.h>

#include <Resources/MicrosoftLogo.h>

#pragma pack (push, 1)
///
/// Guid opcode.
///
typedef struct _EFI_IFR_GUID_BITMAP {
    EFI_IFR_OP_HEADER   Header;
    ///
    /// EFI_IFR_TIANO_GUID.
    ///
    EFI_GUID            Guid;
    ///
    /// Guid for Bitmap file
    ///
    EFI_GUID            BitmapFileGuid;
} EFI_IFR_GUID_BITMAP;
#pragma pack (pop)

//#include <MsIntSafe.h>

///
/// EFI boot mode
///
typedef UINT32  EFI_BOOT_MODE;


#define FP_OSK_WIDTH_PERCENT        75      // On-screen keyboard is 75% the width of the screen.

UINTN       mCallbackKey;
CHAR8       *mLanguageString;
EFI_HANDLE  mImageHandle;

// Protocols.
//
EFI_GRAPHICS_OUTPUT_PROTOCOL        *mGop;
EFI_HII_FONT_PROTOCOL               *mFont;

// UI Elements.
//
UINT32  mTitleBarWidth, mTitleBarHeight;
UINT32  mMasterFrameWidth, mMasterFrameHeight;
ListBox *mTopMenu;
BOOLEAN mShowFullMenu = FALSE;      // By default we won't show the full FrontPage menu (requires validation if there's a system password).
// About Menu is only needed if there is a about bitmap.
BOOLEAN mEnableAboutMenu;

// Master Frame - Form Notifications.
//
UINT32                            mCurrentFormIndex;
EFI_EVENT                         mMasterFrameNotifyEvent;
DISPLAY_ENGINE_SHARED_STATE       mDisplayEngineState;
BOOLEAN                           mTerminateFrontPage = FALSE;
BOOLEAN                           mResetRequired;
//FRONT_PAGE_AUTH_TOKEN_PROTOCOL    *mFrontPageAuthTokenProtocol = NULL;
//MS_AUTHENTICATION_PROTOCOL        *mAuthProtocol = NULL;
EFI_HII_CONFIG_ROUTING_PROTOCOL   *mHiiConfigRouting;
//MS_SYSTEM_SETTING_ACCESS_PROTOCOL *mSettingAccess;
//MS_AUTH_TOKEN                      mAuthToken;

#define MAX_PASSWORD_ATTEMPTS   3

extern EFI_HII_HANDLE gStringPackHandle;
extern EFI_GUID  gMsEventMasterFrameNotifyGroupGuid;

//
// Boot video resolution and text mode.
//
UINT32    mBootHorizontalResolution    = 0;
UINT32    mBootVerticalResolution      = 0;
UINT32    mBootTextModeColumn          = 0;
UINT32    mBootTextModeRow             = 0;
//
// BIOS setup video resolution and text mode.
//
UINT32    mSetupTextModeColumn         = 0;
UINT32    mSetupTextModeRow            = 0;
UINT32    mSetupHorizontalResolution   = 0;
UINT32    mSetupVerticalResolution     = 0;

EFI_FORM_BROWSER2_PROTOCOL          *mFormBrowser2;
MS_ONSCREEN_KEYBOARD_PROTOCOL       *mOSKProtocol;
MS_SIMPLE_WINDOW_MANAGER_PROTOCOL   *mSWMProtocol;

// Map Top Menu entries to HII Form IDs.
//
#define UNUSED_INDEX    (UINT16)-1
struct
{
    UINT16          FullMenuIndex;      // Master Frame full menu index.
    UINT16          LimitedMenuIndex;   // Master Frame limited menu index.
    EFI_STRING_ID   MenuString;         // Master Frame menu string.
    EFI_GUID        FormSetGUID;        // HII FormSet GUID.
    EFI_FORM_ID     FormId;             // HII Form ID.

} mFormMap[] =
{
//    Index (Full)  Index (Limited)     String                                      Formset Guid                       Form ID
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
//    { 0,            0,                  STRING_TOKEN (STR_MF_MENU_OP_PCINFO),       FRONT_PAGE_CONFIG_FORMSET_GUID,    FRONT_PAGE_FORM_ID_PCINFO   },  // PC info
//    { 1,            UNUSED_INDEX,       STRING_TOKEN (STR_MF_MENU_OP_SECURITY),     FRONT_PAGE_CONFIG_FORMSET_GUID,    FRONT_PAGE_FORM_ID_SECURITY },  // Security
//    { 2,            UNUSED_INDEX,       STRING_TOKEN (STR_MF_MENU_OP_DEVICES),      FRONT_PAGE_CONFIG_FORMSET_GUID,    FRONT_PAGE_FORM_ID_DEVICES  },  // Devices
//    { 3,            UNUSED_INDEX,       STRING_TOKEN (STR_MF_MENU_OP_BOOTORDER),    MS_BOOT_MENU_FORMSET_GUID,         MS_BOOT_ORDER_FORM_ID       },  // Boot Order
//    { 4,            1,                  STRING_TOKEN(STR_MF_MENU_OP_SETTIME),       MS_SET_TIME_MENU_FORMSET_GUID,     MS_SET_TIME_MENU_FORM_ID    },  // Set Time and Date
//    { 5,            2,                  STRING_TOKEN(STR_MF_MENU_OP_SEMM),          MS_SEMM_MENU_FORMSET_GUID,         MS_SEMM_MENU_FORM_ID        },  // SEMM
    { 0,            0,                  STRING_TOKEN(STR_MF_MENU_OP_ABOUT),         FRONT_PAGE_CONFIG_FORMSET_GUID,    FRONT_PAGE_FORM_ID_ABOUT    },  // About
    { 1,            1,                  STRING_TOKEN(STR_MF_MENU_OP_EXIT),          FRONT_PAGE_CONFIG_FORMSET_GUID,    FRONT_PAGE_FORM_ID_EXIT     }   // Exit
};

// Surface Frontpage form set GUID
//
EFI_GUID gMsFrontPageConfigFormSetGuid = FRONT_PAGE_CONFIG_FORMSET_GUID;

#pragma pack(1)

///
/// HII specific Vendor Device Path definition.
///
typedef struct
{
    VENDOR_DEVICE_PATH             VendorDevicePath;
    EFI_DEVICE_PATH_PROTOCOL       End;
} HII_VENDOR_DEVICE_PATH;

#pragma pack()

FRONT_PAGE_CALLBACK_DATA  gFrontPagePrivate = {
    FRONT_PAGE_CALLBACK_DATA_SIGNATURE,
    NULL,
    NULL,
    NULL,
    {
        ExtractConfig,    // Lives in FrontPageConfigAccess.c
        RouteConfig,      // Lives in FrontPageConfigAccess.c
        UiCallback          // Lives in FrontPageUi.c
    }
};

HII_VENDOR_DEVICE_PATH  mFrontPageHiiVendorDevicePath = {
    {
        {
            HARDWARE_DEVICE_PATH,
            HW_VENDOR_DP,
            {
                (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
                (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
            }
        },
        FRONT_PAGE_CONFIG_FORMSET_GUID
    },
    {
        END_DEVICE_PATH_TYPE,
        END_ENTIRE_DEVICE_PATH_SUBTYPE,
        {
            (UINT8) (END_DEVICE_PATH_LENGTH),
            (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
        }
    }
};

#define FP_CALLBACK_DATA_FROM_THIS(a)  CR (a, FRONT_PAGE_CALLBACK_DATA, ConfigAccess, FRONT_PAGE_CALLBACK_DATA_SIGNATURE)

EFI_STATUS GetAndDisplayBitmap(EFI_GUID *FileGuid, UINTN XCoord, BOOLEAN XCoordAdj);

//EFI_STATUS GetAuthToken(CHAR16 *PasswordBuffer);

/**
  Updates HII display strings based on associated EFI variable state.

@param[in]  HiiHandle       - Hii database handle.

@retval     EFI_SUCCESS     Updated display strings.

**/
/*
static
EFI_STATUS
UpdateDisplayStrings (IN    EFI_HII_HANDLE  HiiHandle)
{
  EFI_STATUS        Status = EFI_SUCCESS;
  UINTN             Count;
  UINTN             AsciiStringSize;
  CHAR8             AsciiString[MAX_STRING_LENGTH + 1];         // Includes NULL terminator.
  CHAR16            UnicodeString[MAX_STRING_LENGTH + 1];       // Includes NULL terminator.

  // List of display strings to be updated and their associated EFI variables.
  //
  struct
  {
    EFI_STRING_ID     HiiStringID;      // HII string ID.
    EFI_GUID          *VariableGUID;    // EFI variable GUID.
    CHAR16            *VariableName;    // EFI variable name.
  } DisplayElements[] =
  {
    { STRING_TOKEN (STR_INF_VIEW_PC_ASSET_TAG_VALUE),    &gMsNVRAMVariableGuid,   MS_ASSET_TAG_VARIABLE_NAME_PUBLIC  },        // Asset Tag.
    { STRING_TOKEN (STR_INF_VIEW_PC_SERIALNUM_VALUE),    &gMsNVRAMVariableGuid,   MS_SERIAL_NUMBER_VARIABLE_NAME     },        // Chassis Serial Number.
    {      STRING_TOKEN (STR_INF_VIEW_PC_UUID_VALUE),    &gMsNVRAMVariableGuid,   MS_UUID_VARIABLE_NAME              },        // UUID.
    {     STRING_TOKEN (STR_INF_VIEW_PC_MODEL_VALUE),    &gMsNVRAMVariableGuid,   MS_PRODUCT_NAME_VARIABLE_NAME      }         // Product Name.
  };


  // Loop through each of the display elements and update the associated string if the corresponding NVRAM variable is found.
  //
  for (Count=0 ; Count < (sizeof(DisplayElements) / sizeof(DisplayElements[0])); Count++)
  {
    AsciiString[0]      =  '\0';
    UnicodeString[0]    = L'\0';
    AsciiStringSize     = ((MAX_STRING_LENGTH + 1) * sizeof(CHAR8));

    Status = gRT->GetVariable (DisplayElements[Count].VariableName,
                               DisplayElements[Count].VariableGUID,
                               (UINT32 *)NULL,
                               &AsciiStringSize,
                               AsciiString
                              );

    // Got and error?  We'll use the default string, continue to the next variable...
    //
    if (EFI_ERROR(Status))
    {
        continue;
    }

    // Update the display string if we found a valid value.
    //
    if (STRING_TOKEN(STR_INF_VIEW_PC_UUID_VALUE) == DisplayElements[Count].HiiStringID &&
        AsciiStringSize == sizeof(EFI_GUID))
    {
      // Format the UUID for display.
      //
      UnicodeSPrint (UnicodeString,
                     (MAX_STRING_LENGTH * sizeof(CHAR16)),
                     L"%g",
                     (EFI_GUID *)AsciiString
                    );

      // Update the HII string.
      //
      HiiSetString (HiiHandle, DisplayElements[Count].HiiStringID, UnicodeString, NULL);
    }
    else if (AsciiString[0] != '\0')
    {
      // Convert the ASCII string to Unicode.
      //
      AsciiStrToUnicodeStr(AsciiString, UnicodeString);

      // Update the HII string.
      //
      HiiSetString (HiiHandle, DisplayElements[Count].HiiStringID, UnicodeString, NULL);
    }
  }

  return Status;
}
*/

/**
Function to populate the PC INFO firmware version form with the current fw versions
found using FMP.



**/
/*
VOID
UpdateFormWithFirmwareVersions(IN EFI_HII_HANDLE  HiiHandle) {
  EFI_STATUS                    Status;
  VOID                         *StartOpCodeHandle;
  VOID                         *EndOpCodeHandle = NULL;
  EFI_IFR_GUID_LABEL           *StartLabel;
  EFI_IFR_GUID_LABEL           *EndLabel;
  EFI_STRING_ID                 StringId;
  EFI_STRING_ID                 StringId1;

  EFI_FIRMWARE_MANAGEMENT_PROTOCOL**              FmpList;
  EFI_FIRMWARE_MANAGEMENT_PROTOCOL**              Fmp;
  UINTN                                         DescriptorSize;
  EFI_FIRMWARE_IMAGE_DESCRIPTOR                 *FmpImageInfoBuf;
  UINT8                                         FmpImageInfoCount;
  UINT32                                        FmpImageInfoDescriptorVer;
  UINTN                                         ImageInfoSize;
  UINT32                                        PackageVersion;
  CHAR16                                        *PackageVersionName;

  FmpImageInfoBuf = NULL;
  Fmp = NULL;
  FmpList = NULL;
  PackageVersionName = NULL;

  do {
    //
    // Init OpCode Handle and Allocate space for creation of UpdateData Buffer
    //
    StartOpCodeHandle = HiiAllocateOpCodeHandle();
    if (StartOpCodeHandle == NULL)
    {
      ASSERT(StartOpCodeHandle != NULL);
      break;
    }

    EndOpCodeHandle = HiiAllocateOpCodeHandle();
    if (EndOpCodeHandle == NULL)
    {
      ASSERT(EndOpCodeHandle != NULL);
      break;
    }

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

    StartLabel->Number = LABEL_PCINFO_FW_VERSION_TAG_START;
    EndLabel->Number = LABEL_PCINFO_FW_VERSION_TAG_END;

    //
    // Get all FMP instances and then use the descriptor to get string name and version
    //
    Status = GetAllFmp(&FmpList);
    if (EFI_ERROR(Status))
    {
      DEBUG((DEBUG_ERROR, "GetAllFmp returned error.  %r \n", Status));
      break;
    }

    for (Fmp = FmpList; *Fmp != NULL; Fmp++)
    {
      //get the GetImageInfo for the FMP

      ImageInfoSize = 0;
      //
      // get necessary descriptor size
      // this should return TOO SMALL
      Status = (*Fmp)->GetImageInfo(
        (*Fmp),                       // FMP Pointer
        &ImageInfoSize,               // Buffer Size (in this case 0)
        NULL,                         // NULL so we can get size
        &FmpImageInfoDescriptorVer,   // DescriptorVersion
        &FmpImageInfoCount,           // DescriptorCount
        &DescriptorSize,              // DescriptorSize
        &PackageVersion,              // PackageVersion
        &PackageVersionName           // PackageVersionName
        );

      if (Status != EFI_BUFFER_TOO_SMALL) {
        DEBUG((DEBUG_ERROR, "%a - Unexpected Failure in GetImageInfo.  Status = %r\n", __FUNCTION__, Status));
        continue;
      }

      FmpImageInfoBuf = NULL;
      FmpImageInfoBuf = AllocateZeroPool(ImageInfoSize);
      if (FmpImageInfoBuf == NULL) {
        DEBUG((DEBUG_ERROR, "%a - Failed to get memory for descriptors.\n", __FUNCTION__));
        continue;
      }

      PackageVersionName = NULL;
      Status = (*Fmp)->GetImageInfo(
        (*Fmp),
        &ImageInfoSize,               // ImageInfoSize
        FmpImageInfoBuf,              // ImageInfo
        &FmpImageInfoDescriptorVer,   // DescriptorVersion
        &FmpImageInfoCount,           // DescriptorCount
        &DescriptorSize,              // DescriptorSize
        &PackageVersion,              // PackageVersion
        &PackageVersionName           // PackageVersionName
        );

      if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "%a - Failure in GetImageInfo.  Status = %r\n", __FUNCTION__, Status));
        goto FmpCleanUp;
      }

      if (FmpImageInfoCount == 0)
      {
        DEBUG((DEBUG_INFO, "%a - No Image Info descriptors.\n", __FUNCTION__));
        goto FmpCleanUp;
      }

      if (FmpImageInfoCount > 1)
      {
        DEBUG((DEBUG_INFO, "%a - Found %d descriptors.  For FrontPage we only show the 1st descriptor.\n", __FUNCTION__, FmpImageInfoCount));
      }

      if ((StringId = HiiSetString(HiiHandle, 0, FmpImageInfoBuf->ImageIdName, NULL)) == 0)
      {
        DEBUG((DEBUG_ERROR, "%a - Failed to set string for fmp ImageIdName: %s. \n", __FUNCTION__, FmpImageInfoBuf->ImageIdName));
        goto FmpCleanUp;
      }

      if ((StringId1 = HiiSetString(HiiHandle, 0, FmpImageInfoBuf->VersionName, NULL)) == 0)
      {
        DEBUG((DEBUG_ERROR, "%a - Failed to set string for fmp VersionName: %s. \n", __FUNCTION__, FmpImageInfoBuf->VersionName));
        goto FmpCleanUp;
      }

      // Create a Subtitle OpCode to group the Firmware version "key-value" pair that follows.
      //
      HiiCreateSubTitleOpCode (StartOpCodeHandle,
                               STRING_TOKEN(STR_NULL_STRING),
                               STRING_TOKEN(STR_NULL_STRING),
                               EFI_IFR_FLAGS_HORIZONTAL,
                               0
                              );

      HiiCreateTextOpCode(StartOpCodeHandle,
                          StringId,
                          STRING_TOKEN(STR_NULL_STRING),
                          STRING_TOKEN(STR_NULL_STRING)
                         );

      HiiCreateTextOpCode(StartOpCodeHandle,
                          StringId1,
                          STRING_TOKEN(STR_NULL_STRING),
                          STRING_TOKEN(STR_NULL_STRING)
                         );

      // Create empty 3rd text Opcode add an additional column to the display grid, thus moving the firmware version to the left (better alignment).
      //
      HiiCreateTextOpCode(StartOpCodeHandle,
                          STRING_TOKEN(STR_NULL_STRING),
                          STRING_TOKEN(STR_NULL_STRING),
                          STRING_TOKEN(STR_NULL_STRING)
                         );

    FmpCleanUp:
      //clean up -
      FreePool(FmpImageInfoBuf);
      FmpImageInfoBuf = NULL;
      if (PackageVersionName != NULL)
      {
        FreePool(PackageVersionName);
        PackageVersionName = NULL;
      }
    } //for loop for all fmp handles

    //Free up the FmpList of pointers
    if (FmpList != NULL) { FreePool(FmpList); }

    Status = HiiUpdateForm(HiiHandle,                          // HII handle
      &gMsFrontPageConfigFormSetGuid,     // Formset GUID
      FRONT_PAGE_FORM_ID_PCINFO,          // Form ID
      StartOpCodeHandle,                  // Label for where to insert opcodes
      EndOpCodeHandle                     // Replace data
      );
  } while (FALSE);

  if (StartOpCodeHandle != NULL) {
    HiiFreeOpCodeHandle(StartOpCodeHandle);
  }

  if (EndOpCodeHandle != NULL) {
    HiiFreeOpCodeHandle(EndOpCodeHandle);
  }
}
*/


/**
Function to populate About Menu bitmap.

@param[in]  HiiHandle       - Hii database handle.

@retval     None.

**/
VOID
UpdateAboutForm (
                IN EFI_HII_HANDLE  HiiHandle
                ) 
{
  EFI_STATUS                   Status;
  VOID                         *StartOpCodeHandle;
  VOID                         *EndOpCodeHandle = NULL;
  EFI_IFR_GUID_LABEL           *StartLabel;
  EFI_IFR_GUID_LABEL           *EndLabel;
    
  EFI_IFR_GUID_BITMAP          *ComplianceLabelOpCode;
  EFI_GUID                     SurfaceBitmapGuid = SURFACE_BITMAP_OPCODE_GUID;

  do {
    //
    // Init OpCode Handle and Allocate space for creation of UpdateData Buffer
    //
    StartOpCodeHandle = HiiAllocateOpCodeHandle();
    if (StartOpCodeHandle == NULL)
    {
      ASSERT(StartOpCodeHandle != NULL);
      break;
    }

    EndOpCodeHandle = HiiAllocateOpCodeHandle();
    if (EndOpCodeHandle == NULL)
    {
      ASSERT(EndOpCodeHandle != NULL);
      break;
    }

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

    StartLabel->Number = LABEL_ABOUT_COMPLIANCE_LABLEL_TAG_START;
    EndLabel->Number = LABEL_ABOUT_COMPLIANCE_LABLEL_TAG_END;

    //
    // Update the image
    //
    ComplianceLabelOpCode = (EFI_IFR_GUID_BITMAP *)HiiCreateGuidOpCode(StartOpCodeHandle,
                                                                        &SurfaceBitmapGuid,
                                                                        NULL,
                                                                        sizeof(EFI_IFR_GUID_BITMAP)
                                                                       );

    // Note that we cannot use CopyGuid here becaue the destination address is not aligned and compiler does not like it
    CopyMem(&(ComplianceLabelOpCode->BitmapFileGuid), PcdGetPtr(PcdComplianceLabelFile), sizeof(EFI_GUID));

    Status = HiiUpdateForm(HiiHandle,                           // HII handle
                            &gMsFrontPageConfigFormSetGuid,     // Formset GUID
                            FRONT_PAGE_FORM_ID_ABOUT,           // Form ID
                            StartOpCodeHandle,                  // Label for where to insert opcodes
                            EndOpCodeHandle                     // Replace data
                            );
  
    ASSERT_EFI_ERROR(Status);

  } while (FALSE);

  if (StartOpCodeHandle != NULL) {
    HiiFreeOpCodeHandle(StartOpCodeHandle);
  }

  if (EndOpCodeHandle != NULL) {
    HiiFreeOpCodeHandle(EndOpCodeHandle);
  }
}

/**
  Initialize HII information for the FrontPage


  @param InitializeHiiData    TRUE if HII elements need to be initialized.

  @retval  EFI_SUCCESS        The operation is successful.
  @retval  EFI_DEVICE_ERROR   If the dynamic opcode creation failed.

**/
EFI_STATUS
InitializeFrontPage (
                    IN BOOLEAN    InitializeHiiData
                    )
{
    EFI_STATUS                  Status = EFI_SUCCESS;
    CHAR16                      *StringBuffer;
    EFI_HII_HANDLE              HiiHandle;
#if 0
    // MSchange - [UI] Remove language menu selection.  Translation to be done later.
    CHAR8                       *LangCode;
    CHAR8                       *Lang;
    CHAR8                       *CurrentLang;
    UINTN                       OptionCount;
    VOID                        *OptionsOpCodeHandle;
    VOID                        *StartOpCodeHandle;
    VOID                        *EndOpCodeHandle;
    EFI_IFR_GUID_LABEL          *StartLabel;
    EFI_IFR_GUID_LABEL          *EndLabel;
    EFI_HII_STRING_PROTOCOL     *HiiString;
    UINTN                       StringSize;
    UINTN Size;

    Lang         = NULL;
#endif

    StringBuffer = NULL;

    if (InitializeHiiData)
    {

        mCallbackKey  = 0;

        //
        // Locate Hii relative protocols
        //
        Status = gBS->LocateProtocol (&gEfiFormBrowser2ProtocolGuid, NULL, (VOID **) &mFormBrowser2);
        if (EFI_ERROR (Status))
        {
            return Status;
        }
        Status = gBS->LocateProtocol (&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID **) &mHiiConfigRouting);
        if (EFI_ERROR (Status))
        {
            return Status;
        }

        //
        // Install Device Path Protocol and Config Access protocol to driver handle
        //
        Status = gBS->InstallMultipleProtocolInterfaces (
                                                        &gFrontPagePrivate.DriverHandle,
                                                        &gEfiDevicePathProtocolGuid,
                                                        &mFrontPageHiiVendorDevicePath,
                                                        &gEfiHiiConfigAccessProtocolGuid,
                                                        &gFrontPagePrivate.ConfigAccess,
                                                        NULL
                                                        );

        ASSERT_EFI_ERROR (Status);

        //
        // Publish our HII data
        //
        gFrontPagePrivate.HiiHandle = HiiAddPackages (
                                                     &gMsFrontPageConfigFormSetGuid,
                                                     gFrontPagePrivate.DriverHandle,
                                                     FrontPageVfrBin,
                                                     FrontPageStrings,
                                                     NULL
                                                     );
        if (gFrontPagePrivate.HiiHandle == NULL)
        {
            return EFI_OUT_OF_RESOURCES;
        }
    }

    // TODO - Get rid of global.
    //
    HiiHandle = gFrontPagePrivate.HiiHandle;

    // Update PC information display strings from EFI variables.
    //
    //UpdateDisplayStrings (HiiHandle);

    //UpdateFormWithFirmwareVersions(HiiHandle);
    UpdateAboutForm(HiiHandle);

    //UpdateSecureBootStatusStrings( FALSE );


    // MSchange - [UI] Remove language menu selection.  Translation to be done later.
#if 0
    //
    // Init OpCode Handle and Allocate space for creation of UpdateData Buffer
    //
    StartOpCodeHandle = HiiAllocateOpCodeHandle ();
    ASSERT (StartOpCodeHandle != NULL);

    EndOpCodeHandle = HiiAllocateOpCodeHandle ();
    ASSERT (EndOpCodeHandle != NULL);

    OptionsOpCodeHandle = HiiAllocateOpCodeHandle ();
    ASSERT (OptionsOpCodeHandle != NULL);
    //
    // Create Hii Extend Label OpCode as the start opcode
    //
    StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
    StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
    StartLabel->Number       = LABEL_SELECT_LANGUAGE;

    //
    // Create Hii Extend Label OpCode as the end opcode
    //
    EndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
    EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
    EndLabel->Number       = LABEL_END;

    //
    // Collect the languages from what our current Language support is based on our VFR
    //

    GetEfiGlobalVariable2(L"PlatformLang", (VOID**) &CurrentLang, &Size);

    if (CurrentLang != NULL){
        if (*(CurrentLang + Size - 1) != '\0'){
            DEBUG((DEBUG_ERROR, " Language Variable is not Null terminated \n"));
            CurrentLang = NULL;
        }
    }

    // Get Support language list from variable.
    //
    if (mLanguageString == NULL)
    {
        mLanguageString = GetEfiGlobalVariable (L"PlatformLangCodes");
        if (mLanguageString == NULL)
        {
            mLanguageString = AllocateCopyPool (
                                               AsciiStrSize ((CHAR8 *) PcdGetPtr (PcdUefiVariableDefaultPlatformLangCodes)),
                                               (CHAR8 *) PcdGetPtr (PcdUefiVariableDefaultPlatformLangCodes)
                                               );
            ASSERT (mLanguageString != NULL);
        }
    }

    if (gFrontPagePrivate.LanguageToken == NULL)
    {
        //
        // Count the language list number.
        //
        LangCode      = mLanguageString;
        Lang          = AllocatePool (AsciiStrSize (mLanguageString));
        ASSERT (Lang != NULL);
        OptionCount = 0;
        while (*LangCode != 0)
        {
            GetNextLanguage (&LangCode, Lang);
            OptionCount ++;
        }

        //
        // Allocate extra 1 as the end tag.
        //
        gFrontPagePrivate.LanguageToken = AllocateZeroPool ((OptionCount + 1) * sizeof (EFI_STRING_ID));
        ASSERT (gFrontPagePrivate.LanguageToken != NULL);

        Status = gBS->LocateProtocol (&gEfiHiiStringProtocolGuid, NULL, (VOID **) &HiiString);
        ASSERT_EFI_ERROR (Status);

        LangCode     = mLanguageString;
        OptionCount  = 0;
        while (*LangCode != 0)
        {
            GetNextLanguage (&LangCode, Lang);

            StringSize = 0;
            Status = HiiString->GetString (HiiString, Lang, HiiHandle, PRINTABLE_LANGUAGE_NAME_STRING_ID, StringBuffer, &StringSize, NULL);
            if (Status == EFI_BUFFER_TOO_SMALL)
            {
                StringBuffer = AllocateZeroPool (StringSize);
                ASSERT (StringBuffer != NULL);
                Status = HiiString->GetString (HiiString, Lang, HiiHandle, PRINTABLE_LANGUAGE_NAME_STRING_ID, StringBuffer, &StringSize, NULL);
                ASSERT_EFI_ERROR (Status);
            }

            if (EFI_ERROR (Status))
            {
                StringBuffer = AllocatePool (AsciiStrSize (Lang) * sizeof (CHAR16));
                ASSERT (StringBuffer != NULL);
                AsciiStrToUnicodeStr (Lang, StringBuffer);
            }

            ASSERT (StringBuffer != NULL);
            gFrontPagePrivate.LanguageToken[OptionCount] = HiiSetString (HiiHandle, 0, StringBuffer, NULL);
            FreePool (StringBuffer);

            OptionCount++;
        }
    }

    ASSERT (gFrontPagePrivate.LanguageToken != NULL);
    LangCode     = mLanguageString;
    OptionCount  = 0;
    if (Lang == NULL)
    {
        Lang = AllocatePool (AsciiStrSize (mLanguageString));
        ASSERT (Lang != NULL);
    }
    while (*LangCode != 0)
    {
        GetNextLanguage (&LangCode, Lang);

        if (CurrentLang != NULL && AsciiStrCmp (Lang, CurrentLang) == 0)
        {
            HiiCreateOneOfOptionOpCode (
                                       OptionsOpCodeHandle,
                                       gFrontPagePrivate.LanguageToken[OptionCount],
                                       EFI_IFR_OPTION_DEFAULT,
                                       EFI_IFR_NUMERIC_SIZE_1,
                                       (UINT8) OptionCount
                                       );
        }
        else
        {
            HiiCreateOneOfOptionOpCode (
                                       OptionsOpCodeHandle,
                                       gFrontPagePrivate.LanguageToken[OptionCount],
                                       0,
                                       EFI_IFR_NUMERIC_SIZE_1,
                                       (UINT8) OptionCount
                                       );
        }

        OptionCount++;
    }

    if (CurrentLang != NULL)
    {
        FreePool (CurrentLang);
    }
    FreePool (Lang);

    HiiCreateOneOfOpCode (
                         StartOpCodeHandle,
                         FRONT_PAGE_KEY_LANGUAGE,
                         0,
                         0,
                         STRING_TOKEN (STR_LANGUAGE_SELECT),
                         STRING_TOKEN (STR_NULL_STRING),
                         EFI_IFR_FLAG_CALLBACK,
                         EFI_IFR_NUMERIC_SIZE_1,
                         OptionsOpCodeHandle,
                         NULL
                         );

    Status = HiiUpdateForm (
                           HiiHandle,
                           &gMsFrontPageConfigFormSetGuid,
                           FRONT_PAGE_FORM_ID,
                           StartOpCodeHandle, // LABEL_SELECT_LANGUAGE
                           EndOpCodeHandle    // LABEL_END
                           );

    HiiFreeOpCodeHandle (StartOpCodeHandle);
    HiiFreeOpCodeHandle (EndOpCodeHandle);
    HiiFreeOpCodeHandle (OptionsOpCodeHandle);
#endif

    return Status;
}


/**
  Uninitialize HII information for the FrontPage


  @param InitializeHiiData    TRUE if HII elements need to be initialized.

  @retval  EFI_SUCCESS        The operation is successful.
  @retval  EFI_DEVICE_ERROR   If the dynamic opcode creation failed.

**/
EFI_STATUS
UninitializeFrontPage (
                      VOID
                      )
{
    EFI_STATUS Status = EFI_SUCCESS;

    /*
    //Dispose the auth token we acquired for the front page.
    if (mAuthProtocol != NULL) {
        Status = mAuthProtocol->DisposeAuthToken(mAuthProtocol, &mAuthToken);
        if (EFI_ERROR(Status)){
            DEBUG((EFI_D_ERROR, " Dispose Auth Token Failed %r\n", Status));
        }
    }

    if (NULL != mFrontPageAuthTokenProtocol) {
        Status = gBS->UninstallMultipleProtocolInterfaces(
            mImageHandle,
            &gMsFrontPageAuthTokenProtocolGuid,
            mFrontPageAuthTokenProtocol,
            NULL
            );
        FreePool (mFrontPageAuthTokenProtocol);
        mFrontPageAuthTokenProtocol = NULL;
        ASSERT_EFI_ERROR(Status);
    }
    */

    Status = gBS->UninstallMultipleProtocolInterfaces (
                                                      gFrontPagePrivate.DriverHandle,
                                                      &gEfiDevicePathProtocolGuid,
                                                      &mFrontPageHiiVendorDevicePath,
                                                      &gEfiHiiConfigAccessProtocolGuid,
                                                      &gFrontPagePrivate.ConfigAccess,
                                                      NULL
                                                      );
    ASSERT_EFI_ERROR (Status);
    //
    // Remove our published HII data
    //
    HiiRemovePackages (gFrontPagePrivate.HiiHandle);
    if (gFrontPagePrivate.LanguageToken != NULL)
    {
        FreePool (gFrontPagePrivate.LanguageToken);
        gFrontPagePrivate.LanguageToken = (EFI_STRING_ID *)NULL;
    }

    gBS->CloseEvent(mMasterFrameNotifyEvent);

    // TODO: Destroy the FrontPageContext variable. This way others can know whether we're in Setup.
    // TODO: Enable UserPhysicalPresence:
    //              - Before ReadyToBoot()
    //              - IN Setup

    return Status;
}


/**
  Call the browser and display the front page

  @return   Status code that will be returned by
            EFI_FORM_BROWSER2_PROTOCOL.SendForm ().

**/
EFI_STATUS
CallFrontPage (IN UINT32    FormIndex)
{
    EFI_STATUS                    Status = EFI_SUCCESS;
    UINT16  Count, Index = 0;
    EFI_BROWSER_ACTION_REQUEST    ActionRequest;
    EFI_HII_HANDLE                Handles[4];
    UINTN                         HandleCount;


    // Locate Boot Menu form - this should already be registered.
    //
    // TODO - integrate menu into FP?
    //
    /*
    EFI_GUID          BootMenu = MS_BOOT_MENU_FORMSET_GUID;
    EFI_GUID          SemmMenu = MS_SEMM_MENU_FORMSET_GUID;
    EFI_GUID          SetTimeMenu = MS_SET_TIME_MENU_FORMSET_GUID;
    EFI_HII_HANDLE    *BootHandle  = HiiGetHiiHandles(&BootMenu);
    EFI_HII_HANDLE    *SemmHandle  = HiiGetHiiHandles(&SemmMenu);
    EFI_HII_HANDLE    *SetTimeHandle  = HiiGetHiiHandles(&SetTimeMenu);
    */

    Handles[0] = gFrontPagePrivate.HiiHandle;
    HandleCount = 1;

    /*
    if (BootHandle != NULL)
        Handles[HandleCount++] = BootHandle[0];
    if (SemmHandle != NULL)
        Handles[HandleCount++] = SemmHandle[0];
    if (SetTimeHandle != NULL)
        Handles[HandleCount++] = SetTimeHandle[0];
    */

    ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;


    // Search through the form mapping table to find the form set GUID and ID corresponding to the selected index.
    //
    for (Count=0 ; Count < (sizeof(mFormMap) / sizeof(mFormMap[0])); Count++)
    {
        Index = ((FALSE == mShowFullMenu) ? mFormMap[Count].LimitedMenuIndex : mFormMap[Count].FullMenuIndex);

        if (Index == FormIndex)
        {
            break;
        }
    }

    // If we didn't find it, exit with an error.
    //
    if (Index != FormIndex)
    {
        Status = EFI_NOT_FOUND;
        goto Exit;
    }

    // Call the browser to display the selected form.
    //
    Status = mFormBrowser2->SendForm (mFormBrowser2,
                                      Handles,
                                      HandleCount,
                                      &mFormMap[Count].FormSetGUID,
                                      mFormMap[Count].FormId,
                                      (EFI_SCREEN_DESCRIPTOR *)NULL,
                                      &ActionRequest
                                     );

    // If the user selected the "Restart now" button to exit the Surface Frontpage, set the exit flag.
    //
    if (ActionRequest == EFI_BROWSER_ACTION_REQUEST_EXIT)
    {
        mTerminateFrontPage = TRUE;
    }

    // Check whether user change any option setting which needs a reset to be effective
    //
    if (ActionRequest == EFI_BROWSER_ACTION_REQUEST_RESET)
    {
        mResetRequired = TRUE;
    }

Exit:

    return Status;
}


/**
  Present user with password prompt and attempt to validate password.

  NOTE: If user enters password incorrectly too many times, return FALSE.

  @param[in]  MaxAttempts   The number of invalid password attempts before the
                            system will halt with an appropriate message.
                            If 0, user receives unlimited opportunites.

  @retval     TRUE    User entered the password correctly.
  @retval     FALSE   User cancelled password attempt or failed to authenticate.

**/
/*
static
BOOLEAN
ChallengeUserPassword(IN  UINTN    MaxAttempts)
{
    EFI_STATUS      Status;
    SWM_MB_RESULT   SwmResult           = 0;
    CHAR16          *pErrorMessage      = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_NULL_STRING), NULL);
    CHAR16          *PasswordBuffer     = NULL;    // This will be allocated by PasswordDialog(). Needs to be tracked, wiped, and freed.
    BOOLEAN         Result = FALSE, AttemptsExpired = FALSE;

    // Primary UI loop.
    // Display prompt. Process results.
    // This will loop forever if MaxAttempts is 0 and user continues to enter invalid passwords.
    //
    do
    {
        // Present the password dialog to prompt the user.
        //
        Status = mSWMProtocol->PasswordDialog (mSWMProtocol,
                                                HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_ENTER_PWD_TITLEBARTEXT), NULL),   // Dialog titlebar text.
                                                HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_CAPTION), NULL),                  // Dialog caption text.
                                                HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_ENTER_BODYTEXT), NULL),           // Dialog body text.
                                                pErrorMessage,
                                                SWM_PWD_TYPE_PROMPT_PASSWORD,
                                                &SwmResult,
                                                &PasswordBuffer
                                                );

        // Check for errors and whether the user selected cancel.
        //
        if (EFI_ERROR(Status) || SWM_MB_IDCANCEL == SwmResult)
        {
            break;  // Return FALSE.
        }

        // If the user selected "OK", check whether the password provided is valid.
        //
        if (SWM_MB_IDOK == SwmResult)
        {
            if (TRUE == AuthenticatePassword (ADMIN_PW_HANDLE, PasswordBuffer))
            {
                // Password authentication successful.  Display the full menu.
                //
                Result = TRUE;
                break;
            }
            else
            {
                // Password authentication error.  Display error text and ask the user for the password again.
                //
                pErrorMessage  = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_ERRORMSG_AUTHERROR), NULL);

                // If password buffer was used, make sure it's freed.
                //
                if (PasswordBuffer)
                {
                    SecureZeroMem ((UINT8*)PasswordBuffer, StrLen (PasswordBuffer) * sizeof(CHAR16) );
                    FreePool (PasswordBuffer);
                    PasswordBuffer = NULL;
                }

                // If MaxAttempts were specified, determine whether we've used our last attempt.
                //
                if (0 != MaxAttempts)
                {
                    // If we've just used our last attempt, make a note of it and get out of here.
                    //
                    if (0 == --MaxAttempts)
                    {
                        AttemptsExpired = TRUE;
                        break;
                    }
                }
            }
        }
    } while (TRUE);

    // Handle the case where the attempts have expired.
    // Inform the user and return FALSE.
    //
    if (TRUE == AttemptsExpired)
    {
        DEBUG ((DEBUG_INFO, "FrontPage::%a - Max password attempts elapsed!!\n", __FUNCTION__));
        Status = mSWMProtocol->MessageBox (mSWMProtocol,
                                           (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_ATTEMPTS_EXPIRED_TITLE), NULL),   // Dialog titlebar text.
                                           (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_ATTEMPTS_EXPIRED_BODYTEXT), NULL),   // Dialog body text.
                                           (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_PWD_ATTEMPTS_EXPIRED_CAPTION), NULL),    // Dialog caption text.
                                           SWM_MB_OK,           // Show only Ok button.
                                           0,                   // No timeout
                                           &SwmResult);         // Return result.
        Result = FALSE;
    }

    if (Result == TRUE){
        //acquire an auth token and save it in a protocol.
        Status = GetAuthToken(PasswordBuffer);
        if (EFI_ERROR(Status) && (mAuthToken == 0)){
            Result = FALSE;
        }
    }
    // If password buffer was used, make sure it's freed.
    //
    if (NULL != PasswordBuffer)
    {
        SecureZeroMem ((UINT8*)PasswordBuffer, StrLen (PasswordBuffer) * sizeof(CHAR16) );
        FreePool (PasswordBuffer);
        PasswordBuffer = NULL;
    }

    return Result;
} // ChallengeUserPassword()
*/

/**
  IsSemmEnabledForDisplay

  The SEMM dialog will not be present when not enabled.  When SEMM is
  enabled, then enable the SEMM tab of SurfaceFrontPage.

  returns FALSE - do not display SEMM page
  returns TRUE  - display SEMM page
*/
/*
BOOLEAN IsSemmEnabledForDisplay (VOID) {
    EFI_STATUS  Status;
    VOID       *dummy;  // There is no protocol interface - just the existence that is is published

    Status = gBS->LocateProtocol (&gMsSemmMenuFormsetGuid, NULL, (VOID **) &dummy);

    return !EFI_ERROR(Status);
}
*/

 /**
  Checks if the Security, Devices or BootManager Page can be displayed to the user by calling to the provider

  @param SettingProviderType    Type is Either Security, Devices or Boot Manager.

  returns TRUE                  Display Page
  returns FALSE                 Do not DisplayPage
 **/
/*
 BOOLEAN IsMenuItemEnabledForDisplay(
    IN MS_SYSTEM_SETTING_ID_ENUM Id
 )
{

    EFI_STATUS                 Status;
    BOOLEAN                    DisplayItem = TRUE;
    MS_SYSTEM_SETTING_FLAGS    Flags;

    if (mSettingAccess != NULL) {

        Status = mSettingAccess->Get(mSettingAccess,
            Id,
            &mAuthToken,
            MS_SYSTEM_SETTING_TYPE_ENABLE,
            &DisplayItem,
            &Flags);

        if (EFI_ERROR(Status)) {
            DEBUG((DEBUG_ERROR, __FUNCTION__ " Internal error getting Id %x - code=%r\n", Id, Status));
            DisplayItem = 0;
        }
    }

    return DisplayItem;
}
*/

/**
  RemoveMenuFromList - 
    Updates mFormMap so that the menu item specified by MenuId is omitted.
    The item in question will have FullMenuIndex and LimitedMenuIndex set
    to UNUSED_INDEX, and the other indexes in the list are adjusted accordingly.
*/
VOID RemoveMenuFromList (UINT16 MenuId) {
    BOOLEAN FullMenuRemoved = FALSE;
    BOOLEAN LimitedMenuRemoved = FALSE;
    UINTN   Count;

    UINT16  MenuOptionCount  = (sizeof(mFormMap) / sizeof(mFormMap[0]));

    for (Count=0 ; Count < MenuOptionCount ; Count++)
    {
        if (mFormMap[Count].MenuString == MenuId)
        {
            if (mFormMap[Count].FullMenuIndex != UNUSED_INDEX) {
                FullMenuRemoved = TRUE;
                mFormMap[Count].FullMenuIndex = UNUSED_INDEX;
            }
            if (mFormMap[Count].LimitedMenuIndex != UNUSED_INDEX) {
                LimitedMenuRemoved = TRUE;
                mFormMap[Count].LimitedMenuIndex = UNUSED_INDEX;
            }
        }
        if (FullMenuRemoved && mFormMap[Count].FullMenuIndex != UNUSED_INDEX) {
            mFormMap[Count].FullMenuIndex -= 1;
        }
        if (LimitedMenuRemoved && mFormMap[Count].LimitedMenuIndex != UNUSED_INDEX) {
            mFormMap[Count].LimitedMenuIndex -= 1;
        }
    }
}

/**
  Creates the top-level menu in the Master Frame for selecting amongst the various HII forms.

  NOTE: Selectable menu options is dependent on whether there is a System firmware password and on whether the user knows it.


  @param OrigX          Menu's origin (x-axis).
  @param OrigY          Menu's origin (y-axis).
  @param CellWidth      Menu's width.
  @param CellHeight     Menu's height.
  @param CellTextXOffset   Menu entry text indentation.

  @retval  EFI_SUCCESS        The operation is successful.
  @retval  EFI_DEVICE_ERROR   Failed to create the menu.

**/
static
ListBox*
CreateTopMenu(IN UINT32 OrigX,
              IN UINT32 OrigY,
              IN UINT32 CellWidth,
              IN UINT32 CellHeight,
              IN UINT32 CellTextXOffset)
{
    EFI_FONT_INFO   FontInfo;
    EFI_STATUS Status;
    UINT8     *ImageData;
    UINTN      ImageSize;


    // Check whether there is a system password set.  If so, prompt the user for it before deciding the top-level menu list.
    // If the user doesn't know the password, they can dismiss the dialog and will see a limited-functionality menu.
        //
    /*
    if (TRUE == IsPasswordSet(ADMIN_PW_HANDLE))
    {
        if (TRUE == ChallengeUserPassword(MAX_PASSWORD_ATTEMPTS))
        {
            mShowFullMenu = TRUE;
        }
    }
    else
    {
        // If no password is set, show the full menu.
        //
        // if no password is set still we need to get an auth token with a null password.
        Status = GetAuthToken(NULL);
        if ((Status == EFI_SUCCESS) && (mAuthToken != 0)){
           mShowFullMenu = TRUE;
        }
    }
    */

    // Create a listbox with menu options.  The contents of the menu depend on whether a system password is
    // set and whether the user entered the password correctly or not.  If the user cancels the password dialog
    // then only a limited menu is available.
    //
    //
    UINT16  Count, Index;
    UINT16  MenuOptionCount  = (sizeof(mFormMap) / sizeof(mFormMap[0]));
    UIT_LB_CELLDATA  *MenuOptions = AllocateZeroPool((MenuOptionCount + 1) * sizeof(UIT_LB_CELLDATA));     // NOTE: the list relies on a zero-initialized list terminator (hence +1).

    ASSERT (NULL != MenuOptions);
    if (NULL == MenuOptions)
    {
        return NULL;
    }
    //
    // The SEMM menu and the ABOUT menu on FrontPage are optional.
    //
    /*
    if (!IsSemmEnabledForDisplay())
    {
        RemoveMenuFromList (STRING_TOKEN(STR_MF_MENU_OP_SEMM));
    }
    */

    /*
    // Get the Front Page Display State from the MsFrontPageTabDisplaySettings Provider
    if (!IsMenuItemEnabledForDisplay (MS_SYSTEM_SETTING_ID__DEVICES_PAGE_DISPLAY))
    {
        RemoveMenuFromList(STRING_TOKEN(STR_MF_MENU_OP_DEVICES));
    }

    if (!IsMenuItemEnabledForDisplay (MS_SYSTEM_SETTING_ID__BOOTMGR_PAGE_DISPLAY))
    {
        RemoveMenuFromList(STRING_TOKEN(STR_MF_MENU_OP_BOOTORDER));
    }

    if (!IsMenuItemEnabledForDisplay (MS_SYSTEM_SETTING_ID__SECURITY_PAGE_DISPLAY))
    {
        RemoveMenuFromList(STRING_TOKEN(STR_MF_MENU_OP_SECURITY));
    }

    if (!IsMenuItemEnabledForDisplay (MS_SYSTEM_SETTING_ID__SET_TIME_PAGE_DISPLAY))
    {
        RemoveMenuFromList(STRING_TOKEN(STR_MF_MENU_OP_SETTIME));
    }
    */

    // Get the specified About image from FV. If it does not exist, remove the About item from the menu.
    //
    Status = GetSectionFromAnyFv(PcdGetPtr (PcdComplianceLabelFile ), EFI_SECTION_RAW, 0, (VOID **)&ImageData, &ImageSize);
    if (EFI_ERROR(Status))
    {
        RemoveMenuFromList (STRING_TOKEN(STR_MF_MENU_OP_ABOUT));
    }
    else
    {
        FreePool(ImageData);
    }

    for (Count=0 ; Count < MenuOptionCount ; Count++)
    {
        Index = ((FALSE == mShowFullMenu) ? mFormMap[Count].LimitedMenuIndex : mFormMap[Count].FullMenuIndex);

        if (UNUSED_INDEX != Index && Index < MenuOptionCount)
        {
            MenuOptions[Index].CellText = HiiGetString (gStringPackHandle, mFormMap[Count].MenuString, NULL);
        }
    }

    // Create the ListBox that encapsulates the top-level menu.
    //
    FontInfo.FontSize    = FP_MFRAME_MENU_TEXT_FONT_HEIGHT;
    FontInfo.FontStyle   = EFI_HII_FONT_STYLE_NORMAL;

    ListBox *TopMenu = new_ListBox(OrigX,
                                   OrigY,
                                   CellWidth,
                                   CellHeight,
                                   0,
                                   &FontInfo,
                                   CellTextXOffset,
                                   FP_MFRAME_MENU_CELL_NORMAL_COLOR,
                                   FP_MFRAME_MENU_CELL_HOVER_COLOR,
                                   FP_MFRAME_MENU_CELL_SELECT_COLOR,
                                   FP_MFRAME_MENU_CELL_GRAY_COLOR,
                                   MenuOptions,
                                   NULL
                                  );
    // Free HII string buffer.
    //
    if (NULL != MenuOptions)
    {
        FreePool(MenuOptions);
    }


    return TopMenu;
}


/**
  Draws the Front Page Title Bar.


  @param None.

  @retval  EFI_SUCCESS        Success.

**/
EFI_STATUS
RenderTitlebar(VOID)
{
    EFI_STATUS                Status = EFI_SUCCESS;
    EFI_FONT_DISPLAY_INFO     StringInfo;
    EFI_IMAGE_OUTPUT          *pBltBuffer;
    EFI_LOADED_IMAGE_PROTOCOL *ImageInfo;
    CHAR8                     Parameter;
    EFI_GUID                  *IconFile;
    UINTN                     DataSize;
    CHAR8                     RebootReason[MSP_REBOOT_REASON_LENGTH];

    // Draw the titlebar background.
    //
    mGop->Blt(mGop,
              FP_TBAR_BACKGROUND_COLOR,
              EfiBltVideoFill,
              0,
              0,
              0,
              0,
              mTitleBarWidth,
              mTitleBarHeight,
              mTitleBarWidth * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
             );

    /*
    // Draw the Microsoft logo bitmap.
    //
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *pMSLogo = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)gMicrosoftLogo;

    mGop->Blt(mGop,
              pMSLogo,
              EfiBltBufferToVideo,
              0,
              0,
              ((mMasterFrameWidth  * FP_TBAR_MSLOGO_X_PERCENT) / 100),      // NOTE: Based on Master Frame width so it aligns with the text in the menu.
              ((mTitleBarHeight / 2) - (MICROSOFT_LOGO_HEIGHT / 2)),        // Vertically center.
              MICROSOFT_LOGO_WIDTH,
              MICROSOFT_LOGO_HEIGHT,
              0
             );
    */

    GetAndDisplayBitmap(PcdGetPtr(PcdFpMsLogoFile), (mMasterFrameWidth  * FP_TBAR_MSLOGO_X_PERCENT) / 100, FALSE);   // 2nd param is x coordinate

    Status = gBS->HandleProtocol(mImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&ImageInfo);
    ASSERT_EFI_ERROR(Status);

    if ((ImageInfo->LoadOptionsSize == 0) ||
        (ImageInfo->LoadOptions == NULL)) {
        DataSize = MSP_REBOOT_REASON_LENGTH;
        Status = gRT->GetVariable (
                   MSP_REBOOT_REASON_VAR_NAME,
                   &gSurfaceFrontPageNVVarGuid,
                   NULL,
                   &DataSize,
                   &RebootReason[0]
                   );
        if (EFI_ERROR(Status)) {
            if (EFI_NOT_FOUND != Status) {
                DEBUG((DEBUG_ERROR,__FUNCTION__ " error reading RebootReason. Code = %r\n",Status));
            }
            Parameter = 'B';
        } else {
            Parameter = RebootReason[0];
            Status = gRT->SetVariable (
                MSP_REBOOT_REASON_VAR_NAME,
                &gSurfaceFrontPageNVVarGuid,
                EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                0,
                NULL
                );
        }
    } else {
        Parameter = *((CHAR8 *) ImageInfo->LoadOptions);
    }
    DEBUG((DEBUG_ERROR, __FUNCTION__ " Parameter = %c - LoadOption=%p\n",Parameter,ImageInfo->LoadOptions));

    switch (Parameter) {
    case 'V' :  // VOL+
        //IconFile = PcdGetPtr(PcdVolumeUpIndicatorFile);
        break;
    case 'B' : // BOOTFAIL
        IconFile = PcdGetPtr(PcdBootFailIndicatorFile);
        break;
    case 'O' : // OSIndication
        //IconFile = PcdGetPtr(PcdFirmwareSettingsIndicatorFile);
        break;
    default:
        IconFile = NULL;
    }

    if (NULL != IconFile) {
        GetAndDisplayBitmap(IconFile, (mTitleBarWidth * FP_TBAR_ENTRY_INDICATOR_X_PERCENT) / 100, TRUE);
    }

    // Prepare string blitting buffer.
    //
    pBltBuffer = (EFI_IMAGE_OUTPUT *) AllocateZeroPool (sizeof (EFI_IMAGE_OUTPUT));

    ASSERT (pBltBuffer != NULL);
    if (NULL == pBltBuffer)
    {
        Status = EFI_OUT_OF_RESOURCES;
        goto Exit;
    }

    pBltBuffer->Width        = (UINT16)mBootHorizontalResolution;
    pBltBuffer->Height       = (UINT16)mBootVerticalResolution;
    pBltBuffer->Image.Screen = mGop;

    // Select a font (size & style) and font colors.
    //
    StringInfo.FontInfoMask         = EFI_FONT_INFO_ANY_FONT;
    StringInfo.FontInfo.FontSize    = FP_TBAR_TEXT_FONT_HEIGHT;
    StringInfo.FontInfo.FontStyle   = EFI_HII_FONT_STYLE_NORMAL;

    CopyMem (&StringInfo.ForegroundColor, FP_TBAR_TEXT_COLOR,       sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
    CopyMem (&StringInfo.BackgroundColor, FP_TBAR_BACKGROUND_COLOR, sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

    // Determine the size the TitleBar text string will occupy on the screen.
    //
    UINT32      MaxDescent;
    SWM_RECT    StringRect;

    GetTextStringBitmapSize (HiiGetString (gStringPackHandle, STRING_TOKEN (STR_FRONT_PAGE_TITLE), NULL),
                             &StringInfo.FontInfo,
                             FALSE,
                             EFI_HII_OUT_FLAG_CLIP |
                             EFI_HII_OUT_FLAG_CLIP_CLEAN_X | EFI_HII_OUT_FLAG_CLIP_CLEAN_Y |
                             EFI_HII_IGNORE_LINE_BREAK,
                             &StringRect,
                             &MaxDescent
                            );

    // Render the string to the screen, vertically centered.
    //
    mSWMProtocol->StringToWindow (mSWMProtocol,
                                  mImageHandle,
                                  EFI_HII_OUT_FLAG_CLIP |
                                  EFI_HII_OUT_FLAG_CLIP_CLEAN_X | EFI_HII_OUT_FLAG_CLIP_CLEAN_Y |
                                  EFI_HII_IGNORE_LINE_BREAK | EFI_HII_DIRECT_TO_SCREEN,
                                  HiiGetString (gStringPackHandle, STRING_TOKEN (STR_FRONT_PAGE_TITLE), NULL),
                                  &StringInfo,
                                  &pBltBuffer,
                                  ((mMasterFrameWidth  * FP_TBAR_TEXT_X_PERCENT) / 100),                     // Based on Master Frame width - so the logo bitmap aligns with the text in the menu.
                                  ((mTitleBarHeight / 2) - ((StringRect.Bottom - StringRect.Top + 1) / 2)),  // Vertically center.
                                  NULL,
                                  NULL,
                                  NULL
                                 );

Exit:

    if (NULL != pBltBuffer)
    {
        FreePool(pBltBuffer);
    }

    return Status;
}


/**
  Draws the FrontPage Master Frame and its Top-Level menu.


  @param None.

  @retval  EFI_SUCCESS        Success.

**/
EFI_STATUS
RenderMasterFrame(VOID)
{
    EFI_STATUS  Status      = EFI_SUCCESS;
    VOID        *pContext   = NULL;


    // Verify that the top-level menu was created.
    //
    ASSERT (NULL != mTopMenu);
    if (NULL == mTopMenu)
    {
        Status = EFI_INVALID_PARAMETER;
        goto Exit;
    }

    // Draw the master frame background.
    //
    mGop->Blt(mGop,
              FP_MFRAME_BACKGROUND_COLOR,
              EfiBltVideoFill,
              0,
              0,
              0,
              mTitleBarHeight,
              mMasterFrameWidth,
              mMasterFrameHeight,
              0
             );


    // Draw divider line.
    //
    mGop->Blt(mGop,
              FP_TBAR_BACKGROUND_COLOR,
              EfiBltVideoFill,
              0,
              0,
              (mMasterFrameWidth - FP_MFRAME_DIVIDER_LINE_WIDTH_PIXELS),
              mTitleBarHeight,
              FP_MFRAME_DIVIDER_LINE_WIDTH_PIXELS,
              mMasterFrameHeight,
              0
             );


    // Draw the top-level menu.
    //
    mTopMenu->Base.Draw(mTopMenu,
                        FALSE,
                        NULL,
                        &pContext
                       );

Exit:

    return Status;
}


/**
  Determines whether there are any pending messages for the user
  and presents them if there are.

  @retval     EFI_SUCCESS

**/
/*
EFI_STATUS
NotifyUserOfAlerts (
  VOID
  )
{
  EFI_STATUS      Status = EFI_SUCCESS, VarStatus;
  CHAR16          *SbViolationVarName = SFP_SB_VIOLATION_SIGNAL_VAR_NAME, *SbViolationMessage;
  BOOLEAN         SecViolation = FALSE;
  UINTN           DataSize;
  SWM_MB_RESULT   SwmResult           = 0;

  // Check for SecureBoot notifications.
  //
  DataSize = sizeof( SecViolation );
  VarStatus = gRT->GetVariable( SbViolationVarName,
                                &gSurfaceFrontPageNVVarGuid,
                                NULL,
                                &DataSize,
                                (UINT8*)&SecViolation );

  // Inform the user if there was a SecureBoot violation.
  //
  if (SecViolation)
  {
    DEBUG ((DEBUG_INFO, "FrontPage::%a - SecureBoot violation detected! Warning user...\n", __FUNCTION__));
    SbViolationMessage = (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_SB_VIOLATION_WARNING), NULL);
    Status = mSWMProtocol->MessageBox (mSWMProtocol,
                                       (CHAR16*)HiiGetString (gStringPackHandle, STRING_TOKEN (STR_SB_VIOLATION_TITLE), NULL),   // Dialog titlebar text.
                                       SbViolationMessage,  // Dialog body text.
                                       L"",                 // Dialog caption text.
                                       SWM_MB_OK,           // Show Ok button only.
                                       0,                   // No timeout
                                       &SwmResult);         // Return result.
  }

  // If the variable was found successfully, let's delete it so
  // that we don't continue popping up the message.
  //
  if (!EFI_ERROR( Status ) && !EFI_ERROR( VarStatus ))
  {
    Status = gRT->SetVariable( SbViolationVarName,
                               &gSurfaceFrontPageNVVarGuid,
                               0,
                               0,
                               NULL );
  }

  return Status;
} // NotifyUserOfAlerts()
*/


/**
  Master Frame callback (signalled by Display Engine) for receiving user input data (i.e., key, touch, mouse, etc.).


  @param    None.

  @retval   None.

**/
VOID
EFIAPI
MasterFrameNotifyCallback (IN  EFI_EVENT    Event,
                           IN  VOID         *Context)
{
    UINT32          SelectedIndex       = 0;
    VOID            *pSelectionContext  = NULL;
    OBJECT_STATE    MenuState           = NORMAL;
    SWM_INPUT_STATE *pInputState        = &mDisplayEngineState.InputState;
    LB_RETURN_DATA  ReturnData;


    // If we just need to redraw, do that and exit.
    //
    if (REDRAW == mDisplayEngineState.NotificationType)
    {
        mTopMenu->Base.Draw (mTopMenu,
                             mDisplayEngineState.ShowTopMenuHighlight,
                             &mDisplayEngineState.InputState,
                             &pSelectionContext
                            );

        goto Exit;
    }

    // We'll only handle user input from this point onwards.
    //
    if (USERINPUT != mDisplayEngineState.NotificationType)
    {
        goto Exit;
    }

    // If we are receiving touch/mouse data from the display engine and it's a finger/button down event, process it.
    //
    if ((SWM_INPUT_TYPE_TOUCH == pInputState->InputType/* && (pInputState->State.TouchState.ActiveButtons & 0x1) */) ||
        SWM_INPUT_TYPE_KEY   == pInputState->InputType)
    {
        // Draw the top-level menu in the master frame.
        //
        MenuState = mTopMenu->Base.Draw (mTopMenu,
                                         mDisplayEngineState.ShowTopMenuHighlight,
                                         &mDisplayEngineState.InputState,
                                         &pSelectionContext
                                        );

        // If nothing was selected (user may simply have moved the highlighted cell), there's no action to take.
        //
        if (SELECT != MenuState)
        {
            return;
        }

        // Get the currently selected top-level menu entry (may be none).
        //
        mTopMenu->GetSelectedCellIndex (mTopMenu,
                                        &ReturnData);

        SelectedIndex = ReturnData.SelectedCell;

        if (SelectedIndex != mCurrentFormIndex)
        {
            // Update the current form ID to the new one.
            //
            mCurrentFormIndex = SelectedIndex;

            // Signal the form (browser) to close so the new form will be displayed.
            //
            mDisplayEngineState.CloseFormRequest = TRUE;
            mTerminateFrontPage = FALSE;
        }
    }

Exit:
    mDisplayEngineState.NotificationType = NONE;

    return;
}


static
EFI_STATUS
InitializeFrontPageUI (VOID)
{
    EFI_STATUS  Status = EFI_SUCCESS;


    // Establish initial FrontPage TitleBar and Master Frame dimensions based on the current screen size.
    //
    mTitleBarWidth              = mBootHorizontalResolution;
    mTitleBarHeight             = ((mBootVerticalResolution   * FP_TBAR_HEIGHT_PERCENT)  / 100);
    mMasterFrameWidth           = ((mBootHorizontalResolution * FP_MFRAME_WIDTH_PERCENT) / 100);
    mMasterFrameHeight          = (mBootVerticalResolution - mTitleBarHeight);

    DEBUG((DEBUG_INFO, "INFO [FP]: FP Dimensions: %d, %d, %d, %d, %d, %d\r\n", \
           mBootHorizontalResolution, mBootVerticalResolution, mTitleBarWidth, mTitleBarHeight, mMasterFrameWidth, mMasterFrameHeight));

    // Compute Master Frame origin and menu text indentation.
    //
    UINT32 MasterFrameMenuOrigX = 0;
    UINT32 MasterFrameMenuOrigY = mTitleBarHeight;
    UINT32 CellTextXOffset      = ((mMasterFrameWidth * FP_MFRAME_MENU_TEXT_OFFSET_PERCENT) / 100);

    // Determine whether there are any events that require user notification.
    // NOTE: This should come before CreateTopMenu() because it needs to happen before the
    //       Admin Password prompt.
    //
    //NotifyUserOfAlerts();

    // Create the top-level menu in the Master Frame.
    //
    mTopMenu = CreateTopMenu(MasterFrameMenuOrigX,
                             MasterFrameMenuOrigY,
                             (mMasterFrameWidth - FP_MFRAME_DIVIDER_LINE_WIDTH_PIXELS),
                             ((mMasterFrameHeight * FP_MFRAME_MENU_CELL_HEIGHT_PERCENT) / 100),
                             CellTextXOffset);

    ASSERT (NULL != mTopMenu);
    if (NULL == mTopMenu)
    {
        Status = EFI_OUT_OF_RESOURCES;
        goto Exit;
    }

    // Render the TitleBar at the top of the screen.
    //
    RenderTitlebar();

    // Render the Master Frame and its Top-Level menu contents.
    //
    RenderMasterFrame();

    // Create the Master Frame notification event.  This event is signalled by the display engine to note that
    // there is a user input event outside the form area to consider.
    //
    Status = gBS->CreateEventEx (EVT_NOTIFY_SIGNAL,
                                 TPL_CALLBACK,
                                 MasterFrameNotifyCallback,
                                 NULL,
                                 &gMsEventMasterFrameNotifyGroupGuid,
                                 &mMasterFrameNotifyEvent
                                );

    if (EFI_SUCCESS != Status)
    {
        DEBUG((DEBUG_ERROR, "ERROR [FP]: Failed to create master frame notification event.  Status = %r\r\n", Status));
        goto Exit;
    }

    // Set shared pointer to user input context structure in a PCD so it can be shared.
    //
    PcdSet64(PcdCurrentPointerState, (UINT64) (UINTN)&mDisplayEngineState);

Exit:

    return Status;
}

/*
VOID
ProcessBootNext ( VOID )
{
    EFI_STATUS                      Status;
    UINT16                         *BootNext;
    UINTN                           DataSize;
    CHAR16                          BootNextVariableName[sizeof ("Boot####")];
    EFI_BOOT_MANAGER_LOAD_OPTION    LoadOption;
    UINT64                          OsIndication;

    DEBUG((DEBUG_INFO, __FUNCTION__ " entry\n"));
    //
    // Cache and remove the "BootNext" NV variable.
    //
    GetEfiGlobalVariable2 (EFI_BOOT_NEXT_VARIABLE_NAME, (VOID **) &BootNext, &DataSize);
    if (DataSize != sizeof (UINT16)) {
        if (BootNext != NULL) {
            FreePool (BootNext);
        }
        BootNext = NULL;
    }

    Status = gRT->SetVariable (
                    EFI_BOOT_NEXT_VARIABLE_NAME,
                    &gEfiGlobalVariableGuid,
                    0,
                    0,
                    NULL
                    );
    //
    // Deleting NV variable shouldn't fail unless it doesn't exist.
    //
    ASSERT (Status == EFI_SUCCESS || Status == EFI_NOT_FOUND);

    if (NULL != BootNext) {
        UnicodeSPrint (BootNextVariableName, sizeof (BootNextVariableName), L"Boot%04x", *BootNext);
        DEBUG((DEBUG_INFO, "Acting on BootNext %4.4x\n",*BootNext));
        FreePool (BootNext);
        Status = EfiBootManagerVariableToLoadOption (BootNextVariableName, &LoadOption);
        if (!EFI_ERROR (Status)) {
            EfiBootManagerBoot (&LoadOption);
            EfiBootManagerFreeLoadOption (&LoadOption);
        }

        // Reboot to front page

        OsIndication = EFI_OS_INDICATIONS_BOOT_TO_FW_UI;
        Status = gRT->SetVariable (
            L"OsIndications",
            &gEfiGlobalVariableGuid,
            EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
            sizeof(UINT64),
            &OsIndication
            );
        if (EFI_ERROR(Status)) {
            DEBUG((DEBUG_ERROR,"Unable to set OsIndications\n"));
        }
        DEBUG((DEBUG_INFO, __FUNCTION__ " Resetting system\n"));
        gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
    }
}
*/

/**
  This function is the main entry of the platform setup entry.
  The function will present the main menu of the system setup,
  this is the platform reference part and can be customize.
**/
EFI_STATUS
EFIAPI
UefiMain(IN EFI_HANDLE        ImageHandle,
         IN EFI_SYSTEM_TABLE  *SystemTable
        )
{
    EFI_STATUS  Status  = EFI_SUCCESS;
    UINT32      OSKMode = 0;
    EVENT_CHANNEL_STATISTICS stats;

    //Delete BootNext if entry to BootManager.
    Status = gRT->SetVariable(
        L"BootNext",
        &gEfiGlobalVariableGuid,
        0,
        0,
        NULL
        );

    // Save image handle for later.
    //
    mImageHandle = ImageHandle;

    // Disable the watchdog timer
    //
    gBS->SetWatchdogTimer (0, 0, 0, (CHAR16 *)NULL);

    mResetRequired = FALSE;

    ZeroMem(&stats, sizeof(stats));
    BootDeviceEventStatistics(&stats);

    //if (stats.Written == 0)
    //{
    //}

    BootDeviceEventComplete();
    DEBUG((DEBUG_INFO, "[HVBE] Completing boot event\n"));

    BootDeviceEventFlushLog();
    DEBUG((DEBUG_INFO, "[HVBE] Flushing boot event log\n"));

    //
    // Enumerate and display the current boot entries.
    //
    //BootDeviceEventEnumerate(PlatformConsoleEventCallback, &eventCount);

    //
    // Notify the user if any boot event entries were lost because the
    // event log was full.
    //
    //if (stats.Lost > 0)
    //{
    //    PlatformStringPrintById(STRING_TOKEN(STR_BOOT_LOST_EVENT_FORMAT), stats.Lost);
    //}

    //
    // Clear the event log before trying the boot list again.
    //
    BootDeviceEventResetLog();
    DEBUG((DEBUG_INFO, "[HVBE] Resetting boot event log\n"));

    /*
    Status = gBS->LocateProtocol(&gMsSystemSettingAccessProtocolGuid,
        NULL,
        (VOID **)&mSettingAccess
        );
    if (EFI_ERROR(Status))
    {
        ASSERT_EFI_ERROR(Status);
        DEBUG((DEBUG_ERROR, __FUNCTION__"Couldn't locate system setting access protocol\n"));
    }
    */

    // Force-connect all controllers.
    //
    EfiBootManagerConnectAll();

    // Set console mode: *not* VGA, no splashscreen logo.
    // Insure Gop is in Big Display mode prior to accessing GOP.
    MsLogoLibSetConsoleMode (FALSE, FALSE);

    //
    // After the console is ready, get current video resolution
    // and text mode before launching setup at first time.
    //
    Status = gBS->HandleProtocol (gST->ConsoleOutHandle,
                                  &gEfiGraphicsOutputProtocolGuid,
                                  (VOID**)&mGop
                                 );

    if (EFI_ERROR (Status))
    {
        mGop = (EFI_GRAPHICS_OUTPUT_PROTOCOL *)NULL;
        goto Exit;
    }

    // Determine if the Font Protocol is available
    //
    Status = gBS->LocateProtocol (&gEfiHiiFontProtocolGuid,
                                  NULL,
                                  (VOID **)&mFont
                                 );

    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status))
    {
        mFont = (EFI_HII_FONT_PROTOCOL *)NULL;
        Status = EFI_UNSUPPORTED;
        DEBUG((DEBUG_ERROR, "ERROR [FP]: Failed to find Font protocol (%r).\r\n", Status));
        goto Exit;
    }

    // Locate the Simple Window Manager protocol.
    //
    Status = gBS->LocateProtocol (&gMsSWMProtocolGuid,
                                  NULL,
                                  (VOID **)&mSWMProtocol
                                 );

    if (EFI_ERROR(Status))
    {
        mSWMProtocol = NULL;
        Status = EFI_UNSUPPORTED;
        DEBUG((DEBUG_ERROR, "ERROR [FP]: Failed to find the window manager protocol (%r).\r\n", Status));
        goto Exit;
    }

    // Locate the on-screen keyboard (OSK) protocol.
    //
    Status = gBS->LocateProtocol (&gMsOSKProtocolGuid,
                                  NULL,
                                  (VOID **)&mOSKProtocol
                                 );

    if (EFI_ERROR(Status))
    {
        Status = EFI_UNSUPPORTED;
        mOSKProtocol = (MS_ONSCREEN_KEYBOARD_PROTOCOL *)NULL;
        DEBUG((DEBUG_ERROR, "ERROR [FP]: Failed to find the on-screen keyboard protocol (%r).\r\n", Status));
        goto Exit;
    }

    // Set default on-screen keyboard size and position.  Disable icon auto-activation (set by BDS) since
    // we'll display the OSK ourselves when appropriate.
    //

    // Disable OSK icon auto-activation and self-refresh, and ensure keyboard is disabled.
    //
    mOSKProtocol->GetKeyboardMode(mOSKProtocol, &OSKMode);
    OSKMode &= ~(OSK_MODE_AUTOENABLEICON | OSK_MODE_SELF_REFRESH);
    mOSKProtocol->ShowKeyboard(mOSKProtocol,FALSE);
    mOSKProtocol->ShowKeyboardIcon(mOSKProtocol,FALSE);
    mOSKProtocol->SetKeyboardMode(mOSKProtocol, OSKMode);

    // Set keyboard size and position (75% of screen width, bottom-right corner, docked).
    //
    mOSKProtocol->SetKeyboardSize(mOSKProtocol, FP_OSK_WIDTH_PERCENT);
    mOSKProtocol->SetKeyboardPosition(mOSKProtocol, BottomRight, Docked);

    if (mGop != NULL)
    {
        //
        // Get current video resolution and text mode.
        //
        mBootHorizontalResolution = mGop->Mode->Info->HorizontalResolution;
        mBootVerticalResolution   = mGop->Mode->Info->VerticalResolution;
    }

    // Ensure screen is clear when switch Console from Graphics mode to Text mode
    //
    gST->ConOut->EnableCursor (gST->ConOut, FALSE);
    gST->ConOut->ClearScreen (gST->ConOut);

    // Initialize the Simple UI ToolKit.
    //
    Status = InitializeUIToolKit(ImageHandle);

    if (EFI_ERROR(Status))
    {
        DEBUG((DEBUG_ERROR, "ERROR [FP]: Failed to initialize the UI toolkit (%r).\r\n", Status));
        goto Exit;
    }

    // Register Front Page strings with the HII database.
    //
    InitializeStringSupport();


    // Initialize Front Page language support.
    //
    InitializeLanguage(TRUE);


    // Initialize HII data (ex: register strings, etc.).
    //
    InitializeFrontPage(TRUE);

    // Initialize the FrontPage User Interface.
    //
    Status = InitializeFrontPageUI();

    if (EFI_SUCCESS != Status)
    {
        DEBUG((DEBUG_ERROR, "ERROR [FP]: Failed to initialize the FrontPage user interface.  Status = %r\r\n", Status));
        goto Exit;
    }


    // Set the default form ID to show on the canvas.
    //
    mCurrentFormIndex   = 0;
    Status              = EFI_SUCCESS;

    // Display the specified FrontPage form.
    //
    do
    {
        // By default, we'll terminate FrontPage after processing the next Form unless the flag is reset.
        //
        mTerminateFrontPage = TRUE;

        CallFrontPage (mCurrentFormIndex);

    } while (FALSE == mTerminateFrontPage);


    if (mLanguageString != NULL)
    {
        FreePool (mLanguageString);
        mLanguageString = (CHAR8 *)NULL;
    }

    if (mResetRequired)
    {
        //ResetSystemWithSubtype( EfiResetCold, &gMsSurfaceFrontPageResetGuid );
        ResetSystemWithSubtype( EfiResetCold, NULL );
    }

    //ProcessBootNext ();

    // Clean-up
    //
    UninitializeFrontPage();

Exit:

    return Status;
}


EFI_STATUS  GetAndDisplayBitmap (EFI_GUID *FileGuid, UINTN XCoord, BOOLEAN XCoordAdj) {
    EFI_STATUS                       Status;
    UINT8                           *BMPData          = NULL;
    UINTN                            BMPDataSize      = 0;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *BltBuffer        = NULL;
    UINTN                            BltBufferSize;
    UINTN                            BitmapHeight;
    UINTN                            BitmapWidth;

    // Get the specified image from FV.
    //
    Status = GetSectionFromAnyFv(FileGuid,
                                 EFI_SECTION_RAW,
                                 0,
                                 (VOID **)&BMPData,
                                 &BMPDataSize
                                );

    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "ERROR [DE]: Failed to find bitmap file (GUID=%g) (%r).\r\n", FileGuid, Status));
        return Status;
    }

    // Convert the bitmap from BMP format to a GOP framebuffer-compatible form.
    //
    Status = TranslateBmpToGopBlt(BMPData,
                                BMPDataSize,
                                (VOID **)&BltBuffer,
                                &BltBufferSize,
                                &BitmapHeight,
                                &BitmapWidth
                               );
    if (EFI_ERROR(Status)) {
        FreePool(BMPData);
        DEBUG((DEBUG_ERROR, "ERROR [DE]: Failed to convert bitmap file to GOP format (%r).\r\n", Status));
        return Status;
    }

    if (XCoordAdj == TRUE)
    {
       XCoord -= BitmapWidth;
    }

    mGop->Blt(mGop,
              BltBuffer,
              EfiBltBufferToVideo,
              0,
              0,
              XCoord,   //Upper Right corner
              ((mTitleBarHeight / 2) - (BitmapHeight / 2)),
              BitmapWidth,
              BitmapHeight,
              0
             );

    FreePool(BMPData);
    FreePool(BltBuffer );
    return Status;
}


/**
Acquire a Auth Token and save it in a protocol
**/
/*
EFI_STATUS GetAuthToken(CHAR16 *PasswordBuffer){

    EFI_STATUS                            Status;

    Status = gBS->LocateProtocol(
        &gMsAuthenticationProtocolGuid,
        NULL,
        (VOID **)&mAuthProtocol
        );

    if (EFI_ERROR(Status))
    {
        DEBUG((DEBUG_ERROR, "%a - Failed to locate MsAuthProtocol. Can't use check auth. %r\n", __FUNCTION__, Status));
        mAuthProtocol = NULL;
        return Status;
    }
    if (PasswordBuffer != NULL){
        Status = mAuthProtocol->AuthWithPW(mAuthProtocol, PasswordBuffer, StrLen(PasswordBuffer), &mAuthToken);
        DEBUG((DEBUG_INFO, __FUNCTION__"Auth Token Acquired %x\n", mAuthToken, Status));
    }
    else{
        Status = mAuthProtocol->AuthWithPW(mAuthProtocol, NULL, 0, &mAuthToken);
        DEBUG((DEBUG_INFO, __FUNCTION__"Auth Token Acquired with NULL Password %x\n", mAuthToken, Status));
    }
    mFrontPageAuthTokenProtocol = (FRONT_PAGE_AUTH_TOKEN_PROTOCOL *) AllocateZeroPool(sizeof(mFrontPageAuthTokenProtocol));
//regardless of the auth token value we install the protocol.
//when system password is set, if user enters a invalid password, then the frontpage access will be restricted.
//when there isno system password set, if auth token with a null request is returned invalid we
//still allow only a restricted access of the menu. the protocol with invalid auth token will not be used.
    mFrontPageAuthTokenProtocol->AuthToken = (UINTN)mAuthToken;
    Status = gBS->InstallMultipleProtocolInterfaces(&mImageHandle,
        &gMsFrontPageAuthTokenProtocolGuid,
        mFrontPageAuthTokenProtocol,
        NULL);

    if (Status == EFI_SUCCESS){
        DEBUG((DEBUG_INFO, __FUNCTION__" FrontPageAuthTokenProtocol was successfully installed %r\n", Status));
    }

    return Status;
}
*/

