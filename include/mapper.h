#ifndef __MAPPER_H__
#define __MAPPER_H__

#include "cartridge.h"
#include <memory>
namespace yn
{

    enum class NameTableMirroring
    {
        Horizontal=0,
        Vertical=1,
        FourScreen=8,
        OneScreenLower,
        OneScreenHigher
    };

    class Mapper
    {
    public:
        enum class Type
        {
            NROM = 0,

        };
        Mapper(Cartridge& , Type);
        virtual Byte read(Address) = 0;
        virtual void write(Address addr, Byte value) = 0;
        virtual bool ifVRAM() = 0;
        virtual NameTableMirroring getNameTableMirroring();
        static std::unique_ptr<Mapper> createMapper(Type type, Cartridge);
        
    protected:
        Cartridge m_cartrige;
        Type m_type;
    };
} // namespace yn
#endif // __MAPPER_H__