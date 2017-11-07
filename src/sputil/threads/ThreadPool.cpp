/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       ThreadPool.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/threads/ThreadPool.h>

using namespace std;
using namespace sptk;

ThreadPool::ThreadPool(uint32_t threadLimit, uint32_t threadIdleSeconds) :
    Thread("thread manager"),
    m_threadLimit(threadLimit),
    m_threadIdleSeconds(threadIdleSeconds),
    m_shutdown(false)
{
    run();
}

ThreadPool::~ThreadPool()
{
    stop();
}

void ThreadPool::threadFunction()
{
    while (!terminated()) {
        WorkerThread* workerThread = nullptr;
        if (m_terminatedThreads.pop_front(workerThread, std::chrono::milliseconds(1000))) {
            m_threads.remove(workerThread);
            delete workerThread;
        }
    }
}

WorkerThread* ThreadPool::createThread()
{
    auto workerThread = new WorkerThread(&m_taskQueue, this, m_threadIdleSeconds);
    m_threads.push_back(workerThread);
    workerThread->run();
    return workerThread;
}

void ThreadPool::execute(Runable* task)
{
    SYNCHRONIZED_CODE;
    if (m_shutdown)
        throw Exception("Thread manager is stopped");

    if (!m_availableThreads.wait(std::chrono::milliseconds(10))) {
        if (m_threads.size() < m_threadLimit)
            createThread();
    }

    m_taskQueue.push(task);
}

void ThreadPool::threadEvent(Thread* thread, ThreadEvent::Type eventType)
{
    switch (eventType) {
    case ThreadEvent::RUNABLE_STARTED:
        break;
    case ThreadEvent::RUNABLE_FINISHED:
        m_availableThreads.post();
        break;
    case ThreadEvent::THREAD_FINISHED:
        m_terminatedThreads.push_back((WorkerThread*)thread);
        break;
    case ThreadEvent::THREAD_STARTED:
    case ThreadEvent::IDLE_TIMEOUT:
        break;
    }
}

static bool terminateThread(WorkerThread*& thread, void*)
{
    thread->terminate();
    return true;
}

void ThreadPool::stop()
{
    {
        SYNCHRONIZED_CODE;
        m_shutdown = true;
    }
    m_threads.each(terminateThread);
    while (!m_threads.empty())
        this_thread::sleep_for(chrono::seconds(1));
}

size_t ThreadPool::size() const
{
    return m_threads.size();
}

