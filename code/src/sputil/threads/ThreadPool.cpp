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

#include <sptk5/threads/ThreadPool.h>
#include <sptk5/threads/WorkerThread.h>
#include <sstream> // libc++

using namespace std;
using namespace sptk;

ThreadPool::ThreadPool(const uint32_t threadLimit, const std::chrono::milliseconds threadIdleSeconds, const String& threadName,
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

void ThreadPool::createThread()
{
    logThreadEvent("Creating worker thread", nullptr);
    const auto workerThread = make_shared<WorkerThread>(m_taskQueue, m_threadIdleTime);
    m_threadManager->manage(workerThread);
    workerThread->run();
    logThreadEvent("Started worker thread", workerThread.get());
}

void ThreadPool::logThreadEvent(const String& event, const Thread* workerThread) const
{
    static mutex mtx;
    if (m_logger)
    {
        const scoped_lock lock(mtx);
        stringstream      message;
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

void ThreadPool::execute(URunable task)
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

    if (const bool needMoreThreads = m_threadManager->threadCount() == 0 || !m_availableThreads.wait_for(tenMilliseconds);
        needMoreThreads && (m_threadLimit == 0 || m_threadManager->threadCount() < m_threadLimit))
    {
        createThread();
    }

    m_taskQueue.push_back(std::move(task));
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
