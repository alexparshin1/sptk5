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

#pragma once

#include "WSServerThread.h"

#include <sptk5/cnet>
#include <sptk5/cutils>
#include <sptk5/net/SocketEvents.h>
#include <sptk5/wsdl/WSConnection.h>
#include <sptk5/wsdl/WSRequest.h>

namespace sptk {

/**
 * @addtogroup wsdl WSDL-related Classes
 * @{
 */

/**
 * @brief Web Service Server
 *
 * Simple server to accept Web Service requests.
 * Actual request processing is implemented in Web Service request processor,
 * passed to constructor.
 * As a bonus, WSServer also serves static files, located in staticFilesDirectory.
 * That may be used to implement a web application.
 */
class SP_EXPORT WSServer
    : public TCPServer
    , public WSServerThreads
{
    friend class WSServerThread;

public:
    /**
     * Constructor
     * @param service               Web Service request processor
     * @param logger                Logger
     * @param hostname              This service hostname
     * @param threadCount           Max number of simultaneously running requests
     * @param options               Client connection options
     */
    WSServer(const WSServices& services, LogEngine& logger, const String& hostname, size_t threadCount,
             const WSConnection::Options& options);

    /**
     * @brief Destructor
     */
    ~WSServer() override;

    /**
     * @brief Get server options
     * @return Server options
     */
    [[maybe_unused]] const WSConnection::Options& getOptions() const;

protected:
    /**
     * Creates connection thread derived from CTCPServerConnection
     *
     * Application should override this method to create concrete connection object.
     * Created connection object is maintained by CTCPServer.
     * @param connectionType        Incoming connection type
     * @param connectionSocket      Already accepted incoming connection socket
     * @param peer                  Incoming connection information
     */
    UServerConnection createConnection(ServerConnection::Type connectionType, SocketType connectionSocket, const sockaddr_in* peer) override;

    /**
     * @brief Terminate server
     */
    void terminate();

protected:
    /**
     * @brief Start monitoring incoming connection's events
     * @param connection        Client connection
     */
    void watchConnection(const std::shared_ptr<WSConnection>& connection);

    /**
     * @brief Stop monitoring incoming connection's events
     * @param connection        Client connection
     */
    void ignoreConnection(const std::shared_ptr<WSConnection>& connection);

    /**
     * @brief Close client connection
     * @param connection        Client connection
     */
    void closeConnection(const std::shared_ptr<WSConnection>& connection);

private:
    using SWSConnection = std::shared_ptr<WSConnection>;

    mutable std::mutex                     m_mutex;         ///< Mutex that protects internal data
    WSServices                             m_services;      ///< Web Service request processor
    Logger                                 m_logger;        ///< Logger object
    WSConnection::Options                  m_options;       ///< Client connection options
    SocketEvents                           m_socketEvents;  ///< Socket events
    std::map<WSConnection*, SWSConnection> m_connectionMap; ///< Map of active connections

    /**
     * @brief Socket event callback function
     * @param userData          User data
     * @param eventType         Event type
     */
    void socketEventCallback(const uint8_t* userData, SocketEventType eventType);
};

/**
 * @}
 */

} // namespace sptk
