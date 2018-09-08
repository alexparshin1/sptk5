/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       thread_pool_test.cpp - description                     ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/cutils>
#include <sptk5/cthreads>

using namespace std;
using namespace sptk;

SynchronizedQueue<int>  intQueue;

class CMyTask : public Runable
{
    string       m_name; /// Task name, for distinguishing different tasks output
    Logger       m_log;  /// Task proxy log

    static uint32_t taskCount;
public:

    // Constructor
    explicit CMyTask(SysLogEngine& sharedLog);

    // The thread function.
    void run() override;

    string name() const
    {
        return m_name;
    }
};

uint32_t CMyTask::taskCount;

CMyTask::CMyTask(SysLogEngine& sharedLog)
        :
        m_name("Task " + int2string(++taskCount)),
        m_log(sharedLog)
{
}

// The task function. Prints a message once a second till terminated
void CMyTask::run()
{
    m_log.info(name() + " started");

    while (!terminated()) {
        int item;
        if (intQueue.pop(item, chrono::milliseconds(100))) {
            m_log.info("Output " + to_string(item) + " from " + name());
        }
    }

    m_log.info(name() + " is terminated");
}

int main(int, char* [])
{
    try {
        unsigned i;
        vector<CMyTask*> tasks;

        /// Thread manager controls tasks execution.
        ThreadPool threadPool;

        /// Threads send messages through their own Logger objects.
        /// Multiple Logger objects can share same log object thread-safely.
        SysLogEngine logEngine("thread_pool_test");
        logEngine.options(logEngine.options() | LogEngine::LO_STDOUT);

        Logger sharedLog(logEngine);

        // Creating several tasks
        for (i = 0; i < 5; i++)
            tasks.push_back(new CMyTask(logEngine));

        sharedLog.log(LP_NOTICE, "Thread pool has " + to_string(threadPool.size()) + " threads");

        sharedLog.log(LP_NOTICE, "Starting all tasks.");
        for (i = 0; i < tasks.size(); i++)
            threadPool.execute(tasks[i]);

        sharedLog.log(LP_NOTICE, to_string(tasks.size()) + " tasks are running.");

        // Let the tasks start and print start message
        this_thread::sleep_for(chrono::milliseconds(100));

        for (int value = 0; value < 100; value++)
            intQueue.push(value);

        sharedLog.log(LP_NOTICE, "Waiting 1 seconds while tasks are running..");
        this_thread::sleep_for(chrono::milliseconds(1000));

        sharedLog.log(LP_NOTICE, "Sending 'terminate' signal to all the tasks.");
        for (i = 0; i < tasks.size(); i++)
            tasks[i]->terminate();
        this_thread::sleep_for(chrono::seconds(1));

        sharedLog.log(LP_NOTICE, "Thread pool has " + to_string(threadPool.size()) + " threads");

        sharedLog.log(LP_NOTICE, "Stopping thread pool...");
        threadPool.stop();

        sharedLog.log(LP_NOTICE, "Deleting all the tasks.");
        for (i = 0; i < tasks.size(); i++)
            delete tasks[i];

        return 0;
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
