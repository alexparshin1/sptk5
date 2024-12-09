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

#include <sptk5/Logger.h>
#include <sptk5/net/ServerConnection.h>
#include <sptk5/threads/SynchronizedQueue.h>

namespace sptk {
/**
 * @addtogroup net Networking Classes
 * @{
 */

/**
 * Internal TCP server listener thread
 */
class TCPServerListener
    : public Thread
    , public std::mutex
{
public:
    /**
     * Constructor
     * @param server            TCP server created connection
     * @param port              Listener port number
     * @param connectionType    Connection type
     * @param acceptThreadCount Number of accept threads
     */
    TCPServerListener(TCPServer* server, uint16_t port, ServerConnection::Type connectionType, size_t acceptThreadCount = 2);

    ~TCPServerListener() override = default;

    /**
     * Thread function
     */
    void threadFunction() override;

    /**
     * Start socket listening
     */
    void listen();

    /**
     * Returns listener port number
     */
    uint16_t port() const;

    /**
     * Returns latest socket error (if any)
     */
    String error() const;

    /**
     * Stop running listener and join its thread
     */
    void stop();

private:
    TCPServer*             m_server;         ///< TCP server created connection
    TCPSocket              m_listenerSocket; ///< Listener socket
    String                 m_error;          ///< Last socket error
    ServerConnection::Type m_connectionType; ///< Connection type

    struct CreateConnectionItem
    {
        SocketType  connectionFD {0};
        sockaddr_in connectionInfo = {};
    };

    std::vector<std::jthread>               m_createConnectionThreads; ///< Create connection threads
    SynchronizedQueue<CreateConnectionItem> m_createConnectionQueue;   ///< Create connection queue

    bool acceptConnection(const std::chrono::milliseconds& timeout);               ///< Accept connection
    void createConnection(const CreateConnectionItem& createConnectionItem) const; ///< Create connection
};

class TCPServer;


/**
 * @}
 */
} // namespace sptk
