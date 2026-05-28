/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Brotli.h>
#include <sptk5/ZLib.h>
#include <sptk5/md5.h>
#include <sptk5/net/HttpConnect.h>

#include "sptk5/Base64.h"

using namespace std;
using namespace sptk;

HttpConnect::HttpConnect(shared_ptr<TCPSocket> socket)
    : m_socket(std::move(socket))
{
}

String HttpConnect::responseHeader(const String& headerName) const
{
    if (m_reader)
    {
        return m_reader->httpHeader(headerName);
    }
    return "";
}

int HttpConnect::getResponse(Buffer& output, const chrono::milliseconds& readTimeout)
{
    m_reader = make_shared<HttpReader>(m_socket, output, HttpReader::ReadMode::RESPONSE);
    const auto deadline = chrono::steady_clock::now() + readTimeout;
    m_reader->setReadDeadline(deadline);
    while (m_reader->getReaderState() < HttpReader::State::COMPLETED)
    {
        const auto now = chrono::steady_clock::now();
        if (now >= deadline)
        {
            m_socket->close();
            throw Exception("Response read timeout");
        }

        const auto remaining = chrono::duration_cast<chrono::milliseconds>(deadline - now);
        if (!m_socket->readyToRead(remaining))
        {
            m_socket->close();
            throw Exception("Response read timeout");
        }

        m_reader->readStream();
    }

    return m_reader->getStatusCode();
}

void HttpConnect::sendCommand(const String& cmd) const
{
    if (!m_socket->active())
    {
        throw Exception("Socket isn't open");
    }

    if (const chrono::seconds readTimeout(30);
        !m_socket->readyToWrite(readTimeout))
    {
        throw Exception("Server is busy");
    }

    m_socket->write(bit_cast<const uint8_t*>(cmd.c_str()), cmd.length());
}

void HttpConnect::sendCommand(const Buffer& cmd) const
{
    if (!m_socket->active())
    {
        throw Exception("Socket isn't open");
    }

    if (const chrono::seconds readTimeout(30);
        !m_socket->readyToWrite(readTimeout))
    {
        throw Exception("Server is busy");
    }

    m_socket->write(cmd.data(), cmd.bytes());
}

Strings HttpConnect::makeHeaders(const String& httpCommand, const String& pageName, const HttpParams& requestParameters,
                                 const Authorization* authorization) const
{
    Strings headers;

    string command(httpCommand + " " + pageName);

    if (!requestParameters.empty())
    {
        Buffer buffer;
        requestParameters.encode(buffer);
        command += string("?") + buffer.c_str();
    }

    headers.push_back(command + " HTTP/1.1");
    headers.push_back("HOST: " + m_socket->host().toString(false));

    for (const auto& [name, value]: m_requestHeaders)
    {
        headers.push_back(name + ": " + value);
    }

    if (authorization && !authorization->method().empty())
    {
        headers.push_back("Authorization: " + authorization->method() + " " + authorization->value());
    }

    return headers;
}

const HttpHeaders& HttpConnect::responseHeaders() const
{
    if (!m_reader)
    {
        static const HttpHeaders emptyHeaders;
        return emptyHeaders;
    }
    return m_reader->getHttpHeaders();
}

int HttpConnect::cmd_get(const String& pageName, const HttpParams& requestParameters, Buffer& output,
                         const Authorization* authorization, const chrono::milliseconds& timeout)
{
    const Strings headers = makeHeaders("GET", pageName, requestParameters, authorization);
    const String  command = headers.join("\r\n") + "\r\n\r\n";

    sendCommand(command);

    return getResponse(output, timeout);
}

