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

#include <sptk5/MemoryDS.h>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

struct Person {
    String name;
    int age {0};
};

static const vector<Person> people {
    {"John", 30},
    {"Jane", 28},
    {"Bob", 6}};

TEST(SPTK_MemoryDS, createAndVerify)
{
    MemoryDS ds;

    EXPECT_TRUE(ds.empty());

    for (const auto& person: people)
    {
        FieldList row(false);

        auto name = make_shared<Field>("name");
        *name = person.name;
        row.push_back(name);

        auto age = make_shared<Field>("age");
        *age = person.age;
        row.push_back(age);

        ds.push_back(move(row));
    }

    EXPECT_EQ(ds.recordCount(), size_t(3));

    ds.open();

    int i = 0;
    while (!ds.eof())
    {
        EXPECT_EQ(ds.fieldCount(), size_t(2));
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
