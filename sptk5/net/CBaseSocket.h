/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CBaseSocket.cpp  -  description
                             -------------------
    begin                : July 10 2002
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#ifndef __CBASESOCKET_H__
#define __CBASESOCKET_H__

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

    /// A socket handle is an integer
    typedef int SOCKET;
    typedef sa_family_t SOCKET_ADDRESS_FAMILY;

    #ifdef __APPLE__
        typedef int socklen_t;
    #endif

    /// A value to indicate an invalid handle
    #define INVALID_SOCKET -1

#else
    #include <winsock2.h>
    #include <windows.h>
    typedef int socklen_t;
    typedef unsigned short SOCKET_ADDRESS_FAMILY;
#endif

#include <sptk5/CException.h>
#include <sptk5/CStrings.h>
#include <sptk5/CBuffer.h>

namespace sptk
{

/// @addtogroup utility Utility Classes
/// @{

/// @brief Generic socket.
///
/// Allows to establish a network connection
/// to the host by name and port address
class SP_EXPORT CBaseSocket
{
protected:
    SOCKET m_sockfd;        ///< Socket internal (OS) handle
    int32_t m_domain;       ///< Socket domain type
    int32_t m_type;         ///< Socket type
    int32_t m_protocol;     ///< Socket protocol
    std::string m_host;     ///< Host name
    uint32_t m_port;        ///< Port number
protected:

#ifdef _WIN32
    static void init();     ///< WinSock initialization
    static void cleanup();  ///< WinSock cleanup
#endif

public:
    /// @brief A mode to open a socket, one of
    enum CSocketOpenMode
    {
        SOM_CREATE,     ///< Only create (Typical UDP connectionless socket)
        SOM_CONNECT,    ///< Connect
        SOM_BIND        ///< Bind (listen)
    };

    /// @brief Throws socket exception with error description retrieved from socket state
    /// @param message std::string, error message
    /// @param file const char*, source file name
    /// @param line int, source file line number
    static void throwSocketError(std::string message, const char* file, int line) THROWS_EXCEPTIONS;

    /// @brief Opens the socket connection by address.
    /// @param openMode CSocketOpenMode, SOM_CREATE for UDP socket, SOM_BIND for the server socket, and SOM_CONNECT for the client socket
    /// @param addr sockaddr_in*, defines socket address/port information
    void open_addr(CSocketOpenMode openMode = SOM_CREATE, sockaddr_in* addr = 0L);

public:
    /// @brief Constructor
    /// @param domain int32_t, socket domain type
    /// @param type int32_t, socket type
    /// @param protocol int32_t, protocol type
    CBaseSocket(SOCKET_ADDRESS_FAMILY domain = AF_INET, int32_t type = SOCK_STREAM, int32_t protocol = 0);

    /// @brief Destructor
    virtual ~CBaseSocket();

    void blockingMode(bool blocking) THROWS_EXCEPTIONS;

    /// @brief Returns number of bytes available in socket
    uint32_t socketBytes();

    /// @brief Returns socket handle
    int handle() const
    {
        return (int) m_sockfd;
    }

    /// @brief Attaches socket handle
    /// @param socketHandle SOCKET, existing socket handle
    void attach(SOCKET socketHandle);

    /// @brief Sets the host name
    /// @param hostName std::string, the host name
    void host(std::string hostName);

    /// @brief Returns the host name
    std::string host() const
    {
        return m_host;
    }

    /// @brief Sets the port number
    /// @param portNumber int32_t, the port number
    void port(int32_t portNumber);

    /// @brief Returns the current port number
    /// @returns port number
    int32_t port() const
    {
        return (int32_t) m_port;
    }

    /// @brief Opens the server socket connection on port (binds/listens)
    /// @param portNumber uint32_t, the port number
    void listen(uint32_t portNumber = 0);
    
    /// @brief In server mode, waits for the incoming connection.
    ///
    /// When incoming connection is made, exits returning the connection info
    /// @param clientSocketFD int&, connected client socket FD
    /// @param clientInfo sockaddr_in&, connected client info
    void accept(SOCKET& clientSocketFD, struct sockaddr_in& clientInfo);

    /// @brief Closes the socket connection
    virtual void close();

    /// @brief Returns the current socket state
    /// @returns true if socket is opened
    bool active() const
    {
        return m_sockfd != INVALID_SOCKET;
    }

    /// @brief Calls Unix fcntl() or Windows ioctlsocket()
    int32_t control(int flag, uint32_t *check);

    /// @brief Sets socket option value
    /// Throws an error if not succeeded
    void setOption(int level, int option, int value) THROWS_EXCEPTIONS;

    /// @brief Gets socket option value
    ///
    /// Throws an error if not succeeded
    void getOption(int level, int option, int& value) THROWS_EXCEPTIONS;

    /// @brief Reads data from the socket in regular or TLS mode
    /// @param buffer void *, the destination buffer
    /// @param size size_t, the destination buffer size
    /// @returns the number of bytes read from the socket
    virtual size_t recv(void* buffer, size_t size);

    /// @brief Reads data from the socket in regular or TLS mode
    /// @param buffer const void *, the send buffer
    /// @param size size_t, the send data length
    /// @returns the number of bytes sent the socket
    virtual size_t send(const void* buffer, size_t size);

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
    /// @param size size_t, the number of bytes to read
    /// @param from sockaddr_in*, an optional structure for source address
    /// @returns the number of bytes read from the socket
    virtual size_t read(CBuffer& buffer, size_t size, sockaddr_in* from = NULL) THROWS_EXCEPTIONS;

    /// @brief Reads data from the socket into memory buffer
    ///
    /// Buffer bytes() is set to number of bytes read
    /// @param buffer std::string&, the memory buffer
    /// @param size size_t, the number of bytes to read
    /// @param from sockaddr_in*, an optional structure for source address
    /// @returns the number of bytes read from the socket
    virtual size_t read(std::string& buffer, size_t size, sockaddr_in* from = NULL) THROWS_EXCEPTIONS;

    /// @brief Writes data to the socket
    ///
    /// If size is omited then buffer is treated as zero-terminated string
    /// @param buffer const char *, the memory buffer
    /// @param size uint32_t, the memory buffer size
    /// @param peer const sockaddr_in*, optional peer information
    /// @returns the number of bytes written to the socket
    virtual size_t write(const char *buffer, size_t size = size_t(-1), const sockaddr_in* peer = NULL) THROWS_EXCEPTIONS;

    /// @brief Writes data to the socket
    /// @param buffer const CBuffer&, the memory buffer
    /// @returns the number of bytes written to the socket
    virtual size_t write(const CBuffer& buffer, const sockaddr_in* peer = NULL) THROWS_EXCEPTIONS;

    /// @brief Writes data to the socket
    /// @param buffer const std::string&, the memory buffer
    /// @returns the number of bytes written to the socket
    virtual size_t write(const std::string& buffer, const sockaddr_in* peer = NULL) THROWS_EXCEPTIONS;

    /// @brief Reports true if socket is ready for reading from it
    /// @param waitmsec size_t, read timeout in msec
    bool readyToRead(size_t waitmsec);

    /// @brief Reports true if socket is ready for writing to it
    bool readyToWrite();
};

#define THROW_SOCKET_ERROR(msg) CBaseSocket::throwSocketError(msg,__FILE__,__LINE__)

/// @}
}
#endif
