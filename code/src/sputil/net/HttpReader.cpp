/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/sptk.h>

#include "sptk5/net/HttpReader.h"
#include <sptk5/Brotli.h>
#include <sptk5/ZLib.h>
#include <limits>
#include <charconv>

using namespace std;
using namespace sptk;

static constexpr auto            oneKb(1024);
static constexpr chrono::seconds defaultReadTimeout(30);

HttpReader::HttpReader(const shared_ptr<TCPSocket>& socket, Buffer& output, const ReadMode readMode)
    : SocketReader(socket)
    , m_readMode(readMode)
    , m_output(output)
{
    output.reset(oneKb);
}

bool HttpReader::readStatus()
{
    String status;
    while (readBoundedLine(status) < 3)
    {
        if (status.empty())
        {
            return false;
        }

        if (!readyToRead(nextReadTimeout()))
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
        m_statusText = trim(matches[2].value);
    }

    m_readerState = State::READING_HEADERS;

    return true;
}

void HttpReader::setReadLimits(const size_t maxHeaderBytes, const size_t maxBodyBytes)
{
    m_maxHeaderBytes = maxHeaderBytes;
    m_maxBodyBytes = maxBodyBytes;
}

size_t HttpReader::readBoundedLine(String& line)
{
    if (!m_maxHeaderBytes)
        return readLine(line);
    line.clear();
    for (;;)
    {
        if (m_headerBytes >= m_maxHeaderBytes)
            throw Exception("HTTP header limit exceeded");
        const auto timeout = nextReadTimeout();
        if (timeout.count() <= 0 || !readyToRead(timeout))
            throw TimeoutException("HTTP header read timeout");
        uint8_t byte = 0;
        if (read(&byte, 1) != 1)
            throw Exception("Truncated HTTP header");
        ++m_headerBytes;
        if (byte == '\n')
            return line.size();
        line.push_back(static_cast<char>(byte));
    }
}

bool HttpReader::readHttpRequest()
{
    String request;
    readBoundedLine(request);
    if (request.empty())
    {
        return false;
    }

    const RegularExpression parseProtocol("^(GET|POST|DELETE|PUT|OPTIONS) (\\S+)", "i");
    if (const auto matches = parseProtocol.m(request); matches)
    {
        m_requestType = upperCase(matches[0].value);
        m_requestURL = matches[1].value;
        return true;
    }

    return false;
}

void HttpReader::readHttpHeaders()
{

    clear();
    m_headerBytes = 0;

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

        readBoundedLine(header);
        header = trim(header);

        if (header.empty())
        {
            break;
        }

        if (const auto pos = header.find(':');
            pos != string::npos)
        {
            const auto name = header.substr(0, pos);
            const auto value = trim(header.substr(pos + 1));
            if (const auto existing = m_httpHeaders.find(name);
                existing != m_httpHeaders.end() &&
                (lowerCase(name) == "content-length" || lowerCase(name) == "transfer-encoding"))
                throw Exception("Duplicate HTTP framing header");
            m_httpHeaders[name] = value;
        }
    }

    m_contentLength = 0;
    m_contentIsChunked = false;

    auto itor = m_httpHeaders.find("content-length");
    if (itor != m_httpHeaders.end())
    {
        const auto& text = itor->second;
        const auto parsed = from_chars(text.data(), text.data() + text.size(), m_contentLength);
        if (text.empty() || parsed.ec != errc{} || parsed.ptr != text.data() + text.size())
            throw Exception("Invalid HTTP Content-Length");
        if (m_maxBodyBytes && m_contentLength > m_maxBodyBytes)
            throw Exception("HTTP body limit exceeded");
    }

    if (m_maxBodyBytes)
    {
        const auto encoding = m_httpHeaders.find("content-encoding");
        if (encoding != m_httpHeaders.end() && !encoding->second.empty() &&
            encoding->second.toLowerCase() != "identity")
            throw Exception("Compressed HTTP responses are not supported with a body limit");
    }

    itor = m_httpHeaders.find("transfer-encoding");
    if (itor != m_httpHeaders.end())
    {
        if (m_httpHeaders.find("content-length") != m_httpHeaders.end())
            throw Exception("Ambiguous HTTP message framing");
        if (itor->second.toLowerCase() != "chunked")
            throw Exception("Unsupported HTTP Transfer-Encoding");
        m_contentIsChunked = true;
    }

    if ((m_requestType == "POST" || m_requestType == "PUT") && !(m_contentLength > 0 || m_contentIsChunked))
    {
        constexpr auto lengthRequiredResponseCode(411);
        throw HTTPException(lengthRequiredResponseCode, "Length required");
    }

    m_contentReceivedLength = 0;

    m_readerState = State::READING_DATA;
}

