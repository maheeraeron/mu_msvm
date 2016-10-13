/** @file
  Ihis library is TPM2 TREE protocol lib.

Copyright (c) 2013, Intel Corporation. All rights reserved. <BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/TimerLib.h>
#include <IndustryStandard/Tpm20.h>
#include <IndustryStandard/Tpm2Acpi.h>

EFI_TPM2_ACPI_CONTROL_AREA  *mTpm2ControlArea = NULL;
UINT8*   mCommandBuffer = NULL;
UINT8*   mResponseBuffer = NULL;
UINT32   mResponseSize = 0;

EFI_STATUS
EFIAPI
CRSubmitCommand (
    IN  UINT32   InputParameterBlockSize,
    IN  UINT8    *InputParameterBlock,
    IN  UINT32   OutputParameterBlockSize,
    IN  UINT8    *OutputParameterBlock
    )
/*++

Routine Description:

    This routine submits TPM command to vTPM engine.

Arguments:

    InputParameterBlockSize - Command size

    InputParameterBlock - Command Buffer

    OutputParameterBlockSize - Response buffer size

    OutputParameterBlock - Response buffer

Return Value:

    EFI_STATUS

--*/
{
    EFI_STATUS status = EFI_SUCCESS;
    UINT32  outputParameterSize = OutputParameterBlockSize;
    UINT32  waitTime;

    if (mTpm2ControlArea == NULL)
    {
        status = EFI_NOT_READY;
        goto Cleanup;
    }

    if (mTpm2ControlArea->Start != 0)
    {
        // pending command.
        status = EFI_NOT_READY;
        goto Cleanup;
    }

    if (mTpm2ControlArea->Error != 0)
    {
        // device in error state.
        status = EFI_DEVICE_ERROR;
        goto Cleanup;
    }

    // Check if command fits into command buffer.
    if (mTpm2ControlArea->CommandSize < InputParameterBlockSize)
    {
        status = EFI_INVALID_PARAMETER;
        goto Cleanup;
    }

    // Copy command to command buffer.
    CopyMem(mCommandBuffer, InputParameterBlock, InputParameterBlockSize);

    // Set Start to kick off command execution.
    mTpm2ControlArea->Start = 1;

    //
    // Wait/Poll for 90 secs timeout.
    //
    for (waitTime = 0; waitTime < 90000 * 1000; waitTime += 30)
    {
        if (mTpm2ControlArea->Start != 0)
        {
            MicroSecondDelay (30);
            continue;
        }

        if (mTpm2ControlArea->Error != 0)
        {
            status = EFI_DEVICE_ERROR;
            goto Cleanup;
        }

        //
        // Engine finished executing command, get the result back.
        //
        if (outputParameterSize > mResponseSize)
        {
            outputParameterSize = mResponseSize;
        }

        CopyMem(OutputParameterBlock, mResponseBuffer, outputParameterSize);
        goto Cleanup;
    }

    status = EFI_TIMEOUT;
    DEBUG ((EFI_D_ERROR, "SubmitCommand TIMEOUT - %r\n", status));

Cleanup:

    return status;
}


/**
  This service enables the sending of commands to the TPM2.

  @param[in]      InputParameterBlockSize  Size of the TPM2 input parameter block.
  @param[in]      InputParameterBlock      Pointer to the TPM2 input parameter block.
  @param[in,out]  OutputParameterBlockSize Size of the TPM2 output parameter block.
  @param[in]      OutputParameterBlock     Pointer to the TPM2 output parameter block.

  @retval EFI_SUCCESS            The command byte stream was successfully sent to the device and a response was successfully received.
  @retval EFI_DEVICE_ERROR       The command was not successfully sent to the device or a response was not successfully received from the device.
  @retval EFI_BUFFER_TOO_SMALL   The output parameter block is too small.
**/
EFI_STATUS
EFIAPI
Tpm2SubmitCommand (
  IN UINT32            InputParameterBlockSize,
  IN UINT8             *InputParameterBlock,
  IN OUT UINT32        *OutputParameterBlockSize,
  IN UINT8             *OutputParameterBlock
  )
{
    EFI_STATUS                status = EFI_SUCCESS;
    UINT32                    outputParameterBlockSize = (*OutputParameterBlockSize);
    TPM2_RESPONSE_HEADER      *header;

    if (InputParameterBlockSize < sizeof(TPM2_COMMAND_HEADER) || InputParameterBlock == NULL ||
        outputParameterBlockSize < sizeof(TPM2_RESPONSE_HEADER) || OutputParameterBlock == NULL)
    {
        status = EFI_INVALID_PARAMETER;
        goto Cleanup;
    }

    status = CRSubmitCommand(InputParameterBlockSize,
                                InputParameterBlock,
                                outputParameterBlockSize,
                                OutputParameterBlock
                                );
    if (EFI_ERROR (status))
    {
        goto Cleanup;
    }

    header = (TPM2_RESPONSE_HEADER *)OutputParameterBlock;
    *OutputParameterBlockSize = SwapBytes32 (header->paramSize);
    if (outputParameterBlockSize < (*OutputParameterBlockSize))
    {
        status = EFI_BUFFER_TOO_SMALL;
    }

Cleanup:

    return status;
}

/**
  This service requests use TPM2.

  @retval EFI_SUCCESS      Get the control of TPM2 chip.
  @retval EFI_NOT_FOUND    TPM2 not found.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2RequestUseTpm (
  VOID
  )
{
    return EFI_SUCCESS;
}

/**
  This service register TPM2 device.

  @param Tpm2Device  TPM2 device

  @retval EFI_SUCCESS          This TPM2 device is registered successfully.
  @retval EFI_UNSUPPORTED      System does not support register this TPM2 device.
  @retval EFI_ALREADY_STARTED  System already register this TPM2 device.
**/
EFI_STATUS
EFIAPI
Tpm2RegisterTpm2DeviceLib (
  IN TPM2_DEVICE_INTERFACE   *Tpm2Device
  )
{
    mTpm2ControlArea = (EFI_TPM2_ACPI_CONTROL_AREA*)Tpm2Device;
    mCommandBuffer = (UINT8*)(UINTN)mTpm2ControlArea->Command;
    mResponseBuffer = (UINT8*)(UINTN)mTpm2ControlArea->Response;
    mResponseSize = (UINTN)mTpm2ControlArea->ResponseSize;

    return EFI_SUCCESS;
}
