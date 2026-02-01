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

#include "sptk5/threads/Flag.h"
#include <sptk5/net/ServerConnection.h>
#include <sptk5/threads/SynchronizedQueue.h>

namespace sptk {
/**
 * @addtogroup net Networking Classes.
 * @{
 */

/**
 * @brief Internal TCP server listener thread.
 */
class TCPServerListener
    : public Thread
{
public:
    /**
     * @brief Constructor.
     * @param server            TCP server that created the connection.
     * @param listenerHost      Listener host and port number.
     * @param connectionType    Connection type.
     * @param acceptThreadCount Number of the acception threads.
     */
    TCPServerListener(TCPServer* server, const Host& listenerHost, ServerConnection::Type connectionType, size_t acceptThreadCount = 2);

    ~TCPServerListener() override = default;

    /**
     * @brief Thread function.
     */
    void threadFunction() override;

    /**
     * @brief Start socket listening.
     */
    void listen();

    /**
     * @brief Returns listener port number.
     */
    uint16_t port() const;

    /**
     * @brief Returns latest socket error (if any).
     */
    String error() const;

    /**
     * @brief Stop running listener and join its thread.
     */
    void stop();

    bool waitUntilStarted(std::chrono::milliseconds timeout)
    {
        return m_hasStarted.wait_for(true, timeout);
    }

private:
    mutable std::mutex     m_mutex;              ///< Listener mutex.
    TCPServer*             m_server;             ///< TCP server created connection.
    TCPSocket              m_listenerSocket;     ///< Listener socket.
    String                 m_error;              ///< Last socket error.
    ServerConnection::Type m_connectionType;     ///< Connection type.
    Flag                   m_hasStarted {false}; ///< True if the listener thread has started.

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
