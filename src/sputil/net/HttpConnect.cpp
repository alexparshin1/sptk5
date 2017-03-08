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

#include <iostream>
#include <src/sputil/core/ZLib.h>

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

#define RSP_BLOCK_SIZE 1024*64

void HttpConnect::getResponse(uint32_t readTimeout)
{
    Buffer read_buffer(RSP_BLOCK_SIZE);

    m_readBuffer.reset();

    Strings headers;

    int    bytes, contentLength = 0;
    string header;

    if (!m_socket.readyToRead(readTimeout)) {
        m_socket.close();
        throw Exception("Response timeout");
    }

    /// Reading HTTP headers
    for (;;) {
        m_socket.readLine(header);
        if (header.empty())
            throw Exception("Invalid HTTP response");
        size_t pos = header.find("\r");
        if (pos != string::npos)
            header.resize(pos);

        if (header.empty())
            break;

        headers.push_back(header);
    }

    m_responseHeaders.clear();
    cout << headers.join("\n") << endl << endl;

    if (headers.empty())
        throw Exception("Can't detect HTTP headers");

    /// Parsing HTTP headers
    if (headers[0].find("HTTP/1.") != 0)
        throw Exception("Broken HTTP version header");

    m_responseHeaders["HTTP version"] = headers[0];

    bool chunked = 0;

    for (unsigned i = 0; i < headers.size(); i++) {
        char* headerStr = (char*) headers[i].c_str();
        char* headerValue = strstr(headerStr,": ");

        if (!headerValue)
            continue;

        *headerValue = 0;
        headerValue += 2;
        m_responseHeaders[headerStr] = headerValue;

        if (strcmp(headerStr,"Transfer-Encoding") == 0) {
            chunked = strstr(headerValue,"chunked") != NULL;
            continue;
        } else if (strcmp(headerStr,"Content-Length") == 0) {
            contentLength = atoi(headerValue);
            continue;
        }
    }

    int bytesToRead = contentLength;
    if (!chunked) {
        int totalBytes = 0;

        for (;;) {

            if (contentLength) {
                if (!m_socket.readyToRead(readTimeout)) {
                    m_socket.close();
                    throw Exception("Response read timeout");
                }
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
            m_socket.readLine(chunkSizeStr);
            chunkSizeStr = trim(chunkSizeStr);

            if (chunkSizeStr.empty())
                m_socket.readLine(chunkSizeStr);

            size_t chunkSize = (size_t) strtol(chunkSizeStr.c_str(),0L,16);

            if (chunkSize == 0)
                break;

            read_buffer.checkSize(chunkSize);
            bytes = (int) m_socket.read(read_buffer.data(),chunkSize,0);

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
    m_readBuffer.saveToFile("/tmp/1.gz");
    
    m_socket.close();
}

void HttpConnect::sendCommand(string cmd)
{
    cmd += "\n";

    if (!m_socket.active())
        throw Exception("Socket isn't open");

    m_socket.write(cmd.c_str(), (uint32_t) cmd.length());
}

void HttpConnect::cmd_get(string pageName, const HttpParams& postData, uint32_t timeoutMS)
{
    m_readBuffer.checkSize(1024);

    Buffer buffer;
    postData.encode(buffer);

    string parameters(buffer.data());

    string command = "GET " + pageName;

    if (parameters.length())
        command += "?" + parameters;

    command += " HTTP/1.1\n";
    command += "User-Agent: SPTK HttpConnect\n";
    command += "Accept: */*\n";
    command += "Accept-Encoding: gzip\n";
    command += "Host: " + m_socket.host() + ":"+ int2string(m_socket.port()) + "\n";
    command += "Connection: Keep-Alive\n";

    Buffer buff;
    buff.append(command);

    sendCommand(command);

    getResponse(timeoutMS);
}

void HttpConnect::cmd_post(string pageName, const HttpParams& postData, uint32_t timeoutMS)
{
    Strings headers;

    headers.push_back("POST " + pageName + " HTTP/1.1");
    headers.push_back("HOST: " + m_socket.host());

    map<string,string>::iterator itor = m_requestHeaders.begin();
    map<string,string>::iterator iend = m_requestHeaders.end();

    for (; itor != iend; ++itor)
        headers.push_back(itor->first + ": " + itor->second);

    Buffer buffer;
    postData.encode(buffer);

    headers.push_back("Content-Length: " + int2string((uint32_t) buffer.bytes()));

    string command = headers.asString("\r\n") + "\r\n\r\n";

    command += buffer.data();
    command += "\r\n";
    sendCommand(command);

    getResponse(timeoutMS);
}
