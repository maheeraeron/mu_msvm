/*++

Copyright (c) Microsoft Corporation
Copyright (c) 1985-2001, American Megatrends, Inc.

Module Name:

    IDE.asl

Abstract:

    An ASL file defining methods for the IDE controller PCI device. These
    methods configure things like DMA/PIO support and DMA timings, as well
    as disk initialization methods.

--*/

Device(_SB.PCI0.IDE0)
{
    //
    // Device 7, feature 1
    //

    Name(_ADR, 0x00070001)

    //-----------------------------------------------------------------------
    // _GTM, _STM, _GTF control methods for both IDE channels
    // Note. Add custom PM control code for PS0, PS3 in that file
    //-----------------------------------------------------------------------

    Name(REGF, 1)           // PCI Bus access Flag

    Method(_REG, 2)        // is PCI Config space accessible as OpRegion?
                    // _REG to update REGF status
    {
        If(LEqual(Arg0, 0x2))
        {
            Store(Arg1, REGF)
        }
    }

    //
    // Timings package for Primary / Secondary channels
    //

    Name(TIM0, Package()
    {
        // 0.PIO timings   4,   3,   2,   0(compatible)
        Package(){120, 180, 240, 900},    // Timings in ns
        // 1. Drive 0
        Package(){0x23,0x21,0x10, 0 },    // Primary / Secondary Master
        // 2. Drive 1
        Package(){0x0b,0x09,0x04, 0 },    // Primary / Secondary Slave
        // 3. UDMA    33->  0   1   2  -> Reserved for UDma66 mode
        Package(){112, 73, 54, 54},     // Min UDMA Timings in ns
        //Package(){120, 90, 60, 0},      // Max DMA Timings in ns
        // 4. PIO mode
        Package(){4, 3, 2, 0},          // PIO mode (TIM0,0)
        // 5. Multi-word DMA mode
        Package(){2, 1, 0, 0},
    })

    // Buffer to be returned by _GTM
    Name(TMD0, Buffer(20){})            // 5 DWORD length
    CreateDWordField(TMD0, 00, PIO0)
    CreateDWordField(TMD0, 04, DMA0)
    CreateDWordField(TMD0, 8,  PIO1)        // do not use "08"
    CreateDWordField(TMD0, 12, DMA1)
    CreateDWordField(TMD0, 16, CHNF)

    OperationRegion(CFG2, PCI_Config, 0x40, 0x10)
    Field(CFG2,DWordAcc,NoLock,Preserve)
    {
        //      0x40
        // Primary channel
        TIMP,16,    // IDE Timings, Primary channel
        //      0x42
        // Secondary channel
        TIMS,16,    // IDE Timings, Secondary
        //      0x44
        // Slave IDE timing
        STMP,4,        // Slave(Drive 1) IDE Timing - Primary channel
        STMS,4,        //                          Secondary
        //      0x48
        // Ultra DMA/33 control
        Offset(0x08),
        UDMP,2,            // Drive 0/1 UDMA mode on Primary
        UDMS,2,            // Drive 0/1 UDMA mode on Secondary
        Offset(0x0a),
        //      0x4A
        // Ultra DMA/33 timing
        UDTP,6,        // Drive 0/1 UDMA timings for Primary
        ,2,
        UDTS,6,        // Drive 0/1 UDMA timings for Secondary
    }

    Name(GTIM, 0)        // PIO/DMA Timings
    Name(GSTM, 0)        // Slave Timings
    Name(GUDM, 0)        // UDMA enable
    Name(GUDT, 0)        // UDMA Timings


////////////////////////////////////////////////////////////////////////////////
// Primary channel of IDE controller
////////////////////////////////////////////////////////////////////////////////

    Device(CHN0)
    {
        Name(_ADR, 0)

        //
        // Get Timing PIO/DMA Mode
        //

        Method(_GTM, 0)
        {
            Return(GTM(TIMP,STMP,UDMP,UDTP))
        }

        //
        // Set Timing PIO/DMA Mode
        // Arg 0 = Channel Timing Info (Package)
        // Arg 1 = ATA Command set Master(Buffer)
        // Arg 2 = ATA Command set Slave (Buffer)
        //

        Method(_STM, 3)
        {
            Store(Arg0, Debug)

            // Copy Arg0 into TMD0 buffer
            Store(Arg0, TMD0)

            // I/O timing register
            Store(TIMP, GTIM)

            // UDMA timings
            Store(UDTP, GUDT)

            // Update IDE registers if STM = 1
            If(STM())
            {
                Store(GTIM, TIMP)
                Store(GSTM, STMP)
                Store(GUDM, UDMP)
                Store(GUDT, UDTP)
            }

            // Update _GTF buffers for Primary Channel
            Store(GTF(0, Arg1), ATA0)    // Master
            Store(GTF(1, Arg2), ATA1)    // Slave
        }

        // Primary Master
        Device(DRV0)
        {
            Name(_ADR, 0)
            Method(_GTF, 0)
            {
                    Return(RATA(ATA0))
            }
        }

        // Primary Slave
        Device(DRV1)
        {
            Name(_ADR, 1)
            Method(_GTF, 0)
            {
                Return(RATA(ATA1))
            }
        }
    }

    Device(CHN1)
    {
        Name(_ADR, 1)

        //
        // Get Timing PIO/DMA Mode
        //

        Method(_GTM, 0)
        {
            Return(GTM(TIMS,STMS,UDMS,UDTS))
        }

        //
        // Set Timing PIO/DMA Mode
        // Arg 0 = Channel Timing Info (Package)
        // Arg 1 = ATA Command set Master(Buffer)
        // Arg 2 = ATA Command set Slave (Buffer)
        //

        Method(_STM, 3)
        {
            Store(Arg0, Debug)

            // Copy Arg0 into TMD0 buffer
            Store(Arg0, TMD0)

            // I/O timing register
            Store(TIMS, GTIM)

            // UDMA timings
            Store(UDTS, GUDT)

            // Update IDE registers if STM = 1
            If(STM())
            {
                Store(GTIM, TIMS)
                Store(GSTM, STMS)
                Store(GUDM, UDMS)
                Store(GUDT, UDTS)
            }

            // Update _GTF buffers for Primary Channel
            Store(GTF(0, Arg1), ATA2)    // Master
            Store(GTF(1, Arg2), ATA3)    // Slave
        }

        //
        // Secondary Master
        //

        Device(DRV0)
        {
            Name(_ADR, 0)
            Method(_GTF, 0)
            {
                Return(RATA(ATA2))
            }
        }

        //
        // Secondary Slave
        //

        Device(DRV1)
        {
            Name(_ADR, 1)
            Method(_GTF, 0)
            {
                Return(RATA(ATA3))
            }
        }
    }

    //
    // Get Timing PIO/DMA Mode
    //
    //--------------------------------------------
    // INPUT
    // Arg0(Word) - I/O Timing register
    //    0             // Drive 0 Fast Timing Bank
    //    1,        // Drive 0 IORDY sampling mode
    //    2,             // Drive 0 Prefetch/Posting enable
    //    3,        // Drive 0 DMA timing mode
    //    4,             // Drive 1 Fast Timing Bank
    //    5,             // Drive 1 IORDY sampling mode
    //    6,             // Drive 1 Prefetch/Posting enable
    //    7,        // Drive 1 DMA timing mode
    //    8-13,        // Drive 0 IDE Timing Register
    //    14,        // Slave IDE timing enable
    //    15,            // IDE Channel Decode Enable
    // Arg1(Byte) - Slave I/O Timing register
    //    0-3
    // Arg2(Byte) - UDMA Control register
    //    0,             // Primary, Drive 0 SDMA mode
    //    1,        // Primary, Drive 0 SDMA mode
    // Arg3(Byte) - UDMA Timing register
    //    0-1        // UDMA timings for Drive 0
    //    2-3            // Reserved
    //    4-5        // UDMA timings for Drive 1
    //--------------------------------------------
    // OUTPUT
    // DWord Buffer
    //     0 PIO 0 Speed DWORD
    //     1 DMA 0 Speed DWORD
    //     2 PIO 1 Speed DWORD
    //     3 DMA 1 Speed DWORD
    //     4 Flags DWORD
    //

    Method(GTM, 4, Serialized)
    {
        Store(Ones, PIO0)    // Default: no PIO0 timing
        Store(PIO0, PIO1)       // Default: no PIO1 timing
        Store(PIO0, DMA0)       // Default: no DMA0 timing
        Store(PIO0, DMA1)       // Default: no DMA1 timing
        Store(Zero, CHNF)    // Preset Flags

        If(REGF){}            // PCI space not accessible
        Else    { Return(TMD0) }

        // Drive 0
        If(And(Arg0, 0x2))            // Is IOChannelReady0 is used ?
        {
            Or(CHNF, 0x2, CHNF)        // Set flag IORDY0
        }

        // Read Current value of reg 40-41 or 42-43
        ShiftRight(And(Arg0, 0x3300), 0x8, Local4)
        Store(Match(DeRefOf(Index(TIM0, 1)), MLE, Local4, MTR,0,0), Local5)
        // if settings in IDETIM do not match the table values, 900ns will be returned
        Store(DeRefOf(Index(DeReFof(Index(TIM0, 0)), Local5)), Local6)
        // Preset DMA settings in case of multi-word DMA
        Store(Local6, DMA0)
        // If DMA Timing Only bit set - Drive supports PIO 0 mode only
        If(And(Arg0, 0x8)){
            Store(900,  PIO0)
        }
        Else{
            Store(Local6, PIO0)
        }

        // Drive 1 (Slave)
        If(And(Arg0, 0x20))            // Is IOChannelReady1 is used ?
        {
            Or(CHNF, 0x8, CHNF)        // Set flag IORDY1
        }

        // Slave timings can be set independently for Master & Slave
        If(And(Arg0, 0x4000))
        {
            Or(CHNF, 0x10, CHNF)
            Store(Match(DeRefOf(Index(TIM0, 2)), MLE, Arg1, MTR,0,0), Local4)
            // if settings in IDETIM do not match, 900ns will be returned
            Store(DeRefOf(Index(DeReFof(Index(TIM0, 0)), Local4)), Local5)
            // Preset DMA settings in case of multi-word DMA
            Store(Local5, DMA1)
            // If DMA Timing Only bit set - Drive supports PIO 0 mode only
            If(And(Arg0, 0x80)){
                Store(900,  PIO1)
            }
            Else{
                Store(Local5, PIO1)
            }
        }

        // Get DMA timings for
        // Drive 0
        If(And(Arg2,0x1))
        {
            // if UltraDMA enabled
            // xxxx.xxTT
            And(Arg3, 0x3, Local5)
            Store(DeRefOf(Index(DeReFof(Index(TIM0, 3)), Local5)), DMA0)
            Or(CHNF, 0x1, CHNF)            // using UltraDMA on drive 0
        }

        // Drive 1
        If(And(Arg2,0x2))
        {
            // if UltraDMA enabled
            // xxTT.xxxx
            And(ShiftRight(Arg3, 0x4), 0x3, Local5)
            Store(DeRefOf(Index(DeReFof(Index(TIM0, 3)), Local5)), DMA1)
            Or(CHNF, 0x4, CHNF)            // using UltraDMA on drive 1
        }
        Store(TMD0, Debug)
        Return(TMD0)
    }

    //
    // Set Timing PIO/DMA Mode.
    //
    //--------------------------------------------
    // INPUT  : nothing
    // RETURN : 0- Error, 1- OK to update registers
    //--------------------------------------------
    // UPDATED GLOBAL VARIABLES
    // GTIM(Word) - PIO/DMA I/O Timing register
    //    0             // Drive 0 Fast Timing Bank
    //    1,        // Drive 0 IORDY sampling mode
    //    2,             // Drive 0 Prefetch/Posting enable
    //    3,        // Drive 0 DMA timing mode
    //    4,             // Drive 1 Fast Timing Bank
    //    5,             // Drive 1 IORDY sampling mode
    //    6,             // Drive 1 Prefetch/Posting enable
    //    7,        // Drive 1 DMA timing mode
    //    8-13,        // Drive 0 IDE Timing Register
    //    14,        // Slave IDE timing enable
    //    15,            // IDE Channel Decode Enable
    // GSTM(4bit) - Slave I/O Timing register
    //    0-3
    // GUDM(2bit) - UDMA Control register
    //      0,              // Channel 0/1, Drive 0 SDMA mode
    //      1,              // Channel 0/1, Drive 1 SDMA mode
    // GUDT(Byte) - UDMA Timing register
    //    0-1        // UDMA timings for Drive 0
    //    2-3            // Reserved
    //    4-5        // UDMA timings for Drive 1
    //--------------------------------------------
    //

    Method(STM, 0, Serialized)
    {
        If(REGF){}        // PCI space not accessible
            Else    {  Return(0)  }

        // Preserve some IDETIM bits
        And(GTIM, 0x8044, GTIM)

        // Set UDMA to default state
            Store(0, GUDM)          // UltraDMA disabled on channel
            And(GUDT, 0xcc, GUDT)   // UDMA mode 0

        // Drive 0
        If(And(CHNF, 0x1))
        {
            Store(Match(DeRefOf(Index(TIM0, 3)), MLE, DMA0, MTR,0,0), Local0)
            If(LGreater(Local0, 2))            // Max mode is 2
            {
                Store(2, Local0)
            }
            Or(GUDT, Local0, GUDT)
            Or(GUDM, 0x1, GUDM)        // enable UltraDMA for Device 0
        }
        Else    // non - UDMA mode. Possible Multi word DMA
        {
            If(Or(LEqual(PIO0, Ones), LEqual(PIO0,0)))
            {
                If(And(LLess(DMA0, Ones), LGreater(DMA0,0)))
                {
                Store(DMA0, PIO0)        // Make PIO0=DMA0
                Or(GTIM, 0x8,  GTIM)    // Set DMA0 timing mode
                }
            }
        }

        // Drive 1
        If(And(CHNF, 0x4))
        {
            Store(Match(DeRefOf(Index(TIM0, 3)), MLT, DMA1, MTR,0,0), Local0)
            If(LGreater(Local0, 2))            // Max mode is 2
            {
                Store(2, Local0)
            }

            Or(GUDT, ShiftLeft(Local0, 0x4), GUDT)
            Or(GUDM, 0x2, GUDM)        // enable UltraDMA for Device 1
        }
        Else    // non - UDMA mode. Possible Multi word DMA
        {
            If(Or(LEqual(PIO1, Ones), LEqual(PIO1,0)))
            {
                If(And(LLess(DMA1, Ones), LGreater(DMA1,0)))
                {
                    Store(DMA1, PIO1)        // Make PIO1 = DMA1
                    Or(GTIM, 0x80,  GTIM)    // Set DMA1 timing mode
                }
            }
        }

        // Drive 0
        If(And(CHNF, 0x2))
        {
            Or(GTIM, 0x3, GTIM)    // Set IORDY0,Fast Timing Bank 0
        }

        // Drive 1
        If(And(CHNF, 0x8))        // Drive 1 is present
        {
            Or(GTIM, 0x30, GTIM)    // Set IORDY1,Fast Timing Bank 1
        }

        // Set IO timings for Drive 0
        And(Match(DeRefOf(Index(TIM0, 0)), MGE, PIO0, MTR,0,0), 0x3, Local0)
        Store(DeRefOf(Index(DeReFof(Index(TIM0, 1)), Local0)), Local1)
        ShiftLeft(Local1, 8, Local2)
        Or(GTIM, Local2, GTIM)

        // Slave timings can be set independently
        If(And(CHNF, 0x10))
        {
            Or(GTIM, 0x4000, GTIM)
            // Set IO timings for Drive 1
            And(Match(DeRefOf(Index(TIM0, 0)), MGE, PIO1, MTR,0,0), 0x3, Local0)
            Store(DeRefOf(Index(DeReFof(Index(TIM0, 2)), Local0)), GSTM)
        }

        Return(1)       // Return OK
    }

    // ATA command - Set Features
    Name(AT01, Buffer(){
//    1f1  1f2  1f3  1f4  1f5  1f6  1f7
    0x03,0x00,0x00,0x00,0x00,0x00,0xEF,    // Set Transfer mode commands
                            // 01:PIO Default transfer mode, disable IORDY
                            // 08:PIO Mode
                            // 20:Multi-word DMA Mode
                            // 40:Ultra DMA Mode
    })

    // ATA command - Set multiple mode
    Name(AT03, Buffer(){
//    1f1  1f2  1f3  1f4  1f5  1f6  1f7
    0x00,0x00,0x00,0x00,0x00,0x00,0xC6,
    })

    Name(ATA0, Buffer(29){})            // Primary Master
    Name(ATA1, Buffer(29){})            // Primary Slave
    Name(ATA2, Buffer(29){})            // Secondary Master
    Name(ATA3, Buffer(29){})           // Secondary Slave

    Name(ATAB, Buffer(29){})        // Return buffer
                            // First Byte is the number of ATA command
    CreateByteField(ATAB, 0, CMDC)    // ATA commands counter

    //
    // Build the return buffer for GTF() control method
    //      Arg0 - ATA command to write
    //      Arg1 - Subcommand value for "Set Feature command"
    //      Arg2 - Master/Slave ID (A0/B0)
    //

    Method (GTFB, 3, Serialized)
    {
        Multiply(CMDC, 56, Local0)
        Add(Local0, 8, Local1)
        CreateField(ATAB, Local1, 56, CMDx)     // command field
        Multiply(CMDC, 7, Local0)
        CreateByteField(ATAB, Add(Local0, 2), A001)     // Subcommand of "Set Feature" command
        CreateByteField(ATAB, Add(Local0, 6), A005)     // Master/Slave ID
        Store(Arg0, CMDx)                       // Store command into return buffer
        Store(Arg1, A001)                       // Set Subcommand code
        Store(Arg2, A005)                       // Master/Slave
        Increment(CMDC)
    }

    //
    // Update _GTF buffers for Master/Slave drive
    //
    // Arg0 = Master/Slave
    // Arg1 = ATA Block returned by the Drive
    //

    Method (GTF, 2, Serialized)
    {
        Store(Arg1, Debug)

        Store(0, CMDC)          // Initiate Command counter

    // Default ATA settings.
        Name(ID00, 0x80)        // removable media (CDROM) drive
        Name(ID49, 0x0c00)      // IORDY supported/ may be disabled
        Name(ID59, 0x00)        // Multiple mode, not supported
        Name(ID53, 0x04)        // UDMA field validity flag
        Name(ID63, 0xf00)        // DMA mode
        Name(ID88, 0xf00)        // UDMA mode

        Name(IRDY, 1)        // IORDY flag
        Name(PIOT, 0)        // PIO mode timings
        Name(DMAT, 0)        // DMA timings

        If(LEqual(SizeOf(Arg1), 0x200))              // Otherwise
                                    // Drive's either non-ATA(PI) standard or absent
        {
            // Identify Drive settings
            //        CreateByteField(Arg1, 0x0, IB00)    // General Configurration
            //        Store(IB00, ID00)

            //        CreateByteField(Arg1, 0x6, ID03)    // Number of Logical heads
            //        CreateByteField(Arg1, 0xc, ID06)    // Logical sectors per logical track

            CreateWordField(Arg1, 0x62,IW49)    // Get capabilities
                                    // bit 11-IORDY is supported
                                    // bit 10-IORDY can be disabled by setting feature command
            Store(IW49, ID49)

            CreateWordField(Arg1, 0x6a, IW53)    // Field validity (bit 0 -word54-58 are valid
                                    // bit 1 -word64-70, bit 2-word88 is valid)
            Store(IW53, ID53)

            CreateWordField(Arg1, 0x7e, IW63)    // bit 0-7 Multy-word DMA mode supported
                                    // bit 8-15 actual enabled DMA mode
            Store(IW63, ID63)

            CreateWordField(Arg1, 0x76, IW59)   // Multiple Sectors setting
                                    // (bit 8-enable, 0-7 actual value)
            Store(IW59, ID59)

            CreateWordField(Arg1, 0xb0, IW88)    // UltraDMA mode (bi0 Mode0, bit1-Mode1, ...bit4-Mode4)
                                    // bit 7-15 enabled UDMA mode
            Store(IW88, ID88)
        }

        // Initialize Local7 : Master/Slave identificator
        // Local7 to contain : 0xa0-Master / 0xb0-Slave
        Store(0xa0, Local7)

        // Slave drive settings
        If(Arg0)
        {
            Store(0xb0, Local7)        // Slave ID

            // Set IORDY
            And(CHNF, 0x08, IRDY)

            // Set PIO mode
            If(And(CHNF, 0x10))        // Independent timing for Slave
                { Store(PIO1, PIOT) }
            Else  { Store(PIO0, PIOT) }

            // Set DMA timing
            If(And(CHNF, 0x4))        // UDMA mode enabled?
            {
                If(And(CHNF, 0x10))    // Independent timing for Slave
                    { Store(DMA1, DMAT)}
                Else  { Store(DMA0, DMAT)}
            }
        }
        Else
        // Master drive settings
        {
            // Set IORDY
            And(CHNF, 0x02, IRDY)

            // Set PIO mode
            Store(PIO0, PIOT)

            // Is UDMA enabled?
            If(And(CHNF, 0x1)) { Store(DMA0, DMAT)}
        }

    /////////////////////////////////////////////////
    //2.0 Set UltraDMA mode feature command
    /////////////////////////////////////////////////
        If(LAnd(LAnd(And(ID53,0x4), And(ID88,0xff00)),DMAT))    // UDMA enabled?
        {
            Store(Match(DeRefOf(Index(TIM0, 3)), MLE, DMAT, MTR,0,0), Local1)
            If(LGreater(Local1, 2))            // Max supported UDMA mode is 2
            {
                Store(2, Local1)
            }
                                                            // 2nd command, Set features
            GTFB(AT01,Or(0x40, Local1),Local7)    // Subcommand code 40:UDMA mode
        }
        Else        // UDMA is not supported. What about Multi-word DMA ?
        {
    /////////////////////////////////////////////////
    //2.1 Set Multi-word DMA mode feature command
    /////////////////////////////////////////////////
            If(LAnd(And(ID63,0xff00),PIOT))
            {
                And(Match(DeRefOf(Index(TIM0, 0)), MGE, PIOT, MTR,0,0), 0x3, Local0)
                Or(0x20, DeRefOf(Index(DeReFof(Index(TIM0, 5)), Local0)), Local1)
                                    // 2nd command, Set features
                GTFB(AT01, Local1, Local7)      // Subcommand code 20:Multi-word DMA mode
            }
        }
    /////////////////////////////////////////////////
    //3.0 Set PIO mode feature command
    /////////////////////////////////////////////////
        If(IRDY)    // IORDY supported? If PIO mode set?
        {
                And(Match(DeRefOf(Index(TIM0, 0)), MGE, PIOT, MTR,0,0), 0x3, Local0)
                Or(0x08, DeRefOf(Index(DeReFof(Index(TIM0, 4)), Local0)), Local1)
                                    // 3rd command, Set features
                GTFB(AT01,Local1,Local7)      // Subcommand code 08:PIO Mode
        }
        Else
        {
    /////////////////////////////////////////////////
    //3.1 Set Default PIO mode. Disable IORDY
    /////////////////////////////////////////////////
            If(And(ID49,0x400))               // bit 10-IORDY can be disabled by setting feature command
            {
                                    // 3rd command, Set features
                GTFB(AT01,0x01,Local7)        // Subcommand code : 01
                                    // PIO Default transfer mode, disable IORDY
            }
        }
    /////////////////////////////////////////////////
    // 4. Set Multiple mode (if supported)
    /////////////////////////////////////////////////
        If(LAnd(And(ID59,0x100),And(ID59,0xff)))    // Multiple mode is supported
        {
              GTFB(AT03, And(ID59, 0xff),Local7)  // 4th command, Set Multiple mode
        }

        Store(ATAB, Debug)

        Return(ATAB)
    }

    //
    // Prepare ATA buffer to be returned by _GTF
    //
    // Input - Arg0 Source buffer to modify
    //

    Method(RATA, 1)
    {
        CreateByteField(Arg0, 0, CMDN)
        Multiply(CMDN, 56, Local0)        // Return buffer size (bits)
        CreateField(Arg0, 8, Local0, RETB)// Return ATA command Buffer

        Store(RETB, Debug)
        Return(RETB)
    }
}

