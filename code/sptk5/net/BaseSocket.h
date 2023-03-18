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

#include <sptk5/net/BaseSocketVirtualMethods.h>

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
class SP_EXPORT BaseSocket : public BaseSocketVirtualMethods
{
public:
    /**
     * Get socket internal (OS) handle
     */
    SOCKET fd() const
    {
        const std::scoped_lock lock(m_socketMutex);
        return getSocketFdUnlocked();
    }

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
    BaseSocket(BaseSocket&& other) noexcept = delete;

    /**
     * @brief Destructor
     */
    ~BaseSocket() override;

    /**
     * Deleted copy assignment
     * @param other             Other socket
     */
    BaseSocket& operator=(const BaseSocket& other) = delete;

    /**
     * Move assignment
     * @param other             Other socket
     */
    BaseSocket& operator=(BaseSocket&& other) noexcept = delete;

    /**
     * Set blockingMode mode
     * @param blockingMode      Socket blockingMode mode flag
     */
    void blockingMode(bool blockingMode)
    {
        const std::scoped_lock lock(m_socketMutex);
        setBlockModeUnlocked(blockingMode);
    }

    /**
     * Returns number of bytes available in socket
     */
    [[nodiscard]] size_t socketBytes() const
    {
        const std::scoped_lock lock(m_socketMutex);
        return getSocketBytesUnlocked();
    }

    /**
     * Attaches socket handle
     * @param socketHandle      Existing socket handle
     */
    void attach(SOCKET socketHandle, bool accept)
    {
        const std::scoped_lock lock(m_socketMutex);
        return attachUnlocked(socketHandle, accept);
    }

    /**
     * Detaches socket handle, setting it to INVALID_SOCKET.
     * Closes the socket without affecting socket handle.
     * @return Existing socket handle
     */
    SOCKET detach()
    {
        const std::scoped_lock lock(m_socketMutex);
        return detachUnlocked();
    }

    /**
     * Sets the host name
     * @param host              The host
     */
    void host(const Host& host)
    {
        const std::scoped_lock lock(m_socketMutex);
        setHostUnlocked(host);
    }

    /**
     * Returns the host
     */
    [[nodiscard]] const Host& host() const
    {
        const std::scoped_lock lock(m_socketMutex);
        return getHostUnlocked();
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
        openUnlocked(host, openMode, blockingMode, timeoutMS);
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
        openUnlocked(address, openMode, blockingMode, timeoutMS);
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
    void close()
    {
        const std::scoped_lock lock(m_socketMutex);
        closeUnlocked();
    }

    /**
     * Returns the current socket state
     * @returns true if socket is opened
     */
    [[nodiscard]] bool active() const
    {
        const std::scoped_lock lock(m_socketMutex);
        return activeUnlocked();
    }

    /**
     * Calls Unix fcntl() or Windows ioctlsocket()
     */
    int32_t control(int flag, const uint32_t* check) const;

    /**
     * Sets socket option value
     * Throws an error if not succeeded
     */
    void setOption(int level, int option, int value) const
    {
        const std::scoped_lock lock(m_socketMutex);
        setOptionUnlocked(level, option, value);
    }

    /**
     * Gets socket option value
     *
     * Throws an error if not succeeded
     */
    void getOption(int level, int option, int& value) const
    {
        const std::scoped_lock lock(m_socketMutex);
        getOptionUnlocked(level, option, value);
    }

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
    [[nodiscard]] size_t read(uint8_t* buffer, size_t size, sockaddr_in* from = nullptr)
    {
        const std::scoped_lock lock(m_socketMutex);
        return readUnlocked(buffer, size, from);
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
    [[nodiscard]] size_t read(Buffer& buffer, size_t size, sockaddr_in* from = nullptr);

    /**
     * Reads data from the socket into memory buffer
     *
     * Buffer bytes() is set to number of bytes read
     * @param buffer            The memory buffer
     * @param size              The number of bytes to read
     * @param from              The source address
     * @returns the number of bytes read from the socket
     */
    [[nodiscard]] size_t read(String& buffer, size_t size, sockaddr_in* from = nullptr);

    template<typename T>
    size_t read(T& value, sockaddr_in* from = nullptr)
    {
        const std::scoped_lock lock(m_socketMutex);
        return readUnlocked((uint8_t*) &value, sizeof(T), from);
    }

    /**
     * Writes data to the socket
     *
     * If size is omitted then buffer is treated as zero-terminated string
     * @param buffer            The memory buffer
     * @param size              The memory buffer size
     * @param peer              The peer information
     * @returns the number of bytes written to the socket
     */
    virtual size_t write(const uint8_t* buffer, size_t size, const sockaddr_in* peer);

    /**
     * Writes data to the socket
     *
     * If size is omitted then buffer is treated as zero-terminated string
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
    [[nodiscard]] bool readyToRead(std::chrono::milliseconds timeout)
    {
        const std::scoped_lock lock(m_socketMutex);
        return readyToReadUnlocked(timeout);
    }

    /**
     * Reports true if socket is ready for writing to it
     * @param timeout           Write timeout
     */
    [[nodiscard]] virtual bool readyToWrite(std::chrono::milliseconds timeout)
    {
        const std::scoped_lock lock(m_socketMutex);
        return readyToWriteUnlocked(timeout);
    }

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
        m_socketFd = socket;
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

private:
    mutable std::mutex m_socketMutex; ///< Mutex that protects socket data
};

/**
 * @}
 */
} // namespace sptk
