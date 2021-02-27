#include "emulator.h"
#include "Log.h"
#include <chrono>

namespace yn
{
    void Emulator::DMA(Byte page)
    {
        m_cpu.skipDMACycles();
        auto ptr = m_cpuBus.getPagePtr(page);
        m_ppu.doDMA(ptr);
    }

    Emulator::Emulator() : m_cpu(m_cpuBus),
                           m_ppu(m_ppuBus, m_screen),
                           m_pixelSize(2.0f),
                           m_cycleTimer(),
                           m_cpuCycleDuration(std::chrono::nanoseconds(559))
    {
        if (!m_cpuBus.setReadCallback(IORegister::PPUSTATUS, [&](void) { return m_ppu.getStatus(); }) ||
            !m_cpuBus.setReadCallback(IORegister::PPUDATA, [&](void) { return m_ppu.getData(); }) ||
            !m_cpuBus.setReadCallback(IORegister::OAMDATA, [&](void) { return m_ppu.getOAMData(); }))
        {
            LOG(Error) << "Critical error: failed to set IO read callbacks" << std::endl;
        }

        if (!m_cpuBus.setWriteCallback(IORegister::PPUCTRL, [&](Byte b) { m_ppu.control(b); }) ||
            !m_cpuBus.setWriteCallback(IORegister::PPUADDR, [&](Byte b) { m_ppu.setDataAddress(b); }) ||
            !m_cpuBus.setWriteCallback(IORegister::PPUMASK, [&](Byte b) { m_ppu.setMask(b); }) ||
            !m_cpuBus.setWriteCallback(IORegister::PPUSCROL, [&](Byte b) { m_ppu.setScroll(b); }) ||
            !m_cpuBus.setWriteCallback(IORegister::PPUDATA, [&](Byte b) { m_ppu.setData(b); }) ||
            !m_cpuBus.setWriteCallback(IORegister::OAMDATA, [&](Byte b) { m_ppu.setOAMData(b); }) ||
            !m_cpuBus.setWriteCallback(IORegister::OAMADDR, [&](Byte b) { m_ppu.setOAMAddress(b); }) ||
            !m_cpuBus.setWriteCallback(IORegister::OAMDMA, [&](Byte b) { DMA(b); }))
        {
            LOG(Error) << "Critical error: failed to set IO write callbacks" << std::endl;
        }

        m_ppu.setCallback([&](void) { m_cpu.interrupt(Cpu::NMI); });
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

        if (!m_ppuBus.setMapper(m_mapper.get()))
        {
            return;
        }

        m_cpu.reset();
        m_ppu.reset();

        m_window.create(sf::VideoMode(NESwidth * m_pixelSize, NESheight * m_pixelSize), "YDNES", sf::Style::Titlebar | sf::Style::Close);
        m_window.setVerticalSyncEnabled(true);
        m_screen.create(NESwidth, NESheight, m_pixelSize, sf::Color::White.toInteger());

        m_cycleTimer=std::chrono::high_resolution_clock::now();
        m_elapsedTime=m_cycleTimer-m_cycleTimer;

        sf::Event event;
        while (m_window.isOpen())
        {
            while (m_window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed ||
                    (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
                {
                    m_window.close();
                    return;
                }
            }

            m_elapsedTime += std::chrono::high_resolution_clock::now() - m_cycleTimer;
            m_cycleTimer = std::chrono::high_resolution_clock::now();

            while (m_elapsedTime > m_cpuCycleDuration)
            {

                //PPU
                m_ppu.step();
                m_ppu.step();
                m_ppu.step();
                //CPU
                m_cpu.step();

                m_elapsedTime -= m_cpuCycleDuration;
            }

            m_window.draw(m_screen.vertices());
            m_window.display();
        }
    }
} // namespace yn