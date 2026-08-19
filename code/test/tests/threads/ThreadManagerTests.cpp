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

#include "sptk5/Stopwatch.h"


#include <sptk5/Printer.h>
#include <sptk5/threads/ThreadManager.h>

#include <gtest/gtest.h>
#include <sptk5/Strings.h>

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

namespace {
void resetCounters()
{
    ThreadManagerTestThread::taskCounter = 0;
    ThreadManagerTestThread::joinCounter = 0;
}
} // namespace

/**
 * @brief Test starts several threads that each will increment the counter by 1
 * @details The resulting counter is expected to become same as number of threads
 */
namespace sptk {
TEST(ThreadManagerTests, minimal)
{
    resetCounters();

    constexpr size_t maxThreads = 10;
    const auto       threadManager = make_shared<ThreadManager>("Test Manager");

    threadManager->start();

    for (size_t i = 0; i < maxThreads; ++i)
    {
        auto thread = make_shared<ThreadManagerTestThread>(format("thread {}", i));
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
 * @brief Test starts several threads that each increments the counter by 1.
 * @details The iteration through threads should loop through each thread.
 */
TEST(ThreadManagerTests, nextThread)
{
    resetCounters();

    constexpr size_t maxThreads = 3;
    const auto       threadManager = make_shared<ThreadManager>("Test Manager");

    threadManager->start();

    for (size_t i = 0; i < maxThreads; ++i)
    {
        auto thread = make_shared<ThreadManagerTestThread>(format("thread {}", i));
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

TEST(ThreadManagerTests, stopClearsRunningState)
{
    resetCounters();

    constexpr size_t maxThreads = 3;
    const auto       threadManager = make_shared<ThreadManager>("Test Manager");

    threadManager->start();
    for (size_t i = 0; i < maxThreads; ++i)
    {
        auto thread = make_shared<ThreadManagerTestThread>(format("thread {}", i));
        threadManager->manage(thread);
        thread->run();
    }

    this_thread::sleep_for(100ms);
    threadManager->stop();

    EXPECT_EQ(0U, threadManager->threadCount());
    EXPECT_EQ(nullptr, threadManager->getNextThread());
}

TEST(ThreadManagerTests, stopReturnsPromptlyWhenIdle)
{
    resetCounters();

    const auto threadManager = make_shared<ThreadManager>("Test Manager");
    threadManager->start();

    Stopwatch stopwatch;
    stopwatch.start();
    threadManager->stop();
    stopwatch.stop();
    const auto elapsedMs = static_cast<int>(stopwatch.milliseconds());

    EXPECT_LT(elapsedMs, 300);
}

TEST(ThreadManagerTests, stopReturnsPromptlyWhenRunning)
{
    constexpr size_t maxThreads = 3;
    const auto       threadManager = make_shared<ThreadManager>("Test Manager");

    threadManager->start();
    for (size_t i = 0; i < maxThreads; ++i)
    {
        auto thread = make_shared<ThreadManagerTestThread>(format("thread {}", i));
        threadManager->manage(thread);
        thread->run();
    }

    this_thread::sleep_for(100ms);

    Stopwatch stopwatch;
    stopwatch.start();
    threadManager->stop();
    stopwatch.stop();
    const auto elapsedMs = static_cast<int>(stopwatch.milliseconds());

    EXPECT_LT(elapsedMs, 1000);
}

} // namespace sptk
