/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/net/SocketEvents.h>

using namespace std;
using namespace sptk;
using namespace chrono;

#define MAXEVENTS 128

SocketEvents::SocketEvents(const String& name, const SocketEventCallback& eventsCallback, milliseconds timeout)
: Thread(name), m_socketPool(eventsCallback), m_timeout(timeout)
{
    m_socketPool.open();
}

SocketEvents::~SocketEvents()
{
	stop();
}

void SocketEvents::stop()
{
	try {
	    m_socketPool.close();
		if (running()) {
			terminate();
			join();
		}
	}
	catch (const Exception& e) {
		CERR(e.message() << endl)
	}
}

void SocketEvents::add(BaseSocket& socket, void* userData)
{
	if (!running()) {
		lock_guard<mutex> lock(m_mutex);
		if (m_shutdown)
			throw Exception("SocketEvents already stopped");
		run();
		m_started.wait_for(true, milliseconds(1000));
	}
    m_socketPool.watchSocket(socket, userData);
}

void SocketEvents::remove(BaseSocket& socket)
{
    m_socketPool.forgetSocket(socket);
}

void SocketEvents::threadFunction()
{
    m_socketPool.open();
    m_started = true;
    while (!terminated()) {
        try {
            m_socketPool.waitForEvents(m_timeout);
        }
        catch (const Exception& e) {
        	if (m_socketPool.active()) {
				CERR(e.message() << endl)
			} else
        		break;
        }
    }
    m_socketPool.close();
}

void SocketEvents::terminate()
{
	Thread::terminate();
	lock_guard<mutex> lock(m_mutex);
	m_shutdown = true;
}

size_t SocketEvents::size() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_watchList.size();
}
