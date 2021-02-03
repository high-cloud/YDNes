#ifndef __CPU_H__
#define __CPU_H__

#include "constant.h"
#include "cpuOpcode.h"
#include "cpuBus.h"

namespace yn
{
    class Cpu
    {
    public:
        enum InterruptType
        {
            IRQ,
            NMI,
            BRK_
        };

        Cpu(CpuBus &bus);

        void interrupt(InterruptType type);

        void reset();
        void reset(Address addr);
        void step();

    private:
        bool executeType0(Byte opcode);
        bool executeType1(Byte opcode);
        bool executeImplied(Byte opcode);
        bool executeBranch(Byte opcode);
        bool executeType2(Byte opcode);



            void pushStack(Byte value);
        Byte popStack();

        // if a b in different page, skipCycles+=inc
        void setPageCrossed(Address a, Address b, int inc = 1);

        void setZN(Byte value);

        // a warp of bus read
        Address readAddress(Address addr);

        CpuBus &m_bus;

        //registers
        Address r_PC;
        Byte r_SP;
        Byte r_A;
        Byte r_X;
        Byte r_Y;

        // register P is divided into flags
        bool f_C;
        bool f_Z;
        bool f_I;
        bool f_D;
        bool f_V;
        bool f_N;
        // cycle relate
        int m_cycles;
        int m_skipCycles;
    };
} // namespace yn
#endif // __CPU_H__