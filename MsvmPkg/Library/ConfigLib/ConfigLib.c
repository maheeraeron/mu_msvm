/*++

Copyright (c) Microsoft Corporation

Module Name:

    ConfigLib.c

Abstract:

    Library that provides version agnostic access to virtual machine configuration.

Author:

    Larry Cleeton (lcleeton) - 02-May-2014

--*/

#include <EfiNt.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <BiosInterface.h>
#include <BiosConfigPageGuid.h>
#include <BiosDeviceAccess.h>

//
// Private global to indicate that library initialization was performed.
//
static BOOLEAN gConfigLibInitialized = FALSE;

//
// Private global that indicates which config version is active.
//
static UINT32 gConfigVersion = 0;

//
// Private global pointer to the V2 config page.
//
static BIOS_CONFIG_PAGE_V2 *gConfigPageV2 = NULL;

//
// Private global pointer to the V3 config page.
//
static BIOS_CONFIG_PAGE_V3 *gConfigPageV3 = NULL;

//
// Private global that indicates the Vdev Version.
//
static UINT32 gVDevVersion = 0;

//
// Config page versions.
//
static const UINT32 ConfigPageV2 = 2;
static const UINT32 ConfigPageV3 = 3;


#ifdef DUMP_CONFIG_PAGES
static
void
DebugDumpConfigPageV2(
    _In_ CONST BIOS_CONFIG_PAGE_V2* ConfigPage
    )
/*++

Routine Description:

    Private routine to debug dump a version 2 config page.

Arguments:

    ConfigPage      pointer to the config page

Return Value:

    n/a

--*/
{
    DEBUG((DEBUG_VERBOSE, "--- V2 Config Data @ %x\n", ConfigPage));
    if (ConfigPage == NULL)
    {
        return;
    }
    DEBUG((DEBUG_VERBOSE, "  Size                    % 17x\n", ConfigPage->Size));
    DEBUG((DEBUG_VERBOSE, "  SratSize                % 17x\n", ConfigPage->SratSize));
    DEBUG((DEBUG_VERBOSE, "  BiosSizePages           % 17x\n", ConfigPage->BiosSizePages));
    DEBUG((DEBUG_VERBOSE, "  ProcessorCount          % 17x\n", ConfigPage->ProcessorCount));
    DEBUG((DEBUG_VERBOSE, "  LowMemoryBasePages      % 17lx\n", ConfigPage->LowMemoryBasePages));
    DEBUG((DEBUG_VERBOSE, "  LowMemoryLengthPages    % 17lx\n", ConfigPage->LowMemoryLengthPages));
    DEBUG((DEBUG_VERBOSE, "  MiddleMemoryBasePages   % 17lx\n", ConfigPage->MiddleMemoryBasePages));
    DEBUG((DEBUG_VERBOSE, "  MiddleMemoryLengthPages % 17lx\n", ConfigPage->MiddleMemoryLengthPages));
    DEBUG((DEBUG_VERBOSE, "  HighMemoryBasePages     % 17lx\n", ConfigPage->HighMemoryBasePages));
    DEBUG((DEBUG_VERBOSE, "  HighMemoryLengthPages   % 17lx\n", ConfigPage->HighMemoryLengthPages));
    DEBUG((DEBUG_VERBOSE, "  LowMmioGapBasePages     % 17lx\n", ConfigPage->LowMmioGapBasePages));
    DEBUG((DEBUG_VERBOSE, "  LowMmioGapLengthPages   % 17lx\n", ConfigPage->LowMmioGapLengthPages));
    DEBUG((DEBUG_VERBOSE, "  HighMmioGapBasePages    % 17lx\n", ConfigPage->HighMmioGapBasePages));
    DEBUG((DEBUG_VERBOSE, "  HighMmioGapLengthPages  % 17lx\n", ConfigPage->HighMmioGapLengthPages));
    DEBUG((DEBUG_VERBOSE, "  GUID:                %g\n", (EFI_GUID *)ConfigPage->BiosGuid));
    DEBUG((DEBUG_VERBOSE, "  SystemSerialNumber:  %a\n", ConfigPage->SystemSerialNumber));
    DEBUG((DEBUG_VERBOSE, "  BaseSerialNumber:    %a\n", ConfigPage->BaseSerialNumber));
    DEBUG((DEBUG_VERBOSE, "  ChassisSerialNumber: %a\n", ConfigPage->ChassisSerialNumber));
    DEBUG((DEBUG_VERBOSE, "  ChassisAssetTag:     %a\n", ConfigPage->ChassisAssetTag));
    DEBUG((DEBUG_VERBOSE, "  BiosLockString:      %a\n", ConfigPage->BiosLockString));
    DEBUG((DEBUG_VERBOSE, "  DebuggerEnabled            %a\n",
        ConfigPage->Flags.DebuggerEnabled == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  LoadOempTable              %a\n",
        ConfigPage->Flags.LoadOempTable == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  SerialControllersEnabled   %a\n",
        ConfigPage->Flags2.SerialControllersEnabled == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  PauseAfterBootFailure      %a\n",
        ConfigPage->Flags2.PauseAfterBootFailure == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  PxeIpV6                    %a\n",
        ConfigPage->Flags2.PxeIpV6 == 1 ? "TRUE" : "FALSE"));
}
#endif


