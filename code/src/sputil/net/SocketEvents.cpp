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

#include <sptk5/Printer.h>
#include <sptk5/cutils>
#include <sptk5/net/SocketEvents.h>

using namespace std;
using namespace sptk;
using namespace chrono;

SocketEvents::SocketEvents(const String& name, const SocketEventCallback& eventsCallback, std::chrono::milliseconds timeout, SocketPool::TriggerMode triggerMode)
    : Thread(name)
    , m_socketPool(eventsCallback, triggerMode)
    , m_timeout(timeout)
{
    Thread::run();
}

SocketEvents::~SocketEvents()
{
    stop();
}

void SocketEvents::stop()
{
    if (!terminated())
    {
        try
        {
            if (running())
            {
                terminate();
                join();
            }
        }
        catch (const Exception& e)
        {
            CERR(e.message() << '\n');
        }
    }
}

void SocketEvents::threadFunction()
{
    m_socketPool.open();
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

size_t SocketEvents::size() const
{
    const scoped_lock lock(m_mutex);
    return m_watchList.size();
}
