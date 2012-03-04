/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CSocket.cpp  -  description
                             -------------------
    begin                : July 10 2002
    copyright            : (C) 2002-2012 by Alexey Parshin. All rights reserved.
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

    /// A value to indicate an invalid handle
    #define INVALID_SOCKET -1

#else
    #include <winsock2.h>
    #include <windows.h>
#endif

#include <sptk5/CException.h>
#include <sptk5/CStrings.h>
#include <sptk5/CBuffer.h>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

class CSocket;

/// Buffered Socket reader.
class CSocketReader : protected CBuffer {
    CSocket&  m_socket;       ///< Socket to read from
    uint32_t  m_readOffset;   ///< Current offset in the read buffer

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
    /// @param socket CSocket&, socket to work with
    /// @param bufferSize int, the desirable size of the internal buffer
    CSocketReader(CSocket& socket,int bufferSize=4096);

    /// @brief Connects the reader to the socket handle
    void open();

    /// @brief Disconnects the reader from the socket handle
    void close() {
    }

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
class CSocket {
protected:
    SOCKET              m_sockfd;       ///< Socket internal (OS) handle
    int32_t             m_domain;       ///< Socket domain type
    int32_t             m_type;         ///< Socket type
    int32_t             m_protocol;     ///< Socket protocol
    std::string         m_host;         ///< Host name
    int32_t             m_port;         ///< Port number
    fd_set              m_inputs;       ///< The set of socket descriptors for reading
    fd_set              m_outputs;      ///< The set of socket descriptors for writing
    CSocketReader       m_reader;       ///< Socket buffered reader
    CBuffer             m_stringBuffer; ///< Buffer to read a line
#ifdef HAVE_GNUTLS
    bool                m_tlsMode;      ///< True, is socket switched to TLS mode (using GNU TLS)
    gnutls_session_t    m_tlsSession;   ///< GNU TLS session
    gnutls_anon_client_credentials_t
                        m_tlsAnonCred;  ///< GNU TLS anonymous credentials

    static bool         m_tlsInitted;   ///< Was GNU TLS initted?
#endif
protected:

#ifdef _WIN32
    static int32_t      m_socketCount;  ///< Total number of the objects of this class
    static bool         m_inited;       ///< Were the sockets inited?
    static void         init();         ///< WinSock initialization
    static void         cleanup();      ///< WinSock cleanup
#endif

    /// @brief Reads a single char from the socket
    char getChar();

public:
    /// @brief A mode to open a socket, one of
    enum CSocketOpenMode {
        SOM_CREATE,     ///< Only create (Typical UDP connectionless socket)
        SOM_CONNECT,    ///< Connect
        SOM_BIND        ///< Bind (listen)
    };

public:
    /// @brief Constructor
    /// @param domain int32_t, socket domain type
    /// @param type int32_t, socket type
    /// @param protocol int32_t, protocol type
    CSocket(int32_t domain=AF_INET, int32_t type=SOCK_STREAM, int32_t protocol=0);

    /// @brief Destructor
    virtual ~CSocket();

    /// @brief Returns socket handle
    int  handle() const {
        return (int) m_sockfd;
    }

    /// @brief Attaches socket handle
    /// @param socketHandle SOCKET, existing socket handle
    void attach(SOCKET socketHandle);

    /// @brief Sets the host name
    /// @param hostName std::string, the host name
    void host(std::string hostName);

    /// @brief Returns the host name
    std::string host() const {
        return m_host;
    }

    /// @brief Sets the port number
    /// @param portNumber int32_t, the port number
    void port(int32_t portNumber);

    /// @brief Returns the current port number
    /// @returns port number
    int32_t port() const {
        return m_port;
    }

    /// @brief Opens the socket connection by address.
    /// @param openMode CSocketOpenMode, SOM_CREATE for UDP socket, SOM_BIND for the server socket, and SOM_CONNECT for the client socket
    /// @param addr sockaddr_in*, defines socket address/port information
    void open_addr(CSocketOpenMode openMode=SOM_CREATE,sockaddr_in* addr=0L);

