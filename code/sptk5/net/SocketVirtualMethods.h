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

#include <atomic>
#include <chrono>
#include <sptk5/DateTime.h>
#include <sptk5/Exception.h>
#include <sptk5/net/Host.h>

#ifndef _WIN32

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

/**
 * @brief A socket handle is an integer.
 */
using SocketType = int;
using SOCKET_ADDRESS_FAMILY = sa_family_t;

#ifdef __APPLE__
using socklen_t = int;
#endif

/**
 * @brief A value to indicate an invalid handle.
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
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief Virtual methods for the Socket class.
 *
 * All methods are not locking a mutex.
 */
class SP_EXPORT SocketVirtualMethods
{
public:
    /**
    * @brief A mode to open a socket, one of.
    */
    enum class OpenMode : uint8_t
    {
        CREATE,  ///< Only create (Typical UDP connectionless socket).
        CONNECT, ///< Connect (Typical TCP connection socket).
        BIND     ///< Bind (TCP listener).
    };

    /**
     * @brief Constructor.
     * @param domain            Socket domain type.
     * @param type              Socket type.
     * @param protocol          Protocol type.
     */
    explicit SocketVirtualMethods(SOCKET_ADDRESS_FAMILY domain = AF_INET, int32_t type = SOCK_STREAM, int32_t protocol = 0);

    /**
     * @brief Destructor.
     */
    virtual ~SocketVirtualMethods() = default;

protected:
    /**
     * @brief Opens the socket connection by address.
     * @param addr              Defines socket address/port information.
     * @param openMode          SOM_CREATE for the UDP socket, SOM_BIND for the server socket, and SOM_CONNECT for the client socket.
     * @param timeout           Connection timeout. If 0, then wait forever.
     * @param reusePort         If true, reuse port.
     * @param clientBindAddress Client binding IP address.
     */
    void openAddressUnlocked(const sockaddr_in& addr, OpenMode openMode = OpenMode::CREATE,
                             const std::chrono::milliseconds& timeout = std::chrono::milliseconds(0),
                             bool reusePort = true, const char* clientBindAddress = nullptr);

    /**
     * @brief Opens the client socket connection by host and port.
     * @param host              The host.
     * @param openMode          Socket open mode.
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode.
     * @param timeoutMS         Connection timeout. The default is 0 (wait forever).
     * @param clientBindAddress Client binding IP address.
     */
    virtual void openUnlocked(const Host& host, OpenMode openMode, bool blockingMode,
                              const std::chrono::milliseconds& timeoutMS, const char* clientBindAddress);

    /**
     * @brief Opens the client socket connection by host and port.
     * @param address           Address and port.
     * @param openMode          Socket open mode.
     * @param blockMode         Socket blocking (true) on non-blocking (false) mode.
     * @param timeoutMS         Connection timeout, std::chrono::milliseconds. The default is 0 (wait forever).
     * @param clientBindAddress Client binding IP address.
     */
    virtual void openUnlocked(const struct sockaddr_in& address, OpenMode openMode, bool blockMode,
                              const std::chrono::milliseconds& timeoutMS, const char* clientBindAddress);

    /**
     * @brief Returns the current socket state.
     * @returns true if the socket is opened.
     */
    [[nodiscard]] virtual bool activeUnlocked() const;

    /**
     * @brief Binds the socket to port.
     * @param address           Local IP address, or NULL if any.
     * @param portNumber        The port number, or 0 if any.
     * @param reusePort         If true then set SO_REUSEPORT.
     */
    void bindUnlocked(const char* address, uint32_t portNumber, bool reusePort = false);

    /**
     * @brief Opens the server socket connection on port (binds/listens).
     * @param portNumber        The port number.
     * @param reusePort         True if the port is reused.
     */
    void listenUnlocked(uint16_t portNumber, bool reusePort);

    /**
     * @brief Close socket.
     */
    virtual void closeUnlocked();

    /**
     * @brief Get the socket OS handle.
     */
    SocketType getSocketFdUnlocked() const;

    /**
     * @brief Set the socket OS handle.
     */
    void setSocketFdUnlocked(SocketType socket);

    /**
     * @brief Set the host.
     * @param host              The host.
     */
    void setHostUnlocked(const Host& host);

    /**
     * @brief Return the host.
     */
    [[nodiscard]] const Host& getHostUnlocked() const;