namespace {
chrono::milliseconds clampTimeout(const optional<chrono::steady_clock::time_point>& deadline)
{
    if (!deadline)
    {
        return defaultReadTimeout;
    }

    if (const auto now = chrono::steady_clock::now();
        now >= *deadline)
    {
        return chrono::milliseconds::zero();
    }

    return chrono::duration_cast<chrono::milliseconds>(*deadline - chrono::steady_clock::now());
}

size_t readAndAppend(SocketReader& socketReader, Buffer& output, const size_t bytesToRead,
                     const optional<chrono::steady_clock::time_point>& deadline,
                     const size_t maxBodyBytes)
{
    if (maxBodyBytes && (output.bytes() > maxBodyBytes ||
                        bytesToRead > maxBodyBytes - output.bytes()))
        throw Exception("HTTP body limit exceeded");
    const auto readSize = min(bytesToRead, size_t(16384));
    Buffer buffer(readSize);

    if (const auto timeout = clampTimeout(deadline);
        timeout.count() <= 0 || !socketReader.readyToRead(timeout))
    {
        throw TimeoutException("Timeout reading from connection");
    }

    auto readBytes = socketReader.read(buffer, readSize);
    if (readBytes == 0)
    { // 0 bytes case is a workaround for OpenSSL
        readBytes = static_cast<int>(socketReader.read(buffer, readSize));
    }

    output.append(buffer);
    return readBytes;
}

size_t readChunk(SocketReader& socketReader, Buffer& m_output,
                 const optional<chrono::steady_clock::time_point>& deadline,
                 const size_t maxBodyBytes)
{
    // Starting next chunk
    String chunkSizeStr;
    while (chunkSizeStr.empty())
    {
        if (const auto timeout = clampTimeout(deadline);
            timeout.count() <= 0 || !socketReader.readyToRead(timeout))
        {
            throw TimeoutException("Timeout reading next chunk");
        }

        uint8_t byte = 0;
        while (true)
        {
            if (chunkSizeStr.size() >= 128)
                throw Exception("HTTP chunk header limit exceeded");
            const auto timeout = clampTimeout(deadline);
            if (timeout.count() <= 0 || !socketReader.readyToRead(timeout) ||
                socketReader.read(&byte, 1) != 1)
                throw TimeoutException("Truncated HTTP chunk header");
            if (byte == '\n')
                break;
            chunkSizeStr.push_back(static_cast<char>(byte));
        }
        chunkSizeStr = trim(chunkSizeStr);
    }

    errno = 0;
    char* end = nullptr;
    const auto parsedSize = strtoull(chunkSizeStr.c_str(), &end, 16);
    if (errno != 0 || chunkSizeStr.empty() || !isxdigit(static_cast<unsigned char>(chunkSizeStr[0])) ||
        end == chunkSizeStr.c_str() || (*end != '\0' && *end != ';') ||
        parsedSize > std::numeric_limits<size_t>::max())
    {
        throw Exception("Strange chunk size: '" + chunkSizeStr + "'");
    }
    const auto chunkSize = static_cast<size_t>(parsedSize);

    if (chunkSize == 0)
    {
        // Consume trailers, including their terminating empty line, rather than leaving bytes
        // to be mistaken for the next response on a persistent connection.
        size_t trailerBytes = 0;
        size_t lineBytes = 0;
        uint8_t previous = 0;
        for (;;)
        {
            if (++trailerBytes > 64 * 1024)
                throw Exception("HTTP trailer limit exceeded");
            uint8_t byte = 0;
            const auto timeout = clampTimeout(deadline);
            if (timeout.count() <= 0 || !socketReader.readyToRead(timeout) ||
                socketReader.read(&byte, 1) != 1)
                throw Exception("Truncated HTTP trailers");
            if (byte == '\n')
            {
                if (previous != '\r')
                    throw Exception("Invalid HTTP trailer terminator");
                if (lineBytes == 1)
                    break;
                lineBytes = 0;
            }
            else
                ++lineBytes;
            previous = byte;
        }
        return 0;
    } // Last chunk

    size_t received = 0;
    while (received < chunkSize)
    {
        const auto count = readAndAppend(socketReader, m_output, chunkSize - received,
                                         deadline, maxBodyBytes);
        if (!count)
            throw Exception("Truncated HTTP chunk");
        received += count;
    }
    for (const uint8_t expected : {'\r', '\n'})
    {
        uint8_t byte = 0;
        const auto timeout = clampTimeout(deadline);
        if (timeout.count() <= 0 || !socketReader.readyToRead(timeout) ||
            socketReader.read(&byte, 1) != 1 || byte != expected)
            throw Exception("Invalid HTTP chunk terminator");
    }
    return received;
}
} // namespace