    /// @brief Opens the client socket connection by host and port
    /// @param hostName std::string, the host name
    /// @param port int32_t, the port number
    /// @param openMode CSocketOpenMode, socket open mode
    virtual void open(std::string hostName="",int32_t port=0,CSocketOpenMode openMode=SOM_CONNECT);

    /// @brief Opens the server socket connection on port (binds/listens)
    /// @param portNumber int32_t, the port number
    void listen(int32_t portNumber=0);

    /// @brief In server mode, waits for the incoming connection.
    ///
    /// When incoming connection is made, exits returning the connection info
    /// @param clientSocketFD int&, connected client socket FD
    /// @param clientInfo sockaddr_in&, connected client info
    void accept(int& clientSocketFD,struct sockaddr_in& clientInfo);

    /// @brief Closes the socket connection
    virtual void close();

    /// @brief Returns the current socket state
    /// @returns true if socket is opened
    bool active() const {
        return m_sockfd != INVALID_SOCKET;
    }

    /// @brief Calls Unix fcntl() or Windows ioctlsocket()
    int32_t  control(int flag, uint32_t *check);

    /// @brief Sets socket option value
    /// Throws an error if not succeeded
    void setOption(int level,int option,int value) throw(sptk::CException);

    /// @brief Gets socket option value
    ///
    /// Throws an error if not succeeded
    void getOption(int level,int option,int& value) throw(sptk::CException);

    /// @brief Reads data from the socket in regular or TLS mode
    /// @param buffer void *, the destination buffer
    /// @param size uint32_t, the destination buffer size
    /// @returns the number of bytes read from the socket
    ssize_t recv(void* buffer,uint32_t size);

    /// @brief Reads data from the socket in regular or TLS mode
    /// @param buffer const void *, the send buffer
    /// @param size uint32_t, the send data length
    /// @returns the number of bytes sent the socket
    ssize_t send(const void* buffer,uint32_t len);

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
    /// @param size uint32_t, the memory buffer size
    /// @param from sockaddr_in*, an optional structure for source address
    /// @returns the number of bytes read from the socket
    uint32_t read(char *buffer,uint32_t size,sockaddr_in* from=NULL);

    /// @brief Reads data from the socket into memory buffer
    /// @param buffer const CBuffer&, the memory buffer
    /// @param from sockaddr_in*, an optional structure for source address
    /// @returns the number of bytes read from the socket
    uint32_t read(CBuffer& buffer,sockaddr_in* from=NULL);

    /// @brief Writes data to the socket
    /// @param buffer const char *, the memory buffer
    /// @param size uint32_t, the memory buffer size
    /// @param peer const sockaddr_in*, optional peer information
    /// @returns the number of bytes written to the socket
    uint32_t write(const char *buffer,uint32_t size,const sockaddr_in* peer=0);

    /// @brief Writes data to the socket
    /// @param buffer const CBuffer&, the memory buffer
    /// @returns the number of bytes written to the socket
    uint32_t write(const CBuffer& buffer);

    /// @brief Reports true if socket is ready for reading from it
    /// @param waitmsec uint32_t, read timeout in msec
    bool readyToRead(uint32_t waitmsec);

    /// @brief Reports true if socket is ready for writing to it
    bool readyToWrite();

    /// @brief Stream std::string input
    CSocket& operator << ( const std::string& );

    /// @brief Stream std::string output
    CSocket& operator >> ( std::string& );
#ifdef HAVE_GNUTLS
    /// @brief Global TLS initialization
    ///
    /// Should executed from the main thread and only once, otherwise TLS wouldn't work.
    static void globalInitTLS();

    /// @brief Returns TLS mode
    ///
    /// In TLS mode, socket uses gnutls_record_recv() and gnutls_record_send() instead of
    /// recv() and send().
    bool tlsMode() const { return m_tlsMode; }

    /// @brief Inits TLS mode for the client socket
    ///
    /// Initializes session in default anonymous mode, and runs TLS handshake
    /// In TLS mode, socket uses gnutls_record_recv() and gnutls_record_send() instead of
    /// recv() and send().
    void startTLS();

    /// @brief Switches socket into regular mode
    void endTLS();
#endif
};

/// @}
}
#endif
