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
#include <sptk5/xdoc/Node.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

TEST(SPTK_XDocument, typeRegexp)
{
    EXPECT_TRUE(isInteger("0"));
    EXPECT_TRUE(isInteger("+1"));
    EXPECT_TRUE(isInteger("+100"));
    EXPECT_TRUE(isInteger("-1234"));
    EXPECT_FALSE(isInteger("01234"));
    EXPECT_FALSE(isInteger("1234-11"));

    EXPECT_TRUE(isFloat("0.1"));
    EXPECT_TRUE(isFloat("+0.123"));
    EXPECT_TRUE(isFloat("-0.123"));
    EXPECT_TRUE(isFloat("-0.123e4"));
    EXPECT_TRUE(isFloat("-10.123e43"));
    EXPECT_FALSE(isFloat("00.123e43"));
    EXPECT_FALSE(isFloat("127.0.0.1"));
    EXPECT_FALSE(isFloat("127"));
}
