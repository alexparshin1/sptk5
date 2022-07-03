/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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
    /**
     * Constructor
     * @param server            Server that created this connection
     * @param connectionSocket  Already accepted by accept() function incoming connection socket
     * @param taskName          Task name
     */
    ServerConnection(TCPServer& server, SOCKET connectionSocket, const sockaddr_in* connectionAddress,
                     const String& taskName);

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
     */
    void setSocket(const STCPSocket& socket);

    void parseAddress(const sockaddr_in* connectionAddress);

private:
    mutable std::mutex m_mutex;
    TCPServer& m_server; ///< Parent server object
    STCPSocket m_socket; ///< Connection socket
    String m_address;    ///< Incoming connection IP address
    size_t m_serial {0}; ///< Connection serial number

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
