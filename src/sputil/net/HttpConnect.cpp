/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       HttpConnect.cpp - description                          ║
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

#include <sptk5/sptk.h>
#include <sptk5/net/HttpConnect.h>
#include <sptk5/ZLib.h>

using namespace std;
using namespace sptk;

HttpConnect::HttpConnect(TCPSocket& socket)
: m_socket(socket), m_matchProtocolAndResponseCode("^(HTTP/1.\\d)\\s+(\\d+)\\s*(\\S.*)\r")
{
    //m_requestHeaders["Content-Type"] = "application/x-www-form-urlencoded";
    m_requestHeaders["Connection"] = "close";
}

int HttpConnect::readHeaders(std::chrono::milliseconds timeout, String& httpStatus)
{
    m_responseHeaders.clear();

    if (!m_socket.readyToRead(timeout)) {
        m_socket.close();
        throw Exception("Response timeout");
    }

    /// Reading HTTP headers
    Strings matches;
    bool firstRow = true;
    int rc = 0;
    for (;;) {
        string header;

        m_socket.readLine(header);

        if (header.empty())
            throw Exception("Invalid HTTP response");

        if (firstRow) {
            if (header.length() <= 2)
                continue;
            if (!m_matchProtocolAndResponseCode.m(header, matches))
                throw Exception("Broken HTTP version header");
            rc = string2int(matches[1]);
            httpStatus = matches[2];
            firstRow = false;
            continue;
        }
        size_t pos = header.find(':');
        if (pos != string::npos) {
            string headerName = header.substr(0, pos);
            string headerValue = trim(header.substr(pos + 1));
            m_responseHeaders[headerName] = headerValue;
            continue;
        }

        if (header[0] == '\r')
            break;
    }

    if (m_requestHeaders.empty())
        throw Exception("Can't detect HTTP headers");

    return rc;
}

#define RSP_BLOCK_SIZE (1024*64)

string HttpConnect::responseHeader(const string& headerName) const
{
    auto itor = m_responseHeaders.find(headerName);
    if (itor == m_responseHeaders.end())
        return "";
    return itor->second;
}

int HttpConnect::getResponse(std::chrono::milliseconds readTimeout)
{
    Buffer read_buffer(RSP_BLOCK_SIZE);

    m_readBuffer.reset();
    m_responseHeaders.clear();

    String httpStatus;
    int rc = readHeaders(readTimeout, httpStatus);

    string contentLengthStr = responseHeader("Content-Length");
    if (contentLengthStr.empty()) {
        if (rc == 204 || rc == 304)
            contentLengthStr = "0";
        //else
            //contentLengthStr = "-1";
    }
    auto contentLength = (size_t) string2int(contentLengthStr);
    if (contentLengthStr != "0") {
        bool chunked = responseHeader("Transfer-Encoding").find("chunked") != string::npos;

        size_t bytes;
        size_t bytesToRead = contentLength;
        if (!chunked) {
            size_t totalBytes = 0;

            for (;;) {

                if (!m_socket.readyToRead(readTimeout)) {
                    m_socket.close();
                    throw Exception("Response read timeout");
                }

                if (contentLength != 0) {
                    bytes = m_socket.socketBytes();
                    if (bytes == 0 || bytes > bytesToRead) // 0 bytes case is a workaround for OpenSSL
                        bytes = bytesToRead;
                    bytes = (int) m_socket.read(read_buffer, (size_t) bytes);
                    bytesToRead -= bytes;
                } else
                    bytes = (int) m_socket.read(read_buffer, 64*1024);

                if (bytes == 0) // No more data
                    break;

                m_readBuffer.append(read_buffer);
                totalBytes += bytes;

                if (contentLength != 0 && totalBytes >= contentLength)
                    break;
            }
        } else {
            string chunkSizeStr;

            for (;;) {
                if (!m_socket.readyToRead(readTimeout)) {
                    m_socket.close();
                    throw Exception("Response read timeout");
                }
                m_socket.readLine(chunkSizeStr);
                chunkSizeStr = trim(chunkSizeStr);

                if (chunkSizeStr.empty() || chunkSizeStr == "0")
                    m_socket.readLine(chunkSizeStr);

                auto chunkSize = (size_t) strtol(chunkSizeStr.c_str(), nullptr, 16);

                if (chunkSize == 0)
                    break;

                read_buffer.checkSize(chunkSize);
                bytes = (int) m_socket.read(read_buffer, chunkSize, nullptr);

                if (bytes > 0) {
                    read_buffer.data() [bytes] = 0;
                    m_readBuffer.append(read_buffer);
                }
            }
        }

        auto itor = m_responseHeaders.find("Content-Encoding");
        if (itor != m_responseHeaders.end() && itor->second == "gzip") {
#if HAVE_ZLIB
            Buffer unzipBuffer;
            ZLib::decompress(unzipBuffer, m_readBuffer);
            m_readBuffer = move(unzipBuffer);
#else
            throw Exception("Content-Encoding is 'gzip', but zlib support is not enabled in SPTK");
#endif
        }
    }

    auto itor = m_responseHeaders.find("Connection");
    if (itor != m_responseHeaders.end() && itor->second == "close")
        m_socket.close();

    if (rc >= 400)
        throw Exception(httpStatus);

    return rc;
}

