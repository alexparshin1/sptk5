/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
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

#include <sptk5/net/URL.h>
#include "sptk5/wsdl/WSConnection.h"

using namespace std;
using namespace sptk;

WSConnection::WSConnection(TCPServer& server, SOCKET connectionSocket, const sockaddr_in* connectionAddress, WSRequest& service, Logger& logger,
                           const Paths& paths, bool allowCORS, bool keepAlive, bool suppressHttpStatus, const LogDetails& logDetails)
: ServerConnection(server, connectionSocket, connectionAddress, "WSConnection"), m_service(service), m_logger(logger),
  m_paths(paths), m_allowCORS(allowCORS), m_keepAlive(keepAlive), m_suppressHttpStatus(suppressHttpStatus), m_logDetails(logDetails)
{
    if (!m_paths.staticFilesDirectory.endsWith("/"))
        m_paths.staticFilesDirectory += "/";
    if (!m_paths.wsRequestPage.startsWith("/"))
        m_paths.wsRequestPage = "/" + m_paths.wsRequestPage;
}

static void printMessage(stringstream& logMessage, const String& prefix, const RequestInfo::Message& message)
{
    constexpr size_t maxContentLength = 512;

    logMessage << prefix;
    logMessage << message.content().length() << "/" << message.compressedLength()
               << " bytes ";
    if (!message.contentEncoding().empty())
        logMessage << "(" << message.contentEncoding() << ") ";
    String content(message.content().c_str());
    if (content.length() > maxContentLength)
        logMessage << content.substr(0, maxContentLength) << "..";
    else
        logMessage << content;
}

void WSConnection::run()
{
    Buffer  data;

    // Read request data
    String  row;
    Strings matches;
    String  protocolName;
    bool    done {false};

    while (!done) {
        try {
            if (!socket().active()) {
                // We closed connection
                break;
            }

            if (!socket().readyToRead(chrono::seconds(30))) {
                socket().close();
                // Client communication timeout
                break;
            }

            if (socket().socketBytes() == 0) {
                // Client closed connection
                break;
            }

            StopWatch requestStopWatch;
            requestStopWatch.start();

            Buffer contentBuffer;
            HttpReader httpReader(socket(), contentBuffer, HttpReader::REQUEST);

            protocolName = "http";
            if (!httpReader.readHttpHeaders()) {
                m_logger.error("Can't read HTTP request");
                return;
            }

            auto& headers = httpReader.getHttpHeaders();

            String requestType = httpReader.getRequestType();
            URL url(httpReader.getRequestURL());

            if (requestType == "OPTIONS") {
                respondToOptions(headers);
                if (headers["Connection"].toLowerCase() == "close") {
                    httpReader.close();
                    done = true;
                }
                continue;
            }

            if (url.params().has("wsdl"))
                protocolName = "wsdl";

            if (protocolName == "http" && handleHttpProtocol(requestType, url, protocolName, headers))
                return;

            if (protocolName == "websocket") {
                WSWebSocketsProtocol protocol(&socket(), headers);
                protocol.process();
                return;
            }

            bool closeConnection = reviewHeaders(requestType, headers);

            WSWebServiceProtocol protocol(httpReader, url, m_service, server().host(),
                                          m_allowCORS, m_keepAlive, m_suppressHttpStatus);
            auto requestInfo = protocol.process();

            if (closeConnection) {
                httpReader.close();
                done = true;
            }

            requestStopWatch.stop();

            logConnectionDetails(requestStopWatch, httpReader, requestInfo);
        }
        catch (const Exception& e) {
            if (!terminated())
                m_logger.error("Error in incoming connection: " + String(e.what()));
        }
    }
}