namespace {
bool compressPostData(const Strings& possibleContentEncodings, Strings& headers, const Buffer& postData,
                      Buffer& compressedData)
{
    static const Strings& availableContentEncodings {
#ifdef HAVE_BROTLI
        "br",
#endif
#ifdef HAVE_ZLIB
        "gzip",
#endif
    };

    Strings encodings;
    for (const auto& contentEncoding: availableContentEncodings)
    {
        if (possibleContentEncodings.indexOf(contentEncoding) != -1)
        {
            encodings.push_back(contentEncoding);
        }
    }

    String usedEncoding;
    for (const auto& contentEncoding: encodings)
    {
#ifdef HAVE_BROTLI
        if (contentEncoding == "br")
        {
            Brotli::compress(compressedData, postData);
            usedEncoding = contentEncoding;
        }
#endif
#ifdef HAVE_ZLIB
        if (contentEncoding == "gzip")
        {
            ZLib::compress(compressedData, postData);
            usedEncoding = contentEncoding;
        }
#endif
        if (!usedEncoding.empty())
        {
            break;
        }
    }

    if (!usedEncoding.empty() && compressedData.size() < postData.size())
    {
        headers.push_back("Content-Encoding: " + usedEncoding);
        return true;
    }

    return false;
}
} // namespace

int HttpConnect::cmd_post(const String& pageName, const HttpParams& parameters, const Buffer& postData, Buffer& output,
                          const Strings& possibleContentEncodings, const Authorization* authorization,
                          const chrono::milliseconds& timeout)
{
    Strings headers = makeHeaders("POST", pageName, parameters, authorization);

    auto   compressed = false;
    auto   contentLength = postData.bytes();
    Buffer compressBuffer;
    if (!possibleContentEncodings.empty() && compressPostData(possibleContentEncodings, headers, postData, compressBuffer))
    {
        compressed = true;
        contentLength = compressBuffer.bytes();
    }

    headers.push_back("Content-Length: " + to_string(contentLength));

    Buffer command(headers.join("\r\n") + "\r\n\r\n");

    if (compressed)
    {
        command.append(compressBuffer);
    }
    else
    {
        command.append(postData);
    }

    sendCommand(command);

    return getResponse(output, timeout);
}

int HttpConnect::cmd_put(const String& pageName, const HttpParams& requestParameters, const Buffer& putData,
                         Buffer& output, const Authorization* authorization, const chrono::milliseconds& timeout)
{
    Strings headers = makeHeaders("PUT", pageName, requestParameters, authorization);

#ifdef HAVE_ZLIB
    headers.push_back("Accept-Encoding: gzip");
#endif

    headers.push_back(format("Content-Length: {}", putData.bytes()));

    Buffer command(headers.join("\r\n") + "\r\n\r\n");

    if (!putData.empty())
    {
        command.append(putData);
    }

    sendCommand(command);

    return getResponse(output, timeout);
}

int HttpConnect::cmd_delete(const String& pageName, const HttpParams& requestParameters, Buffer& output,
                            const Authorization* authorization, const chrono::milliseconds& timeout)
{
    const Strings headers = makeHeaders("DELETE", pageName, requestParameters, authorization);
    const String  command = headers.join("\r\n") + "\r\n\r\n";

    sendCommand(command);

    return getResponse(output, timeout);
}

int HttpConnect::statusCode() const
{
    if (m_reader)
    {
        return m_reader->getStatusCode();
    }
    return 0;
}

String HttpConnect::statusText() const
{
    if (m_reader)
    {
        return m_reader->getStatusText();
    }
    return "";
}

HttpConnect::Authorization::Authorization(const String& method, const String& username, const String& password,
                                          const String& jwtToken)
    : m_method(method)
{
    if (method.empty())
    {
        return;
    }

    if (method.toLowerCase() == "basic")
    {
        Buffer value(username + ":" + password);
        Buffer encodedValue;
        Base64::encode(encodedValue, value);
        m_value = encodedValue.c_str();
    }
    else if (method.toLowerCase() == "bearer")
    {
        m_value = jwtToken;
    }
    else
    {
        throw Exception("Unsupported authorization method: " + method);
    }
}
