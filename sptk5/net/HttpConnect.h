/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          HttpConnect.h  -  description
                             -------------------
    begin                : July 22 2003
    copyright            : (C) 1999-2016 by Alexey Parshin. All rights reserved.
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

#ifndef __HTTPCONNECT_H__
#define __HTTPCONNECT_H__

#include <sptk5/net/TCPSocket.h>
#include <sptk5/CStrings.h>
#include <sptk5/net/HttpParams.h>

namespace sptk
{

/// @addtogroup utility Utility Classes
/// @{

/// @brief A map of HTTP headers and their values (string to string)
typedef std::map<std::string, std::string> HttpHeaders;

/// @brief HTTP socket
///
/// Implements the GET and POST methods of HTTP protocol.
/// Allows to define the host, port, submit information, and then GET or POST the HTML data to the server.
class SP_EXPORT HttpConnect
{
    CBuffer         m_readBuffer;           ///< Internal read buffer
    TCPSocket&     m_socket;               ///< External socket

protected:
    HttpHeaders     m_requestHeaders;       ///< HTTP request headers
    HttpHeaders     m_responseHeaders;      ///< HTTP response headers

protected:
    /// @brief Sends a single command to HTTP server
    ///
    /// CRLF characters are automatically appended to the command.
    /// @param cmd std::string, HTTP command
    void sendCommand(std::string cmd);

public:

    /// @brief Constructor
    ///
    /// External socket has to be active before HTTP operations.
    /// In order to use HTTPS protocol, use COpenSSLSocket.
    /// @param socket CTCPSocket&, external socket
    HttpConnect(TCPSocket& socket);

    /// @brief Destructor
    ~HttpConnect();

    /// @brief Retrieves the server response on the command
    ///
    /// Stops when HTTP server closes the connection. The server response can then be
    /// accessed through the htmlData() method.
    void getResponse();

    /// @brief Returns the internal read buffer
    ///
    /// The buffer makes sense only after sending a command to the server (if that command calls
    /// getResponse() method internally). The buffer doesn't contain HTTP headers that are parsed
    /// int m_headers map.
    /// @returns internal read buffer reference
    const CBuffer& htmlData() const
    {
        return m_readBuffer;
    }

    /// @brief Returns the HTTP request headers
    ///
    /// The HTTP headers request should be set before sending a command to the server
    /// @returns internal http request headers reference
    HttpHeaders& requestHeaders()
    {
        return m_requestHeaders;
    }

    /// @brief Returns the HTTP headers
    ///
    /// The HTTP response headers make sense only after sending a command to the server (if that command calls
    /// getResponse() method internally).
    /// @returns internal http headers reference
    const HttpHeaders& responseHeaders() const
    {
        return m_responseHeaders;
    }

    /// @brief Sends the GET command to the server
    ///
    /// Retrieves the server response into internal read buffer.
    /// @param pageName std::string, the name of the page without the server name.
    /// @param postData CHttpParams&, the list of HTTP data to pass to the server
    void cmd_get(std::string pageName, HttpParams& postData);

    /// @brief Sends the POST command to the server
    ///
    /// Retrieves the server response into internal read buffer.
    /// @param pageName std::string, the name of the page without the server name.
    /// @param postData CHttpParams&, the list of HTTP data to pass to the server
    void cmd_post(std::string pageName, HttpParams& postData);
};
/// @}
}
#endif
