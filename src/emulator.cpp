#include "emulator.h"
#include "Log.h"

namespace yn
{
    Emulator::Emulator() : m_cpu(m_cpuBus)
    {
    }

    void Emulator::run(std::string path)
    {
        m_cart.loadFromFile(path);

        m_mapper = Mapper::createMapper(static_cast<Mapper::Type>(m_cart.getMapperNumber()), m_cart);

        if (!m_mapper)
        {
            LOG(Error) << "Creating Mapper failed, maybe unsupport Mapper." << std::endl;
            return;
        }

        if (!m_cpuBus.setMapper(m_mapper.get()))
        {
            return;
        }

        m_cpu.reset(0xc000);

        for (int i = 0; i < 1000; ++i)
        {
            m_cpu.step();
        }
    }
} // namespace yn