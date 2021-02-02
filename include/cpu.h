#ifndef __CPU_H__
#define __CPU_H__

#include"constant.h"
#include"cpuBus.h"
#include"cpuOpcode.h"

namespace yn
{
    class Cpu{
        public:
            enum InterruptType{
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
            bool executeType1(Byte opcode);

            void pushStack(Byte value);
            Byte popStack();

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
}
#endif // __CPU_H__