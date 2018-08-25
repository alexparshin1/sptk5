/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SharedStrings.cpp - description                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/sptk.h>
#include <sptk5/SharedStrings.h>

using namespace std;
using namespace sptk;

SharedStrings::SharedStrings()
{
    shareString("");
}

const std::string* SharedStrings::findString(const char *str) const
{
    string s(str);
    auto itor = m_strings.find(s);
    if (itor == m_strings.end()) 
        return nullptr;
    return &(*itor);
}

const string& SharedStrings::shareString(const char* str)
{
    string s(str);
    auto itor = m_strings.find(s);
    if (itor == m_strings.end()) {
        pair<Set::iterator, bool> insertResult = m_strings.insert(s);
        itor = insertResult.first;
    }
    return *itor;
}

void SharedStrings::clear()
{
    m_strings.clear();
    shareString("");
}

#if USE_GTEST
#include <gtest/gtest.h>

TEST(SPTK_SharedStrings, match)
{
    SharedStrings strings;
    strings.shareString("This");
    strings.shareString("is");
    strings.shareString("a");
    strings.shareString("test");

    EXPECT_STREQ("This", strings.findString("This")->c_str());
    EXPECT_STREQ("test", strings.findString("test")->c_str());
}

#endif