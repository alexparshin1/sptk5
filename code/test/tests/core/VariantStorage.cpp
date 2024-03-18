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

#include <gtest/gtest.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

TEST(SPTK_VariantStorage, null)
{
    const VariantStorage variantStorage;
    EXPECT_TRUE(VariantDataType::VAR_NONE == variantStorage.type().type);
    EXPECT_TRUE(variantStorage.isNull());
}

TEST(SPTK_VariantStorage, bool)
{
    VariantStorage variantStorage(true);
    EXPECT_TRUE(VariantDataType::VAR_BOOL == variantStorage.type().type);
    EXPECT_EQ(true, (bool) variantStorage);
    EXPECT_FALSE(variantStorage.isNull());

    variantStorage = 1;
    variantStorage = false;
    EXPECT_TRUE(VariantDataType::VAR_BOOL == variantStorage.type().type);
    EXPECT_EQ(false, (bool) variantStorage);
    EXPECT_FALSE(variantStorage.isNull());

    variantStorage = String("");
    variantStorage = true;
    EXPECT_TRUE(VariantDataType::VAR_BOOL == variantStorage.type().type);
    EXPECT_EQ(true, (bool) variantStorage);
    EXPECT_FALSE(variantStorage.isNull());

    variantStorage.setNull();
    EXPECT_TRUE(VariantDataType::VAR_BOOL == variantStorage.type().type);
    EXPECT_EQ(false, (bool) variantStorage);
    EXPECT_TRUE(variantStorage.isNull());

    variantStorage = false;
    EXPECT_TRUE(VariantDataType::VAR_BOOL == variantStorage.type().type);
    EXPECT_EQ(false, (bool) variantStorage);
    EXPECT_FALSE(variantStorage.isNull());

    auto& myBool = (bool&) variantStorage;
    myBool = true;
    EXPECT_EQ(true, (bool) variantStorage);
    EXPECT_TRUE(true == variantStorage.get<bool>());
}

TEST(SPTK_VariantStorage, integer)
{
    constexpr auto testFloat = 0.1;
    VariantStorage variantStorage1(1);
    EXPECT_TRUE(VariantDataType::VAR_INT == variantStorage1.type().type);
    EXPECT_EQ(1, (int) variantStorage1);
    EXPECT_FALSE(variantStorage1.isNull());

    variantStorage1 = variantStorage1;

    variantStorage1 = testFloat;
    variantStorage1 = (int64_t) 2;
    EXPECT_TRUE(VariantDataType::VAR_INT64 == variantStorage1.type().type);
    EXPECT_EQ(2, (int) variantStorage1);
    EXPECT_FALSE(variantStorage1.isNull());

    VariantStorage variantStorage2((int64_t) 3);
    EXPECT_TRUE(VariantDataType::VAR_INT64 == variantStorage2.type().type);
    EXPECT_EQ(3, (int64_t) variantStorage2);
    EXPECT_FALSE(variantStorage2.isNull());

    variantStorage2 = variantStorage1;
    EXPECT_TRUE(VariantDataType::VAR_INT64 == variantStorage2.type().type);
    EXPECT_EQ(2, (int) variantStorage2);
    EXPECT_FALSE(variantStorage2.isNull());

    variantStorage2 = testFloat;
    variantStorage2 = 4;
    EXPECT_TRUE(VariantDataType::VAR_INT64 == variantStorage1.type().type);
    EXPECT_EQ(4, (int) variantStorage2);
    EXPECT_FALSE(variantStorage2.isNull());

    variantStorage2.setNull();
    variantStorage2 = 3;
    EXPECT_TRUE(VariantDataType::VAR_INT64 == variantStorage1.type().type);
    EXPECT_EQ(3, (int) variantStorage2);
    EXPECT_FALSE(variantStorage2.isNull());

    auto& value = (int&) variantStorage2;
    value = 2;
    EXPECT_EQ(2, (int) variantStorage2);
    EXPECT_FALSE(variantStorage2.isNull());
    EXPECT_TRUE(2 == variantStorage2.get<int>());
}

