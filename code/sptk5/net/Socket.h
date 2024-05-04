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

#include <sptk5/net/SocketVirtualMethods.h>

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
class SP_EXPORT Socket : public SocketVirtualMethods
{
    friend class SocketPool;

public:
    /**
     * Get socket internal (OS) handle
     */
    SocketType fd() const
    {
        return getSocketFdUnlocked();
    }

    /**
     * Constructor
     * @param domain            Socket domain type
     * @param type              Socket type
     * @param protocol          Protocol type
     */
    explicit Socket(SOCKET_ADDRESS_FAMILY domain = AF_INET, int32_t type = SOCK_STREAM, int32_t protocol = 0);

    /**
     * @brief Destructor
     */
    ~Socket() override;

    /**
     * Set blockingMode mode
     * @param blockingMode      Socket blockingMode mode flag
     */
    void blockingMode(bool blockingMode)
    {
        setBlockingModeUnlocked(blockingMode);
    }

    /**
     * Returns number of bytes available in socket
     */
    [[nodiscard]] size_t socketBytes() const
    {
        return getSocketBytesUnlocked();
    }

    /**
     * Attaches socket handle
     * @param socketHandle      Existing socket handle
     */
    void attach(SocketType socketHandle, bool accept)
    {
        return attachUnlocked(socketHandle, accept);
    }

    /**
     * Detaches socket handle, setting it to INVALID_SOCKET.
     * Closes the socket without affecting socket handle.
     * @return Existing socket handle
     */
    SocketType detach()
    {
        return detachUnlocked();
    }

    /**
     * Sets the host name
     * @param host              The host
     */
    void host(const Host& host)
    {
        const std::scoped_lock lock(m_mutex);
        setHostUnlocked(host);
    }

    /**
     * Returns the host
     */
    [[nodiscard]] const Host& host() const
    {
        const std::scoped_lock lock(m_mutex);
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
     * @param reusePort         If true then set SO_REUSEPORT
     */
    void bind(const char* address, uint32_t portNumber, bool reusePort = false)
    {
        bindUnlocked(address, portNumber, reusePort);
    }

    /**
     * Opens the server socket connection on port (binds/listens)
     * @param portNumber        The port number
     * @param reusePort         If true then set SO_REUSEPORT on listener socket
     */
    void listen(uint16_t portNumber = 0, bool reusePort = true)
    {
        const std::scoped_lock lock(m_mutex);
        listenUnlocked(portNumber, reusePort);
    }

    /**
     * Closes the socket connection
     */
    void close()
    {
        const std::scoped_lock lock(m_mutex);
        closeUnlocked();
    }

    /**
     * Returns the current socket state
     * @returns true if socket is opened
     */
    [[nodiscard]] bool active() const
    {
        return activeUnlocked();
    }

    /**
     * Sets socket option value
     * Throws an error if not succeeded
     */
    void setOption(int level, int option, int value) const
    {
        setOptionUnlocked(level, option, value);
    }

    /**
     * Gets socket option value
     *
     * Throws an error if not succeeded
     */
    void getOption(int level, int option, int& value) const
    {
        getOptionUnlocked(level, option, value);
    }

    /**
     * Reads data from the socket
     * @param buffer            The memory buffer
     * @param size              The number of bytes to read
     * @param from              The source address
     * @returns the number of bytes read from the socket
     */
    [[nodiscard]] size_t read(uint8_t* buffer, size_t size, sockaddr_in* from = nullptr)
    {
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
    size_t write(const uint8_t* buffer, size_t size, const sockaddr_in* peer = nullptr)
    {
        return writeUnlocked(buffer, size, peer);
    }

    /**
     * Writes data to the socket
     * @param buffer            The memory buffer
     * @param peer              The peer information
     * @returns the number of bytes written to the socket
     */
    size_t write(const Buffer& buffer, const sockaddr_in* peer = nullptr);

    /**
     * Writes data to the socket
     * @param buffer            The memory buffer
     * @param peer              The peer information
     * @returns the number of bytes written to the socket
     */
    size_t write(const String& buffer, const sockaddr_in* peer = nullptr);

    /**
     * Reports true if socket is ready for reading from it
     * @param timeout           Read timeout
     */
    [[nodiscard]] bool readyToRead(std::chrono::milliseconds timeout)
    {
        return readyToReadUnlocked(timeout);
    }

    /**
     * Reports true if socket is ready for writing to it
     * @param timeout           Write timeout
     */
    [[nodiscard]] virtual bool readyToWrite(std::chrono::milliseconds timeout)
    {
        return readyToWriteUnlocked(timeout);
    }

    /**
     * Return current blocking mode state
     * @return Current blocking mode state
     */
    [[nodiscard]] bool blockingMode() const
    {
        return getBlockingModeUnlocked();
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

protected:
    /**
     * Get socket domain type
     */
    [[nodiscard]] int32_t domain() const
    {
        return getDomainUnlocked();
    }

    /**
     * Get socket type
     */
    [[nodiscard]] int32_t type() const
    {
        return getTypeUnlocked();
    }

    /**
     * Get socket protocol
     */
    [[nodiscard]] int32_t protocol() const
    {
        return getProtocolUnlocked();
    }

    /**
     * @brief Get socket event data, used by SocketPool class
     * @return Socket event data
     */
    const uint8_t* getSocketEventData() const
    {
        std::scoped_lock lock(m_mutex);
        return m_socketEventData;
    }

    /**
     * @brief Set socket event data, used by SocketPool class
     * @param socketEventData   Socket event data
     */
    void setSocketEventData(const uint8_t* socketEventData)
    {
        std::scoped_lock lock(m_mutex);
        m_socketEventData = socketEventData;
    }


private:
    mutable std::mutex m_mutex;                 ///< Mutex that protects host data
    const uint8_t* m_socketEventData = nullptr; ///< Socket event data, used by SocketPool
};

/**
 * @}
 */
} // namespace sptk
