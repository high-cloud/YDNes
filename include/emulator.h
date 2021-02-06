#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include "cpu.h"
#include "cartridge.h"
#include "mapper.h"
#include <string>
#include <memory>

namespace yn
{

    class Emulator
    {
    private:
        Cpu m_cpu;
        Cartridge m_cart;
        CpuBus m_cpuBus;
        std::unique_ptr<Mapper> m_mapper;

    public:
        Emulator();
        void run(std::string path);
    };
} // namespace yn
#endif // __EMULATOR_H__