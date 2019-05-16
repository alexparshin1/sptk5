/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQServer.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday December 23 2018                                ║
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

#ifndef __SMQ_SERVER_H__
#define __SMQ_SERVER_H__

#include <smq/server/SMQSubscriptions.h>
#include <smq/server/SMQConnection.h>
#include <smq/server/SMQSendThreadPool.h>
#include <smq/protocols/MQProtocol.h>

namespace smq {

class SP_EXPORT SMQServer : public sptk::TCPServer
{
    friend class SMQConnection;

    mutable std::mutex              m_mutex;
    MQProtocolType                  m_protocol;
public:
    MQProtocolType protocol() const;

private:
    sptk::String                    m_username;
    sptk::String                    m_password;
    std::set<sptk::String>          m_clientIds;
    std::set<SMQConnection*>        m_connections;
    sptk::SocketEvents              m_socketEvents;
    SMQSubscriptions                m_subscriptions;
    sptk::LogEngine&                m_logEngine;
    uint8_t                         m_debugLogFilter;

    SMQSendThreadPool               m_sendThreadPool;

protected:
    static void socketEventCallback(void *userData, sptk::SocketEventType eventType);
    void watchSocket(sptk::TCPSocket& socket, void* userData);
    void forgetSocket(sptk::TCPSocket& socket);
    void clear();

    void execute(sptk::Runable* task) override;
    void run() override;

public:

    SMQServer(MQProtocolType protocol, const sptk::String& username, const sptk::String& password, sptk::LogEngine& logEngine,
              uint8_t debugLogFilter=(LOG_SERVER_OPS|LOG_CONNECTIONS|LOG_SUBSCRIPTIONS|LOG_MESSAGE_OPS));
    ~SMQServer() override;

    void stop() override;

    sptk::ServerConnection* createConnection(SOCKET connectionSocket, sockaddr_in* peer) override;
    void closeConnection(sptk::ServerConnection* connection, bool brokenConnection);
    bool authenticate(const sptk::String& clientId, const sptk::String& username, const sptk::String& password);

    void distributeMessage(SMessage message);

    void subscribe(SMQConnection* connection, const std::map<sptk::String,QOS>& destinations);
    void unsubscribe(SMQConnection* connection, const sptk::String& destination);
};

}

#endif