#ifdef DUMP_CONFIG_PAGES
static
void
DebugDumpConfigPageV3(
    _In_ CONST BIOS_CONFIG_PAGE_V3* ConfigPage
    )
/*++

Routine Description:

    Private routine to debug dump a version 3 config page.

Arguments:

    ConfigPage      pointer to the config page

Return Value:

    n/a

--*/
{
    DEBUG((DEBUG_VERBOSE, "--- V3 Config Data @ %x\n", ConfigPage));
    if (ConfigPage == NULL)
    {
        return;
    }
    DEBUG((DEBUG_VERBOSE, "  Size                    % 17x\n", ConfigPage->Size));
    DEBUG((DEBUG_VERBOSE, "  SratSize                % 17x\n", ConfigPage->SratSize));
    DEBUG((DEBUG_VERBOSE, "  BiosSizePages           % 17x\n", ConfigPage->BiosSizePages));
    DEBUG((DEBUG_VERBOSE, "  ProcessorCount          % 17x\n", ConfigPage->ProcessorCount));
    DEBUG((DEBUG_VERBOSE, "  LowMmioGapBasePages     % 17lx\n", ConfigPage->LowMmioGapBasePages));
    DEBUG((DEBUG_VERBOSE, "  LowMmioGapLengthPages   % 17lx\n", ConfigPage->LowMmioGapLengthPages));
    DEBUG((DEBUG_VERBOSE, "  HighMmioGapBasePages    % 17lx\n", ConfigPage->HighMmioGapBasePages));
    DEBUG((DEBUG_VERBOSE, "  HighMmioGapLengthPages  % 17lx\n", ConfigPage->HighMmioGapLengthPages));
    DEBUG((DEBUG_VERBOSE, "  GUID:                %g\n", (EFI_GUID *)ConfigPage->BiosGuid));
    DEBUG((DEBUG_VERBOSE, "  SystemSerialNumber:  %a\n", ConfigPage->SystemSerialNumber));
    DEBUG((DEBUG_VERBOSE, "  BaseSerialNumber:    %a\n", ConfigPage->BaseSerialNumber));
    DEBUG((DEBUG_VERBOSE, "  ChassisSerialNumber: %a\n", ConfigPage->ChassisSerialNumber));
    DEBUG((DEBUG_VERBOSE, "  ChassisAssetTag:     %a\n", ConfigPage->ChassisAssetTag));
    DEBUG((DEBUG_VERBOSE, "  BiosLockString:      %a\n", ConfigPage->BiosLockString));
    DEBUG((DEBUG_VERBOSE, "  SerialControllersEnabled   %a\n",
        ConfigPage->Flags.SerialControllersEnabled == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  PauseAfterBootFailure      %a\n",
        ConfigPage->Flags.PauseAfterBootFailure == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  PxeIpV6                    %a\n",
        ConfigPage->Flags.PxeIpV6 == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  DebuggerEnabled            %a\n",
        ConfigPage->Flags.DebuggerEnabled == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  LoadOempTable              %a\n",
        ConfigPage->Flags.LoadOempTable == 1 ? "TRUE" : "FALSE"));
    DEBUG((DEBUG_VERBOSE, "  TpmEnabled                 %a\n",
        ConfigPage->Flags.TpmEnabled == 1 ? "TRUE" : "FALSE"));
}
#endif

