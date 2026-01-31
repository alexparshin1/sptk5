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

#include <sptk5/Buffer.h>
#include <sptk5/Strings.h>
#include <sptk5/net/TCPSocket.h>

namespace sptk {

/**
 * @addtogroup utility Utility Classes.
 * @{
 */

/**
 * @brief Thread-safe socket reader.
 */
class SP_EXPORT SocketReader
{
public:
    /**
     * @brief Constructor.
     * @param socket            Socket to work with.
     * @param bufferSize        The desirable size of the internal buffer.
     */
    explicit SocketReader(TCPSocket& socket, size_t bufferSize = 16384);

    /**
     * @brief Destructor.
     */
    virtual ~SocketReader() = default;

    /**
     * @brief Clears read buffer.
     */
    virtual void clear();

    /**
     * @brief Closes socket.
     */
    virtual void close();

    /**
     * @brief Performs the buffered read.
     * @param destination       Destination buffer.
     * @param sz                Size of the destination buffer.
     * @returns bytes read from the internal buffer.
     */
    size_t read(uint8_t* destination, size_t sz);

    /**
     * @brief Performs the buffered read.
     * @param destination       Destination buffer.
     * @param sz                Size of the destination buffer.
     * @returns bytes read from the internal buffer.
     */
    size_t read(Buffer& destination, size_t sz);

    /**
     * @brief Performs the buffered read.
     * @param destination       Destination buffer.
     * @returns bytes read from the internal buffer.
     */
    template<typename T>
        requires is_socket_readable<T>
    size_t read(T& destination)
    {
        return read(std::bit_cast<uint8_t*>(&destination), sizeof(T));
    }

    /**
     * @brief Performs the buffered read of LF-terminated data.
     * @param dest              Destination buffer.
     * @param delimiter         Line delimiter.
     * @returns bytes read from the internal buffer.
     */
    size_t readLine(Buffer& dest, char delimiter = '\n');

    /**
     * @brief Performs the buffered read of LF-terminated string.
     * @param dest              Destination buffer.
     * @param delimiter         Line delimiter.
     * @returns bytes read from the internal buffer.
     */
    size_t readLine(String& dest, char delimiter = '\n');

    /**
     * @brief Return the number of bytes available to read.
     */
    [[nodiscard]] size_t availableBytes() const;

    /**
     * @brief Check if the reader has at least the number of bytes available to read.
     * @param bytesToRead       Bytes to read.
     * @returns true if the reader has at least number of bytes available to read.
     */
    [[nodiscard]] bool canRead(size_t bytesToRead) const;

    /**
     * @brief Return true if there are bytes available to read.
     * @param timeout           Timeout waiting for data ready to read.
     */
    [[nodiscard]] bool readyToRead(const std::chrono::milliseconds& timeout) const;

    /**
     * @brief Return reader's socket.
     */
    [[nodiscard]] TCPSocket& socket() const;

    /**
     *
     * @brief Return true if the socket is active.
     */
    [[nodiscard]] bool active() const
    {
        std::scoped_lock const lock(m_mutex);
        return m_socket.active();
    }

    /**
     * @brief Performs buffered read.
     * @param destination       Destination buffer.
     * @param size              Size of the destination buffer.
     * @param delimiter         Line delimiter.
     * @returns bytes read from the internal buffer.
     */
    [[nodiscard]] size_t readLine(uint8_t* destination, size_t size, char delimiter);

private:
    mutable std::mutex m_mutex;          ///< Mutex protecting read operations.
    TCPSocket&         m_socket;         ///< Socket to read from.
    size_t             m_readOffset {0}; ///< Current offset in the read buffer.
    Buffer             m_buffer;         ///< Read buffer

    [[nodiscard]] size_t readFromSocket();

    /**
     * @brief Performs buffered read.
     *
     * Data is read from the opened socket into a buffer of limited size.
     * @param destination       Destination buffer.
     * @param size              Size of the destination buffer.
     * @returns number of bytes read.
     */
    [[nodiscard]] size_t bufferedRead(uint8_t* destination, size_t size);

    /**
     * @brief Performs buffered read.
     *
     * Data is read from the opened socket into a character buffer of limited size.
     * @param destination       Destination buffer.
     * @param size              Size of the destination buffer.
     * @param delimiter         Line delimiter.
     * @param endOfLine         End-of-line flag (output).
     * @returns number of bytes read.
     */
    [[nodiscard]] size_t bufferedReadLine(uint8_t* destination, size_t size, char delimiter, bool& endOfLine);

    /**
     * @brief Read more (as much as we can) from socket into buffer.
     * @param availableBytes    Number of bytes already available in the buffer.
     */
    void readMoreFromSocket(size_t availableBytes);

    /**
     * @brief Handle socket read error.
     * @param error             Error code.
     */
    void handleReadFromSocketError(int error) const;
};

/**
 * @}
 */
} // namespace sptk
