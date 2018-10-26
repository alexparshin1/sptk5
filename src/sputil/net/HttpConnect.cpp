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
    //m_requestHeaders["Connection"] = "close";
}

#define RSP_BLOCK_SIZE (1024*64)

String HttpConnect::responseHeader(const String& headerName) const
{
    if (m_reader)
        return m_reader->responseHeader(headerName);
    return "";
}

int HttpConnect::getResponse(Buffer& output, chrono::milliseconds readTimeout)
{
    m_reader = make_unique<HttpReader>(output);
    while (m_reader->getReaderState() < HttpReader::COMPLETED) {
        if (!m_socket.readyToRead(readTimeout)) {
            m_socket.close();
            throw Exception("Response read timeout");
        }

        m_reader->read(m_socket);
    }

    return m_reader->getStatusCode();
}

void HttpConnect::sendCommand(const String& cmd)
{
    if (!m_socket.active())
        throw Exception("Socket isn't open");

    if (!m_socket.readyToWrite(chrono::seconds(30)))
        throw Exception("Server is busy");

    m_socket.write(cmd.c_str(), (uint32_t) cmd.length());
}

void HttpConnect::sendCommand(const Buffer& cmd)
{
    if (!m_socket.active())
        throw Exception("Socket isn't open");

    m_socket.write(cmd.c_str(), (uint32_t) cmd.bytes());
}

Strings HttpConnect::makeHeaders(const String& httpCommand, const String& pageName, const HttpParams& requestParameters)
{
    Strings headers;

    string command(httpCommand + " " + pageName);

    if (!requestParameters.empty()) {
        Buffer buffer;
        requestParameters.encode(buffer);
        command += string("?") + buffer.data();
    }

    headers.push_back(command + " HTTP/1.1");
    headers.push_back("HOST: " + m_socket.host().toString(false));

    for (auto& itor: m_requestHeaders)
        headers.push_back(itor.first + ": " + itor.second);

    return headers;
}

int HttpConnect::cmd_get(const String& pageName, const HttpParams& requestParameters, Buffer& output,
                         chrono::milliseconds timeout)
{
    Strings headers = makeHeaders("GET", pageName, requestParameters);

    string command = headers.asString("\r\n") + "\r\n\r\n";
    sendCommand(command);

    return getResponse(output, timeout);
}

int HttpConnect::cmd_post(const sptk::String& pageName, const HttpParams& parameters, const Buffer& postData, bool gzipContent,
                          Buffer& output, std::chrono::milliseconds timeout)
{
    Strings headers = makeHeaders("POST", pageName, parameters);

#if HAVE_ZLIB
    headers.push_back("Accept-Encoding: gzip");
#endif

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

    return getResponse(output, timeout);
}

int HttpConnect::cmd_put(const sptk::String& pageName, const HttpParams& requestParameters, const Buffer& putData,
                         Buffer& output, std::chrono::milliseconds timeout)
{
    Strings headers = makeHeaders("PUT", pageName, requestParameters);

#if HAVE_ZLIB
    headers.push_back("Accept-Encoding: gzip");
#endif

    if (!putData.empty())
        headers.push_back("Content-Length: " + int2string((uint32_t) putData.bytes()));

    string command = headers.asString("\r\n") + "\r\n\r\n";

    if (!putData.empty())
        command += putData.data();

    sendCommand(command);

    return getResponse(output, timeout);
}

int HttpConnect::cmd_delete(const sptk::String& pageName, const HttpParams& requestParameters, Buffer& output,
                            std::chrono::milliseconds timeout)
{
    Strings headers = makeHeaders("DELETE", pageName, requestParameters);
    string  command = headers.asString("\r\n") + "\r\n\r\n";

    sendCommand(command);

    return getResponse(output, timeout);
}

int HttpConnect::statusCode() const
{
    return m_reader->getStatusCode();
}

String HttpConnect::statusText() const
{
    return m_reader->getStatusText();
}

#if USE_GTEST
#include <gtest/gtest.h>

TEST(SPTK_HttpConnect, get)
{
    Host google("www.google.com:80");

    auto socket = new TCPSocket;

    ASSERT_NO_THROW(socket->open(google));
    ASSERT_TRUE(socket->active());

    HttpConnect http(*socket);
    Buffer      output;

    try {
        http.cmd_get("/", HttpParams(), output);
    }
    catch (const exception& e) {
        FAIL() << e.what();
    }
    EXPECT_EQ(200, http.statusCode());
    EXPECT_STREQ("OK", http.statusText().c_str());

    String data(output.c_str(), output.bytes());
    EXPECT_TRUE(data.toLowerCase().find("</html>") != string::npos);
    //cout << data.c_str() << endl;
	delete socket;
}

#endif
