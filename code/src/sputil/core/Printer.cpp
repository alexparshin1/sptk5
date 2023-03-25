#include <sptk5/Printer.h>

std::mutex& sptk::Console::printMutex()
{
    static std::mutex printMutex;
    return printMutex;
}
