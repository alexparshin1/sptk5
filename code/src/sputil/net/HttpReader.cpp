/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       HttpReader.h - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Saturday November 18 2017                              ║
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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
#include "sptk5/net/HttpReader.h"

using namespace std;
using namespace sptk;

HttpReader::HttpReader(TCPSocket& socket, Buffer& output, ReadMode readMode)
: m_socket(socket),
  m_readMode(readMode),
  m_output(output)
{
    output.reset(128);
}

bool HttpReader::readStatus()
{
    String status;
    while (m_socket.readLine(status) < 3) {
        if (status.empty())
            return false;
        m_socket.readyToRead(chrono::seconds(1));
    }

    auto matches = m_matchProtocolAndResponseCode.m(status);
    if (!matches) {
        m_readerState = READ_ERROR;
        throw Exception("Broken HTTP version header: [" + status + "]");
    }
    m_statusCode = string2int(matches[1].value);
    if (matches.groups().size() > 2)
        m_statusText = matches[2].value.trim();

    m_readerState = READING_HEADERS;

    return true;
}

bool HttpReader::readHttpRequest()
{
    String request;
    m_socket.readLine(request);
    if (request.empty())
        return false;

    RegularExpression parseProtocol("^(GET|POST|DELETE|PUT|OPTIONS) (\\S+)", "i");
    auto matches = parseProtocol.m(request);
    if (matches) {
        m_requestType = matches[0].value.toUpperCase();
        m_requestURL = matches[1].value;
        return true;
    }

    return false;
}

bool HttpReader::readHttpHeaders()
{
    reset();

    if (m_readMode == RESPONSE) {
        if (!readStatus())
            throw Exception("Can't read server response");
    } else {
        if (!readHttpRequest())
            throw Exception("Can't read server response");
    }

    /// Reading HTTP headers
    Strings matches;
    for (;;) {
        String header;

        m_socket.readLine(header);

        if (header.empty())
            return false;

        size_t pos = header.find(':');
        if (pos != string::npos) {
            m_httpHeaders[lowerCase(header.substr(0, pos))] = trim(header.substr(pos + 1));
            continue;
        }

        if (header[0] == '\r')
            break;
    }

    m_contentLength = 0;
    m_contentIsChunked = false;

    auto itor = m_httpHeaders.find("content-length");
    if (itor != m_httpHeaders.end())
        m_contentLength = (size_t) string2int(itor->second);

    itor = m_httpHeaders.find("transfer-encoding");
    if (itor != m_httpHeaders.end())
        m_contentIsChunked = itor->second.find("chunked") != string::npos;

    m_contentReceivedLength = 0;

    m_readerState = READING_DATA;
    return true;
}

static size_t readAndAppend(TCPSocket& socket, Buffer& output, size_t bytesToRead)
{
    Buffer buffer(bytesToRead);

    if (!socket.readyToRead(chrono::seconds(30)))
        throw TimeoutException("Timeout reading from connection");

    size_t readBytes = socket.read(buffer, bytesToRead);
    if (readBytes == 0) // 0 bytes case is a workaround for OpenSSL
        readBytes = (int) socket.read(buffer, bytesToRead);

    output.append(buffer);
    return readBytes;
}

static size_t readChunk(TCPSocket& socket, Buffer& m_output)
{
    // Starting next chunk
    String chunkSizeStr;
    while (chunkSizeStr.empty()) {
        if (!socket.readyToRead(chrono::seconds(30)))
            throw TimeoutException("Timeout reading next chunk");
        if (socket.readLine(chunkSizeStr) > 0)
            chunkSizeStr = trim(chunkSizeStr);
    }

    errno = 0;
    auto chunkSize = (size_t) strtol(chunkSizeStr.c_str(), nullptr, 16);
    if (errno != 0)
        throw Exception("Strange chunk size: '" + chunkSizeStr + "'");

    if (chunkSize == 0)
        return 0; // Last chunk

    return readAndAppend(socket, m_output, chunkSize);
}

bool HttpReader::readData()
{
    int readBytes = 0;
    while (m_socket.readyToRead(chrono::seconds(10))) {
        size_t bytesToRead;
        if (m_contentLength > 0) {
            bytesToRead = m_contentLength - m_contentReceivedLength;
            if (bytesToRead == 0)
                return true;
        } else
            bytesToRead = m_socket.socketBytes();

        if (!m_contentIsChunked) {
            readBytes = (int) readAndAppend(m_socket, m_output, bytesToRead);
            m_contentReceivedLength += readBytes;
            if (m_contentLength > 0 && m_contentReceivedLength >= m_contentLength) // No more data
                return true;
        } else {
            size_t chunkSize = readChunk(m_socket, m_output);
            m_contentReceivedLength += chunkSize;
            return (chunkSize == 0); // 0 means last chunk
        }
        readBytes = (int) m_socket.socketBytes();
        if (readBytes == 0 && m_output.bytes() > 13) {
            size_t tailOffset = m_output.bytes() - 13;
			String tail(m_output.c_str() + tailOffset);
            if (tail.toLowerCase().find("</html>") != string::npos)
				break;
            if (!m_socket.readyToRead(chrono::seconds(30)))
                throw TimeoutException("Read timeout");
        }
    }
    return true;
}

void HttpReader::read()
{
    lock_guard<mutex> lock(m_mutex);

    if (m_readerState == READY) {
        if (!readHttpHeaders())
            return;
        m_readerState = READING_DATA;
    }

    if (m_readerState == READING_DATA && !readData())
        return;

    auto itor = m_httpHeaders.find("content-encoding");
    if (itor != m_httpHeaders.end() && itor->second == "gzip") {
#if HAVE_ZLIB
        Buffer unzipBuffer;
        ZLib::decompress(unzipBuffer, m_output);
        m_output = move(unzipBuffer);
#else
        throw Exception("Content-Encoding is 'gzip', but zlib support is not enabled in SPTK");
#endif
    }

    itor = m_httpHeaders.find("Connection");
    if (itor != m_httpHeaders.end() && itor->second == "close")
        m_socket.close();

    if (m_statusCode >= 400 && m_statusText.empty()) {
        if (m_statusCode >= 500)
            m_statusText = "Unknown server error";
        else
            m_statusText = "Unknown client error";
    }

    m_readerState = COMPLETED;
}

HttpHeaders& HttpReader::getHttpHeaders()
{
    lock_guard<mutex> lock(m_mutex);
    return m_httpHeaders;
}

HttpReader::ReaderState HttpReader::getReaderState() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_readerState;
}

int HttpReader::getStatusCode() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_statusCode;
}

const String& HttpReader::getStatusText() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_statusText;
}

String HttpReader::httpHeader(const String& headerName) const
{
    lock_guard<mutex> lock(m_mutex);

    auto itor = m_httpHeaders.find(lowerCase(headerName));
    if (itor == m_httpHeaders.end())
        return "";
    return itor->second;
}

int HttpReader::readAll(std::chrono::milliseconds timeout)
{
    while (getReaderState() < HttpReader::COMPLETED) {
        if (!m_socket.readyToRead(timeout)) {
            m_socket.close();
            throw Exception("Response read timeout");
        }

        read();
    }

    return getStatusCode();
}

void HttpReader::reset()
{
    m_readerState = READY;
    m_statusText = "";
    m_statusCode = 0;
    m_contentLength = 0;
    m_contentReceivedLength = 0;
    m_contentIsChunked = false;
    m_httpHeaders.clear();
    m_read_buffer.reset(1024);
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
