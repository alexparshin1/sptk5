/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       BaseSocket.h - description                             ║
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

#ifndef __BASESOCKET_H__
#define __BASESOCKET_H__

#ifndef _WIN32
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <sys/time.h>
    #include <sys/ioctl.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>

    /**
     * A socket handle is an integer
     */
    typedef int SOCKET;
    typedef sa_family_t SOCKET_ADDRESS_FAMILY;

    #ifdef __APPLE__
        typedef int socklen_t;
    #endif

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

#include <sptk5/Exception.h>
#include <sptk5/Strings.h>
#include <sptk5/Buffer.h>

namespace sptk
{

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief Generic socket.
 *
 * Allows to establish a network connection
 * to the host by name and port address
 */
class SP_EXPORT BaseSocket
{
protected:
    /**
     * Socket internal (OS) handle
     */
    SOCKET      m_sockfd;

    /**
     * Socket domain type
     */
    int32_t     m_domain;

    /**
     * Socket type
     */
    int32_t     m_type;

    /**
     * Socket protocol
     */
    int32_t     m_protocol;

    /**
     * Host name
     */
    std::string m_host;

    /**
     * Port number
     */
    uint32_t    m_port;

protected:

#ifdef _WIN32
    /**
     * WinSock initialization
     */
    static void init();

    /**
     * WinSock cleanup
     */
    static void cleanup();

#endif

public:
    /**
     * @brief A mode to open a socket, one of
     */
    enum CSocketOpenMode
    {
        /**
         * Only create (Typical UDP connectionless socket)
         */
        SOM_CREATE,

        /**
         * Connect
         */
        SOM_CONNECT,

        /**
         * Bind (listen)
         */
        SOM_BIND

    };

    /**
     * @brief Throws socket exception with error description retrieved from socket state
     * @param message std::string, error message
     * @param file const char*, source file name
     * @param line int, source file line number
     */
    static void throwSocketError(std::string message, const char* file, int line) THROWS_EXCEPTIONS;

    /**
     * @brief Opens the socket connection by address.
     * @param openMode CSocketOpenMode, SOM_CREATE for UDP socket, SOM_BIND for the server socket, and SOM_CONNECT for the client socket
     * @param addr const sockaddr_in*, defines socket address/port information
     * @param timeoutMS uint32_t, Connection timeout, milliseconds. The default is 0 (wait forever)
     */
    void open_addr(CSocketOpenMode openMode = SOM_CREATE, const sockaddr_in* addr = 0L, uint32_t timeoutMS=0);

public:
    /**
     * @brief Constructor
     * @param domain int32_t, socket domain type
     * @param type int32_t, socket type
     * @param protocol int32_t, protocol type
     */
    BaseSocket(SOCKET_ADDRESS_FAMILY domain = AF_INET, int32_t type = SOCK_STREAM, int32_t protocol = 0);

    /**
     * @brief Destructor
     */
    virtual ~BaseSocket();

    /**
     * @brief Set blocking mode
     * @param blocking bool, socket blocking mode flag
     */
    void blockingMode(bool blocking) THROWS_EXCEPTIONS;

    /**
     * @brief Returns number of bytes available in socket
     */
    virtual uint32_t socketBytes();

    /**
     * @brief Returns socket handle
     */
    int handle() const
    {
        return (int) m_sockfd;
    }

    /**
     * @brief Attaches socket handle
     * @param socketHandle SOCKET, existing socket handle
     */
    virtual void attach(SOCKET socketHandle);

    /**
     * @brief Sets the host name
     * @param hostName std::string, the host name
     */
    void host(std::string hostName);

    /**
     * @brief Returns the host name
     */
    std::string host() const
    {
        return m_host;
    }

    /**
     * @brief Sets the port number
     * @param portNumber int32_t, the port number
     */
    void port(int32_t portNumber);

    /**
     * @brief Returns the current port number
     * @returns port number
     */
    int32_t port() const
    {
        return (int32_t) m_port;
    }

    /**
     * @brief Opens the client socket connection by host and port
     * @param hostName std::string, the host name
     * @param port uint32_t, the port number
     * @param openMode CSocketOpenMode, socket open mode
     * @param blockingMode bool, socket blocking (true) on non-blocking (false) mode
     * @param timeoutMS uint32_t, Connection timeout, milliseconds. The default is 0 (wait forever)
     */
    virtual void open(std::string hostName = "", uint32_t port = 0, CSocketOpenMode openMode = SOM_CONNECT, bool blockingMode = true, int timeoutMS=0) THROWS_EXCEPTIONS
    {}

    /**
     * @brief Opens the server socket connection on port (binds/listens)
     * @param portNumber uint32_t, the port number
     */
    void listen(uint32_t portNumber = 0);

