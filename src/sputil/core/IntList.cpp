/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       IntList.cpp - description                              ║
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

#include <sptk5/Strings.h>
#include <sptk5/IntList.h>

using namespace std;
using namespace sptk;

//---------------------------------------------------------------------------
String IntList::toString(const char* separator) const
{
    String s;
    auto cnt = (uint32_t) size();

    if (cnt == 0) return s;

    s = int2string((uint32_t) (*this)[0]);

    for (uint32_t i = 1; i < cnt; i++)
        s += separator + int2string((int32_t) (*this)[i]);

    return s;
}

void IntList::fromString(const char* s, const char* separator)
{
    Strings sl(s, separator);

    clear();
    auto cnt = (uint32_t) sl.size();

    for (uint32_t i = 0; i < cnt; i++)
        push_back((uint32_t) string2int(sl[i]));
}
