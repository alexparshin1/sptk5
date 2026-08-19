/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <future>
#include <gtest/gtest.h>
#include <mutex>
#include <sptk5/Printer.h>
#include <sptk5/cthreads>

using namespace std;
using namespace chrono;
using namespace sptk;
namespace sptk {

TEST(FlagTests, ctor)
{
    const Flag flag;
    EXPECT_EQ(flag.get(), false);
}

TEST(FlagTests, waitFor)
{
    Flag flag;

    bool result = flag.wait_for(true, 10ms);
    EXPECT_EQ(flag.get(), false);
    EXPECT_EQ(result, false);

    result = flag.wait_for(false, 10ms);
    EXPECT_EQ(flag.get(), false);
    EXPECT_EQ(result, true);
}

TEST(FlagTests, setWaitFor)
{
    Flag flag;

    flag.set(true);
    const bool result = flag.wait_for(true, 10ms);
    EXPECT_EQ(flag.get(), true);
    EXPECT_EQ(result, true);
}

TEST(FlagTests, adaptorAndAssignment)
{
    Flag flag;

    flag = true;
    EXPECT_EQ(static_cast<bool>(flag), true);

    flag = false;
    EXPECT_EQ(static_cast<bool>(flag), false);
}

TEST(FlagTests, signalOtherThread)
{
    Flag flag;

    flag.set(false);

    const auto task1 = async(launch::async,
                             [&flag]
                             {
                                 if (flag.wait_for(true, 100ms))
                                 {
                                     COUT("Received true");
                                 }
                                 else
                                 {
                                     CERR("Timeout");
                                 }
                             });

    const auto task2 = async(launch::async,
                             [&flag]
                             {
                                 flag.set(true);
                             });

    EXPECT_TRUE(task1.wait_for(110ms) == future_status::ready);
    task2.wait();
}

TEST(FlagTests, waitUntilTimeout)
{
    Flag flag;

    const bool result = flag.wait_until(true, DateTime::Now() + 10ms);
    EXPECT_FALSE(result);
    EXPECT_FALSE(flag.get());
}

TEST(FlagTests, waitUntilSucceedsWhenSignaled)
{
    Flag flag;

    auto waiter = async(launch::async,
                        [&flag]
                        {
                            return flag.wait_until(true, DateTime::Now() + 100ms);
                        });

    this_thread::sleep_for(5ms);
    flag.set(true);

    EXPECT_EQ(waiter.wait_for(150ms), future_status::ready);
    EXPECT_TRUE(waiter.get());
}

} // namespace sptk
