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
#include <sptk5/threads/Flag.h>

using namespace std;
using namespace sptk;
using namespace chrono;

Flag::Flag(bool startingValue)
    : m_value(startingValue)
{
}

Flag::~Flag()
{
    terminate();
    do
    {
        scoped_lock lock(m_lockMutex);
        m_condition.notify_one();
    } while (waiters() > 0);
}

void Flag::terminate()
{
    scoped_lock lock(m_lockMutex);
    m_terminated = true;
}

size_t Flag::waiters() const
{
    scoped_lock lock(m_lockMutex);
    return m_waiters;
}

bool Flag::get() const
{
    scoped_lock lock(m_lockMutex);
    return m_value;
}

void Flag::set(bool value)
{
    scoped_lock lock(m_lockMutex);
    if (m_value != value)
    {
        m_value = value;
        m_condition.notify_one();
    }
}

bool Flag::wait_for(bool value, chrono::milliseconds timeout)
{
    auto timeoutAt = DateTime::Now() + timeout;
    return wait_until(value, timeoutAt);
}

bool Flag::wait_until(bool value, const DateTime& timeoutAt)
{
    unique_lock lock(m_lockMutex);

    ++m_waiters;

    // Wait until semaphore value is greater than 0
    while (!m_terminated)
    {
        if (!m_condition.wait_until(lock,
                                    timeoutAt.timePoint(),
                                    [this, value]() {
                                        return m_value == value;
                                    }))
        {
            if (timeoutAt < DateTime::Now())
            {
                --m_waiters;
                return false;
            }
        }
        else
        {
            break;
        }
    }

    --m_waiters;

    return true;
}
