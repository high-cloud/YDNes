#include "Log.h"
#include "constant.h"
#include <fstream>
#include "emulator.h"

using namespace yn;

int main()
{
    std::ofstream logFile("ydNes.log", std::ios::trunc | std::ios::out);
    TeeStream logTee(std::cout, logFile);
    std::ofstream cpuFile("cpuTrace.log", std::ios::trunc | std::ios::out);

    if (logFile.is_open() && logFile.good())
    {
        Log::get().setLogStream(logTee);
    }
    else
    {
        Log::get().setLogStream(std::cout);
    }

    if (cpuFile.is_open() && cpuFile.good())
    {
        Log::get().setTraceStream(cpuFile);
    }

    Log::get().setLevel(CpuTrace);

    LOG(Info) << "cann't do anything" << std::endl;

    Emulator emulator;
    emulator.run("D:/cpp_project/YDNes/nestest.nes");

    return 0;
}