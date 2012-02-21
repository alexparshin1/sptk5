/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CThread.cpp  -  description
                             -------------------
    begin                : Thu Jul 12 2001
    copyright            : (C) 2001-2012 by Alexey Parshin. All rights reserved.
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

#include <sptk5/CThread.h>
#include <sptk5/CException.h>

#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace std;
using namespace sptk;

extern "C" {
  typedef void* (*CThreadStartFunction)(void*);
}

#ifndef _WIN32
void *CThread::threadStart(void *p) {
   pthread_cleanup_push(threadCleanup,p);
   CThread *thread = (CThread *)p;
   thread->runThread();
   pthread_cleanup_pop(1);
   return NULL;
}
#else
unsigned __stdcall CThread::threadStart(void *p) {
   CThread *thread = (CThread *)p;
   thread->runThread();
   return 0;
}
#endif

///@todo implement windows version of cleanup registration
void CThread::threadCleanup(void *p) {
   CThread *thread = (CThread *)p;
   thread->onThreadExit();
}

CThread::CThread(string name,bool politeMode,CFileLog* fileLog) {
   m_name = name;
   m_terminated = false;
   m_running = false;
   m_politeMode = politeMode;
   m_log = fileLog;
   if (m_log) *m_log << "Thread " << m_name << ": constructor completed" << endl;
}

CThread::~CThread() {
   terminate();
   if (m_log) *m_log << "Thread " << m_name << ": destructor completed" << endl;
}

void CThread::terminate()  {
   if (m_terminated) return;
   m_terminated = true;

   if (m_log) *m_log << "Thread " << m_name << ": termination started" << endl;
   if (m_running) {
      if (m_politeMode) {
#ifndef _WIN32
         // wait till the thread is stopping
         pthread_join(m_thread,0);
#else
         WaitForSingleObject(m_thread,INFINITE);
         m_thread = 0;
#endif
      } else {
#ifndef _WIN32
         // stop this thread _now_
         pthread_cancel(m_thread);
#else
         CloseHandle(m_thread);
#endif
      }
   }
   if (m_log) *m_log << "Thread " << m_name << ": termination completed" << endl;
}

void CThread::createThread() {
#ifndef _WIN32
   int rc = pthread_create(&m_thread, NULL, (CThreadStartFunction) CThread::threadStart, (void *) this);
   if (rc)
      throw CException("Can't create a new thread");
#else
   unsigned threadID;
   m_thread = (HANDLE) _beginthreadex(0L, 2048, CThread::threadStart, (void *) this, 0, &threadID);
   if (!m_thread)
      throw CException("Can't create a new thread");
#endif
   if (m_log) *m_log << "Thread " << m_name << ": started" << endl;
}

void CThread::destroyThread() {
   if (m_log) *m_log << "Thread " << m_name << ": destroy started" << endl;
#ifndef _WIN32
   pthread_exit(NULL);
#else
   _endthreadex(0);
#endif
   if (m_log) *m_log << "Thread " << m_name << ": destroy completed" << endl;
}

void CThread::runThread() {
   m_running = true;
   threadFunction();
   m_running = false;
   destroyThread();
}

void CThread::run() {
   m_terminated = false;
   m_running = true;
   createThread();
}

void CThread::msleep(int msec) {
#ifndef _WIN32
   usleep(msec * 1000L);
#else
   Sleep(msec);
#endif
}
