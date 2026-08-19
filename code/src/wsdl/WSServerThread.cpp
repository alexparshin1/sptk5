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

#include <sptk5/wsdl/WSServer.h>

using namespace std;
using namespace sptk;

namespace sptk {
WSServerThread::WSServerThread(WSServer* server)
    : Thread("WSServerThread")
    , m_server(server)
{
}

WSServerThread::~WSServerThread()
{
    Thread::terminate();
}

void WSServerThread::threadFunction()
{
    while (!terminated())
    {
        if (shared_ptr<WSConnection> connection;
            m_connectionQueue.pop_front(connection, 1s))
        {
            connection->execute();
            if (connection->getSocket()->socketBytes() > 4)
            {
                m_connectionQueue.push_back(connection);
                continue;
            }
            if (connection->isHangup())
            {
                m_server->closeConnection(connection);
                continue;
            }
            if (connection->getSocket()->active())
            {
                m_server->watchConnection(connection);
            }
        }
    }
}

void WSServerThread::queue(const shared_ptr<WSConnection>& connection)
{
    m_connectionQueue.push_back(connection);
}

WSServerThreads::WSServerThreads(WSServer* server, const size_t threadCount)
{
    scoped_lock lock(m_mutex);
    m_threads.reserve(threadCount);
    for (size_t i = 0; i < threadCount; ++i)
    {
        auto thread = make_shared<WSServerThread>(server);
        m_threads.push_back(thread);
        thread->run();
    }
}

SWSServerThread WSServerThreads::nextThread()
{
    scoped_lock lock(m_mutex);
    if (m_nextThreadIndex >= m_threads.size())
    {
        m_nextThreadIndex = 0;
    }
    auto thread = m_threads[m_nextThreadIndex];
    ++m_nextThreadIndex;
    return thread;
}

void WSServerThreads::terminate()
{
    scoped_lock lock(m_mutex);
    for (const auto& thread: m_threads)
    {
        thread->terminate();
    }

    for (const auto& thread: m_threads)
    {
        thread->join();
    }

    m_threads.clear();
}

} // namespace sptk
