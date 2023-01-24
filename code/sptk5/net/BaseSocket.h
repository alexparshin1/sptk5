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

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Generic socket.
 *
 * Allows establishing a network connection
 * to the host by name and port address
 */
class SP_EXPORT BaseSocket
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
     * Get socket internal (OS) handle
     */
    SOCKET fd() const
    {
        return m_sockfd;
    }

    /**
     * Opens the socket connection by address.
     * @param openMode          SOM_CREATE for UDP socket, SOM_BIND for the server socket, and SOM_CONNECT for the client socket
     * @param addr              Defines socket address/port information
     * @param timeout           Connection timeout. If 0 then wait forever.
     */
    void open_addr(OpenMode openMode = OpenMode::CREATE, const sockaddr_in* addr = nullptr,
                   std::chrono::milliseconds timeout = std::chrono::milliseconds(0));

    /**
     * Constructor
     * @param domain            Socket domain type
     * @param type              Socket type
     * @param protocol          Protocol type
     */
    explicit BaseSocket(SOCKET_ADDRESS_FAMILY domain = AF_INET, int32_t type = SOCK_STREAM, int32_t protocol = 0);

    /**
     * Deleted copy constructor
     * @param other             Other socket
     */
    BaseSocket(const BaseSocket& other) = delete;

    /**
     * Move constructor
     * @param other             Other socket
     */
    BaseSocket(BaseSocket&& other) noexcept = default;

    /**
     * @brief Destructor
     */
    virtual ~BaseSocket();

    /**
     * Deleted copy assignment
     * @param other             Other socket
     */
    BaseSocket& operator=(const BaseSocket& other) = delete;

    /**
     * Move assignment
     * @param other             Other socket
     */
    BaseSocket& operator=(BaseSocket&& other) noexcept = default;

    /**
     * Set blocking mode
     * @param blocking          Socket blocking mode flag
     */
    void blockingMode(bool blocking);

    /**
     * Returns number of bytes available in socket
     */
    [[nodiscard]] virtual size_t socketBytes();

    /**
     * Attaches socket handle
     * @param socketHandle      Existing socket handle
     */
    virtual void attach(SOCKET socketHandle, bool accept);

    /**
     * Detaches socket handle, setting it to INVALID_SOCKET.
     * Closes the socket without affecting socket handle.
     * @return Existing socket handle
     */
    virtual SOCKET detach();

    /**
     * Sets the host name
     * @param host              The host
     */
    void host(const Host& host);

    /**
     * Returns the host
     */
    [[nodiscard]] const Host& host() const
    {
        return m_host;
    }

    /**
     * Opens the client socket connection by host and port
     * @param host              The host
     * @param openMode          Socket open mode
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode
     * @param timeoutMS         Connection timeout. The default is 0 (wait forever)
     */
    void open(const Host& host = Host(), OpenMode openMode = OpenMode::CONNECT, bool blockingMode = true,
              std::chrono::milliseconds timeoutMS = std::chrono::milliseconds(0))
    {
        _open(host, openMode, blockingMode, timeoutMS);
    }

    /**
     * Opens the client socket connection by host and port
     * @param address           Address and port
     * @param openMode          Socket open mode
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode
     * @param timeoutMS         Connection timeout, std::chrono::milliseconds. The default is 0 (wait forever)
     */
    void open(const struct sockaddr_in& address, OpenMode openMode = OpenMode::CONNECT,
              bool blockingMode = true, std::chrono::milliseconds timeoutMS = std::chrono::milliseconds(0))
    {
        _open(address, openMode, blockingMode, timeoutMS);
    }

    /**
     * Binds the socket to port
     * @param address           Local IP address, or NULL if any
     * @param portNumber        The port number, or 0 if any
     */
    void bind(const char* address, uint32_t portNumber);

    /**
     * Opens the server socket connection on port (binds/listens)
     * @param portNumber        The port number
     */
    void listen(uint16_t portNumber = 0);

    /**
     * Closes the socket connection
     */
    virtual void close() noexcept;

    /**
     * Returns the current socket state
     * @returns true if socket is opened
     */
    [[nodiscard]] bool active() const
    {
        return m_sockfd != INVALID_SOCKET;
    }

    /**
     * Calls Unix fcntl() or Windows ioctlsocket()
     */
    int32_t control(int flag, const uint32_t* check) const;

    /**
     * Sets socket option value
     * Throws an error if not succeeded
     */
    void setOption(int level, int option, int value) const;

    /**
     * Gets socket option value
     *
     * Throws an error if not succeeded
     */
    void getOption(int level, int option, int& value) const;

    /**
     * Reads data from the socket in regular or SSL mode
     * @param buffer            The destination buffer
     * @param len              The destination buffer size
     * @returns the number of bytes read from the socket
     */
    [[nodiscard]] virtual size_t recv(uint8_t* buffer, size_t len);

    /**
     * Reads data from the socket in regular or TLS mode
     * @param buffer            The send buffer
     * @param len              The send data length
     * @returns the number of bytes sent the socket
     */
    [[nodiscard]] virtual size_t send(const uint8_t* buffer, size_t len);

    /**
     * Reads data from the socket
     * @param buffer            The memory buffer
     * @param size              The number of bytes to read
     * @param from              The source address
     * @returns the number of bytes read from the socket
     */
    [[nodiscard]] virtual size_t read(uint8_t* buffer, size_t size, sockaddr_in* from = nullptr);

    /**
     * Reads data from the socket
     * @param buffer            The memory buffer
     * @param size              The number of bytes to read
     * @returns the number of bytes read from the socket
     */
    [[nodiscard]] virtual size_t read(uint8_t* buffer, size_t size)
    {
        return read(buffer, size, nullptr);
    }

    /**
     * Reads data from the socket into memory buffer
     *
     * Buffer bytes() is set to number of bytes read
     * @param buffer            The output buffer
     * @param size              The number of bytes to read
     * @param from              The source address
     * @returns the number of bytes read from the socket
     */
    [[nodiscard]] virtual size_t read(Buffer& buffer, size_t size, sockaddr_in* from);

    /**
     * Reads data from the socket into memory buffer
     *
     * Buffer bytes() is set to number of bytes read
     * @param buffer            The output buffer
     * @param size              The number of bytes to read
     * @returns the number of bytes read from the socket
     */
    [[nodiscard]] size_t read(Buffer& buffer, size_t size)
    {
        return read(buffer, size, nullptr);
    }

    /**
     * Reads data from the socket into memory buffer
     *
     * Buffer bytes() is set to number of bytes read
     * @param buffer            The memory buffer
     * @param size              The number of bytes to read
     * @param from              The source address
     * @returns the number of bytes read from the socket
     */
    [[nodiscard]] virtual size_t read(String& buffer, size_t size, sockaddr_in* from);

    /**
     * Reads data from the socket into memory buffer
     *
     * Buffer bytes() is set to number of bytes read
     * @param buffer            The memory buffer
     * @param size              The number of bytes to read
     * @returns the number of bytes read from the socket
     */
    [[nodiscard]] size_t read(String& buffer, size_t size)
    {
        return read(buffer, size, nullptr);
    }

    /**
     * Writes data to the socket
     *
     * If size is omited then buffer is treated as zero-terminated string
     * @param buffer            The memory buffer
     * @param size              The memory buffer size
     * @param peer              The peer information
     * @returns the number of bytes written to the socket
     */
    virtual size_t write(const uint8_t* buffer, size_t size, const sockaddr_in* peer);

    /**
     * Writes data to the socket
     *
     * If size is omited then buffer is treated as zero-terminated string
     * @param buffer            The memory buffer
     * @param size              The memory buffer size
     * @returns the number of bytes written to the socket
     */
    size_t write(const uint8_t* buffer, size_t size)
    {
        return write(buffer, size, nullptr);
    }

    /**
     * Writes data to the socket
     * @param buffer            The memory buffer
     * @param peer              The peer information
     * @returns the number of bytes written to the socket
     */
    virtual size_t write(const Buffer& buffer, const sockaddr_in* peer);

    /**
     * Writes data to the socket
     * @param buffer            The memory buffer
     * @returns the number of bytes written to the socket
     */
    size_t write(const Buffer& buffer)
    {
        return write(buffer, nullptr);
    }

    /**
     * Writes data to the socket
     * @param buffer            The memory buffer
     * @param peer              The peer information
     * @returns the number of bytes written to the socket
     */
    virtual size_t write(const String& buffer, const sockaddr_in* peer);

    /**
     * Writes data to the socket
     * @param buffer            The memory buffer
     * @returns the number of bytes written to the socket
     */
    size_t write(const String& buffer)
    {
        return write(buffer, nullptr);
    }

    /**
     * Reports true if socket is ready for reading from it
     * @param timeout           Read timeout
     */
    [[nodiscard]] virtual bool readyToRead(std::chrono::milliseconds timeout);

    /**
     * Reports true if socket is ready for writing to it
     * @param timeout           Write timeout
     */
    [[nodiscard]] virtual bool readyToWrite(std::chrono::milliseconds timeout);

    /**
     * @brief Return current blocking mode state
     * @return Current blocking mode state
     */
    [[nodiscard]] bool blockingMode() const
    {
        return m_blockingMode;
    }

