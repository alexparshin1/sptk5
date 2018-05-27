/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSListener.h - description                             ║
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

#ifndef __SPTK_WSLISTENER_H__
#define __SPTK_WSLISTENER_H__

#include <sptk5/cutils>
#include <sptk5/cnet>
#include <sptk5/wsdl/WSRequest.h>

namespace sptk {

/**
 * @addtogroup wsdl WSDL-related Classes
 * @{
 */

/**
 * @brief Web Service Listener
 *
 * Simple server to accept Web Service requests.
 * Actual request processing is implemented in Web Service request processor,
 * passed to constructor.
 * As a bonus, WSListener also serves static files, located in staticFilesDirectory.
 * That may be used to implement a web application.
 */
class WSListener : public TCPServer
{
protected:
    /**
     * Web Service request processor
     */
    sptk::WSRequest&    m_service;

    /**
     * Logger object
     */
    sptk::Logger&       m_logger;

    /**
     * Web Service static files directory
     */
    const std::string   m_staticFilesDirectory;


    /**
     * @brief Creates connection thread derived from CTCPServerConnection
     *
     * Application should override this method to create concrete connection object.
     * Created connection object is maintained by CTCPServer.
     * @param connectionSocket SOCKET, Already accepted incoming connection socket
     * @param peer sockaddr_in*, Incoming connection information
     */
    virtual sptk::ServerConnection* createConnection(SOCKET connectionSocket, sockaddr_in* peer);

public:
    /**
     * @brief Constructor
     * @param service sptk::CWSRequest&, Web Service request processor
     * @param logger sptk::Logger&, Logger
     * @param staticFilesDirectory const std::string&, Web Service static files directory
     */
    WSListener(sptk::WSRequest& service, sptk::Logger& logger, const std::string& staticFilesDirectory);
 };

/**
 * @}
 */

}

#endif