static
void
InitializeVdevVersion(
    void
)
/*++

Routine Description:

    Caches the VDevVersion from the Worker process.

Arguments:

    None

Return Value:

    None.

--*/
{
    //
    // Try to get the VDev version from the worker process.
    //
    gVDevVersion = ReadBiosDevice(BiosConfigVdevVersion);
    if (gVDevVersion == 0)
    {
        //
        // If the version comes back zero it must be the Windows Blue
        // VDev that didn't support getting the version.
        // Therefore it is implicitly version 2.
        //
        gVDevVersion = VDevVersion2;
        DEBUG((DEBUG_VERBOSE, "*** VDev version returned as 0. Defaulting to V2 (512).\n"));
    }
    DEBUG((DEBUG_VERBOSE, "--- VDev version is %d.%d\n", vDevVersion >> 8, vDevVersion & 0xFF));

}

static
void
ConfigLibInitialize(
    void
    )
/*++

Routine Description:

    Private routine that initializes the config library. Allows for the
    library to work efficiently without an explicit library constructor.

Arguments:

    None

Return Value:

    n/a
--*/
{
    void* hob;

    //
    // Do nothing if already intialized.
    //
    if (gConfigLibInitialized == TRUE)
    {
        return;
    }

    //
    // First try to get a pointer to the V2 config page.
    //
    hob = GetFirstGuidHob(&gMsvmConfigPageV2Guid);
    if (hob != NULL)
    {
        gConfigVersion = ConfigPageV2;
        gConfigPageV2 = (BIOS_CONFIG_PAGE_V2 *)GET_GUID_HOB_DATA(hob);
    }
    else
    {
        //
        // If not V2 then get a pointer to the V3 config page.
        //
        hob = GetFirstGuidHob(&gMsvmConfigPageV3Guid);
        if (hob != NULL)
        {
            gConfigVersion = ConfigPageV3;
            gConfigPageV3 = (BIOS_CONFIG_PAGE_V3 *)GET_GUID_HOB_DATA(hob);
        }
    }

    //
    // Get the vdev version.
    //
    InitializeVdevVersion();

#ifdef DUMP_CONFIG_PAGES
    DebugDumpConfigPageV2(gConfigPageV2);
    DebugDumpConfigPageV3(gConfigPageV3);
#endif

    gConfigLibInitialized = TRUE;

}


UINT32
GetSratSize(
    void
    )
/*++

Routine Description:

    Returns the size of the SRAT ACPI table.

Arguments:

    None

Return Value:

    The size of the SRAT ACPI table.

--*/
{
    UINT32 value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->SratSize;
    }
    else
    {
        value = gConfigPageV2->SratSize;
    }
    return value;
}


void
GetSrat(
    UINT64 Address
    )
/*++

Routine Description:

    Puts the SRAT ACPI table at the address specified.

Arguments:

    Address - the GPA at which to write the SRAT table

Return Value:

    None

--*/
{
    ASSERT(Address < 0xFFFFFFFFULL);
    WriteBiosDevice(BiosConfigSratData, (UINT32) Address);
}

UINT32
GetNfitSize(
    )
/*++

Routine Description:

    Returns the size of the NFIT.

Arguments:

    None

Return Value:

    The size of the NFIT.

--*/
{
    return ReadBiosDevice(BiosConfigNfitSize);
}

void
GetNfit(
    UINT64 Address
    )
/*++

Routine Description:

    Gets the NFIT.

Arguments:

    Address - the GPA at which to write the NFIT table

Return Value:

    None.

--*/
{
    ASSERT((UINT64) Address < 0xFFFFFFFFULL);
    WriteBiosDevice(BiosConfigNfitPopulate, (UINT32) Address);
}

void
SetVpmemACPIBuffer(
    UINT64 Address
    )
/*++

Routine Description:

    Sets the pointer to the VPMem ACPI Method Buffer.

Arguments:

    None

Return Value:

    None.

--*/
{
    ASSERT(Address < 0xFFFFFFFFULL);
    WriteBiosDevice(BiosConfigVpmemSetACPIBuffer, (UINT32) Address);
}

