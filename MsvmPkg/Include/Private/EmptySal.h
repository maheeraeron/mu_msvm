// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: BSD-2-Clause-Patent
//
// SAL/Prefast/OACR is a compile feature -- a custom build of Visual C++,
// and not available with Clang or Gcc.
//
// The working annotations are defined in headers carried with the compiler.
//
#pragma once
#if defined (__clang__) || defined (__GNUC__)
#define __analysis_assume(...)
#define __in
#define __in_bcount(...)
#define __in_ecount(...)
#define __in_opt
#define __inout
#define __nullterminated
#define __out
#define __out_bcount(...)
#define __out_bcount_part(...)
#define __out_ecount(...)
#define __out_opt
#define _Analysis_assume_(...)
#define _In_
#define _In_bytecount_(...)
#define _In_opt_
#define _In_z_
#define _Inout_
#define _Inout_bytecount_(...)
#define _Inout_opt_
#define _Out_
#define _Out_opt_
#define _Outptr_
#define _Return_type_success_(...)
#endif
