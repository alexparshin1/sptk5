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

#include <sptk5/sptk.h>
#include <sptk5/net/TCPSocket.h>
#include <sptk5/ZLib.h>
#include <thread>
#include "sptk5/net/HttpReader.h"

using namespace std;
using namespace sptk;

HttpReader::HttpReader(Buffer& output)
: m_readerState(READY),
  m_statusCode(0),
  m_contentLength(0),
  m_contentReceivedLength(0),
  m_currentChunkSize(0),
  m_contentIsChunked(false),
  m_matchProtocolAndResponseCode("^(HTTP/1.\\d)\\s+(\\d+)\\s+(\\S.*)?\r"),
  m_output(output)
{
    output.reset(128);
}

bool HttpReader::readStatus(TCPSocket& socket)
{
    string status;
    while (socket.readLine(status) < 3) {
        if (status.empty())
            return false;
        socket.readyToRead(chrono::seconds(1));
    }

    Strings matches;
    if (!m_matchProtocolAndResponseCode.m(status, matches)) {
        m_readerState = READ_ERROR;
        throw Exception("Broken HTTP version header: [" + status + "]");
    }
    m_statusCode = string2int(matches[1]);
    if (matches.size() > 2)
        m_statusText = matches[2];

    return true;
}

bool HttpReader::readHeaders(TCPSocket& socket)
{
    /// Reading HTTP headers
    Strings matches;
    for (;;) {
        string header;

        socket.readLine(header);

        if (header.empty())
            return false;

        size_t pos = header.find(':');
        if (pos != string::npos) {
            m_responseHeaders[lowerCase(header.substr(0, pos))] = trim(header.substr(pos + 1));
            continue;
        }

        if (header[0] == '\r')
            break;
    }

    m_contentLength = 0;
    m_contentIsChunked = false;

    auto itor = m_responseHeaders.find("content-length");
    if (itor != m_responseHeaders.end())
        m_contentLength = (size_t) string2int(itor->second);

    itor = m_responseHeaders.find("transfer-encoding");
    if (itor != m_responseHeaders.end())
        m_contentIsChunked = itor->second.find("chunked") != string::npos;

    m_contentReceivedLength = 0;
    m_currentChunkSize = 0;

    return true;
}

bool HttpReader::readData(TCPSocket& socket)
{
    if (!socket.readyToRead(chrono::seconds(10)))
        throw TimeoutException("Connection timeout");
    auto readBytes = (int) socket.socketBytes();
    if (readBytes == 0) {
        if (m_contentLength != 0)
            throw Exception("Server closed connection");
        return true; // Server closed connection
    }

    while (socket.readyToRead(chrono::seconds(10))) {
        size_t bytesToRead;
        if (m_contentLength > 0) {
            bytesToRead = m_contentLength - m_contentReceivedLength;
            if (bytesToRead == 0)
                return true;
        } else {
            bytesToRead = socket.socketBytes();
        }

        if (!m_contentIsChunked) {
            if (readBytes > (int) bytesToRead) // 0 bytes case is a workaround for OpenSSL
                readBytes = (int) bytesToRead;
            if (!socket.readyToRead(chrono::seconds(30)))
                throw TimeoutException("Read timeout");
            readBytes = (int) socket.read(m_read_buffer, (size_t) readBytes);
            if (readBytes == 0) // 0 bytes case is a workaround for OpenSSL
                readBytes = (int) socket.read(m_read_buffer, (size_t) readBytes);
            m_output.append(m_read_buffer);
            m_contentReceivedLength += readBytes;
            if (m_contentLength > 0 && m_contentReceivedLength >= m_contentLength) // No more data
                return true;
        } else {
            if (m_currentChunkSize == 0) {
                // Starting next chunk
                String chunkSizeStr;
                while (chunkSizeStr.empty()) {
                    if (socket.readLine(chunkSizeStr) == 0) {
                        this_thread::sleep_for(chrono::milliseconds(10));
                        continue;
                    }
                    chunkSizeStr = trim(chunkSizeStr);
                }

                errno = 0;
                m_currentChunkSize = (size_t) strtol(chunkSizeStr.c_str(), nullptr, 16);
                if (errno != 0)
                    throw Exception("Strange chunk size: '" + chunkSizeStr + "'");

                if (m_currentChunkSize == 0)
                    return true; // Last chunk
            }

            m_output.checkSize(m_output.bytes() + m_currentChunkSize + 16384);
            char* appendPtr = m_output.data() + m_output.bytes();
            if (!socket.readyToRead(chrono::seconds(30)))
                throw TimeoutException("Read timeout");
            readBytes = (int) socket.read(appendPtr, m_currentChunkSize, nullptr);
            if (readBytes > 0) {
                m_contentReceivedLength += readBytes;
                appendPtr[readBytes] = 0;
                m_currentChunkSize = 0;
                m_output.bytes(m_output.bytes() + readBytes);
                return false; // Not the last chunk
            }
        }
        readBytes = (int) socket.socketBytes();
        if (readBytes == 0 && m_output.bytes() > 13) {
            size_t tailOffset = m_output.bytes() - 13;
			String tail(m_output.c_str() + tailOffset);
            if (tail.toLowerCase().find("</html>") != string::npos)
				break;
            if (!socket.readyToRead(chrono::seconds(30)))
                throw TimeoutException("Read timeout");
            readBytes = (int) socket.socketBytes();
        }
    }
    return true;
}

void HttpReader::read(TCPSocket& socket)
{
    lock_guard<mutex> lock(m_mutex);

    if (m_readerState == READY) {
        m_output.bytes(0);
        m_responseHeaders.clear();
        m_statusCode = 0;
        m_statusText = "";
        if (!readStatus(socket))
            throw Exception("Can't read server response");
        m_readerState = READING_HEADERS;
        m_contentReceivedLength = 0;
    }

    if (m_readerState == READING_HEADERS) {
        if (!readHeaders(socket))
            return;
        m_readerState = READING_DATA;
    }

    if (m_readerState == READING_DATA) {
        if (!readData(socket))
            return;
    }

    auto itor = m_responseHeaders.find("content-encoding");
    if (itor != m_responseHeaders.end() && itor->second == "gzip") {
#if HAVE_ZLIB
        Buffer unzipBuffer;
        ZLib::decompress(unzipBuffer, m_output);
        *(Buffer*)this = move(unzipBuffer);
#else
        throw Exception("Content-Encoding is 'gzip', but zlib support is not enabled in SPTK");
#endif
    }

    itor = m_responseHeaders.find("Connection");
    if (itor != m_responseHeaders.end() && itor->second == "close")
        socket.close();

    if (m_statusCode >= 400) {
        if (m_statusText.empty()) {
            if (m_statusCode >= 500)
                m_statusText = "Unknown server error";
            else
                m_statusText = "Unknown client error";
        }
    }

    m_readerState = COMPLETED;
}

const HttpHeaders& HttpReader::getResponseHeaders() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_responseHeaders;
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

String HttpReader::responseHeader(const String& headerName) const
{
    lock_guard<mutex> lock(m_mutex);

    auto itor = m_responseHeaders.find(lowerCase(headerName));
    if (itor == m_responseHeaders.end())
        return "";
    return itor->second;
}