UINT32
GetMemmapSize(
    void
    )
/*++

Routine Description:

    Returns the size of the memory map table.

Arguments:

    None

Return Value:

    The size of the memory map table.

--*/
{
    UINT32 value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->MemoryMapSize;
    }
    else
    {
        value = ReadBiosDevice(BiosConfigMemoryMapSize);
    }
    return value;
}


void
GetMemmap(
    UINT64 Address
    )
/*++

Routine Description:

    Puts the memory map table at the address specified.

Arguments:

    Address - the GPA at which to write the Memory Map table

Return Value:

    None

--*/
{
    ASSERT((UINT64) Address < 0xFFFFFFFFULL);
    WriteBiosDevice(BiosConfigMemoryMap, (UINT32) Address);
}


UINT32
GetBiosSizePages(
    void
    )
/*++

Routine Description:

    Returns the BIOS image size in 4K page units.

Arguments:

    None

Return Value:

    The BIOS image size in 4K page units.

--*/
{
    UINT32 value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->BiosSizePages;
    }
    else
    {
        value = gConfigPageV2->BiosSizePages;
    }
    return value;
}


UINT32
GetProcessorCount(
    void
    )
/*++

Routine Description:

    Returns the processor count.

Arguments:

    None

Return Value:

    The processor count.

--*/
{
    UINT32 value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->ProcessorCount;
    }
    else
    {
       value = gConfigPageV2->ProcessorCount;
    }
    return value;
}


UINT64
GetLowMmioGapBasePages(
    void
    )
/*++

Routine Description:

    Returns the size of the low MMIO space in 4K page units.

Arguments:

    None

Return Value:

    The size of the low MMIO space in 4K page units.

--*/
{
    UINT64 value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->LowMmioGapBasePages;
    }
    else
    {
        value = gConfigPageV2->LowMmioGapBasePages;
    }
    return value;
}


UINT64
GetLowMmioGapLengthPages(
    void
    )
/*++

Routine Description:

    Returns the length of the low MMIO space in 4K page units.

Arguments:

    None

Return Value:

    The length of the low MMIO space in 4K page units.

--*/
{
    UINT64 value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->LowMmioGapLengthPages;
    }
    else
    {
        value = gConfigPageV2->LowMmioGapLengthPages;
    }
    return value;
}


UINT64
GetHighMmioGapBasePages(
    void
    )
/*++

Routine Description:

    Returns the size of the high MMIO space in 4K page units.

Arguments:

    None

Return Value:

    The size of the high MMIO space in 4K page units.

--*/
{
    UINT64 value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->HighMmioGapBasePages;
    }
    else
    {
        value = gConfigPageV2->HighMmioGapBasePages;
    }
    return value;
}


UINT64
GetHighMmioGapLengthPages(
    void
    )
/*++

Routine Description:

    Returns the length of the high MMIO space in 4K page units.

Arguments:

    None

Return Value:

    The length of the high MMIO space in 4K page units.

--*/
{
    UINT64 value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->HighMmioGapLengthPages;
    }
    else
    {
        value = gConfigPageV2->HighMmioGapLengthPages;
    }
    return value;
}


void*
GetEntropyData(
    void
    )
/*++

Routine Description:

    Returns a pointer to the entropy data.

Arguments:

    None

Return Value:

    The address of the entropy data.

--*/
{
    void* ptr;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        ptr = gConfigPageV3->Entropy;
    }
    else
    {
        ptr = gConfigPageV2->Entropy;
    }
    return ptr;
}


EFI_GUID*
GetBiosGuid(
    void
    )
/*++

Routine Description:

    Returns the SMBIOS UUID.

Arguments:

    None

Return Value:

    The SMBIOS UUID.

--*/
{
    EFI_GUID* ptr;
    if (gConfigVersion == ConfigPageV3)
    {
        ptr = (EFI_GUID*)gConfigPageV3->BiosGuid;
    }
    else
    {
        ptr = (EFI_GUID*)gConfigPageV2->BiosGuid;
    }
    return ptr;
}


CHAR8*
GetSmbiosSystemSerialNumberString(
    void
    )