    /**
     * @brief In server mode, waits for the incoming connection.
     *
     * When incoming connection is made, exits returning the connection info
     * @param clientSocketFD int&, connected client socket FD
     * @param clientInfo sockaddr_in&, connected client info
     */
    void accept(SOCKET& clientSocketFD, struct sockaddr_in& clientInfo);

    /**
     * @brief Closes the socket connection
     */
    virtual void close();

    /**
     * @brief Returns the current socket state
     * @returns true if socket is opened
     */
    bool active() const
    {
        return m_sockfd != INVALID_SOCKET;
    }

    /**
     * @brief Calls Unix fcntl() or Windows ioctlsocket()
     */
    int32_t control(int flag, uint32_t *check);

    /**
     * @brief Sets socket option value
     * Throws an error if not succeeded
     */
    void setOption(int level, int option, int value) THROWS_EXCEPTIONS;

    /**
     * @brief Gets socket option value
     *
     * Throws an error if not succeeded
     */
    void getOption(int level, int option, int& value) THROWS_EXCEPTIONS;

    /**
     * @brief Reads data from the socket in regular or TLS mode
     * @param buffer void *, the destination buffer
     * @param size size_t, the destination buffer size
     * @returns the number of bytes read from the socket
     */
    virtual size_t recv(void* buffer, size_t size);

    /**
     * @brief Reads data from the socket in regular or TLS mode
     * @param buffer const void *, the send buffer
     * @param size size_t, the send data length
     * @returns the number of bytes sent the socket
     */
    virtual size_t send(const void* buffer, size_t size);

    /**
     * @brief Reads data from the socket
     * @param buffer char *, the memory buffer
     * @param size size_t, the number of bytes to read
     * @param from sockaddr_in*, an optional structure for source address
     * @returns the number of bytes read from the socket
     */
    virtual size_t read(char *buffer, size_t size, sockaddr_in* from = NULL) THROWS_EXCEPTIONS;

    /**
     * @brief Reads data from the socket into memory buffer
     *
     * Buffer bytes() is set to number of bytes read
     * @param buffer CBuffer&, the memory buffer
     * @param size size_t, the number of bytes to read
     * @param from sockaddr_in*, an optional structure for source address
     * @returns the number of bytes read from the socket
     */
    virtual size_t read(Buffer& buffer, size_t size, sockaddr_in* from = NULL) THROWS_EXCEPTIONS;

    /**
     * @brief Reads data from the socket into memory buffer
     *
     * Buffer bytes() is set to number of bytes read
     * @param buffer std::string&, the memory buffer
     * @param size size_t, the number of bytes to read
     * @param from sockaddr_in*, an optional structure for source address
     * @returns the number of bytes read from the socket
     */
    virtual size_t read(std::string& buffer, size_t size, sockaddr_in* from = NULL) THROWS_EXCEPTIONS;

    /**
     * @brief Writes data to the socket
     *
     * If size is omited then buffer is treated as zero-terminated string
     * @param buffer const char *, the memory buffer
     * @param size uint32_t, the memory buffer size
     * @param peer const sockaddr_in*, optional peer information
     * @returns the number of bytes written to the socket
     */
    virtual size_t write(const char *buffer, size_t size = size_t(-1), const sockaddr_in* peer = NULL) THROWS_EXCEPTIONS;

    /**
     * @brief Writes data to the socket
     * @param buffer const CBuffer&, the memory buffer
     * @param peer const sockaddr_in*, optional peer information
     * @returns the number of bytes written to the socket
     */
    virtual size_t write(const Buffer& buffer, const sockaddr_in* peer = NULL) THROWS_EXCEPTIONS;

    /**
     * @brief Writes data to the socket
     * @param buffer const std::string&, the memory buffer
     * @param peer const sockaddr_in*, optional peer information
     * @returns the number of bytes written to the socket
     */
    virtual size_t write(const std::string& buffer, const sockaddr_in* peer = NULL) THROWS_EXCEPTIONS;

    /**
     * @brief Reports true if socket is ready for reading from it
     * @param timeoutMS uint32_t, read timeout in msec
     */
    bool readyToRead(uint32_t timeoutMS);

    /**
     * @brief Reports true if socket is ready for writing to it
     * @param timeoutMS uint32_t, read timeout in msec
     */
    bool readyToWrite(uint32_t timeoutMS);

    /**
     * @brief Get address data from hostname
     * @param hostname std::string&, Host name or address
     * @param address sockaddr_in&, Output address data
     * @param socktype int, SOCK_STREAM, SOCK_DGRAM, etc
     */
    static void getHostAddress(std::string& hostname, sockaddr_in& address, int socktype=SOCK_STREAM);
};

#define THROW_SOCKET_ERROR(msg) BaseSocket::throwSocketError(msg,__FILE__,__LINE__)

/**
 * @}
 */
}
#endif
