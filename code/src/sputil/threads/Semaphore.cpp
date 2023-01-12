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

#include <sptk5/threads/Semaphore.h>

using namespace std;
using namespace sptk;

void Semaphore::post()
{
#if CXX_VERSION < 20
    unique_lock lock(m_lockMutex);
    ++m_value;
    lock.unlock();
    m_condition.notify_one();
#else
    m_value.release();
#endif
}

bool Semaphore::sleep_for(chrono::milliseconds timeout)
{
#if CXX_VERSION < 20
    unique_lock lock(m_lockMutex);

    // Wait until semaphore value is greater than 0
    if (!m_condition.wait_for(lock,
                              timeout,
                              [this]() {
                                  return m_value > 0;
                              }))
    {
        return false;
    }

    --m_value;

    return true;
#else
    return m_value.try_acquire_for(timeout);
#endif
}

bool Semaphore::sleep_until(const DateTime& timeoutAt)
{
    return sleep_until(timeoutAt.timePoint());
}

bool Semaphore::sleep_until(const DateTime::time_point& timeoutAt)
{
#if CXX_VERSION < 20
    unique_lock lock(m_lockMutex);

    if (!m_condition.wait_until(lock,
                                timeoutAt,
                                [this]() {
                                    return m_value > 0;
                                }))
    {
        if (timeoutAt < DateTime::Now())
        {
            return false;
        }
    }

    --m_value;

    return true;
#else
    return m_value.try_acquire_until(timeoutAt);
#endif
}
