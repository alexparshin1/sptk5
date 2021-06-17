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

#include <sptk5/cthreads>
#include <sptk5/net/HttpConnect.h>

#include <sptk5/ZLib.h>
#include <sptk5/md5.h>
#include <sptk5/Brotli.h>

using namespace std;
using namespace sptk;

HttpConnect::HttpConnect(TCPSocket& socket)
: m_socket(socket)
{
}

#define RSP_BLOCK_SIZE (1024*64)

String HttpConnect::responseHeader(const String& headerName) const
{
    if (m_reader)
        return m_reader->httpHeader(headerName);
    return "";
}

int HttpConnect::getResponse(Buffer& output, chrono::milliseconds readTimeout)
{
    m_reader = make_shared<HttpReader>(m_socket, output, HttpReader::ReadMode::RESPONSE);
    while (m_reader->getReaderState() < HttpReader::State::COMPLETED) {
        if (!m_socket.readyToRead(readTimeout)) {
            m_socket.close();
            throw Exception("Response read timeout");
        }

        m_reader->read();
    }

    return m_reader->getStatusCode();
}

void HttpConnect::sendCommand(const String& cmd)
{
    if (!m_socket.active())
        throw Exception("Socket isn't open");

    if (!m_socket.readyToWrite(chrono::seconds(30)))
        throw Exception("Server is busy");

    m_socket.write((const uint8_t*)cmd.c_str(), (uint32_t) cmd.length());
}

void HttpConnect::sendCommand(const Buffer& cmd)
{
    if (!m_socket.active())
        throw Exception("Socket isn't open");

    m_socket.write((const uint8_t*)cmd.c_str(), (uint32_t) cmd.bytes());
}

Strings HttpConnect::makeHeaders(const String& httpCommand, const String& pageName, const HttpParams& requestParameters,
                                 const Authorization* authorization) const
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

    for (const auto& itor: m_requestHeaders)
        headers.push_back(itor.first + ": " + itor.second);

    if (authorization && !authorization->method().empty())
        headers.push_back("Authorization: " + authorization->method() + " " + authorization->value());

    return headers;
}

int HttpConnect::cmd_get(const String& pageName, const HttpParams& requestParameters, Buffer& output,
                         const Authorization* authorization, chrono::milliseconds timeout)
{
    Strings headers = makeHeaders("GET", pageName, requestParameters, authorization);

    string command = headers.join("\r\n") + "\r\n\r\n";
    sendCommand(command);

    return getResponse(output, timeout);
}

static bool compressPostData(const sptk::Strings& possibleContentEncodings, Strings& headers, const Buffer& postData, Buffer& compressedData)
{
    static const sptk::Strings& availableContentEncodings {
#if HAVE_BROTLI
            "br",
#endif
#if HAVE_ZLIB
            "gzip",
#endif
    };

    Strings encodings;
    for (auto& contentEncoding: availableContentEncodings) {
        if (possibleContentEncodings.indexOf(contentEncoding) != -1) {
            encodings.push_back(contentEncoding);
        }
    }

    String usedEncoding;
    for (const auto& contentEncoding: encodings) {
#if HAVE_BROTLI
        if (contentEncoding == "br") {
            Brotli::compress(compressedData, postData);
            usedEncoding = contentEncoding;
        }
#endif
#if HAVE_ZLIB
        if (contentEncoding == "gzip") {
            ZLib::compress(compressedData, postData);
            usedEncoding = contentEncoding;
        }
#endif
        if (!usedEncoding.empty())
            break;
    }

    if (!usedEncoding.empty() && compressedData.length() < postData.length()) {
        headers.push_back("Content-Encoding: " + usedEncoding);
        return true;
    }

    return false;
}

int HttpConnect::cmd_post(const String& pageName, const HttpParams& parameters, const Buffer& postData, Buffer& output,
                          const sptk::Strings& possibleContentEncodings, const Authorization* authorization,
                          chrono::milliseconds timeout)
{
    Strings headers = makeHeaders("POST", pageName, parameters, authorization);

    const Buffer* data = &postData;

    Buffer compressBuffer;
    if (!possibleContentEncodings.empty()
        && compressPostData(possibleContentEncodings, headers, postData, compressBuffer))
    {
        data = &compressBuffer;
    }

    headers.push_back("Content-Length: " + int2string((uint32_t) data->bytes()));

    Buffer command(headers.join("\r\n") + "\r\n\r\n");
    command.append(*data);

    sendCommand(command);

    return getResponse(output, timeout);
}

int HttpConnect::cmd_put(const sptk::String& pageName, const HttpParams& requestParameters, const Buffer& putData,
                         Buffer& output, const Authorization* authorization, chrono::milliseconds timeout)
{
    Strings headers = makeHeaders("PUT", pageName, requestParameters, authorization);

#if HAVE_ZLIB
    headers.push_back("Accept-Encoding: gzip");
#endif

    if (!putData.empty())
        headers.push_back("Content-Length: " + int2string((uint32_t) putData.bytes()));

    string command = headers.join("\r\n") + "\r\n\r\n";

    if (!putData.empty())
        command += putData.data();

    sendCommand(command);

    return getResponse(output, timeout);
}

int HttpConnect::cmd_delete(const sptk::String& pageName, const HttpParams& requestParameters, Buffer& output,
                            const Authorization* authorization, chrono::milliseconds timeout)
{
    Strings headers = makeHeaders("DELETE", pageName, requestParameters, authorization);
    string  command = headers.join("\r\n") + "\r\n\r\n";

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

HttpConnect::Authorization::Authorization(const String& method, const String& username, const String& password, const String& jwtToken)
: m_method(method),
  m_value(method == "basic" ? md5(username + ":" + password) : jwtToken)
{}

#if USE_GTEST

TEST(SPTK_HttpConnect, get)
{
    Host google("www.google.com:80");

    auto* socket = new TCPSocket;

    ASSERT_NO_THROW(socket->open(google));
    ASSERT_TRUE(socket->active());

    HttpConnect http(*socket);
    Buffer      output;

    int statusCode;
    try {
        statusCode = http.cmd_get("/", HttpParams(), output);
    }
    catch (const Exception& e) {
        FAIL() << e.what();
    }
    EXPECT_EQ(200, statusCode);
    EXPECT_STREQ("OK", http.statusText().c_str());

    String data(output.c_str(), output.bytes());
    EXPECT_TRUE(data.toLowerCase().find("</html>") != string::npos);

	delete socket;
}

#endif