void WSConnection::logConnectionDetails(const StopWatch& requestStopWatch, const HttpReader& httpReader,
                                        const RequestInfo& requestInfo) const
{
    if (!m_logDetails.empty()) {
        stringstream logMessage;
        bool listStarted = false;

        if (m_logDetails.has(LogDetails::SOURCE_IP)) {
            auto remoteIp = address();
            auto remoteIpHeader = httpReader.httpHeader("Remote-Ip");
            if (remoteIp == "127.0.0.1" && !remoteIpHeader.empty())
                remoteIp = remoteIpHeader;
            logMessage << "[" << remoteIp << "] ";
        }

                if (m_logDetails.has(LogDetails::REQUEST_NAME)) {
                    logMessage << "(" << requestInfo.name << ") ";
                }

        if (m_logDetails.has(LogDetails::REQUEST_DURATION)) {
            if (listStarted)
                logMessage << ", ";
            listStarted = true;
            logMessage << "duration " << fixed << setprecision(1) << requestStopWatch.seconds() * 1000 << " ms";
        }

        if (m_logDetails.has(LogDetails::REQUEST_DATA)) {
            if (listStarted)
                logMessage << ", ";
            listStarted = true;
            printMessage(logMessage, "IN ", requestInfo.request);
        }

        if (m_logDetails.has(LogDetails::RESPONSE_DATA)) {
            if (listStarted)
                logMessage << ", ";
            printMessage(logMessage, "OUT ", requestInfo.response);
        }

        m_logger.debug(logMessage.str());
    }
}

bool WSConnection::reviewHeaders(const String& requestType, HttpHeaders& headers) const
{
    String contentLength = headers["Content-Length"];
    if (requestType == "GET" && contentLength.empty())
        headers["Content-Length"] = "0";

    bool closeConnection = headers["Connection"].toLowerCase() == "close";
    if (closeConnection)
        headers.erase("Connection");

    return closeConnection;
}

bool WSConnection::handleHttpProtocol(const String& requestType, URL& url, String& protocolName,
                                      HttpHeaders& headers) const
{
    String contentType = headers["Content-Type"];
    bool processed= false;
    if (contentType.find("/json") != string::npos)
        protocolName = "rest";
    else if (contentType.find("/xml") != string::npos)
        protocolName = "WS";
    else if (requestType == "POST")
        protocolName = "rest";
    else {
        if (headers["Upgrade"] == "websocket") {
            WSWebSocketsProtocol protocol(&socket(), headers);
            protocol.process();
            processed = true;
        }
        else if (url.path() != m_paths.wsRequestPage) {
            if (url.path() == "/")
                url.path(m_paths.htmlIndexPage);

            WSStaticHttpProtocol protocol(&socket(), url, headers, m_paths.staticFilesDirectory);
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

    if (m_keepAlive)
        response.append("Connection: keep-alive\r\n");

    if (m_allowCORS) {
        response.append("Access-Control-Allow-Origin: *\r\n");
        response.append("Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n");
        response.append("Access-Control-Allow-Headers: Content-Type, Content-Length, Content-Encoding, Access-Control-Allow-Origin, Authorization\r\n");
    } else {
        response.append("Access-Control-Allow-Origin: null\r\n");
    }

    response.append("Access-Control-Max-Age: 86400\r\n");

    response.append("\r\n", 2);
    socket().write(response);
}

WSSSLConnection::WSSSLConnection(TCPServer& server, SOCKET connectionSocket, const sockaddr_in* addr, WSRequest& service,
                                 Logger& logger, const Paths& paths, int options, const LogDetails& logDetails)
: WSConnection(server, connectionSocket, addr, service, logger, paths,
               (options & ALLOW_CORS) != 0,
               (options & KEEP_ALIVE) != 0,
               (options & SUPPRESS_HTTP_STATUS) != 0,
               logDetails)
{
    if (options & ENCRYPTED) {
        auto& sslKeys = server.getSSLKeys();
        auto* socket = new SSLSocket;
        socket->loadKeys(sslKeys);
        setSocket(socket);
    } else
        setSocket(new TCPSocket);
    socket().attach(connectionSocket, true);
}
