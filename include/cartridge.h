#ifndef __Cartridge_H__
#define __Cartridge_H__

#include <vector>
#include "constant.h"
#include <string>

namespace yn
{

    class Cartridge
    {
    private:
        Byte m_mapperNumber;
        Byte m_nameTableMirroring;
        std::vector<Byte> m_PGR_ROM;
        std::vector<Byte> m_CHR_ROM;
        bool m_extendedRAM;
        bool m_chrRAM;

    public:
        Cartridge();
        bool loadFromFile(std::string path);
        bool hasExtendedRAM();
        const std::vector<Byte> &getROM();
        const std::vector<Byte> &getVROM();
        Byte getMapperNumber();
        Byte getNameTableMirroring();
    };

} // namespace yn
#endif // __Cartridge_H__