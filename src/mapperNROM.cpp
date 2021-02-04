#include "mapperNROM.h"
#include "Log.h"

namespace yn
{
    MapperNROM::MapperNROM(Cartridge cart)
        : Mapper(cart, Type::NROM)
    {
        if (cart.getROM().size() == 0x4000)
        { //one bank, need mirror
            m_oneBank = true;
        }
        else
        {
            m_oneBank = false;
        }

        if (cart.getVROM().size() == 0)
        {
            //has VRAM
            m_useChrRAM = true;
            m_chrRAM.resize(0x2000);
            LOG(Info) << "mapper use chrRAM" << std::endl;
        }
        else
        {
            m_useChrRAM = false;
        }
    }

    Byte MapperNROM::read(Address addr)
    {
        if (addr < 0x2000)
        {
            //CHR ROM
            if (m_useChrRAM)
            {
                return m_chrRAM[addr];
            }
            else
            {
                return m_cartrige.getVROM()[addr];
            }
        }
        else if (addr > 0x8000)
        {
            if (m_oneBank)
            {
                return m_cartrige.getROM()[addr & 0x3fff];
            }
            else
            {
                return m_cartrige.getROM()[addr - 0x3fff];
            }
        }
        else
        {
            LOG(Error) << "mapper read out of range" << std::endl;
        }

        return 0;
    }

    void MapperNROM::write(Address addr, Byte value)
    {
        if (addr < 0x2000)
        {
            if (m_useChrRAM)
            {
                m_chrRAM[addr] = value;
            }
            else
            {
                LOG(Info) << "attempt to write: " << std::hex << value << " at CHR ROM address: " << addr << std::endl;
            }
        }
        else if (addr > 0x8000)
        {
            LOG(Info) << "attempt to write: " << std::hex << value << " at PGR ROM address: " << addr << std::endl;
        }
        else
        {
            LOG(Error) << "mapper write out of range" << std::endl;
        }
    }

    bool MapperNROM::ifVRAM()
    {
        return false;
    }

} // namespace yn