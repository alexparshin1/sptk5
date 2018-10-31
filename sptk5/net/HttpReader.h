/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       HttpReader.h - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Saturday November 18 2017                              ║
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

#ifndef __HTTP_READER_H__
#define __HTTP_READER_H__

#include <sptk5/Buffer.h>
#include <sptk5/net/TCPSocket.h>
#include <sptk5/RegularExpression.h>
#include <mutex>

namespace sptk {

struct caseInsensitiveCompare : public std::binary_function<String, String, bool>
{
    bool operator()(const String &lhs, const String &rhs) const
    {
#ifdef _WIN32
        return _stricmp(lhs.c_str(), rhs.c_str()) < 0;
#else
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
#endif
    }
};
/**
 * @brief A map of HTTP headers and their values (string to string)
 */
typedef std::map<String, String, caseInsensitiveCompare> HttpHeaders;

/**
 * HTTP response reader
 *
 * Designed to be able accepting asynchronous data
 */
class HttpReader
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

private:

    /**
     * State of the response reader
     */
    ReaderState         m_readerState;

public:

    /**
     * Returns current reader state
     */
    ReaderState getReaderState() const;

private:

    /**
     * Mutex that protects internal data
     */
    mutable std::mutex  m_mutex;

    /**
     * HTTP response status text
     */
    String              m_statusText;

    /**
     * HTTP response status code
     */
    int                 m_statusCode;

    /**
     * Content length (as defined in responce headers), or 0
     */
    size_t              m_contentLength;

    /**
     * Received content length so far
     */
    size_t              m_contentReceivedLength;

    /**
     * Current chunk size
     */
    size_t              m_currentChunkSize;

    /**
     * Chunked content (as defined in responce headers)
     */
    bool                m_contentIsChunked;

    /**
     * HTTP response headers
     */
    HttpHeaders         m_responseHeaders;

    /**
     * Regular expression parsing protocol and response code
     */
    RegularExpression   m_matchProtocolAndResponseCode;

    /**
     * Output data buffer
     */
    Buffer&             m_output;

    /**
     * Read buffer
     */
    Buffer              m_read_buffer;

public:

    /**
     * Read-only access to response headers
     */
    const HttpHeaders& getResponseHeaders() const;

    /**
     * Read-only access to response headers by name
     * @param headerName        Header name
     */
    String responseHeader(const String& headerName) const;

private:

    /**
     * Read HTTP status
     * @param socket            Socket to read from
     * @return true if status is read
     */
    bool readStatus(TCPSocket& socket);

    /**
     * Read headers that can be read completely
     * @param socket            Socket to read from
     */
    bool readHeaders(TCPSocket& socket);

    /**
     * Read headers that can be read completely
     * @param socket            Socket to read from
     */
    bool readData(TCPSocket& socket);

public:
    /**
     * Constructor
     * @param output            Output data buffer
     */
    HttpReader(Buffer& output);

    /**
     * Read data that can be read completely
     * @param socket            Socket to read from
     */
    void read(TCPSocket& socket);

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
};

} // namespace sptk

#endif
