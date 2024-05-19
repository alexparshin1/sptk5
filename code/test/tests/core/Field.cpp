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
#include <sptk5/Field.h>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

TEST(SPTK_Field, move_ctor_assign)
{
    constexpr int testInteger = 10;
    Field field1("f1");
    field1 = testInteger;

    Field field2(std::move(field1));
    EXPECT_EQ(field2.asInteger(), testInteger);

    Field field3("f3");
    field3 = std::move(field2);
    EXPECT_EQ(field3.asInteger(), testInteger);
}

TEST(SPTK_Field, double)
{
    Field field1("f1");

    constexpr double testDouble = 12345678.123456;
    field1 = testDouble;
    field1.view().precision = 3;

    EXPECT_DOUBLE_EQ(field1.asFloat(), testDouble);
    EXPECT_STREQ(field1.asString().c_str(), "12345678.123");
}

TEST(SPTK_Field, money)
{
    constexpr int64_t testLong = 1234567890123456789L;
    constexpr int64_t testInt64 = 12345678901;
    constexpr int scaleDigits = 8;

    MoneyData money1(testLong, scaleDigits);
    MoneyData money2(-testLong, scaleDigits);
    Field field1("f1");

    field1.setMoney(money1);
    EXPECT_EQ(field1.asInt64(), testInt64);
    EXPECT_STREQ(field1.asString().c_str(), "12345678901.23456789");

    field1.setMoney(money2);
    EXPECT_EQ(field1.asInt64(), -testInt64);
    EXPECT_STREQ(field1.asString().c_str(), "-12345678901.23456789");
}
