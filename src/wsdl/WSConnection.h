/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSConnection.h - description                           ║
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

#ifndef __WS_CONNECTION_H__
#define __WS_CONNECTION_H__

#include "protocol/WSStaticHttpProtocol.h"
#include "protocol/WSWebServiceProtocol.h"
#include "protocol/WSWebSocketsProtocol.h"
#include <sptk5/wsdl/WSRequest.h>

namespace sptk {

class WSConnection : public ServerConnection
{
protected:
    WSRequest&  m_service;
    Logger&     m_logger;
    String      m_staticFilesDirectory;
    String      m_htmlIndexPage;
    String      m_wsRequestPage;

public:
    WSConnection(SOCKET connectionSocket, sockaddr_in* addr, WSRequest& service, Logger& logger,
                 const String& staticFilesDirectory, const String& htmlIndexPage, const String& wsRequestPage);

    void threadFunction() override;
};

/**
 * @brief WS server connection
 */
class WSSSLConnection : public WSConnection
{
public:
    /**
     * @brief Constructor
     * @param connectionSocket SOCKET, Already accepted by accept() function incoming connection socket
     */
    WSSSLConnection(SOCKET connectionSocket, sockaddr_in* addr, WSRequest& service, Logger& logger,
                    const String& staticFilesDirectory, const String& htmlIndexPage,
                    const String& wsRequestPage, bool encrypted);

    /**
     * @brief Destructor
     */
    virtual ~WSSSLConnection();
};

}

#endif
