/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSConnection.cpp - description                         ║
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

#include "WSConnection.h"

using namespace std;
using namespace sptk;

WSConnection::WSConnection(SOCKET connectionSocket, sockaddr_in* addr, WSRequest& service, Logger& logger,
                           const String& staticFilesDirectory, const String& htmlIndexPage, const String& wsRequestPage)
        : ServerConnection(connectionSocket, "WSConnection"), m_service(service), m_logger(logger),
          m_staticFilesDirectory(staticFilesDirectory), m_htmlIndexPage(htmlIndexPage), m_wsRequestPage(wsRequestPage)
{
    if (!m_staticFilesDirectory.endsWith("/"))
        m_staticFilesDirectory += "/";
    if (!m_wsRequestPage.startsWith("/"))
        m_wsRequestPage = "/" + m_wsRequestPage;
}

void WSConnection::threadFunction()
{
    RegularExpression parseProtocol("^(GET|POST) (\\S+)", "i");
    RegularExpression parseHeader("^([^:]+): \"{0,1}(.*)\"{0,1}$", "i");

    Buffer data;

    // Read request data
    String row;
    Strings matches;
    String protocolName, url, requestType;

    try {
        if (!m_socket->readyToRead(chrono::seconds(30))) {
            m_socket->close();
            m_logger.debug("Client closed connection");
            return;
        }

        HttpHeaders headers;

        String request;

        try {
            while (!terminated()) {
                if (m_socket->readLine(data) == 0)
                    return;
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
        catch (Exception& e) {
            m_logger.error(e.message());
            return;
        }
        catch (exception& e) {
            m_logger.error(e.what());
            return;
        }

        if (protocolName == "http") {

            String contentType = headers["Content-Type"];
            if (contentType.find("/json") != string::npos)
                protocolName = "rest";
            else {

                if (headers["Upgrade"] == "websocket") {
                    WSWebSocketsProtocol protocol(m_socket, headers);
                    protocol.process();
                    return;
                }

                if (url != m_wsRequestPage) {
                    if (url == "/")
                        url = m_htmlIndexPage;

                    WSStaticHttpProtocol protocol(m_socket, url, headers, m_staticFilesDirectory);
                    protocol.process();
                    return;
                }
            }
        }

        if (protocolName == "websocket") {
            WSWebSocketsProtocol protocol(m_socket, headers);
            protocol.process();
            return;
        }

        WSWebServiceProtocol protocol(m_socket, url, headers, m_service);
        protocol.process();
    }
    catch (exception& e) {
        if (!terminated())
            m_logger.error("Error in thread " + name() + ": " + string(e.what()));
    }
    catch (...) {
        if (!terminated())
            m_logger.error("Unknown error in thread " + name());
    }
}

WSSSLConnection::WSSSLConnection(SOCKET connectionSocket, sockaddr_in* addr, WSRequest& service, Logger& logger,
                                 const String& staticFilesDirectory, const String& htmlIndexPage,
                                 const String& wsRequestPage, bool encrypted)
: WSConnection(connectionSocket, addr, service, logger, staticFilesDirectory, htmlIndexPage, wsRequestPage)
{
    if (encrypted)
        m_socket = new SSLSocket;
    else
        m_socket = new TCPSocket;
    m_socket->attach(connectionSocket);
}

WSSSLConnection::~WSSSLConnection()
{
    delete m_socket;
    m_socket = nullptr;
}
