#ifndef __CPUBUS_H__
#define __CPUBUS_H__

#include "constant.h"

namespace yn
{

    class CpuBus
    {
    public:
        CpuBus();
        Byte read(Address addr);
        void write(Address addr, Byte value);
    };
} // namespace yn
#endif // __CPUBUS_H__