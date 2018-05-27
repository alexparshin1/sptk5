/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       HttpConnect.cpp - description                          ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <sptk5/cthreads>
#include <sptk5/net/HttpConnect.h>

#include <sptk5/ZLib.h>

using namespace std;
using namespace sptk;

HttpConnect::HttpConnect(TCPSocket& socket)
: m_socket(socket)
{
    m_requestHeaders["Connection"] = "close";
}

#define RSP_BLOCK_SIZE (1024*64)

string HttpConnect::responseHeader(const string& headerName) const
{
    return m_reader.responseHeader(headerName);
}

int HttpConnect::getResponse(chrono::milliseconds readTimeout)
{
    Buffer read_buffer(RSP_BLOCK_SIZE);

    while (m_reader.getReaderState() < HttpReader::COMPLETED) {
        if (!m_socket.readyToRead(readTimeout)) {
            m_socket.close();
            throw Exception("Response read timeout");
        }

        m_reader.read(m_socket);
    }

    return m_reader.getStatusCode();
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

int HttpConnect::statusCode() const
{
    return m_reader.getStatusCode();
}

String HttpConnect::statusText() const
{
    return m_reader.getStatusText();
}

