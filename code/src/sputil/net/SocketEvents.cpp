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

#include <sptk5/Printer.h>
#include <sptk5/cutils>
#include <sptk5/net/SocketEvents.h>

using namespace std;
using namespace sptk;
using namespace chrono;

SocketEvents::SocketEvents(const String& name, const SocketEventCallback& eventsCallback, milliseconds timeout)
    : Thread(name)
    , m_socketPool(eventsCallback)
    , m_timeout(timeout)
{
    m_socketPool.open();
}

SocketEvents::~SocketEvents()
{
    stop();
}

void SocketEvents::stop()
{
    try
    {
        m_socketPool.close();
        if (running())
        {
            terminate();
            join();
        }
    }
    catch (const Exception& e)
    {
        CERR(e.message() << endl);
    }
}

void SocketEvents::add(Socket& socket, const uint8_t* userData, bool edgeTrigerred)
{
    if (!running())
    {
        const scoped_lock lock(m_mutex);
        if (m_shutdown)
        {
            throw Exception("SocketEvents already stopped");
        }
        run();
        m_started.wait_for(true, seconds(1));
    }
    m_socketPool.watchSocket(socket, userData, edgeTrigerred);
}

void SocketEvents::remove(Socket& socket)
{
    m_socketPool.forgetSocket(socket);
}

bool SocketEvents::has(Socket& socket)
{
    return m_socketPool.hasSocket(socket);
}

void SocketEvents::threadFunction()
{
    m_socketPool.open();
    m_started = true;
    while (!terminated())
    {
        try
        {
            if (!m_socketPool.waitForEvents(m_timeout))
            {
                break;
            }
        }
        catch (const Exception& e)
        {
            CERR(e.message() << endl);
        }
    }
    m_socketPool.close();
}

void SocketEvents::terminate()
{
    Thread::terminate();
    const scoped_lock lock(m_mutex);
    m_shutdown = true;
}

size_t SocketEvents::size() const
{
    const scoped_lock lock(m_mutex);
    return m_watchList.size();
}
