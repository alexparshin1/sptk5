/***************************************************************************
                         SIMPLY POWERFUL TOOLKIT (SPTK)
                         threadman_test.cpp  -  description
                             -------------------
    begin                : Tue Dec 14 1999
    copyright            : (C) 1999 by Alexey Parshin
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
 This library is free software; you can redistribute it and/or modify it
 under the terms of the GNU Library General Public License as published by
 the Free Software Foundation; either version 2 of the License, or (at
 your option) any later version.

 This library is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
 General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; if not, write to the Free Software Foundation,
 Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

 Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

// This example demonstrates basic thread manager usage
#if __BORLANDC__ > 0x500
#include <condefs.h>
#endif

#include <sptk5/cutils>
#include <string>
#include <vector>
#include <iostream>
#include <sys/stat.h>

using namespace std;
using namespace sptk;

class CMyTask: public CRunable
{
    string      m_name; /// Task name, for distinguishing different tasks output
    CProxyLog   m_log;  /// Task proxy log

    static uint32_t taskCount;
public:

    // Constructor
    CMyTask(CBaseLog& sharedLog);

    // The thread function.
    virtual void run() throw (exception);

    string name() const
    {
        return m_name;
    }
};

uint32_t CMyTask::taskCount;

CMyTask::CMyTask(CBaseLog& sharedLog) :
    m_name("Task " + int2string(++taskCount)),
    m_log(sharedLog)
{
}

// The task function. Prints a message once a second till terminated
void CMyTask::run() throw (exception)
{
    m_log << name() << " is started" << endl;

    unsigned counter = 0;
    while (!terminated()) {
        m_log << "Output " << counter << " from " << name() << endl;
        counter++;
        CThread::msleep(100);
    }

    m_log << name() << " is terminated" << endl;
}

int main(int argc, char* argv[])
{
    unsigned i;
    vector<CMyTask*> tasks;

    /// Thread manager controls tasks execution.
    CThreadManager threadManager;

    /// The log file would get messages from all the tasks.
    /// Threads send messages through their own CProxyLog objects.
    /// Multiple CProxyLog objects can share same log object thread-safely.
    CFileLog sharedLog("task_test.log");

    /// Trancate the log file
    sharedLog.reset();

    /// Adding 'duplicate messages to stdout' option to log options
    sharedLog.options(sharedLog.options() | CBaseLog::CLO_STDOUT);

    // Creating several tasks
    for (i = 0; i < 5; i++)
        tasks.push_back(new CMyTask(sharedLog));

    // Starting all the tasks
    for (i = 0; i < tasks.size(); i++)
        threadManager.execute(tasks[i]);

    puts("Waiting 10 seconds while tasks are running..");
    CThread::msleep(10000);

    // Sending 'terminate' signal to all the tasks.
    // That signal suggests task to terminate and exits instantly.
    for (i = 0; i < tasks.size(); i++)
        tasks[i]->terminate();

    // Deleting all the tasks.
    // Since tasks are created in polite mode (see CMyTask class definition),
    // the delete operation would wait for actual task termination.
    for (i = 0; i < tasks.size(); i++)
        delete tasks[i];

    return 0;
}
