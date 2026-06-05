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

#include <sptk5/wsdl/WSConnection.h>
#include <sptk5/wsdl/WSServer.h>

#include <utility>

using namespace std;
using namespace sptk;

WSServer::WSServer(const WSServices& services, LogEngine& logger, const String&, const size_t threadCount,
                   WSConnection::Options options)
    : FastTCPServer(services.get("").title())
    , WSServerThreads(this, threadCount)
    , m_services(services)
    , m_logger(logger)
    , m_options(std::move(options))
{
    if (m_options.paths.htmlIndexPage.empty())
    {
        m_options.paths.htmlIndexPage = "index.html";
    }

    if (m_options.paths.wsRequestPage.empty())
    {
        m_options.paths.wsRequestPage = "request";
    }
}

WSServer::~WSServer()
{
    // Stop the worker threads first so they no longer re-arm or process connections,
    // then stop the listeners and the reactor and close all connections.
    terminate();
    stop();
}

shared_ptr<ServerConnection> WSServer::createConnection(ServerConnection::Type connectionType, SocketType connectionSocket, const sockaddr_in* peer)
{
    m_options.encrypted = connectionType == ServerConnection::Type::SSL;

    auto assignedThread = nextThread();

    auto connection = make_shared<WSSSLConnection>(*this, connectionSocket, peer, m_services,
                                                   m_logger.destination(), m_options, assignedThread);
    if (connection->isHangup())
    {
        // Connection hung up during setup: drop it (its socket is closed by the destructor).
        return {};
    }

    // FastTCPServer will tune the socket and start monitoring this connection.
    return connection;
}

void WSServer::tuneSocket(const STCPSocket&)
{
    // Keep web service connection sockets in blocking mode: the reactor only signals
    // readiness, and the worker thread performs blocking request reads within the
    // request timeout.
}

void WSServer::socketEventCallback(const shared_ptr<ServerConnection>& baseConnection, const SocketEventType eventType)
{
    const auto connection = dynamic_pointer_cast<WSConnection>(baseConnection);
    if (!connection)
    {
        return;
    }

    // Stop monitoring while the worker processes this connection, so the
    // level-triggered reactor does not keep re-signaling the same socket.
    unwatchConnection(connection);

    if (eventType.m_hangup || eventType.m_error)
    {
        connection->setHangup();
    }

    if (eventType.m_data)
    {
        const auto workerThread = dynamic_pointer_cast<WSServerThread>(connection->getWorkerThread());
        workerThread->queue(connection);
    }
    else if (connection->isHangup())
    {
        closeConnection(connection);
    }
}

[[maybe_unused]] const WSConnection::Options& WSServer::getOptions() const
{
    return m_options;
}
