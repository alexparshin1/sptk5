/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include "sptk5/wsdl/WSConnection.h"
#include <sptk5/wsdl/WSListener.h>

using namespace std;
using namespace sptk;

WSListener::WSListener(const WSServices& services, LogEngine& logger, const String& hostname, size_t threadCount,
                       const WSConnection::Options& options)
    : TCPServer(services.get("").title(), threadCount, &logger, options.logDetails),
      m_services(services),
      m_logger(logger),
      m_options(options)
{
    if (!hostname.empty())
    {
        host(Host(hostname));
    }

    if (m_options.paths.htmlIndexPage.empty())
    {
        m_options.paths.htmlIndexPage = "index.html";
    }
    if (m_options.paths.wsRequestPage.empty())
    {
        m_options.paths.wsRequestPage = "request";
    }
}

SServerConnection WSListener::createConnection(SOCKET connectionSocket, sockaddr_in* peer)
{
    return make_shared<WSSSLConnection>(*this, connectionSocket, peer, m_services, m_logger.destination(), m_options);
}
