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

#ifndef __SPTK_WSLISTENER_H__
#define __SPTK_WSLISTENER_H__

#include <sptk5/cutils>
#include <sptk5/cnet>
#include <sptk5/wsdl/WSRequest.h>
#include <sptk5/wsdl/WSConnection.h>

namespace sptk {

/**
 * @addtogroup wsdl WSDL-related Classes
 * @{
 */

/**
 * Web Service Listener
 *
 * Simple server to accept Web Service requests.
 * Actual request processing is implemented in Web Service request processor,
 * passed to constructor.
 * As a bonus, WSListener also serves static files, located in staticFilesDirectory.
 * That may be used to implement a web application.
 */
class SP_EXPORT WSListener : public TCPServer
{
public:
    /**
     * Constructor
     * @param service               Web Service request processor
     * @param logger                Logger
     * @param paths                 Web Service paths
     * @param hostname              This service hostname
     * @param encrypted             True if communication is encrypted
     * @param threadCount           Max number of simultaneously running requests
     * @param allowCORS             Allow CORS (Cross Origin Resourse Sharing)
     * @param keepAlive             Allow keep-alive connections
     * @param suppressHttpStatus    Response HTTP status is always 200 OK, errors are reported in the response content
     * @param logDetails            Log details
     */
    WSListener(WSRequest& service, LogEngine& logger, const WSConnection::Paths& paths,
               const String& hostname, bool encrypted, size_t threadCount, bool allowCORS, bool keepAlive,
               bool suppressHttpStatus, const LogDetails& logDetails = LogDetails());

    /**
     * Get host name of the listener
     * @return host name of the listener
     */
    String hostname() const override;

protected:
    /**
     * Creates connection thread derived from CTCPServerConnection
     *
     * Application should override this method to create concrete connection object.
     * Created connection object is maintained by CTCPServer.
     * @param connectionSocket      Already accepted incoming connection socket
     * @param peer                  Incoming connection information
     */
    ServerConnection* createConnection(SOCKET connectionSocket, sockaddr_in* peer) override;

private:
    mutable SharedMutex m_mutex;                ///< Mutex that protects internal data
    WSRequest&          m_service;              ///< Web Service request processor
    Logger              m_logger;               ///< Logger object
    WSConnection::Paths m_paths;                ///< Pages and paths
    String              m_hostname;             ///< This service hostname
    bool                m_allowCORS;            ///< Allow CORS
    bool                m_keepAlive;            ///< Allow keep-alive connections
    bool                m_suppressHttpStatus;   ///< Response HTTP status is always 200 OK, errors are reported in the response content
    const bool          m_encrypted;            ///< Connection protocol is encrypted flag
};

/**
 * @}
 */

}

#endif
