/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Buffer.h>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

static const String testString("0123456789ABCDEF");

TEST(SPTK_BufferStorage, constructors)
{
    BufferStorage testStorage1((const uint8_t*) testString.c_str(), testString.length());

    BufferStorage testStorage2(testStorage1);
    EXPECT_EQ(testStorage2.length(), 16U);
    EXPECT_STREQ(testStorage2.c_str(), testString.c_str());

    BufferStorage testStorage3(move(testStorage1));
    EXPECT_EQ(testStorage3.length(), 16U);
    EXPECT_STREQ(testStorage3.c_str(), testString.c_str());
}

TEST(SPTK_BufferStorage, assignments)
{
    BufferStorage testStorage1((const uint8_t*) testString.c_str(), testString.length());

    BufferStorage testStorage2;
    testStorage2 = testStorage1;
    EXPECT_EQ(testStorage2.length(), size_t(16));
    EXPECT_STREQ(testStorage2.c_str(), testString.c_str());

    BufferStorage testStorage3;
    testStorage3 = move(testStorage1);
    EXPECT_EQ(testStorage3.length(), size_t(16));
    EXPECT_STREQ(testStorage3.c_str(), testString.c_str());
}

TEST(SPTK_BufferStorage, append)
{
    BufferStorage testStorage;

    for (auto ch: testString)
    {
        testStorage.append(ch);
    }
    testStorage.append(testString.c_str(), testString.length());

    EXPECT_EQ(testStorage.length(), size_t(32));
    EXPECT_STREQ(testStorage.c_str(), "0123456789ABCDEF0123456789ABCDEF");
}

TEST(SPTK_BufferStorage, erase)
{
    constexpr size_t bufferSize {32};
    BufferStorage testStorage(bufferSize);
    testStorage.fill(0, bufferSize);
    testStorage.set("0123456789ABCDEF");
    EXPECT_EQ(testStorage.length(), size_t(16));
    EXPECT_STREQ(testStorage.c_str(), testString.c_str());
    testStorage.erase(0, 4);
    EXPECT_STREQ(testStorage.c_str(), "456789ABCDEF");
}

TEST(SPTK_BufferStorage, reset)
{
    constexpr size_t bufferSize {32};
    BufferStorage testStorage(bufferSize);
    testStorage.set(testString.c_str());

    testStorage.reset();
    EXPECT_EQ(testStorage.length(), size_t(0));

    testStorage.append(testString.c_str(), testString.length());

    EXPECT_EQ(testStorage.length(), testString.length());
    EXPECT_STREQ(testStorage.c_str(), testString.c_str());
}
