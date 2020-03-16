/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSConnection.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

WSConnection::WSConnection(TCPServer& server, SOCKET connectionSocket, sockaddr_in*, WSRequest& service, Logger& logger,
                           const Paths& paths, bool allowCORS)
: ServerConnection(server, connectionSocket, "WSConnection"), m_service(service), m_logger(logger),
  m_paths(paths), m_allowCORS(allowCORS)
{
    if (!m_paths.staticFilesDirectory.endsWith("/"))
        m_paths.staticFilesDirectory += "/";
    if (!m_paths.wsRequestPage.startsWith("/"))
        m_paths.wsRequestPage = "/" + m_paths.wsRequestPage;
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
            if (!socket().readyToRead(chrono::seconds(30))) {
                socket().close();
                m_logger.debug("Client closed connection");
                return;
            }

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

            WSWebServiceProtocol protocol(httpReader, url, m_service, server().hostname(), server().port(), m_allowCORS);
            protocol.process();

            if (closeConnection)
                httpReader.close();

            done = true;
        }
        catch (const Exception& e) {
            if (!terminated())
                m_logger.error("Error in thread " + name() + ": " + String(e.what()));
        }
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

void WSConnection::respondToOptions(const HttpHeaders& headers)
{
    auto itor = headers.find("origin");
    auto origin = itor->second;
    Buffer response;
    response.append("HTTP/1.1 204 No Content\r\n");
    response.append("Connection: keep-alive\r\n");
    if (m_allowCORS) {
        response.append("Access-Control-Allow-Origin: *\r\n");
        response.append("Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n");
        response.append("Access-Control-Allow-Headers: Content-Type, Content-Length, Content-Encoding, Access-Control-Allow-Origin\r\n");
    } else {
        response.append("Access-Control-Allow-Origin: null\r\n");
    }

    response.append("Access-Control-Max-Age: 86400\r\n");

    response.append("\r\n", 2);
    socket().write(response);
}

WSSSLConnection::WSSSLConnection(TCPServer& server, SOCKET connectionSocket, sockaddr_in* addr, WSRequest& service,
                                 Logger& logger, const Paths& paths, int options)
: WSConnection(server, connectionSocket, addr, service, logger, paths, options & ALLOW_CORS)
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
