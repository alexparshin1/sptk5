/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CRunable.cpp - description                             ║
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

#include <sptk5/threads/CRunable.h>

using namespace std;
using namespace sptk;

CRunable::CRunable() :
    m_terminated(false)
{
}

CRunable::~CRunable()
{
}

void CRunable::execute() THROWS_EXCEPTIONS
{
    CSynchronizedCode   sc(m_running);
    m_terminated = false;
    run();
}

void CRunable::terminate()
{
    m_terminated = true;
}

bool CRunable::terminated()
{
    return m_terminated;
}

bool CRunable::completed(uint32_t timeoutMS) THROWS_EXCEPTIONS
{
    try {
        m_running.lock(timeoutMS);
        m_running.unlock();
        // Not running
        return true;
    }
    catch (CTimeoutException&) {
        // Timeout - still running
        return false;
    }
}
