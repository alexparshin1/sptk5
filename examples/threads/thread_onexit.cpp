/***************************************************************************
              thread_onexit.cpp  -  test onThreadExit() function
                        -------------------
            begin                : Mon Sep 26, 2005
            copyright            : (C) 1999-2016 Ilya A. Volynets-Evenbakh
            email                : ilya@total-knowledge.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * How this works:
 * Two threads are created on heap.
 * Thread function sleeps 1010 ms and prints a message with thread
 * name in a loop, until terminated() is called. Main function sleeps 5
 * seconds , then calls terminate() on both threads.
 * Both threads should self-destruct propperly, since onThreadExit() calls
 * delete(this)
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

class CMyThread: public CThread
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
        CThread(threadName)
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

    CThread::msleep(5000);

    thread1->terminate();
    thread2->terminate();

    return 0;
}
