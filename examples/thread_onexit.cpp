/***************************************************************************
                          thread_onexit.cpp  -  test onThreadExit() function
                             -------------------
    begin                : Mon Sep 26, 2005
    copyright            : (C) 2005-2012 Ilya A. Volynets-Evenbakh
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
 * Two threads are created on heap - one in polite mode and one in impolite
 * mode. Thread function sleeps 1010 ms and prints a message with thread
 * name in a loop, untill terminated() isn't set. Main function sleeps 5
 * seconds , then calls terminate() on both threads.
 * Polite thread should  tell us it's been terminated.
 * Impolite thread should die right away.
 * Both threads should selfdestruct propperly, since onThreadExit() calls
 * delete(this)
 */

#if __BORLANDC__ > 0x500
#include <condefs.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <sptk5/CThread.h>
#include <sptk5/CDateTime.h>
#include <string>

using namespace std;
using namespace sptk;

class CMyThread : public CThread {
public:
   
   // Constructor
   CMyThread(string threadName, bool politeMode=true);
   ~CMyThread();
   
   // The thread function.
   virtual void threadFunction();
   virtual void onThreadExit();
};

CMyThread::CMyThread(string threadName, bool politeMode) : CThread(threadName, politeMode) {
   // Put anything you need here to define your actual thread
   puts((name()+" is created").c_str());
}

CMyThread::~CMyThread() {
   printf("%s> bye, the cruel world\n", name().c_str());
}
// The thread function. Prints a message once a second till terminated
void CMyThread::threadFunction() {
   puts((name()+" is started").c_str());
   int i=0;
   while (!terminated()) {
      printf("Output %d from %s\n", i++, name().c_str());
      msleep(1010);
   }
   puts((name()+" is terminated").c_str());
}

void CMyThread::onThreadExit() {
   printf("%s> thread is no longer executing\n", name().c_str());
   delete this;
}

int main(int argc, char* argv[]) {
   CMyThread   *thread1=new CMyThread("Mr. Nice", true);
   CMyThread   *thread2=new CMyThread("Mr. Naughty", false);
   
   thread1->run();
   thread2->run();
   
   puts("Waiting 5 seconds while threads are running..");
   
   CThread::msleep(5000);
   
   thread2->terminate();
   puts("thread2 completed termination");
   thread1->terminate();
   
   return 0;
}
