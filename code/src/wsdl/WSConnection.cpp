/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSConnection.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#include "WSConnection.h"

using namespace std;
using namespace sptk;

WSConnection::WSConnection(TCPServer& server, SOCKET connectionSocket, sockaddr_in*, WSRequest& service,
                           Logger& logger, const String& staticFilesDirectory, const String& htmlIndexPage,
                           const String& wsRequestPage)
: ServerConnection(server, connectionSocket, "WSConnection"), m_service(service), m_logger(logger),
  m_staticFilesDirectory(staticFilesDirectory), m_htmlIndexPage(htmlIndexPage), m_wsRequestPage(wsRequestPage)
{
    if (!m_staticFilesDirectory.endsWith("/"))
        m_staticFilesDirectory += "/";
    if (!m_wsRequestPage.startsWith("/"))
        m_wsRequestPage = "/" + m_wsRequestPage;
}

bool WSConnection::readHttpHeaders(String& protocolName, String& request, String& requestType, String& url,
                                   HttpHeaders& headers)
{
    String row;
    Buffer data;
    Strings matches;

    RegularExpression parseProtocol("^(GET|POST) (\\S+)", "i");
    RegularExpression parseHeader("^([^:]+): \"{0,1}(.*)\"{0,1}$", "i");

    try {
        while (!terminated()) {
            if (socket().readLine(data) == 0)
                return false;
            row = trim(data.c_str());
            if (protocolName.empty()) {
                if (row.find("<?xml") == 0) {
                    protocolName = "xml";
                    break;
                }
                if (parseProtocol.m(row, matches)) {
                    request = row;
                    protocolName = "http";
                    requestType = matches[0];
                    url = matches[1];
                    continue;
                }
            }
            if (parseHeader.m(row, matches)) {
                String header = matches[0];
                String value = matches[1];
                headers[header] = value;
                continue;
            }
            if (row.empty()) {
                data.reset();
                break;
            }
        }
    }
    catch (const Exception& e) {
        m_logger.error(e.message());
        return false;
    }
    return true;
}

void WSConnection::run()
{
    Buffer data;

    // Read request data
    String row;
    Strings matches;
    String protocolName;
    String url;
    String requestType;

    try {
        if (!socket().readyToRead(chrono::seconds(30))) {
            socket().close();
            m_logger.debug("Client closed connection");
            return;
        }

        HttpHeaders headers;
        String request;

        if (!readHttpHeaders(protocolName, request, requestType, url, headers))
            return;

        if (url == m_wsRequestPage + "?wsdl")
            protocolName = "wsdl";

        if (protocolName == "http") {

            String contentType = headers["Content-Type"];
            if (contentType.find("/json") != string::npos)
                protocolName = "rest";
            else {

                if (headers["Upgrade"] == "websocket") {
                    WSWebSocketsProtocol protocol(&socket(), headers);
                    protocol.process();
                    return;
                }

                if (url != m_wsRequestPage) {
                    if (url == "/")
                        url = m_htmlIndexPage;

                    WSStaticHttpProtocol protocol(&socket(), url, headers, m_staticFilesDirectory);
                    protocol.process();
                    return;
                }
            }
        }

        if (protocolName == "websocket") {
            WSWebSocketsProtocol protocol(&socket(), headers);
            protocol.process();
            return;
        }

        WSWebServiceProtocol protocol(&socket(), url, headers, m_service, server().hostname(), server().port());
        protocol.process();
    }
    catch (const Exception& e) {
        if (!terminated())
            m_logger.error("Error in thread " + name() + ": " + String(e.what()));
    }
}

WSSSLConnection::WSSSLConnection(TCPServer& server, SOCKET connectionSocket, sockaddr_in* addr, WSRequest& service,
                                 Logger& logger, const String& staticFilesDirectory, const String& htmlIndexPage,
                                 const String& wsRequestPage, bool encrypted)
: WSConnection(server, connectionSocket, addr, service, logger, staticFilesDirectory, htmlIndexPage, wsRequestPage)
{
    if (encrypted) {
        auto& sslKeys = server.getSSLKeys();
        auto* socket = new SSLSocket;
        socket->loadKeys(sslKeys);
        setSocket(socket);
    } else
        setSocket(new TCPSocket);
    socket().attach(connectionSocket);
}
