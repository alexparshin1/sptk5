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

#pragma once

#include "WSServerThread.h"

#include <sptk5/net/FastTCPServer.h>
#include <sptk5/wsdl/WSConnection.h>

namespace sptk {
/**
 * @addtogroup wsdl WSDL-related Classes.
 * @{
 */

/**
 * @brief Web Service Server.
 *
 * Simple server to accept Web Service requests.
 * Actual request processing is implemented in the Web Service request processor,
 * passed to the constructor.
 * As a bonus, WSServer also serves static files, located in staticFilesDirectory.
 * That may be used to implement a web application.
 *
 * WSServer is event-driven: it derives from FastTCPServer, which monitors all
 * accepted connections with a single reactor (epoll/kqueue/wepoll). When a
 * connection becomes readable, the request is dispatched to a WSServerThread
 * worker that processes it without blocking the reactor.
 */
class SP_EXPORT WSServer
    : public FastTCPServer
    , public WSServerThreads
{
    friend class WSServerThread;

public:
    /**
     * @brief Constructor.
     * @param services              Web Service request processor.
     * @param logger                Logger.
     * @param hostname              This service hostname.
     * @param threadCount           Max number of simultaneously running requests.
     * @param options               Client connection options.
     */
    WSServer(const WSServices& services, LogEngine& logger, const String& hostname, size_t threadCount,
             WSConnection::Options options);

    /**
     * @brief Destructor.
     */
    ~WSServer() override;

    /**
     * @brief Get server options.
     * @return Server options.
     */
    [[maybe_unused]] const WSConnection::Options& getOptions() const;

protected:
    /**
     * @brief Create a WSConnection for an accepted client connection.
     *
     * The created connection is added to the FastTCPServer reactor and monitored
     * for input events.
     * @param connectionType        Incoming connection type.
     * @param connectionSocket      Already accepted incoming connection socket.
     * @param peer                  Incoming connection information.
     * @return The created connection, or empty if the connection hung up during setup.
     */
    std::shared_ptr<ServerConnection> createConnection(ServerConnection::Type connectionType, SocketType connectionSocket, const sockaddr_in* peer) override;

    /**
     * @brief Keep web service connection sockets in blocking mode.
     *
     * The reactor signals readiness; the worker thread then performs blocking
     * request reads within the request timeout.
     * @param socket            Accepted connection socket.
     */
    void tuneSocket(const STCPSocket& socket) override;

    /**
     * @brief Socket event handler.
     *
     * Removes the connection from the reactor (so it is not re-signaled while it
     * is being processed) and hands it to its worker thread for processing, or
     * closes it on peer hangup or error.
     * @param connection        Connection that received the event.
     * @param eventType         Event type.
     */
    void socketEventCallback(const std::shared_ptr<ServerConnection>& connection, SocketEventType eventType) override;

private:
    WSServices            m_services; ///< Web Service request processor.
    Logger                m_logger;   ///< Logger object.
    WSConnection::Options m_options;  ///< Client connection options.
};

/**
 * @}
 */

} // namespace sptk
