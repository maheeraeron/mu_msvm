/** @file
  Functions to help with loading and string formating using HII resource strings.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Library/HiiLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

//
// This is autogenerated when the .uni string file is processed
// The name must match the name of the .uni file.
//
extern UINT8          FrontPageStrings[];

static EFI_HII_HANDLE gPlatformConsoleStringPackHandle;
static const EFI_GUID mPlatformConsoleStringPackGuid =
    {0x7b222b98, 0x4b6f, 0x4adc, {0xbd, 0xc3, 0x67, 0x6e, 0x5b, 0x76, 0x4d, 0x54}};

//
// Buffer used for string formating.
//
CHAR16                *gStringBuffer    = NULL;
static UINT32         gStringBufferSize = 0;


CHAR16*
PlatformStringById(
    EFI_STRING_ID Id
    )
/*++

Routine Description:

    Returns an allocated string for the given string ID
    The caller should free the string via FreePool

Arguments:

    Id      Id of the string to retrieve

Return Value:

    String or NULL on failure

--*/
{
    CHAR16 *string = HiiGetString(gPlatformConsoleStringPackHandle, Id, NULL);
    //
    // Callers should make sure their string IDs are correct.
    //
    ASSERT(string != NULL);
    return string;
}


UINTN
PlatformStringPrintWorker(
    IN  CONST CHAR16    *Format,
    IN  VA_LIST         Args
    )
/*++

Routine Description:

    Internal function to perform the actual string formatting and printing
    to the current console.

Arguments:

    Format      Format string

    Args        Format string arguments

Return Value:

    Count of characters printed.

--*/
{
    UINTN   charsPrinted = 0;

    if (gStringBuffer == NULL)
    {
        gStringBufferSize = (PcdGet32(PcdPlatformStringBufferSize) * sizeof(CHAR16));
        gStringBuffer = AllocatePool(gStringBufferSize);

        if (gStringBuffer == NULL)
        {
            goto Exit;
        }
    }

    charsPrinted = UnicodeVSPrint(gStringBuffer, gStringBufferSize, Format, Args);

    //if (charsPrinted)
    //{
    //    gST->ConOut->OutputString(gST->ConOut, gStringBuffer);
    //}

Exit:

    return charsPrinted;
}


UINTN
PlatformStringPrintById(
    IN  EFI_STRING_ID   Id,
    ...
    )
/*++

Routine Description:

    Prints a formatted string to the current console.
    The format string is retrieved using the provided string resource id.

Arguments:

    Id          Resource id of the string to use for formatting.

Return Value:

    Count of characters printed.

--*/
{
    VA_LIST args;
    UINTN   charsPrinted = 0;
    CHAR16  *formatString = NULL;

    formatString = PlatformStringById(Id);

    if (formatString == NULL)
    {
        goto Exit;
    }

    VA_START(args, Id);
    charsPrinted = PlatformStringPrintWorker(formatString, args);
    VA_END(args);

Exit:
    gBS->FreePool(formatString);
    return charsPrinted;
}


UINTN
PlatformStringPrint(
    IN  CHAR16  *Format,
    ...
    )
/*++

Routine Description:

    Prints a formatted string to the current console.

Arguments:

    Format      Format string

Return Value:

    Count of characters printed

--*/
{
    VA_LIST args;
    UINTN   charsPrinted = 0;

    VA_START(args, Format);
    charsPrinted = PlatformStringPrintWorker(Format, args);
    VA_END(args);

    return charsPrinted;
}


UINTN
PlatformStringPrintSById(
    OUT CHAR16          *StartOfBuffer,
    IN  UINTN           BufferSize,
    IN  EFI_STRING_ID   Id,
    ...
    )
/*++

Routine Description:

    Prints a formated string to the given buffer retrieving the format string
    using the provided string resource ID.
    If the provided buffer is too small this will write up to BufferSize - sizeof(CHAR16)
    characters before terminating the string.

Arguments:

    StartOfBuffer   Start of the buffer to write the formatted string to.

    BufferSize      Size of the buffer in bytes

    Id              String resource ID for the format string

Return Value:

    Count of characters printed to the buffer

--*/
{
    VA_LIST args;
    CHAR16  *formatString = NULL;
    UINTN   charsPrinted = 0;

    formatString = PlatformStringById(Id);

    if (formatString == NULL)
    {
        goto Exit;
    }

    VA_START(args, Id);
    charsPrinted = UnicodeVSPrint(StartOfBuffer, BufferSize, formatString, args);
    VA_END(args);

Exit:

    gBS->FreePool(formatString);
    return charsPrinted;
}


EFI_STATUS
PlatformStringInitialize()
/*++

Routine Description:

    Initializes string resource support

Arguments:

    None.

Return Value:

    EFI_STATUS

--*/
{
    gPlatformConsoleStringPackHandle = HiiAddPackages(&mPlatformConsoleStringPackGuid,
                                            gImageHandle,
                                            FrontPageStrings,
                                            NULL);
    ASSERT (gPlatformConsoleStringPackHandle != NULL);

    if (gPlatformConsoleStringPackHandle == NULL)
    {
        return EFI_NOT_READY;
    }
    else
    {
        return EFI_SUCCESS;
    }
}