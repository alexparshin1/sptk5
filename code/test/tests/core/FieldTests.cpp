/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

namespace sptk {
TEST(FieldTests, moveCtorAssign)
{
    constexpr int testInteger = 10;
    Field         field1("f1");
    field1 = testInteger;

    Field field2(std::move(field1));
    EXPECT_EQ(field2.asInteger(), testInteger);

    Field field3("f3");
    field3 = std::move(field2);
    EXPECT_EQ(field3.asInteger(), testInteger);
}

TEST(FieldTests, double)
{
    Field field1("f1");

    constexpr double testDouble = 12345678.123456;
    field1 = testDouble;
    field1.view().precision = 3;

    EXPECT_DOUBLE_EQ(field1.asFloat(), testDouble);
    EXPECT_STREQ(field1.asString().c_str(), "12345678.123");
}

TEST(FieldTests, money)
{
    constexpr int64_t testLong = 1234567890123456789L;
    constexpr int64_t testInt64 = 12345678901;
    constexpr int     scaleDigits = 8;

    const MoneyData money1(testLong, scaleDigits);
    const MoneyData money2(-testLong, scaleDigits);
    Field           field1("f1");

    field1.setMoney(money1);
    EXPECT_EQ(field1.asInt64(), testInt64);
    EXPECT_STREQ(field1.asString().c_str(), "12345678901.23456789");

    field1.setMoney(money2);
    EXPECT_EQ(field1.asInt64(), -testInt64);
    EXPECT_STREQ(field1.asString().c_str(), "-12345678901.23456789");
}

TEST(FieldTests, externalBuffer)
{
    array<uint8_t, 5> externalData {'A', 'B', 'C', 0, 'D'};
    Field             field1("f1");

    field1.setExternalBuffer(externalData.data(), externalData.size(), VariantDataType::VAR_BUFFER);

    EXPECT_STREQ("ABC", field1.asString().c_str());
    EXPECT_EQ(VariantDataType::VAR_BUFFER, field1.dataType());
    EXPECT_EQ(externalData.size(), field1.dataSize());
}

TEST(FieldTests, varDate)
{
    const DateTime testDate(2026, 4, 11, 12, 34, 56);
    Field          field1("f1");

    field1.setDateTime(testDate, true);

    EXPECT_EQ(VariantDataType::VAR_DATE, field1.dataType());
    EXPECT_STREQ(testDate.date().dateString().c_str(), field1.asString().c_str());
}

TEST(FieldTests, varDateTime)
{
    const DateTime testDateTime(2026, 4, 11, 12, 34, 56);
    Field          field1("f1");

    field1.setDateTime(testDateTime);

    const auto expected = testDateTime.dateString() + " " +
                          testDateTime.timeString(DateTime::PF_TIMEZONE, DateTime::PrintAccuracy::SECONDS);

    EXPECT_EQ(VariantDataType::VAR_DATE_TIME, field1.dataType());
    EXPECT_STREQ(expected.c_str(), field1.asString().c_str());
}

} // namespace sptk
