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

#include <sptk5/Buffer.h>
#include <sptk5/Exception.h>
#include <sptk5/Strings.h>
#include <sptk5/net/Proxy.h>
#include <sptk5/net/Socket.h>

#ifndef _WIN32

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

/**
 * A socket handle is an integer
 */
using SocketType = int;
using SOCKET_ADDRESS_FAMILY = sa_family_t;

/**
 * A value to indicate an invalid handle
 */
#define INVALID_SOCKET (-1)

#else
#include <winsock2.h>

#include <windows.h>

using socklen_t = int;
using SOCKET_ADDRESS_FAMILY = unsigned short;
#endif

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Generic TCP socket.
 *
 * Allows to establish a network connection
 * to the host by name and port address
 */
class SP_EXPORT TCPSocket
    : public Socket
{
public:
    /**
    * Constructor
    * @param domain            Socket domain type
    * @param type              Socket type
    * @param protocol          Protocol type
    */
    explicit TCPSocket(SOCKET_ADDRESS_FAMILY domain = AF_INET, int32_t type = SOCK_STREAM, int32_t protocol = 0);

    /**
    * Destructor
    */
    ~TCPSocket() override;

    /**
     * Set proxy
     * @param proxy             Proxy.
     */
    void setProxy(std::shared_ptr<Proxy> proxy);

    /**
     * In server mode, waits for the incoming connection.
     *
     * When incoming connection is made, exits returning the connection info
     * @param clientSocketFD    Connected client socket FD
     * @param clientInfo        Connected client info
     * @param timeout           Accept operation timeout
     * @returns                 True if accepted a connection
     */
    [[nodiscard]] virtual bool accept(SocketType& clientSocketFD, struct sockaddr_in& clientInfo, std::chrono::milliseconds timeout);

protected:
    /**
     * Opens the client socket connection by host and port
     * @param host              The host
     * @param openMode          Socket open mode
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode
     * @param timeout           Connection timeout. The default is 0 (wait forever)
     */
    void openUnlocked(const Host& host, OpenMode openMode, bool blockingMode, std::chrono::milliseconds timeout) override;

    /**
     * Opens the client socket connection by host and port
     * @param address           Address and port
     * @param openMode          Socket open mode
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode
     * @param timeout           Connection timeout. The default is 0 (wait forever)
     */
    void openUnlocked(const struct sockaddr_in& address, OpenMode openMode, bool blockingMode,
                      std::chrono::milliseconds timeout) override;

    /**
     * Reads data from the socket
     * @param buffer            The memory buffer
     * @param size              The number of bytes to read
     * @param from              An optional structure for source address
     * @returns the number of bytes read from the socket
     */
    [[nodiscard]] size_t readUnlocked(uint8_t* buffer, size_t size, sockaddr_in* from) override;

    /**
     * Get proxy information
     * @return
     */
    Proxy* proxy()
    {
        return m_proxy.get();
    }

private:
    std::shared_ptr<Proxy> m_proxy; ///< Optional proxy
    Buffer m_stringBuffer;          ///< Buffer to read a line

    void handleReadFromSocketError(int error);
};

using STCPSocket = std::shared_ptr<TCPSocket>;

/**
 * @}
 */
} // namespace sptk
