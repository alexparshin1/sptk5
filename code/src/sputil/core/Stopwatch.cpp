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

#include "sptk5/Stopwatch.h"

using namespace std;
using namespace sptk;
using namespace chrono;

void Stopwatch::start()
{
    const scoped_lock lock(m_mutex);
    m_started = steady_clock::now();
    m_ended = m_started;
}

void Stopwatch::stop()
{
    const scoped_lock lock(m_mutex);
    m_ended = steady_clock::now();
}

double Stopwatch::seconds() const
{
    const scoped_lock lock(m_mutex);
    constexpr auto    millisecondsInSecond = 1000.0;
    return static_cast<double>(chrono::duration_cast<chrono::milliseconds>(m_ended - m_started).count()) /
           millisecondsInSecond;
}

double Stopwatch::milliseconds() const
{
    const scoped_lock lock(m_mutex);
    constexpr auto    microsecondsInMillisecond = 1000.0;
    return static_cast<double>(chrono::duration_cast<microseconds>(m_ended - m_started).count()) /
           microsecondsInMillisecond;
}
