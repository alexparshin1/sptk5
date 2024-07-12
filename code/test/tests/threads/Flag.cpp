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

#include <future>
#include <gtest/gtest.h>
#include <mutex>
#include <sptk5/cthreads>

using namespace std;
using namespace chrono;
using namespace sptk;

TEST(SPTK_Flag, ctor)
{
    Flag flag;
    EXPECT_EQ(flag.get(), false);
}

TEST(SPTK_Flag, waitFor)
{
    Flag flag;

    bool result = flag.wait_for(true, 10ms);
    EXPECT_EQ(flag.get(), false);
    EXPECT_EQ(result, false);

    result = flag.wait_for(false, 10ms);
    EXPECT_EQ(flag.get(), false);
    EXPECT_EQ(result, true);
}

TEST(SPTK_Flag, setWaitFor)
{
    Flag flag;

    flag.set(true);
    bool result = flag.wait_for(true, 10ms);
    EXPECT_EQ(flag.get(), true);
    EXPECT_EQ(result, true);
}

TEST(SPTK_Flag, adaptorAndAssignment)
{
    Flag flag;

    flag = true;
    EXPECT_EQ((bool) flag, true);

    flag = false;
    EXPECT_EQ((bool) flag, false);
}

TEST(SPTK_Flag, signalOtherThread)
{
    Flag flag;

    flag.set(false);

    auto task1 = async(launch::async,
                       [&flag]
                       {
                           if (flag.wait_for(true, 100ms))
                           {
                               COUT("Received true" << endl);
                           }
                           else
                           {
                               CERR("Timeout" << endl);
                           }
                       });

    auto task2 = async(launch::async,
                       [&flag]
                       {
                           flag.set(true);
                       });

    EXPECT_TRUE(task1.wait_for(110ms) == future_status::ready);
    task2.wait();
}
