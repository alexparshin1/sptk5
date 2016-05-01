/***************************************************************************
                         SIMPLY POWERFUL TOOLKIT (SPTK)
                         CThread.cpp  -  description
                             -------------------
    begin                : Thu Jul 12 2001
    copyright            : (C) 1999-2016 by Alexey Parshin. All rights reserved.
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
}

CThread::CThread(string name) :
    m_name(name),
    m_terminated(false)
{
}

CThread::~CThread()
{
    m_terminated = true;
    if (m_thread.joinable())
        m_thread.detach();
}

void CThread::terminate()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    m_terminated = true;
}

bool CThread::terminated()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_terminated;
}

CThread::Id CThread::id()
{
    return m_thread.get_id();
}

void CThread::join()
{
    if (m_thread.joinable())
        m_thread.join();
}

void CThread::run()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    m_terminated = false;
    m_thread = thread(threadStart, (void *) this);
}

void CThread::msleep(int msec)
{
    this_thread::sleep_for(chrono::milliseconds(msec));
}
