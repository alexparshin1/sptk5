/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WorkerThread.cpp - description                         ║
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

#include <sptk5/threads/WorkerThread.h>

using namespace std;
using namespace sptk;

WorkerThread::WorkerThread(SynchronizedQueue<Runable*>& queue, ThreadEvent* threadEvent, chrono::milliseconds maxIdleTime)
: Thread("worker"),
  m_queue(queue),
  m_threadEvent(threadEvent),
  m_maxIdleSeconds(maxIdleTime)
{}

void WorkerThread::threadFunction()
{
    if (m_threadEvent != nullptr)
        m_threadEvent->threadEvent(this, ThreadEvent::THREAD_STARTED, nullptr);

    chrono::milliseconds idleSeconds(0);
    while (!terminated()) {

        if (idleSeconds >= m_maxIdleSeconds)
            break;

        Runable* runable = nullptr;
        if (m_queue.pop(runable, chrono::milliseconds(1000))) {
            idleSeconds = chrono::milliseconds(0);
            if (m_threadEvent != nullptr)
                m_threadEvent->threadEvent(this, ThreadEvent::RUNABLE_STARTED, runable);
            try {
                runable->execute();
            }
            catch (exception& e) {
                cerr << "Runable::execute() : " << e.what() << endl;
            }
            catch (...) {
                cerr << "Runable::execute() : unknown exception" << endl;
            }
            if (m_threadEvent != nullptr)
                m_threadEvent->threadEvent(this, ThreadEvent::RUNABLE_FINISHED, runable);
        } else
            idleSeconds++;
    }
    if (m_threadEvent != nullptr)
        m_threadEvent->threadEvent(this, ThreadEvent::THREAD_FINISHED, nullptr);
}

void WorkerThread::execute(Runable* task)
{
    m_queue.push(task);
}
