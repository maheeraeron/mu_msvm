/*++

Copyright (c) Microsoft Corporation

Module Name:

    ConfigLib.h

Abstract:

    A library to simplify access to virtual machine configuration information.

Author:

    Larry Cleeton (lcleeton) - 30-Apr-2014

--*/

#pragma once

#include <EfiNt.h>
#include <BiosInterface.h>

typedef
UINT32
(CONFIG_GET_UINT32)(
    void
    );

typedef
UINT64
(CONFIG_GET_UINT64)(
    void
    );

typedef
void*
(CONFIG_GET_PTR)(
    void
    );

typedef
CHAR8*
(CONFIG_GET_STRING)(
    void
    );

typedef
EFI_GUID*
(CONFIG_GET_GUID)(
    void
    );

typedef
BOOLEAN
(CONFIG_GET_BOOLEAN)(
    void
    );

typedef
void
(CONFIG_SET_UINT64)(
    UINT64 Value
    );

enum
{
    ConfigLibEntropyDataSize =      BiosInterfaceEntropyTableSize,
    ConfigLibSmbiosStringMax =      BiosInterfaceSmbiosStringMax,
    ConfigLibSmbiosV24CpuInfoSize = sizeof(SMBIOS_CPU_INFORMATION)
};

enum
{
    ConfigLibConsoleModeDefault = 0, // video+kbd (having a head)
    ConfigLibConsoleModeCOM1    = 1, // headless with COM1 serial console
    ConfigLibConsoleModeCOM2    = 2, // headless with COM2 serial console
    ConfigLibConsoleModeNone    = 3  // headless
};

CONFIG_GET_UINT32   GetSratSize;
CONFIG_GET_PTR      GetSrat;
CONFIG_GET_UINT32   GetNfitSize;
CONFIG_SET_UINT64   GetNfit;
CONFIG_SET_UINT64   SetVpmemACPIBuffer;
CONFIG_GET_UINT32   GetMemmapSize;
CONFIG_GET_PTR      GetMemmap;
CONFIG_GET_UINT32   GetBiosSizePages;
CONFIG_GET_UINT32   GetProcessorCount;
CONFIG_GET_UINT64   GetLowMmioGapBasePages;
CONFIG_GET_UINT64   GetLowMmioGapLengthPages;
CONFIG_GET_UINT64   GetHighMmioGapBasePages;
CONFIG_GET_UINT64   GetHighMmioGapLengthPages;
CONFIG_GET_PTR      GetEntropyData;
CONFIG_GET_GUID     GetBiosGuid;
CONFIG_GET_STRING   GetSmbiosSystemSerialNumberString;
CONFIG_GET_STRING   GetSmbiosBaseSerialNumberString;
CONFIG_GET_STRING   GetSmbiosChassisSerialNumberString;
CONFIG_GET_STRING   GetSmbiosChassisAssetTagString;
CONFIG_GET_STRING   GetSmbiosOemBiosLockString;
CONFIG_GET_PTR      GetSmbiosV24CpuInfo;
CONFIG_GET_BOOLEAN  GetDebuggerEnabled;
CONFIG_GET_BOOLEAN  GetSerialControllersEnabled;
CONFIG_GET_BOOLEAN  GetPauseAfterBootFailure;
CONFIG_GET_BOOLEAN  GetPxeIpV6Enabled;
CONFIG_GET_BOOLEAN  GetTpmEnabled;
CONFIG_GET_BOOLEAN  GetOempEnabled;
CONFIG_GET_BOOLEAN  GetHibernateEnabled;
CONFIG_SET_UINT64   SetGenerationIdAddress;
CONFIG_GET_UINT32   GetConsoleMode;
CONFIG_GET_BOOLEAN  GetMemoryAttributesTableEnabled;
CONFIG_GET_UINT32   GetVDevVersion;
