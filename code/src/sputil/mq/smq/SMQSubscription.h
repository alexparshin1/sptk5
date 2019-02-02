/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQSubscription.h - description                        ║
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

#ifndef __SMQ_SUBSCRIPTION_H__
#define __SMQ_SUBSCRIPTION_H__

#include <sptk5/mq/SMQConnection.h>

namespace sptk {

typedef std::shared_ptr<SMQConnection> SharedSMQConnection;

class SMQSubscription
{
public:
    enum Type
    {
        QUEUE,
        TOPIC
    };
private:
    mutable sptk::SharedMutex               m_mutex;
    Type                                    m_type;

    std::set<SharedSMQConnection>           m_subscribers;
    std::set<SharedSMQConnection>::iterator m_currentSubscriber;

    std::set<String>                        m_queueNames;

public:
    SMQSubscription(Type type);

    std::shared_ptr<SMQSubscription> clone(SharedSMQConnection connection, const String& addQueue, const String& removeQueue);

    void addConnection(SharedSMQConnection connection);
    void removeConnection(SharedSMQConnection connection);
    bool deliverMessage(const String& queue, const Message& message);

    Type type() const;
};

} // namespace sptk

#endif
