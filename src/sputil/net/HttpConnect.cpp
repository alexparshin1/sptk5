/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       HttpConnect.cpp - description                          ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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
#include <sptk5/RegularExpression.h>
#include <sptk5/ZLib.h>

#include <sstream>
#include <iostream>

using namespace std;
using namespace sptk;

HttpConnect::HttpConnect(TCPSocket& socket)
: m_socket(socket)
{
    m_requestHeaders["Content-Type"] = "application/x-www-form-urlencoded";
    m_requestHeaders["Connection"] = "close";
}

HttpConnect::~HttpConnect()
{
    m_socket.close();
}

int HttpConnect::readHeaders(uint32_t timeoutMS)
{
    m_responseHeaders.clear();

    if (!m_socket.readyToRead(timeoutMS)) {
        m_socket.close();
        throw Exception("Response timeout");
    }

    RegularExpression matchProtocolAndResponseCode("^(HTTP/1.\\d)\\s+(\\d+)\\s*$");
    RegularExpression matchHeader("^([^:]+):\\s+(.*)\\r$");

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
            if (!matchProtocolAndResponseCode.m(header, matches))
                throw Exception("Broken HTTP version header");
            rc = string2int(matches[1]);
            firstRow = false;
            continue;
        }
        if (matchHeader.m(header, matches)) {
            m_responseHeaders[ matches[0] ] = matches[1];
            continue;
        }
        if (header[0] == '\r')
            break;
    }

    if (m_requestHeaders.empty())
        throw Exception("Can't detect HTTP headers");

    return rc;
}

#define RSP_BLOCK_SIZE 1024*64

string HttpConnect::responseHeader(string headerName) const
{
    auto itor = m_responseHeaders.find(headerName);
    if (itor == m_responseHeaders.end())
        return "";
    return itor->second;
}

int HttpConnect::getResponse(string resourceName, uint32_t readTimeout)
{
    Buffer read_buffer(RSP_BLOCK_SIZE);

    m_readBuffer.reset();

    int rc = readHeaders(readTimeout);
    if (rc >= 500)
        throw Exception("Server error: Error " + int2string(rc) + ", resource: " + resourceName);
    if (rc >= 400)
        throw Exception("Resource not found: Error " + int2string(rc) + ", resource: " + resourceName);

    int contentLength = string2int(responseHeader("Content-Length"));
    bool chunked = responseHeader("Transfer-Encoding").find("chunked") != string::npos;

    int bytes;
    int bytesToRead = contentLength;
    if (!chunked) {
        int totalBytes = 0;

        for (;;) {

            if (!m_socket.readyToRead(readTimeout)) {
                m_socket.close();
                throw Exception("Response read timeout");
            }

            if (contentLength) {
                bytes = m_socket.socketBytes();
                if (bytes == 0 || bytes > bytesToRead) // 0 bytes case is a workaround for OpenSSL
                    bytes = bytesToRead;
                bytes = (int) m_socket.read(read_buffer, bytes);
                bytesToRead -= bytes;
            } else
                bytes = (int) m_socket.read(read_buffer, 64*1024);

            if (bytes <= 0) // No more data
                break;

            m_readBuffer.append(read_buffer);
            totalBytes += bytes;

            if (contentLength && totalBytes >= contentLength)
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

            if (chunkSizeStr.empty())
                m_socket.readLine(chunkSizeStr);

            size_t chunkSize = (size_t) strtol(chunkSizeStr.c_str(), 0L, 16);

            if (chunkSize == 0)
                break;

            read_buffer.checkSize(chunkSize);
            bytes = (int) m_socket.read(read_buffer, chunkSize, NULL);

            if (bytes > 0) {
                read_buffer.data() [bytes] = 0;
                m_readBuffer.append(read_buffer);
            }
        }
    }

    if (m_responseHeaders["Content-Encoding"] == "gzip") {
        Buffer unzipBuffer;
        ZLib::decompress(unzipBuffer, m_readBuffer);
        m_readBuffer = move(unzipBuffer);
    }

    m_socket.close();

    return rc;
}

void HttpConnect::sendCommand(string cmd)
{
    //cmd += "\r\n";

    if (!m_socket.active())
        throw Exception("Socket isn't open");

    m_socket.write(cmd.c_str(), (uint32_t) cmd.length());
}

Strings HttpConnect::makeHeaders(string httpCommand, string pageName, const HttpParams& requestParameters)
{
    Strings headers;

    string command(httpCommand + " " + pageName);

    if (!requestParameters.empty()) {
        Buffer buffer;
        requestParameters.encode(buffer);
        command += string("?") + buffer.data();
    }

    headers.push_back(command + " HTTP/1.1");
    headers.push_back("HOST: " + m_socket.host() + ":" + int2string(m_socket.port()));

    for (auto& itor: m_requestHeaders)
        headers.push_back(itor.first + ": " + itor.second);

    return move(headers);
}

int HttpConnect::cmd_get(string pageName, const HttpParams& requestParameters, uint32_t timeoutMS)
{
    m_readBuffer.checkSize(1024);

    Strings headers = makeHeaders("GET", pageName, requestParameters);
    //command += "Accept: */*\n";

    string command = headers.asString("\r\n") + "\r\n\r\n";
    sendCommand(command);

    return getResponse(pageName, timeoutMS);
}

int HttpConnect::cmd_post(string pageName, const Buffer& postData, uint32_t timeoutMS)
{
    Strings headers = makeHeaders("POST", pageName, HttpParams());
    headers.push_back("Accept-Encoding: gzip");
    headers.push_back("Content-Length: " + int2string((uint32_t) postData.bytes()));

    string command = headers.asString("\r\n") + "\r\n\r\n";
    command += postData.data();
    sendCommand(command);

    return getResponse(pageName, timeoutMS);
}

int HttpConnect::cmd_put(string pageName, const HttpParams& requestParameters, const Buffer& putData, uint32_t timeoutMS)
{
    Strings headers = makeHeaders("PUT", pageName, requestParameters);
    headers.push_back("Accept-Encoding: gzip");

    if (!putData.empty())
        headers.push_back("Content-Length: " + int2string((uint32_t) putData.bytes()));

    string command = headers.asString("\r\n") + "\r\n\r\n";

    if (!putData.empty())
        command += putData.data();

    sendCommand(command);

    return getResponse(pageName, timeoutMS);
}

int HttpConnect::cmd_delete(string pageName, const HttpParams& requestParameters, uint32_t timeoutMS)
{
    Strings headers = makeHeaders("DELETE", pageName, requestParameters);

    string command = headers.asString("\r\n") + "\r\n\r\n";
    sendCommand(command);

    return getResponse(pageName, timeoutMS);
}
