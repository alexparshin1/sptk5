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

#include <chrono>
#include <sptk5/Buffer.h>
#include <sptk5/DateTime.h>
#include <sptk5/Exception.h>
#include <sptk5/Strings.h>
#include <sptk5/net/Host.h>

#ifndef _WIN32

#include <arpa/inet.h>
#include <atomic>
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
using SocketType = int;
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
using SocketType = SOCKET;
#endif

namespace sptk {

/**
 * @brief Virtual methods for the Socket class.
 *
 * All methods are not locking a mutex
 */
class SP_EXPORT SocketVirtualMethods
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
    explicit SocketVirtualMethods(SOCKET_ADDRESS_FAMILY domain = AF_INET, int32_t type = SOCK_STREAM, int32_t protocol = 0);

    /**
     * @Destructor
     */
    virtual ~SocketVirtualMethods() = default;

protected:
    /**
     * Opens the socket connection by address.
     * @param addr              Defines socket address/port information
     * @param openMode          SOM_CREATE for UDP socket, SOM_BIND for the server socket, and SOM_CONNECT for the client socket
     * @param timeout           Connection timeout. If 0 then wait forever.
     */
    void openAddressUnlocked(const sockaddr_in& addr, OpenMode openMode = OpenMode::CREATE, std::chrono::milliseconds timeout = std::chrono::milliseconds(0), bool reusePort = true);

    /**
     * Opens the client socket connection by host and port
     * @param host              The host
     * @param openMode          Socket open mode
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode
     * @param timeoutMS         Connection timeout. The default is 0 (wait forever)
     */
    virtual void openUnlocked(const Host& host, OpenMode openMode, bool blockingMode, std::chrono::milliseconds timeoutMS);

    /**
     * Opens the client socket connection by host and port
     * @param address           Address and port
     * @param openMode          Socket open mode
     * @param blockMode         Socket blocking (true) on non-blocking (false) mode
     * @param timeoutMS         Connection timeout, std::chrono::milliseconds. The default is 0 (wait forever)
     */
    virtual void openUnlocked(const struct sockaddr_in& address, OpenMode openMode, bool blockMode,
                              std::chrono::milliseconds timeoutMS)
    {
        openAddressUnlocked(address, openMode, timeoutMS);
        setBlockingModeUnlocked(blockMode);
    }

    /**
     * Returns the current socket state
     * @returns true if socket is opened
     */
    [[nodiscard]] virtual bool activeUnlocked() const
    {
        return m_socketFd != INVALID_SOCKET;
    }


    /**
     * Binds the socket to port
     * @param address           Local IP address, or NULL if any
     * @param portNumber        The port number, or 0 if any
     * @param reusePort         If true then set SO_REUSEPORT
     */
    void bindUnlocked(const char* address, uint32_t portNumber, bool reusePort = false);

    /**
     * Opens the server socket connection on port (binds/listens)
     * @param portNumber        The port number
     */
    void listenUnlocked(uint16_t portNumber, bool reusePort);

    /**
     * Close socket
     */
    virtual void closeUnlocked();

    /**
     * Get socket internal (OS) handle
     */
    SocketType getSocketFdUnlocked() const
    {
        return m_socketFd;
    }

    /**
     * Set socket internal (OS) handle
     */
    void setSocketFdUnlocked(SocketType socket)
    {
        m_socketFd = socket;
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
     * @brief Return current blocking mode state
     * @return Current blocking mode state
     */
    [[nodiscard]] bool getBlockingModeUnlocked() const
    {
        return m_blockingMode;
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
    virtual void attachUnlocked(SocketType socketHandle, bool accept);

    /**
     * Detaches socket handle, setting it to INVALID_SOCKET.
     * Closes the socket without affecting socket handle.
     * @return Existing socket handle
     */
    virtual SocketType detachUnlocked();

    /**
     * Reports true if socket is ready for reading from it
     * @param timeout           Read timeout
     */
    [[nodiscard]] virtual bool readyToReadUnlocked(std::chrono::milliseconds timeout);

    /**
     * Reports true if socket is ready for writing to it
     * @param timeout           Write timeout
     */
    [[nodiscard]] virtual bool readyToWriteUnlocked(std::chrono::milliseconds timeout);

    /**
     * Reads data from the socket in regular or SSL mode
     * @param buffer            The destination buffer
     * @param len              The destination buffer size
     * @returns the number of bytes read from the socket
     */
    [[nodiscard]] virtual size_t recvUnlocked(uint8_t* buffer, size_t len);

    /**
     * Reads data from the socket
     * @param buffer            The memory buffer
     * @param size              The number of bytes to read
     * @param from              The source address
     * @returns the number of bytes read from the socket
     */
    [[nodiscard]] virtual size_t readUnlocked(uint8_t* buffer, size_t size, sockaddr_in* from);

    /**
     * Reads data from the socket in regular or TLS mode
     * @param buffer            The send buffer
     * @param len              The send data length
     * @returns the number of bytes sent the socket
     */
    [[nodiscard]] virtual size_t sendUnlocked(const uint8_t* buffer, size_t len);

    /**
     * Writes data to the socket
     *
     * If size is omitted then buffer is treated as zero-terminated string
     * @param buffer            The memory buffer
     * @param size              The memory buffer size
     * @param peer              The peer information
     * @returns the number of bytes written to the socket
     */
    virtual size_t writeUnlocked(const uint8_t* buffer, size_t size, const sockaddr_in* peer);

    /**
     * Get socket domain type
     */
    [[nodiscard]] int32_t getDomainUnlocked() const
    {
        return m_domain;
    }

    /**
     * Get socket type
     */
    [[nodiscard]] int32_t getTypeUnlocked() const
    {
        return m_type;
    }

    /**
     * Get socket protocol
     */
    [[nodiscard]] int32_t getProtocolUnlocked() const
    {
        return m_protocol;
    }

private:
    std::atomic<SocketType> m_socketFd {INVALID_SOCKET}; ///< Socket internal (OS) handle
    std::atomic<int32_t> m_domain;                       ///< Socket domain type
    std::atomic<int32_t> m_type;                         ///< Socket type
    std::atomic<int32_t> m_protocol;                     ///< Socket protocol
    Host m_host;                                         ///< Host
    std::atomic<bool> m_blockingMode {false};            ///< Blocking mode flag
};

/**
 * Throws socket exception with error description retrieved from socket state
 * @param message           Error message
 * @param file              Source file name
 * @param line              Source file line number
 */
SP_EXPORT void throwSocketError(const String& message, const std::source_location& location = std::source_location::current());

} // namespace sptk
