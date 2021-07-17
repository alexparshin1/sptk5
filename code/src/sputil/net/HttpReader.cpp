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

#include <sptk5/sptk.h>
#include <sptk5/net/TCPSocket.h>
#include <sptk5/ZLib.h>
#include <thread>
#include <sptk5/Brotli.h>
#include "sptk5/net/HttpReader.h"

using namespace std;
using namespace sptk;

static constexpr int oneKb(1024);
static constexpr chrono::seconds thirtySeconds(30);

HttpReader::HttpReader(TCPSocket& socket, Buffer& output, ReadMode readMode)
    : m_socket(socket),
      m_readMode(readMode),
      m_output(output)
{
    output.reset(oneKb);
}

bool HttpReader::readStatus()
{
    String status;
    while (m_socket.readLine(status) < 3)
    {
        if (status.empty())
        {
            return false;
        }
        m_socket.readyToRead(chrono::seconds(1));
    }

    auto matches = m_matchProtocolAndResponseCode.m(status);
    if (!matches)
    {
        m_readerState = State::READ_ERROR;
        throw Exception("Broken HTTP version header: [" + status + "]");
    }
    m_statusCode = string2int(matches[1].value);
    if (matches.groups().size() > 2)
    {
        m_statusText = matches[2].value.trim();
    }

    m_readerState = State::READING_HEADERS;

    return true;
}

bool HttpReader::readHttpRequest()
{
    String request;
    m_socket.readLine(request);
    if (request.empty())
    {
        return false;
    }

    RegularExpression parseProtocol("^(GET|POST|DELETE|PUT|OPTIONS) (\\S+)", "i");
    if (auto matches = parseProtocol.m(request); matches)
    {
        m_requestType = matches[0].value.toUpperCase();
        m_requestURL = matches[1].value;
        return true;
    }

    return false;
}

void HttpReader::readHttpHeaders()
{
    constexpr int lengthRequiredResponseCode(411);

    reset();

    if (m_readMode == ReadMode::RESPONSE)
    {
        if (!readStatus())
        {
            throw Exception("Can't read server response");
        }
    }
    else
    {
        if (!readHttpRequest())
        {
            throw Exception("Can't read server response");
        }
    }

    /// Reading HTTP headers
    Strings matches;
    for (;;)
    {
        String header;

        m_socket.readLine(header);

        if (header.empty())
        {
            throw Exception("Empty HTTP header");
        }


        if (size_t pos = header.find(':'); pos != string::npos)
        {
            m_httpHeaders[lowerCase(header.substr(0, pos))] = trim(header.substr(pos + 1));
            continue;
        }

        if (header[0] == '\r')
        {
            break;
        }
    }

    m_contentLength = 0;
    m_contentIsChunked = false;

    auto itor = m_httpHeaders.find("content-length");
    if (itor != m_httpHeaders.end())
    {
        m_contentLength = (size_t) string2int(itor->second);
    }

    itor = m_httpHeaders.find("transfer-encoding");
    if (itor != m_httpHeaders.end())
    {
        m_contentIsChunked = itor->second.find("chunked") != string::npos;
    }

    if ((m_requestType == "POST" || m_requestType == "PUT") && !(m_contentLength > 0 || m_contentIsChunked))
    {
        throw HTTPException(lengthRequiredResponseCode, "Length required");
    }

    m_contentReceivedLength = 0;

    m_readerState = State::READING_DATA;
}

static size_t readAndAppend(TCPSocket& socket, Buffer& output, size_t bytesToRead)
{
    Buffer buffer(bytesToRead);

    if (!socket.readyToRead(thirtySeconds))
    {
        throw TimeoutException("Timeout reading from connection");
    }

    size_t readBytes = socket.read(buffer, bytesToRead);
    if (readBytes == 0)
    { // 0 bytes case is a workaround for OpenSSL
        readBytes = (int) socket.read(buffer, bytesToRead);
    }

    output.append(buffer);
    return readBytes;
}

static size_t readChunk(TCPSocket& socket, Buffer& m_output)
{
    // Starting next chunk
    String chunkSizeStr;
    while (chunkSizeStr.empty())
    {
        if (!socket.readyToRead(thirtySeconds))
        {
            throw TimeoutException("Timeout reading next chunk");
        }
        if (socket.readLine(chunkSizeStr) > 0)
        {
            chunkSizeStr = trim(chunkSizeStr);
        }
    }

    errno = 0;
    auto chunkSize = (size_t) strtol(chunkSizeStr.c_str(), nullptr, 16);
    if (errno != 0)
    {
        throw Exception("Strange chunk size: '" + chunkSizeStr + "'");
    }

    if (chunkSize == 0)
    {
        return 0;
    } // Last chunk

    return readAndAppend(socket, m_output, chunkSize);
}

