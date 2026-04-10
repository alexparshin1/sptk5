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

#include <fstream>
#include <sptk5/RegularExpression.h>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

static const String testString("This is a test");
namespace sptk {

TEST(StringTests,matches)
{
    EXPECT_TRUE(testString.matches("is a "));
}

TEST(StringTests,caseOps)
{
    EXPECT_STREQ("THIS IS A TEST", testString.toUpperCase().c_str());
    EXPECT_STREQ("this is a test", testString.toLowerCase().c_str());
}

TEST(StringTests,in)
{
    EXPECT_TRUE(String("true").in({"true", "false"}));
    EXPECT_FALSE(String("yes").in({"true", "false"}));
}

TEST(StringTests,inEmptyList)
{
    EXPECT_FALSE(String("value").in({}));
}

TEST(StringTests,split)
{
    Strings words(testString.split("[\\s]+"));
    EXPECT_EQ(static_cast<size_t>(4), words.size());
    EXPECT_STREQ("This", words[0].c_str());
    EXPECT_STREQ("test", words[3].c_str());
}

TEST(StringTests,startsEnds)
{
    EXPECT_TRUE(testString.startsWith("This "));
    EXPECT_FALSE(testString.startsWith("this "));
    EXPECT_TRUE(testString.endsWith(" test"));
    EXPECT_FALSE(testString.endsWith(" tesT"));
}

TEST(StringTests,replace)
{
    EXPECT_STREQ("This is a Test", testString.replace(" t", " T").c_str());
}

TEST(StringTests,trim)
{
    const String testString2(" \n\r\t" + testString + "\n\r\t ");
    EXPECT_STREQ(testString.c_str(), testString2.trim().c_str());
}

TEST(StringTests,trimAllWhitespace)
{
    const String testString2(" \n\r\t\b");
    EXPECT_TRUE(testString2.trim().empty());
}

} // namespace sptk_test
