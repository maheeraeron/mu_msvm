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
// Establish Windows style alias for processor architecture.
//

#include <ProcessorBind.h>
#if defined MDE_CPU_IA32
#define _X86_
#endif
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
typedef unsigned long ULONG, *PULONG, ULONG32, *PULONG32;
typedef long long LONGLONG, *PLONGLONG, LONG64, *PLONG64;
typedef unsigned long long ULONGLONG, *PULONGLONG, ULONG64, *PULONG64;
typedef unsigned int UINT, *PUINT;
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

//
// General defines.
//

#if defined(MDE_CPU_X64) || defined(MDE_CPU_IA32)

#if defined(MDE_CPU_X64)

#define MemoryBarrier() __faststorefence()

#pragma intrinsic(__cpuid)

void
__cpuid(
    int CPUInfo[4],
    int InfoType
    );

#elif defined(MDE_CPU_IA32)

FORCEINLINE
VOID
MemoryBarrier (
    VOID
    )
{
    LONG Barrier;
    __asm {
        xchg Barrier, eax
    }
}
#endif

FORCEINLINE
LONG
ReadAcquire (
    _In_ _Interlocked_operand_ LONG const volatile *Source
    )

{

    LONG Value;

    Value = *Source;
    return Value;
}

FORCEINLINE
LONG
ReadNoFence (
    _In_ _Interlocked_operand_ LONG const volatile *Source
    )

{

    LONG Value;

    Value = *Source;
    return Value;
}

FORCEINLINE
VOID
WriteRelease (
    _Out_ _Interlocked_operand_ LONG volatile *Destination,
    _In_ LONG Value
    )

{

    *Destination = Value;
    return;
}

FORCEINLINE
VOID
WriteNoFence (
    _Out_ _Interlocked_operand_ LONG volatile *Destination,
    _In_ LONG Value
    )

{

    *Destination = Value;
    return;
}

FORCEINLINE
VOID
WriteNoFence16 (
    _Out_ _Interlocked_operand_ SHORT volatile *Destination,
    _In_ SHORT Value
    )

{

    *Destination = Value;
    return;
}

#elif defined(MDE_CPU_AARCH64)

#pragma intrinsic(__dmb)

typedef enum _tag_ARM64INTR_BARRIER_TYPE
{
    _ARM64_BARRIER_SY    = 0xF,
    _ARM64_BARRIER_ST    = 0xE,
    _ARM64_BARRIER_LD    = 0xD,
    _ARM64_BARRIER_ISH   = 0xB,
    _ARM64_BARRIER_ISHST = 0xA,
    _ARM64_BARRIER_ISHLD = 0x9,
    _ARM64_BARRIER_NSH   = 0x7,
    _ARM64_BARRIER_NSHST = 0x6,
    _ARM64_BARRIER_NSHLD = 0x5,
    _ARM64_BARRIER_OSH   = 0x3,
    _ARM64_BARRIER_OSHST = 0x2,
    _ARM64_BARRIER_OSHLD = 0x1
}
_ARM64INTR_BARRIER_TYPE;

void __dmb(unsigned int _Type);

#define MemoryBarrier() __dmb(_ARM64_BARRIER_SY)

FORCEINLINE
LONG
ReadAcquire (
    _In_ _Interlocked_operand_ LONG const volatile *Source
    )

{

    LONG Value;

    Value = __iso_volatile_load32((int *)Source);
    __dmb(_ARM64_BARRIER_ISH);
    return Value;
}

FORCEINLINE
LONG
ReadNoFence (
    _In_ _Interlocked_operand_ LONG const volatile *Source
    )

{

    LONG Value;

    Value = __iso_volatile_load32((int *)Source);
    return Value;
}

FORCEINLINE
VOID
WriteRelease (
    _Out_ _Interlocked_operand_ LONG volatile *Destination,
    _In_ LONG Value
    )

{

    __dmb(_ARM64_BARRIER_ISH);
    __iso_volatile_store32((int *)Destination, Value);
    return;
}

FORCEINLINE
VOID
WriteNoFence (
    _Out_ _Interlocked_operand_ LONG volatile *Destination,
    _In_ LONG Value
    )

{

    __iso_volatile_store32((int *)Destination, Value);
    return;
}

FORCEINLINE
VOID
WriteNoFence16 (
    _Out_ _Interlocked_operand_ SHORT volatile *Destination,
    _In_ SHORT Value
    )

{

    __iso_volatile_store16(Destination, Value);
    return;
}

#else
#error Unsupported architecture
#endif

#define MemoryBarrierWithoutFence() _ReadWriteBarrier()

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

#ifndef ANYSIZE_ARRAY
#define ANYSIZE_ARRAY 1       // winnt
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