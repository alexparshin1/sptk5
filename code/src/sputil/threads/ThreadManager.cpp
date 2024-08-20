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

using namespace std;
using namespace sptk;
using namespace chrono;

ThreadManager::ThreadManager(const String& name)
    : Thread(name)
{
}

ThreadManager::~ThreadManager()
{
    try
    {
        stop();
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }
}

void ThreadManager::threadFunction()
{
    constexpr auto timeout = std::chrono::milliseconds(1000);
    while (!terminated())
    {
        joinTerminatedThreads(timeout);
    }
}

void ThreadManager::joinTerminatedThreads(milliseconds timeout)
{
    deque<SThread> joinThreads;

    if (!m_terminatedThreads.empty())
    {
        SThread thread;
        while (m_terminatedThreads.pop_front(thread, timeout))
        {
            const scoped_lock lock(m_mutex);
            thread->terminate();
            joinThreads.push_back(thread);
        }
    }

    while (!joinThreads.empty())
    {
        shared_ptr<Thread> thread = joinThreads.front();
        joinThreads.pop_front();
        thread->join();
        const scoped_lock lock(m_mutex);
        thread.reset();
    }
}

void ThreadManager::start()
{
    run();
}

void ThreadManager::stop()
{
    terminateRunningThreads();
    joinTerminatedThreads(milliseconds(0));
    terminate();
    join();
}

void ThreadManager::terminateRunningThreads()
{
    const scoped_lock lock(m_mutex);
    for (const auto& thread: m_runningThreads)
    {
        m_terminatedThreads.push_back(thread);
        thread->terminate();
    }
}

void ThreadManager::manage(const SThread& thread)
{
    if (thread)
    {
        const scoped_lock lock(m_mutex);
        const auto itor = ranges::find(m_runningThreads, thread);
        if (itor == m_runningThreads.end())
        {
            thread->setThreadManager(this);
            m_runningThreads.push_back(std::move(thread));
        }
    }
}

void ThreadManager::destroyThread(const Thread* thread)
{
    if (thread && thread->running())
    {
        const scoped_lock lock(m_mutex);

        auto matchThread =
            [&thread](const SThread& aThread) {
                return thread == aThread.get();
            };

        const auto itor = ranges::find_if(m_runningThreads, matchThread);
        if (itor != m_runningThreads.end())
        {
            const auto matchedThread = *itor;
            m_runningThreads.erase(itor);
            m_terminatedThreads.push_back(matchedThread);
        }
    }
}

size_t ThreadManager::threadCount() const
{
    const scoped_lock lock(m_mutex);
    return m_runningThreads.size();
}

SThread ThreadManager::getNextThread()
{
    const scoped_lock lock(m_mutex);

    if (m_runningThreads.empty())
    {
        return nullptr;
    }

    if (m_nextThreadIndex >= m_runningThreads.size())
    {
        m_nextThreadIndex = 0;
    }

    auto nextThread = m_runningThreads[m_nextThreadIndex];
    ++m_nextThreadIndex;

    return nextThread;
}
