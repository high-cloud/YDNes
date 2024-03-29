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
        if (!m_cpuBus.setReadCallback(IORegister::PPUSTATUS, [&](void)
                                      { return m_ppu.getStatus(); }) ||
            !m_cpuBus.setReadCallback(IORegister::PPUDATA, [&](void)
                                      { return m_ppu.getData(); }) ||
            !m_cpuBus.setReadCallback(IORegister::JOY1, [&](void)
                                      { return m_controller1.read(); }) ||
            !m_cpuBus.setReadCallback(IORegister::JOY2, [&](void)
                                      { return m_controller2.read(); }) ||
            !m_cpuBus.setReadCallback(IORegister::OAMDATA, [&](void)
                                      { return m_ppu.getOAMData(); }))
        {
            LOG(Error) << "Critical error: failed to set IO read callbacks" << std::endl;
        }

        if (!m_cpuBus.setWriteCallback(IORegister::PPUCTRL, [&](Byte b)
                                       { m_ppu.control(b); }) ||
            !m_cpuBus.setWriteCallback(IORegister::PPUADDR, [&](Byte b)
                                       { m_ppu.setDataAddress(b); }) ||
            !m_cpuBus.setWriteCallback(IORegister::PPUMASK, [&](Byte b)
                                       { m_ppu.setMask(b); }) ||
            !m_cpuBus.setWriteCallback(IORegister::PPUSCROL, [&](Byte b)
                                       { m_ppu.setScroll(b); }) ||
            !m_cpuBus.setWriteCallback(IORegister::PPUDATA, [&](Byte b)
                                       { m_ppu.setData(b); }) ||
            !m_cpuBus.setWriteCallback(IORegister::OAMDATA, [&](Byte b)
                                       { m_ppu.setOAMData(b); }) ||
            !m_cpuBus.setWriteCallback(IORegister::OAMADDR, [&](Byte b)
                                       { m_ppu.setOAMAddress(b); }) ||
            !m_cpuBus.setWriteCallback(IORegister::JOY1, [&](Byte b)
                                       {
                                           m_controller1.write(b);
                                           m_controller2.write(b);
                                       }) ||
            !m_cpuBus.setWriteCallback(IORegister::OAMDMA, [&](Byte b)
                                       { DMA(b); }))
        {
            LOG(Error) << "Critical error: failed to set IO write callbacks" << std::endl;
        }

        m_ppu.setCallback([&](void)
                          { m_cpu.interrupt(Cpu::NMI); });
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
            LOG(Error) << " set Mapper failed." << std::endl;
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

        m_cycleTimer = std::chrono::high_resolution_clock::now();
        m_elapsedTime = m_cycleTimer - m_cycleTimer;

        sf::Event event;
        bool focus = true, pause = false;

        while (m_window.isOpen())
        {
            // if (m_cpu.getCycle() > 2000000)
            // {
            //     Log::get().setLevel(CpuTrace);
            // }
            // if (m_cpu.getCycle() > 3000000)
            // {
            //     Log::get().setLevel(Info);
            // }

            while (m_window.pollEvent(event))
            {

                if (event.type == sf::Event::Closed ||
                    (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
                {
                    m_window.close();
                    return;
                }
                else if (event.type == sf::Event::GainedFocus)
                {
                    focus = true;
                    m_cycleTimer = std::chrono::high_resolution_clock::now();
                }
                else if (event.type == sf::Event::LostFocus)
                    focus = false;
                else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F2)
                {
                    pause = !pause;
                    if (!pause)
                        m_cycleTimer = std::chrono::high_resolution_clock::now();
                }
                else if (pause && event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::F3)
                {
                    for (int i = 0; i < 29781; ++i) //Around one frame
                    {
                        //PPU
                        m_ppu.step();
                        m_ppu.step();
                        m_ppu.step();
                        //CPU
                        m_cpu.step();
                    }
                }
            }

            if (focus && !pause)
            {
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
            else
            {
                sf::sleep(sf::milliseconds(1000 / 60));
                //std::this_thread::sleep_for(std::chrono::milliseconds(1000/60)); //1/60 second
            }
            // while (m_window.pollEvent(event))
            // {
            //     if (event.type == sf::Event::Closed ||
            //         (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
            //     {
            //         m_window.close();
            //         return;
            //     }
            // }

            // m_elapsedTime += std::chrono::high_resolution_clock::now() - m_cycleTimer;
            // m_cycleTimer = std::chrono::high_resolution_clock::now();

            // while (m_elapsedTime > m_cpuCycleDuration)
            // {

            //     //PPU
            //     m_ppu.step();
            //     m_ppu.step();
            //     m_ppu.step();
            //     //CPU
            //     m_cpu.step();

            //     m_elapsedTime -= m_cpuCycleDuration;
            // }

            // m_window.draw(m_screen.vertices());
            // m_window.display();
        }
    }

    void Emulator::setKeyBindings(std::vector<sf::Keyboard::Key> &keys1, std::vector<sf::Keyboard::Key> &keys2)
    {
        m_controller1.setKeyBindings(keys1);
        m_controller2.setKeyBindings(keys2);
    }
} // namespace yn