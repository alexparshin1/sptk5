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

#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPServerListener.h>

using namespace std;
using namespace sptk;

TCPServerListener::TCPServerListener(TCPServer* server, uint16_t port)
: Thread("CTCPServer::Listener"), m_server(server)
{
    m_listenerSocket.host(Host("localhost", port));
}

TCPServerListener::~TCPServerListener()
{
    stop();
}

void TCPServerListener::acceptConnection()
{
    try {
        SOCKET connectionFD;
        sockaddr_in connectionInfo = {};
        m_listenerSocket.accept(connectionFD, connectionInfo);
        if (connectionFD == -1)
            return;
        if (m_server->allowConnection(&connectionInfo)) {
            auto* connection = m_server->createConnection(connectionFD, &connectionInfo);
            m_server->execute(connection);
        }
        else {
#ifndef _WIN32
            shutdown(connectionFD,SHUT_RDWR);
            ::close (connectionFD);
#else
            closesocket(connectionFD);
#endif
        }
    }
    catch (const Exception& e) {
        m_server->log(LogPriority::ERROR, e.what());
    }
}

void TCPServerListener::threadFunction()
{
    try {
        while (!terminated()) {
            scoped_lock lock(*this);
            if (m_listenerSocket.readyToRead(chrono::milliseconds(100)))
                acceptConnection();
        }
    }
    catch (const Exception& e) {
        m_server->log(LogPriority::ERROR, e.what());
    }
}

void TCPServerListener::terminate()
{
    Thread::terminate();
	scoped_lock lock(*this);
	m_listenerSocket.close();
}

void TCPServerListener::stop()
{
    terminate();
    join();
}
