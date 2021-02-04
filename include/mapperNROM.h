#ifndef __MAPPERNROM_H__
#define __MAPPERNROM_H__

#include "mapper.h"
#include "cartridge.h"
#include <vector>

namespace yn
{
    class MapperNROM : public Mapper
    {
    public:
        MapperNROM(Cartridge);
        Byte read(Address) override;
        void write(Address addr, Byte value) override;
        bool ifVRAM() override;

    private:
        bool m_oneBank;
        bool m_useChrRAM;
        std::vector<Byte> m_chrRAM;
    };
} // namespace yn
#endif // __MAPPERNROM_H__