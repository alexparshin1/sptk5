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

#include <functional>
#include <sptk5/net/TCPSocket.h>

#ifndef _WIN32

#include <netinet/in.h>

#endif

namespace sptk {

class TCPServer;

/**
 * @addtogroup net Networking Classes
 * @{
 */

/**
 * Abstract TCP or SSL server connection thread
 *
 * Used a base class for CTCPServerConnection and COpenSSLServerConnection
 */
class SP_EXPORT ServerConnection
{
    friend class TCPServer;

public:
    enum class Type
    {
        TCP,
        SSL
    };

    using Function = std::function<void(ServerConnection& socket)>;

    /**
     * Constructor
     * @param server            Server that created this connection
     * @param type              Connection type
     * @param connectionAddress Connection address
     */
    ServerConnection(TCPServer& server, Type type, const sockaddr_in* connectionAddress);

    virtual ~ServerConnection() = default;

    void close();

    /**
     * Access to internal socket for derived classes
     * @return internal socket
     */
    TCPSocket& socket() const;

    /**
     * Access to internal socket for derived classes
     * @return internal socket
     */
    STCPSocket getSocket() const;

    /**
     * Parent TCP server reference
     * @return
     */
    TCPServer& server() const;

    /**
     * Get incoming connection address
     * @return incoming connection address
     */
    String address() const
    {
        return m_address;
    }

    /**
     * Get connection serial number
     * @return connection serial number
     */
    size_t serial() const
    {
        return m_serial;
    }

protected:
    /**
     * Assign new socket
     * @param socket            Socket to assign
     * @return previous socket
     */
    STCPSocket setSocket(const STCPSocket& socket);

    void parseAddress(const sockaddr_in* connectionAddress);

public:
    uint16_t port() const;

private:
    mutable std::mutex m_mutex;      ///< Mutex that protects internal data
    TCPServer&         m_server;     ///< Parent server object
    STCPSocket         m_socket;     ///< Connection socket
    String             m_address;    ///< Incoming connection IP address
    uint16_t           m_port {0};   ///< Incoming connection port
    size_t             m_serial {0}; ///< Connection serial number
    Type               m_type;       ///< Connection type (TCP or SSL)

    /**
     * Create next connection serial number
     * @return
     */
    static size_t nextSerial();
};

using UServerConnection = std::unique_ptr<ServerConnection>;

/**
 * @}
 */
} // namespace sptk
