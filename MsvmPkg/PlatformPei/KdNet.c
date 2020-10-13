/*++

Copyright (c) Microsoft Corporation

Module Name:

    KdNet.c

Abstract:

    Support routines for loading and configuring the KDNET debugger transport.

--*/

#include <EfiNt.h>
#include <Platform.h>
#include <Hob.h>
#include <KdNet.h>
#include <kdnetinterface.h>
#include <IndustryStandard/PeImage.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HostVisibilityLib.h>
#include <Library/PeCoffLib.h>
#include <Library/PeiServicesLib.h>
#include <IsolationTypes.h>

#define MSR_GHCB        0xC0010130

BOOLEAN UseKdNetDebugger;
DEBUG_NET_PARAMETERS KdNetParameters;

#if defined(MDE_CPU_X64)

EFI_STATUS
AllocateHostVisiblePages(
    _In_ UINT32 IsolationType,
    _In_ UINT32 NumberOfBytes,
    _Out_ EFI_PHYSICAL_ADDRESS *Allocation
    )
{
    HV_GPA_PAGE_NUMBER gpaPage;
    UINT32 numberOfPages;
    EFI_STATUS status;

    //
    // Allocate the correct number of pages.  Because these pages will be made
    // host visible without a clear point in time at which they become
    // host-not-visible again, they must be allocated as reserved memory so
    // they are not reused by the OS.  Since debugging UEFI is a rare
    // scenario, this memory leak is inconsequential.
    //

    numberOfPages = (NumberOfBytes + EFI_PAGE_SIZE - 1) / EFI_PAGE_SIZE;

    status = PeiServicesAllocatePages(
        EfiReservedMemoryType,
        numberOfPages,
        Allocation);

    if (EFI_ERROR(status))
    {
        return status;
    }

    //
    // Make each page host visible.  If conversion fails, simply return the
    // failure.  Since the pages were allocated as reserved, they will never
    // be reused, so the indeterminate visibility state is irrelevant.
    //

    gpaPage = *Allocation / EFI_PAGE_SIZE;

    while (numberOfPages != 0)
    {
        status = EfiMakePageHostVisible(IsolationType, gpaPage);
        if (EFI_ERROR(status))
        {
            return status;
        }

        gpaPage += 1;
        numberOfPages -= 1;
    }

    *Allocation += PcdGet64(PcdIsolationSharedGpaBoundary);

    return EFI_SUCCESS;
}

#endif


PUCHAR
FindDebugStringToken(
    _In_z_ PUCHAR String,
    _In_z_ PUCHAR Token
    )
/*++

Routine Description:

    Perform a case-sensitive search for the specified token in the command
    string.

Arguments:

    String - The NULL-terminated string to extract a token from.

    Token - The token string.

Return Value:

    A pointer to the first character following the token, or NULL if the token
    was not found.

--*/
{
    UINT32 index;

    //
    // Continue searching for the first character in the token.
    //

    while (*String != '\0')
    {
        if (*String == *Token)
        {
            index = 0;

            //
            // Search until the token ends or until there is a mismatch
            // between the token and the input string.
            //

            while (TRUE)
            {
                if (Token[index] == '\0')
                {
                    return String + index;
                }

                if (String[index] != Token[index])
                {
                    break;
                }

                index += 1;

                //
                // The token string (which is a constant input parameter) is
                // expected to be a reasonable size.
                //

                ASSERT(index != 0);
            }
        }

        String += 1;
    }

    return NULL;
}


UINT32
ConvertAsciiToUint32(
    _In_z_ PUCHAR String,
    _Out_opt_ PUCHAR *Next
    )
/*++

Routine Description:

    Attempt to convert the string to a number, like cstdlib atoi.

Arguments:

    String - The string to convert to a number.

    Next - Optionally specifies a pointer to a variable to receive the next
           character in the string following the number that was parsed.

Return Value:

    UINT32 representing the numerical value of the string.

--*/
{
    UINT32 result = 0;

    while (*String >= '0' && *String <= '9')
    {
        result *= 10;
        result += *String - '0';
        String++;
    }

    if (ARGUMENT_PRESENT(Next))
    {
        *Next = String;
    }

    return result;
}


