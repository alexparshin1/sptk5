/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       HttpConnect.h - description                            ║
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
    std::unique_ptr<HttpReader> m_reader;

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
    sptk::Strings makeHeaders(const sptk::String& httpCommand, const sptk::String& pageName,
                              const HttpParams& requestParameters);

    /**
     * @brief Sends a single command to HTTP server
     *
     * CRLF characters are automatically appended to the command.
     * @param cmd               HTTP command
     */
    void sendCommand(const sptk::String& cmd);

    /**
     * @brief Sends a single command to HTTP server
     *
     * CRLF characters are automatically appended to the command.
     * @param cmd               HTTP command
     */
    void sendCommand(const Buffer& cmd);

    /**
     * @brief Retrieves the server response on the command
     *
     * Stops when HTTP server closes the connection. The server response can then be
     * accessed through the htmlData() method.
     * @param timeout           Response timeout
     * @return HTTP result code
     */
    int getResponse(Buffer& output, std::chrono::milliseconds timeout);

public:

    /**
     * @brief Constructor
     *
     * External socket has to be active before HTTP operations.
     * In order to use HTTPS protocol, use COpenSSLSocket.
     * @param socket            external socket
     */
    explicit HttpConnect(TCPSocket& socket);

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
        return m_reader->getResponseHeaders();
    }

    /**
     * @brief Sends the GET command to the server
     *
     * Retrieves the server response into internal read buffer.
     * @param pageName          Page URL without the server name.
     * @param parameters        HTTP request parameters
     * @param output            Output data
     * @param timeout           Response timeout
     * @return HTTP result code
     */
    int cmd_get(const String& pageName, const HttpParams& parameters, Buffer& output,
                std::chrono::milliseconds timeout=std::chrono::seconds(60));

    /**
     * @brief Sends the POST command to the server
     *
     * Retrieves the server response into internal read buffer.
     * @param pageName          Page URL without the server name.
     * @param parameters        HTTP request parameters
     * @param content           The data to post to the server
     * @param gzipContent       If true then compress buffer and set HTTP header Content-Encoding
     * @param output            Output data
     * @param timeout           Response timeout
     * @return HTTP result code
     */
    int cmd_post(const sptk::String& pageName, const HttpParams& parameters, const Buffer& content, bool gzipContent,
                 Buffer& output, std::chrono::milliseconds timeout = std::chrono::seconds(60));

    /**
     * @brief Sends the PUT command to the server
     *
     * Retrieves the server response into internal read buffer.
     * @param pageName          Page URL without the server name.
     * @param parameters        HTTP request parameters
     * @param content           The data to post to the server
     * @param output            Output data
     * @param timeout           Response timeout
     * @return HTTP result code
     */
    int cmd_put(const sptk::String& pageName, const HttpParams& parameters, const Buffer& content, Buffer& output,
                std::chrono::milliseconds timeout = std::chrono::seconds(60));

   /**
     * @brief Sends the DELETE command to the server
     *
     * Retrieves the server response into internal read buffer.
     * @param pageName          Page URL without the server name.
     * @param parameters        HTTP request parameters
     * @param output            Output data
     * @param timeout           Request timeout
     * @return HTTP result code
     */
   int cmd_delete(const sptk::String& pageName, const HttpParams& parameters, Buffer& output,
                 std::chrono::milliseconds timeout = std::chrono::seconds(60));

    /**
     * @brief Get value of response header
     * @param headerName        Response header name
     * @return header value, or empty string if header is not a part of the response
     */
    String responseHeader(const sptk::String& headerName) const;

    /**
     * @brief Get the request execution status code
     * @return request execution status code
     */
    int statusCode() const;

    /**
     * @brief Get the request execution status text
     * @return request execution status text
     */
    sptk::String statusText() const;
};

/**
 * @}
 */
}
#endif
