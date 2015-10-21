/*++

Copyright (c) Microsoft Corporation

Module Name:

    ComponentName.c

Abstract:

    Implementation of ComponentName and ComponentName2 protocol for VideoDxe.

Author:

    Bhanu Gogineni (bhanug) - 04-Sep-2012

--*/


#include "VideoDxe.h"


EFI_COMPONENT_NAME2_PROTOCOL gVideoDxeComponentName2 =
{
    VideoDxeComponentNameGetDriverName,
    VideoDxeComponentNameGetControllerName,
    "en"
};

EFI_COMPONENT_NAME_PROTOCOL gVideoDxeComponentName =
{
    (EFI_COMPONENT_NAME_GET_DRIVER_NAME) VideoDxeComponentNameGetDriverName,
    (EFI_COMPONENT_NAME_GET_CONTROLLER_NAME)VideoDxeComponentNameGetControllerName,
    "eng"
};

EFI_UNICODE_STRING_TABLE gVideoDxeDriverNameTable[] =
{
    { "eng;en", (CHAR16 *)L"Hyper-V Video Driver"},
    { NULL, NULL }
};

EFI_UNICODE_STRING_TABLE gVideoDxeControllerNameTable[] =
{
    { "eng;en", (CHAR16 *)L"Hyper-V Video Controller"},
    { NULL, NULL }
};


EFI_STATUS
EFIAPI
VideoDxeComponentNameGetDriverName (
    __in EFI_COMPONENT_NAME2_PROTOCOL *This,
    __in CHAR8 *Language,
    __out CHAR16 **DriverName
    )
/*++

Routine Description:

    Retrieves a string that is the user readable name of the EFI Driver.

Arguments:

    This - A pointer to the EFI_COMPONENT_NAME2_PROTOCOL instance.

    Language - A pointer to a Null-terminated ASCII string array
        indicating the language.

    DriverName - A pointer to the string to return.

Return Value:

    EFI_STATUS.

--*/
{
    return LookupUnicodeString2(
        Language,
        This->SupportedLanguages,
        gVideoDxeDriverNameTable,
        DriverName,
        (BOOLEAN)(This != &gVideoDxeComponentName2));
}


EFI_STATUS
EFIAPI
VideoDxeComponentNameGetControllerName (
    __in EFI_COMPONENT_NAME2_PROTOCOL *This,
    __in EFI_HANDLE ControllerHandle,
    __in_opt EFI_HANDLE ChildHandle,
    __in CHAR8 *Language,
    __out CHAR16 **ControllerName
    )
/*++

Routine Description:

    Retrieves a string that is the user readable name of the controller
    that is being managed by an EFI Driver.

Arguments:

    This - A pointer to the EFI_COMPONENT_NAME2_PROTOCOL instance.

    ControllerHandle - The controller whose name is to be returned.

    ChildHandle - The handle of the child controller to retrieve the name of.

    Language - A pointer to a Null-terminated ASCII string array
        indicating the language.

    ControllerName -A pointer to the string to return.

--*/
{
    EFI_STATUS status;

    //
    // ChildHandle must be NULL for a Device Driver
    //

    if (ChildHandle != NULL)
    {
        return EFI_UNSUPPORTED;
    }

    //
    // Make sure this driver is currently managing ControllerHandle
    //

    status = EfiTestManagedDevice(
        ControllerHandle,
        gVideoDxeDriverBinding.DriverBindingHandle,
        &gEfiEmclProtocolGuid);

    if (EFI_ERROR(status))
    {
        return status;
    }

    return LookupUnicodeString2(
        Language,
        This->SupportedLanguages,
        gVideoDxeControllerNameTable,
        ControllerName,
        (BOOLEAN)(This != &gVideoDxeComponentName2));
}

