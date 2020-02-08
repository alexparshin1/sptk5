/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

using namespace std;
using namespace sptk;
using namespace chrono;

ThreadManager::ThreadManager(const String& name)
: m_joiner(name + ".Joiner")
{
}

ThreadManager::~ThreadManager()
{
    stop();
}

ThreadManager::Joiner::Joiner(const String& name)
: Thread(name)
{
}

ThreadManager::Joiner::~Joiner()
{
    stop();
}

void ThreadManager::Joiner::threadFunction()
{
    while (!terminated()) {
        joinTerminatedThreads(std::chrono::milliseconds());
    }
}

void ThreadManager::Joiner::push(SThread& thread)
{
    m_terminatedThreads.push(thread);
}

void ThreadManager::Joiner::stop()
{
    terminate();
    join();
    joinTerminatedThreads(milliseconds(0));
}

void ThreadManager::Joiner::joinTerminatedThreads(milliseconds timeout)
{
    SThread thread;
    while (m_terminatedThreads.pop(thread, timeout)) {
        thread->terminate();
        thread->join();
    }
}

void ThreadManager::start()
{
    m_joiner.run();
}

void ThreadManager::stop()
{
    terminateRunningThreads();
    m_joiner.stop();
}

void ThreadManager::terminateRunningThreads()
{
    lock_guard<mutex> lock(m_mutex);
    for (auto& itor: m_runningThreads) {
        m_joiner.push(itor.second);
        itor.second->terminate();
    }
}

void ThreadManager::registerThread(Thread* thread)
{
    if (thread) {
        lock_guard<mutex> lock(m_mutex);
        auto itor = m_runningThreads.find(thread);
        if (itor == m_runningThreads.end()) {
            m_runningThreads[thread] = shared_ptr<Thread>(thread);
        }
    }
}

void ThreadManager::destroyThread(Thread* thread)
{
    if (thread && thread->running()) {
        lock_guard<mutex> lock(m_mutex);
        auto itor = m_runningThreads.find(thread);
        if (itor != m_runningThreads.end()) {
            auto sthread = itor->second;
            m_runningThreads.erase(itor);
            m_joiner.push(sthread);
        }
    }
}

size_t ThreadManager::threadCount() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_runningThreads.size();
}

bool ThreadManager::running() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_joiner.running();
}

#if USE_GTEST

class ThreadManagerTestThread : public Thread
{
public:
    static atomic<size_t>   taskCounter;
    static atomic<size_t>   joinCounter;

    ThreadManagerTestThread(const String& name, const shared_ptr<ThreadManager> threadManager)
    : Thread(name, threadManager)
    {
    }

    void join() override
    {
        joinCounter++;
    }

protected:
    void threadFunction() override
    {
        taskCounter++;
        sleep_for(milliseconds(10));
    }
};

atomic<size_t> ThreadManagerTestThread::taskCounter;
atomic<size_t> ThreadManagerTestThread::joinCounter;

TEST(SPTK_ThreadManager, minimal)
{
    size_t  maxThreads = 10;
    auto    threadManager = make_shared<ThreadManager>("Test Manager");

    threadManager->start();

    for (size_t i = 0; i < maxThreads; i++) {
        auto thread = new ThreadManagerTestThread("thread " + to_string(i), threadManager);
        thread->run();
    }

    this_thread::sleep_for(milliseconds(200));
    threadManager->stop();

    EXPECT_EQ(maxThreads, ThreadManagerTestThread::taskCounter);
    EXPECT_EQ(maxThreads, ThreadManagerTestThread::joinCounter);
}

#endif
