/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQSubscriptions.cpp - description                     ║
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

#include <sptk5/md5.h>
#include <smq/server/SMQSubscriptions.h>

using namespace std;
using namespace sptk;

void SMQSubscriptions::deliverMessage(const String& queueName, const SMessage message)
{
    SharedLock(m_mutex);
    auto itor = m_subscriptions.find(queueName);
    if (itor != m_subscriptions.end())
        itor->second->deliverMessage(message);
}

void SMQSubscriptions::subscribe(SMQConnection* connection, const map<String,sptk::QOS>& queueNames)
{
    UniqueLock(m_mutex);

    for (auto& qtor: queueNames) {
        auto& queueName = qtor.first;
        auto  qos = qtor.second;
        // Does subscription already exist?
        SharedSMQSubscription subscription;
        auto itor = m_subscriptions.find(queueName);
        if (itor == m_subscriptions.end()) {
            SMQSubscription::Type subscriptionType = queueName.startsWith("/queue/") ? SMQSubscription::QUEUE
                                                                                     : SMQSubscription::TOPIC;
            subscription = make_shared<SMQSubscription>(subscriptionType, qos);
            m_subscriptions[queueName] = subscription;
        } else
            subscription = itor->second;
        subscription->addConnection(connection);
    }
}

void SMQSubscriptions::unsubscribe(SMQConnection* connection, const String& queueName)
{
    UniqueLock(m_mutex);

    // Does subscription already exist?
    SharedSMQSubscription subscription;
    auto itor = m_subscriptions.find(queueName);
    if (itor == m_subscriptions.end())
        return;

    subscription = itor->second;
    subscription->removeConnection(connection, true);
}

void SMQSubscriptions::clear()
{
    UniqueLock(m_mutex);
    m_subscriptions.clear();
}
