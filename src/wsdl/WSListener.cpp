/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSListener.cpp - description                           ║
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
#include <sptk5/wsdl/WSListener.h>

using namespace std;
using namespace sptk;

WSListener::WSListener(WSRequest& service, LogEngine& logger, const String& staticFilesDirectory,
                       const String& indexPage, const String& wsRequestPage, const String& hostname,
                       bool encrypted)
: m_service(service), m_logger(logger), m_staticFilesDirectory(staticFilesDirectory),
  m_indexPage(indexPage.empty() ? "index.html" : indexPage),
  m_wsRequestPage(wsRequestPage.empty() ? "request" : wsRequestPage),
  m_encrypted(encrypted), m_hostname(hostname)
{
}

ServerConnection* WSListener::createConnection(SOCKET connectionSocket, sockaddr_in* peer)
{
    return new WSSSLConnection(connectionSocket, peer, m_service, m_logger, m_staticFilesDirectory, m_indexPage,
                               m_wsRequestPage, m_encrypted);
}
