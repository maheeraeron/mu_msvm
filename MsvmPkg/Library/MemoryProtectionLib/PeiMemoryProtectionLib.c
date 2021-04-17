/*++

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2018 Microsoft Corporation. All Rights Reserved.

Module Name:

PeiMemoryProtectionLib.c

Abstract:

This module implements routines that indicate if memory protection
is enabled.

Environment:

SEC, PEI

--*/


#include <Library/MemoryProtectionLib.h>


/**
 Updates the memory protection global toggle

  @param[in] Setting      What the memory protection global toggle should
                          be set to

  @retval EFI_UNSUPPORTED This function is not supported
 **/
EFI_STATUS
EFIAPI
SetMemoryProtectionGlobalToggle(
  IN BOOLEAN     Setting
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Queries the memory protection global toggle

  @retval TRUE            Memory protection global toggle is on
  @retval FALSE           Memory protection global toggle is off, meaning
                          no memory protections can be initialized
 **/
BOOLEAN
EFIAPI
IsMemoryProtectionGlobalToggleEnabled(
  VOID
  )
{
    // For the SEC and PEI phase, always return TRUE so that the memory protections
    // can be applied.
    return TRUE;
}