/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#ifdef USE_GTEST
#include <future>
#include <gtest/gtest.h>
#endif

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

bool Semaphore::sleep_until(DateTime timeoutAt)
{
#if CXX_VERSION < 20
    unique_lock lock(m_lockMutex);

    if (!m_condition.wait_until(lock,
                                timeoutAt.timePoint(),
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
    return m_value.try_acquire_until(timeoutAt.timePoint());
#endif
}

#ifdef USE_GTEST

TEST(SPTK_Semaphore, waitAndPost)
{
    Semaphore semaphore;

    DateTime started = DateTime::Now();
    constexpr chrono::milliseconds interval(100);
    semaphore.sleep_for(interval);
    DateTime ended = DateTime::Now();
    EXPECT_NEAR(100, (int) chrono::duration_cast<chrono::milliseconds>(ended - started).count(), 20);
    semaphore.post();
    started = ended;
    ended = DateTime::Now();
    EXPECT_NEAR(0, (int) chrono::duration_cast<chrono::milliseconds>(ended - started).count(), 20);
}

TEST(SPTK_Semaphore, threads)
{
    Semaphore semaphore;

    auto poster = async([&semaphore]() {
        semaphore.post();
    });
    constexpr chrono::milliseconds timeout(100);
    bool posted = semaphore.sleep_for(chrono::milliseconds(timeout));
    EXPECT_TRUE(posted);
    poster.wait();
}

#endif