UINT64
ConvertBase36ToUint64(
    _In_z_ PUCHAR String,
    _Out_opt_ PUCHAR *Next
    )
/*++

Routine Description:

    Attempt to convert the string to a number, like cstdlib _strtoui64.

Arguments:

    String - The string to convert to a number.

    Next - Optionally specifies a pointer to a variable to receive the next
           character in the string following the number that was parsed.

Return Value:

    UINT64 representing the numerical value of the string.

--*/
{
    UINT64 result = 0;

    while (TRUE)
    {
        if (*String >= '0' && *String <= '9')
        {
            result *= 36;
            result += *String - '0';
            String++;
        }
        else if (*String >= 'A' && *String <= 'Z')
        {
            result *= 36;
            result += *String - 'A' + 10;
            String++;
        }
        else if (*String >= 'a' && *String <= 'z')
        {
            result *= 36;
            result += *String - 'a' + 10;
            String++;
        }
        else
        {
            if (ARGUMENT_PRESENT(Next))
            {
                *Next = String;
            }

            return result;
        }
    }
}


VOID
ParseKdNetParameters(
    _In_z_ PUCHAR CommandLine
    )
/*++

Routine Description:

    Parses the KDNET parameters from the command line.

Arguments:

    CommandLine - Supplies the command line.

Return Value:

    None.

--*/
{
    UINT32 index;
    UINT8 ipAddress[4];
    PUCHAR parameter;
    IPV6_ADDRESS unspecifiedIPv4Address = {0x0000ffff00000000, 0};
    UINT64 value;

    //
    // Check for NET debug parameters.
    //

    parameter = FindDebugStringToken(CommandLine, "HOST_IP");
    if ((parameter != NULL) && (*parameter == ' '))
    {
        UseKdNetDebugger = TRUE;

        //
        // Parse host ipv4 address.
        //

        for (index = 0; index < 4; index += 1)
        {
            //
            // Parse the IP address as well as possible.  Since the IP address
            // is host-controlled and therefore not trustworthy, it doesn't
            // really matter whether the syntax is legitimate since the host
            // can jam in any IP address that it wants.  Aim for simple
            // parsing, ensuring that valid addresses can be parsed and
            // invalid addresses don't corrupt the parser state.
            //

            parameter += 1;
            ipAddress[index] = (UINT8)ConvertAsciiToUint32(parameter, &parameter);
        }

        KdNetParameters.HostIP = unspecifiedIPv4Address;
        KdNetParameters.HostIP.DW0 =
            (ipAddress[0] << 24) +
            (ipAddress[1] << 16) +
            (ipAddress[2] << 8) +
            (ipAddress[3] << 0);

        KdNetParameters.AssignedHostIP = KdNetParameters.HostIP;
    }

    parameter = FindDebugStringToken(CommandLine, "HOST_PORT");
    if ((parameter != NULL) && (*parameter == ' '))
    {
        KdNetParameters.HostPort = (UINT16)ConvertAsciiToUint32(parameter + 1, NULL);
        KdNetParameters.AssignedHostPort = KdNetParameters.HostPort;
        KdNetParameters.TargetPort = KdNetParameters.HostPort;
    }

    parameter = FindDebugStringToken(CommandLine, "ENCRYPTION_KEY");
    if ((parameter != NULL) && (*parameter == ' '))
    {
        KdNetParameters.EncryptedLink = TRUE;

        //
        // Parse the encryption key.
        //

        for (index = 0; index < 4; index += 1)
        {
            parameter += 1;
            value = ConvertBase36ToUint64(parameter, &parameter);
            CopyMem(&KdNetParameters.Key[index * sizeof(UINT64)], &value, sizeof(UINT64));
        }
    }
}


RETURN_STATUS
EFIAPI
PeCoffLocateExport(
    _In_ VOID *Pe32Data,
    _In_ CONST CHAR8 *Name,
    _Out_ VOID **Export
    )
