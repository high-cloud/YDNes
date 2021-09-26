#include "Log.h"
#include "constant.h"
#include <fstream>
#include "emulator.h"
#include <vector>
#include <SFML/Window.hpp>

using namespace yn;

int main()
{
    std::ofstream logFile("ydNes.log", std::ios::trunc | std::ios::out);
    TeeStream logTee(std::cout, logFile);

    if (logFile.is_open() && logFile.good())
    {
        Log::get().setLogStream(logTee);
    }
    else
    {
        Log::get().setLogStream(std::cout);
    }
    Log::get().setLevel(Info);

    std::ofstream cpuFile("cpuTrace.log", std::ios::trunc | std::ios::out);
    if (cpuFile.is_open() && cpuFile.good())
    {
        Log::get().setTraceStream(cpuFile);
        // Log::get().setLevel(CpuTrace);
    }


    std::vector<sf::Keyboard::Key> p1{sf::Keyboard::J, sf::Keyboard::K, sf::Keyboard::RShift, sf::Keyboard::Return,
                                      sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D},
        p2{sf::Keyboard::Numpad5, sf::Keyboard::Numpad6, sf::Keyboard::Numpad8, sf::Keyboard::Numpad9,
           sf::Keyboard::Up, sf::Keyboard::Down, sf::Keyboard::Left, sf::Keyboard::Right};

    Emulator emulator;
    emulator.setKeyBindings(p1, p2);
    
    emulator.run("E:/cpp_project/YDNes/smb.nes");

    return 0;
}