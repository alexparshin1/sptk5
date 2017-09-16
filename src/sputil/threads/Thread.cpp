/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Thread.cpp - description                               ║
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

#include <sptk5/threads/Thread.h>
#include <iostream>

using namespace std;
using namespace sptk;

namespace sptk {

    void Thread::threadStart(void* athread)
    {
        auto th = (Thread*) athread;
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

Thread::Thread(const string& name) :
    m_name(name),
    m_terminated(false)
{
}

Thread::~Thread()
{
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_terminated = true;
    }
    if (m_thread.joinable())
        m_thread.detach();
}

void Thread::terminate()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    m_terminated = true;
}

bool Thread::terminated()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_terminated;
}

Thread::Id Thread::id()
{
    return m_thread.get_id();
}

void Thread::join()
{
    if (m_thread.joinable())
        m_thread.join();
}

void Thread::run()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    m_terminated = false;
    m_thread = thread(threadStart, (void *) this);
}

void Thread::msleep(int msec)
{
    this_thread::sleep_for(chrono::milliseconds(msec));
}
