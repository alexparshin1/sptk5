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

#include <functional>
#include <sptk5/net/TCPSocket.h>

#ifndef _WIN32

#include <netinet/in.h>

#endif

namespace sptk {
class TCPServer;

/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief Abstract TCP or SSL server connection thread.
 *
 * Used a base class for CTCPServerConnection and COpenSSLServerConnection.
 */
class SP_EXPORT ServerConnection
{
    friend class TCPServer;
    friend class FastTCPServer;

public:
    enum class Type
    {
        TCP,
        SSL
    };

    using Function = std::function<void(ServerConnection& socket)>;

    /**
     * @brief Constructor.
     * @param server            Server that created this connection.
     * @param type              Connection type.
     * @param connectionAddress Connection address.
     */
    ServerConnection(TCPServer& server, Type type, const sockaddr_in* connectionAddress);

    /**
     * @brief Constructor for a server connection that is not owned by a TCPServer.
     *
     * Used by event-driven servers such as FastTcpServer, where the connection
     * lifetime and I/O are managed by a SocketEvents reactor rather than by a
     * dedicated connection thread.
     * @param type              Connection type.
     * @param connectionAddress Connection address.
     */
    ServerConnection(Type type, const sockaddr_in* connectionAddress);

    /**
     * @brief Destructor.
     */
    virtual ~ServerConnection() = default;

    /**
     * @brief Close the connection.
     */
    void close() const;

    /**
     * @brief Access to the internal socket for derived classes.
     * @return internal socket.
     */
    STCPSocket getSocket() const;

    /**
     * @brief Parent TCP server reference.
     * @return server object.
     */
    TCPServer& server() const;

    /**
     * @brief Get the incoming connection address.
     * @return incoming connection address.
     */
    String address() const
    {
        return m_address;
    }

    /**
     * @brief Get connection serial number.
     * @return connection serial number.
     */
    size_t serial() const
    {
        return m_serial;
    }

    static std::tuple<std::string, uint16_t> parseAddress(const sockaddr_in* connectionAddress);

    /**
     * @brief Assign the new socket.
     * @param socket            Socket to assign.
     * @return previous socket.
     */
    STCPSocket setSocket(const STCPSocket& socket);

    /**
     * @return Incoming connection port number.
     */
    uint16_t port() const;

private:
    mutable std::mutex m_mutex;            ///< Mutex that protects internal data.
    TCPServer*         m_server {nullptr}; ///< Parent server object (may be null for FastTcpServer connections).
    STCPSocket         m_socket;           ///< Connection socket.
    String             m_address;          ///< Incoming connection IP address.
    uint16_t           m_port {0};         ///< Incoming connection port.
    size_t             m_serial {0};       ///< Connection serial number.
    Type               m_type {Type::TCP}; ///< Connection type (TCP or SSL).

    /**
     * @brief Create the next connection serial number.
     * @return next connection serial number.
     */
    static size_t nextSerial();
};

using UServerConnection = std::unique_ptr<ServerConnection>;

/**
 * @}
 */
} // namespace sptk
