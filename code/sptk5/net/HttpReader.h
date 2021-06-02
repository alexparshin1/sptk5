/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <sptk5/Buffer.h>
#include <sptk5/net/TCPSocket.h>
#include <sptk5/RegularExpression.h>
#include <sptk5/CaseInsensitiveCompare.h>

#include <mutex>

namespace sptk {

/**
 * @brief A map of HTTP headers and their values (string to string)
 */
typedef std::map<String, String, CaseInsensitiveCompare> HttpHeaders;

/**
 * HTTP response reader
 *
 * Designed to be able accepting asynchronous data
 */
class SP_EXPORT HttpReader
{
public:

    /**
     * State of the response reader
     */
    enum ReaderState : unsigned {
        READY = 0,              ///< Reader is ready to start
        READING_HEADERS = 1,    ///< Reader is reading headers
        READING_DATA = 2,       ///< Reader is reading data
        COMPLETED = 4,          ///< Reading completed
        READ_ERROR = 8          ///< Reading error (transfer terminated prematurely)
    };

    /**
     * Read mode, defines is it HTTP request (GET, POST, etc) or response.
     */
    enum ReadMode {
        REQUEST,
        RESPONSE
    };

    /**
     * Returns current reader state
     */
    ReaderState getReaderState() const;

    /**
     * Access to response headers
     */
    HttpHeaders& getHttpHeaders();

    /**
     * Read-only access to response headers by name
     * @param headerName        Header name
     */
    String httpHeader(const String& headerName) const;

    /**
     * Constructor
     * @param socket            Socket to read from
     * @param output            Output data buffer
     */
    HttpReader(TCPSocket& socket, Buffer& output, ReadMode readMode);

    /**
     * Get read socket
     * @return socket to read from
     */
    TCPSocket& socket();

    /**
     * Get output buffer
     * @return output buffer
     */
    Buffer& output();

    /**
     * Read data that can be read completely
     */
    void read();

    /**
     * Read HTTP request string
     */
    bool readHttpRequest();

    /**
     * Read headers that can be read completely
     */
    void readHttpHeaders();

    /**
     * Read HTTP headers and data after socket is just connected.
     * For requests, received by server, call readHttpRequest() first
     * @param timeout           Read timeout
     * @return HTTP status code
     */
    int readAll(std::chrono::milliseconds timeout);

    /**
     * Status code getter
     * @return status code
     */
    int getStatusCode() const;

    /**
     * Status text getter
     * @return status text
     */
    const String& getStatusText() const;

    String getRequestType() const;
    String getRequestURL() const;

    void close();

private:

    TCPSocket&          m_socket;                       ///< Socket to read from
    ReadMode            m_readMode;                     ///< Read mode
    ReaderState         m_readerState {READY};          ///< State of the reader
    mutable std::mutex  m_mutex;                        ///< Mutex that protects internal data
    String              m_statusText;                   ///< HTTP response status text
    int                 m_statusCode {0};               ///< HTTP response status code
    size_t              m_contentLength {0};            ///< Content length (as defined in responce headers), or 0
    size_t              m_contentReceivedLength {0};    ///< Received content length so far
    bool                m_contentIsChunked {false};     ///< Chunked content (as defined in responce headers)
    HttpHeaders         m_httpHeaders;                  ///< HTTP response headers
    RegularExpression   m_matchProtocolAndResponseCode {"^(HTTP\\S+)\\s+(\\d+)\\s+(.*)?\r?"}; ///< Regular expression parsing protocol and response code
    Buffer&             m_output;                       ///< Output data buffer
    Buffer              m_read_buffer;                  ///< Read buffer
    String              m_requestType;                  ///< Request type (GET, POST, etc)
    String              m_requestURL;                   ///< Request URL (for REQUEST read mode only)

    /**
     * Clear reader state
     */
    void reset();

    /**
     * Read HTTP status
     * @return true if status is read
     */
    bool readStatus();

    /**
     * Read headers that can be read completely
     */
    bool readData();

    void readDataChunk(bool& done);
};

} // namespace sptk