/*++

Routine Description:

    Returns the SMBIOS System Serial Number.

Arguments:

    None

Return Value:

    The SMBIOS System Serial Number.

--*/
{
    CHAR8* ptr;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        ptr = gConfigPageV3->SystemSerialNumber;
    }
    else
    {
        ptr = gConfigPageV2->SystemSerialNumber;
    }
    return ptr;
}


CHAR8*
GetSmbiosBaseSerialNumberString(
    void
    )
/*++

Routine Description:

    Returns the SMBIOS Base Serial Number.

Arguments:

    None

Return Value:

    The SMBIOS Base Serial Number.

--*/
{
    CHAR8* ptr;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        ptr = gConfigPageV3->BaseSerialNumber;
    }
    else
    {
        ptr = gConfigPageV2->BaseSerialNumber;
    }
    return ptr;
}


CHAR8*
GetSmbiosChassisSerialNumberString(
    void
    )
/*++

Routine Description:

    Returns the SMBIOS Chassis Serial Number.

Arguments:

    None

Return Value:

    The SMBIOS Chassis Serial Number.

--*/
{
    CHAR8* ptr;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        ptr = gConfigPageV3->ChassisSerialNumber;
    }
    else
    {
        ptr = gConfigPageV2->ChassisSerialNumber;
    }
    return ptr;
}


CHAR8*
GetSmbiosChassisAssetTagString(
    void
    )
/*++

Routine Description:

    Returns the SMBIOS Chassis Asset Tag.

Arguments:

    None

Return Value:

    The SMBIOS Chassis Asset Tag.

--*/
{
    CHAR8* ptr;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        ptr = gConfigPageV3->ChassisAssetTag;
    }
    else
    {
        ptr = gConfigPageV2->ChassisAssetTag;
    }
    return ptr;
}


CHAR8*
GetSmbiosOemBiosLockString(
    void
    )
/*++

Routine Description:

    Returns the SMBIOS OEM BIOS Lock String.

Arguments:

    None

Return Value:

    The SMBIOS OEM BIOS Lock String.

--*/
{
    CHAR8* ptr;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        ptr = gConfigPageV3->BiosLockString;
    }
    else
    {
        ptr = gConfigPageV2->BiosLockString;
    }
    return ptr;
}


void*
GetSmbiosV24CpuInfo(
    void
    )
/*++

Routine Description:

    Returns a pointer to the SMBIOS CPU Information structure.

Arguments:

    None

Return Value:

    The address of the SMBIOS CPU Information structure.

--*/
{
    void* ptr;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        ptr = (void *)&gConfigPageV3->ProcessorInformation;
    }
    else
    {
        ptr = (void *)&gConfigPageV2->ProcessorInformation;
    }
    return ptr;
}


BOOLEAN
GetDebuggerEnabled(
    void
    )
/*++

Routine Description:

    Returns TRUE if the Debugger is enabled.

Arguments:

    None

Return Value:

    TRUE if the debugger is enabled, FALSE otherwise.

--*/
{
    BOOLEAN value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->Flags.DebuggerEnabled == 1 ? TRUE : FALSE;
    }
    else
    {
        value = gConfigPageV2->Flags.DebuggerEnabled == 1 ? TRUE : FALSE;
    }
    return value;
}


BOOLEAN
GetSerialControllersEnabled(
    void
    )
/*++

Routine Description:

    Returns TRUE if the Serial Controllers are enabled.

Arguments:

    None

Return Value:

    TRUE if the serial controllers are enabled, FALSE otherwise.

--*/
{
    BOOLEAN value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->Flags.SerialControllersEnabled == 1 ? TRUE : FALSE;
    }
    else
    {
        value = gConfigPageV2->Flags2.SerialControllersEnabled == 1 ? TRUE : FALSE;
    }
    return value;
}


BOOLEAN
GetPauseAfterBootFailure(
    void
    )
/*++

Routine Description:

    Returns TRUE if "Pause after boot failure" is enabled.

Arguments:

    None

Return Value:

    TRUE if "Pause after boot failure" is enabled, FALSE otherwise.

--*/
{
    BOOLEAN value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->Flags.PauseAfterBootFailure == 1 ? TRUE : FALSE;
    }
    else
    {
        value = gConfigPageV2->Flags2.PauseAfterBootFailure == 1 ? TRUE : FALSE;
    }
    return value;
}


