/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/threads/Runable.h>
#include <sptk5/threads/Thread.h>

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
    : public Runable
{
    friend class TCPServer;

public:
    enum class Type
    {
        TCP,
        SSL
    };

    using Function = std::function<void(const Runable& task, TCPSocket& socket, const String& address)>;

    /**
     * Constructor
     * @param server            Server that created this connection
     * @param connectionSocket  Already accepted by accept() function incoming connection socket
     * @param taskName          Task name
     * @param connectionFunction Connection function processing this connection
     */
    ServerConnection(TCPServer& server, SOCKET connectionSocket, Type type, const sockaddr_in* connectionAddress,
                     const String& taskName, const ServerConnection::Function& connectionFunction = {});

    /**
     * Access to internal socket for derived classes
     * @return internal socket
     */
    TCPSocket& socket() const;

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

    void run() override
    {
        if (m_connectionFunction)
        {
            m_connectionFunction(*this, socket(), address());
        }
    }

private:
    mutable std::mutex m_mutex;
    TCPServer& m_server;                             ///< Parent server object
    STCPSocket m_socket;                             ///< Connection socket
    String m_address;                                ///< Incoming connection IP address
    size_t m_serial {0};                             ///< Connection serial number
    Type m_type;                                     ///< Connection type (TCP or SSL)
    ServerConnection::Function m_connectionFunction; ///< Function that is executed for each client connection

    /**
     * Create next connection serial number
     * @return
     */
    static size_t nextSerial();
};

using SServerConnection = std::shared_ptr<ServerConnection>;

/**
 * @}
 */
} // namespace sptk
