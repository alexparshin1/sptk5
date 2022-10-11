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

#include <sptk5/Brotli.h>
#include <sptk5/Exception.h>

#include <gtest/gtest.h>
#include <sptk5/Base64.h>
#include <sptk5/StopWatch.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

static const String originalTestString = "This is a test of compression using Brotli algorithm";

#ifdef _WIN32
static const String originalTestStringBase64 = "H4sIAAAAAAAACwvJyCxWAKJEhZLU4hKF/DSF5PzcgqLU4uLM/DyF0uLMvHQF96jMAoXEnPT8osySjFwAes7C0zIAAAA=";
#else
static const String originalTestStringBase64 = "oZgBACBuY+u6dus1GkIllLABJwCJBp6OOGyMnS2IO2hX4F/C9cYbegYltUixcXITXgA=";
#endif

TEST(SPTK_Brotli, compress)
{
    Buffer compressed;
    String compressedBase64;
    Brotli::compress(compressed, Buffer(originalTestString));
    Base64::encode(compressedBase64, compressed);

    EXPECT_STREQ(originalTestStringBase64.c_str(), compressedBase64.c_str());
}

TEST(SPTK_Brotli, decompress)
{
    Buffer compressed;
    Buffer decompressed;

    Base64::decode(compressed, originalTestStringBase64);
    Brotli::decompress(decompressed, compressed);

    EXPECT_STREQ(originalTestString.c_str(), decompressed.c_str());
}

TEST(SPTK_Brotli, performance)
{
    Buffer data;
    Buffer compressed;
    Buffer decompressed;

    // Using uncompressed mplayer manual as test data
    data.loadFromFile(String(TEST_DIRECTORY) + "/data/mplayer.1");
    EXPECT_EQ(data.bytes(), size_t(345517));

    StopWatch stopWatch;
    stopWatch.start();
    Brotli::compress(compressed, data);
    stopWatch.stop();

    constexpr auto megabyte = double(1024 * 1024);

    COUT("Brotli compressor:" << endl)
    COUT("Compressed " << data.bytes() << " bytes to " << compressed.bytes() << " bytes for "
                       << stopWatch.seconds() << " seconds (" << data.bytes() / stopWatch.seconds() / megabyte << " Mb/s)"
                       << endl)

    stopWatch.start();
    Brotli::decompress(decompressed, compressed);
    stopWatch.stop();

    COUT("Decompressed " << compressed.bytes() << " bytes to " << decompressed.bytes() << " bytes for "
                         << stopWatch.seconds() << " seconds (" << decompressed.bytes() / stopWatch.seconds() / megabyte
                         << " Mb/s)" << endl)

    EXPECT_STREQ(data.c_str(), decompressed.c_str());
}
