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

#include <iomanip>
#include <sptk5/Buffer.h>

#include "sptk5/StopWatch.h"
#include "sptk5/cthreads"
#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

static const String           testPhrase("This is a test");
static const filesystem::path tempFileName("./gtest_sptk5_buffer.tmp");

TEST(SPTK_Buffer, create)
{
    const Buffer buffer1(testPhrase);
    EXPECT_STREQ(testPhrase.c_str(), buffer1.c_str());
    EXPECT_EQ(testPhrase.length(), buffer1.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer1.capacity());
}

TEST(SPTK_Buffer, copyCtor)
{
    auto         buffer1 = make_shared<Buffer>(testPhrase);
    const Buffer buffer2(*buffer1);
    buffer1.reset();

    EXPECT_STREQ(testPhrase.c_str(), buffer2.c_str());
    EXPECT_EQ(testPhrase.length(), buffer2.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer2.capacity());
}

TEST(SPTK_Buffer, move)
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

TEST(SPTK_Buffer, assign)
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

TEST(SPTK_Buffer, append)
{
    Buffer buffer1;

    buffer1.append(testPhrase);

    EXPECT_STREQ(testPhrase.c_str(), buffer1.c_str());
    EXPECT_EQ(testPhrase.length(), buffer1.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer1.capacity());
}

TEST(SPTK_Buffer, saveLoadFile)
{
    const Buffer buffer1(testPhrase);
    Buffer       buffer2;

    buffer1.saveToFile(tempFileName);
    buffer2.loadFromFile(tempFileName);

    EXPECT_STREQ(testPhrase.c_str(), buffer2.c_str());
    EXPECT_EQ(testPhrase.length(), buffer2.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer2.capacity());
}

TEST(SPTK_Buffer, fill)
{
    Buffer buffer1;

    constexpr int repeatCharCount = 12;
    buffer1.fill('#', repeatCharCount);

    EXPECT_STREQ("############", buffer1.c_str());
    EXPECT_EQ(static_cast<size_t>(12), buffer1.bytes());
}

TEST(SPTK_Buffer, reset)
{
    Buffer buffer1(testPhrase);

    buffer1.reset();

    EXPECT_STREQ("", buffer1.c_str());
    EXPECT_EQ(static_cast<size_t>(0), buffer1.bytes());
    EXPECT_TRUE(buffer1.capacity() > 0);
}

TEST(SPTK_Buffer, erase)
{
    Buffer buffer1(testPhrase);

    constexpr int removeCharCount = 5;
    buffer1.erase(4, removeCharCount);

    EXPECT_STREQ("This test", buffer1.c_str());
}

TEST(SPTK_Buffer, compare)
{
    const Buffer buffer1(testPhrase);
    const Buffer buffer2(testPhrase);
    const Buffer buffer3("something else");

    EXPECT_TRUE(buffer1 == buffer2);
    EXPECT_FALSE(buffer1 != buffer2);

    EXPECT_FALSE(buffer1 == buffer3);
    EXPECT_TRUE(buffer1 != buffer3);
}

TEST(SPTK_Buffer, textDump)
{
    Buffer buffer(testPhrase);
    buffer.append(testPhrase);

    stringstream stream;
    stream << buffer;

    Strings output(stream.str(), "\n\r", Strings::SplitMode::ANYCHAR);
    EXPECT_STREQ(stream.str().c_str(), buffer.c_str());
}

TEST(SPTK_Buffer, hexDump)
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

TEST(SPTK_Buffer, createPerformance)
{
    constexpr size_t count = 1000000;

    StopWatch stopWatch;

    stopWatch.start();
    for (size_t i = 0; i < count; ++i)
    {
        Buffer buffer(testPhrase);
        buffer.checkSize(1024);
        buffer.checkSize(16384);
    }
    stopWatch.stop();
    auto duration = stopWatch.milliseconds();
    COUT("sptk::Buffer: " << duration << "ms");

    stopWatch.start();
    for (size_t i = 0; i < count; ++i)
    {
        string buffer(testPhrase);
        buffer.resize(1024);
        buffer.resize(16384);
    }
    stopWatch.stop();
    duration = stopWatch.milliseconds();
    COUT("std::string: " << duration << "ms");

    stopWatch.start();
    for (size_t i = 0; i < count; ++i)
    {
        vector<char> buffer(testPhrase.length());
        memcpy(buffer.data(), testPhrase.c_str(), testPhrase.length());
        buffer.resize(1024);
        buffer.resize(16384);
    }
    stopWatch.stop();
    duration = stopWatch.milliseconds();
    COUT("std::vector: " << duration << "ms");

    stopWatch.start();
    for (size_t i = 0; i < count; ++i)
    {
        char* buffer = bit_cast<char*>(malloc(testPhrase.length() + 1));
        memcpy(buffer, testPhrase.c_str(), testPhrase.length());
        auto temp = bit_cast<char*>(realloc(buffer, 1024));
        if (temp != nullptr)
        {
            buffer = temp;
        }
        temp = bit_cast<char*>(realloc(buffer, 16384));
        if (temp != nullptr)
        {
            buffer = temp;
        }
        free(buffer);
    }
    stopWatch.stop();
    duration = stopWatch.milliseconds();
    COUT("malloc: " << duration << "ms");

    stopWatch.start();
    for (size_t i = 0; i < count; ++i)
    {
        char* buffer = bit_cast<char*>(realloc(nullptr, testPhrase.length() + 1));
        memcpy(buffer, testPhrase.c_str(), testPhrase.length());
        auto temp = bit_cast<char*>(realloc(buffer, 1024));
        if (temp != nullptr)
        {
            buffer = temp;
        }
        temp = bit_cast<char*>(realloc(buffer, 16384));
        if (temp != nullptr)
        {
            buffer = temp;
        }
        free(buffer);
    }
    stopWatch.stop();
    duration = stopWatch.milliseconds();
    COUT("realloc: " << duration << "ms");
}
