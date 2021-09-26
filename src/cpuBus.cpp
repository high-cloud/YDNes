#include "cpuBus.h"
#include "Log.h"

namespace yn
{
    CpuBus::CpuBus():
        m_internalRam(0x800,0),
        m_mapper(nullptr)
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
            auto it = m_readCallbacks.find(static_cast<IORegister>(addr & 0x2007));
            if (it != m_readCallbacks.end())
            {
                return (it->second)();
            }
            else
                LOG(Info) << "NO read callback for IO register at " << std::hex << +addr << std::endl;
        }
        else if (addr < 0x4020)
        { // IO registers

            auto it = m_readCallbacks.find(static_cast<IORegister>(addr));
            if (it != m_readCallbacks.end())
                return (it->second)();
            //Second object is the pointer to the function object
            //Dereference the function pointer and call it
            else
                LOG(Info) << "No read callback registered for I/O register at: " << std::hex << +addr << std::endl;
        }
        else if (addr < 0x6000)
        {
            // Expansion ROM
            LOG(Error) << "havent realized expansion rom" << std::endl;
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
            auto it = m_writeCallbacks.find(static_cast<IORegister>(addr & 0x2007));
            if (it != m_writeCallbacks.end())
            {
                return (it->second)(value);
            }
            else
                LOG(Info) << "NO write callback for IO register at " << std::hex << +addr << std::endl;
        }
        else if (addr < 0x4020)
        { // IO registers
        auto it = m_writeCallbacks.find(static_cast<IORegister>(addr));
            if (it != m_writeCallbacks.end())
            {
                return (it->second)(value);
            }
            else
                LOG(Debug) << "NO write callback for IO register at " << std::hex << +addr << std::endl;
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

    Byte *CpuBus::getPagePtr(Byte page)
    {
        Address addr = page << 8;
        if (addr < 0x2000)
        {
            return &m_internalRam[addr & 0x07ff];
        }
        else if (addr < 0x4020)
        {
            LOG(Error) << "Register address memory pointer access attempt" << std::endl;
        }
        else if (addr < 0x6000)
        {
            LOG(Error) << "Expansion ROM access attempted, which is unsupported" << std::endl;
        }
        else if (addr < 0x8000)
        {
            if (m_mapper->ifVRAM())
            {
                return &m_VRAM[addr - 0x6000];
            }
        }
            
        return nullptr;

    }

} // namespace yn