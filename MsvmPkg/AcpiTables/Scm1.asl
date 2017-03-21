/*++

Copyright (c) Microsoft Corporation

Module Name:

    Scm1.asl

Abstract:

    A table (SCM1) that contains the declarations of the NVDIMM root device
    and any NVDIMM child devices.

--*/

DefinitionBlock (
    "scm1.aml",
    "OEM3",
    0x02,           // DSDT Compliance revision.
    "MSFTVM",       // OEM ID (6 byte string)
    "VSCM",         // OEM table ID  (8 byte string)
    0x01            // OEM version of DSDT table (4 byte Integer)
    )
{
    Scope(_SB)
    {
        //
        // NVDIMM Root Device
        //

        Device(NVDR)
        {
            Name (_HID, "ACPI0012")
            Name (_STA, 0xF) 

            //
            // TODO-adburch: implement _DSM methods for NVDIMM root and children
            //

            //
            // NVDIMM Child Devices
            //

            Device(N000)
            {
                Name (_ADR, 0)
            }

            Device(N001)
            {
                Name (_ADR, 1)
            }

            Device(N002)
            {
                Name (_ADR, 2)
            }

            Device(N003)
            {
                Name (_ADR, 3)
            }

            Device(N004)
            {
                Name (_ADR, 4)
            }

            Device(N005)
            {
                Name (_ADR, 5)
            }

            Device(N006)
            {
                Name (_ADR, 6)
            }

            Device(N007)
            {
                Name (_ADR, 7)
            }

            Device(N008)
            {
                Name (_ADR, 8)
            }

            Device(N009)
            {
                Name (_ADR, 9)
            }

            Device(N010)
            {
                Name (_ADR, 10)
            }

            Device(N011)
            {
                Name (_ADR, 11)
            }

            Device(N012)
            {
                Name (_ADR, 12)
            }

            Device(N013)
            {
                Name (_ADR, 13)
            }

            Device(N014)
            {
                Name (_ADR, 14)
            }

            Device(N015)
            {
                Name (_ADR, 15)
            }

            Device(N016)
            {
                Name (_ADR, 16)
            }

            Device(N017)
            {
                Name (_ADR, 17)
            }

            Device(N018)
            {
                Name (_ADR, 18)
            }

            Device(N019)
            {
                Name (_ADR, 19)
            }

            Device(N020)
            {
                Name (_ADR, 20)
            }

            Device(N021)
            {
                Name (_ADR, 21)
            }

            Device(N022)
            {
                Name (_ADR, 22)
            }

            Device(N023)
            {
                Name (_ADR, 23)
            }

            Device(N024)
            {
                Name (_ADR, 24)
            }

            Device(N025)
            {
                Name (_ADR, 25)
            }

            Device(N026)
            {
                Name (_ADR, 26)
            }

            Device(N027)
            {
                Name (_ADR, 27)
            }

            Device(N028)
            {
                Name (_ADR, 28)
            }

            Device(N029)
            {
                Name (_ADR, 29)
            }

            Device(N030)
            {
                Name (_ADR, 30)
            }

            Device(N031)
            {
                Name (_ADR, 31)
            }

            Device(N032)
            {
                Name (_ADR, 32)
            }

            Device(N033)
            {
                Name (_ADR, 33)
            }

            Device(N034)
            {
                Name (_ADR, 34)
            }

            Device(N035)
            {
                Name (_ADR, 35)
            }

            Device(N036)
            {
                Name (_ADR, 36)
            }

            Device(N037)
            {
                Name (_ADR, 37)
            }

            Device(N038)
            {
                Name (_ADR, 38)
            }

            Device(N039)
            {
                Name (_ADR, 39)
            }

            Device(N040)
            {
                Name (_ADR, 40)
            }

            Device(N041)
            {
                Name (_ADR, 41)
            }

            Device(N042)
            {
                Name (_ADR, 42)
            }

            Device(N043)
            {
                Name (_ADR, 43)
            }

            Device(N044)
            {
                Name (_ADR, 44)
            }

            Device(N045)
            {
                Name (_ADR, 45)
            }

            Device(N046)
            {
                Name (_ADR, 46)
            }

            Device(N047)
            {
                Name (_ADR, 47)
            }

            Device(N048)
            {
                Name (_ADR, 48)
            }

            Device(N049)
            {
                Name (_ADR, 49)
            }

            Device(N050)
            {
                Name (_ADR, 50)
            }

            Device(N051)
            {
                Name (_ADR, 51)
            }

            Device(N052)
            {
                Name (_ADR, 52)
            }

            Device(N053)
            {
                Name (_ADR, 53)
            }

            Device(N054)
            {
                Name (_ADR, 54)
            }

            Device(N055)
            {
                Name (_ADR, 55)
            }

            Device(N056)
            {
                Name (_ADR, 56)
            }

            Device(N057)
            {
                Name (_ADR, 57)
            }

            Device(N058)
            {
                Name (_ADR, 58)
            }

            Device(N059)
            {
                Name (_ADR, 59)
            }

            Device(N060)
            {
                Name (_ADR, 60)
            }

            Device(N061)
            {
                Name (_ADR, 61)
            }

            Device(N062)
            {
                Name (_ADR, 62)
            }

            Device(N063)
            {
                Name (_ADR, 63)
            }

            Device(N064)
            {
                Name (_ADR, 64)
            }

            Device(N065)
            {
                Name (_ADR, 65)
            }

            Device(N066)
            {
                Name (_ADR, 66)
            }

            Device(N067)
            {
                Name (_ADR, 67)
            }

            Device(N068)
            {
                Name (_ADR, 68)
            }

            Device(N069)
            {
                Name (_ADR, 69)
            }

            Device(N070)
            {
                Name (_ADR, 70)
            }

            Device(N071)
            {
                Name (_ADR, 71)
            }

            Device(N072)
            {
                Name (_ADR, 72)
            }

            Device(N073)
            {
                Name (_ADR, 73)
            }

            Device(N074)
            {
                Name (_ADR, 74)
            }

            Device(N075)
            {
                Name (_ADR, 75)
            }

            Device(N076)
            {
                Name (_ADR, 76)
            }

            Device(N077)
            {
                Name (_ADR, 77)
            }

            Device(N078)
            {
                Name (_ADR, 78)
            }

            Device(N079)
            {
                Name (_ADR, 79)
            }

            Device(N080)
            {
                Name (_ADR, 80)
            }

            Device(N081)
            {
                Name (_ADR, 81)
            }

            Device(N082)
            {
                Name (_ADR, 82)
            }

            Device(N083)
            {
                Name (_ADR, 83)
            }

            Device(N084)
            {
                Name (_ADR, 84)
            }

            Device(N085)
            {
                Name (_ADR, 85)
            }

            Device(N086)
            {
                Name (_ADR, 86)
            }

            Device(N087)
            {
                Name (_ADR, 87)
            }

            Device(N088)
            {
                Name (_ADR, 88)
            }

            Device(N089)
            {
                Name (_ADR, 89)
            }

            Device(N090)
            {
                Name (_ADR, 90)
            }

            Device(N091)
            {
                Name (_ADR, 91)
            }

            Device(N092)
            {
                Name (_ADR, 92)
            }

            Device(N093)
            {
                Name (_ADR, 93)
            }

            Device(N094)
            {
                Name (_ADR, 94)
            }

            Device(N095)
            {
                Name (_ADR, 95)
            }

            Device(N096)
            {
                Name (_ADR, 96)
            }

            Device(N097)
            {
                Name (_ADR, 97)
            }

            Device(N098)
            {
                Name (_ADR, 98)
            }

            Device(N099)
            {
                Name (_ADR, 99)
            }

            Device(N100)
            {
                Name (_ADR, 100)
            }

            Device(N101)
            {
                Name (_ADR, 101)
            }

            Device(N102)
            {
                Name (_ADR, 102)
            }

            Device(N103)
            {
                Name (_ADR, 103)
            }

            Device(N104)
            {
                Name (_ADR, 104)
            }

            Device(N105)
            {
                Name (_ADR, 105)
            }

            Device(N106)
            {
                Name (_ADR, 106)
            }

            Device(N107)
            {
                Name (_ADR, 107)
            }

            Device(N108)
            {
                Name (_ADR, 108)
            }

            Device(N109)
            {
                Name (_ADR, 109)
            }

            Device(N110)
            {
                Name (_ADR, 110)
            }

            Device(N111)
            {
                Name (_ADR, 111)
            }

            Device(N112)
            {
                Name (_ADR, 112)
            }

            Device(N113)
            {
                Name (_ADR, 113)
            }

            Device(N114)
            {
                Name (_ADR, 114)
            }

            Device(N115)
            {
                Name (_ADR, 115)
            }

            Device(N116)
            {
                Name (_ADR, 116)
            }

            Device(N117)
            {
                Name (_ADR, 117)
            }

            Device(N118)
            {
                Name (_ADR, 118)
            }

            Device(N119)
            {
                Name (_ADR, 119)
            }

            Device(N120)
            {
                Name (_ADR, 120)
            }

            Device(N121)
            {
                Name (_ADR, 121)
            }

            Device(N122)
            {
                Name (_ADR, 122)
            }

            Device(N123)
            {
                Name (_ADR, 123)
            }

            Device(N124)
            {
                Name (_ADR, 124)
            }

            Device(N125)
            {
                Name (_ADR, 125)
            }

            Device(N126)
            {
                Name (_ADR, 126)
            }

            Device(N127)
            {
                Name (_ADR, 127)
            }
        }
    }
}
