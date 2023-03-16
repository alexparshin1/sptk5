/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/threads/ThreadManager.h>
#include <utility>

using namespace std;
using namespace sptk;

void Thread::threadStart()
{
    try
    {
        if (m_threadManager)
        {
            m_threadManager->registerThread(this);
        }
        threadFunction();
        onThreadExit();
        if (m_threadManager)
        {
            m_threadManager->destroyThread(this);
        }
    }
    catch (const Exception& e)
    {
        CERR("Exception in thread '" << name() << "': " << e.what() << endl);
    }
}

Thread::Thread(String name, SThreadManager threadManager)
    : m_name(std::move(name))
    , m_threadManager(std::move(threadManager))
{
}

void Thread::terminate()
{
    if (m_thread && m_thread->joinable())
    {
        m_thread->request_stop();
    }
}

bool Thread::terminated()
{
    if (m_thread && m_thread->joinable())
    {
        return m_thread->get_stop_token().stop_requested();
    }
    return false;
}

Thread::Id Thread::id() const
{
    if (m_thread)
    {
        return m_thread->get_id();
    }
    return {};
}

void Thread::join()
{
    if (m_thread && m_thread->joinable())
    {
        m_thread->join();
        m_thread.reset();
    }
}

void Thread::run()
{
    const scoped_lock lock(m_mutex);
    if (m_thread && m_thread->joinable())
    {
        return;
    }

    m_thread = make_shared<jthread>([this](stop_token stopToken) {
        threadStart();
    });
}

bool Thread::running() const
{
    return m_thread && m_thread->joinable();
}
