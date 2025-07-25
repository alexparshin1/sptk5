/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQConnectionQueue.h - description                     ║
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

#ifndef __SMQ_CONNECTION_QUEUE_H__
#define __SMQ_CONNECTION_QUEUE_H__

#include <smq/Message.h>
#include <sptk5/cthreads>

namespace sptk {

class SMQConnection;

class SMQSendQueue : public Runable
{
    mutable std::mutex          m_mutex;
    SMQConnection&              m_connection;
    std::queue<SMessage>        m_messages;
    ThreadPool&                 m_threadPool;
    std::atomic<bool>           m_processing {false};

    void setProcessing(bool processing);

protected:
    void run() override;
    SMessage getMessage();

public:
    SMQSendQueue(ThreadPool& threadPool, SMQConnection& connection);

    void push(SMessage& message);
};

}

#endif
