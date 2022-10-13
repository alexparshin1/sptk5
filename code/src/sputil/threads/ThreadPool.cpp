/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

ThreadPool::ThreadPool(uint32_t threadLimit, std::chrono::milliseconds threadIdleSeconds, const String& threadName,
                       LogEngine* logEngine)
    : m_threadManager(make_shared<ThreadManager>(threadName + ".ThreadManager"))
    , m_threadLimit(threadLimit)
    , m_threadIdleTime(threadIdleSeconds)
{
    if (logEngine != nullptr)
    {
        m_logger = make_shared<Logger>(*logEngine);
    }
}

WorkerThread* ThreadPool::createThread()
{
    logThreadEvent("Creating worker thread", nullptr);
    auto* workerThread = new WorkerThread(m_threadManager, m_taskQueue, this, m_threadIdleTime);
    workerThread->run();
    logThreadEvent("Started worker thread", workerThread);
    return workerThread;
}

void ThreadPool::logThreadEvent(const String& event, const Thread* workerThread) const
{
    static mutex mtx;
    if (m_logger)
    {
        scoped_lock lock(mtx);
        stringstream message;
        if (workerThread != nullptr)
        {
            message << event << " " << workerThread->id();
        }
        else
        {
            message << event;
        }
        m_logger->debug(message.str());
    }
}

void ThreadPool::execute(const SRunable& task)
{
    if (m_shutdown)
    {
        throw Exception("Thread pool is stopped");
    }

    if (!m_threadManager->running())
    {
        m_threadManager->start();
    }

    constexpr std::chrono::milliseconds tenMilliseconds(10);

    if (bool needMoreThreads = m_threadManager->threadCount() == 0 || !m_availableThreads.sleep_for(tenMilliseconds);
        needMoreThreads && (m_threadLimit == 0 || m_threadManager->threadCount() < m_threadLimit))
    {
        createThread();
    }

    m_taskQueue.push(task);
}

void ThreadPool::threadEvent(Thread* thread, Type eventType, SRunable runable)
{
    switch (eventType)
    {
        case ThreadEvent::Type::RUNABLE_STARTED:
            logThreadEvent("Runable started", thread);
            break;
        case ThreadEvent::Type::RUNABLE_FINISHED:
            m_availableThreads.post();
            logThreadEvent("Runable finished", thread);
            break;
        default:
            break;
    }
}

void ThreadPool::stop()
{
    m_shutdown = true;
    m_threadManager->stop();
}

size_t ThreadPool::size() const
{
    return m_threadManager->threadCount();
}

