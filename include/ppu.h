#ifndef __PPU_H__
#define __PPU_H__

#include "ppuBus.h"
#include"screen.h"
#include <vector>
#include "constant.h"
#include<functional>

namespace yn
{
    const int ScanlineCycleLength = 341;
    const int ScanlineEndCycle = 340;
    const int VisibleScanlines = 240;
    const int ScanlineVisibleDots = 256;
    const int FrameEndScanline = 261;

    const int AttributeOffset = 0x3C0;

    class PPU
    {
    public:
        PPU(PpuBus &bus,Screen &screen);
        void step();
        void reset();

        void doDMA(const Byte *page_ptr);

        void control(Byte ctrl);
        void setMask(Byte mask);
        void setOAMAddress(Byte addr);
        void setOAMData(Byte value);
        void setDataAddress(Byte addr);
        void setScroll(Byte scroll);
        void setData(Byte data);

        // read
        Byte getStatus();
        Byte getData();
        Byte getOAMData();

    private:
        Byte read(Address addr);
        PpuBus &m_bus;
        Screen &m_screen;
        std::vector<Byte> m_spriteMemory;
        std::vector<Byte> m_scanlineSprites;
        std::function<void(void)> m_vblankCallback;

        // status
        bool m_vbank;
        bool m_spZreoHit;
        bool m_spOverFlow;
        bool m_evenFrame;
        int m_cycle;
        int m_scanline;

        enum class State
        {
            PreRender,
            Render,
            PostRender,
            VerticalBlank,
        } m_pipelineState;

        //registers
        Address m_dataAddress;
        Address m_tempAddress;
        Byte m_fineXScroll;
        bool m_firstWrite; // used to indicate upper or lower byte to data address
        Byte m_dataBuffer;

        Byte m_spriteDataAddress;

        // control flags
        bool m_generateInterrupt;
        bool m_longSprites;
        bool m_bgPageHigh;
        bool m_spPageHigh;
        Address m_dataAddrIncrement;

        // mask flags
        bool m_greyscaleMode;
        bool m_showSprites;
        bool m_showBackground;
        bool m_hideEdgeSprites;
        bool m_hideEdgeBackground;

        std::vector<std::vector<uint32_t>> m_pictureBuffer;
    };

}
#endif // __PPU_H__