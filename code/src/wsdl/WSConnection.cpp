/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include "sptk5/wsdl/WSConnection.h"

using namespace std;
using namespace sptk;

WSConnection::WSConnection(TCPServer& server, const sockaddr_in* connectionAddress, WSServices& services, LogEngine& logEngine, const Options& options)
    : ServerConnection(server, ServerConnection::Type::SSL, connectionAddress, "WSConnection")
    , m_services(services)
    , m_logger(logEngine, "(" + to_string(serial()) + ") ")
    , m_options(options)
{
    if (!m_options.paths.staticFilesDirectory.endsWith("/"))
    {
        m_options.paths.staticFilesDirectory += "/";
    }
    if (!m_options.paths.wsRequestPage.startsWith("/"))
    {
        m_options.paths.wsRequestPage = "/" + m_options.paths.wsRequestPage;
    }
}

static void printMessage(stringstream& logMessage, const String& prefix, const RequestInfo::Message& message)
{
    constexpr size_t maxContentLength = 512;

    logMessage << prefix;
    logMessage << message.content().length() << "/" << message.compressedLength()
               << " bytes ";
    if (!message.contentEncoding().empty())
    {
        logMessage << "(" << message.contentEncoding() << ") ";
    }
    String content(message.content().c_str());
    if (content.length() > maxContentLength)
    {
        logMessage << content.substr(0, maxContentLength) << "..";
    }
    else
    {
        logMessage << content;
    }
}

void WSConnection::processSingleConnection(bool& done)
{
    m_logger.debug("Processing connection");

    if (constexpr chrono::seconds readTimeout30sec(30);
        !socket().readyToRead(readTimeout30sec) // Client communication timeout
        || socket().socketBytes() == 0)         // Client closed connection
    {
        socket().close();
        done = true;
        m_logger.debug("Client closed connection");
        return;
    }

    StopWatch requestStopWatch;
    requestStopWatch.start();

    Buffer contentBuffer;
    HttpReader httpReader(socket(), contentBuffer, HttpReader::ReadMode::REQUEST);

    String protocolName = "http";
    httpReader.readHttpHeaders();
    auto& headers = httpReader.getHttpHeaders();

    String requestType = httpReader.getRequestType();
    URL url(httpReader.getRequestURL());

    if (requestType == "OPTIONS")
    {
        respondToOptions(headers);
        if (headers["Connection"].toLowerCase() == "close")
        {
            httpReader.close();
            done = true;
        }
        m_logger.debug("Processed OPTIONS");
        return;
    }

    if (url.params().has("wsdl"))
    {
        protocolName = "wsdl";
    }

    bool processed = false;

    if (protocolName == "http")
    {
        processed = handleHttpProtocol(requestType, url, protocolName, headers);
    }

    if (protocolName == "websocket")
    {
        WSWebSocketsProtocol protocol(&socket(), headers);
        protocol.process();
        processed = true;
    }

    if (processed)
    {
        m_logger.debug("Processed " + protocolName);
        return;
    }

    bool closeConnection = reviewHeaders(requestType, headers);

    WSWebServiceProtocol protocol(httpReader, url, m_services, server().host(),
                                  m_options.allowCors, m_options.keepAlive, m_options.suppressHttpStatus);
    auto requestInfo = protocol.process();

    if (closeConnection)
    {
        httpReader.close();
        done = true;
    }

    requestStopWatch.stop();

    logConnectionDetails(requestStopWatch, httpReader, requestInfo);
}

void WSConnection::run()
{
    Buffer data;

    // Read request data
    String row;
    Strings matches;
    String protocolName;
    bool done {false};

    while (!done && socket().active())
    {
        try
        {
            processSingleConnection(done);
        }
        catch (const Exception& e)
        {
            if (!terminated())
            {
                m_logger.error("Error in incoming connection: " + String(e.what()));
            }
        }
    }
}

