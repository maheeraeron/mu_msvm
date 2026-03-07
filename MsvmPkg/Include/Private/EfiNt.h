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
#include <stddef.h>
#include <stdint.h>
#if !(defined (__clang__) || defined (__GNUC__))
#include <specstrings.h>
#endif
#include "AllowNamelessAggregate.h"
#include "EmptySal.h"
#include "Inline.h"
#include "WarningDisable.h"
MS_WARNING_DISABLE(4200) // nonstandard: zero-sized array in struct/union
MS_WARNING_DISABLE(4214) // nonstandard: bit field types other than int
MS_WARNING_DISABLE(4324) // pad due to __declspec(align())

//
// Establish Windows style alias for processor architecture.
//

#include <ProcessorBind.h>
#if defined MDE_CPU_X64
#define _AMD64_
#endif
#if defined MDE_CPU_ARM
#define _ARM_
#endif
#if defined MDE_CPU_AARCH64
#define _ARM64_
#endif

//
// Declspec wrappers.
//

#include "DeclspecAlign.h"
#include "DeclspecCacheAlign.h"
#define FORCEINLINE __forceinline

//
// String types.
//
typedef CHAR16 WCHAR, *PWCHAR;
typedef __nullterminated CHAR8 *PSTR;
typedef __nullterminated const CHAR8 *PCSTR;
typedef __nullterminated CHAR16 *PWSTR;
typedef __nullterminated const CHAR16* PCWSTR;

typedef char CHAR, *PCHAR;
typedef uint8_t UCHAR, *PUCHAR;
typedef int16_t SHORT, *PSHORT;
typedef uint16_t USHORT, *PUSHORT;
typedef int32_t LONG, *PLONG;
typedef uint32_t ULONG, *PULONG, ULONG32, *PULONG32;
typedef int64_t LONGLONG, *PLONGLONG, LONG64, *PLONG64;
typedef uint64_t ULONGLONG, *PULONGLONG, ULONG64, *PULONG64;
typedef uint32_t UINT, *PUINT;
typedef int BOOL;

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

typedef ULONG_PTR KAFFINITY;

typedef struct _STRING
{
    USHORT Length;
    USHORT MaximumLength;
    CHAR *Buffer;
} STRING, *PSTRING;

#define UNREFERENCED_PARAMETER(_Parameter_) ((void)(_Parameter_))
#define FIELD_OFFSET offsetof

#define RTL_FIELD_SIZE(type, field) (sizeof(((type *)0)->field))

#define RTL_SIZEOF_THROUGH_FIELD(type, field) \
    (FIELD_OFFSET(type, field) + RTL_FIELD_SIZE(type, field))

#define CONTAINING_RECORD(Record, TYPE, Field) \
    ((TYPE *) ((CHAR8 *) (Record) - (CHAR8 *) &(((TYPE *) 0)->Field)))

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        extern const GUID __declspec(selectany) name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(x[0]))
#endif

#ifndef ANYSIZE_ARRAY
#define ANYSIZE_ARRAY 1       // winnt
#endif

typedef int32_t NTSTATUS;
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#include <Vmbus/NtStatus.h>
