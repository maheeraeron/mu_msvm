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

    Processor(P004, 4, 0, 0)
    {
        Name(PNUM, 4)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P005, 5, 0, 0)
    {
        Name(PNUM, 5)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P006, 6, 0, 0)
    {
        Name(PNUM, 6)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P007, 7, 0, 0)
    {
        Name(PNUM, 7)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P008, 8, 0, 0)
    {
        Name(PNUM, 8)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P009, 9, 0, 0)
    {
        Name(PNUM, 9)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P010, 10, 0, 0)
    {
        Name(PNUM, 10)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P011, 11, 0, 0)
    {
        Name(PNUM, 11)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P012, 12, 0, 0)
    {
        Name(PNUM, 12)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P013, 13, 0, 0)
    {
        Name(PNUM, 13)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P014, 14, 0, 0)
    {
        Name(PNUM, 14)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P015, 15, 0, 0)
    {
        Name(PNUM, 15)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P016, 16, 0, 0)
    {
        Name(PNUM, 16)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P017, 17, 0, 0)
    {
        Name(PNUM, 17)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P018, 18, 0, 0)
    {
        Name(PNUM, 18)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P019, 19, 0, 0)
    {
        Name(PNUM, 19)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P020, 20, 0, 0)
    {
        Name(PNUM, 20)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P021, 21, 0, 0)
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

    Processor(P023, 23, 0, 0)
    {
        Name(PNUM, 23)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P024, 24, 0, 0)
    {
        Name(PNUM, 24)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P025, 25, 0, 0)
    {
        Name(PNUM, 25)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P026, 26, 0, 0)
    {
        Name(PNUM, 26)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P027, 27, 0, 0)
    {
        Name(PNUM, 27)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P028, 28, 0, 0)
    {
        Name(PNUM, 28)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P029, 29, 0, 0)
    {
        Name(PNUM, 29)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P030, 30, 0, 0)
    {
        Name(PNUM, 30)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P031, 31, 0, 0)
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

    Processor(P065, 65, 0, 0)
    {
        Name(PNUM, 65)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P066, 66, 0, 0)
    {
        Name(PNUM, 66)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P067, 67, 0, 0)
    {
        Name(PNUM, 67)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P068, 68, 0, 0)
    {
        Name(PNUM, 68)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P069, 69, 0, 0)
    {
        Name(PNUM, 69)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P070, 70, 0, 0)
    {
        Name(PNUM, 70)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P071, 71, 0, 0)
    {
        Name(PNUM, 71)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P072, 72, 0, 0)
    {
        Name(PNUM, 72)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P073, 73, 0, 0)
    {
        Name(PNUM, 73)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P074, 74, 0, 0)
    {
        Name(PNUM, 74)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P075, 75, 0, 0)
    {
        Name(PNUM, 75)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P076, 76, 0, 0)
    {
        Name(PNUM, 76)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P077, 77, 0, 0)
    {
        Name(PNUM, 77)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P078, 78, 0, 0)
    {
        Name(PNUM, 78)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P079, 79, 0, 0)
    {
        Name(PNUM, 79)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P080, 80, 0, 0)
    {
        Name(PNUM, 80)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P081, 81, 0, 0)
    {
        Name(PNUM, 81)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P082, 82, 0, 0)
    {
        Name(PNUM, 82)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P083, 83, 0, 0)
    {
        Name(PNUM, 83)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P084, 84, 0, 0)
    {
        Name(PNUM, 84)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P085, 85, 0, 0)
    {
        Name(PNUM, 85)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P086, 86, 0, 0)
    {
        Name(PNUM, 86)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P087, 87, 0, 0)
    {
        Name(PNUM, 87)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P088, 88, 0, 0)
    {
        Name(PNUM, 88)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P089, 89, 0, 0)
    {
        Name(PNUM, 89)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P090, 90, 0, 0)
    {
        Name(PNUM, 90)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P091, 91, 0, 0)
    {
        Name(PNUM, 91)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P092, 92, 0, 0)
    {
        Name(PNUM, 92)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P093, 93, 0, 0)
    {
        Name(PNUM, 93)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P094, 94, 0, 0)
    {
        Name(PNUM, 94)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P095, 95, 0, 0)
    {
        Name(PNUM, 95)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P096, 96, 0, 0)
    {
        Name(PNUM, 96)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P097, 97, 0, 0)
    {
        Name(PNUM, 97)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P098, 98, 0, 0)
    {
        Name(PNUM, 98)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P099, 99, 0, 0)
    {
        Name(PNUM, 99)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P100, 100, 0, 0)
    {
        Name(PNUM, 100)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P101, 101, 0, 0)
    {
        Name(PNUM, 101)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P102, 102, 0, 0)
    {
        Name(PNUM, 102)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P103, 103, 0, 0)
    {
        Name(PNUM, 103)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P104, 104, 0, 0)
    {
        Name(PNUM, 104)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P105, 105, 0, 0)
    {
        Name(PNUM, 105)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P106, 106, 0, 0)
    {
        Name(PNUM, 106)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P107, 107, 0, 0)
    {
        Name(PNUM, 107)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P108, 108, 0, 0)
    {
        Name(PNUM, 108)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P109, 109, 0, 0)
    {
        Name(PNUM, 109)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P110, 110, 0, 0)
    {
        Name(PNUM, 110)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P111, 111, 0, 0)
    {
        Name(PNUM, 111)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P112, 112, 0, 0)
    {
        Name(PNUM, 112)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P113, 113, 0, 0)
    {
        Name(PNUM, 113)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P114, 114, 0, 0)
    {
        Name(PNUM, 114)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P115, 115, 0, 0)
    {
        Name(PNUM, 115)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P116, 116, 0, 0)
    {
        Name(PNUM, 116)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P117, 117, 0, 0)
    {
        Name(PNUM, 117)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P118, 118, 0, 0)
    {
        Name(PNUM, 118)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P119, 119, 0, 0)
    {
        Name(PNUM, 119)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P120, 120, 0, 0)
    {
        Name(PNUM, 120)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P121, 121, 0, 0)
    {
        Name(PNUM, 121)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P122, 122, 0, 0)
    {
        Name(PNUM, 122)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P123, 123, 0, 0)
    {
        Name(PNUM, 123)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P124, 124, 0, 0)
    {
        Name(PNUM, 124)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P125, 125, 0, 0)
    {
        Name(PNUM, 125)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P126, 126, 0, 0)
    {
        Name(PNUM, 126)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P127, 127, 0, 0)
    {
        Name(PNUM, 127)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P128, 128, 0, 0)
    {
        Name(PNUM, 128)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P129, 129, 0, 0)
    {
        Name(PNUM, 129)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P130, 130, 0, 0)
    {
        Name(PNUM, 130)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P131, 131, 0, 0)
    {
        Name(PNUM, 131)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P132, 132, 0, 0)
    {
        Name(PNUM, 132)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P133, 133, 0, 0)
    {
        Name(PNUM, 133)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P134, 134, 0, 0)
    {
        Name(PNUM, 134)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P135, 135, 0, 0)
    {
        Name(PNUM, 135)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P136, 136, 0, 0)
    {
        Name(PNUM, 136)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P137, 137, 0, 0)
    {
        Name(PNUM, 137)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P138, 138, 0, 0)
    {
        Name(PNUM, 138)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P139, 139, 0, 0)
    {
        Name(PNUM, 139)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P140, 140, 0, 0)
    {
        Name(PNUM, 140)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P141, 141, 0, 0)
    {
        Name(PNUM, 141)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P142, 142, 0, 0)
    {
        Name(PNUM, 142)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P143, 143, 0, 0)
    {
        Name(PNUM, 143)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P144, 144, 0, 0)
    {
        Name(PNUM, 144)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P145, 145, 0, 0)
    {
        Name(PNUM, 145)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P146, 146, 0, 0)
    {
        Name(PNUM, 146)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P147, 147, 0, 0)
    {
        Name(PNUM, 147)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P148, 148, 0, 0)
    {
        Name(PNUM, 148)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P149, 149, 0, 0)
    {
        Name(PNUM, 149)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P150, 150, 0, 0)
    {
        Name(PNUM, 150)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P151, 151, 0, 0)
    {
        Name(PNUM, 151)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P152, 152, 0, 0)
    {
        Name(PNUM, 152)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P153, 153, 0, 0)
    {
        Name(PNUM, 153)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P154, 154, 0, 0)
    {
        Name(PNUM, 154)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P155, 155, 0, 0)
    {
        Name(PNUM, 155)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P156, 156, 0, 0)
    {
        Name(PNUM, 156)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P157, 157, 0, 0)
    {
        Name(PNUM, 157)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P158, 158, 0, 0)
    {
        Name(PNUM, 158)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P159, 159, 0, 0)
    {
        Name(PNUM, 159)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P160, 160, 0, 0)
    {
        Name(PNUM, 160)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P161, 161, 0, 0)
    {
        Name(PNUM, 161)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P162, 162, 0, 0)
    {
        Name(PNUM, 162)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P163, 163, 0, 0)
    {
        Name(PNUM, 163)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P164, 164, 0, 0)
    {
        Name(PNUM, 164)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P165, 165, 0, 0)
    {
        Name(PNUM, 165)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P166, 166, 0, 0)
    {
        Name(PNUM, 166)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P167, 167, 0, 0)
    {
        Name(PNUM, 167)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P168, 168, 0, 0)
    {
        Name(PNUM, 168)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P169, 169, 0, 0)
    {
        Name(PNUM, 169)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P170, 170, 0, 0)
    {
        Name(PNUM, 170)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P171, 171, 0, 0)
    {
        Name(PNUM, 171)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P172, 172, 0, 0)
    {
        Name(PNUM, 172)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P173, 173, 0, 0)
    {
        Name(PNUM, 173)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P174, 174, 0, 0)
    {
        Name(PNUM, 174)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P175, 175, 0, 0)
    {
        Name(PNUM, 175)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P176, 176, 0, 0)
    {
        Name(PNUM, 176)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P177, 177, 0, 0)
    {
        Name(PNUM, 177)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P178, 178, 0, 0)
    {
        Name(PNUM, 178)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P179, 179, 0, 0)
    {
        Name(PNUM, 179)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P180, 180, 0, 0)
    {
        Name(PNUM, 180)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P181, 181, 0, 0)
    {
        Name(PNUM, 181)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P182, 182, 0, 0)
    {
        Name(PNUM, 182)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P183, 183, 0, 0)
    {
        Name(PNUM, 183)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P184, 184, 0, 0)
    {
        Name(PNUM, 184)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P185, 185, 0, 0)
    {
        Name(PNUM, 185)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P186, 186, 0, 0)
    {
        Name(PNUM, 186)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P187, 187, 0, 0)
    {
        Name(PNUM, 187)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P188, 188, 0, 0)
    {
        Name(PNUM, 188)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P189, 189, 0, 0)
    {
        Name(PNUM, 189)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P190, 190, 0, 0)
    {
        Name(PNUM, 190)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P191, 191, 0, 0)
    {
        Name(PNUM, 191)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P192, 192, 0, 0)
    {
        Name(PNUM, 192)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P193, 193, 0, 0)
    {
        Name(PNUM, 193)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P194, 194, 0, 0)
    {
        Name(PNUM, 194)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P195, 195, 0, 0)
    {
        Name(PNUM, 195)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P196, 196, 0, 0)
    {
        Name(PNUM, 196)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P197, 197, 0, 0)
    {
        Name(PNUM, 197)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P198, 198, 0, 0)
    {
        Name(PNUM, 198)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P199, 199, 0, 0)
    {
        Name(PNUM, 199)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P200, 200, 0, 0)
    {
        Name(PNUM, 200)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P201, 201, 0, 0)
    {
        Name(PNUM, 201)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P202, 202, 0, 0)
    {
        Name(PNUM, 202)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P203, 203, 0, 0)
    {
        Name(PNUM, 203)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P204, 204, 0, 0)
    {
        Name(PNUM, 204)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P205, 205, 0, 0)
    {
        Name(PNUM, 205)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P206, 206, 0, 0)
    {
        Name(PNUM, 206)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P207, 207, 0, 0)
    {
        Name(PNUM, 207)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P208, 208, 0, 0)
    {
        Name(PNUM, 208)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P209, 209, 0, 0)
    {
        Name(PNUM, 209)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P210, 210, 0, 0)
    {
        Name(PNUM, 210)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P211, 211, 0, 0)
    {
        Name(PNUM, 211)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P212, 212, 0, 0)
    {
        Name(PNUM, 212)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P213, 213, 0, 0)
    {
        Name(PNUM, 213)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P214, 214, 0, 0)
    {
        Name(PNUM, 214)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P215, 215, 0, 0)
    {
        Name(PNUM, 215)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P216, 216, 0, 0)
    {
        Name(PNUM, 216)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P217, 217, 0, 0)
    {
        Name(PNUM, 217)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P218, 218, 0, 0)
    {
        Name(PNUM, 218)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P219, 219, 0, 0)
    {
        Name(PNUM, 219)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P220, 220, 0, 0)
    {
        Name(PNUM, 220)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P221, 221, 0, 0)
    {
        Name(PNUM, 221)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P222, 222, 0, 0)
    {
        Name(PNUM, 222)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P223, 223, 0, 0)
    {
        Name(PNUM, 223)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P224, 224, 0, 0)
    {
        Name(PNUM, 224)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P225, 225, 0, 0)
    {
        Name(PNUM, 225)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P226, 226, 0, 0)
    {
        Name(PNUM, 226)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P227, 227, 0, 0)
    {
        Name(PNUM, 227)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P228, 228, 0, 0)
    {
        Name(PNUM, 228)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P229, 229, 0, 0)
    {
        Name(PNUM, 229)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P230, 230, 0, 0)
    {
        Name(PNUM, 230)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P231, 231, 0, 0)
    {
        Name(PNUM, 231)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P232, 232, 0, 0)
    {
        Name(PNUM, 232)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P233, 233, 0, 0)
    {
        Name(PNUM, 233)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P234, 234, 0, 0)
    {
        Name(PNUM, 234)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P235, 235, 0, 0)
    {
        Name(PNUM, 235)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P236, 236, 0, 0)
    {
        Name(PNUM, 236)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P237, 237, 0, 0)
    {
        Name(PNUM, 237)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P238, 238, 0, 0)
    {
        Name(PNUM, 238)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P239, 239, 0, 0)
    {
        Name(PNUM, 239)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

    Processor(P240, 240, 0, 0)
    {
        Name(PNUM, 240)

        Method(_STA, 0)
        {
            Return(PSTA(PNUM))
        }
    }

}

