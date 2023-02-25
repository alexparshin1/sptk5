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

TEST(SPTK_VariantStorage, ctors)
{
    VariantStorage variantStorage;
    EXPECT_EQ(VariantStorage::Type::Null, variantStorage.type());
    EXPECT_EQ(0L, (int64_t) variantStorage);
    EXPECT_EQ(16, sizeof(variantStorage));

    VariantStorage variantStorage1((int) 1);
    EXPECT_EQ(VariantStorage::Type::Integer, variantStorage1.type());
    EXPECT_EQ(1, (int) variantStorage1);

    VariantStorage variantStorage2((int64_t) 2);
    EXPECT_EQ(VariantStorage::Type::Integer, variantStorage2.type());
    EXPECT_EQ(2, (int64_t) variantStorage2);

    VariantStorage variantStorage3(3.0);
    EXPECT_EQ(VariantStorage::Type::Double, variantStorage3.type());
    EXPECT_DOUBLE_EQ(3.0, (double) variantStorage3);

    Buffer testBuffer("Test buffer");
    VariantStorage variantStorage4(testBuffer);
    EXPECT_EQ(VariantStorage::Type::Buffer, variantStorage4.type());
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage4).c_str());

    DateTime testDateTime(2023, 2, 25);
    VariantStorage variantStorage5(testDateTime);
    EXPECT_EQ(VariantStorage::Type::DateTime, variantStorage5.type());
    EXPECT_STREQ(testDateTime.dateString().c_str(), ((const DateTime&) variantStorage5).dateString().c_str());

    MoneyData testMoneyData(123456, 2);
    VariantStorage variantStorage6(testMoneyData);
    EXPECT_EQ(VariantStorage::Type::Money, variantStorage6.type());
    EXPECT_DOUBLE_EQ((double) testMoneyData, (double) ((const MoneyData&) variantStorage6));

    const char* testText = "Test text";
    VariantStorage variantStorage7(testText);
    EXPECT_EQ(VariantStorage::Type::CharPointer, variantStorage7.type());
    EXPECT_STREQ(testText, (const char*) variantStorage7);
}

TEST(SPTK_VariantStorage, copy_ctor)
{
    Buffer testBuffer("Test buffer");
    VariantStorage variantStorage(testBuffer);
    VariantStorage variantStorage2(variantStorage);
    EXPECT_EQ(VariantStorage::Type::Buffer, variantStorage2.type());
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage2).c_str());

    DateTime testDateTime(2023, 2, 25);
    VariantStorage variantStorage1(testDateTime);
    VariantStorage variantStorage3(variantStorage1);
    EXPECT_EQ(VariantStorage::Type::DateTime, variantStorage3.type());
    EXPECT_STREQ(testDateTime.dateString().c_str(), ((const DateTime&) variantStorage3).dateString().c_str());

    MoneyData testMoneyData(123456, 2);
    VariantStorage variantStorage4(testMoneyData);
    VariantStorage variantStorage5(variantStorage4);
    EXPECT_EQ(VariantStorage::Type::Money, variantStorage5.type());
    EXPECT_DOUBLE_EQ((double) testMoneyData, (double) ((const MoneyData&) variantStorage5));

    const char* testText = "Test text";
    VariantStorage variantStorage6(testText);
    VariantStorage variantStorage7(variantStorage6);
    EXPECT_EQ(VariantStorage::Type::CharPointer, variantStorage7.type());
    EXPECT_STREQ(testText, (const char*) variantStorage7);
}

TEST(SPTK_VariantStorage, move_ctor)
{
    Buffer testBuffer("Test buffer");
    VariantStorage variantStorage(testBuffer);
    VariantStorage variantStorage2(std::move(variantStorage));
    EXPECT_EQ(VariantStorage::Type::Buffer, variantStorage2.type());
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage2).c_str());

    DateTime testDateTime(2023, 2, 25);
    VariantStorage variantStorage1(testDateTime);
    VariantStorage variantStorage3(std::move(variantStorage1));
    EXPECT_EQ(VariantStorage::Type::DateTime, variantStorage3.type());
    EXPECT_STREQ(testDateTime.dateString().c_str(), ((const DateTime&) variantStorage3).dateString().c_str());

    MoneyData testMoneyData(123456, 2);
    VariantStorage variantStorage4(testMoneyData);
    VariantStorage variantStorage5(std::move(variantStorage4));
    EXPECT_EQ(VariantStorage::Type::Money, variantStorage5.type());
    EXPECT_DOUBLE_EQ((double) testMoneyData, (double) ((const MoneyData&) variantStorage5));

    const char* testText = "Test text";
    VariantStorage variantStorage6(testText);
    VariantStorage variantStorage7(std::move(variantStorage6));
    EXPECT_EQ(VariantStorage::Type::CharPointer, variantStorage7.type());
    EXPECT_STREQ(testText, (const char*) variantStorage7);
}

TEST(SPTK_VariantStorage, assigns)
{
    VariantStorage variantStorage;

    variantStorage = (int) 1;
    EXPECT_EQ(VariantStorage::Type::Integer, variantStorage.type());
    EXPECT_EQ(1, (int) variantStorage);

    variantStorage = (int64_t) 2;
    EXPECT_EQ(VariantStorage::Type::Integer, variantStorage.type());
    EXPECT_EQ(2, (int64_t) variantStorage);

    variantStorage = 3.0;
    EXPECT_EQ(VariantStorage::Type::Double, variantStorage.type());
    EXPECT_DOUBLE_EQ(3.0, (double) variantStorage);

    Buffer testBuffer("Test buffer");
    variantStorage = testBuffer;
    EXPECT_EQ(VariantStorage::Type::Buffer, variantStorage.type());
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage).c_str());

    DateTime testDateTime(2023, 2, 25);
    variantStorage = testDateTime;
    EXPECT_EQ(VariantStorage::Type::DateTime, variantStorage.type());
    EXPECT_STREQ(testDateTime.dateString().c_str(), ((const DateTime&) variantStorage).dateString().c_str());

    MoneyData testMoneyData(123456, 2);
    variantStorage = testMoneyData;
    EXPECT_EQ(VariantStorage::Type::Money, variantStorage.type());
    EXPECT_DOUBLE_EQ((double) testMoneyData, (double) ((const MoneyData&) variantStorage));

    const char* testText = "Test text";
    variantStorage = testText;
    EXPECT_EQ(VariantStorage::Type::CharPointer, variantStorage.type());
    EXPECT_STREQ(testText, (const char*) variantStorage);

    VariantStorage variantStorage2(testBuffer);
    variantStorage = variantStorage2;
    EXPECT_EQ(VariantStorage::Type::Buffer, variantStorage.type());
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage).c_str());

    variantStorage = std::move(variantStorage2);
    EXPECT_EQ(VariantStorage::Type::Buffer, variantStorage.type());
    EXPECT_STREQ(testBuffer.c_str(), ((const Buffer&) variantStorage).c_str());
}
