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
│   You shouldint have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <cmath>
#include <iomanip>

#include <sptk5/Field.h>
#include <sptk5/xdoc/Document.h>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

TEST(SPTK_Variant, ctors)
{
    constexpr double testDoubleValue {2.22};
    DateTime testDate("2018-02-01 09:11:14.345Z");

    Variant v1(1);
    Variant v2(testDoubleValue);
    Variant v3("Test");
    Variant v4(String("Test"));
    Variant v5(testDate);

    EXPECT_EQ(1, v1.asInteger());
    EXPECT_DOUBLE_EQ(testDoubleValue, v2.asFloat());
    EXPECT_STREQ("Test", v3.asString().c_str());
    EXPECT_STREQ("Test", v4.asString().c_str());
    EXPECT_STREQ("2018-02-01T09:11:14.345Z",
                 v5.asDateTime().isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS, true).c_str());
}

TEST(SPTK_Variant, copy_ctors)
{
    constexpr double testDoubleValue {2.22};
    DateTime testDate("2018-02-01 09:11:14.345Z");

    Variant v1(1);
    Variant v2(testDoubleValue);
    Variant v3("Test");
    Variant v4(testDate);
    Variant v5;
    Variant v6(Buffer((const uint8_t*) "A test", 6));
    Variant v7(int64_t(1));

    v5.setNull(VariantDataType::VAR_STRING);

    Variant v1c(v1);
    Variant v2c(v2);
    Variant v3c(v3);
    Variant v4c(v4);
    Variant v5c(v5);
    Variant v6c(v6);
    Variant v7c(v7);

    EXPECT_EQ(1, v1c.asInteger());
    EXPECT_DOUBLE_EQ(testDoubleValue, v2c.asFloat());
    EXPECT_STREQ("Test", v3c.asString().c_str());
    EXPECT_STREQ("2018-02-01T09:11:14.345Z",
                 v4c.asDateTime().isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS, true).c_str());
    EXPECT_EQ(v5c.isNull(), true);
    EXPECT_EQ(v5c.dataType(), VariantDataType::VAR_STRING);
    EXPECT_STREQ(v6c.asString().c_str(), "A test");
    EXPECT_EQ(v6c.dataType(), VariantDataType::VAR_BUFFER);
    EXPECT_EQ(int64_t(1), v1c.asInt64());
}

TEST(SPTK_Variant, move_ctors)
{
    DateTime testDate("2018-02-01 09:11:14.345Z");

    Variant v1(1);
    Variant v2(2.22);
    Variant v3("Test");
    Variant v4(testDate);
    Variant v5;
    Variant v6(Buffer((const uint8_t*) "A test", 6));

    v5.setNull(VariantDataType::VAR_STRING);

    Variant v1m(move(v1));
    Variant v2m(move(v2));
    Variant v3m(move(v3));
    Variant v4m(move(v4));
    Variant v5m(move(v5));
    Variant v6m(move(v6));

    EXPECT_EQ(1, v1m.asInteger());
    EXPECT_DOUBLE_EQ(2.22, v2m.asFloat());
    EXPECT_STREQ("Test", v3m.asString().c_str());
    EXPECT_STREQ("2018-02-01T09:11:14.345Z",
                 v4m.asDateTime().isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS, true).c_str());
    EXPECT_EQ(v5m.isNull(), true);
    EXPECT_EQ(v5m.dataType(), VariantDataType::VAR_STRING);
    EXPECT_STREQ(v6m.asString().c_str(), "A test");
    EXPECT_EQ(v6m.dataType(), VariantDataType::VAR_BUFFER);
}

