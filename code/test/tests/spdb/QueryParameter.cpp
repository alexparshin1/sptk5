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

#include <gtest/gtest.h>
#include <sptk5/db/QueryParameter.h>

using namespace std;
using namespace sptk;

TEST(SPTK_QueryParameter, minimal)
{
    const QueryParameter param1("param1");

    EXPECT_STREQ(param1.name().c_str(), "param1");
}

TEST(SPTK_QueryParameter, setString)
{
    QueryParameter param1("param1");

    param1.setString("String 1");
    EXPECT_STREQ(param1.getString(), "String 1");

    param1.setString("String 1", 3);
    EXPECT_STREQ(param1.getString(), "Str");

    param1.setString("String 1 + String 2");
    EXPECT_STREQ(param1.getString(), "String 1 + String 2");

    param1.setString("String 1");
    EXPECT_STREQ(param1.getString(), "String 1");

    param1.setString("String 1 + String 2 + String 3", 22);
    EXPECT_STREQ(param1.getString(), "String 1 + String 2 + ");

    param1.setString(nullptr);
    EXPECT_TRUE(param1.isNull());
}

TEST(SPTK_QueryParameter, assign)
{
    QueryParameter param1("param1");

    param1 = "String 1";
    EXPECT_STREQ(param1.getString(), "String 1");

    param1 = "String 1, String 2";
    EXPECT_STREQ(param1.getString(), "String 1, String 2");

    param1 = 123;
    EXPECT_EQ(param1.get<int>(), 123);

    param1 = 123.0;
    EXPECT_FLOAT_EQ(param1.get<double>(), 123.0);

    param1.setString(nullptr);
    EXPECT_TRUE(param1.isNull());

    DateTime dt("2020-03-01 10:11:12");
    Variant v1(dt);
    param1 = v1;
    EXPECT_TRUE(param1.asDateTime() == dt);
}
