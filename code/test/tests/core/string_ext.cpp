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

#include <fstream>
#include <sptk5/Buffer.h>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

TEST(SPTK_string_ext, to_string)
{
    EXPECT_EQ(222, string2int("222"));
    EXPECT_DOUBLE_EQ(2.22, string2double("2.22"));
    EXPECT_STREQ("2.22", double2string(2.22).c_str());
    EXPECT_STREQ("This is a Short Text", capitalizeWords("THIS IS a short text").c_str());
}

TEST(SPTK_string_ext, capitalizeWords)
{
    auto capitalized = capitalizeWords("tHis is  :-  a STrinG");
    EXPECT_STREQ("This is  :-  a String", capitalized.c_str());
}
