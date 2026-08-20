/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/Exception.h>
#include <sptk5/ZLib.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <sptk5/Base64.h>
#include <sptk5/Stopwatch.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

static const String originalTestString =
    "This is a test of compression using GZip algorithm. "
    "This is a test of compression using GZip algorithm. "
    "This is a test of compression using GZip algorithm. ";
namespace sptk {

TEST(ZLibTests, compressDecompress)
{
    Buffer compressed;
    Buffer decompressed;
    ZLib::compress(compressed, Buffer(originalTestString));
    EXPECT_LT(compressed.bytes(), originalTestString.length());
    ZLib::decompress(decompressed, compressed);

    EXPECT_STREQ(originalTestString.c_str(), decompressed.c_str());
}

TEST(ZLibTests, decompressEmptyInputThrows)
{
    Buffer       decompressed;
    const Buffer emptyInput;
    EXPECT_THROW(ZLib::decompress(decompressed, emptyInput), Exception);
}

TEST(ZLibTests, decompressTruncatedInputThrows)
{
    Buffer compressed;
    Buffer decompressed;
    ZLib::compress(compressed, Buffer(originalTestString));
    ASSERT_GT(compressed.bytes(), 8U);
    compressed.bytes(compressed.bytes() - 8);
    EXPECT_THROW(ZLib::decompress(decompressed, compressed), Exception);
}

TEST(ZLibTests, decompressInvalidInputThrows)
{
    Buffer       decompressed;
    const Buffer invalidInput("not a gzip payload");
    EXPECT_THROW(ZLib::decompress(decompressed, invalidInput), Exception);
}

TEST(ZLibTests, performance)
{
    Buffer data;
    Buffer compressed;
    Buffer decompressed;

#ifdef _WIN32
    // Using the own executable file for the test.
    const filesystem::path testFile {"C:/Program Files/SPTK/bin/sptk_unit_tests.exe"};
#else
    const filesystem::path testFile {"/usr/bin/tar"};
#endif
    if (!filesystem::exists(testFile))
    {
        GTEST_SKIP() << "Test file not found: " << testFile;
    }
    // Repeated to reach the test size, rather than the buffer simply being told it is that big.
    // bytes() reallocates but does not initialise, so asking a 474 KB file for a megabyte fed the
    // compressor half a megabyte of whatever had last been in that memory - different on every
    // operating system, every allocator and every run. On some of them it was incompressible
    // enough for the round trip to fail outright, and the test could not be believed anywhere.
    constexpr auto testDataSize = 1024 * 1024;
    Buffer         fileContent;
    fileContent.loadFromFile(testFile);
    ASSERT_FALSE(fileContent.empty()) << "Test file is empty: " << testFile;

    while (data.bytes() < testDataSize)
    {
        const auto stillNeeded = testDataSize - data.bytes();
        data.append(fileContent.data(), std::min(stillNeeded, fileContent.bytes()));
    }

    Stopwatch stopWatch;
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

    // Compared as bytes, not as C strings: this is a binary, and STREQ stops at the first NUL -
    // which in an ELF file is within the first few bytes, so it was checking almost nothing.
    ASSERT_EQ(data.bytes(), decompressed.bytes());
    EXPECT_EQ(0, memcmp(data.data(), decompressed.data(), data.bytes()));
}

} // namespace sptk
