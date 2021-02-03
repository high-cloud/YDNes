#include "cpu.h"
#include "cpuOpcode.h"
#include "Log.h"
#include <iomanip>

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
        if (m_skipCycles-- > 1)
        {
            return;
        }

        m_skipCycles = 0;

        int r_p = f_N << 7 |
                  f_V << 6 |
                  1 << 5 |
                  f_D << 3 |
                  f_I << 2 |
                  f_Z << 1 |
                  f_C;

        // log cpu
        LOG_CPU << std::hex << std::setfill('0') << std::uppercase
                << std::setw(4) << +r_PC
                << "  "
                << std::setw(2) << +m_bus.read(r_PC)
                << "  "
                << "A:" << std::setw(2) << +r_A << " "
                << "X:" << std::setw(2) << +r_X << " "
                << "Y:" << std::setw(2) << +r_Y << " "
                << "P:" << std::setw(2) << r_p << " "
                << "SP:" << std::setw(2) << +r_SP << /*std::endl;*/ " "
                << "CYC:" << std::setw(3) << std::setfill(' ') << std::dec << ((m_cycles - 1) * 3) % 341
                << std::endl;

        Byte opcode = m_bus.read(r_PC++);
        int requireCycles = OperationCycles[opcode];

        if (requireCycles && (executeImplied(opcode) || executeType1(opcode) ||
                              executeBranch(opcode) || executeType2(opcode) || executeType0(opcode)))
        {
            m_skipCycles += requireCycles;
        }
        else
        {
            LOG(Error) << "Can't recognise opcode." << std::hex << opcode << std::endl;
        }
    }

    bool Cpu::executeType0(Byte opcode)
    {
        if ((opcode & InstructionTypeMask) == 0x0)
        {
            AddrMode2 addrMode = static_cast<AddrMode2>((opcode & AddrModeMask) >> AddrModeShift);
            auto op = static_cast<Operation0>((opcode & OperationMask) >> OperationShift);

            Address location = 0;
            switch (addrMode)
            {
            case Immediate_:
                location = r_PC++;
                break;
            case ZeroPage_:
                location = m_bus.read(r_PC++);
                break;
            case Absolute_:
                location = readAddress(r_PC);
                r_PC += 2;
                break;
            case IndexedXorY:
            {
                Byte index;
                if (op == LDX || op == STX)
                    index = r_Y;
                else
                    index = r_X;
                location = (m_bus.read(r_PC++) + index) & 0xff;
            }
            break;
            case AbsoluteXorY:
            {
                location = readAddress(r_PC);
                r_PC += 2;
                Byte index;
                if (op == LDX || op == STX)
                    index = r_Y;
                else
                    index = r_X;
                if (op == LDX)
                    setPageCrossed(index + location, location);
                location += r_X;
            }
            break;
            default:
                return false;
            }

            Byte operand = 0;
            switch (op)
            {
            case BIT:
                operand = m_bus.read(location);
                f_Z = !(operand & r_A);
                f_V = operand & 0x40;
                f_N = operand & 0x80;
                break;
            case STY:
                m_bus.write(location, r_Y);
                break;
            case LDY:
                operand = m_bus.read(location);
                r_Y = operand;
                setZN(r_Y);
                break;
            case CPY:
            {
                Address diff = r_Y - m_bus.read(location);
                f_C = !(diff & 0x100);
                setZN(diff);
            }
            break;
            case CPX:
            {
                Address diff = r_X - m_bus.read(location);
                f_C = !(diff & 0x100);
                setZN(diff);
            }
            break;
            default:
                return false;
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    bool Cpu::executeType1(Byte opcode)
    {
        if ((opcode & InstructionTypeMask) == 0x1)
        {
            AddrMode1 addrMode = static_cast<AddrMode1>((opcode & AddrModeMask) >> AddrModeShift);
            auto op = static_cast<Operation1>((opcode & OperationMask) >> OperationShift);

            Address location = 0;
            switch (addrMode)
            {
            case Immediate:
                location = r_PC++;
                break;
            case IndexedIndirectX:
            {
                Byte tempPage = r_X + m_bus.read(r_PC++);
                location = (m_bus.read((tempPage + 1) & 0xff) << 8) | m_bus.read(tempPage & 0xff);
            }
            break;
            case ZeroPage:
                location = m_bus.read(r_PC++);
                break;
            case Absolute:
                location = readAddress(r_PC);
                r_PC += 2;
                break;
            case IndirectY:
            {
                Byte tempPage = m_bus.read(r_PC++);
                location = (m_bus.read((tempPage + 1) & 0xff) << 8) | m_bus.read(tempPage & 0xff);
                if (op != STA)
                {
                    setPageCrossed(location, location + r_Y);
                }
                location += r_Y;
            }
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
            {
                auto operand = m_bus.read(location);
                Address temp = r_A + f_C + operand;
                // set Carry flag
                f_C = temp & 0x100;
                // set overflow flag
                f_V = (temp ^ r_A) & (temp ^ operand) & 0x80;

                r_A = temp & 0xff;
                setZN(r_A);
            }
            break;
            case STA:
                m_bus.write(location, r_A);
                break;
            case LDA:
                r_A = m_bus.read(location);
                setZN(r_A);
                break;
            case CMP:
            {
                Address diff = r_A - m_bus.read(location);
                f_C = !(diff & 0x100);
                setZN(diff);
            }
            break;
            case SBC:
            {
                std::uint16_t subtrahend = m_bus.read(location);
                std::uint16_t diff = r_A - subtrahend - !f_C;
                f_C = !(diff & 0x100);
                f_V = (r_A ^ diff) & (~subtrahend ^ diff) & 0x80;
                r_A = diff;
                setZN(r_A);
            }
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

    bool Cpu::executeImplied(Byte opcode)
    {
        switch (opcode)
        {
        case NOP:
            ++r_PC;
            break;
        case BRK:
            interrupt(BRK_);
            break;
        case JSR:
            //The JSR instruction pushes the address (minus one) of
            //the return point on to the stack and then sets
            //the program counter to the target memory address.
            pushStack(static_cast<Byte>((r_PC + 1) >> 8));
            pushStack(static_cast<Byte>(r_PC + 1));
            r_PC = readAddress(r_PC);
            break;
        case RTI:
        {
            Byte flags = popStack();
            f_N = flags & 0x80;
            f_V = flags & 0x40;
            f_D = flags & 0x8;
            f_I = flags & 0x4;
            f_Z = flags & 0x2;
            f_C = flags & 0x1;
        }
            r_PC = popStack();
            r_PC |= popStack() << 8;
            break;
        case RTS:
            r_PC = popStack();
            r_PC |= popStack() << 8;
            ++r_PC;
            break;
        case JMP:
            r_PC = readAddress(r_PC);
            break;
        case JMPI:
        {
            r_PC = readAddress(r_PC);
            // conssider the bug of nes
            Address page = r_PC & 0xff00;
            r_PC = m_bus.read(r_PC) | (m_bus.read(page | (r_PC + 1) & 0x00ff) << 8);
        }
        break;

        case PHP:
        {
            Byte flags = f_N << 7 |
                         f_V << 6 |
                         1 << 5 |
                         f_D << 3 |
                         f_I << 2 |
                         f_Z << 1 |
                         f_C;
            pushStack(flags);
        }
        break;
        case PLP:
        {
            Byte flags = popStack();
            f_N = flags & 0x80;
            f_V = flags & 0x40;
            f_D = flags & 0x8;
            f_I = flags & 0x4;
            f_Z = flags & 0x2;
            f_C = flags & 0x1;
        }
        break;
        case PHA:
            pushStack(r_A);
            break;
        case PLA:
            r_A = popStack();
            setZN(r_A);
            break;
        case DEY:
            --r_Y;
            setZN(r_Y);
            break;
        case DEX:
            --r_X;
            setZN(r_X);
            break;
        case TAY:
            r_Y = r_A;
            setZN(r_Y);
            break;
        case INY:
            ++r_Y;
            setZN(r_Y);
            break;
        case INX:
            ++r_X;
            setZN(r_X);
            break;
        case CLC:
            f_C = false;
            break;
        case SEC:
            f_C = true;
            break;
        case CLI:
            f_I = false;
            break;
        case SEI:
            f_I = true;
            break;
        case TYA:
            r_A = r_Y;
            setZN(r_A);
            break;
        case CLV:
            f_V = false;
            break;
        case CLD:
            f_D = false;
            break;
        case SED:
            f_D = true;
            break;
        case TXA:
            r_A = r_X;
            setZN(r_A);
            break;
        case TXS:
            r_SP = r_X;
            break;
        case TAX:
            r_X = r_A;
            setZN(r_X);
            break;
        case TSX:
            r_X = r_SP;
            setZN(r_X);
            break;
        default:
            return false;
        }
        return true;
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
                setPageCrossed(r_PC, new_PC, 2);
                r_PC = new_PC;
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

    bool Cpu::executeType2(Byte opcode)
    {
        if ((opcode & InstructionTypeMask) == 0x2)
        {
            AddrMode2 addrMode = static_cast<AddrMode2>((opcode & AddrModeMask) >> AddrModeShift);
            auto op = static_cast<Operation2>((opcode & OperationMask) >> OperationShift);

            Address location = 0;
            switch (addrMode)
            {
            case Immediate_:
                location = r_PC++;
                break;
            case ZeroPage_:
                location = m_bus.read(r_PC++);
                break;
            case Absolute_:
                location = readAddress(r_PC);
                r_PC += 2;
                break;
            case IndexedXorY:
            {
                Byte index;
                if (op == LDX || op == STX)
                    index = r_Y;
                else
                    index = r_X;
                location = (m_bus.read(r_PC++) + index) & 0xff;
            }
            break;
            case AbsoluteXorY:
            {
                location = readAddress(r_PC);
                r_PC += 2;
                Byte index;
                if (op == LDX || op == STX)
                    index = r_Y;
                else
                    index = r_X;
                if (op == LDX)
                    setPageCrossed(index + location, location);
                location += r_X;
            }
            break;
            case Accumulator:
                break;
            default:
                return false;
            }

            switch (op)
            {
            case ASL:
            {
                if (addrMode == Accumulator)
                {
                    f_C = r_A & 0x80;
                    r_A <<= 1;
                    setZN(r_A);
                }
                else
                {
                    Byte operand = m_bus.read(location);
                    f_C = operand & 0x80;
                    operand <<= 1;
                    setZN(operand);
                    m_bus.write(location, operand);
                }
            }
            break;
            case ROL:
            {

                if (addrMode == Accumulator)
                {
                    bool prev_C = f_C;
                    f_C = r_A & 0x80;
                    r_A <<= 1;
                    r_A |= prev_C;
                    setZN(r_A);
                }
                else
                {
                    bool prev_C = f_C;
                    Byte operand = m_bus.read(location);
                    f_C = operand & 0x80;
                    operand <<= 1;
                    operand |= prev_C;
                    setZN(operand);
                    m_bus.write(location, operand);
                }
            }
            break;
            case ROR:
            {
                if (addrMode == Accumulator)
                {
                    bool prev_C = f_C;
                    f_C = r_A & 0x1;
                    r_A >>= 1;
                    r_A |= prev_C << 7;
                    setZN(r_A);
                }
                else
                {
                    bool prev_C = f_C;
                    Byte operand = m_bus.read(location);
                    f_C = operand & 0x1;
                    operand >>= 1;
                    operand |= prev_C << 7;
                    setZN(operand);
                    m_bus.write(location, operand);
                }
            }
            break;
            case LSR:
            {
                if (addrMode == Accumulator)
                {
                    f_C = r_A & 1;
                    r_A >>= 1;
                    setZN(r_A);
                }
                else
                {
                    Byte operand = m_bus.read(location);
                    f_C = operand & 0x1;
                    operand >>= 1;
                    setZN(operand);
                    m_bus.write(location, operand);
                }
            }
            break;
            case STX:
                m_bus.write(location, r_X);
                break;
            case LDX:
                r_X = m_bus.read(location);
                setZN(r_X);
                break;
            case DEC:
            {
                Byte operand = m_bus.read(location);
                --operand;
                setZN(operand);
                m_bus.write(location, operand);
            }
            break;
            case INC:
            {
                Byte operand = m_bus.read(location);
                ++operand;
                setZN(operand);
                m_bus.write(location, operand);
            }
            break;
            default:
                return false;
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

    void Cpu::setPageCrossed(Address a, Address b, int inc)
    {
        if ((a & 0x00ff) != (b & 0x00ff))
        {
            m_skipCycles += inc;
        }
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