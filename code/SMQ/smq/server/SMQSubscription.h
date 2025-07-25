/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQSubscription.h - description                        ║
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

#ifndef __SMQ_SUBSCRIPTION_H__
#define __SMQ_SUBSCRIPTION_H__

#include <smq/server/SMQConnection.h>

namespace sptk {

typedef std::shared_ptr<SMQConnection> SharedSMQConnection;

class SP_EXPORT SMQSubscription
{
    friend class SMQConnection;
public:
    enum Type : uint8_t
    {
        QUEUE,
        TOPIC
    };

private:
    mutable sptk::SharedMutex               m_mutex;
    String                                  m_destination;
    Type                                    m_type;
    QOS                                     m_qos;
    LogEngine&                              m_logEngine;
    uint8_t                                 m_debugLogFilter;

    std::set<SMQConnection*>                m_connections;
    std::set<SMQConnection*>::iterator      m_currentConnection;

protected:
    Type typeUnlocked() const;
    String typeNameUnlocked() const;

public:
    SMQSubscription(const String& destination, Type type, QOS qos, sptk::LogEngine& logEngine, uint8_t debugLogFilter);

    virtual ~SMQSubscription();

    void addConnection(SMQConnection* connection);
    void removeConnection(SMQConnection* connection, bool updateConnection);
    bool deliverMessage(SMessage message);

    Type type() const;
    String typeName() const;

    String destination() const { return m_destination; }
};

} // namespace sptk

#endif
