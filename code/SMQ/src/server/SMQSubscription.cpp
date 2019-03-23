/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQSubscription.cpp - description                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Friday February 1 2019                                 ║
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

#include <smq/server/SMQSubscription.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

SMQSubscription::SMQSubscription(const String& destination, Type type, sptk::QOS qos, LogEngine& logEngine, uint8_t debugLogFilter)
: m_destination(destination), m_type(type), m_qos(qos), m_logEngine(logEngine), m_debugLogFilter(debugLogFilter),
  m_currentConnection(m_connections.end())
{
}

SMQSubscription::~SMQSubscription()
{
    UniqueLock(m_mutex);
    for (auto* connection: m_connections)
        connection->unsubscribe(m_destination, this);
    m_connections.clear();
}

void SMQSubscription::addConnection(SMQConnection* connection)
{
    UniqueLock(m_mutex);
    m_connections.insert(connection);
    connection->subscribe(m_destination, this);
}

void SMQSubscription::removeConnection(SMQConnection* connection, bool updateConnection)
{
    UniqueLock(m_mutex);
    m_connections.erase(connection);
    if (updateConnection)
        connection->unsubscribe(m_destination, this);
}

bool SMQSubscription::deliverMessage(SMessage message)
{
    Logger logger(m_logEngine, "(SMQ) ");
    SharedLock(m_mutex);

    // If the subscription is TOPIC, send it to every subscriber:
    if (m_type == TOPIC) {
        for (auto subscriber: m_connections) {
            try {
                subscriber->sendMessage(message);
                if (m_debugLogFilter & LOG_MESSAGE_OPS)
                    logger.debug("Sent message to " + subscriber->clientId());
                if (m_debugLogFilter & LOG_MESSAGE_DETAILS)
                    logger.debug(message->toString());
            }
            catch (const Exception& e) {
                logger.error("Can't send message to a subscriber " + subscriber->clientId() + ": " + String(e.what()));
            }
        }
    } else {
        // If the subscription is QUEUE, send it to current subscriber,
        // and switch to next subscriber
        if (m_connections.empty())
            return true;
        if (m_currentConnection == m_connections.end())
            m_currentConnection = m_connections.begin();
        try {
            (*m_currentConnection)->sendMessage(message);
            if (m_debugLogFilter & LOG_MESSAGE_OPS)
                logger.debug("Sent message to " + (*m_currentConnection)->clientId());
            if (m_debugLogFilter & LOG_MESSAGE_DETAILS)
                logger.debug(message->toString());
        }
        catch (const Exception& e) {
            logger.error("Can't send message to a subscriber " + (*m_currentConnection)->clientId() + ": " + String(e.what()));
        }
        ++m_currentConnection;
    }
    return true;
}

SMQSubscription::Type SMQSubscription::type() const
{
    SharedLock(m_mutex);
    return m_type;
}

SMQSubscription::Type SMQSubscription::typeUnlocked() const
{
    return m_type;
}

String SMQSubscription::typeName() const
{
    SharedLock(m_mutex);
    if (m_type == QUEUE)
        return "queue";
    return "topic";
}

String SMQSubscription::typeNameUnlocked() const
{
    if (m_type == QUEUE)
        return "queue";
    return "topic";
}
