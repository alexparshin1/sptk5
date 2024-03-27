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

#include <mutex>
#include <sptk5/threads/Counter.h>

using namespace std;
using namespace sptk;
using namespace chrono;

Counter::Counter(size_t startingValue)
    : m_counter(startingValue)
{
}

Counter::~Counter()
{
    const scoped_lock lock(m_lockMutex);
    m_condition.notify_all();
}

size_t Counter::get() const
{
    const scoped_lock lock(m_lockMutex);
    return m_counter;
}

void Counter::set(size_t value)
{
    const scoped_lock lock(m_lockMutex);
    if (m_counter != value)
    {
        m_counter = value;
        m_condition.notify_all();
    }
}

bool Counter::wait_for(size_t value, chrono::milliseconds timeout)
{
    unique_lock lock(m_lockMutex);

    if (m_counter == value)
    {
        return true;
    }

    // Wait until m_counter is matching the value
    return m_condition.wait_for(lock,
                                timeout,
                                [this, value]() {
                                    return m_counter == value;
                                });
}

bool Counter::wait_until(size_t value, const DateTime& timeoutAt)
{
    unique_lock lock(m_lockMutex);

    // Wait until semaphore value is greater than 0
    return m_condition.wait_until(lock,
                                  timeoutAt.timePoint(),
                                  [this, value]() {
                                      return m_counter == value;
                                  });
}

size_t Counter::increment(size_t value)
{
    const scoped_lock lock(m_lockMutex);
    if (value != 0)
    {
        m_counter += value;
        m_condition.notify_all();
    }
    return m_counter;
}

size_t Counter::decrement(size_t value)
{
    const scoped_lock lock(m_lockMutex);
    if (value <= m_counter)
    {
        m_counter -= value;
        m_condition.notify_all();
    }
    return m_counter;
}
