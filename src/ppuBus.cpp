#include "ppuBus.h"
#include "Log.h"

namespace yn
{
    PpuBus::PpuBus() : m_mapper(nullptr), m_RAM(0x800), m_palette(0x20)
    {
    }

    void PpuBus::write(Address addr, Byte value)
    {

        if (addr < 0x2000)
        {
            m_mapper->write(addr, value);
        }
        else if (addr < 0x3eff) //Name tables upto 0x3000, then mirrored upto 3eff
        {
            auto index = addr & 0x3ff;
            if (addr < 0x2400) //NT0
                m_RAM[NameTable0 + index] = value;
            else if (addr < 0x2800) //NT1
                m_RAM[NameTable1 + index] = value;
            else if (addr < 0x2c00) //NT2
                m_RAM[NameTable2 + index] = value;
            else //NT3
                m_RAM[NameTable3 + index] = value;
        }
        else if (addr < 0x3fff)
        {
            if (addr == 0x3f10)
                m_palette[0] = value;
            else
                m_palette[addr & 0x1f] = value;
        }
    }

    Byte PpuBus::read(Address addr)
    {
        if (addr < 0x2000)
        {
            return m_mapper->read(addr);
        }
        else if (addr < 0x3f00)
        {
            auto index = addr & 0x3ff;
            if (addr < 0x2400)
                return m_RAM[NameTable0 + index];
            else if (addr < 0x2800)
                return m_RAM[NameTable1 + index];
            else if (addr < 0x2c00)
                return m_RAM[NameTable2 + index];
            else
                return m_RAM[NameTable3 + index];
        }
        else if (addr < 0x4000)
        {
            return m_palette[addr & 0x1f];
        }
        else
        {
            LOG(Error)<<"not expect read address."<<std::endl;
        }
        return 0;
    }

    Byte PpuBus::readPalette(Byte paletteAddr)
    {
        return m_palette[paletteAddr];
    }

    bool PpuBus::setMapper(Mapper *mapper)
    {
        if (!mapper)
        {
            LOG(Error) << "Mapper argument is nullptr" << std::endl;
            return false;
        }

        m_mapper = mapper;
        updateMirroring();
        return true;
    }

    void PpuBus::updateMirroring()
    {
        switch (m_mapper->getNameTableMirroring())
        {
        case NameTableMirroring::Horizontal:
            NameTable0 = NameTable1 = 0;
            NameTable2 = NameTable3 = 0x400;
            LOG(Info) << "Horizontal Name Table mirroring set." << std::endl;
            break;
        case NameTableMirroring::Vertical:
            NameTable0 = NameTable2 = 0;
            NameTable1 = NameTable3 = 0x400;
            LOG(Info) << "Vertical Name Table mirroring set." << std::endl;
            break;
        // TODO other nametable mirroring
        default:
            NameTable0=0;
            NameTable1=0x400;
            NameTable2=0x800;
            NameTable3=0xc00;
            LOG(Error)<<"Four screen Name Table mirroring set but not implimented."<<std::endl;
        }
    }
}