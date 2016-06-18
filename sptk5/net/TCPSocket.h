/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       TCPSocket.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>

    /// A socket handle is an integer
    typedef int SOCKET;
    typedef sa_family_t SOCKET_ADDRESS_FAMILY;

    /// A value to indicate an invalid handle
    #define INVALID_SOCKET -1

#else
    #include <winsock2.h>
    #include <windows.h>
    typedef int socklen_t;
    typedef unsigned short SOCKET_ADDRESS_FAMILY;
#endif

#include <sptk5/Exception.h>
#include <sptk5/Strings.h>
#include <sptk5/Buffer.h>
#include <sptk5/net/BaseSocket.h>

namespace sptk
{

/// @addtogroup utility Utility Classes
/// @{

/// Buffered Socket reader.
class SP_EXPORT TCPSocketReader: protected Buffer
{
    BaseSocket&    m_socket;       ///< Socket to read from
    uint32_t        m_readOffset;   ///< Current offset in the read buffer

    /// @brief Performs buffered read
    ///
    /// Data is read from the opened socket into a character buffer of limited size
    /// @param dest char *, destination buffer
    /// @param sz size_t, size of the destination buffer
    /// @param delimiter char, line delimiter
    /// @param readLine bool, true if we want to read one line (ended with CRLF) only
    /// @param from sockaddr_in*, an optional structure for source address
    /// @returns number of bytes read
    int32_t bufferedRead(char *dest, size_t sz, char delimiter, bool readLine, struct sockaddr_in* from = NULL);

public:

    /// @brief Constructor
    /// @param socket CBaseSocket&, socket to work with
    /// @param bufferSize size_t, the desirable size of the internal buffer
    TCPSocketReader(BaseSocket& socket, size_t bufferSize = 65536);

    /// @brief Connects the reader to the socket handle
    void open();

    /// @brief Performs the buffered read
    /// @param dest char *, destination buffer
    /// @param sz size_t, size of the destination buffer
    /// @param delimiter char, line delimiter
    /// @param readLine bool, true if we want to read one line (ended with CRLF) only
    /// @param from sockaddr_in*, an optional structure for source address
    /// @returns bytes read from the internal buffer
    size_t read(char *dest, size_t sz, char delimiter, bool readLine, struct sockaddr_in* from = NULL);

    /// @brief Performs the buffered read of LF-terminated string
    /// @param dest CBuffer&, destination buffer
    /// @param delimiter char, line delimiter
    /// @returns bytes read from the internal buffer
    size_t readLine(Buffer& dest, char delimiter);
};

/// @brief Generic TCP socket.
///
/// Allows to establish a network connection
/// to the host by name and port address
class SP_EXPORT TCPSocket: public BaseSocket
{
protected:
    TCPSocketReader     m_reader;       ///< Socket buffered reader
    Buffer             m_stringBuffer; ///< Buffer to read a line
protected:

    /// @brief Reads a single char from the socket
    char getChar();
    
    /// @brief Get address data from hostname
    /// @param hostname std::string&, Host name or address
    /// @param address sockaddr_in&, Output address data
    void getHostAddress(std::string& hostname, sockaddr_in& address);

public:
    /// @brief Constructor
    /// @param domain int32_t, socket domain type
    /// @param type int32_t, socket type
    /// @param protocol int32_t, protocol type
    TCPSocket(SOCKET_ADDRESS_FAMILY domain = AF_INET, int32_t type = SOCK_STREAM, int32_t protocol = 0);

    /// @brief Destructor
    virtual ~TCPSocket();

    /// @brief Opens the client socket connection by host and port
    /// @param hostName std::string, the host name
    /// @param port uint32_t, the port number
    /// @param openMode CSocketOpenMode, socket open mode
    virtual void open(std::string hostName = "", uint32_t port = 0, CSocketOpenMode openMode = SOM_CONNECT) THROWS_EXCEPTIONS;

    /// @brief In server mode, waits for the incoming connection.
    ///
    /// When incoming connection is made, exits returning the connection info
    /// @param clientSocketFD int&, connected client socket FD
    /// @param clientInfo sockaddr_in&, connected client info
    void accept(SOCKET& clientSocketFD, struct sockaddr_in& clientInfo);

    /// @brief Reads one line from the socket into existing memory buffer
    ///
    /// The output string should fit the buffer or it will be returned incomplete.
    /// @param buffer char *, the destination buffer
    /// @param size size_t, the destination buffer size
    /// @param delimiter char, line delimiter
    /// @returns the number of bytes read from the socket
    size_t readLine(char *buffer, size_t size, char delimiter='\n');

    /// @brief Reads one line (terminated with CRLF) from the socket into existing memory buffer
    ///
    /// The memory buffer is extended automatically to fit the string.
    /// @param buffer CBuffer&, the destination buffer
    /// @param delimiter char, line delimiter
    /// @returns the number of bytes read from the socket
    size_t readLine(Buffer& buffer, char delimiter='\n');

    /// @brief Reads one line (terminated with CRLF) from the socket into string
    /// @param s std::string&, the destination string
    /// @param delimiter char, line delimiter
    /// @returns the number of bytes read from the socket
    size_t readLine(std::string& s, char delimiter='\n');

    /// @brief Reads data from the socket
    /// @param buffer char *, the memory buffer
    /// @param size size_t, the number of bytes to read
    /// @param from sockaddr_in*, an optional structure for source address
    /// @returns the number of bytes read from the socket
    virtual size_t read(char *buffer, size_t size, sockaddr_in* from = NULL) THROWS_EXCEPTIONS;

    /// @brief Reads data from the socket into memory buffer
    ///
    /// Buffer bytes() is set to number of bytes read
    /// @param buffer CBuffer&, the memory buffer
    /// @param size size_t, number of bytes to read from socket
    /// @param from sockaddr_in*, an optional structure for source address
    /// @returns the number of bytes read from the socket
    size_t read(Buffer& buffer, size_t size, sockaddr_in* from = NULL) THROWS_EXCEPTIONS;

    /// @brief Reads data from the socket into memory buffer
    ///
    /// Buffer bytes() is set to number of bytes read
    /// @param buffer std::string&, the memory buffer
    /// @param size size_t, number of bytes to read from socket
    /// @param from sockaddr_in*, an optional structure for source address
    /// @returns the number of bytes read from the socket
    size_t read(std::string& buffer, size_t size, sockaddr_in* from = NULL) THROWS_EXCEPTIONS;
};

/// @}
}
#endif
