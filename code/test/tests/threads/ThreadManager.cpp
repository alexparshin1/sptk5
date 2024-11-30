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

#include <sptk5/Printer.h>
#include <sptk5/threads/ThreadManager.h>

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

    explicit ThreadManagerTestThread(const String& name)
        : Thread(name)
    {
    }

    void join() override
    {
        if (running())
        {
            ++joinCounter;
            Thread::join();
        }
    }

protected:
    void threadFunction() override
    {
        constexpr auto tenMilliseconds = milliseconds(50);
        ++taskCounter;
        this_thread::sleep_for(tenMilliseconds);
    }
};

atomic<size_t> ThreadManagerTestThread::taskCounter;
atomic<size_t> ThreadManagerTestThread::joinCounter;

/**
 * @brief Test starts several threads that each will increment the counter by 1
 * @details The resulting counter is expected to become same as number of threads
 */
TEST(SPTK_ThreadManager, minimal)
{
    constexpr size_t maxThreads = 10;
    const auto threadManager = make_shared<ThreadManager>("Test Manager");

    threadManager->start();

    for (size_t i = 0; i < maxThreads; ++i)
    {
        auto thread = make_shared<ThreadManagerTestThread>("thread " + to_string(i));
        threadManager->manage(thread);
        thread->run();
    }

    constexpr auto smallDelay = milliseconds(50);
    this_thread::sleep_for(smallDelay);

    threadManager->stop();

    EXPECT_EQ(maxThreads, ThreadManagerTestThread::taskCounter);
    EXPECT_EQ(maxThreads, ThreadManagerTestThread::joinCounter);
}

/**
 * @brief Test starts several threads that each will increment the counter by 1
 * @details The iteration through threads should loop through each thread
 */
TEST(SPTK_ThreadManager, nextThread)
{
    constexpr size_t maxThreads = 3;
    const auto threadManager = make_shared<ThreadManager>("Test Manager");

    threadManager->start();

    for (size_t i = 0; i < maxThreads; ++i)
    {
        auto thread = make_shared<ThreadManagerTestThread>("thread " + to_string(i));
        threadManager->manage(thread);
        thread->run();
    }

    Strings threadNames;
    for (size_t index = 0; index <= maxThreads; ++index)
    {
        const auto thread = threadManager->getNextThread();
        threadNames.push_back(thread->name());
    }

    threadManager->stop();

    EXPECT_STREQ("thread 0, thread 1, thread 2, thread 0", threadNames.join(", ").c_str());
}
