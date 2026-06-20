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

#include "sptk5/Buffer.h"


#include <sptk5/net/SocketVirtualMethods.h>

namespace sptk {
template<typename T>
concept is_integral_array = std::is_array_v<T> && std::is_integral_v<std::remove_all_extents_t<T>>;

template<typename T>
concept is_floating_point_array = std::is_array_v<T> && std::is_floating_point_v<std::remove_all_extents_t<T>>;

template<typename T>
concept is_socket_readable = std::is_integral_v<T> || std::is_floating_point_v<T> || is_integral_array<T> || is_floating_point_array<T> || std::is_enum_v<T>;

/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief Generic socket.
 *
 * Allows establishing a network connection to the host by name and port address.
 */
class SP_EXPORT Socket : public SocketVirtualMethods
{
    friend class SocketPool;

public:
    /**
     * @brief Get socket internal (OS) handle.
     */
    SocketType fd() const
    {
        return getSocketFdUnlocked();
    }

    /**
     * @brief Constructor.
     * @param domain            Socket domain type.
     * @param type              Socket type.
     * @param protocol          Protocol type.
     */
    explicit Socket(SOCKET_ADDRESS_FAMILY domain = AF_INET, int32_t type = SOCK_STREAM, int32_t protocol = 0);

    /**
     * @brief Destructor.
     */
    ~Socket() override;

    /**
     * @brief Set blockingMode mode.
     * @param blockingMode      Socket blockingMode mode flag.
     */
    void blockingMode(const bool blockingMode)
    {
        setBlockingModeUnlocked(blockingMode);
    }

    /**
     * @brief Returns the number of bytes available in the socket.
     */
    [[nodiscard]] size_t socketBytes() const
    {
        return getSocketBytesUnlocked();
    }

    /**
     * @brief Attaches the socket handle.
     * @param socketHandle      Existing socket handle.
     * @param accept            Socket is attached for accepting connection.
     */
    void attach(const SocketType socketHandle, const bool accept)
    {
        std::scoped_lock lock(m_mutex);
        return attachUnlocked(socketHandle, accept);
    }

    /**
     * @brief Detaches the socket handle, setting it to INVALID_SOCKET.
     * The original socket handle is returned.
     * @return Detached socket handle.
     */
    SocketType detach()
    {
        std::scoped_lock lock(m_mutex);
        return detachUnlocked();
    }

    /**
     * @brief Sets the host name.
     * @param host              The host.
     */
    void host(const Host& host)
    {
        const std::scoped_lock lock(m_mutex);
        setHostUnlocked(host);
    }

    /**
     * @brief Returns the host.
     */
    [[nodiscard]] Host host() const
    {
        const std::scoped_lock lock(m_mutex);
        return getHostUnlocked();
    }

    /**
     * @brief Opens the client socket connection by host and port.
     * @param host              The host.
     * @param openMode          Socket open mode.
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode.
     * @param timeoutMS         Connection timeout. The default is 0 (wait forever).
     * @param clientBindAddress Client bind address.
     */
    void open(const Host& host = Host(), const OpenMode openMode = OpenMode::CONNECT, const bool blockingMode = true,
              const std::chrono::milliseconds& timeoutMS = std::chrono::milliseconds(0), const char* clientBindAddress = nullptr)
    {
        std::scoped_lock lock(m_mutex);
        openUnlocked(host, openMode, blockingMode, timeoutMS, clientBindAddress);
    }

    /**
     * @brief Opens the client socket connection by host and port.
     * @param address           Address and port.
     * @param openMode          Socket open mode.
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode.
     * @param timeoutMS         Connection timeout, std::chrono::milliseconds. The default is 0 (wait forever).
     * @param clientBindAddress Client bind address.
     */
    void open(const sockaddr_in& address, const OpenMode openMode = OpenMode::CONNECT,
              const bool blockingMode = true, const std::chrono::milliseconds& timeoutMS = std::chrono::milliseconds(0),
              const char* clientBindAddress = nullptr)
    {
        std::scoped_lock lock(m_mutex);
        openUnlocked(address, openMode, blockingMode, timeoutMS, clientBindAddress);
    }

    /**
     * @brief Binds the socket to port.
     * @param address           Local IP address, or NULL if any.
     * @param portNumber        The port number, or 0 if any.
     * @param reusePort         If true then set SO_REUSEPORT.
     */
    void bind(const char* address, const uint32_t portNumber, const bool reusePort = false)
    {
        std::scoped_lock lock(m_mutex);
        bindUnlocked(address, portNumber, reusePort);
    }

    /**
     * @brief Opens the server socket connection on port (binds/listens).
     * @param portNumber        The port number.
     * @param reusePort         If true, then set SO_REUSEPORT on listener socket.
     */
    void listen(const uint16_t portNumber = 0, const bool reusePort = true)
    {
        const std::scoped_lock lock(m_mutex);
        listenUnlocked(portNumber, reusePort);
    }

