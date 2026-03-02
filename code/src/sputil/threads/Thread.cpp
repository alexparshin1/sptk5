/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-02                                             ║
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

Thread::Thread(String name)
    : m_name(std::move(name))
{
}

void Thread::terminate()
{
    m_terminated = true;
}

bool Thread::terminated()
{
    return m_terminated;
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
        try
        {
            m_thread->join();
            const scoped_lock lock(m_mutex);
            m_thread.reset();
        }
        catch (const exception& e)
        {
            CERR(format("Exception in thread '{}': {}.", name().c_str(), e.what()));
        }
        catch (...)
        {
            CERR(format("Unknown exception in thread '{}'.", name().c_str()));
        }
    }
}

void Thread::run()
{
    const scoped_lock lock(m_mutex);

    if (weak_from_this().expired())
    {
        throw Exception("Thread must be created as shared_ptr");
    }

    if (m_thread && m_thread->joinable())
    {
        return;
    }

    m_terminated = false;

    m_thread = make_shared<jthread>(
        [this]()
        {
            try
            {
                threadFunction();
                onThreadExit();
                if (m_threadManager)
                {
                    m_threadManager->destroyThread(shared_from_this());
                }
            }
            catch (const exception& e)
            {
                CERR(format("Exception in thread '{}': {}.", name().c_str(), e.what()));
            }
            catch (...)
            {
                CERR(format("Unknown exception in thread '{}'.", name().c_str()));
            }
        });
}

bool Thread::running() const
{
    const scoped_lock lock(m_mutex);
    return m_thread && m_thread->joinable();
}

void Thread::setThreadManager(shared_ptr<ThreadManager> threadManager)
{
    const scoped_lock lock(m_mutex);
    m_threadManager = std::move(threadManager);
}
