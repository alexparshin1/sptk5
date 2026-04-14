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

#include <sptk5/Exception.h>
#include <sptk5/MemoryDS.h>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

namespace {
struct Person
{
    String name;
    int    age {0};
};

const vector<Person> people {
    {"John", 30},
    {"Jane", 28},
    {"Bob", 6}};

SFieldList makePersonRow(const String& name, int age)
{
    auto row = make_shared<FieldList>(false);

    const auto nameField = make_shared<Field>("name");
    *nameField = name;
    row->push_back(nameField);

    const auto ageField = make_shared<Field>("age");
    *ageField = age;
    row->push_back(ageField);

    return row;
}

} // namespace

namespace sptk {

TEST(MemoryDSTests, createAndVerify)
{
    MemoryDS ds;

    EXPECT_TRUE(ds.empty());

    for (const auto& person: people)
    {
        ds.push_back(makePersonRow(person.name, person.age));
    }

    EXPECT_EQ(ds.recordCount(), static_cast<size_t>(3));

    ds.open();

    auto i = 0;
    while (!ds.eof())
    {
        EXPECT_EQ(ds.fieldCount(), static_cast<size_t>(2));
        EXPECT_STREQ(ds["name"].asString().c_str(), people[i].name.c_str());
        EXPECT_EQ(ds["age"].asInteger(), people[i].age);
        ++i;
        ds.next();
    }

    EXPECT_FALSE(ds.find("age", 31));
    EXPECT_TRUE(ds.find("age", 28));
    EXPECT_STREQ(ds["name"].asString().c_str(), "Jane");
    EXPECT_EQ(ds[1].asInteger(), 28);

    ds.prior();
    EXPECT_STREQ(ds["name"].asString().c_str(), "John");

    ds.last();
    EXPECT_STREQ(ds["name"].asString().c_str(), "Bob");

    ds.first();
    EXPECT_STREQ(ds["name"].asString().c_str(), "John");

    ds.close();

    ds.clear();
    EXPECT_TRUE(ds.empty());
}

TEST(MemoryDSTests, defaultConstructedEofCurrent)
{
    MemoryDS ds;

    EXPECT_TRUE(ds.empty());
    EXPECT_EQ(ds.recordCount(), static_cast<size_t>(0));

    ds.open();

    EXPECT_TRUE(ds.eof());
    EXPECT_THROW(ds.current(), Exception);
}

TEST(MemoryDSTests, iteratorStaysOnCurrentRowAfterAppend)
{
    MemoryDS ds;
    ds.push_back(makePersonRow("John", 30));
    ds.push_back(makePersonRow("Jane", 28));

    ds.open();
    ASSERT_TRUE(ds.find("name", "Jane"));
    const auto currentRowBeforeAppend = ds.current();
    ASSERT_NE(currentRowBeforeAppend, nullptr);

    ds.push_back(makePersonRow("Bob", 6));

    const auto currentRowAfterAppend = ds.current();
    ASSERT_NE(currentRowAfterAppend, nullptr);
    EXPECT_EQ(currentRowBeforeAppend.get(), currentRowAfterAppend.get());
    EXPECT_STREQ(ds["name"].asString().c_str(), "Jane");
    EXPECT_EQ(ds["age"].asInteger(), 28);
}

TEST(MemoryDSTests, iteratorStaysOnCurrentRowAfterManyAppends)
{
    MemoryDS ds;
    ds.push_back(makePersonRow("John", 30));
    ds.push_back(makePersonRow("Jane", 28));

    ds.open();
    ASSERT_TRUE(ds.find("name", "Jane"));
    const auto currentRowBeforeAppend = ds.current();
    ASSERT_NE(currentRowBeforeAppend, nullptr);

    for (int i = 0; i < 512; ++i)
    {
        ds.push_back(makePersonRow("Person " + to_string(i), i));
    }

    const auto currentRowAfterAppend = ds.current();
    ASSERT_NE(currentRowAfterAppend, nullptr);
    EXPECT_EQ(currentRowBeforeAppend.get(), currentRowAfterAppend.get());
    EXPECT_STREQ(ds["name"].asString().c_str(), "Jane");
    EXPECT_EQ(ds["age"].asInteger(), 28);
}

} // namespace sptk
