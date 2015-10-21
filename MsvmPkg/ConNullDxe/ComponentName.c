/*++

Copyright (c) Microsoft Corporation

Module Name:

    ComponentName.c

Abstract:

    Provides the UEFI Component Name and Name2 protocol for the Hyper-V null console driver.

Author:

    Larry Cleeton (lcleeton) - 01-Dec-2014

--*/

#include "Console.h"

//
// EFI Component Name Protocol.
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_COMPONENT_NAME_PROTOCOL  gConsoleComponentName =
{
    ConsoleComponentNameGetDriverName,
    ConsoleComponentNameGetControllerName,
    "eng"
};

//
// EFI Component Name 2 Protocol.
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_COMPONENT_NAME2_PROTOCOL gConsoleComponentName2 =
{
    (EFI_COMPONENT_NAME2_GET_DRIVER_NAME) ConsoleComponentNameGetDriverName,
    (EFI_COMPONENT_NAME2_GET_CONTROLLER_NAME) ConsoleComponentNameGetControllerName,
    "en"
};

//
// Root Controller name table.
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_UNICODE_STRING_TABLE gConsoleControllerNameTable[] =
{
    { "eng;en", (CHAR16 *)L"Hyper-V Null Console Controller"},
    { NULL, NULL }
};

//
// Driver name table.
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_UNICODE_STRING_TABLE mConsoleDriverNameTable[] =
{
    {
        "eng;en",
        L"Hyper-V Null Console Driver"
    },
    {
        NULL,
        NULL
    }
};


EFI_STATUS
EFIAPI
ConsoleComponentNameGetDriverName(
    _In_  EFI_COMPONENT_NAME_PROTOCOL   *This,
    _In_  CHAR8                         *Language,
    _Out_ CHAR16                        **DriverName
    )
/*++

Routine Description:

    Retrieves a Unicode string that is the user readable name of the driver.

    This function retrieves the user readable name of a driver in the form of a
    Unicode string. If the driver specified by This has a user readable name in
    the language specified by Language, then a pointer to the driver name is
    returned in DriverName, and EFI_SUCCESS is returned. If the driver specified
    by This does not support the language specified by Language,
    then EFI_UNSUPPORTED is returned.

Arguments:

    This            A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
                    EFI_COMPONENT_NAME_PROTOCOL instance.

    Language        A pointer to a Null-terminated ASCII string
                    array indicating the language. This is the
                    language of the driver name that the caller is
                    requesting, and it must match one of the
                    languages specified in SupportedLanguages. The
                    number of languages supported by a driver is up
                    to the driver writer. Language is specified
                    in RFC 4646 or ISO 639-2 language code format.

    DriverName      A pointer to the Unicode string to return.
                    This Unicode string is the name of the
                    driver specified by This in the language
                    specified by Language.

Return Value:

    EFI_SUCCESS           The Unicode string for the Driver specified by
                          This and the language specified by Language was
                          returned in DriverName.

    EFI_INVALID_PARAMETER Language is NULL.

    EFI_INVALID_PARAMETER DriverName is NULL.

    EFI_UNSUPPORTED       The driver specified by This does not support
                          the language specified by Language.

--*/
{
    return LookupUnicodeString2(Language,
                                This->SupportedLanguages,
                                mConsoleDriverNameTable,
                                DriverName,
                                (BOOLEAN)(This == &gConsoleComponentName));
}


EFI_STATUS
EFIAPI
ConsoleComponentNameGetControllerName(
    _In_     EFI_COMPONENT_NAME_PROTOCOL    *This,
    _In_     EFI_HANDLE                     ControllerHandle,
    _In_opt_ EFI_HANDLE                     ChildHandle,
    _In_     CHAR8                          *Language,
    _Out_    CHAR16                         **ControllerName
    )
/*++

Routine Description:

    Retrieves a Unicode string that is the user readable name of the controller
    that is being managed by a driver.

    This function retrieves the user readable name of the controller specified by
    ControllerHandle and ChildHandle in the form of a Unicode string. If the
    driver specified by This has a user readable name in the language specified by
    Language, then a pointer to the controller name is returned in ControllerName,
    and EFI_SUCCESS is returned.  If the driver specified by This is not currently
    managing the controller specified by ControllerHandle and ChildHandle,
    then EFI_UNSUPPORTED is returned.  If the driver specified by This does not
    support the language specified by Language, then EFI_UNSUPPORTED is returned.

Arguments:

    This                    A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
                            EFI_COMPONENT_NAME_PROTOCOL instance.

    ControllerHandle        The handle of a controller that the driver
                            specified by This is managing.  This handle
                            specifies the controller whose name is to be
                            returned.

    ChildHandle             The handle of the child controller to retrieve
                            the name of.  This is an optional parameter that
                            may be NULL.  It will be NULL for device
                            drivers.  It will also be NULL for a bus drivers
                            that wish to retrieve the name of the bus
                            controller.  It will not be NULL for a bus
                            driver that wishes to retrieve the name of a
                            child controller.

    Language                A pointer to a Null-terminated ASCII string
                            array indicating the language.  This is the
                            language of the driver name that the caller is
                            requesting, and it must match one of the
                            languages specified in SupportedLanguages. The
                            number of languages supported by a driver is up
                            to the driver writer. Language is specified in
                            RFC 4646 or ISO 639-2 language code format.

    ControllerName          A pointer to the Unicode string to return.
                            This Unicode string is the name of the
                            controller specified by ControllerHandle and
                            ChildHandle in the language specified by
                            Language from the point of view of the driver
                            specified by This.

Return Value:

    EFI_SUCCES              The Unicode string for the user readable name in
                            the language specified by Language for the
                            driver specified by This was returned in
                            DriverName.

    EFI_INVALID_PARAMETER   ControllerHandle is NULL.

    EFI_INVALID_PARAMETER   ChildHandle is not NULL and it is not a valid
                            EFI_HANDLE.

    EFI_INVALID_PARAMETER   Language is NULL.

    EFI_INVALID_PARAMETER   ControllerName is NULL.

    EFI_UNSUPPORTED         The driver specified by This is not currently
                            managing the controller specified by
                            ControllerHandle and ChildHandle.

    EFI_UNSUPPORTED         The driver specified by This does not support
                            the language specified by Language.

--**/
{
    //
    // Just get the name of the root handle.
    //
    return LookupUnicodeString2(Language,
                                This->SupportedLanguages,
                                gConsoleControllerNameTable,
                                ControllerName,
                                (BOOLEAN)(This == &gConsoleComponentName));
}
