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

#include <sptk5/Buffer.h>
#include <sstream>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

static const String testString("This is a test\ntext that contains several\nexample rows");
static const String resultString("This is a test\rtext that contains several\rexample rows");

TEST(SPTK_Strings, ctor)
{
    Strings strings(testString, "[\\n\\r]+", Strings::SplitMode::REGEXP);
    EXPECT_EQ(static_cast<size_t>(3), strings.size());
    EXPECT_STREQ(resultString.c_str(), strings.join("\r").c_str());

    const Strings strings2(strings);
    EXPECT_EQ(static_cast<size_t>(3), strings2.size());
    EXPECT_STREQ(resultString.c_str(), strings2.join("\r").c_str());

    strings.fromString(testString, "\n", Strings::SplitMode::DELIMITER);
    EXPECT_EQ(static_cast<size_t>(3), strings.size());
    EXPECT_STREQ(resultString.c_str(), strings.join("\r").c_str());

    const Strings strings3({"1", "2", "3"});
    EXPECT_EQ(static_cast<size_t>(3), strings3.size());
    EXPECT_STREQ("1,2,3", strings3.join(",").c_str());

    Strings numbers = {{"one", 3, 1},
                       {"two", 3, 2},
                       {"three", strlen("three"), 3}};
    EXPECT_EQ(static_cast<size_t>(3), numbers.size());
    EXPECT_STREQ("one,two,three", numbers.join(",").c_str());
    EXPECT_EQ(2, numbers[1].ident());
}

TEST(SPTK_Strings, sort)
{
    Strings strings(testString, "[\\n\\r]+", Strings::SplitMode::REGEXP);
    strings.sort();
    EXPECT_STREQ("This is a test\nexample rows\ntext that contains several", strings.join("\n").c_str());
}

TEST(SPTK_Strings, remove)
{
    Strings strings({"1", "2", "3"});
    strings.remove("2");
    EXPECT_STREQ("1,3", strings.join(",").c_str());
}

TEST(SPTK_Strings, indexOf)
{
    Strings strings(testString, "[\\n\\r]+", Strings::SplitMode::REGEXP);
    EXPECT_EQ(1, strings.indexOf("text that contains several"));
    EXPECT_EQ(-1, strings.indexOf("text that contains"));

    strings.sort();
    EXPECT_EQ(2, strings.indexOf("text that contains several"));
    EXPECT_EQ(-1, strings.indexOf("text that Contains"));

    strings.sort(false);
    EXPECT_EQ(2, strings.indexOf("text that contains several"));
    EXPECT_EQ(-1, strings.indexOf("text that Contains"));
}

TEST(SPTK_Strings, grep)
{
    const Strings strings(testString, "[\\n\\r]+", Strings::SplitMode::REGEXP);

    const Strings group1 = strings.grep("text");
    EXPECT_EQ(static_cast<size_t>(1), group1.size());

    const Strings group2 = strings.grep("text|rows");
    EXPECT_EQ(static_cast<size_t>(2), group2.size());
}
