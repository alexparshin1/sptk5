/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/Exception.h>
#include <sptk5/ZLib.h>

#include <gtest/gtest.h>
#include <sptk5/Base64.h>
#include <sptk5/StopWatch.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

static const String originalTestString = 
    "This is a test of compression using GZip algorithm. "
    "This is a test of compression using GZip algorithm. "
    "This is a test of compression using GZip algorithm. ";

TEST(SPTK_ZLib, compressDecompress)
{
    Buffer compressed;
    Buffer decompressed;
    ZLib::compress(compressed, Buffer(originalTestString));
    EXPECT_LT(compressed.bytes(), originalTestString.length());
    ZLib::decompress(decompressed, compressed);

    EXPECT_STREQ(originalTestString.c_str(), decompressed.c_str());
}

TEST(SPTK_ZLib, performance)
{
    Buffer data;
    Buffer compressed;
    Buffer decompressed;

    // Using own executable file for the test.
#ifdef _WIN32
    const filesystem::path testFile {"C:/Program Files/SPTK/bin/sptk_unit_tests.exe"};
#else
    const filesystem::path testFile {"/usr/local/bin/sptk_unit_tests"};
#endif
    if (!filesystem::exists(testFile))
    {
        GTEST_SKIP() << "Test file not found: " << testFile;
    }
    constexpr auto testDataSize = 1024 * 1024;
    data.loadFromFile(testFile);
    data.bytes(testDataSize);

    StopWatch stopWatch;
    stopWatch.start();
    ZLib::compress(compressed, data);
    stopWatch.stop();

    constexpr auto bytesInMB = 1E6;
    COUT("ZLib compressor:");
    COUT("Compressed " << data.bytes() << " bytes to " << compressed.bytes() << " bytes for "
                       << stopWatch.seconds() << " seconds (" << data.bytes() / stopWatch.seconds() / bytesInMB << " Mb/s)"
                       << endl);

    stopWatch.start();
    ZLib::decompress(decompressed, compressed);
    stopWatch.stop();

    COUT("Decompressed " << compressed.bytes() << " bytes to " << decompressed.bytes() << " bytes for "
                         << stopWatch.seconds() << " seconds (" << decompressed.bytes() / stopWatch.seconds() / bytesInMB
                         << " Mb/s)" << endl);

    EXPECT_STREQ(data.c_str(), decompressed.c_str());
}
