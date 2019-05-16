/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQConnection.cppQueue - description                   ║
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

#include <SMQ/smq/server/SMQConnection.h>

using namespace std;
using namespace sptk;
using namespace smq;

SMQSendQueue::SMQSendQueue(ThreadPool& threadPool, SMQConnection& connection)
: Runable("SMQ Send Queue"), m_connection(connection), m_threadPool(threadPool)
{}

void SMQSendQueue::push(SMessage& message)
{
    lock_guard<mutex> lock(m_mutex);
    m_messages.push(message);
    if (!m_processing) {
        m_processing = true;
        m_threadPool.execute(this);
    }
}

void SMQSendQueue::run()
{
    setProcessing(true);
    while (true) {
        SMessage message = getMessage();
        if (!message)
            break;
        m_connection.protocol().sendMessage(message->destination(), message);
    }
}

SMessage SMQSendQueue::getMessage()
{
    SMessage message;

    lock_guard<mutex> lock(m_mutex);

    if (!m_messages.empty()) {
        message = m_messages.front();
        m_messages.pop();
    }

    m_processing = !m_messages.empty();

    return message;
}

void SMQSendQueue::setProcessing(bool processing)
{
    lock_guard<mutex> lock(m_mutex);
    m_processing = processing;
}
