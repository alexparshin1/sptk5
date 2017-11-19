/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       HttpConnect.h - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __HTTPCONNECT_H__
#define __HTTPCONNECT_H__

#include <sptk5/RegularExpression.h>
#include <sptk5/Strings.h>
#include <sptk5/net/HttpParams.h>
#include <sptk5/net/HttpReader.h>
#include <sptk5/net/TCPSocket.h>

namespace sptk
{

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief HTTP socket
 *
 * Implements the GET, POST, PUT and DELETE methods of HTTP protocol.
 * Allows to define the host, port, submit information, and then sends the HTML request to the server.
 */
class SP_EXPORT HttpConnect
{
    /**
     * HTTP reader
     */
    HttpReader      m_reader;

    /**
     * External socket
     */
    TCPSocket&      m_socket;

protected:
    /**
     * HTTP request headers
     */
    HttpHeaders     m_requestHeaders;

protected:

    /**
     * Create default headers for HTTP request
     */
    sptk::Strings makeHeaders(const std::string& httpCommand, const std::string& pageName,
                              const HttpParams& requestParameters);

    /**
     * @brief Sends a single command to HTTP server
     *
     * CRLF characters are automatically appended to the command.
     * @param cmd std::string, HTTP command
     */
    void sendCommand(const std::string& cmd);

    /**
     * @brief Sends a single command to HTTP server
     *
     * CRLF characters are automatically appended to the command.
     * @param cmd const Buffer&, HTTP command
     */
    void sendCommand(const Buffer& cmd);

    /**
     * @brief Retrieves the server response on the command
     *
     * Stops when HTTP server closes the connection. The server response can then be
     * accessed through the htmlData() method.
     * @param timeout std::chrono::milliseconds, Response timeout
     * @return HTTP result code
     */
    int getResponse(std::chrono::milliseconds timeout);

public:

    /**
     * @brief Constructor
     *
     * External socket has to be active before HTTP operations.
     * In order to use HTTPS protocol, use COpenSSLSocket.
     * @param socket CTCPSocket&, external socket
     */
    HttpConnect(TCPSocket& socket);

    /**
     * @brief Returns the internal read buffer
     *
     * The buffer makes sense only after sending a command to the server (if that command calls
     * getResponse() method internally). The buffer doesn't contain HTTP headers that are parsed
     * int m_headers map.
     * @returns internal read buffer reference
     */
    const Buffer& htmlData() const
    {
        return m_reader;
    }

    /**
     * @brief Returns the HTTP request headers
     *
     * The HTTP headers request should be set before sending a command to the server
     * @returns internal http request headers reference
     */
    HttpHeaders& requestHeaders()
    {
        return m_requestHeaders;
    }

    /**
     * @brief Returns the HTTP headers
     *
     * The HTTP response headers make sense only after sending a command to the server (if that command calls
     * getResponse() method internally).
     * @returns internal http headers reference
     */
    const HttpHeaders& responseHeaders() const
    {
        return m_reader.getResponseHeaders();
    }

    /**
     * @brief Sends the GET command to the server
     *
     * Retrieves the server response into internal read buffer.
     * @param pageName std::string, the name of the page without the server name.
     * @param parameters const HttpParams&, the list of HTTP data to pass to the server
     * @param timeout std::chrono::milliseconds, response timeout
     * @return HTTP result code
     */
    int cmd_get(const std::string& pageName, const HttpParams& parameters, std::chrono::milliseconds timeout);

    /**
     * @brief Sends the POST command to the server
     *
     * Retrieves the server response into internal read buffer.
     * @param pageName std::string, the name of the page without the server name.
     * @param parameters const HttpParams&, the list of HTTP data to pass to the server
     * @param content const Buffer&, the data to post to the server
     * @param gzipContent bool, if true then compress buffer and set HTTP header Content-Encoding
     * @param timeout std::chrono::milliseconds, response timeout
     * @return HTTP result code
     */
    int cmd_post(const std::string& pageName, const HttpParams& parameters, const Buffer& content, bool gzipContent,
                 std::chrono::milliseconds timeout);

    /**
     * @brief Sends the PUT command to the server
     *
     * Retrieves the server response into internal read buffer.
     * @param pageName std::string, the name of the page without the server name.
     * @param parameters const HttpParams&, the list of HTTP data to pass to the server
     * @param content const Buffer&, the data to post to the server
     * @param timeout std::chrono::milliseconds, response timeout
     * @return HTTP result code
     */
    int cmd_put(const std::string& pageName, const HttpParams& parameters, const Buffer& content, std::chrono::milliseconds timeout);

   /**
     * @brief Sends the DELETE command to the server
     *
     * Retrieves the server response into internal read buffer.
     * @param pageName std::string, the name of the page without the server name.
     * @param parameters const HttpParams&, the list of HTTP data to pass to the server
     * @param timeout std::chrono::milliseconds, response timeout, milliseconds
     * @return HTTP result code
     */
    int cmd_delete(const std::string& pageName, const HttpParams& parameters, std::chrono::milliseconds timeout);

    /**
     * @brief Get value of response header
     * @param headerName std::string, response header name
     * @return header value, or empty string if header is not a part of the response
     */
    std::string responseHeader(const std::string& headerName) const;
};

/**
 * @}
 */
}
#endif
