#include "mapper.h"
#include "mapperNROM.h"

namespace yn
{
    Mapper::Mapper(Cartridge cartridge, Type type)
        : m_cartrige(cartridge), m_type(type)
    {
    }

    std::unique_ptr<Mapper> Mapper::createMapper(Type type, Cartridge cartridge)
    {
        std::unique_ptr<Mapper> ret(nullptr);
        if (type == Type::NROM)
        {
            ret.reset(new MapperNROM(cartridge));
        }
        return ret;
    }

} // namespace yn