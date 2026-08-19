/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/cutils>
#include <sptk5/threads/WorkerThread.h>

using namespace std;
using namespace sptk;

WorkerThread::WorkerThread(SynchronizedQueue<URunable>& queue, const std::chrono::milliseconds maxIdleTime)
    : Thread("worker")
    , m_queue(queue)
    , m_maxIdleSeconds(maxIdleTime)
{
}

void WorkerThread::threadFunction()
{
    auto queueTimeout = 1s;
    auto idleSeconds = 0s;
    while (!terminated())
    {
        if (idleSeconds >= m_maxIdleSeconds)
        {
            break;
        }

        URunable runable;
        if (m_queue.pop_front(runable, queueTimeout))
        {
            setRunable(runable.get());
            idleSeconds = 0s;

            try
            {
                runable->execute();
            }
            catch (const Exception& e)
            {
                CERR("Runable::execute() : " << e.what());
            }
            setRunable(nullptr);
        }
        else
        {
            idleSeconds += queueTimeout;
        }
    }
}

void WorkerThread::execute(URunable& task) const
{
    m_queue.push_back(std::move(task));
}

void WorkerThread::setRunable(Runable* runable)
{
    const scoped_lock lock(m_mutex);
    m_currentRunable = runable;
}

void WorkerThread::terminate()
{
    const scoped_lock lock(m_mutex);
    if (m_currentRunable != nullptr)
    {
        m_currentRunable->terminate();
    }
    Thread::terminate();
    m_queue.wakeup();
}
