#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <chrono>
#include "PersistentMemoryBucket.h"

using namespace std;
using namespace chrono;

int main()
{
    try {
        PersistentMemoryBucket bucket("C:\\tmp\\mmfile.1", 16 * 1024 * 1024);
        //bucket.clear();

        steady_clock::time_point started = steady_clock::now();

        size_t maxMessages = 1024 * 128; // bucket.size() / 110; // 100 bytes of message + 8 bytes header

        size_t i = 0;
        vector<char> buffer(100);
        memcpy(buffer.data(), "Hello, World!", 14);
        while (bucket.available() > buffer.size()) {
            size_t bucketAvailable = bucket.available();
            bucket.insert(i + 1, buffer.data(), buffer.size());
            i++;
        }
        maxMessages = i;

        steady_clock::time_point ended = steady_clock::now();
        double durationSec = duration_cast<milliseconds>(ended - started).count() / 1000.0;
        cout << "Stored " << i / 1024 << "K messages: " << maxMessages/durationSec << " msg/sec" << endl;
        
        started = steady_clock::now();
        i = 0;
        size_t sz;
        for (i = 0; i < maxMessages; i++) {
            char* ptr = (char*) bucket.find(i + 1, sz);
            strcpy(buffer.data(), ptr);
        }
        ended = steady_clock::now();
        durationSec = duration_cast<milliseconds>(ended - started).count() / 1000.0;
        cout << "Found " << i / 1024 << "K messages: " << maxMessages / durationSec << " msg/sec" << endl;
/*
        started = steady_clock::now();
        i = 0;
        for (i = 0; i < maxMessages; i++) {
            void* ptr = bucket.find(i + 1, sz);
            bucket.free(ptr);
        }
        ended = steady_clock::now();
        durationSec = duration_cast<milliseconds>(ended - started).count() / 1000.0;
        cout << "Found and erased " << i / 1024 << "K messages: " << maxMessages / durationSec << " msg/sec" << endl;
*/
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 0;
}