TEST(SPTK_VariantStorage, double)
{
    constexpr double testValue {3.0};
    VariantStorage variantStorage(testValue);
    EXPECT_TRUE(VariantDataType::VAR_FLOAT == variantStorage.type().type);
    EXPECT_DOUBLE_EQ(testValue, (double) variantStorage);
    EXPECT_FALSE(variantStorage.isNull());

    variantStorage = 1;
    variantStorage = testValue + 1;
    EXPECT_TRUE(VariantDataType::VAR_FLOAT == variantStorage.type().type);
    EXPECT_DOUBLE_EQ(testValue + 1, (double) variantStorage);
    EXPECT_FALSE(variantStorage.isNull());

    VariantStorage variantStorage2;
    variantStorage2 = variantStorage;
    EXPECT_TRUE(VariantDataType::VAR_FLOAT == variantStorage2.type().type);
    EXPECT_DOUBLE_EQ(testValue + 1, (double) variantStorage2);

    variantStorage.setNull();
    variantStorage = testValue;
    EXPECT_TRUE(VariantDataType::VAR_FLOAT == variantStorage.type().type);
    EXPECT_EQ(testValue, (double) variantStorage);
    EXPECT_FALSE(variantStorage.isNull());

    auto& value = (double&) variantStorage;
    value = testValue + 1;
    EXPECT_EQ(testValue + 1, (double) variantStorage);
    EXPECT_FALSE(variantStorage.isNull());
    EXPECT_EQ(testValue + 1, variantStorage.get<double>());
}

TEST(SPTK_VariantStorage, Buffer)
{
    const Buffer testBuffer("Test buffer");
    const Buffer testBuffer2("Test buffer 2");

    VariantStorage variantStorage(testBuffer);
    EXPECT_TRUE(VariantDataType::VAR_BUFFER == variantStorage.type().type);
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage).c_str());
    EXPECT_FALSE(variantStorage.isNull());

    variantStorage = String("test");
    variantStorage = testBuffer2;
    EXPECT_TRUE(VariantDataType::VAR_BUFFER == variantStorage.type().type);
    EXPECT_STREQ(testBuffer2.c_str(), ((const Buffer&) variantStorage).c_str());

    Buffer buffer(testBuffer);
    VariantStorage variantStorage2(std::move(buffer));
    EXPECT_TRUE(VariantDataType::VAR_BUFFER == variantStorage2.type().type);
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage2).c_str());

    Buffer buffer2(testBuffer2);
    variantStorage2 = std::move(buffer2);
    EXPECT_TRUE(VariantDataType::VAR_BUFFER == variantStorage2.type().type);
    EXPECT_STREQ(testBuffer2.c_str(), ((const Buffer&) variantStorage2).c_str());

    VariantStorage variantStorage3(variantStorage2);
    EXPECT_TRUE(VariantDataType::VAR_BUFFER == variantStorage3.type().type);
    EXPECT_STREQ(testBuffer2.c_str(), ((const Buffer&) variantStorage3).c_str());

    VariantStorage variantStorage4(std::move(variantStorage3));
    EXPECT_TRUE(VariantDataType::VAR_BUFFER == variantStorage4.type().type);
    EXPECT_STREQ(testBuffer2.c_str(), ((const Buffer&) variantStorage4).c_str());

    variantStorage = String("test");
    variantStorage = variantStorage4;
    EXPECT_TRUE(VariantDataType::VAR_BUFFER == variantStorage.type().type);
    EXPECT_STREQ(testBuffer2.c_str(), ((const Buffer&) variantStorage).c_str());

    variantStorage.setNull();
    variantStorage = testBuffer;
    EXPECT_TRUE(VariantDataType::VAR_BUFFER == variantStorage.type().type);
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage).c_str());
    EXPECT_FALSE(variantStorage.isNull());

    auto& value = (Buffer&) variantStorage;
    value = testBuffer2;
    EXPECT_STREQ(testBuffer2.c_str(), ((const Buffer&) variantStorage).c_str());
    EXPECT_FALSE(variantStorage.isNull());
    EXPECT_STREQ(testBuffer2.c_str(), variantStorage.get<Buffer&>().c_str());
    EXPECT_STREQ(testBuffer2.c_str(), variantStorage.get<Buffer>().c_str());
}

