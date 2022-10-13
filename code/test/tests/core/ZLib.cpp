/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include "zlib.h"
#include <sptk5/Exception.h>
#include <sptk5/ZLib.h>

#include <gtest/gtest.h>
#include <sptk5/Base64.h>
#include <sptk5/StopWatch.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

static const String originalTestString = "This is a test of compression using GZip algorithm";

// Note: ZLib under Windows produces slightly different result,
// due old Windows port version.
#ifdef _WIN32
static const String originalTestStringBase64 = "H4sIAAAAAAAACwvJyCxWAKJEhZLU4hKF/DSF5PzcgqLU4uLM/DyF0uLMvHQF96jMAoXEnPT8osySjFwAes7C0zIAAAA=";
#else
static const String originalTestStringBase64 = "H4sIAAAAAAAAAwvJyCxWAKJEhZLU4hKF/DSF5PzcgqLU4uLM/DyF0uLMvHQF96jMAoXEnPT8osySjFwAes7C0zIAAAA=";
#endif

TEST(SPTK_ZLib, compressDecompress)
{
    Buffer compressed;
    Buffer decompressed;
    ZLib::compress(compressed, Buffer(originalTestString));
    ZLib::decompress(decompressed, compressed);

    EXPECT_STREQ(originalTestString.c_str(), decompressed.c_str());
}

TEST(SPTK_ZLib, compress)
{
    Buffer compressed;
    String compressedBase64;
    ZLib::compress(compressed, Buffer(originalTestString));
    Base64::encode(compressedBase64, compressed);

    compressed.saveToFile("/tmp/00.gz");

    EXPECT_STREQ(originalTestStringBase64.c_str(), compressedBase64.c_str());
}

TEST(SPTK_ZLib, decompress)
{
    Buffer compressed;
    Buffer decompressed;

    Base64::decode(compressed, originalTestStringBase64);
    ZLib::decompress(decompressed, compressed);

    EXPECT_STREQ(originalTestString.c_str(), decompressed.c_str());
}

TEST(SPTK_ZLib, performance)
{
    Buffer data;
    Buffer compressed;
    Buffer decompressed;

    // Using uncompressed mplayer manual as test data
    data.loadFromFile(String(TEST_DIRECTORY) + "/data/mplayer.1");
    EXPECT_EQ(data.bytes(), size_t(345517));

    StopWatch stopWatch;
    stopWatch.start();
    ZLib::compress(compressed, data);
    stopWatch.stop();

    constexpr auto bytesInMB = 1E6;
    COUT("ZLib compressor:" << endl)
    COUT("Compressed " << data.bytes() << " bytes to " << compressed.bytes() << " bytes for "
                       << stopWatch.seconds() << " seconds (" << data.bytes() / stopWatch.seconds() / bytesInMB << " Mb/s)"
                       << endl)

    stopWatch.start();
    ZLib::decompress(decompressed, compressed);
    stopWatch.stop();

    COUT("Decompressed " << compressed.bytes() << " bytes to " << decompressed.bytes() << " bytes for "
                         << stopWatch.seconds() << " seconds (" << decompressed.bytes() / stopWatch.seconds() / bytesInMB
                         << " Mb/s)" << endl)

    EXPECT_STREQ(data.c_str(), decompressed.c_str());
}
