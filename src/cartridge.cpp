#include "cartridge.h"
#include "Log.h"
#include "constant.h"
#include <fstream>
#include <iostream>
#include <string>

namespace yn
{

    Cartridge::Cartridge() : m_mapperNumber(0),
                             m_nameTableMirroring(0),
                             m_extendedRAM(false)
    {
    }

    bool Cartridge::loadFromFile(std::string path)
    {
        std::ifstream romFile(path, std::ios::binary | std::ios::in);
        if (!romFile)
        {
            LOG(Error) << "can't open rom from: " << path << std::endl;
            return false;
        }

        std::vector<Byte> header;
        LOG(Info) << "read rom from: " << path << std::endl;

        header.resize(16);
        if (!romFile.read(reinterpret_cast<char *>(&header[0]), 16))
        {
            LOG(Error) << "can't read header from rom" << std::endl;
            return false;
        }
        if (std::string{&header[0], &header[4]} != "NES\x1A")
        {
            LOG(Error) << "Not valid iNES image. Magic Number:"
                       << std::hex << header[0] << " " << header[1] << " " << header[2] << " " << header[3] << std::endl
                       << "Valid Number : N E S 1a" << std::endl;
            return false;
        }

        Byte numPGR = header[4];
        if (!numPGR)
        {
            LOG(Error) << "ROM has no PGR-ROM. Failed to load ROM" << std::endl;
            return false;
        }
        LOG(Info) << "16KB PGR-ROM Banks: " << +numPGR << std::endl;

        Byte numCHR = header[5];
        LOG(Info) << "8KB CHR-ROM Banks: " << +numCHR << std::endl;

        m_nameTableMirroring = header[6] & 0x9;
        LOG(Info) << "Name Table Mirroring: " << +m_nameTableMirroring << std::endl;

        m_mapperNumber = ((header[6] >> 4) & 0xF) | (header[7] & 0xf0);
        LOG(Info) << "Mapper number: " << std::hex << +m_mapperNumber << std::endl;

        m_extendedRAM = header[6] & 0x2;
        LOG(Info) << "Extended RAM: " << std::boolalpha << m_extendedRAM << std::endl;

        if (header[6] & 0x4)
        {
            LOG(Error) << "failed. Haven't support Trainer." << std::endl;
            return false;
        }

        if (header[10] & 0x2)
        {
            LOG(Error) << "failed. Haven't support PAL." << std::endl;
            return false;
        }

        // read PGR-ROM
        m_PGR_ROM.resize(0x4000 * numPGR);
        if (!romFile.read(reinterpret_cast<char *>(&m_PGR_ROM[0]), numPGR * 0x4000))
        {
            LOG(Error) << "failed to read PGR-ROM." << std::endl;
            return false;
        }

        //read 8KB CHR-ROM
        if (numCHR)
        {
            m_chrRAM = false;
            m_CHR_ROM.resize(numCHR * 0x2000);
            if (!romFile.read(reinterpret_cast<char *>(&m_CHR_ROM[0]), numCHR * 0x2000))
            {
                LOG(Error) << "failed to read CHR-ROM." << std::endl;
                return false;
            }
        }
        else
        {
            LOG(Info) << "Cartridge with CHR-RAM." << std::endl;
        }

        return true;
    }

    bool Cartridge::hasExtendedRAM()
    {
        return m_extendedRAM;
    }

    const std::vector<Byte> &Cartridge::getROM()
    {
        return m_PGR_ROM;
    }
    const std::vector<Byte> &Cartridge::getVROM()
    {
        return m_CHR_ROM;
    }

    Byte Cartridge::getMapperNumber()
    {
        return m_mapperNumber;
    }

    Byte Cartridge::getNameTableMirroring()
    {
        return m_nameTableMirroring;
    }

} // namespace yn