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

SourceDebugEnabledLib.c

Abstract:

This module implements routines that indicate if source debugging is
runtime enabled for DXE only.

Environment:

DXE

--*/

#include <PiDxe.h>
#include <Base.h>
#include <Library/SourceDebugEnabledLib.h>
#include <Library/HobLib.h>

/**
  Check if source debugging is runtime enabled.

  @InitFlag      Supplies a value that indicates what kind of initialization
                 is being performed. Ignored.

  @return TRUE   Source debugging is enabled.
  @return FALSE  Source debugging is not enabled.

**/
BOOLEAN
EFIAPI
IsSourceDebugEnabled (
  IN UINT32 InitFlag
  )
{
    BOOLEAN debugEnabled = FALSE;

    //
    // There are two ways to figure out if debugging is enabled:
    //      1. Use the PcdDebuggerEnabled set in PEI
    //      2. Use the hob passed that contains if the debugger is enabled
    //
    // We use the hob here since we don't know exactly when this function could
    // be called. If it's called before PCDs are available, early on in DxeCore,
    // the system will die in mysterious ways.
    //
    // This is the same behavior done with the older debug stubs on X64.
    //
    void* hob = GetFirstGuidHob(&gMsvmDebuggerEnabledGuid);
    if (hob != NULL)
    {
        debugEnabled = *((BOOLEAN *)GET_GUID_HOB_DATA(hob));
    }
    else
    {
        // We should always be passing this HOB.
        // ASSERT(FALSE);
    }

    return debugEnabled;
}