    /**
     * @brief Return the current blocking mode state.
     * @return Current blocking mode state.
     */
    [[nodiscard]] bool getBlockingModeUnlocked() const;

    /**
     * @brief Set blockingMode mode.
     * @param blockingMode      Socket blockingMode mode flag.
     */
    void setBlockingModeUnlocked(bool blockingMode);

    /**
     * @brief Sets socket option value.
     * Throws an error if not succeeded.
     */
    void setOptionUnlocked(int level, int option, int value) const;

    /**
     * @brief Gets socket option value.
     *
     * Throws an error if not succeeded.
     */
    void getOptionUnlocked(int level, int option, int& value) const;

    /**
     * @brief Returns the number of bytes available in the socket.
     */
    [[nodiscard]] virtual size_t getSocketBytesUnlocked() const;

    /**
     * @brief Attaches the socket handle.
     * @param socketHandle      Existing socket handle.
     * @param accept            True if the socket is attached for accepting connection.
     */
    virtual void attachUnlocked(SocketType socketHandle, bool accept);

    /**
     * @brief Detaches the socket handle, setting it to INVALID_SOCKET.
     * Closes the socket without affecting the returned socket handle.
     * @return Detached socket handle.
     */
    virtual SocketType detachUnlocked();

    /**
     * @brief Reports true if the socket is ready for reading from it.
     * @param timeout           Read timeout.
     */
    [[nodiscard]] virtual bool readyToReadUnlocked(const std::chrono::milliseconds& timeout);

    /**
     * @brief Reports true if the socket is ready for writing to it.
     * @param timeout           Write timeout.
     */
    [[nodiscard]] virtual bool readyToWriteUnlocked(const std::chrono::milliseconds& timeout);

    /**
     * @brief Reads data from the socket in regular or SSL mode.
     * @param buffer            The destination buffer.
     * @param len              The destination buffer size.
     * @returns the number of bytes read from the socket.
     */
    [[nodiscard]] virtual size_t recvUnlocked(uint8_t* buffer, size_t len);

    /**
     * @brief Reads data from the socket.
     * @param buffer            The memory buffer.
     * @param size              The number of bytes to read.
     * @param from              The source address.
     * @returns the number of bytes read from the socket.
     */
    [[nodiscard]] virtual size_t readUnlocked(uint8_t* buffer, size_t size, sockaddr* from);

    /**
     * @brief Reads data from the socket in regular or TLS mode.
     * @param buffer            The send buffer.
     * @param len              The data length.
     * @returns the number of bytes sent to the socket.
     */
    [[nodiscard]] virtual size_t sendUnlocked(const uint8_t* buffer, size_t len);

    /**
     * @brief Writes data to the socket.
     *
     * If size is omitted, then the buffer is treated as a zero-terminated string.
     * @param buffer            The memory buffer.
     * @param size              The memory buffer size.
     * @param peer              The peer information.
     * @returns the number of bytes written to the socket.
     */
    virtual size_t writeUnlocked(const uint8_t* buffer, size_t size, const sockaddr* peer);

    /**
     * @brief Get socket domain type.
     */
    [[nodiscard]] int32_t getDomainUnlocked() const;

    /**
     * @brief Get socket type.
     */
    [[nodiscard]] int32_t getTypeUnlocked() const;

    /**
     * @brief Get socket protocol.
     */
    [[nodiscard]] int32_t getProtocolUnlocked() const;

private:
    std::atomic<SocketType> m_socketFd {INVALID_SOCKET}; ///< Socket OS handle. Atomic: fd() reads it
                                                         ///< locklessly while close()/attach() write it under the owner's lock.
    int32_t    m_domain;                    ///< Socket domain type.
    int32_t    m_type;                      ///< Socket type.
    int32_t    m_protocol;                  ///< Socket protocol.
    Host       m_host;                      ///< Host.
    bool       m_blockingMode {false};      ///< Blocking mode flag.
};

/**
 * @brief Translates Windows socket error to corresponding errno.
 * For Linux, simply returns errno value.
 * @return errno equivalent.
 */
[[nodiscard]] SP_EXPORT int getSocketError(int nativeErrorCode = -1);

/**
 * @brief Throws socket exception with the error description retrieved from the socket state.
 * @param message           Error message.
 * @param location          Source location.
 */
[[noreturn]] SP_EXPORT void throwSocketError(const String& message, const std::source_location& location = std::source_location::current());

/**
 * @}
 */

} // namespace sptk
