/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQSubscription.cpp - description                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Friday February 1 2019                                 ║
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

#include "SMQSubscription.h"
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

SMQSubscription::SMQSubscription(SMQSubscription::Type type)
: m_type(type), m_currentSubscriber(m_subscribers.end())
{
}

void SMQSubscription::addConnection(SharedSMQConnection connection)
{
    UniqueLock(m_mutex);
    m_subscribers.insert(connection);
}

void SMQSubscription::removeConnection(SharedSMQConnection connection)
{
    UniqueLock(m_mutex);
    m_subscribers.erase(connection);
}

bool SMQSubscription::deliverMessage(const String& queue, const Message& message)
{
    SharedLock(m_mutex);

    // Does this subscriiption incude the queue?
    auto itor = m_queueNames.find(queue);
    if (itor == m_queueNames.end())
        return false;

    // If the subscription is TOPIC, send it to every subscriber:
    if (m_type == TOPIC) {
        for (auto subscriber: m_subscribers) {
            try {
                subscriber->sendMessage(message);
            }
            catch (const Exception& e) {
                CERR("Can't send message to a subscriber " << subscriber->getClientId() << ": " << e.what() << endl);
            }
        }
    } else {
        // If the subscription is QUEUE, send it to current subscriber,
        // and switch to next subscriber
        if (m_subscribers.empty())
            return true;
        if (m_currentSubscriber == m_subscribers.end())
            m_currentSubscriber = m_subscribers.begin();
        try {
            (*m_currentSubscriber)->sendMessage(message);
        }
        catch (const Exception& e) {
            CERR("Can't send message to a subscriber " << (*m_currentSubscriber)->getClientId() << ": " << e.what() << endl);
        }
        ++m_currentSubscriber;
    }
    return true;
}

SMQSubscription::Type SMQSubscription::type() const
{
    SharedLock(m_mutex);
    return m_type;
}

shared_ptr<SMQSubscription> SMQSubscription::clone(SharedSMQConnection connection, const String& addQueue,
                                                   const String& removeQueue)
{
    if (addQueue.empty() && removeQueue.empty())
        throw Exception("Subscription is not modified");

    UniqueLock(m_mutex);

    auto newSubscription = make_shared<SMQSubscription>(m_type);

    newSubscription->m_subscribers.insert(connection);
    m_subscribers.erase(connection);

    newSubscription->m_queueNames = m_queueNames;
    if (!addQueue.empty())
        newSubscription->m_queueNames.insert(addQueue);
    if (!removeQueue.empty())
        newSubscription->m_queueNames.erase(removeQueue);

    return newSubscription;
}
