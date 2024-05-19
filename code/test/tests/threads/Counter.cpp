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

#include "sptk5/StopWatch.h"
#include "sptk5/cutils"
#include <gtest/gtest.h>
#include <mutex>
#include <sptk5/threads/Counter.h>

using namespace std;
using namespace chrono;
using namespace sptk;

// @brief Test that default counter value
TEST(SPTK_Counter, ctor)
{
    Counter flag1;
    EXPECT_EQ(flag1.get(), false);

    Counter flag2(1);
    EXPECT_EQ(flag2.get(), true);
}

TEST(SPTK_Counter, waitFor)
{
    Counter counter;

    constexpr milliseconds timeout(10);
    bool result = counter.wait_for(1, timeout);
    EXPECT_EQ(counter.get(), false);
    EXPECT_EQ(result, false);

    result = counter.wait_for(0, timeout);
    EXPECT_EQ(result, true);

    counter.increment();

    result = counter.wait_for(1, timeout);
    EXPECT_EQ(counter.get(), true);
    EXPECT_EQ(result, true);

    counter.decrement();

    result = counter.wait_for(0, timeout);
    EXPECT_EQ(counter.get(), false);
    EXPECT_EQ(result, true);
}

TEST(SPTK_Counter, waitForPerformance)
{
    Counter counter;

    constexpr auto iterations = 10000000;

    StopWatch stopWatch(
        [&counter] {
            for (int i = 0; i < iterations; ++i)
            {
                counter.wait_for(0, 10ms);
            }
        });

    COUT("Executed " << iterations << " counter waits for " << stopWatch.seconds() << "ms, "
                     << fixed << setprecision(1) << iterations / 1E6 / stopWatch.seconds() << "M/sec"
                     << endl);
}

TEST(SPTK_Counter, adaptorAndAssignment)
{
    Counter flag;

    flag = 1;
    EXPECT_EQ((size_t) flag, true);

    flag = 0;
    EXPECT_EQ((size_t) flag, false);
}
