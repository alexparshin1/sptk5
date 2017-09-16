/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       thread_pool_test.cpp - description                     ║
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

#include <sptk5/cutils>
#include <sptk5/cthreads>

using namespace std;
using namespace sptk;

class CMyTask : public Runable
{
    string          m_name; /// Task name, for distinguishing different tasks output
    Logger       m_log;  /// Task proxy log

    static uint32_t taskCount;
public:

    // Constructor
    CMyTask(FileLogEngine& sharedLog);

    // The thread function.
    void run() override;

    string name() const
    {
        return m_name;
    }
};

uint32_t CMyTask::taskCount;

CMyTask::CMyTask(FileLogEngine& sharedLog)
        :
        m_name("Task " + int2string(++taskCount)),
        m_log(sharedLog)
{
}

// The task function. Prints a message once a second till terminated
void CMyTask::run()
{
    m_log << name() << " started" << endl;

    unsigned counter = 0;
    while (!terminated()) {
        m_log << "Output " << counter << " from " << name() << endl;
        counter++;
        Thread::msleep(100);
    }

    m_log << name() << " is terminated" << endl;
}

int main(int, char* [])
{
    unsigned i;
    vector<CMyTask*> tasks;

    /// Thread manager controls tasks execution.
    ThreadPool threadPool;

    /// The log file would get messages from all the tasks.
    /// Threads send messages through their own Logger objects.
    /// Multiple Logger objects can share same log object thread-safely.
    FileLogEngine sharedLog("task_test.log");

    /// Trancate the log file
    sharedLog.reset();

    /// Adding 'duplicate messages to stdout' option to log options
    sharedLog.options(sharedLog.options() | LogEngine::LO_STDOUT);

    // Creating several tasks
    for (i = 0; i < 5; i++)
        tasks.push_back(new CMyTask(sharedLog));

    cout << tasks.size() << " tasks are created." << endl;
    Thread::msleep(100);

    cout << "Thread pool has " << threadPool.size() << " threads" << endl;

    cout << "Starting all tasks." << endl;
    for (i = 0; i < tasks.size(); i++)
        threadPool.execute(tasks[i]);

    cout << tasks.size() << " tasks are created." << endl;
    Thread::msleep(100);

    cout << "Waiting 1 seconds while tasks are running.." << endl;
    Thread::msleep(1000);

    cout << "Thread pool has " << threadPool.size() << " threads" << endl;
    cout << "Sending 'terminate' signal to all the tasks." << endl;
    for (i = 0; i < tasks.size(); i++)
        tasks[i]->terminate();
    Thread::msleep(1000);

    cout << "Thread pool has " << threadPool.size() << " threads" << endl << endl;

    cout << "Starting tasks again." << endl;
    for (i = 0; i < tasks.size(); i++)
        threadPool.execute(tasks[i]);

    cout << "Thread pool has " << threadPool.size() << " threads" << endl << endl;

    Thread::msleep(1000);

    cout << "Sending 'terminate' signal to all the tasks." << endl;
    for (i = 0; i < tasks.size(); i++)
        tasks[i]->terminate();
    Thread::msleep(1000);

    cout << "Stopping thread pool..." << endl;
    threadPool.stop();

    cout << "Deleting all the tasks." << endl;
    for (i = 0; i < tasks.size(); i++)
        delete tasks[i];

    return 0;
}
