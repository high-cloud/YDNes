#include"Log.h"
#include<fstream>

using namespace yn;

int main(){
    std::ofstream logFile("ydNes.log");
    TeeStream logTee(std::cout,logFile);

    if(logFile.is_open() && logFile.good())
    {
        Log::get().setLogStream(logTee);
    }
    else
    {
        Log::get().setLogStream(std::cout);
    }

    Log::get().setLevel(Info);

    LOG(Info)<<"cann't do anything"<<std::endl;
    
    return 0;
}