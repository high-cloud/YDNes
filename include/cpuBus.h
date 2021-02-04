#ifndef __CPUBUS_H__
#define __CPUBUS_H__

#include "constant.h"
#include "mapper.h"
#include<vector>

namespace yn
{

    class CpuBus
    {
    public:
        CpuBus();
        Byte read(Address addr);
        void write(Address addr, Byte value);
        bool setMapper(Mapper *mapper);

    private:
        Byte m_internalRam[0x800];
        std::vector<Byte> m_VRAM;
        
        Mapper *m_mapper;
    };
} // namespace yn
#endif // __CPUBUS_H__