#include <stdlib.h>
#include <stdio.h>
#include <sptk5/sptk.h>

#ifdef _WIN32
    #include <io.h>
#endif

#include <sptk5/cutils>

#ifndef O_BINARY
# define O_BINARY 0
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

        printf("static size_t %s_len = %ld;\n", argv[2], dataSize);
        printf("static unsigned char %s[%ld] = {\n", argv[2], dataSize);

        auto x = (unsigned char *)data.data();

        for (unsigned i = 0; i < dataSize; i++) {
            printf("0x%02x, ", (unsigned) x[i]);
            if (!(i & 0xf)) printf("\n");
        }
        printf("};\n");

        return 0;
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}