/*++

Routine Description:

    Locates an exported symbol from a PE/COFF image.

Arguments:

    Pe32Data - Supplies the base of the PE/COFF image.

    Name - Supplies a NULL-terminated string containing the exported symbol to
           locate.

    Export - Supplies a pointer to a variable to receive a pointer to the
             exported symbol.  NULL will be returned if the symbol could not
             be found.

Return Value:

    RETURN_SUCCESS if the symbol search was completed (regardless of whether
    the symbol was found).

    RETURN_UNSUPPORTED if the symbol search was not possible on the specified
    image.

--*/
{
    INTN diff;
    EFI_IMAGE_DOS_HEADER *dosHeader;
    EFI_IMAGE_EXPORT_DIRECTORY *exportDirectory;
    UINT32 *functionList;
    UINT32 index;
    UINT32 maxIndex;
    UINT32 minIndex;
    UINT32 *nameList;
    EFI_IMAGE_NT_HEADERS64 *ntHeader;
    UINT16 *ordinalList;

    dosHeader = (EFI_IMAGE_DOS_HEADER *)Pe32Data;
    if (dosHeader->e_magic != EFI_IMAGE_DOS_SIGNATURE)
    {
        return RETURN_UNSUPPORTED;
    }

    ntHeader = (EFI_IMAGE_NT_HEADERS64 *)((UINTN)Pe32Data + (UINTN)((dosHeader->e_lfanew) & 0x0ffff));
    if (ntHeader->OptionalHeader.Magic != EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    {
        return RETURN_UNSUPPORTED;
    }

    //
    // Assume no matching export unless it is found.
    //
    *Export = NULL;

    //
    // Find the export directory.
    //
    if (ntHeader->OptionalHeader.NumberOfRvaAndSizes <= EFI_IMAGE_DIRECTORY_ENTRY_EXPORT)
    {
        return RETURN_SUCCESS;
    }

    exportDirectory = (EFI_IMAGE_EXPORT_DIRECTORY *)
        ((UINTN)Pe32Data + ntHeader->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    if ((UINTN)exportDirectory == (UINTN)Pe32Data)
    {
        return RETURN_SUCCESS;
    }

    nameList = (UINT32 *)((UINTN)Pe32Data + exportDirectory->AddressOfNames);
    ordinalList = (UINT16 *)((UINTN)Pe32Data + exportDirectory->AddressOfNameOrdinals);
    functionList = (UINT32 *)((UINTN)Pe32Data + exportDirectory->AddressOfFunctions);

    //
    // Perform a binary search to find the name.
    //
    minIndex = 0;
    maxIndex = exportDirectory->NumberOfNames;
    while (minIndex < maxIndex)
    {
        index = (minIndex + maxIndex) / 2;
        diff = AsciiStrCmp(Name, (CHAR8 *)Pe32Data + nameList[index]);
        if (diff == 0)
            break;
        else if (diff < 0)
            maxIndex = index;
        else
            minIndex = index + 1;
    }

    if (minIndex < maxIndex)
    {
        *Export = (VOID *)((UINTN)Pe32Data + functionList[ordinalList[index]]);
    }

    return RETURN_SUCCESS;
}


VOID
LoadKdNet(
    _In_ EFI_PEI_FILE_HANDLE FileHandle
    )
/*++

Routine Description:

    Loads the embedded KDNET binary into memory, along with publishing a HOB to
    the debug stubs that run in DxeCore indicating where the KDNET binary was
    loaded.

Arguments:

    FileHandle - Handle of the PlatformPei file.

Return Value:

    None.

--*/
{
    UINT32 dataSize;
    VOID *exportedFunction;
    KDNET_GET_HARDWARE_CONTEXT_SIZE getHardwareContextSize;
    KDNET_GET_NET_DATA_SIZE getNetDataSize;
    UINT32 hardwareContextSize;
    EFI_KDNET_HOB hobData;
    PE_COFF_LOADER_IMAGE_CONTEXT imageContext;
    KDNET_INITIALIZE_EARLY initializeEarly;
    UINT32 imageSizeInPages;
    VOID* kdnetBinary;
    EFI_PHYSICAL_ADDRESS kdnetData;
    EFI_PHYSICAL_ADDRESS kdnetImage;
    RETURN_STATUS returnStatus;
    EFI_STATUS status;
#if defined(MDE_CPU_X64)
    EFI_PHYSICAL_ADDRESS ghcbAddress;
    EFI_PHYSICAL_ADDRESS hostVisibleData;
    UINT32 isolationType;
#endif

    status = PeiServicesFfsFindSectionData(EFI_SECTION_RAW,
        FileHandle,
        &kdnetBinary);

    ASSERT_EFI_ERROR(status);

    DEBUG((DEBUG_ERROR, "KDNET Load - kdnet found at 0x%x\n", kdnetBinary));

    //
    // Determine the size of the kdnet binary and allocate a suitable buffer
    // into which it can be loaded.
    //
    ZeroMem(&imageContext, sizeof(PE_COFF_LOADER_IMAGE_CONTEXT));

    imageContext.Handle = kdnetBinary;
    imageContext.ImageRead = PeCoffLoaderImageReadFromMemory;

    status = PeCoffLoaderGetImageInfo(&imageContext);
    ASSERT_EFI_ERROR(status);

    imageSizeInPages = (UINT32)((imageContext.ImageSize + EFI_PAGE_SIZE - 1) / EFI_PAGE_SIZE);

    status = PeiServicesAllocatePages(
        EfiBootServicesCode,
        imageSizeInPages,
        &kdnetImage);

    if (EFI_ERROR(status))
    {
        //
        // If memory cannot be allocated, then simply give up on the load.
        //
        return;
    }

    imageContext.ImageAddress = kdnetImage;

    status = PeCoffLoaderLoadImage(&imageContext);
    if (EFI_ERROR(status))
    {
        //
        // Don't bother to reclaim the memory, as it will be reclaimed by the
        // OS after ExitBootServices.
        //
        return;
    }

    //
    // Apply base relocations.  This should not fail.
    //
    status = PeCoffLoaderRelocateImage(&imageContext);
    ASSERT_EFI_ERROR(status);

    DEBUG((DEBUG_ERROR, "KDNET Load - memory allocated at 0x%x\n", kdnetImage));

    //
    // Locate all of the required exports.
    //

    returnStatus = PeCoffLocateExport((VOID *)kdnetImage, "KdNetGetHardwareContextSize", &exportedFunction);
    if (returnStatus != RETURN_SUCCESS)
    {
        return;
    }
    getHardwareContextSize = (KDNET_GET_HARDWARE_CONTEXT_SIZE)(UINTN)exportedFunction;

    returnStatus = PeCoffLocateExport((VOID *)kdnetImage, "KdNetGetNetDataSize", &exportedFunction);
    if (returnStatus != RETURN_SUCCESS)
    {
        return;
    }
    getNetDataSize = (KDNET_GET_NET_DATA_SIZE)(UINTN)exportedFunction;

    returnStatus = PeCoffLocateExport((VOID *)kdnetImage, "KdNetInitializeEarly", &exportedFunction);
    if (returnStatus != RETURN_SUCCESS)
    {
        return;
    }
    initializeEarly = (KDNET_INITIALIZE_EARLY)exportedFunction;

    returnStatus = PeCoffLocateExport((VOID *)kdnetImage, "KdNetInitializeLibrary", &exportedFunction);
    if (returnStatus != RETURN_SUCCESS)
    {
        return;
    }
    hobData.InitializeLibrary = (KDNET_INITIALIZE_LIBRARY)exportedFunction;

    returnStatus = PeCoffLocateExport((VOID *)kdnetImage, "KdNetInitializeDebugging", &exportedFunction);
    if (returnStatus != RETURN_SUCCESS)
    {
        return;
    }
    hobData.InitializeDebugging = (KDNET_INITIALIZE_DEBUGGING)exportedFunction;

    returnStatus = PeCoffLocateExport((VOID *)kdnetImage, "KdNetSendPacket", &exportedFunction);
    if (returnStatus != RETURN_SUCCESS)
    {
        return;
    }
    hobData.SendPacket = (KDNET_SEND_PACKET)exportedFunction;

    returnStatus = PeCoffLocateExport((VOID *)kdnetImage, "KdNetReceivePacket", &exportedFunction);
    if (returnStatus != RETURN_SUCCESS)
    {
        return;
    }
    hobData.ReceivePacket = (KDNET_RECEIVE_PACKET)exportedFunction;

    returnStatus = PeCoffLocateExport((VOID *)kdnetImage, "KdNetGetSentPacketCount", &exportedFunction);
    if (returnStatus != RETURN_SUCCESS)
    {
        return;
    }
    hobData.GetSentPacketCount = (KDNET_GET_PACKET_COUNT)exportedFunction;

    returnStatus = PeCoffLocateExport((VOID *)kdnetImage, "KdNetGetReceivedPacketCount", &exportedFunction);
    if (returnStatus != RETURN_SUCCESS)
    {
        return;
    }
    hobData.GetReceivedPacketCount = (KDNET_GET_PACKET_COUNT)exportedFunction;

    //
    // Determine the amount of data that will be required to support the
    // hypervisor-based KDNET transport.
    //

    KdNetParameters.DbgDeviceDescriptor.VendorID = PCI_VID_MSHV_NET;
    KdNetParameters.DbgDeviceDescriptor.Memory.Length =
        getHardwareContextSize(&KdNetParameters.DbgDeviceDescriptor);
    hardwareContextSize = (KdNetParameters.DbgDeviceDescriptor.Memory.Length + EFI_PAGE_SIZE - 1) / EFI_PAGE_SIZE;

    //
    // Allocate a memory block for the KDNET data.  This is the sum of the
    // hardware context size, the net data size, plus one page in between.
    //

    dataSize = hardwareContextSize + 1 +
               ((getNetDataSize(&KdNetParameters.DbgDeviceDescriptor) + EFI_PAGE_SIZE - 1) / EFI_PAGE_SIZE);

    status = PeiServicesAllocatePages(
        EfiBootServicesData,
        dataSize,
        &kdnetData);
    if (EFI_ERROR(status))
    {
        return;
    }

    KdNetParameters.DbgDeviceDescriptor.Memory.VirtualAddress = (VOID *)kdnetData;

#if defined(MDE_CPU_X64)

    //
    // If this is a hardware-isolated partition with no paravisor, then
    // allocate host-visible pages as required.
    //
    isolationType = PcdGet32(PcdIsolationArchitecture);
    if ((isolationType >= UefiIsolationTypeSnp) &&
        !PcdGetBool(PcdIsolationParavisorPresent))
    {
        status = AllocateHostVisiblePages(
            isolationType,
            KdNetParameters.DbgDeviceDescriptor.TransportData.SharedVisibleDataSize,
            &hostVisibleData);
        if (EFI_ERROR(status))
        {
            return;
        }

        KdNetParameters.DbgDeviceDescriptor.BaseAddress[0].Length = KdNetParameters.DbgDeviceDescriptor.TransportData.SharedVisibleDataSize;
        KdNetParameters.DbgDeviceDescriptor.BaseAddress[0].TranslatedAddress = (VOID *)hostVisibleData;

        //
        // Obtain the address of the GHCB.  This comes from an untrusted
        // register, but any page that's above the shared GPA boundary will be
        // suitable, since all shared pages are assumed to be untrustworthy.
        //

        ghcbAddress = AsmReadMsr64(MSR_GHCB);
        if ((ghcbAddress % EFI_PAGE_SIZE != 0) ||
            (ghcbAddress < PcdGet64(PcdIsolationSharedGpaBoundary)))
        {
            return;
        }

        KdNetParameters.DbgDeviceDescriptor.BaseAddress[1].TranslatedAddress = (VOID *)ghcbAddress;
    }

#endif

    //
    // Complete early initialization of the KDNET library to hand off all
    // required configuration.
    //

    initializeEarly((VOID *)(kdnetData + (hardwareContextSize + 1) * EFI_PAGE_SIZE), &KdNetParameters);

    // Publish hob with where kdnet was relocated to.
    HobAddGuidData(&gMsvmDebuggerKdnetBinaryGuid,
        &hobData,
        sizeof(EFI_KDNET_HOB));
}
