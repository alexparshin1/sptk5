/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Thread.cpp - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/cutils>
#include <sptk5/threads/Thread.h>

using namespace std;
using namespace sptk;

namespace sptk {

    void Thread::threadStart(void* athread)
    {
        auto* th = (Thread*) athread;
        try {
            th->threadFunction();
            th->onThreadExit();
        }
        catch (const Exception& e) {
            CERR("Exception in thread '" << th->name() << "': " << e.what() << endl);
        }
    }
}

Thread::Thread(const String& name)
: m_name(name),
  m_terminated(false)
{
}

Thread::~Thread()
{
    terminate();
    if (m_thread.joinable())
        m_thread.join();
}

void Thread::terminate()
{
    UniqueLock(m_mutex);
    m_pause.post();
    m_terminated = true;
}

bool Thread::terminated()
{
    SharedLock(m_mutex);
    return m_terminated;
}

Thread::Id Thread::id()
{
    return m_thread.get_id();
}

void Thread::join()
{
    if (m_thread.joinable())
        m_thread.join();
}

void Thread::run()
{
    UniqueLock(m_mutex);
    m_terminated = false;
    m_thread = thread(threadStart, (void *) this);
}

bool Thread::sleep_for(std::chrono::milliseconds interval)
{
    return m_pause.sleep_for(interval);
}

bool Thread::sleep_until(DateTime timestamp)
{
    return m_pause.sleep_until(timestamp);
}

bool Thread::running() const
{
    return m_thread.joinable();
}

#if USE_GTEST

class ThreadTestThread: public Thread
{
    atomic_int  m_counter;
public:
    explicit ThreadTestThread(const String& threadName)
    : Thread(threadName), m_counter(0)
    {}

    void threadFunction() override
    {
        while (!terminated()) {
            m_counter++;
            sleep_for(chrono::milliseconds(50));
        }
    }

    int counter() const { return m_counter; }
};

TEST(SPTK_Thread, run)
{
    ThreadTestThread testThread("Test Thread");
    testThread.run();
    this_thread::sleep_for(chrono::milliseconds(250));
    testThread.terminate();
    testThread.join();
    EXPECT_NEAR(5, testThread.counter(),2);
}

#endif
