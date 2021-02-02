#ifndef __CPUOPCODE_H__
#define __CPUOPCODE_H__

namespace yn
{
    // interrupt vector
    const auto NMIVector=0xfffc;
    const auto IRQVector=0xfffe;
    const auto ResetVector=0xfffa;

    // judge operation
    const auto InstructionTypeMask=0x3;
    
    const auto AddrModeMask=0x1c;
    const auto AddrModeShift=2;
    


    enum Operation1{
        ORA,
        AND,
        EOR,
        ADC,
        STA,
        LDA,
        CMP,
        SBC
    };

    enum AddrMode1{
        IndexedIndirectX,
        ZeroPage,
        Immediate,
        Absolute,
        IndirectY,
        IndexedX,
        AbsoluteY,
        AbsoluteX,
    };

    enum Operation2{
        ASL,
        ROL,
        LSR,
        ROR,
        STX,
        LDX,
        DEC,
        INC
    };

}

#endif // __CPUOPCODE_H__