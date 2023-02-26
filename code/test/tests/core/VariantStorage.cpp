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
│   You shouldint have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <gtest/gtest.h>
#include <sptk5/DateTime.h>
#include <sptk5/VariantStorage.h>

using namespace std;
using namespace sptk;

TEST(SPTK_VariantStorage, ctor)
{
    const VariantStorage variantStorage;
    EXPECT_EQ(VariantStorage::Type::Null, variantStorage.type());
    EXPECT_EQ(16, sizeof(variantStorage));

    const VariantStorage variantStorage1(1);
    EXPECT_EQ(VariantStorage::Type::Integer, variantStorage1.type());
    EXPECT_EQ(1, (int) variantStorage1);

    const VariantStorage variantStorage2((int64_t) 2);
    EXPECT_EQ(VariantStorage::Type::Integer, variantStorage2.type());
    EXPECT_EQ(2, (int64_t) variantStorage2);

    const VariantStorage variantStorage3(3.0);
    EXPECT_EQ(VariantStorage::Type::Double, variantStorage3.type());
    EXPECT_DOUBLE_EQ(3.0, (double) variantStorage3);

    const Buffer testBuffer("Test buffer");
    const VariantStorage variantStorage4(testBuffer);
    EXPECT_EQ(VariantStorage::Type::Buffer, variantStorage4.type());
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage4).c_str());

    const String testString("Test string");
    const VariantStorage variantStorage41(testString);
    EXPECT_EQ(VariantStorage::Type::String, variantStorage41.type());
    EXPECT_STREQ(testString.c_str(), ((const String&) variantStorage41).c_str());

    const DateTime testDateTime(2023, 2, 25);
    const VariantStorage variantStorage5(testDateTime);
    EXPECT_EQ(VariantStorage::Type::DateTime, variantStorage5.type());
    EXPECT_STREQ(testDateTime.dateString().c_str(), ((const DateTime&) variantStorage5).dateString().c_str());

    const MoneyData testMoneyData(123456, 2);
    const VariantStorage variantStorage6(testMoneyData);
    EXPECT_EQ(VariantStorage::Type::Money, variantStorage6.type());
    EXPECT_DOUBLE_EQ((double) testMoneyData, (double) ((const MoneyData&) variantStorage6));

    const char* testText = "Test text";
    const VariantStorage variantStorage7(testText);
    EXPECT_EQ(VariantStorage::Type::CharPointer, variantStorage7.type());
    EXPECT_STREQ(testText, (const char*) variantStorage7);
}

TEST(SPTK_VariantStorage, copy_ctor)
{
    const Buffer testBuffer("Test buffer");
    const VariantStorage variantStorage(testBuffer);
    const VariantStorage variantStorage2(variantStorage);
    EXPECT_EQ(VariantStorage::Type::Buffer, variantStorage2.type());
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage2).c_str());

    const String testString("Test string");
    const VariantStorage variantStorage21(testString);
    const VariantStorage variantStorage22(variantStorage21);
    EXPECT_EQ(VariantStorage::Type::String, variantStorage22.type());
    EXPECT_STREQ(testString.c_str(), ((const String&) variantStorage22).c_str());

    const DateTime testDateTime(2023, 2, 25);
    const VariantStorage variantStorage1(testDateTime);
    const VariantStorage variantStorage3(variantStorage1);
    EXPECT_EQ(VariantStorage::Type::DateTime, variantStorage3.type());
    EXPECT_STREQ(testDateTime.dateString().c_str(), ((const DateTime&) variantStorage3).dateString().c_str());

    const MoneyData testMoneyData(123456, 2);
    const VariantStorage variantStorage4(testMoneyData);
    const VariantStorage variantStorage5(variantStorage4);
    EXPECT_EQ(VariantStorage::Type::Money, variantStorage5.type());
    EXPECT_DOUBLE_EQ((double) testMoneyData, (double) ((const MoneyData&) variantStorage5));

    const char* testText = "Test text";
    const VariantStorage variantStorage6(testText);
    const VariantStorage variantStorage7(variantStorage6);
    EXPECT_EQ(VariantStorage::Type::CharPointer, variantStorage7.type());
    EXPECT_STREQ(testText, (const char*) variantStorage7);
}