void HttpConnect::sendCommand(const string& cmd)
{
    if (!m_socket.active())
        throw Exception("Socket isn't open");

    m_socket.write(cmd.c_str(), (uint32_t) cmd.length());
}

void HttpConnect::sendCommand(const Buffer& cmd)
{
    if (!m_socket.active())
        throw Exception("Socket isn't open");

    m_socket.write(cmd.c_str(), (uint32_t) cmd.bytes());
}

Strings HttpConnect::makeHeaders(const string& httpCommand, const string& pageName, const HttpParams& requestParameters)
{
    Strings headers;

    string command(httpCommand + " " + pageName);

    if (!requestParameters.empty()) {
        Buffer buffer;
        requestParameters.encode(buffer);
        command += string("?") + buffer.data();
    }

    headers.push_back(command + " HTTP/1.1");
    headers.push_back("HOST: " + m_socket.host().toString());
    headers.push_back("User-Agent: SPTK Connect 5.x");

    for (auto& itor: m_requestHeaders)
        headers.push_back(itor.first + ": " + itor.second);

    return headers;
}

int HttpConnect::cmd_get(const string& pageName, const HttpParams& requestParameters, chrono::milliseconds timeout)
{
    m_readBuffer.checkSize(1024);

    Strings headers = makeHeaders("GET", pageName, requestParameters);
    //command += "Accept: */*\n";

    string command = headers.asString("\r\n") + "\r\n\r\n";
    //cout << command;
    sendCommand(command);

    return getResponse(timeout);
}

int HttpConnect::cmd_post(const string& pageName, const HttpParams& parameters, const Buffer& postData,
                          bool gzipContent, chrono::milliseconds timeout)
{
    Strings headers = makeHeaders("POST", pageName, parameters);
    headers.push_back("Accept-Encoding: gzip");

    const Buffer* data = &postData;
    Buffer compressedData;
    if (gzipContent) {
#if HAVE_ZLIB
        ZLib::compress(compressedData, postData);
        headers.push_back("Content-Encoding: gzip");
        data = &compressedData;
#else
		throw Exception("Content-Encoding is 'gzip', but zlib support is not enabled in SPTK");
#endif
	}
    headers.push_back("Content-Length: " + int2string((uint32_t) data->bytes()));

    Buffer command(headers.asString("\r\n") + "\r\n\r\n");
    command.append(*data);

    sendCommand(command);

    return getResponse(timeout);
}

int HttpConnect::cmd_put(const string& pageName, const HttpParams& requestParameters, const Buffer& putData,
                         chrono::milliseconds timeout)
{
    Strings headers = makeHeaders("PUT", pageName, requestParameters);
    headers.push_back("Accept-Encoding: gzip");

    if (!putData.empty())
        headers.push_back("Content-Length: " + int2string((uint32_t) putData.bytes()));

    string command = headers.asString("\r\n") + "\r\n\r\n";

    if (!putData.empty())
        command += putData.data();

    sendCommand(command);

    return getResponse(timeout);
}

int HttpConnect::cmd_delete(const string& pageName, const HttpParams& requestParameters, chrono::milliseconds timeout)
{
    Strings headers = makeHeaders("DELETE", pageName, requestParameters);
    string  command = headers.asString("\r\n") + "\r\n\r\n";

    sendCommand(command);

    return getResponse(timeout);
}