TEST(SPTK_Variant, assigns)
{
    DateTime testDate("2018-02-01 09:11:14.345Z");

    Variant v;
    Variant v1;

    v = 1;
    EXPECT_EQ(1, (int) v);
    EXPECT_EQ(1, v.asInteger());
    EXPECT_EQ(1, v.asInt64());
    EXPECT_EQ(1, v.get<int>());

    v = int64_t(1);
    EXPECT_EQ(1, (int64_t) v);
    EXPECT_EQ(1, v.asInt64());
    EXPECT_EQ(1, v.asInteger());
    EXPECT_EQ(1, v.get<int64_t>());

    v = 2.22;
    EXPECT_DOUBLE_EQ(2.22, (double) v);
    EXPECT_DOUBLE_EQ(2.22, v.asFloat());
    EXPECT_DOUBLE_EQ(2.22, v.get<double>());
    EXPECT_EQ(2, v.asInteger());
    EXPECT_EQ(2, v.asInt64());

    v = "Test";
    EXPECT_STREQ("Test", ((String) v).c_str());
    EXPECT_STREQ("Test", v.asString().c_str());
    EXPECT_STREQ("Test", v.getString());

    v = String("Test1");
    EXPECT_STREQ("Test1", v.asString().c_str());

    v1 = v;
    EXPECT_STREQ("Test1", v1.asString().c_str());

    v = testDate;

    EXPECT_STREQ("2018-02-01T09:11:14.345Z",
                 ((DateTime) v).isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS, true).c_str());

    EXPECT_STREQ("2018-02-01T09:11:14.345Z",
                 v.asDateTime().isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS, true).c_str());

    v = true;
    EXPECT_TRUE(v);
    EXPECT_TRUE((bool) v);
    EXPECT_TRUE(v.get<bool>());
    EXPECT_DOUBLE_EQ(v.asInteger(), 1);
    EXPECT_EQ(v.asInt64(), 1L);

    v = false;
    EXPECT_FALSE(v);
    EXPECT_FALSE((bool) v);
    EXPECT_FALSE(v.get<bool>());
    EXPECT_DOUBLE_EQ(v.asInteger(), 0);
    EXPECT_EQ(v.asInt64(), 0L);

    v = MoneyData(1234, 2);
    EXPECT_DOUBLE_EQ(v.asFloat(), 12.34);
    EXPECT_DOUBLE_EQ(v.asInteger(), 12);
    EXPECT_EQ(v.asInt64(), 12L);

    v.setDateTime(testDate, true);
    EXPECT_STREQ("2018-02-01T00:00:00.000Z",
                 v.asDateTime().isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS, true).c_str());
    EXPECT_STREQ("2018-02-01T00:00:00.000Z",
                 v.get<DateTime>().isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS, true).c_str());

    auto* ptr = (uint8_t*) &v;
    v.setImagePtr(ptr);
    EXPECT_EQ(v.asImagePtr(), ptr);
    EXPECT_EQ(v.getImagePtr(), ptr);

    ++ptr;
    v = ptr;
    EXPECT_EQ(v.asImagePtr(), ptr);

    Buffer b("Hello");
    v = b;
    EXPECT_STREQ(v.asString().c_str(), "Hello");

    v.setBuffer((const uint8_t*) nullptr, 10, sptk::VariantDataType::VAR_BUFFER);
    EXPECT_TRUE(v.isNull());
    EXPECT_EQ(v.dataSize(), size_t(0));

    v.setImageNdx(12);
    EXPECT_EQ(v.getImageNdx(), 12U);
}

TEST(SPTK_Variant, move_assigns)
{
    DateTime testDate("2018-02-01 09:11:14.345Z");

    Variant v;
    Variant vm;

    v = 1;
    vm = move(v);
    EXPECT_EQ(1, vm.asInteger());

    v = 2.22;
    vm = move(v);
    EXPECT_DOUBLE_EQ(2.22, vm.asFloat());

    v = "Test";
    vm = move(v);
    EXPECT_STREQ("Test", vm.asString().c_str());

    v = testDate;
    vm = move(v);
    EXPECT_STREQ("2018-02-01T09:11:14.345Z",
                 vm.asDateTime().isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS, true).c_str());

    Variant v2;
    v2.setDateTime(testDate, true);
    vm = move(v2);
    EXPECT_STREQ("2018-02-01T00:00:00.000Z",
                 vm.asDateTime().isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS, true).c_str());
}

