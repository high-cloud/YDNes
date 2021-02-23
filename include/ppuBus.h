#ifndef __PPUBUS_H__
#define __PPUBUS_H__

#include "constant.h"
#include "mapper.h"
#include <vector>

namespace yn
{
    class PpuBus
    {
    public:
        PpuBus();
        void write(Address addr, Byte value);
        Byte read(Address addr);
        Byte readPalette(Byte paletteAddr);
        
        bool setMapper(Mapper* mapper);
        void updateMirroring();

    private:
        std::vector<Byte> m_RAM;
        std::vector<Byte> m_palette;
        Mapper* m_mapper;
        Address NameTable0,NameTable1,NameTable2,NameTable3;
    };
} // namespace yn
#endif // __PPUBUS_H__