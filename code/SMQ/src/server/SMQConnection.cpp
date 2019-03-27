/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQConnection.cpp - description                        ║
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

#include <smq/server/SMQServer.h>
#include <SMQ/smq/server/SMQConnection.h>


using namespace std;
using namespace sptk;

static String clientLogPrefix(const String& clientId)
{
    return "{" + clientId + "} ";
}

SMQConnection::SMQConnection(TCPServer& server, ThreadPool& sendThreadPool, SOCKET connectionSocket, sockaddr_in*, sptk::LogEngine& logEngine, uint8_t debugLogFilter)
: TCPServerConnection(server, connectionSocket),
  m_logEngine(logEngine),
  m_debugLogFilter(debugLogFilter),
  m_sendQueue(sendThreadPool, *this)
{
    auto* smqServer = dynamic_cast<SMQServer*>(&server);
    if (smqServer != nullptr) {
        m_protocolType = smqServer->protocol();
        m_protocol = MQProtocol::factory(m_protocolType, socket());
        smqServer->watchSocket(socket(), this);
    }
}

SMQConnection::~SMQConnection()
{
    auto* smqServer = dynamic_cast<SMQServer*>(&server());
    if (smqServer != nullptr && socket().active())
        smqServer->forgetSocket(socket());

    UniqueLock(m_mutex);

    for (auto* subscription: m_subscriptions)
        subscription->removeConnection(this, false);
    m_subscriptions.clear();

    socket().close();

    if (m_debugLogFilter & LOG_SUBSCRIPTIONS) {
        Logger logger(m_logEngine, clientLogPrefix(m_clientId));
        logger.debug("Disconnected");
    }
}

void SMQConnection::terminate()
{
    socket().close();
}

void SMQConnection::run()
{
    // SMQServer doesn't run tasks
}

String SMQConnection::clientId() const
{
    SharedLock(m_mutex);
    return m_clientId;
}

void SMQConnection::setupClient(const String& id, const String& lastWillDestination, const String& lastWillMessage)
{
    UniqueLock(m_mutex);
    m_clientId = id;
    if (!lastWillDestination.empty())
        m_lastWillMessage = make_shared<MQLastWillMessage>(lastWillDestination, lastWillMessage);
    else
        m_lastWillMessage.reset();
    if (m_debugLogFilter & LOG_CONNECTIONS) {
        Logger logger(m_logEngine, clientLogPrefix(m_clientId));
        logger.debug("Connected");
    }
}

void SMQConnection::sendMessage(SMessage& message)
{
    m_sendQueue.push(message);
    //protocol().sendMessage(message->destination(), message);
}

void SMQConnection::subscribe(const String& destination, SMQSubscription* subscription)
{
    UniqueLock(m_mutex);
    m_subscriptions.insert(subscription);
    if (m_debugLogFilter & LOG_SUBSCRIPTIONS) {
        Logger logger(m_logEngine, clientLogPrefix(m_clientId));
        logger.debug("Subscribed to " + subscription->typeNameUnlocked() + " " + destination);
    }
}

void SMQConnection::unsubscribe(const String& destination, SMQSubscription* subscription)
{
    UniqueLock(m_mutex);
    if (m_debugLogFilter & LOG_SUBSCRIPTIONS) {
        Logger logger(m_logEngine, clientLogPrefix(m_clientId));
        logger.debug("Subscribed to " + subscription->typeNameUnlocked() + " " + destination);
    }
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

SMessage SMQConnection::getLastWillMessage()
{
    if (!m_lastWillMessage)
        return SMessage();

    auto lastWillMessage = make_shared<Message>(Message::MESSAGE);
    lastWillMessage->destination(m_lastWillMessage->destination());
    lastWillMessage->set(m_lastWillMessage->message());
    return lastWillMessage;
}

MQProtocol& SMQConnection::protocol()
{
    return *m_protocol;
}
