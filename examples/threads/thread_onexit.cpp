/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       thread_onexit.cpp - description                        ║
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

#if __BORLANDC__ > 0x500
#include <condefs.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <sptk5/cutils>
#include <sptk5/cthreads>

using namespace std;
using namespace sptk;

class CMyThread: public Thread
{
public:

    // Constructor
    CMyThread(string threadName);
    ~CMyThread();

    // The thread function.
    virtual void threadFunction();
    virtual void onThreadExit();
};

CMyThread::CMyThread(string threadName) :
        Thread(threadName)
{
    // Put anything you need here to define your actual thread
    cout << name() << " thread: created" << endl;
}

CMyThread::~CMyThread()
{
    cout << name() << " thread: destroyed" << endl;
}

// The thread function. Prints a message once a second till terminated
void CMyThread::threadFunction()
{
    cout << name() << " thread: started" << endl;
    int i = 0;
    while (!terminated()) {
        cout << "Output " << i << " from " << name() << endl;
        i++;
        msleep(1010);
    }
    cout << name() + " thread: terminated" << endl;
}

void CMyThread::onThreadExit()
{
    cout << name() << " thread: no longer executing" << endl;
    delete this;
}

int main(int, char*[])
{
    CMyThread *thread1 = new CMyThread("Mr. Nice");
    CMyThread *thread2 = new CMyThread("Mr. Naughty");

    thread1->run();
    thread2->run();

    cout << "Waiting 5 seconds while threads are running.." << endl;

    Thread::msleep(5000);

    thread1->terminate();
    thread2->terminate();

    return 0;
}
