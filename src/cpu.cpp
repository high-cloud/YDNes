#include "cpu.h"
#include "cpuOpcode.h"

namespace yn
{
    Cpu::Cpu(CpuBus &bus)
        : m_bus(bus)
    {
    }

    void Cpu::interrupt(InterruptType type)
    {
        //TODO
    }

    void Cpu::reset()
    {
        reset(ResetVector);
    }

    void Cpu::reset(Address addr)
    {
        r_A = r_X = r_Y = 0;
        m_cycles = m_skipCycles = 0;
        f_I = true;
        f_C = f_D = f_N = f_N = f_V = false;
        r_PC = addr;
        r_SP = 0xfd;
    }

    void Cpu::step()
    {
        ++m_cycles;
        if (m_skipCycles-- > 0)
        {
            return;
        }

        m_skipCycles = 0;
    }

    bool Cpu::executeType1(Byte opcode)
    {
        if ((opcode & InstructionTypeMask) == 0x1)
        {
            AddrMode1 addrMode = (opcode & AddrModeMask) >> AddrModeShift;
            auto op = static_cast<Operation1>((opcode & OperationMask) >> OperationShift);

            Address location = 0;
            switch (addrMode)
            {
            case Immediate:
                location = r_PC++;
                break;
            case IndexedIndirectX:
                Byte tempPage = r_X + m_bus.read(r_PC++);
                location = (m_bus.read((tempPage + 1) & 0xff) << 8) | m_bus.read(tempPage & 0xff);
                break;
            case ZeroPage:
                location = m_bus.read(r_PC++);
                break;
            case Absolute:
                location = readAddress(r_PC);
                r_PC += 2;
                break;
            case IndirectY:
                Byte tempPage = m_bus.read(r_PC++);
                location = (m_bus.read((tempPage + 1) & 0xff) << 8) | m_bus.read(tempPage & 0xff);
                if (op != STA)
                {
                    setPageCrossed(location, location + r_Y);
                }
                location += r_Y;
                break;
            case IndexedX:
                location = (m_bus.read(r_PC++) + r_X) & 0xff;
                break;
            case AbsoluteX:
                location = readAddress(r_PC);
                r_PC += 2;
                if (op != STA)
                {
                    setPageCrossed(location, location + r_X);
                }
                location += r_X;
            case AbsoluteY:
                location = readAddress(r_PC);
                r_PC += 2;
                if (op != STA)
                {
                    setPageCrossed(location, location + r_Y);
                }
                location += r_Y;
                break;
            default:
                return false;
            }

            switch (op)
            {
            case ORA:
                r_A |= m_bus.read(location);
                setZN(r_A);
                break;
            case AND:
                r_A &= m_bus.read(location);
                setZN(r_A);
                break;
            case EOR:
                r_A ^= m_bus.read(location);
                setZN(r_A);
                break;
            case ADC:
                auto operand = m_bus.read(location);
                Address temp = r_A + f_C + operand;
                // set Carry flag
                f_C = temp & 0x100;
                // set overflow flag
                f_V = (temp ^ r_A) & (temp ^ operand) & 0x80;

                r_A = temp & 0xff;
                setZN(r_A);
                break;
            case STA:
                m_bus.write(location, r_A);
                break;
            case LDA:
                r_A = m_bus.read(location);
                setZN(r_A);
                break;
            case CMP:
                Address diff = r_A - m_bus.read(location);
                f_C = !(diff & 0x100);
                setZN(diff);
                break;
            case SBC:
                std::uint16_t subtrahend = m_bus.read(location);
                std::uint16_t diff = r_A - subtrahend - !f_C;
                f_C = !(diff & 0x100);
                f_V = (r_A ^ diff) & (~subtrahend ^ diff) & 0x80;
                r_A = diff;
                setZN(r_A);
                break;
            default:
                return false;
                break;
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    bool Cpu::executeBranch(Byte opcode)
    {
        if ((opcode & BranchInstructionMask) == BranchInstructionMaskResult)
        {
            bool branch = opcode & BranchConditionMask;

            switch (opcode >> BranchFlagShift)
            {
            case Negative:
                branch = !(branch ^ f_N);
                break;
            case OverFlow:
                branch = !(branch ^ f_V);
                break;
            case Carry:
                branch = !(branch ^ f_C);
                break;
            case Zero:
                branch = !(branch ^ f_Z);
                break;
            default:
                return false;
            }

            if (branch)
            {
                ++m_skipCycles;
                Byte offset = m_bus.read(r_PC++); // TODO: whether plus pc
                Address new_PC = r_PC + offset;
                setPageCrossed(r_PC,new_PC,2);
                r_PC=new_PC;
            }
            else
            {
                ++r_PC;
            }
            
            return true;
        }
        else
        {
            return false;
        }
    }

    void Cpu::pushStack(Byte value)
    {
        m_bus.write(0x0100 | r_SP, value);
        --r_SP;
    }

    Byte Cpu::popStack()
    {
        return m_bus.read(0x100 | ++r_SP);
    }

    void Cpu::setZN(Byte value)
    {
        f_Z = !value;
        f_N = value & 0x80;
    }

    Address Cpu::readAddress(Address addr)
    {
        return m_bus.read(addr) | (m_bus.read(addr + 1) & 0xff) << 8;
    }
} // namespace yn