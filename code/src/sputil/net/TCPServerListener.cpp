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

#include <sptk5/net/RunableServerConnection.h>
#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPServerListener.h>

using namespace std;
using namespace sptk;

TCPServerListener::TCPServerListener(TCPServer* server, const uint16_t port, const ServerConnection::Type connectionType, size_t acceptThreadCount)
    : Thread("CTCPServer::Listener")
    , m_server(server)
    , m_connectionType(connectionType)
{
    m_listenerSocket.host(Host("localhost", port));
    for (size_t i = 0; i < acceptThreadCount; ++i)
    {
        auto createConnectionThread = jthread(
            [this]
            {
                while (!terminated())
                {
                    CreateConnectionItem createConnectionItem;
                    if (m_createConnectionQueue.pop_front(createConnectionItem, 100ms))
                    {
                        createConnection(createConnectionItem);
                    }
                }
            });
        m_createConnectionThreads.push_back(std::move(createConnectionThread));
    }
}

void TCPServerListener::createConnection(const CreateConnectionItem& createConnectionItem) const
{
    if (m_server->allowConnection(&createConnectionItem.connectionInfo))
    {
        if (auto connection = m_server->createConnection(m_connectionType, createConnectionItem.connectionFD, &createConnectionItem.connectionInfo))
        {
            auto* runableServerConnectionPtr = dynamic_cast<RunableServerConnection*>(connection.release());
            auto  runnableServerConnection = std::unique_ptr<RunableServerConnection>(runableServerConnectionPtr);
            m_server->execute(std::move(runnableServerConnection));
        }
    }
    else
    {
#ifndef _WIN32
        shutdown(createConnectionItem.connectionFD, SHUT_RDWR);
        ::close(createConnectionItem.connectionFD);
#else
        closesocket(connectionFD);
#endif
    }
}

bool TCPServerListener::acceptConnection(const chrono::milliseconds& timeout)
{
    try
    {
        SocketType  connectionFD {0};
        sockaddr_in connectionInfo = {};
        if (m_listenerSocket.accept(connectionFD, connectionInfo, timeout))
        {
            CreateConnectionItem createConnectionItem {connectionFD, connectionInfo};
            m_createConnectionQueue.push_back(createConnectionItem);
            return true;
        }
    }
    catch (const Exception& e)
    {
        m_server->log(LogPriority::Error, e.what());
    }
    return false;
}


void TCPServerListener::threadFunction()
{
    try
    {
        constexpr auto readTimeout = chrono::milliseconds(100);
        m_listenerSocket.blockingMode(false);
        while (!terminated())
        {
            //const scoped_lock lock(*this);
            if (!acceptConnection(readTimeout))
            {
                if (!m_listenerSocket.active())
                {
                    break;
                }
            }
        }
    }
    catch (const Exception& e)
    {
        m_server->log(LogPriority::Error, e.what());
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
    m_listenerSocket.close();
    const scoped_lock lock(*this);
}
