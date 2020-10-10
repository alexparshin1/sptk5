#include <stdio.h>
#include <sptk5/sptk.h>

#ifdef _WIN32
#include <io.h>
#endif

#include <sptk5/cutils>

#ifndef O_BINARY
#define O_BINARY 0
#endif

using namespace std;
using namespace sptk;

int main(int argc, char **argv)
{
    try {
        if (argc != 3) {
            fprintf(stderr, "Usage: tar2h <input file> valuename.\n");
            return 1;
        }

        Buffer data;
        data.loadFromFile(argv[1]);

        size_t dataSize = data.bytes();

        COUT("static size_t " << argv[2] << "_len = " << dataSize << ";" << endl)
        COUT("static unsigned char " << argv[2] << "[" << dataSize << "] = {" << endl)

        const auto* x = (const unsigned char *)data.c_str();

        stringstream str;
        str.fill('0');
        for (unsigned i = 0; i < dataSize; ++i) {
            str << "0x" << hex << setw(2) << ", " << (unsigned) x[i];
            if (!(i & 0xf)) str << endl;
        }
        str << "};" << endl;
        COUT(str.str())

        return 0;
    }
    catch (const Exception& e) {
        CERR(e.what() << endl)
        return 1;
    }
}

