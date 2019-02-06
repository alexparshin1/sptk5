/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQServer.cpp - description                            ║
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

#include <sptk5/mq/SMQServer.h>
#include <sptk5/mq/SMQConnection.h>
#include <sptk5/mq/SMQSubscription.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

SMQConnection::SMQConnection(TCPServer& server, SOCKET connectionSocket, sockaddr_in*)
: TCPServerConnection(server, connectionSocket)
{
    SMQServer* smqServer = dynamic_cast<SMQServer*>(&server);
    if (smqServer != nullptr) {
        m_protocolType = smqServer->protocol();
        m_protocol = MQProtocol::factory(m_protocolType, socket());
        smqServer->watchSocket(socket(), this);
    }
}

SMQConnection::~SMQConnection()
{
    SMQServer* smqServer = dynamic_cast<SMQServer*>(&server());
    if (smqServer != nullptr && socket().active())
        smqServer->forgetSocket(socket());

    UniqueLock(m_mutex);

    for (auto* subscription: m_subscriptions)
        subscription->removeConnection(this, false);
    m_subscriptions.clear();

    socket().close();
}

void SMQConnection::terminate()
{
    socket().close();
}

void SMQConnection::run()
{
    // SMQServer doesn't tasks
    return;
}

String SMQConnection::clientId() const
{
    SharedLock(m_mutex);
    return m_clientId;
}

void SMQConnection::setupClient(String& id)
{
    UniqueLock(m_mutex);
    m_clientId = id;
}

void SMQConnection::sendMessage(SMessage& message)
{
    sendMessage(message->destination(), message);
}

void SMQConnection::subscribe(SMQSubscription* subscription)
{
    UniqueLock(m_mutex);
    m_subscriptions.insert(subscription);
}

void SMQConnection::unsubscribe(SMQSubscription* subscription)
{
    UniqueLock(m_mutex);
    m_subscriptions.erase(subscription);
}

MQProtocolType SMQConnection::getProtocolType() const
{
    SharedLock(m_mutex);
    return m_protocolType;
}

void SMQConnection::ack(Message::Type sourceMessageType, const String& messageId)
{
    m_protocol->ack(sourceMessageType, messageId);
}

bool SMQConnection::readMessage(SMessage& message)
{
    protocol().readMessage(message);
    return false;
}

bool SMQConnection::sendMessage(const String& destination, SMessage& message)
{
    protocol().sendMessage(destination, message);
    return false;
}

MQProtocol& SMQConnection::protocol()
{
    return *m_protocol;
}
