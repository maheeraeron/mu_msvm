/*++

    Copyright (c) Microsoft Corporation

Module Name:

    WatchdogTimerLib.h

Abstract:

    This module contains code to interact with the Hyper-V watchdog timer.

Author:

    Kris Harper (kharp) 8-Oct-2013

Environment:

    UEFI

--*/
#pragma once


typedef enum
{
    //
    // Watchdog is disabled.  This should only be used with a count
    // value of zero.
    //
    WatchdogDisabled,
    //
    //  The count represents the amount of time before the timer will
    //  expired.
    //
    WatchdogOneShot,
    //
    // The hardware timer will run periodically and decrement the count.
    // The timer is expired when the count reaches zero.
    // When used in periodic mode there is normally a periodic entity in
    // UEFI that will reset the count to its original value.
    //
    WatchdogPeriodic
}WATCHDOG_MODE;


VOID
WatchdogConfigure(
    _In_    UINT32                  Count,
    _In_    WATCHDOG_MODE           Mode
    );


VOID
WatchdogSetCount(
    _In_    UINT32                  Count
    );


UINT32
WatchdogGetResolution();


BOOLEAN
WatchdogSuspend();


VOID
WatchdogResume(
    _In_    BOOLEAN                 PreviouslyRunning
    );