TEST(SPTK_Variant, copy)
{
    DateTime testDate("2018-02-01 09:11:14.345Z");

    Variant v;
    Variant v0(12345);
    Variant v1(testDate);
    Variant v2(1.2345);
    Variant v3("Test");

    Variant v4;
    v4.setDateTime(testDate, true);

    v = v0;
    EXPECT_EQ(12345, v.asInteger());

    v = v1;
    EXPECT_STREQ("2018-02-01T09:11:14.345Z",
                 v.asDateTime().isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS, true).c_str());

    v = v2;
    EXPECT_DOUBLE_EQ(1.2345, v.asFloat());

    v = v3;
    EXPECT_STREQ("Test", v.asString().c_str());

    v = v4;
    EXPECT_STREQ("2018-02-01T00:00:00.000Z",
                 v.asDateTime().isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS, true).c_str());
}

TEST(SPTK_Variant, toString)
{
    DateTime testDate("2018-02-01 09:11:14.345Z");

    Variant v1(1);
    Variant v2(2.22);
    Variant v3("Test");
    Variant v4(testDate);
    Variant v5;
    DateTime dt;
    String dtStr;

    v5.setDateTime(testDate, true);

    EXPECT_STREQ("1", v1.asString().c_str());
    EXPECT_STREQ("2.22", v2.asString().c_str());
    EXPECT_STREQ("Test", v3.asString().c_str());

    dt = v4.asDateTime();
    dtStr = dt.dateString(DateTime::PF_RFC_DATE) + "T" + dt.timeString();
    EXPECT_STREQ(dtStr.c_str(), v4.asString().substr(0, 19).c_str());

    dt = v5.asDateTime();
    dtStr = dt.dateString(DateTime::PF_RFC_DATE);
    EXPECT_STREQ(dtStr.c_str(), v5.asString().c_str());
}

TEST(SPTK_Variant, money)
{
    Variant money(10001234, 4);
    EXPECT_DOUBLE_EQ((double) money.getMoney(), 1000.1234);
    EXPECT_EQ((int) money.getMoney(), 1000);
    EXPECT_EQ((int64_t) money.getMoney(), 1000);
    EXPECT_TRUE((bool) money.getMoney());
    EXPECT_STREQ(money.asString().c_str(), "1000.1234");

    money.setMoney(200055, 2);
    EXPECT_STREQ(money.asString().c_str(), "2000.55");
    EXPECT_EQ(VariantDataType::VAR_MONEY, Variant::nameType("money"));
    EXPECT_STREQ("money", Variant::typeName(VariantDataType::VAR_MONEY).c_str());

    MoneyData value {12345678, 4};
    money.setMoney(value);
    EXPECT_DOUBLE_EQ((double) money, 1234.5678);
    EXPECT_TRUE(money.dataType() == VariantDataType::VAR_MONEY);

    Variant s((const uint8_t*) "test", 4);
    s.setMoney(1234567, 4);
    EXPECT_DOUBLE_EQ((double) s.getMoney(), 123.4567);
}

TEST(SPTK_Variant, externalBuffer)
{
    Buffer externalBuffer("External Data");
    Variant v;
    v.setExternalBuffer(externalBuffer.data(), externalBuffer.length());
    EXPECT_STREQ(externalBuffer.c_str(), v.asString().c_str());
    externalBuffer[1] = 'X';
    EXPECT_STREQ(externalBuffer.c_str(), v.asString().c_str());
}

TEST(SPTK_Variant, json)
{
    constexpr int testInteger1 = 12345;
    const char* json = R"({ "value": 12345 })";
    xdoc::Document document;
    document.load(json);
    auto node = document.root()->findFirst("value");

    Variant v;
    v.load(node);
    EXPECT_EQ(v.asInteger(), testInteger1);

    constexpr int testInteger2 = 123456;
    v = testInteger2;
    v.save(node);
    EXPECT_STREQ(node->getString().c_str(), "123456");
}

TEST(SPTK_Variant, bool)
{
    Variant v1(false);
    EXPECT_FALSE(v1);

    Variant v2(true);
    EXPECT_TRUE(v2);

    Variant v3((const uint8_t*) "test", 4);
    v3.setBool(true);
    EXPECT_TRUE(v3);
}

TEST(SPTK_Variant, xml)
{
    const char* xml = "<value>12345</value>";
    xdoc::Document document;
    document.load(xml);
    auto node = document.root()->findFirst("value");

    Variant v;
    v.load(node);
    EXPECT_EQ(v.asInteger(), 12345);

    v = 123456;
    v.save(node);
    EXPECT_STREQ(node->getString().c_str(), "123456");
}