TEST(SPTK_VariantStorage, String)
{
    const String testString("Test string");
    const String testString2("Test string 2");

    VariantStorage variantStorage(testString);
    EXPECT_TRUE(VariantDataType::VAR_STRING == variantStorage.type().type);
    EXPECT_STREQ(testString.c_str(), ((const String&) variantStorage).c_str());
    EXPECT_FALSE(variantStorage.isNull());

    variantStorage = Buffer("test");
    variantStorage = testString2;
    EXPECT_TRUE(VariantDataType::VAR_STRING == variantStorage.type().type);
    EXPECT_STREQ(testString2.c_str(), ((const String&) variantStorage).c_str());

    VariantStorage variantStorage2(variantStorage);
    EXPECT_TRUE(VariantDataType::VAR_STRING == variantStorage2.type().type);
    EXPECT_STREQ(testString2.c_str(), ((const String&) variantStorage2).c_str());

    VariantStorage variantStorage3(std::move(variantStorage2));
    EXPECT_TRUE(VariantDataType::VAR_STRING == variantStorage3.type().type);
    EXPECT_STREQ(testString2.c_str(), ((const String&) variantStorage3).c_str());

    VariantStorage variantStorage4(String("test 2"));
    variantStorage4 = variantStorage;
    EXPECT_TRUE(VariantDataType::VAR_STRING == variantStorage4.type().type);
    EXPECT_STREQ(testString2.c_str(), ((const String&) variantStorage4).c_str());

    variantStorage4.setNull();
    variantStorage4 = testString;
    EXPECT_TRUE(VariantDataType::VAR_STRING == variantStorage4.type().type);
    EXPECT_STREQ(testString.c_str(), ((const String&) variantStorage4).c_str());
    EXPECT_FALSE(variantStorage4.isNull());

    auto& value = (String&) variantStorage;
    value = testString2;
    EXPECT_STREQ(testString2.c_str(), ((const String&) variantStorage).c_str());
    EXPECT_FALSE(variantStorage.isNull());
}

TEST(SPTK_VariantStorage, DateTime)
{
    const DateTime testDateTime(2023, 2, 25);
    const DateTime testDateTime2(2021, 1, 24);

    VariantStorage variantStorage(testDateTime);
    EXPECT_TRUE(VariantDataType::VAR_DATE_TIME == variantStorage.type().type);
    EXPECT_STREQ(testDateTime.dateString().c_str(), ((const DateTime&) variantStorage).dateString().c_str());
    EXPECT_FALSE(variantStorage.isNull());

    variantStorage = Buffer("test");
    variantStorage = testDateTime2;
    EXPECT_TRUE(VariantDataType::VAR_DATE_TIME == variantStorage.type().type);
    EXPECT_STREQ(testDateTime2.dateString().c_str(), ((const DateTime&) variantStorage).dateString().c_str());

    VariantStorage variantStorage2(variantStorage);
    EXPECT_TRUE(VariantDataType::VAR_DATE_TIME == variantStorage2.type().type);
    EXPECT_STREQ(testDateTime2.dateString().c_str(), ((const DateTime&) variantStorage2).dateString().c_str());

    VariantStorage variantStorage3(std::move(variantStorage2));
    EXPECT_TRUE(VariantDataType::VAR_DATE_TIME == variantStorage3.type().type);
    EXPECT_STREQ(testDateTime2.dateString().c_str(), ((const DateTime&) variantStorage3).dateString().c_str());

    variantStorage3 = testDateTime;
    variantStorage3 = variantStorage;
    EXPECT_TRUE(VariantDataType::VAR_DATE_TIME == variantStorage3.type().type);
    EXPECT_STREQ(testDateTime2.dateString().c_str(), ((const DateTime&) variantStorage3).dateString().c_str());

    variantStorage3 = 1;
    variantStorage3 = testDateTime2;
    EXPECT_TRUE(VariantDataType::VAR_DATE_TIME == variantStorage3.type().type);
    EXPECT_STREQ(testDateTime2.dateString().c_str(), ((const DateTime&) variantStorage3).dateString().c_str());

    variantStorage3.setNull();
    variantStorage3 = testDateTime2;
    EXPECT_TRUE(VariantDataType::VAR_DATE_TIME == variantStorage3.type().type);
    EXPECT_STREQ(testDateTime2.dateString().c_str(), ((const DateTime&) variantStorage3).dateString().c_str());
    EXPECT_FALSE(variantStorage3.isNull());

    auto& value = (DateTime&) variantStorage3;
    value = testDateTime;
    EXPECT_STREQ(testDateTime.dateString().c_str(), ((const DateTime&) variantStorage3).dateString().c_str());
    EXPECT_FALSE(variantStorage.isNull());
}

