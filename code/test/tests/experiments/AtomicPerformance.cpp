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

#include "sptk5/threads/SynchronizedQueue.h"
#include <gtest/gtest.h>
#include <sptk5/StopWatch.h>

using namespace std;
using namespace sptk;

TEST(SPTK_AtomicPerformance, AtomicVsMutex)
{
    mutex m;
    int   a = 0;
    int   b = 0;
    int   c = 0;

    constexpr int N = 1000000;

    StopWatch sw;
    sw.start();
    for (size_t i = 0; i < N; i++)
    {
        lock_guard lock(m);
        a++;
        b++;
        c++;
    }
    sw.stop();

    EXPECT_EQ(N, a);
    EXPECT_EQ(N, b);
    EXPECT_EQ(N, c);

    COUT("Lock guard: " << sw.milliseconds() << " ms");

    a = 0;
    b = 0;
    c = 0;

    sw.start();
    for (size_t i = 0; i < N; i++)
    {
        lock_guard lock(m);
        a++;
        b++;
        c++;
    }
    sw.stop();

    EXPECT_EQ(N, a);
    EXPECT_EQ(N, b);
    EXPECT_EQ(N, c);

    COUT("Scoped lock: " << sw.milliseconds() << " ms");

    atomic_int aa(0);
    atomic_int bb(0);
    atomic_int cc(0);
    sw.start();
    for (size_t i = 0; i < 1000000; i++)
    {
        aa++;
        bb++;
        cc++;
    }
    sw.stop();
    COUT("Atomic: " << sw.milliseconds() << " ms");

    atomic_flag fa;
    atomic_flag fb;
    atomic_flag fc;
    sw.start();
    for (size_t i = 0; i < 1000000; i++)
    {
        fa.test_and_set();
        fb.test_and_set();
        fc.test_and_set();
    }
    sw.stop();
    COUT("Flag: " << sw.milliseconds() << " ms");
}
