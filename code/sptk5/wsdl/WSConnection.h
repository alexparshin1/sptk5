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

#pragma once

#include "WSServices.h"
#include <sptk5/wsdl/WSRequest.h>
#include <sptk5/wsdl/protocol/WSStaticHttpProtocol.h>
#include <sptk5/wsdl/protocol/WSWebServiceProtocol.h>
#include <sptk5/wsdl/protocol/WSWebSocketsProtocol.h>

namespace sptk {

class SP_EXPORT WSConnection
    : public ServerConnection
{
public:
    class Paths
    {
    public:
        String htmlIndexPage;
        String wsRequestPage;
        String staticFilesDirectory;

        Paths(String htmlIndexPage, String wsRequestPage, String staticFilesDirectory)
            : htmlIndexPage(std::move(htmlIndexPage))
            , wsRequestPage(std::move(wsRequestPage))
            , staticFilesDirectory(std::move(staticFilesDirectory))
        {
        }

        Paths(const Paths& other) = default;
    };

    struct Options {
        Paths paths;
        bool encrypted {false};
        bool allowCors {false};
        bool keepAlive {false};
        bool suppressHttpStatus {false};
        LogDetails logDetails;

        Options(const Paths& paths, bool encrypted = false)
            : paths(paths)
            , encrypted(encrypted)
        {
        }
    };

    /**
     * Constructor
     * @param server            Server object
     * @param connectionSocket  Incoming connection socket
     * @param service           Web service object
     * @param logEngine         Logger engine
     * @param options           Connection options
     */
    WSConnection(TCPServer& server, const sockaddr_in* connectionAddress, WSServices& services, LogEngine& logEngine, Options options);

    /**
     * Destructor
     */
    ~WSConnection() override = default;

    /**
     * Thread function
     */
    void run() override;

private:
    WSServices& m_services;
    Logger m_logger;
    Options m_options;

    void respondToOptions(const HttpHeaders& headers) const;

    bool handleHttpProtocol(const String& requestType, URL& url, String& protocolName, HttpHeaders& headers) const;

    static bool reviewHeaders(const String& requestType, HttpHeaders& headers);

    void logConnectionDetails(const StopWatch& requestStopWatch, const HttpReader& httpReader,
                              const RequestInfo& requestInfo);

    void processSingleConnection(bool& done);
};

/**
 * WS server connection
 */
class SP_EXPORT WSSSLConnection
    : public WSConnection
{
public:
    /**
     * Constructor
     * @param server            TCP server
     * @param connectionSocket  Incoming connection socket, accepted by accept() function
     * @param addr              Incoming connection info
     * @param services          Registered services to process incoming connection
     * @param logEngine         Log engine
     * @param options           Connection options
     */
    WSSSLConnection(TCPServer& server, SocketType connectionSocket, const sockaddr_in* addr, WSServices& services,
                    LogEngine& logEngine, const Options& options);

    /**
     * Destructor
     */
    ~WSSSLConnection() override = default;
};

} // namespace sptk
