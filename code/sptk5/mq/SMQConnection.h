/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQConnection.h - description                          ║
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

#ifndef __SMQ_CONNECTION_H__
#define __SMQ_CONNECTION_H__

#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPServerConnection.h>
#include <sptk5/mq/protocols/SMQProtocol.h>
#include <sptk5/net/SocketEvents.h>

namespace sptk {

class SMQSubscription;

class SMQConnection : public TCPServerConnection
{
    mutable SharedMutex             m_mutex;
    MQProtocolType                  m_protocolType { MP_SMQ };
public:
    MQProtocolType getProtocolType() const;

private:
    std::shared_ptr<MQProtocol>     m_protocol;
    String                          m_clientId;
    std::set<SMQSubscription*>      m_subscriptions;

public:
    SMQConnection(TCPServer& server, SOCKET connectionSocket, sockaddr_in*);
    ~SMQConnection() override;

    void run() override;
    void terminate() override;

    MQProtocol& protocol();

    String clientId() const;
    void   setupClient(String& id);

    void subscribe(SMQSubscription* subscription);
    void unsubscribe(SMQSubscription* subscription);

    void sendMessage(SMessage& message);

    // Low-level operations
    void ack(Message::Type sourceMessageType, const String& messageId);
    bool readMessage(SMessage& message);
    bool sendMessage(const String& destination, SMessage& message);
};

}

#endif
