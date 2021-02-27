#include "ppu.h"
#include "paletteColor.h"
#include "Log.h"
#include <array>

namespace yn
{
    PPU::PPU(PpuBus &bus, Screen &screen) : m_bus(bus),
                                            m_screen(screen),
                                            m_spriteMemory(64 * 4),
                                            m_pictureBuffer(ScanlineVisibleDots, std::vector<uint32_t>(VisibleScanlines, 0x800080ff))
    {
    }

    void PPU::step()
    {
        switch (m_pipelineState)
        {
        case State::PreRender:
            if (m_cycle == 1)
                m_vblank = m_spZreoHit = false;
            else if (m_cycle == ScanlineVisibleDots + 2 && m_showBackground && m_showSprites)
            {
                //set bits related to horizontal position
                m_dataAddress &= ~0x041f;
                m_dataAddress |= m_tempAddress & 0x041f;
            }
            else if (m_cycle > 280 && m_cycle <= 304 && m_showSprites && m_showBackground)
            {
                //set vertical bits
                m_dataAddress &= ~0x7be0;
                m_dataAddress |= m_tempAddress & 0x7be0;
            }

            if (m_cycle >= ScanlineEndCycle - (!m_evenFrame && m_showBackground && m_showSprites))
            {
                m_pipelineState = State::Render;
                m_cycle = m_scanline = 0;
            }
            break;
        case State::Render:
            if (m_cycle > 0 && m_cycle <= ScanlineVisibleDots)
            {
                Byte bgColor = 0, sprColor = 0;
                bool bgOpaque = false, sprOpaque = true;
                bool spriteForeGround = false;

                int x = m_cycle - 1;
                int y = m_scanline;

                if (m_showBackground)
                {
                    auto x_fine = (m_fineXScroll + x) % 8;
                    if (!m_hideEdgeBackground || x >= 8)
                    {
                        //fetch tile
                        auto addr = 0x2000 | (m_dataAddress & 0xfff);
                        Byte tile = read(addr);

                        //fetch pattern
                        // each pattern occupies 16 bytes, so multiply by 16
                        addr = (tile * 16) + ((m_dataAddress >> 12) & 0x7);
                        addr |= m_bgPageHigh << 12; // whether the pattern is in the high or low page
                        // get the bit
                        bgColor = (read(addr) >> (7 ^ x_fine)) & 1;             //bit 0
                        bgColor |= ((read(addr + 8) >> (7 ^ x_fine)) & 1) << 1; //bit 1

                        bgOpaque = bgColor;

                        //fetch attribute and calculate higher two bits
                        addr = 0x23c0 | (m_dataAddress & 0x0c00) | ((m_dataAddress >> 4) & 0x38) | ((m_dataAddress >> 2) & 0x07);
                        auto attribute = read(addr);
                        int shift = ((m_dataAddress >> 4) & 4) | (m_dataAddress & 2);
                        bgColor |= ((attribute >> shift) & 0x3) << 2;
                    }
                    //increment coarse X
                    if (x_fine == 7)
                    {
                        if ((m_dataAddress & 0x001f) == 31) // coarse X ==31
                        {
                            m_dataAddress &= ~0x001f;
                            m_dataAddress ^= 0x0400;
                        }
                        else
                            m_dataAddress += 1;
                    }
                }

                if (m_showSprites && ((!m_hideEdgeSprites) || x >= 8))
                {
                    for (auto i : m_scanlineSprites)
                    {
                        Byte spr_x = m_spriteMemory[4 * i + 3];

                        if (0 > x - spr_x || x - spr_x >= 8)
                            continue;

                        Byte spr_y = m_spriteMemory[4 * i + 0] + 1,
                             tile = m_spriteMemory[4 * i + 1],
                             attribute = m_spriteMemory[i * 4 + 2];

                        int length = (m_longSprites) ? 16 : 8;

                        int x_shift = (x - spr_x) % 8,
                            y_offset = (y - spr_y) % length;

                        if ((attribute & 0x40) == 0) //if not flipping horizontally
                            x_shift ^= 7;
                        if ((attribute & 80) != 0)
                            y_offset ^= length - 1;

                        Address addr = 0;

                        if (!m_longSprites)
                        {
                            addr = tile * 16 + y_offset;
                            if (m_spPageHigh)
                                addr += 0x1000;
                        }
                        else //long sprites
                        {
                            y_offset = (y_offset & 7) | ((y_offset & 8) << 1);
                            addr = (tile >> 1) * 32 + y_offset;
                            addr |= (tile & 1) << 12;
                        }

                        sprColor = (read(addr) >> (x_shift)) & 1;             //bit 0
                        sprColor |= ((read(addr + 8) >> (x_shift)) & 1) << 1; //bit 1

                        if (!(sprOpaque == sprColor))
                        {
                            sprColor = 0;
                            continue;
                        }

                        sprColor |= 0x10;
                        sprColor |= (attribute & 0x3) << 2; // bits 2-3

                        // sprite 0 detection
                        if (!m_spZreoHit && m_showBackground && i == 0 && sprOpaque && bgOpaque)
                        {
                            m_spZreoHit = true;
                        }

                        break; //exit because already found highest priority
                    }
                }

                Byte paletteAddr = bgColor;

                if ((!bgOpaque && sprOpaque) ||
                    (bgOpaque && sprOpaque && spriteForeGround))
                    paletteAddr = sprColor;
                else if (!bgOpaque && !sprOpaque)
                    paletteAddr = 0;

                m_pictureBuffer[x][y] = colors[m_bus.readPalette(paletteAddr)];
            }
            else if (m_cycle == ScanlineVisibleDots + 1 && m_showBackground)
            {
                // Shamelessly copied
                if ((m_dataAddress & 0x7000) != 0x7000) // if fine 7 <7
                    m_dataAddress += 0x1000;            //increment Y
                else
                {
                    m_dataAddress &= ~0x7000;
                    int y = (m_dataAddress & 0x03e0) >> 5; // let y = coarsey
                    if (y == 29)
                    {
                        y = 0;                   //coarse y=0
                        m_dataAddress ^= 0x0800; //switch vertical nametable
                    }
                    else if (y == 31)
                    {
                        y = 0; //coarse Y =0, nametable not switched
                    }
                    else
                    {
                        ++y;
                    }
                    m_dataAddress = (m_dataAddress & ~0x03e0) | (y << 5); // put coarse Y back into v
                }
            }
            else if (m_cycle == ScanlineVisibleDots + 2&& m_showBackground && m_showSprites)
            {
                //set bits related to horizontal position
                m_dataAddress &= ~0x041f;
                m_dataAddress |= m_tempAddress & 0x041f;
            }

            if (m_cycle >= ScanlineEndCycle)
            {
                // find sprites for next scanline
                m_scanlineSprites.resize(0);

                int range = 8;
                if (m_longSprites)
                {
                    range = 16;
                }

                std::size_t j = 0;
                for (std::size_t i = m_spriteDataAddress / 4; i < 64; ++i)
                {
                    auto diff = m_scanline - m_spriteMemory[i * 4];
                    if (diff >= 0 && diff < range)
                    {
                        m_scanlineSprites.push_back(i);
                        ++j;
                        if (j >= 8)
                        {
                            break;
                        }
                    }
                }

                ++m_scanline;
                m_cycle = 0;
            }

            if (m_scanline >= VisibleScanlines)
                m_pipelineState = State::PostRender;

            break;
        case State::PostRender:
            if (m_cycle >= ScanlineEndCycle)
            {
                ++m_scanline;
                m_cycle = 0;
                m_pipelineState = State::VerticalBlank;

                for (int x = 0; x < m_pictureBuffer.size(); ++x)
                    for (int y = 0; y < m_pictureBuffer[0].size(); ++y)
                    {
                        m_screen.setPixel(x, y, m_pictureBuffer[x][y]);
                    }
                break;
            }
        case State::VerticalBlank:
            if (m_cycle == 1 && m_scanline == VisibleScanlines + 1)
            {
                m_vblank = true;
                if (m_generateInterrupt)
                    m_vblankCallback();
            }

            if (m_cycle >= ScanlineEndCycle)
            {
                ++m_scanline;
                m_cycle = 0;
            }

            if (m_scanline >= FrameEndScanline)
            {
                m_pipelineState = State::PreRender;
                m_scanline = 0;
                m_evenFrame = !m_evenFrame;
            }
            break;
        default:
            LOG(Error) << "oh, this should not happen" << std::endl;
        }

        ++m_cycle;
    }