TEST(SPTK_VariantStorage, MoneyData)
{
    const MoneyData testMoneyData(123456, 2);
    const MoneyData testMoneyData2(1234567, 2);

    VariantStorage variantStorage(testMoneyData);
    EXPECT_TRUE(VariantDataType::VAR_MONEY == variantStorage.type().type);
    EXPECT_DOUBLE_EQ((double) testMoneyData, (double) ((const MoneyData&) variantStorage));
    EXPECT_FALSE(variantStorage.isNull());

    constexpr auto year = 2023;
    variantStorage = DateTime(year, 1, 1);
    variantStorage = testMoneyData2;
    EXPECT_TRUE(VariantDataType::VAR_MONEY == variantStorage.type().type);
    EXPECT_DOUBLE_EQ((double) testMoneyData2, (double) ((const MoneyData&) variantStorage));

    VariantStorage variantStorage2(variantStorage);
    EXPECT_TRUE(VariantDataType::VAR_MONEY == variantStorage2.type().type);
    EXPECT_DOUBLE_EQ((double) testMoneyData2, (double) ((const MoneyData&) variantStorage2));

    VariantStorage variantStorage3(std::move(variantStorage2));
    EXPECT_TRUE(VariantDataType::VAR_MONEY == variantStorage3.type().type);
    EXPECT_DOUBLE_EQ((double) testMoneyData2, (double) ((const MoneyData&) variantStorage3));

    VariantStorage variantStorage4(testMoneyData);
    variantStorage4 = variantStorage;
    EXPECT_TRUE(VariantDataType::VAR_MONEY == variantStorage4.type().type);
    EXPECT_DOUBLE_EQ((double) testMoneyData2, (double) ((const MoneyData&) variantStorage4));

    variantStorage2 = testMoneyData;
    EXPECT_TRUE(VariantDataType::VAR_MONEY == variantStorage2.type().type);
    EXPECT_DOUBLE_EQ((double) testMoneyData, (double) ((const MoneyData&) variantStorage2));
    EXPECT_FALSE(variantStorage2.isNull());

    variantStorage2.setNull();
    variantStorage2 = testMoneyData;
    EXPECT_TRUE(VariantDataType::VAR_MONEY == variantStorage2.type().type);
    EXPECT_DOUBLE_EQ((double) testMoneyData, (double) ((const MoneyData&) variantStorage2));
    EXPECT_FALSE(variantStorage2.isNull());

    auto& value = (MoneyData&) variantStorage2;
    value = testMoneyData2;
    EXPECT_DOUBLE_EQ((double) testMoneyData2, (double) ((const MoneyData&) variantStorage2));
    EXPECT_FALSE(variantStorage2.isNull());
}

TEST(SPTK_VariantStorage, externalBuffer)
{
    const array<uint8_t, 4> testBytes = {0, 1, 2, 3};
    const array<uint8_t, 5> testBytes2 = {0, 1, 2, 3, 4};
    const char* testText = "Test text";

    VariantStorage variantStorage(testBytes.data(), sizeof(testBytes), true);
    EXPECT_TRUE(VariantDataType::VAR_BYTE_POINTER == variantStorage.type().type);
    EXPECT_TRUE(4u == variantStorage.type().size);
    EXPECT_TRUE(true == variantStorage.type().isExternalBuffer);
    EXPECT_EQ(testBytes.data(), (const uint8_t*) (variantStorage));
    EXPECT_FALSE(variantStorage.isNull());

    variantStorage = 1;
    variantStorage.setExternalBuffer(bit_cast<const uint8_t*>(testText), strlen(testText), VariantDataType::VAR_STRING);
    EXPECT_TRUE(VariantDataType::VAR_STRING == variantStorage.type().type);
    EXPECT_TRUE(strlen(testText) == variantStorage.type().size);
    EXPECT_TRUE(true == variantStorage.type().isExternalBuffer);
    EXPECT_EQ((const uint8_t*) testText, (const uint8_t*) variantStorage);

    variantStorage = Buffer("");
    variantStorage.setExternalBuffer(testBytes2.data(), sizeof(testBytes2), VariantDataType::VAR_BYTE_POINTER);
    EXPECT_TRUE(VariantDataType::VAR_BYTE_POINTER == variantStorage.type().type);
    EXPECT_EQ(testBytes2.data(), (const uint8_t*) variantStorage);

    VariantStorage variantStorage2(variantStorage);
    EXPECT_TRUE(VariantDataType::VAR_BYTE_POINTER == variantStorage2.type().type);
    EXPECT_EQ(testBytes2.data(), (const uint8_t*) variantStorage2);

    VariantStorage variantStorage3(std::move(variantStorage2));
    EXPECT_TRUE(VariantDataType::VAR_BYTE_POINTER == variantStorage3.type().type);
    EXPECT_EQ(testBytes2.data(), (const uint8_t*) variantStorage3);

    variantStorage2 = Buffer("test");
    variantStorage2 = variantStorage;
    EXPECT_TRUE(VariantDataType::VAR_BYTE_POINTER == variantStorage2.type().type);
    EXPECT_EQ(testBytes2.data(), (const uint8_t*) variantStorage2);

    variantStorage2.setExternalBuffer(testBytes.data(), sizeof(testBytes), VariantDataType::VAR_BYTE_POINTER);
    EXPECT_TRUE(VariantDataType::VAR_BYTE_POINTER == variantStorage2.type().type);
    EXPECT_EQ(testBytes.data(), (const uint8_t*) variantStorage2);

    variantStorage2.setNull();
    variantStorage2.setExternalBuffer(testBytes.data(), sizeof(testBytes2), VariantDataType::VAR_BYTE_POINTER);
    EXPECT_TRUE(VariantDataType::VAR_BYTE_POINTER == variantStorage2.type().type);
    EXPECT_EQ(testBytes.data(), (const uint8_t*) variantStorage2);
    EXPECT_FALSE(variantStorage2.isNull());
}

