/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include "sptk5/threads/Thread.h"
#include <sptk5/cutils>
#include <sptk5/threads/ThreadManager.h>
#include <utility>


using namespace std;
using namespace sptk;

Thread::Thread(String name, vector<int> ignoreSignals)
    : m_name(std::move(name))
    , m_ignoreSignals(std::move(ignoreSignals))
{
}

void Thread::terminate()
{
    m_terminated.store(true, std::memory_order_relaxed);
}

bool Thread::terminated()
{
    return m_terminated.load(std::memory_order_relaxed);
}

Thread::Id Thread::id() const
{
    const scoped_lock lock(m_mutex);
    if (m_thread)
    {
        return m_thread->get_id();
    }
    return {};
}

void Thread::join()
{
    if (running())
    {
        m_thread->join();
        const scoped_lock lock(m_mutex);
        m_thread.reset();
    }
}

void Thread::run()
{
    if (running())
    {
        return;
    }

    const scoped_lock lock(m_mutex);
    m_thread = make_shared<jthread>(
        [this]()
        {
            // Ignore signals
            for (const auto sig: m_ignoreSignals)
            {
                signal(sig, SIG_IGN);
            }

            try
            {
                m_terminated = false;
                threadFunction();
                onThreadExit();
                if (m_threadManager)
                {
                    m_threadManager->destroyThread(this);
                }
            }
            catch (const Exception& e)
            {
                CERR("Exception in thread '" << name() << "': " << e.what());
            }
        });
}

bool Thread::running() const
{
    const scoped_lock lock(m_mutex);
    return m_thread && m_thread->joinable();
}

void Thread::setThreadManager(ThreadManager* threadManager)
{
    const scoped_lock lock(m_mutex);
    m_threadManager = threadManager;
}