TEST(SPTK_VariantStorage, move_ctor)
{
    const Buffer testBuffer("Test buffer");
    VariantStorage variantStorage(testBuffer);
    const VariantStorage variantStorage2(std::move(variantStorage));
    EXPECT_EQ(VariantStorage::Type::Buffer, variantStorage2.type());
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage2).c_str());

    const String testString("Test string");
    VariantStorage variantStorage21(testString);
    const VariantStorage variantStorage22(std::move(variantStorage));
    EXPECT_EQ(VariantStorage::Type::String, variantStorage22.type());
    EXPECT_STREQ(testString.c_str(), ((const String&) variantStorage22).c_str());

    const DateTime testDateTime(2023, 2, 25);
    VariantStorage variantStorage1(testDateTime);
    const VariantStorage variantStorage3(std::move(variantStorage1));
    EXPECT_EQ(VariantStorage::Type::DateTime, variantStorage3.type());
    EXPECT_STREQ(testDateTime.dateString().c_str(), ((const DateTime&) variantStorage3).dateString().c_str());

    const MoneyData testMoneyData(123456, 2);
    VariantStorage variantStorage4(testMoneyData);
    const VariantStorage variantStorage5(std::move(variantStorage4));
    EXPECT_EQ(VariantStorage::Type::Money, variantStorage5.type());
    EXPECT_DOUBLE_EQ((double) testMoneyData, (double) ((const MoneyData&) variantStorage5));

    const char* testText = "Test text";
    VariantStorage variantStorage6(testText);
    const VariantStorage variantStorage7(std::move(variantStorage6));
    EXPECT_EQ(VariantStorage::Type::CharPointer, variantStorage7.type());
    EXPECT_STREQ(testText, (const char*) variantStorage7);

    const array<uint8_t, 4> testBytes = {0, 1, 2, 3};
    VariantStorage variantStorage8(testBytes.data());
    const VariantStorage variantStorage9(std::move(variantStorage8));
    EXPECT_EQ(VariantStorage::Type::BytePointer, variantStorage9.type());
    EXPECT_EQ(testBytes.data(), (const uint8_t*) variantStorage9);
}

TEST(SPTK_VariantStorage, assign)
{
    VariantStorage variantStorage;

    variantStorage = variantStorage; // test self-assignment

    variantStorage = 1;
    EXPECT_EQ(VariantStorage::Type::Integer, variantStorage.type());
    EXPECT_EQ(1, (int) variantStorage);

    variantStorage = (int64_t) 2;
    EXPECT_EQ(VariantStorage::Type::Integer, variantStorage.type());
    EXPECT_EQ(2, (int64_t) variantStorage);

    constexpr double testDouble = 3.0;
    variantStorage = testDouble;
    EXPECT_EQ(VariantStorage::Type::Double, variantStorage.type());
    EXPECT_DOUBLE_EQ(testDouble, (double) variantStorage);

    const Buffer testBuffer("Test buffer");
    const Buffer testBuffer2("Test buffer 2");
    variantStorage = testBuffer;
    EXPECT_EQ(VariantStorage::Type::Buffer, variantStorage.type());
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage).c_str());

    variantStorage = testBuffer2;
    EXPECT_EQ(VariantStorage::Type::Buffer, variantStorage.type());
    EXPECT_STREQ(testBuffer2.c_str(), ((const Buffer&) variantStorage).c_str());

    const String testString("Test string");
    const String testString2("Test string 2");
    variantStorage = testString;
    EXPECT_EQ(VariantStorage::Type::String, variantStorage.type());
    EXPECT_STREQ(testString.c_str(), ((const String&) variantStorage).c_str());

    variantStorage = testString2;
    EXPECT_EQ(VariantStorage::Type::String, variantStorage.type());
    EXPECT_STREQ(testString2.c_str(), ((const String&) variantStorage).c_str());

    const DateTime testDateTime(2023, 2, 21);
    const DateTime testDateTime2(2023, 2, 25);
    variantStorage = testDateTime;
    variantStorage = testDateTime2;
    EXPECT_EQ(VariantStorage::Type::DateTime, variantStorage.type());
    EXPECT_STREQ(testDateTime2.dateString().c_str(), ((const DateTime&) variantStorage).dateString().c_str());

    variantStorage = 1;
    variantStorage = testDateTime2;
    EXPECT_EQ(VariantStorage::Type::DateTime, variantStorage.type());
    EXPECT_STREQ(testDateTime2.dateString().c_str(), ((const DateTime&) variantStorage).dateString().c_str());

    const MoneyData testMoneyData(1234567, 2);
    const MoneyData testMoneyData2(123456, 2);
    variantStorage = testMoneyData;
    variantStorage = testMoneyData2;
    EXPECT_EQ(VariantStorage::Type::Money, variantStorage.type());
    EXPECT_DOUBLE_EQ((double) testMoneyData2, (double) ((const MoneyData&) variantStorage));

    variantStorage = 1;
    variantStorage = testMoneyData2;
    EXPECT_EQ(VariantStorage::Type::Money, variantStorage.type());
    EXPECT_DOUBLE_EQ((double) testMoneyData2, (double) ((const MoneyData&) variantStorage));

    const char* testText = "Test text";
    variantStorage = testText;
    EXPECT_EQ(VariantStorage::Type::CharPointer, variantStorage.type());
    EXPECT_EQ(testText, (const char*) variantStorage);

    variantStorage = 1;
    variantStorage = testText;
    EXPECT_EQ(VariantStorage::Type::CharPointer, variantStorage.type());
    EXPECT_EQ(testText, (const char*) variantStorage);

    const array<uint8_t, 4> testBytes = {0, 1, 2, 3};
    variantStorage = testBytes.data();
    EXPECT_EQ(VariantStorage::Type::BytePointer, variantStorage.type());
    EXPECT_EQ(testBytes.data(), (const uint8_t*) variantStorage);

    variantStorage = Buffer("Test text");
    variantStorage = testBytes.data();
    EXPECT_EQ(VariantStorage::Type::BytePointer, variantStorage.type());
    EXPECT_EQ(testBytes.data(), (const uint8_t*) variantStorage);

    VariantStorage variantStorage2(testBuffer);
    variantStorage = variantStorage2;
    EXPECT_EQ(VariantStorage::Type::Buffer, variantStorage.type());
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage).c_str());

    variantStorage = std::move(variantStorage2);
    EXPECT_EQ(VariantStorage::Type::Buffer, variantStorage.type());
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage).c_str());

    // Replace Buffer with Integer: check for memory leaks
    variantStorage = 1;
    EXPECT_EQ(VariantStorage::Type::Integer, variantStorage.type());
    EXPECT_EQ(1, (int) variantStorage);
}

