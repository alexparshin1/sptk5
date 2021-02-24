/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
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

#include <sptk5/threads/ThreadPool.h>

using namespace std;
using namespace sptk;

ThreadPool::ThreadPool(uint32_t threadLimit, std::chrono::milliseconds threadIdleSeconds, const String& threadName,
                       LogEngine* logEngine)
: m_threadManager(make_shared<ThreadManager>(threadName + ".ThreadManager")),
  m_threadLimit(threadLimit),
  m_threadIdleTime(threadIdleSeconds)
{
    if (logEngine != nullptr)
        m_logger = make_shared<Logger>(*logEngine);
}

WorkerThread* ThreadPool::createThread()
{
    auto* workerThread = new WorkerThread(m_threadManager, m_taskQueue, this, m_threadIdleTime);
    workerThread->run();
    logThreadEvent("Started thread", workerThread);
    return workerThread;
}

void ThreadPool::logThreadEvent(const String& event, const Thread* workerThread)
{
    static mutex mtx;
    if (m_logger) {
        lock_guard<mutex> lock(mtx);
        stringstream message;
        message << event << " " << workerThread->id();
        m_logger->debug(message.str());
    }
}

void ThreadPool::execute(Runable* task)
{
    if (m_shutdown)
        throw Exception("Thread pool is stopped");

    if (!m_threadManager->running())
        m_threadManager->start();

    if (!m_availableThreads.sleep_for(std::chrono::milliseconds(10)) &&
        (m_threadLimit == 0 || m_threadManager->threadCount() < m_threadLimit))
            createThread();

    m_taskQueue.push(task);
}

void ThreadPool::threadEvent(Thread* workerThread, ThreadEvent::Type eventType, Runable*)
{
    switch (eventType) {
    case ThreadEvent::RUNABLE_STARTED:
        logThreadEvent("Runable started", workerThread);
        break;
    case ThreadEvent::RUNABLE_FINISHED:
        m_availableThreads.post();
        logThreadEvent("Runable finished", workerThread);
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

#if USE_GTEST

class MyTask : public Runable
{
public:
    static SynchronizedQueue<int>  intQueue;

    MyTask() : Runable("MyTask") {}
    void run() override
    {
        while (!terminated()) {
            int item;
            if (intQueue.pop(item, chrono::milliseconds(10))) {
                ++m_count;
            }
            this_thread::sleep_for(chrono::milliseconds(1));
        }
    }
    int count() const { return m_count; }
private:
    atomic_int      m_count {0};
};

SynchronizedQueue<int> MyTask::intQueue;

TEST(SPTK_ThreadPool, run)
{
    unsigned i;
    vector<MyTask*> tasks;

    /// Thread manager controls tasks execution.
    auto* threadPool = new ThreadPool(16, std::chrono::milliseconds(60), "test thread pool", nullptr);

    // Creating several tasks
    for (i = 0; i < 5; ++i)
        tasks.push_back(new MyTask);

    for (i = 0; i < tasks.size(); ++i)
        threadPool->execute(tasks[i]);

    for (int value = 0; value < 100; ++value)
        MyTask::intQueue.push(value);

    this_thread::sleep_for(chrono::milliseconds(300));

    EXPECT_EQ(size_t(5), tasks.size());
    for (const auto* task: tasks)
        EXPECT_NEAR(20, task->count(), 10);

    EXPECT_EQ(size_t(5), threadPool->size());

    threadPool->stop();
    EXPECT_EQ(size_t(0), threadPool->size());

    delete threadPool;

    for (auto* task: tasks) {
        task->terminate();
        delete task;
    }
}

#endif

