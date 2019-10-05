/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Semaphore.cpp - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

Semaphore::Semaphore(size_t startingValue, size_t maxValue)
: m_value(startingValue), m_maxValue(maxValue)
{
}

Semaphore::~Semaphore()
{
    terminate();
    do {
        post();
    } while (waiters() > 0);
}

void Semaphore::terminate()
{
    lock_guard<mutex>  lock(m_lockMutex);
    m_terminated = true;
}

size_t Semaphore::waiters() const
{
    lock_guard<mutex>  lock(m_lockMutex);
    return m_waiters;
}

void Semaphore::post()
{
    lock_guard<mutex>  lock(m_lockMutex);
    if (m_maxValue == 0 || m_value < m_maxValue) {
        m_value++;
        m_condition.notify_one();
    }
}

void Semaphore::set(size_t value)
{
    lock_guard<mutex>  lock(m_lockMutex);
    if (m_maxValue == 0 || value < m_maxValue) {
        if (m_value != value) {
            m_value = value;
            m_condition.notify_one();
        }
    }
}

bool Semaphore::sleep_for(chrono::milliseconds timeout)
{
    auto timeoutAt = DateTime::Now() + timeout;
    return sleep_until(timeoutAt);
}

bool Semaphore::sleep_until(DateTime timeoutAt)
{
    unique_lock<mutex>  lock(m_lockMutex);

    m_waiters++;

    // Wait until semaphore value is greater than 0
    while (!m_terminated) {
        if (!m_condition.wait_until(lock,
                                    timeoutAt.timePoint(),
                                    [this]() { return m_value > 0; }))
        {
            if (timeoutAt < DateTime::Now()) {
                m_waiters--;
                return false;
            }
        } else
            break;
    }

    m_value--;
    m_waiters--;

    return true;
}

#if USE_GTEST

TEST(SPTK_Semaphore, waitAndPost)
{
    Semaphore semaphore;

    DateTime started = DateTime::Now();
    semaphore.sleep_for(chrono::milliseconds(100));
    DateTime ended = DateTime::Now();
    EXPECT_NEAR(100, (int) chrono::duration_cast<chrono::milliseconds>(ended - started).count(), 20);
    semaphore.post();
    started = ended;
    ended = DateTime::Now();
    EXPECT_NEAR(0, (int) chrono::duration_cast<chrono::milliseconds>(ended - started).count(), 20);
}

#endif
