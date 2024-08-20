/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include "sptk5/net/HttpReader.h"
#include <sptk5/Brotli.h>
#include <sptk5/ZLib.h>
#include <thread>

using namespace std;
using namespace sptk;

static constexpr int oneKb(1024);
static constexpr chrono::seconds thirtySeconds(30);

HttpReader::HttpReader(TCPSocket& socket, Buffer& output, ReadMode readMode)
    : SocketReader(socket)
    , m_readMode(readMode)
    , m_output(output)
{
    output.reset(oneKb);
}

bool HttpReader::readStatus()
{
    String status;
    while (readLine(status) < 3)
    {
        if (status.empty())
        {
            return false;
        }

        if (!readyToRead(thirtySeconds))
        {
            throw Exception("Can't read server response: timeout");
        }
    }

    const auto matches = m_matchProtocolAndResponseCode.m(status);
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
    readLine(request);
    if (request.empty())
    {
        return false;
    }

    const RegularExpression parseProtocol("^(GET|POST|DELETE|PUT|OPTIONS) (\\S+)", "i");
    if (const auto matches = parseProtocol.m(request); matches)
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

    clear();

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
    for (;;)
    {
        String header;

        readLine(header);

        if (header.empty())
        {
            continue;
            //throw Exception("Empty HTTP header");
        }


        if (const size_t pos = header.find(':');
            pos != string::npos)
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
        m_contentLength = static_cast<size_t>(string2int(itor->second));
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

namespace {
size_t readAndAppend(SocketReader& socketReader, Buffer& output, size_t bytesToRead)
{
    Buffer buffer(bytesToRead);

    if (!socketReader.readyToRead(thirtySeconds))
    {
        throw TimeoutException("Timeout reading from connection");
    }

    size_t readBytes = socketReader.read(buffer, bytesToRead);
    if (readBytes == 0)
    { // 0 bytes case is a workaround for OpenSSL
        readBytes = static_cast<int>(socketReader.read(buffer, bytesToRead));
    }

    output.append(buffer);
    return readBytes;
}

size_t readChunk(SocketReader& socketReader, Buffer& m_output)
{
    // Starting next chunk
    String chunkSizeStr;
    while (chunkSizeStr.empty())
    {
        if (!socketReader.readyToRead(thirtySeconds))
        {
            throw TimeoutException("Timeout reading next chunk");
        }

        if (socketReader.readLine(chunkSizeStr) > 0)
        {
            chunkSizeStr = trim(chunkSizeStr);
        }
    }

    errno = 0;
    const auto chunkSize = static_cast<size_t>(strtol(chunkSizeStr.c_str(), nullptr, 16));
    if (errno != 0)
    {
        throw Exception("Strange chunk size: '" + chunkSizeStr + "'");
    }

    if (chunkSize == 0)
    {
        return 0;
    } // Last chunk

    return readAndAppend(socketReader, m_output, chunkSize);
}
} // namespace

void HttpReader::readDataChunk(bool& done)
{
    size_t bytesToRead = availableBytes();
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
        bytesToRead = availableBytes();
    }

    if (!m_contentIsChunked)
    {
        const auto readBytes = static_cast<int>(readAndAppend(*this, m_output, bytesToRead));
        m_contentReceivedLength += readBytes;
        if (m_contentLength > 0 && m_contentReceivedLength >= m_contentLength)
        { // No more data
            done = true;
        }
    }
    else
    {
        const size_t chunkSize = readChunk(*this, m_output);
        m_contentReceivedLength += chunkSize;
        done = (chunkSize == 0); // 0 means last chunk
    }

    if (!done)
    {
        const auto readBytes = static_cast<int>(availableBytes());
        if (readBytes == 0 && m_output.bytes() > 13)
        {
            const size_t tailOffset = m_output.bytes() - 13;
            const String tail(m_output.c_str() + tailOffset);
            if (tail.toLowerCase().find("</html>") != string::npos)
            {
                done = true;
            }
            else if (!readyToRead(thirtySeconds))
            {
                throw TimeoutException("Read timeout");
            }
        }
    }
}

bool HttpReader::readData()
{
    bool done {false};
    while (!done && readyToRead(thirtySeconds))
    {
        readDataChunk(done);
    }
    return true;
}

void HttpReader::readStream()
{
    constexpr int httpErrorResponseCode(400);
    constexpr int serverErrorResponseCode(500);

    const scoped_lock lock(m_mutex);

    if (m_readerState == State::READY)
    {
        readHttpHeaders();
        m_readerState = State::READING_DATA;
    }

    if (m_readerState == State::READING_DATA)
    {
        readData();
    }

    auto itor = m_httpHeaders.find("content-encoding");
    if (itor != m_httpHeaders.end())
    {
        if (itor->second == "gzip")
        {
#ifdef HAVE_ZLIB
            Buffer unzipBuffer;
            ZLib::decompress(unzipBuffer, m_output);
            m_output = std::move(unzipBuffer);
            itor->second = "";
#else
            throw Exception("Content-Encoding is 'gzip', but zlib support is not enabled in SPTK");
#endif
        }
        if (itor->second == "br")
        {
#ifdef HAVE_BROTLI
            Buffer unzipBuffer;
            Brotli::decompress(unzipBuffer, m_output);
            m_output = std::move(unzipBuffer);
            itor->second = "";
#else
            throw Exception("Content-Encoding is 'br', but libbrotli support is not enabled in SPTK");
#endif
        }
    }

    itor = m_httpHeaders.find("Connection");
    if (itor != m_httpHeaders.end() && itor->second == "close")
    {
        close();
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
    const scoped_lock lock(m_mutex);
    return m_httpHeaders;
}

HttpReader::State HttpReader::getReaderState() const
{
    const scoped_lock lock(m_mutex);
    return m_readerState;
}

int HttpReader::getStatusCode() const
{
    const scoped_lock lock(m_mutex);
    return m_statusCode;
}

const String& HttpReader::getStatusText() const
{
    const scoped_lock lock(m_mutex);
    return m_statusText;
}

String HttpReader::httpHeader(const String& headerName) const
{
    const scoped_lock lock(m_mutex);

    const auto itor = m_httpHeaders.find(lowerCase(headerName));
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
        if (!readyToRead(timeout))
        {
            close();
            throw Exception("Response read timeout");
        }

        readStream();
    }

    return getStatusCode();
}

void HttpReader::clear()
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
    SocketReader::clear();
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
