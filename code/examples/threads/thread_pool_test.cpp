/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       thread_pool_test.cpp - description                     ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <sptk5/cthreads>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

SynchronizedQueue<int> intQueue;

class CMyTask
    : public Runable
{
    Logger m_log; /// Task proxy log

    static uint32_t taskCount;

public:
    // Constructor
    explicit CMyTask(SysLogEngine& sharedLog);

    // The thread function.
    void run() override;
};

uint32_t CMyTask::taskCount {1};

CMyTask::CMyTask(SysLogEngine& sharedLog)
    : Runable("Task " + int2string(taskCount))
    , m_log(sharedLog)
{
    taskCount++;
}

// The task function. Prints a message once a second till terminated
void CMyTask::run()
{
    m_log.info(name() + " started");

    while (!terminated())
    {
        int item;
        if (intQueue.pop_front(item, chrono::milliseconds(100)))
        {
            m_log.info("Output " + to_string(item) + " from " + name());
        }
    }

    m_log.info(name() + " is terminated");
}

int main()
{
    try
    {
        unsigned i;
        vector<unique_ptr<CMyTask>> tasks;
        vector<CMyTask*> taskPointers;

        /// Thread manager controls tasks execution.
        ThreadPool threadPool(16, std::chrono::milliseconds(30000), "test thread pool", nullptr);

        /// Threads send messages through their own Logger objects.
        /// Multiple Logger objects can share same log object thread-safely.
        SysLogEngine logEngine("thread_pool_test");
        logEngine.option(LogEngine::Option::STDOUT, true);

        Logger sharedLog(logEngine);

        // Creating several tasks
        for (i = 0; i < 5; i++)
        {
            auto task = make_unique<CMyTask>(logEngine);
            taskPointers.push_back(task.get());
            tasks.push_back(std::move(task));
        }

        sharedLog.log(LogPriority::Notice, "Thread pool has " + to_string(threadPool.size()) + " threads");

        sharedLog.log(LogPriority::Notice, "Starting all tasks.");
        for (auto& task: tasks)
        {
            threadPool.execute(std::move(task));
        }
        tasks.clear();

        sharedLog.log(LogPriority::Notice, to_string(tasks.size()) + " tasks are running.");

        // Let the tasks start and print start message
        this_thread::sleep_for(chrono::milliseconds(100));

        for (int value = 0; value < 100; value++)
        {
            intQueue.push_back(value);
        }

        sharedLog.log(LogPriority::Notice, "Waiting 1 seconds while tasks are running..");
        this_thread::sleep_for(chrono::milliseconds(1000));

        sharedLog.log(LogPriority::Notice, "Sending 'terminate' signal to all the tasks.");
        for (auto& task: taskPointers)
        {
            task->terminate();
        }
        this_thread::sleep_for(chrono::seconds(1));

        sharedLog.log(LogPriority::Notice, "Thread pool has " + to_string(threadPool.size()) + " threads");

        sharedLog.log(LogPriority::Notice, "Stopping thread pool...");
        threadPool.stop();

        sharedLog.log(LogPriority::Notice, "Deleting all the tasks.");

        tasks.clear();

        return 0;
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl);
        return 1;
    }
}