BOOLEAN
GetPxeIpV6Enabled(
    void
    )
/*++

Routine Description:

    Returns TRUE if PXE should used IPv6 versus IPv4.

Arguments:

    None

Return Value:

    TRUE if PXE should use IPv6 - FALSE otherwise.

--*/
{
    BOOLEAN value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->Flags.PxeIpV6 == 1 ? TRUE : FALSE;
    }
    else
    {
        value = gConfigPageV2->Flags2.PxeIpV6 == 1 ? TRUE : FALSE;
    }
    return value;
}


BOOLEAN
GetTpmEnabled(
    void
    )
/*++

Routine Description:

    Returns TRUE if the virtual TPM is enabled.

Arguments:

    None

Return Value:

    TRUE if the virtual TPM is enabled - FALSE otherwise.

--*/
{
    BOOLEAN value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->Flags.TpmEnabled == 1 ? TRUE : FALSE;
    }
    else
    {
        value = FALSE;
    }
    return value;
}


BOOLEAN
GetOempEnabled(
    void
    )
/*++

Routine Description:

    Returns TRUE if the OEMP ACPI table should be loaded in the DSDT.

Arguments:

    None

Return Value:

    TRUE if OEMP ACPI table should be loaded - FALSE otherwise.

--*/
{
    BOOLEAN value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->Flags.LoadOempTable == 1 ? TRUE : FALSE;
    }
    else
    {
        value = gConfigPageV2->Flags.LoadOempTable == 1 ? TRUE : FALSE;
    }
    return value;
}

BOOLEAN
GetHibernateEnabled(
    void
    )
/*++

Routine Description:

    Returns TRUE if the hibernate should be described in ACPI.

Arguments:

    None

Return Value:

    TRUE if hibernate should be described - FALSE otherwise.

--*/
{
    BOOLEAN value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->Flags.HibernateEnabled == 1 ? TRUE : FALSE;
    }
    else
    {
        value = gConfigPageV2->Flags.HibernateEnabled == 1 ? TRUE : FALSE;
    }
    return value;
}

BOOLEAN
GetVirtualBatteryEnabled(
    )
/*++

Routine Description:

    Returns TRUE if the virtual battery device should be loaded.

Arguments:

    None

Return Value:

    TRUE if virtual battery is enabled, FALSE otherwise.

--*/
{
    BOOLEAN value = FALSE;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->Flags.VirutalBatteryEnabled == 1 ? TRUE : FALSE;
    }
    return value;
}

void
SetGenerationIdAddress(
    UINT64 Value
    )
/*++

Routine Description:

    Communicates the Generation ID memory location to the VDev.

Arguments:

    Value - the GPA of the Generation ID

Return Value:

    n/a

--*/
{
    // No need for ConfigLibInitialize()
    IoWrite32(BiosAddressPort, BiosConfigGenerationIdPtrLow);
    IoWrite32(BiosDataPort, (UINT32)Value);
    IoWrite32(BiosAddressPort, BiosConfigGenerationIdPtrHigh);
    IoWrite32(BiosDataPort, (UINT32)(Value >> 32));
}

UINT32
GetConsoleMode(
    void
    )
/*++

Routine Description:

    Returns the console mode.

Arguments:

    n/a

Return Value:

    The console mode of the VM.

--*/
{
    UINT32 value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->Flags.ConsoleMode;
    }
    else
    {
        value = 0;
    }
    return value;
}

BOOLEAN
GetMemoryAttributesTableEnabled
(
    void
    )
/*++

Routine Description:

    Returns TRUE if the VM is an Xenon VM.

Arguments:

    None

Return Value:

    TRUE if VM is a Xenon VM - FALSE otherwise.

--*/
{
    BOOLEAN value;
    ConfigLibInitialize();
    if (gConfigVersion == ConfigPageV3)
    {
        value = gConfigPageV3->Flags.MemoryAttributesTableEnabled == 1 ? TRUE : FALSE;
    }
    else
    {
        value = FALSE;
    }
    return value;
}

UINT32
GetVDevVersion
(
    void
    )
/*++

Routine Description:

    Returns the VDevVersion.

Arguments:

    None

Return Value:

    The VDevVersion.

--*/
{
    ConfigLibInitialize();
    return gVDevVersion;
}
