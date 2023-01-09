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
 * Buffered Socket reader.
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
     * Connects the reader to the socket handle
     */
    void open();

    /**
     * Disconnects the reader from the socket handle, and compacts allocated memory
     */
    void close() noexcept;

    /**
     * Performs the buffered read
     * @param destination              Destination buffer
     * @param sz                Size of the destination buffer
     * @param delimiter         Line delimiter
     * @param read_line          True if we want to read one line (ended with CRLF) only
     * @param from              An optional structure for source address
     * @returns bytes read from the internal buffer
     */
    size_t read(uint8_t* destination, size_t sz, char delimiter, bool read_line, struct sockaddr_in* from = nullptr);

    /**
     * Performs the buffered read of LF-terminated string
     * @param dest              Destination buffer
     * @param delimiter         Line delimiter
     * @returns bytes read from the internal buffer
     */
    size_t readLine(Buffer& dest, char delimiter);

    /**
     * Returns number of bytes available to read
     */
    [[nodiscard]] size_t availableBytes() const;

    /**
     * Read more (as much as we can) from socket into buffer
     * @param availableBytes    Number of bytes already available in buffer
     */
    void readMoreFromSocket(int availableBytes);

private:
    /**
     * Socket to read from
     */
    BaseSocket& m_socket;

    /**
     * Current offset in the read buffer
     */
    uint32_t m_readOffset {0};

    [[nodiscard]] int32_t readFromSocket(sockaddr_in* from);

    /**
     * Performs buffered read
     *
     * Data is read from the opened socket into a character buffer of limited size
     * @param destination       Destination buffer
     * @param size                Size of the destination buffer
     * @param delimiter         Line delimiter
     * @param read_line          True if we want to read one line (ended with CRLF) only
     * @param from              An optional structure for source address
     * @returns number of bytes read
     */
    [[nodiscard]] int32_t bufferedRead(uint8_t* destination, size_t size, char delimiter, bool read_line,
                                       struct sockaddr_in* from = nullptr);

    void handleReadFromSocketError(int error);
};

/**
 * @}
 */
} // namespace sptk
