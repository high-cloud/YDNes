#ifndef __CPUBUS_H__
#define __CPUBUS_H__

#include "constant.h"
#include "mapper.h"
#include<vector>
#include<map>
#include<functional>

namespace yn
{
    enum class IORegister{
        PPUCTRL=0X2000,
        PPUMASK,
        PPUSTATUS,
        OAMADDR,
        OAMDATA,
        PPUSCROL,
        PPUADDR,
        PPUDATA,
        OAMDMA=0X4014,
        JOY1=0X4016,
        JOY2=0X4017,
    };

    class CpuBus
    {
    public:
        CpuBus();
        Byte read(Address addr);
        void write(Address addr, Byte value);
        bool setMapper(Mapper *mapper);
        bool setWriteCallback(IORegister reg,std::function<void(Byte)> callback);
        bool setReadCallback(IORegister reg,std::function<Byte(void)> callback);
        Byte* getPagePtr(Byte page);

    private:
        Byte m_internalRam[0x800];
        std::vector<Byte> m_VRAM;
        
        Mapper *m_mapper;
        std::map<IORegister,std::function<void(Byte)>> m_writeCallbacks;
        std::map<IORegister,std::function<Byte(void)>> m_readCallbacks;

    };
} // namespace yn
#endif // __CPUBUS_H__