void WSConnection::logConnectionDetails(const StopWatch& requestStopWatch, const HttpReader& httpReader,
                                        const RequestInfo& requestInfo)
{
    if (!m_options.logDetails.empty())
    {
        stringstream logMessage;
        bool listStarted = false;

        if (m_options.logDetails.has(LogDetails::MessageDetail::SOURCE_IP))
        {
            auto remoteIp = address();

            if (auto remoteIpHeader = httpReader.httpHeader("Remote-Ip");
                remoteIp == "127.0.0.1" && !remoteIpHeader.empty())
            {
                remoteIp = remoteIpHeader;
            }
            logMessage << "[" << remoteIp << "] ";
        }

        if (m_options.logDetails.has(LogDetails::MessageDetail::REQUEST_NAME))
        {
            logMessage << "(" << requestInfo.name << ") ";
        }

        if (m_options.logDetails.has(LogDetails::MessageDetail::REQUEST_DURATION))
        {
            if (listStarted)
            {
                logMessage << ", ";
            }
            listStarted = true;
            logMessage << "duration " << fixed << setprecision(1) << requestStopWatch.milliseconds() << " ms";
        }

        if (m_options.logDetails.has(LogDetails::MessageDetail::REQUEST_DATA))
        {
            if (listStarted)
            {
                logMessage << ", ";
            }
            listStarted = true;
            printMessage(logMessage, "IN ", requestInfo.request);
        }

        if (m_options.logDetails.has(LogDetails::MessageDetail::RESPONSE_DATA))
        {
            if (listStarted)
            {
                logMessage << ", ";
            }
            printMessage(logMessage, "OUT ", requestInfo.response);
        }

        m_logger.debug(logMessage.str());
    }
}

bool WSConnection::reviewHeaders(const String& requestType, HttpHeaders& headers)
{
    if (String contentLength = headers["Content-Length"];
        requestType == "GET" && contentLength.empty())
    {
        headers["Content-Length"] = "0";
    }

    bool closeConnection = headers["Connection"].toLowerCase() == "close";
    if (closeConnection)
    {
        headers.erase("Connection");
    }

    return closeConnection;
}

bool WSConnection::handleHttpProtocol(const String& requestType, URL& url, String& protocolName,
                                      HttpHeaders& headers) const
{
    String contentType = headers["Content-Type"];
    bool processed = false;
    if (contentType.find("/json") != string::npos || requestType == "POST")
    {
        protocolName = "rest";
    }
    else if (contentType.find("/xml") != string::npos)
    {
        protocolName = "WS";
    }
    else
    {
        if (headers["Upgrade"] == "websocket")
        {
            WSWebSocketsProtocol protocol(&socket(), headers);
            protocol.process();
            processed = true;
        }
        else if (url.path() != m_options.paths.wsRequestPage)
        {
            if (url.path() == "/")
            {
                url.path(m_options.paths.htmlIndexPage);
            }

            WSStaticHttpProtocol protocol(&socket(), url, headers, m_options.paths.staticFilesDirectory);
            protocol.process();
            processed = true;
        }
    }
    return processed;
}

void WSConnection::respondToOptions(const HttpHeaders& headers) const
{
    auto itor = headers.find("origin");
    auto origin = itor->second;
    Buffer response;

    response.append("HTTP/1.1 204 No Content\r\n");

    if (m_options.keepAlive)
    {
        response.append("Connection: keep-alive\r\n");
    }

    if (m_options.allowCors)
    {
        response.append("Access-Control-Allow-Origin: *\r\n");
        response.append("Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n");
        response.append(
            "Access-Control-Allow-Headers: Content-Type, Content-Length, Content-Encoding, Access-Control-Allow-Origin, Authorization\r\n");
    }
    else
    {
        response.append("Access-Control-Allow-Origin: null\r\n");
    }

    response.append("Access-Control-Max-Age: 86400\r\n");

    response.append("\r\n", 2);
    socket().write(response);
}

WSSSLConnection::WSSSLConnection(TCPServer& server, SOCKET connectionSocket, const sockaddr_in* addr,
                                 WSServices& services,
                                 LogEngine& logEngine, const Options& options)
    : WSConnection(server, addr, services, logEngine, options)
{
    if (options.encrypted)
    {
        const auto& sslKeys = server.getSSLKeys();
        auto socket = make_shared<SSLSocket>();
        socket->loadKeys(sslKeys);
        setSocket(socket);
    }
    else
    {
        setSocket(make_shared<TCPSocket>());
    }
    socket().attach(connectionSocket, true);
}
