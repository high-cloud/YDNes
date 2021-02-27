#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include"ppu.h"
#include"ppuBus.h"

#include"sfmlScreen.h"

#include "cpu.h"
#include "cartridge.h"
#include "mapper.h"
#include <string>
#include <memory>
#include<sfml/Window.hpp>

#include<chrono>

namespace yn
{
    using TimePoint=std::chrono::high_resolution_clock::time_point;
    class Emulator
    {
        const int NESwidth=ScanlineVisibleDots;
        const int NESheight=VisibleScanlines;

    private:
        Cpu m_cpu;
        Cartridge m_cart;
        CpuBus m_cpuBus;

        PPU m_ppu;
        PpuBus m_ppuBus;
        SFMLScreen m_screen;
        std::unique_ptr<Mapper> m_mapper;

        sf::RenderWindow m_window;
        float m_pixelSize;

        //clock relate
        TimePoint m_cycleTimer;
        std::chrono::high_resolution_clock::duration m_elapsedTime;
        std::chrono::nanoseconds m_cpuCycleDuration;


        void DMA(Byte page);

    public:
        Emulator();
        void run(std::string path);
    };
} // namespace yn
#endif // __EMULATOR_H__