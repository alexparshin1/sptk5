/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SSLSocket.h - description                              ║
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

#ifndef __SSLSOCKET_H__
#define __SSLSOCKET_H__

#include <sptk5/sptk.h>
#include <sptk5/net/SSLContext.h>
#include <sptk5/net/TCPSocket.h>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief Encrypted TCP Socket
 */
class SSLSocket: public TCPSocket, public Synchronized
{
    /**
     * SSL socket
     */
    SSL*        m_ssl;


public:
    /**
     * @brief Returns number of bytes available for read
     */
    virtual uint32_t socketBytes(); 

    /**
     * @brief Throws SSL error based on SSL function return code
     * @param rc int, SSL function return code
     */
    void throwSSLError(int rc);

protected:

    /**
     * @brief Reads data from SSL socket
     * @param buffer void *, destination buffer
     * @param size size_t, destination buffer size
     * @return the number of bytes read from the socket
     */
    virtual size_t recv(void* buffer, size_t size) throw (std::exception);

    /**
     * @brief Sends data through SSL socket
     * @param buffer const void *, the send buffer
     * @param len uint32_t, the send data length
     * @return the number of bytes sent the socket
     */
    virtual size_t send(const void* buffer, size_t len) throw (std::exception);

    /**
     * @brief Get error description for SSL error code
     * @param function std::string, SSL function
     * @param SSLError int32_t, error code returned by SSL_get_error() result
     * @return Error description
     */
    virtual std::string getSSLError(std::string function, int32_t SSLError) const;

    /**
     * @brief Switch already open socket to SSL
     * @param blockingMode bool, socket blocking mode
     */
    void startSSL(bool blockingMode) THROWS_EXCEPTIONS;
    
public:

    /**
     * @brief Constructor
     * @param sslContext CSSLContext&, SSL context that is used as SSL connection template
     */
    SSLSocket(SSLContext& sslContext);

    /**
     * @brief Destructor
     */
    virtual ~SSLSocket();

    /**
     * @brief opens the socket connection by host and port
     *
     * Initializes SSL first, if host name is empty or port is 0 then the current host and port values are used.
     * They could be defined by previous calls of  open(), port(), or host() methods.
     * @param hostName std::string, the host name
     * @param port uint32_t, the port number
     * @param openMode CSocketOpenMode, socket open mode
     * @param blockingMode bool, socket blocking (true) on non-blocking (false) mode
     * @param timeoutMS uint32_t, Connection timeout, milliseconds. The default is 0 (wait forever)
     */
    virtual void open(std::string hostName = "", uint32_t port = 0, CSocketOpenMode openMode = SOM_CONNECT, bool blockingMode = true, uint32_t timeoutMS=0) THROWS_EXCEPTIONS;

    /**
     * @brief Opens the client socket connection by host address
     * @param addr const struct sockaddr_in&, the host address
     * @param openMode CSocketOpenMode, socket open mode
     * @param blockingMode bool, socket blocking (true) on non-blocking (false) mode
     * @param timeoutMS uint32_t, connection timeout, milliseconds. The default is 0 (wait forever)
     */
    virtual void open(const struct sockaddr_in& addr, CSocketOpenMode openMode = SOM_CONNECT, bool blockingMode = true, uint32_t timeoutMS=0) THROWS_EXCEPTIONS;

    /**
     * @brief Attaches socket handle
     *
     * This method is designed to only attach socket handles
     * obtained with accept().
     * @param socketHandle SOCKET, existing socket handle
     */
    virtual void attach(SOCKET socketHandle) throw (std::exception);

    /**
     * @brief Closes the socket connection
     *
     * This method is not thread-safe.
     */
    virtual void close();

    /**
     * @brief Returns SSL handle
     */
    SSL* handle()
    {
        return m_ssl;
    }
};

/**
 * @}
 */
}

#endif
