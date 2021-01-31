#include "Log.h"
#include "constant.h"
#include <fstream>
#include"cartridge.h"

using namespace yn;

int main(int argc,char** argv)
{
    std::ofstream logFile("ydNes.log");
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

    LOG(Info) << "cann't do anything" << std::endl;

    if(argc>1)
    {
        Cartridge rom;
        rom.loadFromFile(argv[1]);
    }
    else
    {
        LOG(Info)<<"nothing happened."<<std::endl;
    }

    return 0;
}