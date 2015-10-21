/*++

Copyright (c) Microsoft Corporation

Module Name:

    EfiNt.h

Abstract:

    Definitions necessary to include some NT-based headers in the UEFI
    environment.

Author:

    John Starks (jostarks) - 2-Jul-2012

--*/

#pragma once

//
// Include SAL
//

#include <specstrings.h>

//
// Declspec wrappers.
//

#define DECLSPEC_ALIGN(x) __declspec(align(x))
#define DECLSPEC_CACHEALIGN DECLSPEC_ALIGN(64)
#define DECLSPEC_DEPRECATED __declspec(deprecated)
#define FORCEINLINE __forceinline

//
// String types.
//

typedef CHAR16 WCHAR, *PWCHAR;
typedef __nullterminated CHAR8 *PSTR;
typedef __nullterminated const CHAR8 *PCSTR;
typedef __nullterminated CHAR16 *PWSTR;
typedef __nullterminated const CHAR16* PCWSTR;

//
// "Legacy" types.
//

typedef char CHAR, *PCHAR;
typedef unsigned char UCHAR, *PUCHAR;
typedef short SHORT, *PSHORT;
typedef unsigned short USHORT, *PUSHORT;
typedef long LONG, *PLONG;
typedef unsigned long ULONG, *PULONG;
typedef long long LONGLONG, *PLONGLONG, LONG64, *PLONG64;
typedef unsigned long long ULONGLONG, *PULONGLONG, ULONG64, *PULONG64;
typedef unsigned int UINT, *PUINT;

typedef UINT8 *PUINT8;
typedef UINT16 *PUINT16;
typedef UINT32 *PUINT32;
typedef UINT64 *PUINT64;
typedef INT8 *PINT8;
typedef INT16 *PINT16;
typedef INT32 *PINT32;
typedef INT64 *PINT64;

typedef INTN INT_PTR, *PINT_PTR, LONG_PTR, *PLONG_PTR;
typedef UINTN UINT_PTR, *PUINT_PTR, ULONG_PTR, *PULONG_PTR, SIZE_T;

typedef VOID *PVOID;

//
// General defines.
//

#define MemoryBarrier() __faststorefence()
#define MemoryBarrierWithoutFence() _ReadWriteBarrier()

#pragma intrinsic(__cpuid)

void
__cpuid(
    int CPUInfo[4],
    int InfoType
    );

#define UNREFERENCED_PARAMETER(_Parameter_) (_Parameter_)
#define ARGUMENT_PRESENT(_ArgumentPointer_) ((_ArgumentPointer_) != NULL)
#define FIELD_OFFSET(x, y) ((UINTN)&(((x *)0)->y))

#define RTL_FIELD_SIZE(type, field) (sizeof(((type *)0)->field))

#define RTL_SIZEOF_THROUGH_FIELD(type, field) \
    (FIELD_OFFSET(type, field) + RTL_FIELD_SIZE(type, field))
    
#define CONTAINING_RECORD(Record, TYPE, Field) \
    ((TYPE *) ((CHAR8 *) (Record) - (CHAR8 *) &(((TYPE *) 0)->Field)))

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        extern const GUID __declspec(selectany) name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#define C_ASSERT(x) static_assert(x, "Failed assertion")

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(x[0]))
#endif

DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

//
// Include NTSTATUS
//

typedef _Return_type_success_(return >= 0) long NTSTATUS;
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#include <ntstatus.h>

//
// 4200 - nonstandard extension used : zero-sized array in struct/union
// 4201 - nonstandard extension used : nameless struct/union
// 4214 - nonstandard extension used : bit field types other than int
// 4324 - structure was padded due to __declspec(align())
//
#pragma warning(disable: 4200 4201 4214 4324)


//
// Some time conversion helper functions
//
#define MS_TO_100NS(x)  ((x) * 10LL * 1000)
#define SEC_TO_100NS(x) ((x) * 10LL * 1000 * 1000)
#define SEC_TO_MS(x)    ((x) * 1000LL)

#define MIN_TO_SEC(x)   ((x) * 60LL)
#define MIN_TO_MS(x)    ((x) * 60LL * 1000)
#define MIN_TO_100NS(x) ((x) * 60LL * 10 * 1000 * 1000)

#define MS_TO_SEC(x)    ((x) / 1000LL)
#define _100NS_TO_MS(x) ((x) / (10LL * 1000))
#define _100NS_TO_S(x)  ((x) / (10LL * 1000 * 1000))