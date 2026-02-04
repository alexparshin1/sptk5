/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

TCPServerListener::TCPServerListener(TCPServer* server, const Host& listenerHost, const ServerConnection::Type connectionType, const size_t acceptThreadCount)
    : Thread("CTCPServer::Listener")
    , m_server(server)
    , m_connectionType(connectionType)
{
    m_listenerSocket.host(listenerHost);
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
        closesocket(createConnectionItem.connectionFD);
#endif
    }
}

bool TCPServerListener::acceptConnection(const chrono::milliseconds& timeout)
{
    try
    {
        sockaddr_in connectionInfo = {};
        if (SocketType connectionFD {0};
            m_listenerSocket.accept(connectionFD, connectionInfo, timeout))
        {
            const CreateConnectionItem createConnectionItem {connectionFD, connectionInfo};
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
        // Indicate that the listener thread has started:
        m_hasStarted = true;

        constexpr auto readTimeout = chrono::milliseconds(500);

        m_listenerSocket.blockingMode(false);

        while (!terminated() && m_listenerSocket.active())
        {
            if (!acceptConnection(readTimeout))
            {
                if (!m_listenerSocket.active())
                {
                    break;
                }
            }
        }

        if (m_listenerSocket.active())
        {
            m_listenerSocket.close();
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

Host TCPServerListener::host() const
{
    return m_listenerSocket.host();
}

String TCPServerListener::error() const
{
    return m_error;
}

void TCPServerListener::stop()
{
    terminate();
    join();
}
