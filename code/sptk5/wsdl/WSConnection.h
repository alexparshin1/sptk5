/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSConnection.h - description                           ║
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

#ifndef __WS_CONNECTION_H__
#define __WS_CONNECTION_H__

#include <sptk5/wsdl/protocol/WSStaticHttpProtocol.h>
#include <sptk5/wsdl/protocol/WSWebServiceProtocol.h>
#include <sptk5/wsdl/protocol/WSWebSocketsProtocol.h>
#include <sptk5/wsdl/WSRequest.h>

namespace sptk {

class WSConnection : public ServerConnection
{
public:

    class Paths
    {
    public:
        String  htmlIndexPage;
        String  wsRequestPage;
        String  staticFilesDirectory;
        Paths(String htmlIndexPage, String wsRequestPage, String staticFilesDirectory)
        : htmlIndexPage(std::move(htmlIndexPage)),
          wsRequestPage(std::move(wsRequestPage)),
          staticFilesDirectory(std::move(staticFilesDirectory))
        {
        }
        Paths(const Paths& other) = default;
    };

    WSConnection(TCPServer& server, SOCKET connectionSocket, sockaddr_in*, WSRequest& service,
                 Logger& logger, const Paths& paths);

    /**
     * Destructor
     */
    ~WSConnection() override = default;

    /**
     * Thread function
     */
    void run() override;

private:

    WSRequest&  m_service;
    Logger&     m_logger;
    Paths       m_paths;
};

/**
 * WS server connection
 */
class WSSSLConnection : public WSConnection
{
public:
    /**
     * Constructor
     * @param connectionSocket SOCKET, Already accepted by accept() function incoming connection socket
     */
    WSSSLConnection(TCPServer& server, SOCKET connectionSocket, sockaddr_in* addr, WSRequest& service,
                    Logger& logger, const Paths& paths, bool encrypted);

    /**
     * Destructor
     */
    ~WSSSLConnection() override = default;
};

}

#endif
