/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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
#include "sptk5/cutils"

using namespace std;
using namespace sptk;
using namespace chrono;

ThreadManager::ThreadManager(const String& name)
: m_joiner(name + "::joiner")
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
        joinTerminatedThreads();
    }
}

void ThreadManager::Joiner::push(SThread& thread)
{
    m_terminatedThreads.push(thread);
}

void ThreadManager::Joiner::stop()
{
    joinTerminatedThreads();
    terminate();
    join();
}

void ThreadManager::Joiner::joinTerminatedThreads()
{
    SThread thread;
    while (m_terminatedThreads.pop(thread, milliseconds(100))) {
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
    for (auto& itor: m_runningThreads)
        m_joiner.push(itor.second);
}

void ThreadManager::registerThread(SThread thread)
{
    if (thread) {
        lock_guard<mutex> lock(m_mutex);
        auto itor = m_runningThreads.find(thread.get());
        if (itor == m_runningThreads.end()) {
            m_runningThreads[thread.get()] = thread;
        }
    }
}

void ThreadManager::finalizeThread(Thread* thread)
{
    if (thread) {
        lock_guard<mutex> lock(m_mutex);
        auto itor = m_runningThreads.find(thread);
        if (itor != m_runningThreads.end()) {
            auto sthread = itor->second;
            m_runningThreads.erase(itor);
            m_joiner.push(sthread);
        }
    }
}

#if USE_GTEST

class ThreadManagerTestThread : public Thread
{
public:
    ThreadManagerTestThread(const String& name, const shared_ptr<ThreadManager> threadManager)
    : Thread(name, threadManager)
    {
        COUT("Thread " << this->name() << " created" << endl);
    }

    ~ThreadManagerTestThread() override
    {
        COUT("Thread " << name() << " destroyed" << endl);
    }

protected:
    void threadFunction() override
    {
        COUT("Thread " << name() << " running" << endl);
        sleep_for(milliseconds(10));
    }
};

TEST(SPTK_ThreadManager, minimal)
{
    size_t  maxThreads = 10;
    auto    threadManager = make_shared<ThreadManager>("Test Manager");

    threadManager->start();

    vector<shared_ptr<ThreadManagerTestThread>> threads;
    for (size_t i = 0; i < maxThreads; i++) {
        auto thread = make_shared<ThreadManagerTestThread>("thread " + to_string(i), threadManager);
        threads.push_back(thread);
        thread->run();
    }

    this_thread::sleep_for(milliseconds(200));
    threadManager->stop();
}

#endif
