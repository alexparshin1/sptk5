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

#include <chrono>
#include <sptk5/Buffer.h>
#include <sptk5/DateTime.h>
#include <sptk5/Exception.h>
#include <sptk5/Strings.h>
#include <sptk5/net/Host.h>

#ifndef _WIN32

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

/**
 * A socket handle is an integer
 */
using SOCKET = int;
using SOCKET_ADDRESS_FAMILY = sa_family_t;

#ifdef __APPLE__
using socklen_t = int;
#endif

/**
 * A value to indicate an invalid handle
 */
#define INVALID_SOCKET (-1)

#else
#include <winsock2.h>
#include <ws2tcpip.h>

#include <windows.h>
using socklen_t = int;
using SOCKET_ADDRESS_FAMILY = unsigned short;
#endif

namespace sptk {

class BaseSocketVirtualMethods
{
public:
    /**
    * A mode to open a socket, one of
    */
    enum class OpenMode : uint8_t
    {
        CREATE,  ///< Only create (Typical UDP connectionless socket)
        CONNECT, ///< Connect (Typical TCP connection socket)
        BIND     ///< Bind (TCP listener)
    };

    /**
     * Constructor
     * @param domain            Socket domain type
     * @param type              Socket type
     * @param protocol          Protocol type
     */
    explicit BaseSocketVirtualMethods(SOCKET_ADDRESS_FAMILY domain = AF_INET, int32_t type = SOCK_STREAM, int32_t protocol = 0);

protected:
    /**
     * Returns the current socket state
     * @returns true if socket is opened
     */
    [[nodiscard]] virtual bool activeUnlocked() const
    {
        return m_sockfd != INVALID_SOCKET;
    }

    virtual void closeUnlocked();

    /**
     * Get socket internal (OS) handle
     */
    SOCKET getSocketFdUnlocked() const
    {
        return m_sockfd;
    }

    /**
     * Set the host
     * @param host              The host
     */
    void setHostUnlocked(const Host& host)
    {
        m_host = host;
    }

    /**
     * Return the host
     */
    [[nodiscard]] const Host& getHostUnlocked() const
    {
        return m_host;
    }

    /**
     * Set blockingMode mode
     * @param blockingMode      Socket blockingMode mode flag
     */
    void setBlockingModeUnlocked(bool blockingMode);

    /**
     * Sets socket option value
     * Throws an error if not succeeded
     */
    void setOptionUnlocked(int level, int option, int value) const;

    /**
     * Gets socket option value
     *
     * Throws an error if not succeeded
     */
    void getOptionUnlocked(int level, int option, int& value) const;

    /**
     * Returns number of bytes available in socket
     */
    [[nodiscard]] virtual size_t getSocketBytesUnlocked() const;

    /**
     * Attaches socket handle
     * @param socketHandle      Existing socket handle
     */
    virtual void attachUnlocked(SOCKET socketHandle, bool accept);

    /**
     * Detaches socket handle, setting it to INVALID_SOCKET.
     * Closes the socket without affecting socket handle.
     * @return Existing socket handle
     */
    virtual SOCKET detachUnlocked();

protected:
    SOCKET m_sockfd {INVALID_SOCKET}; ///< Socket internal (OS) handle
    int32_t m_domain;                 ///< Socket domain type
    int32_t m_type;                   ///< Socket type
    int32_t m_protocol;               ///< Socket protocol
    Host m_host;                      ///< Host
    bool m_blockingMode {false};      ///< Blocking mode flag
};

/**
 * Throws socket exception with error description retrieved from socket state
 * @param message           Error message
 * @param file              Source file name
 * @param line              Source file line number
 */
SP_EXPORT void throwSocketError(const String& message, const std::source_location& location = std::source_location::current());

} // namespace sptk
