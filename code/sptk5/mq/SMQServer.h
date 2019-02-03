/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQServer.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday December 23 2018                                ║
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

#ifndef __SMQ_SERVER_H__
#define __SMQ_SERVER_H__

#include <sptk5/mq/SMQSubscriptions.h>
#include "SMQConnection.h"

namespace sptk {

class SMQServer : public TCPServer
{
    friend class SMQConnection;

    mutable std::mutex              m_mutex;
    String                          m_username;
    String                          m_password;
    std::set<String>                m_clientIds;
    std::set<SMQConnection*>        m_connections;
    SocketEvents                    m_socketEvents;
    SMQSubscriptions				m_subscriptions;

protected:
    static void socketEventCallback(void *userData, SocketEventType eventType);
    void watchSocket(TCPSocket& socket, void* userData);
    void forgetSocket(TCPSocket& socket);
    void clear();
    void run() override;

public:

    SMQServer(const String& username, const String& password, LogEngine& logEngine);
    ~SMQServer();

    void stop() override;

    ServerConnection* createConnection(SOCKET connectionSocket, sockaddr_in* peer) override;
    void removeConnection(ServerConnection* connection);
    bool authenticate(const String& clientId, const String& username, const String& password);

    void distributeMessage(SMessage message);

    void subscribe(SMQConnection* connection, const String& destination);
	void unsubscribe(SMQConnection* connection, const String& destination);
};

}

#endif