protected:
    /**
     * Set socket internal (OS) handle
     */
    void setSocketFD(SOCKET socket)
    {
        m_sockfd = socket;
    }

    /**
     * Get socket domain type
     */
    [[nodiscard]] int32_t domain() const
    {
        return m_domain;
    }

    /**
     * Get socket type
     */
    [[nodiscard]] int32_t type() const
    {
        return m_type;
    }

    /**
     * Get socket protocol
     */
    [[nodiscard]] int32_t protocol() const
    {
        return m_protocol;
    }

#ifdef _WIN32
    /**
     * WinSock initialization
     */
    static void init() noexcept;

    /**
     * WinSock cleanup
     */
    static void cleanup() noexcept;
#endif

    /**
     * Opens the client socket connection by host and port
     * @param host              The host
     * @param openMode          Socket open mode
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode
     * @param timeoutMS         Connection timeout. The default is 0 (wait forever)
     */
    virtual void _open(const Host& host, OpenMode openMode, bool blockingMode, std::chrono::milliseconds timeoutMS);

    /**
     * Opens the client socket connection by host and port
     * @param address           Address and port
     * @param openMode          Socket open mode
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode
     * @param timeoutMS         Connection timeout, std::chrono::milliseconds. The default is 0 (wait forever)
     */
    virtual void _open(const struct sockaddr_in& address, OpenMode openMode, bool blockingMode,
                       std::chrono::milliseconds timeoutMS)
    {
        // Implement in derived class
    }

private:
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
SP_EXPORT void throwSocketError(const String& message, const char* file, int line);

#define THROW_SOCKET_ERROR(msg) sptk::throwSocketError(msg, __FILE__, __LINE__)

/**
 * @}
 */
} // namespace sptk
