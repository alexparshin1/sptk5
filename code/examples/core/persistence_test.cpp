#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <chrono>
#include <sptk5/persist/MemoryBucket.h>

using namespace std;
using namespace sptk;
using namespace persist;
using namespace chrono;

int main()
{
    try {
#ifdef _WIN32
        const char* fileName = "C:\\tmp\\mmfile.1";
#else
        const char* fileName = "/tmp/mmfile.1";
#endif

        size_t maxMessages = 1024 * 1024;

        MemoryBucket bucket(fileName, "test", 0, 128 * maxMessages); // Message size is 100 bytes, plus 24 bytes of storage header
        bucket.clear();

        steady_clock::time_point started = steady_clock::now();

        size_t i = 0;
        vector<char> buffer(100);
        memcpy(buffer.data(), "Hello, World!", 14);
        while (bucket.available() > buffer.size()) {
            bucket.insert(buffer.data(), buffer.size());
            i++;
        }
        maxMessages = i;

        steady_clock::time_point ended = steady_clock::now();
        double durationSec = duration_cast<milliseconds>(ended - started).count() / 1000.0;
        cout << "Found " << i / 1024 << "K messages: " << int(maxMessages / durationSec) / 1024 << "K msg/sec" << endl;
        
/*
        started = steady_clock::now();
        i = 0;
        for (i = 0; i < maxMessages; i++) {
            void* ptr = bucket.find(i + 1, sz);
            bucket.free(ptr);
        }
        ended = steady_clock::now();
        durationSec = duration_cast<milliseconds>(ended - started).count() / 1000.0;
        cout << "Found " << i / 1024 << "K messages: " << int(maxMessages / durationSec) / 1024 << "K msg/sec" << endl;
*/
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 0;
}