/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-02                                             ║
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
    if (weak_from_this().expired())
    {
        throw Exception("ThreadManager must be created as shared_ptr");
    }

    {
        const scoped_lock lock(m_mutex);
        m_terminatedQueueTimeout = 1000ms;
    }

    while (!terminated())
    {
        joinTerminatedThreads();
    }
}

void ThreadManager::joinTerminatedThreads()
{
    deque<SThread> joinThreads;

    milliseconds terminatedQueueTimeout;
    {
        const scoped_lock lock(m_mutex);
        terminatedQueueTimeout = m_terminatedQueueTimeout;
    }

    SThread thread;
    while (m_terminatedThreads.pop_front(thread, terminatedQueueTimeout))
    {
        const scoped_lock lock(m_mutex);
        terminatedQueueTimeout = m_terminatedQueueTimeout;
        thread->terminate();
        joinThreads.push_back(thread);
    }

    while (!joinThreads.empty())
    {
        thread = joinThreads.front();
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
    {
        const scoped_lock lock(m_mutex);
        m_terminatedQueueTimeout = 0ms;
        m_terminatedThreads.wakeup();
    }

    terminate();
    join();
}

void ThreadManager::terminateRunningThreads()
{
    const scoped_lock lock(m_mutex);
    for (const auto& thread: m_managedThreads)
    {
        thread->setThreadManager(nullptr);
        m_terminatedThreads.push_back(thread);
        thread->terminate();
    }
}

void ThreadManager::manage(const SThread& thread)
{
    if (thread)
    {
        const scoped_lock lock(m_mutex);
        const auto        itor = ranges::find(m_managedThreads, thread);
        if (itor == m_managedThreads.end())
        {
            const auto self = dynamic_pointer_cast<ThreadManager>(shared_from_this());
            thread->setThreadManager(self);
            m_managedThreads.push_back(thread);
        }
    }
}

void ThreadManager::destroyThread(const SThread& thread)
{
    if (thread)
    {
        const scoped_lock lock(m_mutex);

        auto matchThread = [&thread](const SThread& aThread)
        {
            return thread == aThread;
        };

        const auto itor = ranges::find_if(m_managedThreads, matchThread);
        if (itor != m_managedThreads.end())
        {
            const auto matchedThread = *itor;
            m_managedThreads.erase(itor);
            m_terminatedThreads.push_back(matchedThread);
        }
    }
}

size_t ThreadManager::threadCount() const
{
    const scoped_lock lock(m_mutex);
    return m_managedThreads.size();
}

SThread ThreadManager::getNextThread()
{
    const scoped_lock lock(m_mutex);

    if (m_managedThreads.empty())
    {
        return nullptr;
    }

    if (m_nextThreadIndex >= m_managedThreads.size())
    {
        m_nextThreadIndex = 0;
    }

    auto nextThread = m_managedThreads[m_nextThreadIndex];
    ++m_nextThreadIndex;

    return nextThread;
}
