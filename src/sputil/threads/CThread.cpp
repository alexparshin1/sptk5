/***************************************************************************
                         SIMPLY POWERFUL TOOLKIT (SPTK)
                         CThread.cpp  -  description
                             -------------------
    begin                : Thu Jul 12 2001
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
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

namespace sptk {
#if USE_CXX11
    void CThread::threadStart(void* athread)
    {
        CThread* th = (CThread*) athread;
        try {
            th->threadFunction();
            th->onThreadExit();
        }
        catch (exception& e) {
            cerr << "Exception in thread '" << th->name() << "': " << e.what() << endl;
        }
        catch (...) {
            cerr << "Unknown Exception in thread '" << th->name() << "'" << endl;
        }
    }
#else
    void* CThread::threadStart(void *athread)
    {
        CThread *th = (CThread *) athread;
        th->runThread();
        return NULL;
    }
#endif
}

CThread::CThread(string name) :
    m_name(name),
#if USE_CXX11 == 0
    m_id(0),
    m_thread(0),
#endif
    m_terminated(false)
{
}

CThread::~CThread()
{
    m_terminated = true;
#if USE_CXX11
    if (m_thread.joinable())
        m_thread.detach();
#else
    if (m_thread)
        pthread_detach(m_thread);
#endif
}

void CThread::terminate()
{
#if USE_CXX11
    std::lock_guard<std::mutex> lk(m_mutex);
#else
    CSynchronizedCode lk(m_mutex);
#endif
    m_terminated = true;
}

bool CThread::terminated()
{
#if USE_CXX11
    std::lock_guard<std::mutex> lk(m_mutex);
#else
    CSynchronizedCode lk(m_mutex);
#endif
    return m_terminated;
}

CThread::Id CThread::id()
{
#if USE_CXX11
    return m_thread.get_id();
#else
    return m_id;
#endif
}

void CThread::join()
{
#if USE_CXX11
    if (m_thread.joinable())
        m_thread.join();
#else
    if (m_thread) {
        pthread_join(m_thread, 0);
        m_thread = NULL;
    }
#endif
}

#if USE_CXX11 == 0
void CThread::runThread()
{
    m_id = pthread_self();
    try {
        threadFunction();
        onThreadExit();
    }
    catch (exception& e) {
        cerr << "Exception in thread '" << m_name << "': " << e.what() << endl;
    }
    catch (...) {
        cerr << "Unknown Exception in thread '" << m_name << "'" << endl;
    }
    m_id = 0;
    pthread_exit(NULL);
}
#endif

void CThread::run()
{
#if USE_CXX11
    std::lock_guard<std::mutex> lk(m_mutex);
    m_terminated = false;
    m_thread = thread(threadStart, (void *) this);
#else
    CSynchronizedCode lk(m_mutex);
    m_terminated = false;
    int rc = pthread_create(&m_thread, NULL, CThread::threadStart, (void *) this);
    if (rc)
        throw CException("Can't create a new thread");
#endif
}

void CThread::msleep(int msec)
{
#if USE_CXX11
    this_thread::sleep_for(chrono::milliseconds(msec));
#else
    usleep(msec * 1000L);
#endif
}
