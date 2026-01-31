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

#include <gtest/gtest.h>
#include <sptk5/StopWatch.h>

#include <chrono>
#include <thread>

using namespace std;
using namespace sptk;

TEST(SPTK_StopWatch, startZeroElapsed)
{
    StopWatch stopWatch;
    stopWatch.start();
    EXPECT_DOUBLE_EQ(0.0, stopWatch.seconds());
    EXPECT_DOUBLE_EQ(0.0, stopWatch.milliseconds());
}

TEST(SPTK_StopWatch, elapsedTime)
{
    StopWatch stopWatch;
    stopWatch.start();
    this_thread::sleep_for(chrono::milliseconds(20));
    stopWatch.stop();

    const auto elapsedMs = stopWatch.milliseconds();
    EXPECT_GE(elapsedMs, 20.0);
    EXPECT_LT(elapsedMs, 21.0);

    const auto deltaMs = abs(stopWatch.seconds() * 1000.0 - elapsedMs);
    EXPECT_LE(deltaMs, 1.0);
}

TEST(SPTK_StopWatch, resetElapsedTime)
{
    StopWatch stopWatch;
    stopWatch.start();
    this_thread::sleep_for(chrono::milliseconds(20));
    stopWatch.stop();
    const auto firstMs = stopWatch.milliseconds();

    stopWatch.start();
    this_thread::sleep_for(chrono::milliseconds(19));
    stopWatch.stop();
    const auto secondMs = stopWatch.milliseconds();

    EXPECT_LT(secondMs, firstMs);
}
