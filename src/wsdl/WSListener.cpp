/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSListener.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

#include "protocol/WSStaticHttpProtocol.h"
#include "protocol/WSWebServiceProtocol.h"
#include "protocol/WSWebSocketsProtocol.h"
#include <sptk5/wsdl/WSListener.h>

using namespace std;
using namespace sptk;

class WSConnection : public TCPServerConnection
{
protected:
    WSRequest&      m_service;
    Logger&         m_logger;
    string          m_staticFilesDirectory;

public:
    WSConnection(SOCKET connectionSocket, sockaddr_in* addr, WSRequest& service, Logger& logger, const string& staticFilesDirectory)
    : TCPServerConnection(connectionSocket), m_service(service), m_logger(logger), m_staticFilesDirectory(staticFilesDirectory)
    {
    }

    void threadFunction() override;
};

void WSConnection::threadFunction()
{
    static const RegularExpression parseProtocol("^(GET|POST) (\\S+)", "i");
    static const RegularExpression parseHeader("^([^:]+): \"{0,1}(.*)\"{0,1}$", "i");

    Buffer data;

    // Read request data
    string      row;
    Strings     matches;
    String      protocolName, url, requestType;

    try {
        if (!m_socket->readyToRead(chrono::seconds(30))) {
            m_socket->close();
            m_logger << LP_DEBUG << "Client closed connection" << endl;
            return;
        }

        map<String,String>  headers;

        try {
            while (!terminated()) {
                if (m_socket->readLine(data) == 0)
                    return;
                row = trim(data.c_str());
                if (protocolName.empty()) {
                    if (strstr(row.c_str(), "<?xml") != nullptr) {
                        protocolName = "xml";
                        break;
                    }
                    if (parseProtocol.m(row, matches)) {
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
            m_logger << LP_ERROR << e.message() << endl;
            return;
        }
        catch (exception& e) {
            m_logger << LP_ERROR << e.what() << endl;
            return;
        }

        if (protocolName == "http") {
            if (headers["Upgrade"] == "websocket") {
                WSWebSocketsProtocol protocol(m_socket, headers);
                protocol.process();
                return;
            }

            if (url == "/")
                url = "index.html";

            WSStaticHttpProtocol protocol(m_socket, url, headers, m_staticFilesDirectory);
            protocol.process();
            return;
        }

        if (protocolName == "websocket") {
            WSWebSocketsProtocol protocol(m_socket, headers);
            protocol.process();
            return;
        }

        WSWebServiceProtocol protocol(m_socket, headers, m_service);
        protocol.process();
    }
    catch (exception& e) {
        if (!terminated())
            m_logger << LP_ERROR << "Error in thread " << name() << ": " << e.what() << endl;
    }
    catch (...) {
        if (!terminated())
            m_logger << LP_ERROR << "Unknown error in thread " << name() << endl;
    }
}

WSListener::WSListener(WSRequest& service, Logger& logger, const string& staticFilesDirectory)
: m_service(service), m_logger(logger), m_staticFilesDirectory(staticFilesDirectory)
{
}

ServerConnection* WSListener::createConnection(SOCKET connectionSocket, sockaddr_in* peer)
{
    return new WSConnection(connectionSocket, peer, m_service, m_logger, m_staticFilesDirectory);
}
