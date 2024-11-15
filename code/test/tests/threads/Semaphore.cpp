/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <gtest/gtest.h>

#include <sptk5/Printer.h>
#include <sptk5/StopWatch.h>
#include <sptk5/threads/Semaphore.h>

#include <future>

using namespace std;
using namespace sptk;

TEST(SPTK_Semaphore, waitAndPost)
{
    Semaphore semaphore;

    constexpr chrono::milliseconds interval(100);

    DateTime started = DateTime::Now();
    semaphore.wait_for(interval);
    DateTime ended = DateTime::Now();
    EXPECT_NEAR(100, static_cast<int>(chrono::duration_cast<chrono::milliseconds>(ended - started).count()), 50);

    semaphore.post();
    started = ended;
    ended = DateTime::Now();
    EXPECT_NEAR(0, static_cast<int>(chrono::duration_cast<chrono::milliseconds>(ended - started).count()), 50);
}

TEST(SPTK_Semaphore, threads)
{
    constexpr const chrono::milliseconds timeout(1000);
    Semaphore                            semaphore;

    const auto poster = async([&semaphore]()
                              {
                                  semaphore.post();
                              });

    const bool posted = semaphore.wait_for(timeout);

    poster.wait();

    EXPECT_TRUE(posted);
}

static void waitPerformance(bool withTimeout)
{
    Semaphore    semaphore;
    const size_t iterations = 1000000;

    StopWatch stopWatch;

    stopWatch.start();
    for (size_t i = 0; i <= iterations; ++i)
    {
        semaphore.post();
    }
    stopWatch.stop();

    stopWatch.start();
    for (size_t i = 0; i < iterations; ++i)
    {
        if (withTimeout)
        {
            semaphore.wait_for(chrono::milliseconds(1));
        }
        else
        {
            semaphore.wait_for(chrono::minutes(1));
        }
    }
    stopWatch.stop();

    COUT("Executed " << iterations << " Semaphore waits " << (withTimeout ? "with" : "without") << " timeout: " << fixed << setprecision(2) << static_cast<int>(iterations) / stopWatch.milliseconds() / 1000 << "M/sec");
}

TEST(SPTK_Semaphore, waitPerformance)
{
    waitPerformance(false);
    waitPerformance(true);
}