void HttpReader::readDataChunk(bool& done)
{
    size_t bytesToRead = m_socket.socketBytes();
    if (bytesToRead == 0)
    {
        done = true;
        return;
    }

    if (m_contentLength > 0)
    {
        bytesToRead = m_contentLength - m_contentReceivedLength;
        if (bytesToRead == 0)
        { // received all content bytes
            done = true;
            return;
        }
    }
    else
    {
        bytesToRead = m_socket.socketBytes();
    }

    int readBytes = 0;
    if (!m_contentIsChunked)
    {
        readBytes = (int) readAndAppend(m_socket, m_output, bytesToRead);
        m_contentReceivedLength += readBytes;
        if (m_contentLength > 0 && m_contentReceivedLength >= m_contentLength)
        { // No more data
            done = true;
        }
    }
    else
    {
        size_t chunkSize = readChunk(m_socket, m_output);
        m_contentReceivedLength += chunkSize;
        done = (chunkSize == 0); // 0 means last chunk
    }

    if (!done)
    {
        readBytes = (int) m_socket.socketBytes();
        if (readBytes == 0 && m_output.bytes() > 13)
        {
            size_t tailOffset = m_output.bytes() - 13;
            String tail(m_output.c_str() + tailOffset);
            if (tail.toLowerCase().find("</html>") != string::npos)
            {
                done = true;
            }
            else if (!m_socket.readyToRead(thirtySeconds))
            {
                throw TimeoutException("Read timeout");
            }
        }
    }
}

bool HttpReader::readData()
{
    bool done {false};
    while (!done && m_socket.readyToRead(thirtySeconds))
    {
        readDataChunk(done);
    }
    return true;
}

void HttpReader::read()
{
    constexpr int httpErrorResponseCode(400);
    constexpr int serverErrorResponseCode(500);

    scoped_lock lock(m_mutex);

    if (m_readerState == State::READY)
    {
        readHttpHeaders();
        m_readerState = State::READING_DATA;
    }

    if (m_readerState == State::READING_DATA && !readData())
    {
        return;
    }

    auto itor = m_httpHeaders.find("content-encoding");
    if (itor != m_httpHeaders.end())
    {
        if (itor->second == "gzip")
        {
#if HAVE_ZLIB
            Buffer unzipBuffer;
            ZLib::decompress(unzipBuffer, m_output);
            m_output = move(unzipBuffer);
            itor->second = "";
#else
            throw Exception("Content-Encoding is 'gzip', but zlib support is not enabled in SPTK");
#endif
        }
        if (itor->second == "br")
        {
#if HAVE_BROTLI
            Buffer unzipBuffer;
            Brotli::decompress(unzipBuffer, m_output);
            m_output = move(unzipBuffer);
            itor->second = "";
#else
            throw Exception("Content-Encoding is 'br', but libbrotli support is not enabled in SPTK");
#endif
        }
    }

    itor = m_httpHeaders.find("Connection");
    if (itor != m_httpHeaders.end() && itor->second == "close")
    {
        m_socket.close();
    }

    if (m_statusCode >= httpErrorResponseCode && m_statusText.empty())
    {
        if (m_statusCode >= serverErrorResponseCode)
        {
            m_statusText = "Unknown server error";
        }
        else
        {
            m_statusText = "Unknown client error";
        }
    }

    m_readerState = State::COMPLETED;
}

HttpHeaders& HttpReader::getHttpHeaders()
{
    scoped_lock lock(m_mutex);
    return m_httpHeaders;
}

HttpReader::State HttpReader::getReaderState() const
{
    scoped_lock lock(m_mutex);
    return m_readerState;
}

int HttpReader::getStatusCode() const
{
    scoped_lock lock(m_mutex);
    return m_statusCode;
}

const String& HttpReader::getStatusText() const
{
    scoped_lock lock(m_mutex);
    return m_statusText;
}

String HttpReader::httpHeader(const String& headerName) const
{
    scoped_lock lock(m_mutex);

    auto itor = m_httpHeaders.find(lowerCase(headerName));
    if (itor == m_httpHeaders.end())
    {
        return "";
    }
    return itor->second;
}

int HttpReader::readAll(std::chrono::milliseconds timeout)
{
    while (getReaderState() < HttpReader::State::COMPLETED)
    {
        if (!m_socket.readyToRead(timeout))
        {
            m_socket.close();
            throw Exception("Response read timeout");
        }

        read();
    }

    return getStatusCode();
}

void HttpReader::reset()
{
    m_readerState = State::READY;
    m_statusText = "";
    m_statusCode = 0;
    m_contentLength = 0;
    m_contentReceivedLength = 0;
    m_contentIsChunked = false;
    m_httpHeaders.clear();
    m_read_buffer.reset(oneKb);
    m_requestType = "";
    m_requestURL = "";
}

TCPSocket& HttpReader::socket()
{
    return m_socket;
}

Buffer& HttpReader::output()
{
    return m_output;
}

String HttpReader::getRequestType() const
{
    return m_requestType;
}

String HttpReader::getRequestURL() const
{
    return m_requestURL;
}

void HttpReader::close()
{
    m_socket.close();
}
