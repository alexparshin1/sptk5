/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       ThreadPool.cpp - description                           ║
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

#include <sptk5/threads/ThreadPool.h>

using namespace std;
using namespace sptk;

ThreadPool::ThreadPool(uint32_t threadLimit, std::chrono::milliseconds threadIdleSeconds, const String& threadName)
: Thread(threadName),
  m_threadLimit(threadLimit),
  m_threadIdleTime(threadIdleSeconds),
  m_shutdown(false)
{
}

ThreadPool::~ThreadPool()
{
    ThreadPool::stop();
}

void ThreadPool::threadFunction()
{
    chrono::milliseconds timeout(100);
    while (!terminated()) {
        WorkerThread* workerThread = nullptr;
        if (m_terminatedThreads.pop(workerThread, timeout)) {
            m_threads.remove(workerThread);
            workerThread->join();
            delete workerThread;
        }
    }
}

WorkerThread* ThreadPool::createThread()
{
    auto* workerThread = new WorkerThread(m_taskQueue, this, m_threadIdleTime);
    m_threads.push_back(workerThread);
    workerThread->run();
    return workerThread;
}

void ThreadPool::execute(Runable* task)
{
    if (terminated())
        return;

    if (!running())
        run();

    if (m_shutdown)
        throw Exception("Thread manager is stopped");

    if (!m_availableThreads.sleep_for(std::chrono::milliseconds(10)) &&
        m_threads.size() < m_threadLimit)
            createThread();

    m_taskQueue.push(task);
}

void ThreadPool::threadEvent(Thread* athread, ThreadEvent::Type eventType, Runable*)
{
    switch (eventType) {
    case ThreadEvent::RUNABLE_STARTED:
        break;
    case ThreadEvent::RUNABLE_FINISHED:
        m_availableThreads.post();
        break;
    case ThreadEvent::THREAD_FINISHED:
        m_terminatedThreads.push((WorkerThread*)athread);
        break;
    default:
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
    m_shutdown = true;
    m_threads.each(terminateThread);
    while (!m_threads.empty())
        sleep_for(chrono::milliseconds(100));
    while (!m_terminatedThreads.empty()) {
        WorkerThread* terminatedThread;
        if (m_terminatedThreads.pop(terminatedThread, chrono::milliseconds(100))) {
            terminatedThread->join();
            delete terminatedThread;
        }
    }
    terminate();
    join();
}

size_t ThreadPool::size() const
{
    return m_threads.size();
}

#if USE_GTEST

static SynchronizedQueue<int>  intQueue;

class MyTask : public Runable
{
    atomic_int m_count {0};
public:
    MyTask() : Runable("MyTask") {}
    void run() override
    {
        while (!terminated()) {
            int item;
            if (intQueue.pop(item, chrono::milliseconds(10)))
                m_count++;
            this_thread::sleep_for(chrono::milliseconds(1));
        }
    }
    int count() const { return m_count; }
};

TEST(SPTK_ThreadPool, run)
{
    unsigned i;
    vector<MyTask*> tasks;

    /// Thread manager controls tasks execution.
    ThreadPool threadPool(16, std::chrono::milliseconds(60), "test thread pool");

    // Creating several tasks
    for (i = 0; i < 5; i++)
        tasks.push_back(new MyTask);

    for (i = 0; i < tasks.size(); i++)
        threadPool.execute(tasks[i]);

    for (int value = 0; value < 100; value++)
        intQueue.push(value);

    this_thread::sleep_for(chrono::milliseconds(300));

    EXPECT_EQ(size_t(5), tasks.size());
    for (auto* task: tasks)
        EXPECT_NEAR(20, task->count(), 10);

    for (auto* task: tasks)
        task->terminate();

    EXPECT_EQ(size_t(5), threadPool.size());

    threadPool.stop();
    EXPECT_EQ(size_t(0), threadPool.size());
}

#endif