TEST(SPTK_VariantStorage, copyAssign)
{
    VariantStorage variantStorage;
    VariantStorage variantStorage2;

    variantStorage = 1;
    variantStorage2 = variantStorage;
    EXPECT_EQ(VariantStorage::Type::Integer, variantStorage2.type());
    EXPECT_EQ(1, (int) variantStorage2);

    variantStorage = (int64_t) 2;
    variantStorage2 = variantStorage;
    EXPECT_EQ(VariantStorage::Type::Integer, variantStorage2.type());
    EXPECT_EQ(2, (int64_t) variantStorage2);

    constexpr double testDouble = 3.0;
    variantStorage = testDouble;
    variantStorage2 = variantStorage;
    EXPECT_EQ(VariantStorage::Type::Double, variantStorage2.type());
    EXPECT_DOUBLE_EQ(testDouble, (double) variantStorage2);

    const Buffer testBuffer("Test string");
    variantStorage = testBuffer;
    variantStorage2 = variantStorage;
    EXPECT_EQ(VariantStorage::Type::Buffer, variantStorage2.type());
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage2).c_str());

    const String testString("Test buffer");
    variantStorage = testString;
    variantStorage2 = variantStorage;
    EXPECT_EQ(VariantStorage::Type::String, variantStorage2.type());
    EXPECT_STREQ(testString.c_str(), ((const String&) variantStorage2).c_str());

    const DateTime testDateTime(2023, 2, 21);
    variantStorage = testDateTime;
    variantStorage2 = variantStorage;
    EXPECT_EQ(VariantStorage::Type::DateTime, variantStorage.type());
    EXPECT_STREQ(testDateTime.dateString().c_str(), ((const DateTime&) variantStorage2).dateString().c_str());

    const MoneyData testMoneyData(1234567, 2);
    variantStorage = testMoneyData;
    variantStorage2 = variantStorage;
    EXPECT_EQ(VariantStorage::Type::Money, variantStorage.type());
    EXPECT_DOUBLE_EQ((double) testMoneyData, (double) ((const MoneyData&) variantStorage2));

    const char* testText = "Test text";
    variantStorage = testText;
    variantStorage2 = variantStorage;
    EXPECT_EQ(VariantStorage::Type::CharPointer, variantStorage2.type());
    EXPECT_EQ(testText, (const char*) variantStorage2);

    const array<uint8_t, 4> testBytes = {0, 1, 2, 3};
    variantStorage = testBytes.data();
    variantStorage2 = variantStorage;
    EXPECT_EQ(VariantStorage::Type::BytePointer, variantStorage2.type());
    EXPECT_EQ(testBytes.data(), (const uint8_t*) variantStorage2);
}

TEST(SPTK_VariantStorage, getInvalidType)
{
    VariantStorage variantStorage;

    variantStorage = 1;
    EXPECT_THROW((double) variantStorage, invalid_argument);
    EXPECT_THROW(((const Buffer&) variantStorage).bytes(), invalid_argument);
    EXPECT_THROW(((const DateTime&) variantStorage).date(), invalid_argument);
    EXPECT_THROW(((const MoneyData&) variantStorage).quantity, invalid_argument);
    EXPECT_THROW((const uint8_t*) variantStorage, invalid_argument);
    EXPECT_THROW((const char*) variantStorage, invalid_argument);
    EXPECT_THROW((const String&) variantStorage, invalid_argument);

    variantStorage = (int64_t) 2;
    EXPECT_THROW((double) variantStorage, invalid_argument);

    constexpr double testDouble = 3.0;
    variantStorage = testDouble;
    EXPECT_THROW((int) variantStorage, invalid_argument);

    const Buffer testBuffer("Test buffer");
    variantStorage = testBuffer;
    EXPECT_THROW((int) variantStorage, invalid_argument);

    const DateTime testDateTime(2023, 2, 21);
    variantStorage = testDateTime;
    EXPECT_THROW((int) variantStorage, invalid_argument);

    const MoneyData testMoneyData(1234567, 2);
    variantStorage = testMoneyData;
    EXPECT_THROW((int) variantStorage, invalid_argument);

    const char* testText = "Test text";
    variantStorage = testText;
    EXPECT_THROW((int) variantStorage, invalid_argument);

    const array<uint8_t, 4> testBytes = {0, 1, 2, 3};
    variantStorage = testBytes.data();
    EXPECT_THROW((int) variantStorage, invalid_argument);
}
