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
    EXPECT_NEAR(100, (int) chrono::duration_cast<chrono::milliseconds>(ended - started).count(), 20);

    semaphore.post();
    started = ended;
    ended = DateTime::Now();
    EXPECT_NEAR(0, (int) chrono::duration_cast<chrono::milliseconds>(ended - started).count(), 20);
}

TEST(SPTK_Semaphore, threads)
{
    constexpr chrono::milliseconds timeout(1000);
    Semaphore semaphore;

    auto poster = async([&semaphore]() {
        semaphore.post();
    });

    const bool posted = semaphore.wait_for(timeout);

    poster.wait();

    EXPECT_TRUE(posted);
}

TEST(SPTK_Semaphore, waitPerformance)
{
    Semaphore semaphore;
    const size_t iterations = 10000;

    StopWatch stopWatch;

    stopWatch.start();
    for (size_t i = 0; i < iterations; ++i)
    {
        semaphore.post();
    }
    stopWatch.stop();
    auto scheduleTime = stopWatch.seconds();

    stopWatch.start();
    for (size_t i = 0; i < iterations; ++i)
    {
        semaphore.wait_for(chrono::microseconds(1));
    }
    stopWatch.stop();

    COUT("Executed " << iterations << " Semaphore waits. Scheduled: " << setprecision(2) << scheduleTime << " Elapsed " << setprecision(2) << stopWatch.seconds() << " seconds" << endl);
}
