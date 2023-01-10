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

#include <sptk5/Buffer.h>
#include <sptk5/Exception.h>
#include <sptk5/Strings.h>
#include <sptk5/net/BaseSocket.h>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Thread-safe socket reader.
 */
class SP_EXPORT SocketReader
    : public Buffer
{
public:
    /**
     * Constructor
     * @param socket            Socket to work with
     * @param bufferSize        The desirable size of the internal buffer
     */
    explicit SocketReader(BaseSocket& socket, size_t bufferSize = 16384);

    /**
     * Resets reader buffer position
     */
    virtual void reset();

    /**
     * Closes socket
     */
    virtual void close();

    /**
     * Performs the buffered read
     * @param destination       Destination buffer
     * @param sz                Size of the destination buffer
     * @returns bytes read from the internal buffer
     */
    size_t read(uint8_t* destination, size_t sz);

    /**
     * Performs the buffered read
     * @param destination       Destination buffer
     * @param sz                Size of the destination buffer
     * @returns bytes read from the internal buffer
     */
    size_t read(Buffer& destination, size_t sz);

    /**
     * Performs the buffered read of LF-terminated string
     * @param dest              Destination buffer
     * @param delimiter         Line delimiter
     * @returns bytes read from the internal buffer
     */
    size_t readLine(Buffer& dest, char delimiter = '\n');

    /**
     * Performs the buffered read of LF-terminated string
     * @param dest              Destination buffer
     * @param delimiter         Line delimiter
     * @returns bytes read from the internal buffer
     */
    size_t readLine(String& dest, char delimiter = '\n');

    /**
     * Returns number of bytes available to read
     */
    [[nodiscard]] size_t availableBytes() const;

    /**
     * Returns true if there are bytes available to read
     * @param dest              Timeout waiting for data ready to read
     */
    [[nodiscard]] bool readyToRead(std::chrono::milliseconds timeout) const;

private:

    mutable std::mutex m_mutex; ///< Mutex protecting read operations
    BaseSocket& m_socket;       ///< Socket to read from
    uint32_t m_readOffset {0};  ///< Current offset in the read buffer

    /**
     * Performs the buffered read
     * @param destination              Destination buffer
     * @param sz                Size of the destination buffer
     * @param delimiter         Line delimiter
     * @param read_line          True if we want to read one line (ended with CRLF) only
     * @returns bytes read from the internal buffer
     */
    size_t read(uint8_t* destination, size_t sz, char delimiter, bool read_line);

    [[nodiscard]] int32_t readFromSocket();

    /**
     * Performs buffered read
     *
     * Data is read from the opened socket into a character buffer of limited size
     * @param destination       Destination buffer
     * @param size                Size of the destination buffer
     * @param delimiter         Line delimiter
     * @param read_line          True if we want to read one line (ended with CRLF) only
     * @returns number of bytes read
     */
    [[nodiscard]] int32_t bufferedRead(uint8_t* destination, size_t size, char delimiter, bool read_line);

    /**
     * Read more (as much as we can) from socket into buffer
     * @param availableBytes    Number of bytes already available in buffer
     */
    void readMoreFromSocket(int availableBytes);

    void handleReadFromSocketError(int error);
};

/**
 * @}
 */
} // namespace sptk
