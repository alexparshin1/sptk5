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

#include <iomanip>
#include <sptk5/Buffer.h>

#include "sptk5/Stopwatch.h"
#include "sptk5/cthreads"
#include <gtest/gtest.h>
#include <sptk5/Printer.h>

using namespace std;
using namespace sptk;

namespace {

const string testPhrase("This is a test");

#ifdef _WIN32
const filesystem::path tempFileName("/Windows/temp/gtest_sptk5_buffer.tmp");
#else
const filesystem::path tempFileName("/tmp/gtest_sptk5_buffer.tmp");
#endif

} // namespace
namespace sptk {

TEST(BufferTests, create)
{
    const Buffer buffer1(testPhrase);
    EXPECT_STREQ(testPhrase.c_str(), buffer1.c_str());
    EXPECT_EQ(testPhrase.length(), buffer1.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer1.capacity());
}

TEST(BufferTests, copyCtor)
{
    auto         buffer1 = make_shared<Buffer>(testPhrase);
    const Buffer buffer2(*buffer1);
    buffer1.reset();

    EXPECT_STREQ(testPhrase.c_str(), buffer2.c_str());
    EXPECT_EQ(testPhrase.length(), buffer2.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer2.capacity());
}

TEST(BufferTests, move)
{
    Buffer buffer1(testPhrase);
    Buffer buffer2(std::move(buffer1));

    EXPECT_STREQ(testPhrase.c_str(), buffer2.c_str());
    EXPECT_EQ(testPhrase.length(), buffer2.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer2.capacity());

    buffer1 = "Test 1";
    EXPECT_STREQ("Test 1", buffer1.c_str());

    buffer2 = testPhrase;
    buffer1 = std::move(buffer2);

    EXPECT_STREQ(testPhrase.c_str(), buffer1.c_str());
    EXPECT_EQ(testPhrase.length(), buffer1.bytes());
}

TEST(BufferTests, assign)
{
    Buffer buffer1(testPhrase);
    Buffer buffer2;

    buffer2 = buffer1;

    EXPECT_STREQ(testPhrase.c_str(), buffer2.c_str());
    EXPECT_EQ(testPhrase.length(), buffer2.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer2.capacity());

    buffer1 = "Test 1";
    EXPECT_STREQ("Test 1", buffer1.c_str());

    buffer1 = String("Test 2");
    EXPECT_STREQ("Test 2", buffer1.c_str());
}

TEST(BufferTests, append)
{
    Buffer buffer1;

    buffer1.append(testPhrase);

    EXPECT_STREQ(testPhrase.c_str(), buffer1.c_str());
    EXPECT_EQ(testPhrase.length(), buffer1.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer1.capacity());
}

TEST(BufferTests, saveLoadFile)
{
    const Buffer buffer1(testPhrase);
    Buffer       buffer2;

    buffer1.saveToFile(tempFileName);
    buffer2.loadFromFile(tempFileName);

    EXPECT_STREQ(testPhrase.c_str(), buffer2.c_str());
    EXPECT_EQ(testPhrase.length(), buffer2.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer2.capacity());
}

TEST(BufferTests, fill)
{
    Buffer buffer1;

    constexpr int repeatCharCount = 12;
    buffer1.fill('#', repeatCharCount);

    EXPECT_STREQ("############", buffer1.c_str());
    EXPECT_EQ(static_cast<size_t>(12), buffer1.bytes());
}

TEST(BufferTests, reset)
{
    Buffer buffer1(testPhrase);

    buffer1.reset();

    EXPECT_STREQ("", buffer1.c_str());
    EXPECT_EQ(static_cast<size_t>(0), buffer1.bytes());
    EXPECT_TRUE(buffer1.capacity() > 0);
}

TEST(BufferTests, erase)
{
    Buffer buffer1(testPhrase);

    constexpr int removeCharCount = 5;
    buffer1.erase(4, removeCharCount);

    EXPECT_STREQ("This test", buffer1.c_str());
}

TEST(BufferTests, compare)
{
    const Buffer buffer1(testPhrase);
    const Buffer buffer2(testPhrase);
    const Buffer buffer3("something else");

    EXPECT_TRUE(buffer1 == buffer2);
    EXPECT_FALSE(buffer1 != buffer2);

    EXPECT_FALSE(buffer1 == buffer3);
    EXPECT_TRUE(buffer1 != buffer3);
}

TEST(BufferTests, textDump)
{
    Buffer buffer(testPhrase);
    buffer.append(testPhrase);

    stringstream stream;
    stream << buffer;

    Strings output(stream.str(), "\n\r", Strings::SplitMode::ANYCHAR);
    EXPECT_STREQ(stream.str().c_str(), buffer.c_str());
}

TEST(BufferTests, hexDump)
{
    const Strings expected {
        "00000000  54 68 69 73 20 69 73 20  61 20 74 65 73 74 54 68  This is  a testTh",
        "00000010  69 73 20 69 73 20 61 20  74 65 73 74              is is a  test"};

    Buffer buffer(testPhrase);
    buffer.append(testPhrase);

    stringstream stream;
    stream << hex << buffer;

    const Strings output(stream.str(), "\n\r", Strings::SplitMode::ANYCHAR);
    EXPECT_TRUE(output == expected);
}

TEST(BufferTests, createPerformance)
{
    // 100K x 100 appends is 10M appends per contender - still far more than enough to rank
    // Buffer against std::string and std::vector, while keeping this comparison under a second
    // instead of the seven it took at 1M.
    constexpr auto count = 100000;
    constexpr auto appendCount = 100;

    Stopwatch stopWatch;

    stopWatch.start();
    for (auto i = 0; i < count; ++i)
    {
        Buffer buffer(testPhrase);
        for (auto j = 0; j < appendCount; ++j)
        {
            buffer.append(testPhrase);
        }
    }
    stopWatch.stop();
    auto duration = stopWatch.milliseconds();
    COUT("sptk::Buffer: " << duration << "ms");

    stopWatch.start();
    for (auto i = 0; i < count; ++i)
    {
        string buffer(testPhrase.c_str(), testPhrase.length());
        for (auto j = 0; j < appendCount; ++j)
        {
            buffer += testPhrase;
        }
    }
    stopWatch.stop();
    duration = stopWatch.milliseconds();
    COUT("std::string: " << duration << "ms");

    stopWatch.start();
    for (auto i = 0; i < count; ++i)
    {
        vector<char> buffer(testPhrase.length());
        memcpy(buffer.data(), testPhrase.c_str(), testPhrase.length());
        for (auto j = 0; j < appendCount; ++j)
        {
            const auto bufferLength = buffer.size();
            buffer.resize(bufferLength + testPhrase.length());
            memcpy(buffer.data() + bufferLength, testPhrase.c_str(), testPhrase.length());
        }
    }
    stopWatch.stop();
    duration = stopWatch.milliseconds();
    COUT("std::vector: " << duration << "ms");
}

TEST(BufferTests, format)
{
    Buffer buffer;
    buffer.append((size_t) 1024, "test {}: {}", 1, "Ok");

    EXPECT_STREQ("test 1: Ok", buffer.c_str());
}

} // namespace sptk