    void PPU::reset()
    {
        //control
        m_generateInterrupt = m_longSprites = m_bgPageHigh = m_spPageHigh = false;
        m_dataAddrIncrement = 1;
        //mask
        m_greyscaleMode = false;
        m_showBackground = m_showSprites = m_evenFrame = true;
        m_vblank = false;
        m_dataAddress = m_cycle = m_scanline = m_spriteDataAddress = m_fineXScroll = m_tempAddress = 0;
        m_firstWrite = true;

        m_pipelineState = State::PreRender;
        m_scanlineSprites.reserve(8);
        m_scanlineSprites.resize(0);
    }

    void PPU::setCallback(std::function<void(void)> callback)
    {
        m_vblankCallback = callback;
    }

    void PPU::doDMA(const Byte *page_ptr)
    {
        memcpy(m_spriteMemory.data() + m_spriteDataAddress, page_ptr, 256 - m_spriteDataAddress);
        if (m_spriteDataAddress)
        {
            memcpy(m_spriteMemory.data(), page_ptr + (256 - m_spriteDataAddress), m_spriteDataAddress);
        }
    }

    void PPU::control(Byte ctrl)
    {
        m_generateInterrupt = ctrl & 0x80;
        m_longSprites = ctrl & 0x20;
        m_bgPageHigh = ctrl & 0x10;
        m_spPageHigh = ctrl & 0x08;

        if (ctrl & 0x04)
            m_dataAddrIncrement = 0x20;
        else
            m_dataAddrIncrement = 0x1;

        m_tempAddress &= ~0x0c00;
        m_tempAddress |= (ctrl & 0x3) << 10;
    }

