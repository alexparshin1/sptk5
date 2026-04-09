/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/CaseInsensitiveCompare.h>
#include <sptk5/RegularExpression.h>
#include <sptk5/net/SocketReader.h>
#include <sptk5/net/TCPSocket.h>

#include <mutex>

namespace sptk {

/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief A map of HTTP headers and their values (string to string).
 */
using HttpHeaders = std::map<String, String, CaseInsensitiveCompare<String>>;

/**
 * @brief HTTP response reader.
 *
 * Designed to be able to accept asynchronous data.
 */
class SP_EXPORT HttpReader : public SocketReader
{
public:
    /**
     * @brief State of the response reader.
     */
    enum class State : unsigned
    {
        READY = 0,           ///< Reader is ready to start.
        READING_HEADERS = 1, ///< Reader is reading headers.
        READING_DATA = 2,    ///< Reader is reading data.
        COMPLETED = 4,       ///< Reading completed.
        READ_ERROR = 8       ///< Reading error (transfer terminated prematurely).
    };

    /**
     * @brief Read mode, defines is it the HTTP request (GET, POST, etc.) or response.
     */
    enum class ReadMode
    {
        REQUEST,
        RESPONSE
    };

    /**
     * @brief Returns current reader state.
     */
    State getReaderState() const;

    /**
     * @brief Access to response headers.
     */
    HttpHeaders& getHttpHeaders();

    /**
     * @brief Read-only access to response headers by name.
     * @param headerName        Header name.
     */
    String httpHeader(const String& headerName) const;

    /**
     * @brief Constructor.
     * @param socket            Socket to read from.
     * @param output            Output data buffer.
     * @param readMode          HTTP response read mode.
     */
    HttpReader(const std::shared_ptr<TCPSocket>& socket, Buffer& output, ReadMode readMode);

    /**
     * @brief Get the output buffer.
     * @return output buffer.
     */
    Buffer& output();

    /**
     * @brief Read data that can be read completely.
     */
    void readStream();

    /**
     * @brief Read HTTP request string.
     */
    bool readHttpRequest();

    /**
     * @brief Read headers that can be read completely.
     */
    void readHttpHeaders();

    /**
     * @brief Read HTTP headers and data after the socket is just connected.
     * For requests received by the server, call readHttpRequest() first.
     * @param timeout           Read timeout.
     * @return HTTP status code.
     */
    int readAll(const std::chrono::milliseconds& timeout);

    /**
     * @brief Status code getter.
     * @return status code.
     */
    int getStatusCode() const;

    /**
     * @brief Status text getter.
     * @return status text.
     */
    const String& getStatusText() const;

    String getRequestType() const;
    String getRequestURL() const;

private:
    ReadMode           m_readMode;                                                           ///< Read mode.
    State              m_readerState {State::READY};                                         ///< State of the reader.
    mutable std::mutex m_mutex;                                                              ///< Mutex that protects internal data.
    String             m_statusText;                                                         ///< HTTP response status text.
    int                m_statusCode {0};                                                     ///< HTTP response status code.
    size_t             m_contentLength {0};                                                  ///< Content length (as defined in responce headers), or 0.
    size_t             m_contentReceivedLength {0};                                          ///< Received content length so far.
    bool               m_contentIsChunked {false};                                           ///< Chunked content (as defined in responce headers).
    HttpHeaders        m_httpHeaders;                                                        ///< HTTP response headers.
    RegularExpression  m_matchProtocolAndResponseCode {"^(HTTP\\S+)\\s+(\\d+)\\s+(.*)?\r?"}; ///< Regular expression parsing protocol and response code.
    Buffer&            m_output;                                                             ///< Output data buffer.
    Buffer             m_read_buffer;                                                        ///< Read buffer.
    String             m_requestType;                                                        ///< Request type (GET, POST, etc.).
    String             m_requestURL;                                                         ///< Request URL (for REQUEST read mode only).

    /**
     * @brief Clear reader state.
     */
    void clear() override;

    /**
     * @brief Read HTTP status.
     * @return true if status is read.
     */
    bool readStatus();

    /**
     * @brief Read headers that can be read completely.
     */
    bool readData();

    void readDataChunk(bool& done);
};

/**
 * @}
 */

} // namespace sptk
