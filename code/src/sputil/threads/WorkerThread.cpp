/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/threads/WorkerThread.h>

using namespace std;
using namespace sptk;

WorkerThread::WorkerThread(const SThreadManager& threadManager, SynchronizedQueue<SRunable>& queue,
                           ThreadEvent* threadEvent, chrono::milliseconds maxIdleTime)
    : Thread("worker", threadManager)
    , m_queue(queue)
    , m_threadEvent(threadEvent)
    , m_maxIdleSeconds(
          maxIdleTime)
{
}

void WorkerThread::threadFunction()
{
    if (m_threadEvent != nullptr)
    {
        m_threadEvent->threadEvent(this, ThreadEvent::Type::THREAD_STARTED, nullptr);
    }

    constexpr chrono::seconds oneSecond(1);
    chrono::milliseconds idleSeconds(0);
    while (!terminated())
    {

        if (idleSeconds >= m_maxIdleSeconds)
        {
            break;
        }

        SRunable runable;
        if (m_queue.pop(runable, oneSecond))
        {
            setRunable(runable);
            idleSeconds = chrono::milliseconds(0);
            if (m_threadEvent != nullptr)
            {
                m_threadEvent->threadEvent(this, ThreadEvent::Type::RUNABLE_STARTED, runable);
            }
            try
            {
                runable->execute();
            }
            catch (const Exception& e)
            {
                CERR("Runable::execute() : " << e.what() << endl)
            }
            setRunable(nullptr);
            if (m_threadEvent != nullptr)
            {
                m_threadEvent->threadEvent(this, ThreadEvent::Type::RUNABLE_FINISHED, runable);
            }
        }
        else
        {
            ++idleSeconds;
        }
    }
    if (m_threadEvent != nullptr)
    {
        m_threadEvent->threadEvent(this, ThreadEvent::Type::THREAD_FINISHED, nullptr);
    }
}

void WorkerThread::execute(const SRunable& task)
{
    m_queue.push(task);
}

void WorkerThread::setRunable(const SRunable& runable)
{
    scoped_lock lock(m_mutex);
    m_currentRunable = runable;
}

void WorkerThread::terminate()
{
    scoped_lock lock(m_mutex);
    if (m_currentRunable != nullptr)
    {
        m_currentRunable->terminate();
    }
    Thread::terminate();
    m_queue.wakeup();
}
