#include "cpu.h"

namespace yn
{
    bool Cpu::executeType1(Byte opcode)
    {
        if ((opcode & instructionTypeMask) == 0x1)
        {
            switch ((opcode & AddrMode1))
            {
            case :
                break;
            
            default:
                break;
            }
        }
        else
        {
            return false;
        }
        
    }
} // namespace yn