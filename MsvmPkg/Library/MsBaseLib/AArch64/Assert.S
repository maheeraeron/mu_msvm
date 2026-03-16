# Copyright (c) Microsoft Corporation.
# SPDX-License-Identifier: BSD-2-Clause-Patent

# intrinsic for NT_ASSERT
#
# This is anathema to its usual use.
# Usually it is inline, preserves all context, uses __annotation to find parameters in pdb.

#include <AArch64/AArch64.h>
#include <AsmMacroLib.h>

ASM_FUNC(NtAssert)
  brk   0xf001
  ret
