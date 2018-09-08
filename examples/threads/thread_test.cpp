/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       thread_test.cpp - description                          ║
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

class CMyThread: public Thread
{
    Logger m_log; /// Thread proxy log
public:

    // Constructor
    CMyThread(const string& threadName, FileLogEngine& sharedLog);

    // The thread function.
    void threadFunction() override;
};

CMyThread::CMyThread(const string& threadName, FileLogEngine& sharedLog) :
        Thread(threadName), m_log(sharedLog)
{
    // Put anything you need here to define your actual thread
    m_log.info(name() + " is created");
}

// The thread function. Prints a message once a second till terminated
void CMyThread::threadFunction()
{
    m_log.info(name() + " is started");

    unsigned counter = 0;
    while (!terminated()) {
        m_log.info("Output (1) " + to_string(counter) + " from " + name());
        m_log.info("Output (2) " + to_string(counter) + " from " + name());
        counter++;
        //msleep(1);
    }

    m_log.notice(name() + " is terminated");
}

int main()
{
    unsigned i;
    vector<CMyThread*> threads;

    /// The log file would get messages from all the threads.
    /// Threads send messages through their own Logger objects.
    /// Multiple Logger objects can share same log object thread-safely.
    FileLogEngine sharedLog("thread_test.log");
    Logger  log(sharedLog);

    try {

        /// Trancate the log file
        sharedLog.reset();

        /// Adding 'duplicate messages to stdout' option to log options
        sharedLog.options(sharedLog.options() | LogEngine::LO_STDOUT);

        // Creating several threads
        for (i = 0; i < 5; i++) {
            string threadName = "Thread" + int2string(i);
            threads.push_back(new CMyThread(threadName, sharedLog));
        }

        // Starting all the threads
        for (i = 0; i < threads.size(); i++)
            threads[i]->run();

        log.info("Waiting 1 second while threads are running..");
        this_thread::sleep_for(chrono::seconds(1));

        log.info("Sending 'terminate' signal to all the threads.");
        // That signal suggests thread to terminate and exits ASAP.
        for (i = 0; i < threads.size(); i++)
            threads[i]->terminate();

        log.info("Joining all the threads.");
        for (i = 0; i < threads.size(); i++)
            threads[i]->join();

        log.info("Deleting all the threads.");
        // Since threads are created in polite mode (see CMyThread class definition),
        // the delete operation would wait for actual thread termination.
        for (i = 0; i < threads.size(); i++)
            delete threads[i];

        return 0;
    }
    catch (const exception& e) {
        log.error(e.what());
        return 1;
    }
}
