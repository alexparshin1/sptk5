/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SharedStrings.cpp - description                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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
#include <sptk5/Exception.h>
#include <sptk5/SharedStrings.h>

using namespace std;
using namespace sptk;

SharedStrings::SharedStrings() {
    shareString("");
}

const string& SharedStrings::shareString(const char* str) {
    CSIMap::iterator itor = m_stringIdMap.find(str);
    if (itor == m_stringIdMap.end()) {
        pair<CSIMap::iterator,bool> insertResult = m_stringIdMap.insert(CSIMap::value_type(str,0));
        itor = insertResult.first;
    }
    itor->second++;
    return itor->first;
}

void SharedStrings::releaseString(const char* str) THROWS_EXCEPTIONS {
    CSIMap::iterator itor = m_stringIdMap.find(str);
    if (itor == m_stringIdMap.end())
        throw Exception("The string "+string(str)+" isn't registered in SST");
    if (itor->second > 1)
        itor->second--;
    else
        m_stringIdMap.erase(itor);
}

void SharedStrings::clear() {
    m_stringIdMap.clear();
    shareString("");
}