TEST(SPTK_VariantStorage, moveAssignment)
{
    VariantStorage variantStorage;
    VariantStorage variantStorage2(Buffer("Test"));
    variantStorage = std::move(variantStorage2);
    EXPECT_TRUE(VariantDataType::VAR_BUFFER == variantStorage.type().type);
    EXPECT_STREQ("Test", ((const Buffer&) variantStorage).c_str());
}

TEST(SPTK_VariantStorage, getInvalidType)
{
    VariantStorage variantStorage;

    variantStorage = 1;
    EXPECT_THROW(COUT((double) variantStorage), invalid_argument);
    EXPECT_THROW(COUT((const Buffer&) variantStorage), invalid_argument);
    EXPECT_THROW(COUT(((const DateTime&) variantStorage).dateString()), invalid_argument);
    EXPECT_THROW(COUT(((const MoneyData&) variantStorage).quantity), invalid_argument);
    EXPECT_THROW(COUT((const uint8_t*) variantStorage), invalid_argument);
    EXPECT_THROW(COUT(((const String&) variantStorage).length()), invalid_argument);
    EXPECT_THROW(COUT((bool) variantStorage), invalid_argument);

    variantStorage = (int64_t) 2;
    EXPECT_THROW(COUT((double) variantStorage), invalid_argument);

    constexpr double testDouble = 3.0;
    variantStorage = testDouble;
    EXPECT_THROW(COUT((int) variantStorage), invalid_argument);

    const Buffer testBuffer("Test buffer");
    variantStorage = testBuffer;
    EXPECT_THROW(COUT((int) variantStorage), invalid_argument);

    const DateTime testDateTime(2023, 2, 21);
    variantStorage = testDateTime;
    EXPECT_THROW(COUT((int) variantStorage), invalid_argument);

    const MoneyData testMoneyData(1234567, 2);
    variantStorage = testMoneyData;
    EXPECT_THROW(COUT((int) variantStorage), invalid_argument);

    const char* testText = "Test text";
    variantStorage = testText;
    EXPECT_THROW(COUT((int) variantStorage), invalid_argument);

    const array<uint8_t, 4> testBytes = {0, 1, 2, 3};
    variantStorage.setExternalBuffer(testBytes.data(), sizeof(testBytes), VariantDataType::VAR_BYTE_POINTER);
    EXPECT_THROW(COUT((int) variantStorage), invalid_argument);
}

TEST(SPTK_VariantStorage, getAndSet)
{
    VariantStorage variantStorage;
    const Buffer testBuffer("test");
    variantStorage.set(testBuffer);
    EXPECT_TRUE(VariantDataType::VAR_BUFFER == variantStorage.type().type);
    EXPECT_STREQ(testBuffer.c_str(), variantStorage.get<Buffer>().c_str());
}
