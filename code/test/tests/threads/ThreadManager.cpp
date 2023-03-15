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

#include "sptk5/threads/ThreadManager.h"
#include <gtest/gtest.h>

using namespace std;
using namespace chrono;
using namespace sptk;

class ThreadManagerTestThread
    : public Thread
{
public:
    static atomic<size_t> taskCounter;
    static atomic<size_t> joinCounter;

    ThreadManagerTestThread(const String& name, const shared_ptr<ThreadManager>& threadManager)
        : Thread(name, threadManager)
    {
    }

    void join() override
    {
        ++joinCounter;
    }

protected:
    void threadFunction() override
    {
        constexpr auto tenMilliseconds = milliseconds(10);
        ++taskCounter;
        this_thread::sleep_for(tenMilliseconds);
    }
};

atomic<size_t> ThreadManagerTestThread::taskCounter;
atomic<size_t> ThreadManagerTestThread::joinCounter;

TEST(SPTK_ThreadManager, minimal)
{
    constexpr size_t maxThreads = 10;
    auto threadManager = make_shared<ThreadManager>("Test Manager");

    threadManager->start();

    for (size_t i = 0; i < maxThreads; ++i)
    {
        auto* thread = new ThreadManagerTestThread("thread " + to_string(i), threadManager);
        thread->run();
    }

    constexpr auto smallDelay = milliseconds(200);
    this_thread::sleep_for(smallDelay);
    threadManager->stop();

    EXPECT_EQ(maxThreads, ThreadManagerTestThread::taskCounter);
    EXPECT_EQ(maxThreads, ThreadManagerTestThread::joinCounter);
}
