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
    catch (const Exception&)
    {
        // suppress any exceptions
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
    queue<SThread> joinThreads;
    {
        SThread thread;
        while (m_terminatedThreads.pop(thread, timeout))
        {
            const scoped_lock lock(m_mutex);
            thread->terminate();
            joinThreads.push(thread);
        }
    }

    while (!joinThreads.empty())
    {
        shared_ptr<Thread> thread = joinThreads.front();
        joinThreads.pop();
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
    for (const auto& [thread, threadSPtr]: m_runningThreads)
    {
        m_terminatedThreads.push(threadSPtr);
        threadSPtr->terminate();
    }
}

void ThreadManager::manage(const SThread& thread)
{
    if (thread)
    {
        const scoped_lock lock(m_mutex);
        auto itor = m_runningThreads.find(thread.get());
        if (itor == m_runningThreads.end())
        {
            thread->setThreadManager(this);
            m_runningThreads[thread.get()] = std::move(thread);
        }
    }
}

void ThreadManager::destroyThread(Thread* thread)
{
    if (thread && thread->running())
    {
        const scoped_lock lock(m_mutex);
        auto itor = m_runningThreads.find(thread);
        if (itor != m_runningThreads.end())
        {
            auto sthread = itor->second;
            m_runningThreads.erase(itor);
            m_terminatedThreads.push(sthread);
        }
    }
}

size_t ThreadManager::threadCount() const
{
    const scoped_lock lock(m_mutex);
    return m_runningThreads.size();
}
