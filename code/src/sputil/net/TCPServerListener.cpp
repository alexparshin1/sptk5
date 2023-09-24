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
#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPServerListener.h>

using namespace std;
using namespace sptk;

TCPServerListener::TCPServerListener(TCPServer* server, uint16_t port)
    : Thread("CTCPServer::Listener")
    , m_server(shared_ptr<TCPServer>(server,
                                     [this](const TCPServer*) {
                                         stop();
                                     }))
{
    m_listenerSocket.host(Host("localhost", port));
}

void TCPServerListener::acceptConnection(std::chrono::milliseconds timeout)
{
    try
    {
        SocketType connectionFD {0};
        sockaddr_in connectionInfo = {};
        if (m_listenerSocket.accept(connectionFD, connectionInfo, timeout))
        {
            if (m_server->allowConnection(&connectionInfo))
            {
                auto connection = m_server->createConnection(connectionFD, &connectionInfo);
                if (connection)
                {
                    m_server->execute(std::move(connection));
                }
            }
            else
            {
#ifndef _WIN32
                shutdown(connectionFD, SHUT_RDWR);
                ::close(connectionFD);
#else
                closesocket(connectionFD);
#endif
            }
        }
    }
    catch (const Exception& e)
    {
        m_server->log(LogPriority::ERR, e.what());
    }
}

void TCPServerListener::threadFunction()
{
    try
    {
        constexpr auto readTimeout = chrono::milliseconds(100);
        while (!terminated())
        {
            const scoped_lock lock(*this);
            if (m_listenerSocket.readyToRead(readTimeout))
            {
                if (!m_listenerSocket.active())
                {
                    break;
                }
                acceptConnection(readTimeout);
            }
        }
    }
    catch (const Exception& e)
    {
        m_server->log(LogPriority::ERR, e.what());
    }
}

void TCPServerListener::listen()
{
    if (!running())
    {
        m_listenerSocket.listen(0, true);
        run();
    }
}

uint16_t TCPServerListener::port() const
{
    return m_listenerSocket.host().port();
}

String TCPServerListener::error() const
{
    return m_error;
}

void TCPServerListener::stop()
{
    terminate();
    join();
    const scoped_lock lock(*this);
    m_listenerSocket.close();
}
