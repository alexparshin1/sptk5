/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       BaseSocket.h - description                             ║
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
	#include <ws2tcpip.h>
	#include <windows.h>
    typedef int socklen_t;
    typedef unsigned short SOCKET_ADDRESS_FAMILY;
#endif

#include <chrono>
#include <sptk5/DateTime.h>
#include <sptk5/Exception.h>
#include <sptk5/net/Host.h>
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
 * Allows establishing a network connection
 * to the host by name and port address
 */
class SP_EXPORT BaseSocket
{
public:
    /**
    * @brief A mode to open a socket, one of
    */
    enum CSocketOpenMode : uint8_t
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
     * Host
     */
    Host        m_host;

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

    /**
     * @brief Opens the client socket connection by host and port
     * @param host              The host
     * @param openMode          Socket open mode
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode
     * @param timeoutMS         Connection timeout. The default is 0 (wait forever)
     */
    virtual void _open(const Host& host, CSocketOpenMode openMode, bool blockingMode, std::chrono::milliseconds timeoutMS);

    /**
     * @brief Opens the client socket connection by host and port
     * @param address           Address and port
     * @param openMode          Socket open mode
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode
     * @param timeoutMS         Connection timeout, std::chrono::milliseconds. The default is 0 (wait forever)
     */
    virtual void _open(const struct sockaddr_in& address, CSocketOpenMode openMode, bool blockingMode, std::chrono::milliseconds timeoutMS)
    {}

public:

    /**
    * @brief Throws socket exception with error description retrieved from socket state
    * @param message           Error message
    * @param file              Source file name
    * @param line              Source file line number
    */
    static void throwSocketError(const String& message, const char* file, int line);

    /**
    * @brief Opens the socket connection by address.
    * @param openMode          SOM_CREATE for UDP socket, SOM_BIND for the server socket, and SOM_CONNECT for the client socket
    * @param addr              Defines socket address/port information
    * @param timeout           Connection timeout. If 0 the wait forever;
    */
    void open_addr(CSocketOpenMode openMode = SOM_CREATE, const sockaddr_in* addr = nullptr, std::chrono::milliseconds timeout = std::chrono::milliseconds(0));

public:
    /**
     * @brief Constructor
     * @param domain            Socket domain type
     * @param type              Socket type
     * @param protocol          Protocol type
     */
    BaseSocket(SOCKET_ADDRESS_FAMILY domain = AF_INET, int32_t type = SOCK_STREAM, int32_t protocol = 0);

    /**
     * @brief Destructor
     */
    virtual ~BaseSocket();

    /**
     * @brief Set blocking mode
     * @param blocking          Socket blocking mode flag
     */
    void blockingMode(bool blocking);

    /**
     * @brief Returns number of bytes available in socket
     */
    virtual size_t socketBytes();

    /**
     * @brief Returns socket handle
     */
    int handle() const
    {
        return (int) m_sockfd;
    }

    /**
     * @brief Attaches socket handle
     * @param socketHandle      Existing socket handle
     */
    virtual void attach(SOCKET socketHandle);

    /**
     * @brief Sets the host name
     * @param host              The host
     */
    void host(const Host& host);

    /**
     * @brief Returns the host
     */
    const Host& host() const
    {
        return m_host;
    }

    /**
     * @brief Opens the client socket connection by host and port
     * @param host              The host
     * @param openMode          Socket open mode
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode
     * @param timeoutMS         Connection timeout. The default is 0 (wait forever)
     */
    void open(const Host& host = Host(), CSocketOpenMode openMode = SOM_CONNECT, bool blockingMode = true,
              std::chrono::milliseconds timeoutMS = std::chrono::milliseconds(0))
    {
        _open(host, openMode, blockingMode, timeoutMS);
    }

    /**
     * @brief Opens the client socket connection by host and port
     * @param address           Address and port
     * @param openMode          Socket open mode
     * @param blockingMode      Socket blocking (true) on non-blocking (false) mode
     * @param timeoutMS         Connection timeout, std::chrono::milliseconds. The default is 0 (wait forever)
     */
    void open(const struct sockaddr_in& address, CSocketOpenMode openMode = SOM_CONNECT,
              bool blockingMode = true, std::chrono::milliseconds timeoutMS = std::chrono::milliseconds(0))
    {
        _open(address, openMode, blockingMode, timeoutMS);
    }

