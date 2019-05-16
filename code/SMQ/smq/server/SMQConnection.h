/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQConnection.h - description                          ║
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

#ifndef __SMQ_CONNECTION_H__
#define __SMQ_CONNECTION_H__

#include <smq/protocols/SMQProtocol.h>
#include <smq/protocols/MQLastWillMessage.h>
#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPServerConnection.h>
#include <sptk5/net/SocketEvents.h>
#include "SMQSendQueue.h"

namespace smq {

enum SMQLogGroup : uint8_t
{
    LOG_SERVER_OPS            = 1,
    LOG_CONNECTIONS           = 2,
    LOG_SUBSCRIPTIONS         = 4,
    LOG_MESSAGE_OPS           = 8,
    LOG_MESSAGE_DETAILS       = 16
};

class SMQSubscription;

class SMQConnection : public sptk::TCPServerConnection
{
    mutable sptk::SharedMutex                   m_mutex;
    MQProtocolType                              m_protocolType { MP_SMQ };

public:
    MQProtocolType getProtocolType() const;

private:
    std::shared_ptr<MQProtocol>                 m_protocol;
    sptk::String                                m_clientId;
    std::set<SMQSubscription*>                  m_subscriptions;
    std::shared_ptr<MQLastWillMessage>          m_lastWillMessage;
    sptk::LogEngine&                            m_logEngine;
    uint8_t                                     m_debugLogFilter;

    SMQSendQueue                                m_sendQueue;

public:
    SMQConnection(sptk::TCPServer& server, sptk::ThreadPool& sendThreadPool, SOCKET connectionSocket, sockaddr_in* peer, sptk::LogEngine& logEngine, uint8_t debugLogFilter);
    ~SMQConnection() override;

    void run() override;
    void terminate() override;

    MQProtocol& protocol();

    sptk::String clientId() const;
    void setupClient(const sptk::String& id, const sptk::String& lastWillDestination, const sptk::String& lastWillMessage);

    void subscribe(const sptk::String& destination, SMQSubscription* subscription);
    void unsubscribe(const sptk::String& destination, SMQSubscription* subscription);

    void sendMessage(SMessage& message);

    // Low-level operations
    void ack(Message::Type sourceMessageType, const sptk::String& messageId);
    bool readMessage(SMessage& message);
    SMessage getLastWillMessage();
};

}

#endif
