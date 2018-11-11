/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSMessageIndex.cpp - description                       ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Monday September 17 2018                               ║
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

#include "sptk5/wsdl/WSMessageIndex.h"

using namespace std;
using namespace sptk;

WSMessageIndex::WSMessageIndex(const Strings& messages)
{
    int i = 0;
    for (auto& message: messages) {
        m_messageIndex[message] = i;
        i++;
    }
}

int WSMessageIndex::indexOf(const String& message) const
{
    lock_guard<mutex> lock(m_mutex);
    auto itor = m_messageIndex.find(message);
    if (itor == m_messageIndex.end())
        return -1;
    return itor->second;
}