    /**
     * @brief Binds the socket to port
     * @param address           Local IP address, or NULL if any
     * @param portNumber        The port number, or 0 if any
     */
    void bind(const char* address, uint32_t portNumber);

    /**
     * @brief Opens the server socket connection on port (binds/listens)
     * @param portNumber        The port number
     */
    void listen(uint16_t portNumber = 0);

    /**
     * @brief In server mode, waits for the incoming connection.
     *
     * When incoming connection is made, exits returning the connection info
     * @param clientSocketFD    Connected client socket FD
     * @param clientInfo        Connected client info
     */
    virtual void accept(SOCKET& clientSocketFD, struct sockaddr_in& clientInfo) {}

    /**
     * @brief Closes the socket connection
     */
    virtual void close() noexcept;

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
    int32_t control(int flag, const uint32_t* check);

    /**
     * @brief Sets socket option value
     * Throws an error if not succeeded
     */
    void setOption(int level, int option, int value);

    /**
     * @brief Gets socket option value
     *
     * Throws an error if not succeeded
     */
    void getOption(int level, int option, int& value);

    /**
     * @brief Reads data from the socket in regular or TLS mode
     * @param buffer            The destination buffer
     * @param size              The destination buffer size
     * @returns the number of bytes read from the socket
     */
    virtual size_t recv(void* buffer, size_t size);

    /**
     * @brief Reads data from the socket in regular or TLS mode
     * @param buffer            The send buffer
     * @param size              The send data length
     * @returns the number of bytes sent the socket
     */
    virtual size_t send(const void* buffer, size_t size);

    /**
     * @brief Reads data from the socket
     * @param buffer            The memory buffer
     * @param size              The number of bytes to read
     * @param from              Optional structure for source address
     * @returns the number of bytes read from the socket
     */
    virtual size_t read(char *buffer, size_t size, sockaddr_in* from = nullptr);

    /**
     * @brief Reads data from the socket into memory buffer
     *
     * Buffer bytes() is set to number of bytes read
     * @param buffer            The output buffer
     * @param size              The number of bytes to read
     * @param from              An optional structure for source address
     * @returns the number of bytes read from the socket
     */
    virtual size_t read(Buffer& buffer, size_t size, sockaddr_in* from = nullptr);

    /**
     * @brief Reads data from the socket into memory buffer
     *
     * Buffer bytes() is set to number of bytes read
     * @param buffer            The memory buffer
     * @param size              The number of bytes to read
     * @param from              Optional structure for source address
     * @returns the number of bytes read from the socket
     */
    virtual size_t read(String& buffer, size_t size, sockaddr_in* from = nullptr);

    /**
     * @brief Writes data to the socket
     *
     * If size is omited then buffer is treated as zero-terminated string
     * @param buffer            The memory buffer
     * @param size              The memory buffer size
     * @param peer              Optional peer information
     * @returns the number of bytes written to the socket
     */
    virtual size_t write(const char *buffer, size_t size = size_t(-1), const sockaddr_in* peer = nullptr);

    /**
     * @brief Writes data to the socket
     * @param buffer            The memory buffer
     * @param peer              Optional peer information
     * @returns the number of bytes written to the socket
     */
    virtual size_t write(const Buffer& buffer, const sockaddr_in* peer = nullptr);

    /**
     * @brief Writes data to the socket
     * @param buffer            The memory buffer
     * @param peer              Optional peer information
     * @returns the number of bytes written to the socket
     */
    virtual size_t write(const String& buffer, const sockaddr_in* peer = nullptr);

    /**
     * @brief Reports true if socket is ready for reading from it
     * @param timeout           Read timeout
     */
    virtual bool readyToRead(std::chrono::milliseconds timeout);

    /**
     * @brief Reports true if socket is ready for reading from it
     * @param timeout           Read timeout date and time
     */
    virtual bool readyToRead(DateTime timeout);

    /**
     * @brief Reports true if socket is ready for writing to it
     * @param timeout           Write timeout
     */
    virtual bool readyToWrite(std::chrono::milliseconds timeout);

    /**
     * @brief Reports true if socket is ready for writing to it
     * @param timeout           Write timeout
     */
    virtual bool readyToWrite(DateTime timeout);
};

#define THROW_SOCKET_ERROR(msg) BaseSocket::throwSocketError(msg,__FILE__,__LINE__)

/**
 * @}
 */
}
#endif
