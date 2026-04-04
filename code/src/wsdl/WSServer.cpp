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

using namespace std;
using namespace sptk;

WSServer::WSServer(const WSServices& services, LogEngine& logger, const String&, const size_t threadCount,
                   const WSConnection::Options& options)
    : TCPServer(services.get("").title(), threadCount, &logger, options.logDetails)
    , WSServerThreads(this, threadCount)
    , m_services(services)
    , m_logger(logger)
    , m_options(options)
    , m_socketEvents(
          "XMQ Server",
          [this](const std::shared_ptr<WSConnection>& connection, auto type)
          {
              socketEventCallback(connection, type);
          },
          100ms, SocketPoolTriggerMode::LevelTriggered)
{
    // if (!hostname.empty())
    // {
    //     host(Host(hostname));
    // }

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
    terminate();
}

void WSServer::terminate()
{
    WSServerThreads::terminate();
}

UServerConnection WSServer::createConnection(ServerConnection::Type connectionType, SocketType connectionSocket, const sockaddr_in* peer)
{
    m_options.encrypted = connectionType == ServerConnection::Type::SSL;

    auto assignedThread = nextThread();

    const auto connection = make_shared<WSSSLConnection>(*this, connectionSocket, peer, m_services,
                                                         m_logger.destination(), m_options, assignedThread);
    if (!connection->isHangup())
    {
        watchConnection(connection);
    }

    return {};
}

void WSServer::watchConnection(const std::shared_ptr<WSConnection>& connection)
{
    scoped_lock lock(m_mutex);
    m_connectionMap[connection.get()] = connection;
    m_socketEvents.add(connection->getSocket(), connection);
}

void WSServer::closeConnection(const std::shared_ptr<WSConnection>& connection)
{
    connection->close();

    scoped_lock lock(m_mutex);
    m_socketEvents.remove(connection->getSocket());
    m_connectionMap.erase(connection.get());
}

[[maybe_unused]] const WSConnection::Options& WSServer::getOptions() const
{
    return m_options;
}

void WSServer::socketEventCallback(const shared_ptr<WSConnection>& connection, const SocketEventType eventType)
{
    if (connection != nullptr)
    {
        scoped_lock lock(m_mutex);

        WSConnection* connectionPtr = connection.get();
        if (const auto connectionIterator = m_connectionMap.find(connectionPtr);
            connectionIterator == m_connectionMap.end())
        {
            return;
        }
    }

    m_socketEvents.remove(connection->getSocket());

    if (eventType.m_hangup || eventType.m_error)
    {
        connection->setHangup();
    }

    if (eventType.m_data)
    {
        const auto workerThread = dynamic_pointer_cast<WSServerThread>(connection->getWorkerThread());
        workerThread->queue(connection);
    }
}