    /**
     * @brief Closes the socket connection.
     */
    void close()
    {
        std::scoped_lock lock(m_mutex);
        closeUnlocked();
    }

    /**
     * @brief Returns the current socket state.
     * @returns true if the socket is opened.
     */
    [[nodiscard]] bool active() const
    {
        std::scoped_lock lock(m_mutex);
        return activeUnlocked();
    }

    /**
     * @brief Sets socket option value.
     * Throws an error if not succeeded.
     */
    void setOption(const int level, const int option, const int value) const
    {
        std::scoped_lock lock(m_mutex);
        setOptionUnlocked(level, option, value);
    }

    /**
     * @brief Gets socket option value.
     *
     * Throws an error if not succeeded.
     */
    void getOption(const int level, const int option, int& value) const
    {
        std::scoped_lock lock(m_mutex);
        getOptionUnlocked(level, option, value);
    }

    /**
     * @brief Reads data from the socket.
     * @param buffer            The memory buffer.
     * @param size              The number of bytes to read.
     * @param from              The source address.
     * @returns the number of bytes read from the socket.
     */
    size_t read(uint8_t* buffer, const size_t size, sockaddr* from = nullptr)
    {
        std::scoped_lock lock(m_mutex);
        return readUnlocked(buffer, size, from);
    }

    /**
     * @brief Reads data from the socket into the memory buffer.
     *
     * Buffer bytes() is set to the number of bytes read.
     * @param buffer            The output buffer.
     * @param size              The number of bytes to read.
     * @param from              The source address.
     * @returns the number of bytes read from the socket.
     */
    size_t read(Buffer& buffer, size_t size, sockaddr* from = nullptr);

    /**
     * @brief Reads data from the socket into the memory buffer.
     *
     * Buffer bytes() is set to the number of bytes read.
     * @param buffer            The memory buffer.
     * @param size              The number of bytes to read.
     * @param from              The source address.
     * @returns the number of bytes read from the socket.
     */
    size_t read(String& buffer, size_t size, sockaddr* from = nullptr);

    template<typename T>
        requires is_socket_readable<T>
    size_t read(T& value, sockaddr* from = nullptr)
    {
        std::scoped_lock lock(m_mutex);
        return readUnlocked(reinterpret_cast<uint8_t*>(&value), sizeof(T), from);
    }

    /**
     * @brief Writes data to the socket.
     *
     * If size is omitted, the buffer is treated as  a zero-terminated string.
     * @param buffer            The memory buffer.
     * @param size              The memory buffer size.
     * @param peer              The peer information.
     * @returns the number of bytes written to the socket.
     */
    size_t write(const uint8_t* buffer, const size_t size, const sockaddr* peer = nullptr)
    {

        std::scoped_lock lock(m_mutex);
        return writeUnlocked(buffer, size, peer);
    }

    /**
     * @brief Writes data to the socket.
     * @param buffer            The memory buffer.
     * @param peer              The peer information.
     * @returns the number of bytes written to the socket.
     */
    size_t write(const Buffer& buffer, const sockaddr* peer = nullptr);

    /**
     * @brief Writes data to the socket.
     * @param buffer            The memory buffer.
     * @param peer              The peer information.
     * @returns the number of bytes written to the socket.
     */
    size_t write(const String& buffer, const sockaddr* peer = nullptr);

    /**
     * @brief Reports true if the socket is ready for reading from it.
     * @param timeout           Read timeout.
     */
    [[nodiscard]] bool readyToRead(const std::chrono::milliseconds& timeout)
    {
        const std::scoped_lock lock(m_mutex);
        return readyToReadUnlocked(timeout);
    }

    /**
     * @brief Reports true if the socket is ready for writing to it.
     * @param timeout           Write timeout.
     */
    [[nodiscard]] virtual bool readyToWrite(const std::chrono::milliseconds& timeout)
    {
        std::scoped_lock lock(m_mutex);
        return readyToWriteUnlocked(timeout);
    }

    /**
     * @brief Return the current blocking mode state.
     * @return Current blocking mode state.
     */
    [[nodiscard]] bool blockingMode() const
    {
        return getBlockingModeUnlocked();
    }

#ifdef _WIN32
    /**
     * @brief WinSock initialization.
     */
    static void init() noexcept;

    /**
     * @brief WinSock cleanup.
     */
    static void cleanup() noexcept;
#endif

protected:
    std::mutex& getMutex() const
    {
        return m_mutex;
    }

    /**
     * @brief Get socket domain type.
     */
    [[nodiscard]] int32_t domain() const
    {
        return getDomainUnlocked();
    }

    /**
     * @brief Get socket type.
     */
    [[nodiscard]] int32_t type() const
    {
        return getTypeUnlocked();
    }

    /**
     * @brief Get socket protocol.
     */
    [[nodiscard]] int32_t protocol() const
    {
        return getProtocolUnlocked();
    }

private:
    mutable std::mutex m_mutex; ///< Mutex that protects host data.
};

/**
 * @}
 */
} // namespace sptk