void HttpReader::readDataChunk(bool& done)
{
    auto bytesToRead = availableBytes();
    if (bytesToRead == 0)
    {
        if (m_contentIsChunked || (m_contentLength > m_contentReceivedLength))
            throw Exception("Truncated HTTP body");
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
        const auto readBytes = static_cast<int>(readAndAppend(*this, m_output, bytesToRead,
                                                              m_readDeadline, m_maxBodyBytes));
        if (readBytes == 0 && m_contentLength > m_contentReceivedLength)
            throw Exception("Truncated HTTP body");
        m_contentReceivedLength += readBytes;
        if (m_contentLength > 0 && m_contentReceivedLength >= m_contentLength)
        { // No more data
            done = true;
        }
    }
    else
    {
        const auto chunkSize = readChunk(*this, m_output, m_readDeadline, m_maxBodyBytes);
        m_contentReceivedLength += chunkSize;
        done = chunkSize == 0; // 0 means last chunk
    }

    if (!done && m_httpHeaders.find("content-length") == m_httpHeaders.end() && !m_contentIsChunked &&
        !m_maxBodyBytes)
    {
        const auto readBytes = static_cast<int>(availableBytes());
        if (readBytes == 0 && m_output.bytes() > 13)
        {
            const auto   tailOffset = m_output.bytes() - 13;
            const String tail(m_output.c_str() + tailOffset);
            if (tail.toLowerCase().find("</html>") != string::npos)
            {
                done = true;
            }
            else if (const auto timeout = nextReadTimeout();
                     timeout.count() <= 0 || !readyToRead(timeout))
            {
                throw TimeoutException("Read timeout");
            }
        }
    }
}

bool HttpReader::readData()
{
    if (m_httpHeaders.find("content-length") != m_httpHeaders.end() && m_contentLength == 0)
        return true;
    auto done {false};
    while (!done)
    {
        if (const auto timeout = nextReadTimeout();
            timeout.count() <= 0 || !readyToRead(timeout))
        {
            throw TimeoutException("Read timeout");
        }
        readDataChunk(done);
    }
    return true;
}

void HttpReader::readStream()
{
    constexpr auto httpErrorResponseCode(400);
    constexpr auto serverErrorResponseCode(500);

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

    // Only when reading a response. On a request, "Connection: close" is the client saying it
    // will send no more of them - the server still owes it an answer, and closing here threw that
    // answer away: a POST carrying the header got no reply at all, while a GET, whose body is
    // never read, was answered normally. Reading a response, the header means the peer is done
    // with the connection and closing is right. Compared without regard to case, as HTTP defines
    // it: "Connection: Close" used to take the other branch and behave differently.
    if (m_readMode == ReadMode::RESPONSE)
    {
        itor = m_httpHeaders.find("Connection");
        if (itor != m_httpHeaders.end() && itor->second.toLowerCase() == "close")
        {
            close();
        }
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

int HttpReader::readAll(const chrono::milliseconds& timeout)
{
    setReadDeadline(chrono::steady_clock::now() + timeout);
    while (getReaderState() < State::COMPLETED)
    {
        if (const auto readTimeout = nextReadTimeout();
            readTimeout.count() <= 0 || !readyToRead(readTimeout))
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

void HttpReader::setReadDeadline(const optional<chrono::steady_clock::time_point>& deadline)
{
    const scoped_lock lock(m_mutex);
    m_readDeadline = deadline;
}

String HttpReader::getRequestType() const
{
    return m_requestType;
}

String HttpReader::getRequestURL() const
{
    return m_requestURL;
}

chrono::milliseconds HttpReader::nextReadTimeout() const
{
    if (!m_readDeadline)
    {
        return defaultReadTimeout;
    }

    const auto now = chrono::steady_clock::now();
    if (now >= *m_readDeadline)
    {
        return chrono::milliseconds::zero();
    }

    return chrono::duration_cast<chrono::milliseconds>(*m_readDeadline - now);
}
