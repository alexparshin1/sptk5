/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CTCPSocket.cpp  -  description
                             -------------------
    begin                : July 10 2002
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
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

#ifndef __CSOCKET_H__
#define __CSOCKET_H__

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

#include <sptk5/CException.h>
#include <sptk5/CStrings.h>
#include <sptk5/CBuffer.h>
#include <sptk5/net/CBaseSocket.h>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// Buffered Socket reader.
class SP_EXPORT CTCPSocketReader : protected CBuffer {
    CBaseSocket&    m_socket;       ///< Socket to read from
    uint32_t        m_readOffset;   ///< Current offset in the read buffer

    /// @brief Performs buffered read
    ///
    /// Data is read from the opened socket into a character buffer of limited size
    /// @param dest char *, destination buffer
    /// @param sz uint32_t, size of the destination buffer
    /// @param readLine bool, true if we want to read one line (ended with CRLF) only
    /// @param from sockaddr_in*, an optional structure for source address
    /// @returns number of bytes read
    int32_t bufferedRead(char *dest,uint32_t sz,bool readLine,struct sockaddr_in* from=NULL);

public:

    /// @brief Constructor
    /// @param socket CBaseSocket&, socket to work with
    /// @param bufferSize int, the desirable size of the internal buffer
    CTCPSocketReader(CBaseSocket& socket,int bufferSize=4096);

    /// @brief Connects the reader to the socket handle
    void open();

    /// @brief Performs the buffered read
    /// @param dest char *, destination buffer
    /// @param sz uint32_t, size of the destination buffer
    /// @param readLine bool, true if we want to read one line (ended with CRLF) only
    /// @param from sockaddr_in*, an optional structure for source address
    /// @returns bytes read from the internal buffer
    uint32_t read(char *dest,uint32_t sz,bool readLine=false,struct sockaddr_in* from=NULL);

    /// @brief Performs the buffered read of '\n'-terminated string
    /// @param dest CBuffer&, destination buffer
    /// @returns bytes read from the internal buffer
    uint32_t readLine(CBuffer& dest);
};

/// @brief Generic socket.
///
/// Allows to establish a network connection
/// to the host by name and port address
class SP_EXPORT CTCPSocket : public CBaseSocket {
protected:
    CTCPSocketReader    m_reader;       ///< Socket buffered reader
    CBuffer             m_stringBuffer; ///< Buffer to read a line
protected:

    /// @brief Reads a single char from the socket
    char getChar();

public:
    /// @brief Constructor
    /// @param domain int32_t, socket domain type
    /// @param type int32_t, socket type
    /// @param protocol int32_t, protocol type
    CTCPSocket(SOCKET_ADDRESS_FAMILY domain=AF_INET, int32_t type=SOCK_STREAM, int32_t protocol=0);

    /// @brief Destructor
    virtual ~CTCPSocket();

    /// @brief Opens the client socket connection by host and port
    /// @param hostName std::string, the host name
    /// @param port uint32_t, the port number
    /// @param openMode CSocketOpenMode, socket open mode
    virtual void open(std::string hostName="",uint32_t port=0,CSocketOpenMode openMode=SOM_CONNECT);

    /// @brief Opens the server socket connection on port (binds/listens)
    /// @param portNumber uint32_t, the port number
    void listen(uint32_t portNumber=0);

    /// @brief In server mode, waits for the incoming connection.
    ///
    /// When incoming connection is made, exits returning the connection info
    /// @param clientSocketFD int&, connected client socket FD
    /// @param clientInfo sockaddr_in&, connected client info
    void accept(int& clientSocketFD,struct sockaddr_in& clientInfo);

    /// @brief Reads one line (terminated with CRLF) from the socket into existing memory buffer
    ///
    /// The output string should fit the buffer or it will be returned incomplete.
    /// @param buffer char *, the destination buffer
    /// @param size uint32_t, the destination buffer size
    /// @returns the number of bytes read from the socket
    uint32_t readLine(char *buffer,uint32_t size);

    /// @brief Reads one line (terminated with CRLF) from the socket into existing memory buffer
    ///
    /// The memory buffer is extended automatically to fit the string.
    /// @param buffer CBuffer&, the destination buffer
    /// @returns the number of bytes read from the socket
    uint32_t readLine(CBuffer& buffer);

    /// @brief Reads one line (terminated with CRLF) from the socket into string
    /// @param s std::string&, the destination string
    /// @returns the number of bytes read from the socket
    uint32_t readLine(std::string& s);

    /// @brief Reads data from the socket
    /// @param buffer char *, the memory buffer
    /// @param size uint32_t, the number of bytes to read
    /// @param from sockaddr_in*, an optional structure for source address
    /// @returns the number of bytes read from the socket
    virtual uint32_t read(char *buffer,uint32_t size,sockaddr_in* from=NULL);

    /// @brief Reads data from the socket into memory buffer
    ///
    /// Buffer bytes() is set to number of bytes read
    /// @param buffer CBuffer&, the memory buffer
    /// @param from sockaddr_in*, an optional structure for source address
    /// @returns the number of bytes read from the socket
    uint32_t read(CBuffer& buffer,uint32_t size,sockaddr_in* from=NULL);

    /// @brief Reads data from the socket into memory buffer
    ///
    /// Buffer bytes() is set to number of bytes read
    /// @param buffer std::string&, the memory buffer
    /// @param from sockaddr_in*, an optional structure for source address
    /// @returns the number of bytes read from the socket
    uint32_t read(std::string& buffer,uint32_t size,sockaddr_in* from=NULL);
};

/// @}
}
#endif
