/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Synchronized.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/threads/Synchronized.h>

using namespace std;
using namespace sptk;

Synchronized::Synchronized() :
    m_location(nullptr, 0)
{
}

void Synchronized::throwError(const char* fileName, int lineNumber)
{
    string error("Lock failed");

    if (fileName != nullptr) {
        error += " at " + CLocation(fileName, lineNumber).toString();
        if (m_location.file() != nullptr)
            error += ", conflicting lock at " + m_location.toString();
    }

    throw Exception(error + ": Lock timeout");
}


void Synchronized::lock(const char* fileName, int lineNumber)
{
    lock(uint32_t(-1), fileName, lineNumber);
}

void Synchronized::lock(uint32_t timeoutMS, const char* fileName, int lineNumber)
{
    if (timeoutMS == uint32_t(-1))
        m_synchronized.lock();
    else {
        if (!m_synchronized.try_lock_for(chrono::milliseconds(timeoutMS)))
            throwError(fileName, lineNumber);
    }
    // Storing successful lock invocation location
    m_location.set(fileName, lineNumber);
}

bool Synchronized::tryLock()
{
    return m_synchronized.try_lock();
}

void Synchronized::unlock()
{
    m_synchronized.unlock();
}
