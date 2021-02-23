#include "cpuBus.h"
#include "Log.h"

namespace yn
{
    CpuBus::CpuBus()
    {
    }

    Byte CpuBus::read(Address addr)
    {
        if (addr < 0x2000)
        {
            // internal RAM and mirrors
            return m_internalRam[addr & 0x07ff];
        }
        else if (addr < 0x4000)
        {
            // ppu register and mirrors  (& 0x2007)
        }
        else if (addr < 0x4020)
        { // IO registers
        }
        else if (addr < 0x6000)
        {
            // Expansion ROM
            LOG(Error) << "not support expansion rom" << std::endl;
        }
        else if (addr < 0x8000)
        {
            if (m_mapper->ifVRAM())
            {
                return m_VRAM[addr - 0x6000];
            }
            else
            {
                LOG(Error) << "try to read VRAM at: " << addr << " but have no VRAM." << std::endl;
            }
        }
        else
        {
            return m_mapper->read(addr);
        }

        return 0;
    }

    void CpuBus::write(Address addr, Byte value)
    {
        if (addr < 0x2000)
        {
            // internal RAM and mirrors
            m_internalRam[addr & 0x07ff] = value;
        }
        else if (addr < 0x4000)
        {
            // ppu register and mirrors  (& 0x2007)
        }
        else if (addr < 0x4020)
        { // IO registers
        }
        else if (addr < 0x6000)
        {
            // Expansion ROM
            LOG(Error) << "not support expansion rom" << std::endl;
        }
        else if (addr < 0x8000)
        {
            if (m_mapper->ifVRAM())
            {
                m_VRAM[addr - 0x6000] = value;
            }
            else
            {
                LOG(Error) << "try to write " << std::hex << value << "VRAM at: " << addr << " but have no VRAM." << std::endl;
            }
        }
        else
        {
            m_mapper->write(addr, value);
        }
    }

    bool CpuBus::setMapper(Mapper *mapper)
    {
        m_mapper = mapper;

        if (!m_mapper)
        {
            LOG(Error) << "Mapper pointer is nullptr" << std::endl;
            return false;
        }

        if (m_mapper->ifVRAM())
        {
            m_VRAM.resize(0x2000);
        }

        return true;
    }

    bool CpuBus::setWriteCallback(IORegister reg, std::function<void(Byte)> callback)
    {
        if (!callback)
        {
            LOG(Error) << "callback argument is nullptr" << std::endl;
            return false;
        }
        return m_writeCallbacks.emplace(reg, callback).second;
    }

    bool CpuBus::setReadCallback(IORegister reg, std::function<Byte(void)> callback)
    {
        if (!callback)
        {
            LOG(Error) << "callback argument is nullptr" << std::endl;
            return false;
        }
        return m_readCallbacks.emplace(reg, callback).second;
    }

} // namespace yn