    void PPU::setMask(Byte mask)
    {
        m_greyscaleMode = mask & 0x01;
        m_hideEdgeBackground = !(mask & 0x02);
        m_hideEdgeSprites = !(mask & 0x04);
        m_showBackground = mask & 0x08;
        m_showSprites = mask & 0x10;
    }

    void PPU::setOAMAddress(Byte addr)
    {
        m_spriteDataAddress = addr;
    }

    void PPU::setOAMData(Byte value)
    {
        m_spriteMemory[m_spriteDataAddress++] = value;
    }

    void PPU::setDataAddress(Byte addr)
    {
        if (m_firstWrite)
        {
            m_tempAddress &= ~0xff00;
            m_tempAddress |= (addr & 0x3f) << 8;
        }
        else
        {
            m_tempAddress &= 0xff00;
            m_tempAddress |= addr;
            m_dataAddress = m_tempAddress;
        }
        m_firstWrite = !m_firstWrite;
    }

    void PPU::setScroll(Byte scroll)
    {
        if (!m_firstWrite)
        {
            m_tempAddress &= ~0x001f;
            m_tempAddress |= scroll >> 3;
            m_fineXScroll = scroll & 0x07;
        }
        else
        {
            m_tempAddress &= ~0x73e0;
            m_tempAddress |= ((scroll & 0x7) << 12) |
                             ((scroll & 0xf8) << 2);
        }
        m_firstWrite = !m_firstWrite;
    }

    void PPU::setData(Byte data)
    {
        m_bus.write(m_dataAddress, data);
        m_dataAddress += m_dataAddrIncrement;
    }

    Byte PPU::getStatus()
    {
        Byte status = m_spZreoHit << 6 | m_vblank << 7;
        m_vblank = false;
        m_firstWrite = true;
        return status;
    }

    Byte PPU::getData()
    {
        auto data = m_bus.read(m_dataAddress);

        if (m_dataAddress < 0x3f00)
        {
            std::swap(data, m_dataBuffer);
        }
        m_dataAddress += m_dataAddrIncrement;

        return data;
    }

    Byte PPU::getOAMData()
    {
        return m_spriteMemory[m_spriteDataAddress];
    }

    Byte PPU::read(Address addr)
    {
        return m_bus.read(addr);
    }

}