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
#include <sptk5/net/Proxy.h>

#ifndef _WIN32

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

/**
 * A socket handle is an integer
 */
using SOCKET = int;
using SOCKET_ADDRESS_FAMILY = sa_family_t;

/**
 * A value to indicate an invalid handle
 */
#define INVALID_SOCKET (-1)

#else
#include <winsock2.h>

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
 * Buffered Socket reader.
 */
class SP_EXPORT TCPSocketReader
    : public Buffer
{
public:
    /**
     * Constructor
     * @param socket            Socket to work with
     * @param bufferSize        The desirable size of the internal buffer
     */
    explicit TCPSocketReader(BaseSocket& socket, size_t bufferSize = 16384);

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
 * Generic TCP socket.
 *
 * Allows to establish a network connection
 * to the host by name and port address
 */
class SP_EXPORT TCPSocket
    : public BaseSocket
{
public:
    /**
    * Constructor
    * @param domain            Socket domain type
    * @param type              Socket type
    * @param protocol          Protocol type
    */
    explicit TCPSocket(SOCKET_ADDRESS_FAMILY domain = AF_INET, int32_t type = SOCK_STREAM, int32_t protocol = 0);

    /**
    * Destructor
    */
    ~TCPSocket() override;

    /**
     * Set proxy
     * @param proxy             Proxy.
     */
    void setProxy(std::shared_ptr<Proxy> proxy);

    /**
     * Close socket connection
     */
    void close() noexcept override;

    /**
     * In server mode, waits for the incoming connection.
     *
     * When incoming connection is made, exits returning the connection info
     * @param clientSocketFD    Connected client socket FD
     * @param clientInfo        Connected client info
     * @param timeout           Accept operation timeout
     * @returns                 True if accepted a connection
     */
    [[nodiscard]] virtual bool accept(SOCKET& clientSocketFD, struct sockaddr_in& clientInfo, std::chrono::milliseconds timeout);

    /**
     * Returns number of bytes available in socket
     */
    size_t socketBytes() override;

    /**
     * Reports true if socket is ready for reading from it
     * @param timeout           Read timeout
     */
    bool readyToRead(std::chrono::milliseconds timeout) override;

    /**
     * Reads one line from the socket into existing memory buffer
     *
     * The output string should fit the buffer or it will be returned incomplete.
     * @param buffer            The destination buffer
     * @param size              The destination buffer size
     * @param delimiter         Line delimiter
     * @returns the number of bytes read from the socket
     */
    [[deprecated("Use SocketReader instead")]]
    size_t readLine(char* buffer, size_t size, char delimiter = '\n');

    /**
     * Reads one line (terminated with CRLF) from the socket into existing memory buffer
     *
     * The memory buffer is extended automatically to fit the string.
     * @param buffer            The destination buffer
     * @param delimiter         Line delimiter
     * @returns the number of bytes read from the socket
     */
    [[deprecated("Use SocketReader instead")]]
    size_t readLine(Buffer& buffer, char delimiter = '\n');

    /**
     * Reads one line (terminated with CRLF) from the socket into string
     * @param str                 The destination string
     * @param delimiter         Line delimiter
     * @returns the number of bytes read from the socket
     */
    [[deprecated("Use SocketReader instead")]]
    size_t readLine(String& str, char delimiter = '\n');

    /**
     * Reads data from the socket
     * @param buffer            The memory buffer
     * @param size              The number of bytes to read
     * @param from              An optional structure for source address
     * @returns the number of bytes read from the socket
     */
    size_t read(uint8_t* buffer, size_t size, sockaddr_in* from = nullptr) override;

    /**
     * Reads data from the socket into memory buffer
     *
     * Buffer bytes() is set to number of bytes read
     * @param buffer            The memory buffer
     * @param size              Number of bytes to read from socket
     * @param from              An optional structure for source address
     * @returns the number of bytes read from the socket
     */
    size_t read(Buffer& buffer, size_t size, sockaddr_in* from = nullptr) override;

    /**
     * Reads data from the socket into memory buffer
     *
     * Buffer bytes() is set to number of bytes read
     * @param buffer            The memory buffer
     * @param size              Number of bytes to read from socket
     * @param from              An optional structure for source address
     * @returns the number of bytes read from the socket
     */
    size_t read(String& buffer, size_t size, sockaddr_in* from = nullptr) override;

    template<typename T>
    size_t read(T& value, sockaddr_in* from = nullptr)
    {
        return read((uint8_t*) &value, sizeof(T), from);
    }

protected:
    /**
     * Access to internal socket reader for derived classes
     * @return internal socket reader
     */
    TCPSocketReader& reader()
    {
        return m_reader;
    }

    /**
     * Opens the client socket connection by host and port
     * @param host              The host
     * @param openMode          Socket open mode
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode
     * @param timeout           Connection timeout. The default is 0 (wait forever)
     */
    void _open(const Host& host, OpenMode openMode, bool blockingMode, std::chrono::milliseconds timeout) override;

    /**
     * Opens the client socket connection by host and port
     * @param address           Address and port
     * @param openMode          Socket open mode
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode
     * @param timeout           Connection timeout. The default is 0 (wait forever)
     */
    void _open(const struct sockaddr_in& address, OpenMode openMode, bool blockingMode,
               std::chrono::milliseconds timeout) override;

    /**
     * Get proxy information
     * @return
     */
    Proxy* proxy()
    {
        return m_proxy.get();
    }

private:
    TCPSocketReader m_reader;       ///< Buffered socket reader
    std::shared_ptr<Proxy> m_proxy; ///< Optional proxy
    Buffer m_stringBuffer;          ///< Buffer to read a line
};

using STCPSocket = std::shared_ptr<TCPSocket>;

/**
 * @}
 */
} // namespace sptk
