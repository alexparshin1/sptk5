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

#include <sptk5/threads/CThread.h>
#include <sptk5/CException.h>

#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace std;
using namespace sptk;

extern "C"
{
    typedef void* (*CThreadStartFunction)(void*);
}

#ifndef WIN32
void *CThread::threadStart(void *p)
{
    CThread *thread = (CThread *) p;
    thread->runThread();
    return    NULL;
}
#else
unsigned __stdcall CThread::threadStart(void *p)
{
    CThread *thread = (CThread *)p;
    thread->runThread();
    return 0;
}
#endif

CThread::CThread(string name) :
    m_terminated(false),
    m_running(false),
    m_name(name),
    m_thread(0)
{
}

CThread::~CThread()
{
    terminate();
    joinThread();
}

void CThread::terminate()
{
    if (m_terminated)
        return;
    m_terminated = true;
}

void CThread::joinThread()
{
    if (m_thread) {
#ifndef WIN32
        // wait till the thread is stopping
        pthread_join(m_thread, 0);
#else
        WaitForSingleObject(m_thread,INFINITE);
#endif
        m_thread = 0;
    }
}

void CThread::createThread()
{
#ifndef WIN32
    int rc = pthread_create(&m_thread, NULL,
            (CThreadStartFunction) CThread::threadStart, (void *) this);
    if (rc)
        throw CException("Can't create a new thread");
#else
    unsigned threadID;
    m_thread = (HANDLE) _beginthreadex(0L, 2048, CThread::threadStart, (void *) this, 0, &threadID);
    if (!m_thread)
        throw CException("Can't create a new thread");
#endif
}

void CThread::destroyThread()
{
#ifndef WIN32
    pthread_exit(NULL);
#else
    _endthreadex(0);
#endif
}

void CThread::runThread()
{
    m_running = true;
    threadFunction();
    m_running = false;
    destroyThread();
}

void CThread::run()
{
    m_terminated = false;
    m_running = true;
    createThread();
}

void CThread::msleep(int msec)
{
#ifndef WIN32
    usleep(msec * 1000L);
#else
    Sleep(msec);
#endif
}

uint64_t CThread::id()
{
#ifdef WIN32
    return GetCurrentThreadId();
#else
    return (uint64_t)pthread_self();
#endif
}

