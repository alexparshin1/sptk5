/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       TCPSocket.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __TCPSOCKET_H__
#define __TCPSOCKET_H__

#ifndef _WIN32
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <unistd.h>

    /**
     * A socket handle is an integer
     */
    typedef int SOCKET;
    typedef sa_family_t SOCKET_ADDRESS_FAMILY;

    /**
     * A value to indicate an invalid handle
     */
    #define INVALID_SOCKET -1

#else
    #include <winsock2.h>
    #include <windows.h>
    typedef int socklen_t;
    typedef unsigned short SOCKET_ADDRESS_FAMILY;
#endif

#include <sptk5/Buffer.h>
#include <sptk5/Exception.h>
#include <sptk5/Strings.h>
#include <sptk5/net/BaseSocket.h>

namespace sptk
{

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Buffered Socket reader.
 */
class SP_EXPORT TCPSocketReader: protected Buffer
{
    /**
     * Socket to read from
     */
    BaseSocket&    m_socket;

    /**
     * Current offset in the read buffer
     */
    uint32_t        m_readOffset;


    /**
     * @brief Performs buffered read
     *
     * Data is read from the opened socket into a character buffer of limited size
     * @param destination       Destination buffer
     * @param sz                Size of the destination buffer
     * @param delimiter         Line delimiter
     * @param readLine          True if we want to read one line (ended with CRLF) only
     * @param from              An optional structure for source address
     * @returns number of bytes read
     */
    int32_t bufferedRead(char *destination, size_t sz, char delimiter, bool readLine, struct sockaddr_in* from = NULL);

public:

    /**
     * @brief Constructor
     * @param socket            Socket to work with
     * @param bufferSize        The desirable size of the internal buffer
     */
    TCPSocketReader(BaseSocket& socket, size_t bufferSize = 65536);

    /**
     * @brief Connects the reader to the socket handle
     */
    void open();

    /**
     * @brief Disconnects the reader from the socket handle, and compacts allocated memory
     */
    void close() noexcept;

    /**
     * @brief Performs the buffered read
     * @param destination              Destination buffer
     * @param sz                Size of the destination buffer
     * @param delimiter         Line delimiter
     * @param readLine          True if we want to read one line (ended with CRLF) only
     * @param from              An optional structure for source address
     * @returns bytes read from the internal buffer
     */
    size_t read(char* destination, size_t sz, char delimiter, bool readLine, struct sockaddr_in* from = NULL);

    /**
     * @brief Performs the buffered read of LF-terminated string
     * @param dest              Destination buffer
     * @param delimiter         Line delimiter
     * @returns bytes read from the internal buffer
     */
    size_t readLine(Buffer& dest, char delimiter);

    /**
     * Returns number of bytes available to read
     */
    size_t availableBytes() const;
};

/**
 * @brief Generic TCP socket.
 *
 * Allows to establish a network connection
 * to the host by name and port address
 */
class SP_EXPORT TCPSocket: public BaseSocket
{
protected:
    /**
     * Socket buffered reader
     */
    TCPSocketReader     m_reader;

    /**
     * Buffer to read a line
     */
    Buffer              m_stringBuffer;

    /**
     * @brief Opens the client socket connection by host and port
     * @param host              The host
     * @param openMode          Socket open mode
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode
     * @param timeout           Connection timeout. The default is 0 (wait forever)
     */
    void _open(const Host& host, CSocketOpenMode openMode, bool blockingMode, std::chrono::milliseconds timeout) override;

    /**
     * @brief Opens the client socket connection by host and port
     * @param address           Address and port
     * @param openMode          Socket open mode
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode
     * @param timeout           Connection timeout. The default is 0 (wait forever)
     */
    void _open(const struct sockaddr_in& address, CSocketOpenMode openMode, bool blockingMode, std::chrono::milliseconds timeout) override;

public:
    /**
    * @brief Constructor
    * @param domain            Socket domain type
    * @param type              Socket type
    * @param protocol          Protocol type
    */
    TCPSocket(SOCKET_ADDRESS_FAMILY domain = AF_INET, int32_t type = SOCK_STREAM, int32_t protocol = 0);

    /**
    * @brief Destructor
    */
    virtual ~TCPSocket() = default;

    /**
     * Close socket connection
     */
    void close() noexcept override;

    /**
     * @brief In server mode, waits for the incoming connection.
     *
     * When incoming connection is made, exits returning the connection info
     * @param clientSocketFD    Connected client socket FD
     * @param clientInfo        Connected client info
     */
    void accept(SOCKET& clientSocketFD, struct sockaddr_in& clientInfo) override;

    /**
     * @brief Returns number of bytes available in socket
     */
    size_t socketBytes() override;

    /**
     * @brief Reports true if socket is ready for reading from it
     * @param timeout           Read timeout
     */
    bool readyToRead(std::chrono::milliseconds timeout) override;

    /**
     * @brief Reports true if socket is ready for reading from it
     * @param timeout           Read timeout date and time
     */
    bool readyToRead(DateTime timeout) override;

    /**
     * @brief Reads one line from the socket into existing memory buffer
     *
     * The output string should fit the buffer or it will be returned incomplete.
     * @param buffer            The destination buffer
     * @param size              The destination buffer size
     * @param delimiter         Line delimiter
     * @returns the number of bytes read from the socket
     */
    size_t readLine(char *buffer, size_t size, char delimiter='\n');

    /**
     * @brief Reads one line (terminated with CRLF) from the socket into existing memory buffer
     *
     * The memory buffer is extended automatically to fit the string.
     * @param buffer            The destination buffer
     * @param delimiter         Line delimiter
     * @returns the number of bytes read from the socket
     */
    size_t readLine(Buffer& buffer, char delimiter='\n');

    /**
     * @brief Reads one line (terminated with CRLF) from the socket into string
     * @param s                 The destination string
     * @param delimiter         Line delimiter
     * @returns the number of bytes read from the socket
     */
    size_t readLine(std::string& s, char delimiter='\n');

    /**
     * @brief Reads data from the socket
     * @param buffer            The memory buffer
     * @param size              The number of bytes to read
     * @param from              An optional structure for source address
     * @returns the number of bytes read from the socket
     */
    size_t read(char *buffer, size_t size, sockaddr_in* from = nullptr) override;

    /**
     * @brief Reads data from the socket into memory buffer
     *
     * Buffer bytes() is set to number of bytes read
     * @param buffer            The memory buffer
     * @param size              Number of bytes to read from socket
     * @param from              An optional structure for source address
     * @returns the number of bytes read from the socket
     */
    size_t read(Buffer& buffer, size_t size, sockaddr_in* from = nullptr) override;

    /**
     * @brief Reads data from the socket into memory buffer
     *
     * Buffer bytes() is set to number of bytes read
     * @param buffer            The memory buffer
     * @param size              Number of bytes to read from socket
     * @param from              An optional structure for source address
     * @returns the number of bytes read from the socket
     */
    size_t read(String& buffer, size_t size, sockaddr_in* from = nullptr) override;
};

/**
 * @}
 */
}
#endif
