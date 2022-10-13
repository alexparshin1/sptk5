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

#include <mutex>
#include <sptk5/Printer.h>
#include <sptk5/threads/Locks.h>
#include <gtest/gtest.h>
#include <sptk5/threads/Thread.h>

using namespace std;
using namespace sptk;

class LockTestThread
    : public Thread
{
public:
    static SharedMutex amutex;

    LockTestThread()
        : Thread("test")
    {
    }

    void threadFunction() override
    {
        try
        {
            TimedUniqueLock(amutex, chrono::milliseconds(100));
            aresult = "locked";
        }
        catch (const Exception& e)
        {
            aresult = "lock timeout: " + String(e.what());
        }
    }

    String result() const
    {
        return aresult;
    }

private:
    String aresult;
};

SharedMutex LockTestThread::amutex;

TEST(SPTK_Locks, writeLockAndWait)
{
    UniqueLock(LockTestThread::amutex);
    LockTestThread th;
    th.run();
    constexpr auto smallDelay = chrono::milliseconds(200);
    this_thread::sleep_for(smallDelay);
    th.join();
    EXPECT_TRUE(th.result().startsWith("lock timeout"));
}
