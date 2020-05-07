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

#include "sptk5/wsdl/WSConnection.h"
#include <sptk5/wsdl/WSListener.h>

using namespace std;
using namespace sptk;

WSListener::WSListener(WSRequest& service, LogEngine& logger, const WSConnection::Paths& paths,
                       const String& hostname, bool encrypted, size_t threadCount, bool allowCORS,
                       const LogDetails& logDetails)
: TCPServer(service.title(), threadCount, nullptr, logDetails),
  m_service(service),
  m_logger(logger),
  m_paths(paths),
  m_hostname(hostname),
  m_allowCORS(allowCORS),
  m_encrypted(encrypted)
{
    if (m_hostname.empty()) {
        char buffer[256];
        gethostname(buffer, sizeof(buffer));
        m_hostname = buffer;
    }

    if (m_paths.htmlIndexPage.empty())
        m_paths.htmlIndexPage = "index.html";
    if (m_paths.wsRequestPage.empty())
        m_paths.wsRequestPage = "request";
}

ServerConnection* WSListener::createConnection(SOCKET connectionSocket, sockaddr_in* peer)
{
    int options = 0;
    if (m_encrypted)
        options |= WSConnection::ENCRYPTED;
    if (m_allowCORS)
        options |= WSConnection::ALLOW_CORS;
    return new WSSSLConnection(*this, connectionSocket, peer, m_service, m_logger, m_paths, options, logDetails());
}

String WSListener::hostname() const
{
    SharedLock(m_mutex);
    return m_hostname;
}
