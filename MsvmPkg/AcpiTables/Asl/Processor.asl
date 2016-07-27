/*++

Copyright (c) Microsoft Corporation

Module Name:

    Processor.asl

Abstract:

    An ASL file defining processors.

--*/

Scope(\_SB)
{
    //
    // Return processor status for processor n, 1-indexed.
    //

    Method(PSTA, 1, Serialized)
    {
        If(LLessEqual(Arg0, PCNT))
        {
            Return(0xF)
        }

        Return(0)
    }

    //
    // Processor #1 (Boot processor) is special because it's ALWAYS enabled
    //

    Processor(P001,  1, 0, 0)
    {
        Method(_STA, 0, Serialized)
        {
            Return(0xF)
        }
    }

    //
    // The remaining processors.
    //

    Processor(P002, 2, 0, 0)
    {
        Name(PNUM, 2)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P003, 3, 0, 0)
    {
        Name(PNUM, 3)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P004,  4, 0, 0)
    {
        Name(PNUM, 4)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P005,  5, 0, 0)
    {
        Name(PNUM, 5)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P006,  6, 0, 0)
    {
        Name(PNUM, 6)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P007,  7, 0, 0)
    {
        Name(PNUM, 7)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P008,  8, 0, 0)
    {
        Name(PNUM, 8)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P009,  9, 0, 0)
    {
        Name(PNUM, 9)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P010,  10, 0, 0)
    {
        Name(PNUM, 10)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P011,  11, 0, 0)
    {
        Name(PNUM, 11)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P012,  12, 0, 0)
    {
        Name(PNUM, 12)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P013,  13, 0, 0)
    {
        Name(PNUM, 13)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P014,  14, 0, 0)
    {
        Name(PNUM, 14)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P015,  15, 0, 0)
    {
        Name(PNUM, 15)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P016,  16, 0, 0)
    {
        Name(PNUM, 16)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P017,  17, 0, 0)
    {
        Name(PNUM, 17)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P018,  18, 0, 0)
    {
        Name(PNUM, 18)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P019,  19, 0, 0)
    {
        Name(PNUM, 19)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P020,  20, 0, 0)
    {
        Name(PNUM, 20)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P021,  21, 0, 0)
    {
        Name(PNUM, 21)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P022, 22, 0, 0)
    {
        Name(PNUM, 22)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P023,  23, 0, 0)
    {
        Name(PNUM, 23)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P024,  24, 0, 0)
    {
        Name(PNUM, 24)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P025,  25, 0, 0)
    {
        Name(PNUM, 25)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P026,  26, 0, 0)
    {
        Name(PNUM, 26)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P027,  27, 0, 0)
    {
        Name(PNUM, 27)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P028,  28, 0, 0)
    {
        Name(PNUM, 28)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P029,  29, 0, 0)
    {
        Name(PNUM, 29)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P030,  30, 0, 0)
    {
        Name(PNUM, 30)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P031,  31, 0, 0)
    {
        Name(PNUM, 31)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P032, 32, 0, 0)
    {
        Name(PNUM, 32)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }


    Processor(P033, 33, 0, 0)
    {
        Name(PNUM, 33)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P034, 34, 0, 0)
    {
        Name(PNUM, 34)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P035, 35, 0, 0)
    {
        Name(PNUM, 35)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P036, 36, 0, 0)
    {
        Name(PNUM, 36)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P037, 37, 0, 0)
    {
        Name(PNUM, 37)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P038, 38, 0, 0)
    {
        Name(PNUM, 38)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P039, 39, 0, 0)
    {
        Name(PNUM, 39)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P040, 40, 0, 0)
    {
        Name(PNUM, 40)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P041, 41, 0, 0)
    {
        Name(PNUM, 41)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P042, 42, 0, 0)
    {
        Name(PNUM, 42)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P043, 43, 0, 0)
    {
        Name(PNUM, 43)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P044, 44, 0, 0)
    {
        Name(PNUM, 44)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P045, 45, 0, 0)
    {
        Name(PNUM, 45)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P046, 46, 0, 0)
    {
        Name(PNUM, 46)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P047, 47, 0, 0)
    {
        Name(PNUM, 47)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P048, 48, 0, 0)
    {
        Name(PNUM, 48)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P049, 49, 0, 0)
    {
        Name(PNUM, 49)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P050, 50, 0, 0)
    {
        Name(PNUM, 50)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P051, 51, 0, 0)
    {
        Name(PNUM, 51)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P052, 52, 0, 0)
    {
        Name(PNUM, 52)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P053, 53, 0, 0)
    {
        Name(PNUM, 53)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P054, 54, 0, 0)
    {
        Name(PNUM, 54)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P055, 55, 0, 0)
    {
        Name(PNUM, 55)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P056, 56, 0, 0)
    {
        Name(PNUM, 56)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P057, 57, 0, 0)
    {
        Name(PNUM, 57)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P058, 58, 0, 0)
    {
        Name(PNUM, 58)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P059, 59, 0, 0)
    {
        Name(PNUM, 59)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P060, 60, 0, 0)
    {
        Name(PNUM, 60)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P061, 61, 0, 0)
    {
        Name(PNUM, 61)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P062, 62, 0, 0)
    {
        Name(PNUM, 62)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P063, 63, 0, 0)
    {
        Name(PNUM, 63)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P064, 64, 0, 0)
    {
        Name(PNUM, 64)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }
}

