/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          COpenSSLSocket.h  -  description
                             -------------------
    begin                : Oct 30 2014
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

#ifndef __COPENSSLSOCKET_H__
#define __COPENSSLSOCKET_H__

#include <sptk5/sptk.h>
#include <sptk5/net/COpenSSLContext.h>
#include <sptk5/net/CTCPSocket.h>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// @brief Encrypted TCP Socket
class COpenSSLSocket: public sptk::CTCPSocket, public sptk::CSynchronized
{
    SSL*        m_ssl;          ///< SSL socket
    bool        m_readInitted;  ///< Read initialized flag

    /// @brief Returns number of bytes in the SSL socket
    virtual uint32_t socketBytes();
    
    void throwOpenSSLError(int rc);

protected:

    /// @brief Closes the socket connection
    ///
    /// This method is not thread-safe.
    virtual void close();

    /// @brief Reads data from SSL socket
    /// @param buffer void *, destination buffer
    /// @param size size_t, destination buffer size
    /// @return the number of bytes read from the socket
    virtual size_t recv(void* buffer, size_t size) throw (std::exception);

    /// @brief Sends data through SSL socket
    /// @param buffer const void *, the send buffer
    /// @param len uint32_t, the send data length
    /// @return the number of bytes sent the socket
    virtual size_t send(const void* buffer, size_t len) throw (std::exception);

    /// @brief Get error description for OpenSSL error code
    /// @param function std::string, OpenSSL function
    /// @param openSSLError int32_t, error code returned by SSL_get_error() result
    /// @return Error description
    virtual std::string getOpenSSLError(std::string function, int32_t openSSLError) const;

public:

    /// @brief Constructor
    /// @param sslContext COpenSSLContext&, SSL context that is used as SSL connection template
    COpenSSLSocket(COpenSSLContext& sslContext);

    /// @brief Destructor
    virtual ~COpenSSLSocket();

    /// @brief Opens the socket connection by host and port
    ///
    /// Initializes SSL first, if host name is empty or port is 0 then the current host and port values are used.
    /// They could be defined by previous calls of  open(), port(), or host() methods.
    /// @param hostName std::string, the host name
    /// @param port uint32_t, the port number
    /// @param openMode CSocketOpenMode, socket open mode
    virtual void open(std::string hostName = "", uint32_t port = 0, CSocketOpenMode openMode = SOM_CONNECT) THROWS_EXCEPTIONS;

    /// @brief Attaches socket handle
    ///
    /// This method is designed to only attach socket handles
    /// obtained with accept().
    /// @param socketHandle SOCKET, existing socket handle
    void attach(SOCKET socketHandle) throw (std::exception);
};

/// @}
}

#endif
